// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include <QIdentityProxyModel>
#include <QTimer>

class QProcess;

class DeviceRenameModel : public QIdentityProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString error MEMBER m_error NOTIFY errorChanged)
    Q_PROPERTY(bool busy MEMBER m_busy NOTIFY busyChanged)
public:
    enum class Role { PulseProperties = Qt::UserRole, HasOverride };
    explicit DeviceRenameModel(QObject *parent = nullptr);

    [[nodiscard]] QVariant data(const QModelIndex &index, int intRole) const override;
    [[nodiscard]] bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

public Q_SLOTS:
    void reset(const QModelIndex &index);

Q_SIGNALS:
    void errorChanged();
    void busyChanged();

private:
    [[nodiscard]] static QHash<QString, QVariantMap> readOverrides();
    [[nodiscard]] QString pulseName(const QModelIndex &index) const;
    [[nodiscard]] QVariant pulseProperties(const QModelIndex &index) const;
    void restartWirePlumber();
    void markBusy();

    QHash<QString, QVariantMap> m_overrides = readOverrides();
    QString m_error;
    bool m_busy = false; // happens when we have data reloading pending
    QTimer m_readyTimer;
};
