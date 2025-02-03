/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2025 David Redondo <kde@david-redondo.de>
*/
#pragma once

#include <QObject>

#include <QDBusConnection>
#include <QDBusMessage>

using namespace Qt::StringLiterals;

struct OsdInhibitor : public QObject {
    Q_OBJECT
public:
    OsdInhibitor()
    {
        auto message =
            QDBusMessage::createMethodCall(u"org.kde.kded6"_s, u"/modules/audioshortcutsservice"_s, u"org.kde.kded6.AudioShortcutsService"_s, u"viewVisible"_s);
        QDBusConnection::sessionBus().send(message);
    }
    ~OsdInhibitor() override
    {
        auto message =
            QDBusMessage::createMethodCall(u"org.kde.kded6"_s, u"/modules/audioshortcutsservice"_s, u"org.kde.kded6.AudioShortcutsService"_s, u"viewHidden"_s);
        QDBusConnection::sessionBus().send(message);
    }
};
