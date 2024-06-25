// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#include "devicerenamemodel.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QProcess>

#include <QMetaMethod>
#include <ranges>

#include <KLocalizedString>

#include <PulseAudioQt/Context>
#include <PulseAudioQt/PulseObject>
#include <PulseAudioQt/Server>

#include "debug.h"

using namespace std::chrono_literals;
using namespace Qt::StringLiterals;

namespace
{
QString configDir()
{
    return QDir::homePath() + "/.config/wireplumber/wireplumber.conf.d"_L1;
}

QString configPath()
{
    return configDir() + "/00-plasma-pa.conf"_L1;
}

void writeOverrides(const QHash<QString, QVariantMap> &overrides)
{
    QJsonArray rules;
    for (const auto &[name, override] : overrides.asKeyValueRange()) {
        QJsonObject rule;
        rule.insert("matches"_L1,
                     QJsonArray({QJsonObject{{"node.name"_L1, name}, //
                                             {"port.monitor"_L1, "!true"_L1}}}));
        rule.insert("actions"_L1, QJsonObject{{"update-props", QJsonObject::fromVariantMap(override)}});
        rules.append(rule);
    }

    QJsonDocument document{QJsonObject{{"monitor.alsa.rules"_L1, rules}}};
#warning TODO bluetooth

    if (!QDir(configDir()).exists()) {
        if (!QDir().mkpath(configDir())) {
            qCWarning(PLASMAPA) << "Failed to create" << configDir();
            return;
        }
    }

    QFile file(configPath());
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        qCWarning(PLASMAPA) << "Failed to open file for writing" << file.fileName();
        return;
    }
    const auto blob = document.toJson();
    const auto written = file.write(blob);
    Q_ASSERT(written == blob.size());
}
} // namespace

class SignalLogger : public QObject
{
    Q_OBJECT
public:
    SignalLogger(QObject *object)
        : QObject(object)
    {
        auto mo = object->metaObject();

        const auto methods = std::views::transform(std::views::iota(mo->methodOffset(), mo->methodCount()), [mo](int i) {
            return mo->method(i);
        });
        for (const auto &method : methods) {
            qDebug() << "method";
            if (method.methodType() != QMetaMethod::Signal) {
                continue;
            }
            qDebug() << "Connecting" << method.name();
            connect(object, method, this, logMethod());
        }
    }

private Q_SLOTS:
    void log()
    {
        auto mo = sender()->metaObject();
        auto signal = mo->method(senderSignalIndex());
        qDebug() << signal.returnMetaType().name() << signal.name();
    }

private:
    QMetaMethod logMethod()
    {
        static auto logMethod = [] {
            auto index = staticMetaObject.indexOfMethod("log()");
            return staticMetaObject.method(index);
        }();
        return logMethod;
    }
};

DeviceRenameModel::DeviceRenameModel(QObject *parent)
    : QIdentityProxyModel(parent)
{
    // This is a bit garbage but the best we can do.
    // We don't know when the reload is actually complete (as in: all devices have been re-enumerated). So we use
    // data changes as indication of busyness.
    // The delay is arbitrary long to be relatively certain that the devices have settled.
    m_readyTimer.setInterval(250ms);
    m_readyTimer.setSingleShot(true);
    connect(&m_readyTimer, &QTimer::timeout, this, [this] {
        if (!m_busy) {
            return;
        }

        if (PulseAudioQt::Context::instance()->state() != PulseAudioQt::Context::State::Ready) {
            return;
        }

        m_readyTimer.stop();
        m_busy = false;
        Q_EMIT busyChanged();
    });

    auto context = PulseAudioQt::Context::instance();
    connect(context, &PulseAudioQt::Context::stateChanged, &m_readyTimer, qOverload<>(&QTimer::start));
    connect(context, &PulseAudioQt::Context::sinkAdded, &m_readyTimer, qOverload<>(&QTimer::start));
    connect(context, &PulseAudioQt::Context::sinkRemoved, &m_readyTimer, qOverload<>(&QTimer::start));
    connect(context, &PulseAudioQt::Context::sinkInputAdded, &m_readyTimer, qOverload<>(&QTimer::start));
    connect(context, &PulseAudioQt::Context::sinkInputRemoved, &m_readyTimer, qOverload<>(&QTimer::start));
    connect(context, &PulseAudioQt::Context::sourceAdded, &m_readyTimer, qOverload<>(&QTimer::start));
    connect(context, &PulseAudioQt::Context::sourceRemoved, &m_readyTimer, qOverload<>(&QTimer::start));
    connect(context, &PulseAudioQt::Context::sourceOutputAdded, &m_readyTimer, qOverload<>(&QTimer::start));
    connect(context, &PulseAudioQt::Context::sourceOutputRemoved, &m_readyTimer, qOverload<>(&QTimer::start));
    connect(context, &PulseAudioQt::Context::clientAdded, &m_readyTimer, qOverload<>(&QTimer::start));
    connect(context, &PulseAudioQt::Context::clientRemoved, &m_readyTimer, qOverload<>(&QTimer::start));
    connect(context, &PulseAudioQt::Context::cardAdded, &m_readyTimer, qOverload<>(&QTimer::start));
    connect(context, &PulseAudioQt::Context::cardRemoved, &m_readyTimer, qOverload<>(&QTimer::start));
    connect(context, &PulseAudioQt::Context::moduleAdded, &m_readyTimer, qOverload<>(&QTimer::start));
    connect(context, &PulseAudioQt::Context::moduleRemoved, &m_readyTimer, qOverload<>(&QTimer::start));
    connect(context, &PulseAudioQt::Context::streamRestoreAdded, &m_readyTimer, qOverload<>(&QTimer::start));
    connect(context, &PulseAudioQt::Context::streamRestoreRemoved, &m_readyTimer, qOverload<>(&QTimer::start));

    new SignalLogger(context);
    new SignalLogger(context->server());
}

QHash<QString, QVariantMap> DeviceRenameModel::readOverrides()
{
    // SPA-JSON is a superset of JSON so we can simply treat things as JSON and we'll be fine.
    // We write JSON we read JSON.

    QFile file(configPath());
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open file for reading" << file.fileName() << file.errorString();
        return {};
    }

    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse config file" << error.error << error.errorString();
        return {};
    }

    QHash<QString, QVariantMap> data;
    const auto configObject = document.object();
    const auto rulesArray = configObject.value("monitor.alsa.rules"_L1).toArray();
#warning TODO bluetooth
    for (const auto &ruleObject : rulesArray) {
        const auto matchesArray = ruleObject["matches"_L1].toArray();
        const auto actionsObject = ruleObject["actions"_L1].toObject();

        Q_ASSERT(matchesArray.size() == 1);
        if (matchesArray.size() != 1) {
            qWarning() << "Invalid matches section" << ruleObject;
            continue;
        }
        Q_ASSERT(actionsObject.contains("update-props"_L1));
        if (!actionsObject.contains("update-props"_L1)) {
            qWarning() << "Invalid update-props section" << ruleObject;
            continue;
        }

        const auto deviceName = matchesArray.at(0).toObject()["node.name"_L1].toString();
        const auto properties = actionsObject["update-props"_L1].toObject().toVariantMap();

        Q_ASSERT(!deviceName.isEmpty());
        Q_ASSERT(!properties.isEmpty());
        if (deviceName.isEmpty() || properties.isEmpty()) {
            qWarning() << "Failed to parse rule." << deviceName.isEmpty() << properties.isEmpty() << ruleObject;
            continue;
        }

        data.insert(deviceName, properties);
    }
    qDebug() << data;
    return data;
}

QVariant DeviceRenameModel::data(const QModelIndex &index, int intRole) const
{
    qDebug() << index << intRole;
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        return {};
    }

    switch (static_cast<Role>(intRole)) {
    case Role::HasOverride: {
        const auto name = pulseName(index);
        if (name.isEmpty()) {
            qWarning() << "Name unexpectedly empty. Cannot override" << index;
            return false;
        }
        return m_overrides.contains(name);
    }
    case Role::PulseProperties:
        const auto base = pulseProperties(index).toMap();
        qDebug() << "base" << base;
        const auto override = m_overrides.value(pulseName(index));
        auto result = base;
        for (const auto &[key, value] : override.asKeyValueRange()) {
            result.insert(key, value);
        }
        return result;
    }

    return QIdentityProxyModel::data(index, intRole);
}

bool DeviceRenameModel::setData(const QModelIndex &index, const QVariant &value, int intRole)
{
    if (!index.isValid()) {
        return false;
    }

    switch (static_cast<Role>(intRole)) {
    case Role::PulseProperties:
        const auto name = pulseName(index);
        if (name.isEmpty()) {
            qWarning() << "Name unexpectedly empty. Cannot override" << index;
            return false;
        }
        beginResetModel();
        const auto base = pulseProperties(index).toMap();
        const auto incoming = value.toMap();
        auto override = m_overrides.value(name);
        for (const auto &[key, value] : incoming.asKeyValueRange()) {
            qDebug() << key << value;
            if (value.toString().isEmpty()) {
                // Empty means delete override
                qDebug() << "  removing";
                override.remove(key);
                continue;
            }

            if (base.value(key) == value) {
                // leave override unchanged in case we overrid things
                qDebug() << "  unchanged";
                continue;
            }

            qDebug() << "  updating";
            // If the values are not equal we override with the new value
            override.insert(key, value);
        }
        qDebug() << override;
        if (override.isEmpty()) {
            m_overrides.remove(name);
        } else {
            m_overrides.insert(name, override); // replace
        }
        endResetModel();
        qDebug() << m_overrides;
        writeOverrides(m_overrides);
        restartWirePlumber();
        return true;
    }

    return false;
}

QHash<int, QByteArray> DeviceRenameModel::roleNames() const
{
    // Do not start out with the original role names. We'd get confused over what
    // is what since this is an IdentityProxy and churns through roles from the source model
    QHash<int, QByteArray> roleNames;
    roleNames.insert(static_cast<int>(Role::PulseProperties), "PulseProperties"_qba);
    roleNames.insert(static_cast<int>(Role::HasOverride), "HasOverride"_qba);
    return roleNames;
}

QString DeviceRenameModel::pulseName(const QModelIndex &index) const
{
    const auto key = sourceModel()->roleNames().key("Name");
    return QIdentityProxyModel::data(index, key).toString();
}

QVariant DeviceRenameModel::pulseProperties(const QModelIndex &index) const
{
    const auto key = sourceModel()->roleNames().key("PulseProperties");
    return QIdentityProxyModel::data(index, key);
}

void DeviceRenameModel::restartWirePlumber()
{
    auto systemctl = new QProcess(this);
    systemctl->setProgram(u"systemctl"_s);
    // NOTE: We also restart pipewire even though it is technically not necessary. The problem is that we need
    // information on our context `State` but that is not provided when only wireplumber restarts (i.e. the
    // pulseaudio socket remains working it just has no devices). To work around this problem we also
    // restart pipewire to forcefully reconnect to the daemon and get state change signals.
    systemctl->setArguments({u"--user"_s, u"restart"_s, u"wireplumber.service"_s, u"pipewire.service"_s});
    connect(systemctl, &QProcess::finished, this, [this, systemctl](int exitCode, QProcess::ExitStatus exitStatus) {
        systemctl->deleteLater();

        // Only here as a safeguard. It will also be started by changes on the pulsaudio context.
        m_readyTimer.start();

        switch (exitStatus) {
        case QProcess::CrashExit:
            qCWarning(PLASMAPA) << "Failed to restart wireplumber.service. systemctl crashed!";
            m_error = xi18nc("@info:status error message",
                             "Changes have not been applied.<nl/>"
                             "Failed to restart wireplumber.service. The command crashed.");
            Q_EMIT errorChanged();
            return;
        case QProcess::NormalExit:
            break;
        }

        if (exitCode != 0) {
            qCWarning(PLASMAPA) << "Failed to restart wireplumber.service. Unexpected exit code" << exitCode;
            m_error = xi18nc("@info:status error message %1 is an integer exit code",
                             "Changes have not been applied.<nl/>"
                             "Failed to restart wireplumber.service. The command terminated with code: %1.",
                             QString::number(exitCode));
            Q_EMIT errorChanged();
            return;
        }

    });

    m_busy = true;
    Q_EMIT busyChanged();

    systemctl->start();
}

void DeviceRenameModel::reset(const QModelIndex &index)
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        return;
    }

    const auto name = pulseName(index);
    if (name.isEmpty()) {
        qCWarning(PLASMAPA) << "Name unexpectedly empty. Cannot override" << index;
        return;
    }

    beginResetModel();
    m_overrides.remove(name);
    endResetModel();
    writeOverrides(m_overrides);
    restartWirePlumber();
}

#include "devicerenamemodel.moc"
