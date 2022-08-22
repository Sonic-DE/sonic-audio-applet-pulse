/*
    SPDX-FileCopyrightText: 2014-2015 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2019 Sefa Eyeoglu <contact@scrumplex.net>
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.kquickcontrolsaddons 2.0
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.private.volume 0.1

PC3.Slider {
    id: control

    property VolumeObject volumeObject

    // Prevents the groove from showing through the handle
    layer.enabled: opacity < 1

    background: PlasmaCore.FrameSvgItem {
        imagePath: "widgets/slider"
        prefix: "groove"
        colorGroup: PlasmaCore.ColorScope.colorGroup
        implicitWidth: control.horizontal ? PlasmaCore.Units.gridUnit * 12 : fixedMargins.left + fixedMargins.right
        implicitHeight: control.vertical ? PlasmaCore.Units.gridUnit * 12 : fixedMargins.top + fixedMargins.bottom

        width: control.horizontal ? Math.max(fixedMargins.left + fixedMargins.right, control.availableWidth) : implicitWidth
        height: control.vertical ? Math.max(fixedMargins.top + fixedMargins.bottom, control.availableHeight) : implicitHeight
        x: control.leftPadding + (control.horizontal ? 0 : Math.round((control.availableWidth - width) / 2))
        y: control.topPadding + (control.vertical ? 0 : Math.round((control.availableHeight - height) / 2))

        PlasmaCore.FrameSvgItem {
            imagePath: "widgets/slider"
            prefix: "groove-highlight"
            colorGroup: PlasmaCore.ColorScope.colorGroup

            LayoutMirroring.enabled: control.mirrored
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            width: control.horizontal ? Math.max(fixedMargins.left + fixedMargins.right, Math.round(control.position * (control.availableWidth - control.handle.width / 2) + (control.handle.width / 2))) : parent.width
            height: control.vertical ? Math.max(fixedMargins.top + fixedMargins.bottom, Math.round(control.position * (control.availableHeight - control.handle.height / 2) + (control.handle.height / 2))) : parent.height
        }

        PlasmaCore.FrameSvgItem {
            imagePath: "widgets/slider"
            prefix: "groove-highlight"

            LayoutMirroring.enabled: control.mirrored
            anchors.left: parent.left
            anchors.bottom: parent.bottom

            implicitWidth: control.horizontal ? Math.max(fixedMargins.left + fixedMargins.right, meter.volume * control.position * control.availableWidth) : parent.width
            implicitHeight: control.vertical ? Math.max(fixedMargins.top + fixedMargins.bottom, meter.volume * control.position * control.availableHeight) : parent.height

            Behavior on implicitWidth {
                enabled: control.horizontal
                NumberAnimation  {
                    id: animateWidth
                    duration: PlasmaCore.Units.shortDuration
                    easing.type: Easing.OutQuad
                }
            }
            Behavior on implicitHeight {
                enabled: control.vertical
                NumberAnimation  {
                    id: animateHeight
                    duration: PlasmaCore.Units.shortDuration
                    easing.type: Easing.OutQuad
                }
            }

            // Animations produce floating point values
            width: Math.round(implicitWidth)
            height: Math.round(implicitHeight)

            opacity: meter.available && (meter.volume > 0 || animateWidth.running || animateHeight.running)
            status: PlasmaCore.FrameSvgItem.Selected
            clip: true // prevents a visual glitch, BUG 434927
        }
    }

    VolumeMonitor {
        id: meter
        target: control.visible ? control.volumeObject : null
    }
}
