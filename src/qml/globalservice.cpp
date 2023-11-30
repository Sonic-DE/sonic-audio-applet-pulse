#include "globalservice.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QString>

using namespace Qt::Literals::StringLiterals;

void GlobalService::globalMute()
{
    invokeShortcut("mute"_L1);
}

void GlobalService::volumeUp()
{
    invokeShortcut("increase_volume"_L1);
}

void GlobalService::volumeDown()
{
    invokeShortcut("decrease_volume"_L1);
}

void GlobalService::volumeUpSmall()
{
    invokeShortcut("increase_volume_small"_L1);
}

void GlobalService::volumeDownSmall()
{
    invokeShortcut("decrease_volume_small"_L1);
}

void GlobalService::invokeShortcut(const QString name)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusMessage invokeMsg =
        QDBusMessage::createMethodCall("org.kde.kglobalaccel"_L1, "/component/kmix"_L1, "org.kde.kglobalaccel.Component"_L1, "invokeShortcut"_L1);
    invokeMsg.setArguments({QVariant(name)});
    bus.call(invokeMsg, QDBus::NoBlock);
}