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
