/*
    SPDX-FileCopyrightText: 2023 Bharadwaj Raju <bharadwaj.raju777@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "audioshortcutsservice.h"

#include "audioicon.h"

#include <QAction>
#include <QStringBuilder>

#include <KGlobalAccel>
#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(AudioShortcutsService, "audioshortcutsservice.json")

const QString OSD_DBUS_SERVICE = "org.kde.plasmashell"_L1;
const QString OSD_DBUS_PATH = "/org/kde/osdService"_L1;

AudioShortcutsService::AudioShortcutsService(QObject *parent, const QList<QVariant> &)
    : KDEDModule(parent)
    , m_sinkModel(new QPulseAudio::SinkModel(this))
    , m_sourceModel(new QPulseAudio::SourceModel(this))
    , m_cardModel(new QPulseAudio::CardModel(this))
    , m_osdDBusInterface(new OsdServiceInterface(OSD_DBUS_SERVICE, OSD_DBUS_PATH, QDBusConnection::sessionBus(), this))
    , m_feedback(new VolumeFeedback(this))
{
    m_configWatcher = KConfigWatcher::create(KSharedConfig::openConfig(QStringLiteral("plasmaparc")));
    connect(m_configWatcher.data(), &KConfigWatcher::configChanged, this, &AudioShortcutsService::loadConfig);
    loadConfig();

    connect(m_sinkModel, &QPulseAudio::SinkModel::defaultSinkChanged, this, &AudioShortcutsService::handleDefaultSinkChange);
    connect(m_sinkModel, &QPulseAudio::SinkModel::rowsInserted, this, &AudioShortcutsService::handleNewSink);

    QList<QAction *> actions;

    QAction *volumeUpAction = new QAction(this);
    actions.append(volumeUpAction);
    volumeUpAction->setObjectName("increase_volume"_L1);
    volumeUpAction->setText(i18n("Increase Volume"));
    volumeUpAction->setShortcut(Qt::Key_VolumeUp);
    connect(volumeUpAction, &QAction::triggered, this, [this]() {
        if (!m_sinkModel->preferredSink()) {
            return;
        }
        int percent = changeVolumePercent(m_sinkModel->preferredSink(), m_volumeStep);
        showVolume(percent);
        playFeedback(-1);
    });

    QAction *volumeDownAction = new QAction(this);
    actions.append(volumeDownAction);
    volumeDownAction->setObjectName("decrease_volume"_L1);
    volumeDownAction->setText(i18n("Decrease Volume"));
    volumeDownAction->setShortcut(Qt::Key_VolumeDown);
    connect(volumeDownAction, &QAction::triggered, this, [this]() {
        if (!m_sinkModel->preferredSink()) {
            return;
        }
        int percent = changeVolumePercent(m_sinkModel->preferredSink(), -m_volumeStep);
        showVolume(percent);
        playFeedback(-1);
    });

    QAction *volumeUpSmallAction = new QAction(this);
    actions.append(volumeUpSmallAction);
    volumeUpSmallAction->setObjectName("increase_volume_small"_L1);
    volumeUpSmallAction->setText(i18n("Increase Volume by 1%"));
    volumeUpSmallAction->setShortcut(Qt::ShiftModifier | Qt::Key_VolumeUp);
    connect(volumeUpSmallAction, &QAction::triggered, this, [this]() {
        if (!m_sinkModel->preferredSink()) {
            return;
        }
        int percent = changeVolumePercent(m_sinkModel->preferredSink(), 1);
        showVolume(percent);
        playFeedback(-1);
    });

    QAction *volumeDownSmallAction = new QAction(this);
    actions.append(volumeDownSmallAction);
    volumeDownSmallAction->setObjectName("decrease_volume_small"_L1);
    volumeDownSmallAction->setText(i18n("Decrease Volume by 1%"));
    volumeDownSmallAction->setShortcut(Qt::ShiftModifier | Qt::Key_VolumeDown);
    connect(volumeDownSmallAction, &QAction::triggered, this, [this]() {
        if (!m_sinkModel->preferredSink()) {
            return;
        }
        int percent = changeVolumePercent(m_sinkModel->preferredSink(), -1);
        showVolume(percent);
        playFeedback(-1);
    });

    QAction *volumeUpMicAction = new QAction(this);
    actions.append(volumeUpMicAction);
    volumeUpMicAction->setObjectName("increase_microphone_volume"_L1);
    volumeUpMicAction->setText(i18n("Increase Microphone Volume"));
    volumeUpMicAction->setShortcut(Qt::Key_MicVolumeUp);
    connect(volumeUpMicAction, &QAction::triggered, this, [this]() {
        if (!m_sourceModel->defaultSource()) {
            return;
        }
        int percent = changeVolumePercent(m_sourceModel->defaultSource(), m_volumeStep);
        showMicVolume(percent);
    });

    QAction *volumeDownMicAction = new QAction(this);
    actions.append(volumeDownMicAction);
    volumeDownMicAction->setObjectName("decrease_microphone_volume"_L1);
    volumeDownMicAction->setText(i18n("Decrease Microphone Volume"));
    volumeDownMicAction->setShortcut(Qt::Key_MicVolumeDown);
    connect(volumeDownMicAction, &QAction::triggered, this, [this]() {
        if (!m_sourceModel->defaultSource()) {
            return;
        }
        int percent = changeVolumePercent(m_sourceModel->defaultSource(), -m_volumeStep);
        showMicVolume(percent);
    });

    QAction *muteAction = new QAction(this);
    actions.append(muteAction);
    muteAction->setObjectName("mute"_L1);
    muteAction->setText(i18n("Mute"));
    muteAction->setShortcut(Qt::Key_VolumeMute);
    connect(muteAction, &QAction::triggered, this, [this]() {
        if (m_globalMute) {
            disableGlobalMute();
        } else {
            enableGlobalMute();
        }
    });

    QAction *muteMicAction = new QAction(this);
    actions.append(muteMicAction);
    muteMicAction->setObjectName("mic_mute"_L1);
    muteMicAction->setText(i18n("Mute Microphone"));
    muteMicAction->setShortcuts({Qt::Key_MicMute, Qt::MetaModifier | Qt::Key_VolumeMute});
    connect(muteMicAction, &QAction::triggered, this, [this]() {
        if (!m_sourceModel->defaultSource()) {
            return;
        }
        const bool toMute = !m_sourceModel->defaultSource()->isMuted();
        m_sourceModel->defaultSource()->setMuted(toMute);
        showMicMute(toMute ? 0 : volumePercent(m_sourceModel->defaultSource()->volume()));
    });

    for (const auto action : actions) {
        action->setProperty("componentName", "kmix"_L1);
        action->setProperty("componentDisplayName", i18n("Audio Volume"));
        KGlobalAccel::setGlobalShortcut(action, action->shortcuts());
    }
}

void AudioShortcutsService::loadConfig()
{
    KConfigGroup group(m_configWatcher->config(), "General"_L1);
    m_volumeStep = group.readEntry(QStringLiteral("VolumeStep"), 5);
    m_raiseMaxVolume = group.readEntry(QStringLiteral("RaiseMaximumVolume"), false);
    m_globalMute = group.readEntry(QStringLiteral("GlobalMute"), false);
}

qint64 AudioShortcutsService::boundVolume(qint64 volume, int maxVolume)
{
    return qMax(QPulseAudio::Context::MinimalVolume, qMin(volume, maxVolume));
}

int AudioShortcutsService::volumePercent(qint64 volume)
{
    return std::round((double)volume / QPulseAudio::Context::NormalVolume * 100.0);
}

int AudioShortcutsService::changeVolumePercent(QPulseAudio::Device *device, int deltaPercent)
{
    const qint64 oldVolume = device->volume();
    const int oldPercent = volumePercent(oldVolume);
    const int targetPercent = oldPercent + deltaPercent;
    const int maxVolume = QPulseAudio::Context::NormalVolume * (m_raiseMaxVolume ? 150 : 100) / 100.f;
    const qint64 newVolume = boundVolume(std::round(QPulseAudio::Context::NormalVolume * (targetPercent / 100.f)), maxVolume);
    const int newPercent = volumePercent(newVolume);
    device->setMuted(newPercent == 0);
    device->setVolume(newVolume);
    return newPercent;
}

void AudioShortcutsService::handleDefaultSinkChange()
{
    const QPulseAudio::Sink *defaultSink = m_sinkModel->defaultSink();
    // we don't want to show the OSD on startup
    if (!m_initialDefaultSinkSet) {
        m_initialDefaultSinkSet = true;
        return;
    }
    KConfigGroup group(m_configWatcher->config(), "General"_L1);
    if (!group.readEntry("DefaultOutputDeviceOsd"_L1, true)) {
        return;
    }
    if (!defaultSink) {
        return;
    }
    QString description = defaultSink->description();
    if (defaultSink->name() == DUMMY_OUTPUT_NAME) {
        description = i18n("No output device");
    } else {
        auto cardIdx = m_cardModel->index(-1, 0);
        for (int i = 0; i < m_cardModel->rowCount(); i++) {
            const auto idx = m_cardModel->index(i, 0);
            if (m_cardModel->data(idx, m_cardModel->role("Index"_ba)) == defaultSink->cardIndex()) {
                cardIdx = idx;
            }
        }
        if (cardIdx.isValid()) {
            const QVariantMap cardProperties = m_cardModel->data(cardIdx, m_cardModel->role("Properties"_ba)).toMap();
            bool convOk = false;
            const int cardBluetoothBattery = cardProperties["bluetooth.battery"_L1].toString().remove('%').toInt(&convOk);
            if (convOk) {
                description = i18nc("Device name (Battery percent)", "%1 (%2% Battery)", description, cardBluetoothBattery);
            }
        }
        QString icon = AudioIcon::forFormFactor(defaultSink->formFactor());
        if (icon.isEmpty()) {
            if (defaultSink->name() == DUMMY_OUTPUT_NAME) {
                icon = "audio-volume-muted"_L1;
            } else {
                icon = AudioIcon::forVolume(volumePercent(defaultSink->volume()), defaultSink->isMuted(), ""_L1);
            }
        }
        m_osdDBusInterface->showText(icon, description);
    }
}

void AudioShortcutsService::handleNewSink()
{
    if (m_globalMute) {
        for (int i = 0; i < m_sinkModel->rowCount(); i++) {
            m_sinkModel->setData(m_sinkModel->index(i, 0), true, m_sinkModel->role("Muted"_ba));
        }
    }
}

void AudioShortcutsService::muteVolume()
{
    if (!m_sinkModel->preferredSink() || m_sinkModel->preferredSink()->name() == DUMMY_OUTPUT_NAME) {
        return;
    }
    if (!m_sinkModel->preferredSink()->isMuted()) {
        enableGlobalMute();
        showMute(0);
    } else {
        if (m_globalMute) {
            disableGlobalMute();
        }
        m_sinkModel->preferredSink()->setMuted(false);
        showMute(m_sinkModel->preferredSink()->volume());
        playFeedback(-1);
    }
}

void AudioShortcutsService::enableGlobalMute()
{
    QStringList alreadyMutedDevices;
    for (int i = 0; i < m_sinkModel->rowCount(); i++) {
        const auto idx = m_sinkModel->index(i, 0);
        const bool isAlreadyMuted = m_sinkModel->data(idx, m_sinkModel->role("Muted"_ba)).toBool();
        const QString name = m_sinkModel->data(idx, m_sinkModel->role("Name"_ba)).toString();
        const QString activePortIdx = QString::number(m_sinkModel->data(idx, m_sinkModel->role("ActivePortIndex"_ba)).toUInt());
        if (isAlreadyMuted) {
            alreadyMutedDevices.append(name % "." % activePortIdx);
        } else {
            m_sinkModel->setData(idx, true, m_sinkModel->role("Muted"_ba));
        }
    }
    if (alreadyMutedDevices.length() == m_sinkModel->rowCount()) {
        alreadyMutedDevices.clear();
    }
    KConfigGroup group(m_configWatcher->config(), "General"_L1);
    group.writeEntry("GlobalMute", true);
    group.writeEntry("GlobalMuteDevices", alreadyMutedDevices);
    group.sync();
    loadConfig();
    showMute(0);
}

void AudioShortcutsService::disableGlobalMute()
{
    KConfigGroup group(m_configWatcher->config(), "General"_L1);
    QStringList keepMutedDevices = group.readEntry("GlobalMuteDevices", QStringList{});
    for (int i = 0; i < m_sinkModel->rowCount(); i++) {
        const auto idx = m_sinkModel->index(i, 0);
        const QString name = m_sinkModel->data(idx, m_sinkModel->role("Name"_ba)).toString();
        const QString activePortIdx = QString::number(m_sinkModel->data(idx, m_sinkModel->role("ActivePortIndex"_ba)).toUInt());
        const QString deviceId = name % "." % activePortIdx;
        if (!keepMutedDevices.contains(deviceId)) {
            m_sinkModel->setData(idx, false, m_sinkModel->role("Muted"_ba));
        }
    }
    group.writeEntry("GlobalMute", false);
    group.writeEntry("GlobalMuteDevices", QStringList{});
    group.sync();
    loadConfig();
    if (m_sinkModel->preferredSink()) {
        showMute(volumePercent(m_sinkModel->preferredSink()->volume()));
        playFeedback(-1);
    }
}

void AudioShortcutsService::playFeedback(int sinkIdx)
{
    KConfigGroup group(m_configWatcher->config(), "General"_L1);
    if (!group.readEntry("AudioFeedback"_L1, true)) {
        return;
    }
    if (sinkIdx == -1 && m_sinkModel->preferredSink()) {
        sinkIdx = m_sinkModel->preferredSink()->index();
    }
    m_feedback->play(sinkIdx);
}

void AudioShortcutsService::showMute(int percent)
{
    KConfigGroup group(m_configWatcher->config(), "General"_L1);
    if (!group.readEntry("MuteOsd"_L1, true)) {
        return;
    }

    m_osdDBusInterface->volumeChanged(percent, m_raiseMaxVolume ? 150 : 100);
}

void AudioShortcutsService::showVolume(int percent)
{
    KConfigGroup group(m_configWatcher->config(), "General"_L1);
    if (!group.readEntry("VolumeOsd"_L1, true)) {
        return;
    }
    m_osdDBusInterface->volumeChanged(percent, m_raiseMaxVolume ? 150 : 100);
}

void AudioShortcutsService::showMicVolume(int percent)
{
    KConfigGroup group(m_configWatcher->config(), "General"_L1);
    if (!group.readEntry("MicrophoneSensitivityOsd"_L1, true)) {
        return;
    }
    m_osdDBusInterface->microphoneVolumeChanged(percent);
}

void AudioShortcutsService::showMicMute(int percent)
{
    KConfigGroup group(m_configWatcher->config(), "General"_L1);
    if (!group.readEntry("MuteOsd"_L1, true)) {
        return;
    }
    m_osdDBusInterface->microphoneVolumeChanged(percent);
}

#include "audioshortcutsservice.moc"