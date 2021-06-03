/*
    SPDX-FileCopyrightText: 2014-2015 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2016 David Rosca <nowrep@gmail.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.14
import QtQuick.Controls 2.14

ComboBox {
    id: combo
    textRole: "Description"
    valueRole: "Index"

    onActivated: DeviceIndex = currentValue

    Component.onCompleted: currentIndex = indexOfValue(DeviceIndex)

    // HACK: Work-around https://bugs.kde.org/show_bug.cgi?id=403153
    // TODO QT6: use `implicitContentWidthPolicy: WidestTextWhenCompleted`
    // https://doc.qt.io/qt-6/qml-qtquick-controls2-combobox.html#implicitContentWidthPolicy-prop
    delegate: ItemDelegate {
        highlighted: combo.highlightedIndex == index
        text: Description
        width: combo.popup.width
        Component.onCompleted: {
            combo.popup.width = Math.min(Math.max(implicitWidth, combo.width, combo.popup.width),
                                         Overlay.overlay.width);
        }
    }
    popup.x: combo.width - combo.popup.width
}
