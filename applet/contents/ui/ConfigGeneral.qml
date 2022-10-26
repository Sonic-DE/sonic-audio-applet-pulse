/*
    SPDX-FileCopyrightText: 2016 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.5
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5 as QQC2

import org.kde.kirigami 2.5 as Kirigami

import org.kde.plasma.private.volume 0.1

Kirigami.FormLayout {
    property alias cfg_showVirtualDevices: showVirtualDevices.checked

    QQC2.CheckBox {
        id: showVirtualDevices
        text: i18n("Show virtual devices")
    }
}
