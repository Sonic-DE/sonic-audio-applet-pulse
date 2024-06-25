// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2014-2024 Harald Sitter <sitter@kde.org>
// SPDX-FileCopyrightText: 2016 David Rosca <nowrep@gmail.com>
// SPDX-FileCopyrightText: 2019 Sefa Eyeoglu <contact@scrumplex.net>
// SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-FileCopyrightText: 2021 Ismael Asensio <isma.af@gmail.com>

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.kde.coreaddons as KCoreAddons
import org.kde.kcmutils as KCM
import org.kde.kitemmodels as KItemModels
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.plasma.private.volume

KCM.ScrollViewKCM {
    id: page

    required property var config
    required property DeviceNameSourceModel deviceNameSourceModel
    property string defaultKey: 'node.nick'

    title: i18nc("@title rename audio devices", "Rename Devices")
    implicitHeight: Kirigami.Units.gridUnit * 28
    implicitWidth: Kirigami.Units.gridUnit * 28

    actions: [
        Kirigami.Action {
            id: sourceAction
            text: "Device Name Source:"
            displayComponent: RowLayout {
                Label { text: sourceAction.text }
                ComboBox {
                    textRole: "display"
                    model: deviceNameSourceModel
                    onActivated: index => {
                        page.defaultKey = model.valueToProperty(currentIndex)
                        config.deviceNameSource = currentIndex
                        config.save()
                    }
                    Component.onCompleted: currentIndex = config.deviceNameSource
                }
            }
        }
    ]

    DeviceRenameModel {
        id: sinkRenameModel
        sourceModel: SinkModel {}
    }

    DeviceRenameModel {
        id: sourceRenameModel
        sourceModel: SourceModel {}
    }

    headerPaddingEnabled: false // Let the InlineMessages touch the edges
    header: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: sinkRenameModel.error !== ''
            type: Kirigami.MessageType.Error
            position: Kirigami.InlineMessage.Position.Header
            text: sinkRenameModel.error

            actions: [
                Kirigami.Action {
                    text: i18nc("@action", "Report Bug")
                    icon.name: "tools-report-bug-symbolic"
                    onTriggered: Qt.openUrlExternally("https://bugs.kde.org/enter_bug.cgi?product=plasma-pa")
                }
            ]
        }
    }

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
        visible: page.state === "busy"
        text: i18nc("@info:status reloading the backing data", "Reloading")
    }

    component RenameDelegate: ColumnLayout {
        id: delegate

        width: parent.width

        readonly property var pulseProperties: PulseProperties
        readonly property var hasOverride: HasOverride

        ColumnLayout     {
            Layout.fillWidth: true

            RowLayout {
                id: root
                readonly property string key: page.defaultKey
                readonly property string initialText: {
                    const text = delegate.pulseProperties[key]
                    return (text !== undefined) ? text : ''
                }

                Layout.fillWidth: true

                Kirigami.ActionTextField {
                    id: textField
                    Layout.fillWidth: true
                    placeholderText: root.initialText

                    onEditingFinished: {
                        if (text === root.initialText) {
                            return
                        }
                        let properties = pulseProperties
                        properties['node.nick'] = text
                        properties['device.description'] = text
                        properties['alsa.card_name'] = text
                        properties['alsa.long_card_name'] = text
                        PulseProperties = properties
                    }

                    rightActions:  [
                        Kirigami.Action {
                            enabled: delegate.hasOverride
                            text: i18nc("@action reset device name to default", "Reset")
                            icon.name: "edit-reset-symbolic"
                            onTriggered: {
                                const model = delegate.ListView.view.model
                                model.reset(model.index(index, 0))
                                textField.reset()
                            }
                        }
                    ]

                    function reset() {
                        clear()
                        if (delegate.hasOverride) {
                            // the current value is already an override. use it as text rather than placeholder
                            // There is a bit of a problem because we can't rightly tell what the original property
                            // was, so we can't keep placeholder and text 100% correct, but we can at least
                            // indicate if this is an override or not.
                            text = root.initialText
                            // placeholderText = ""
                        } else {
                            text = ""
                            // placeholderText = Qt.binding(function() { return root.initialText })
                        }
                    }

                    Component.onCompleted: reset()
                }
            }
        }
    }

    view: Flickable {
        id: flickable

        contentWidth: column.width
        contentHeight: column.height
        clip: true
        visible: page.state === ""

        ColumnLayout {
            id: column
            width: flickable.width

            Kirigami.ListSectionHeader {
                Layout.fillWidth: true
                visible: sinks.visible
                text: i18nd("kcm_pulseaudio", "Playback Devices")
            }

            ListView {
                id: sinks
                visible: count > 0
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.topMargin: Kirigami.Units.smallSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing
                Layout.preferredHeight: contentHeight
                interactive: false
                spacing: Kirigami.Units.largeSpacing
                model: sinkRenameModel

                delegate: RenameDelegate {}
            }

            Kirigami.ListSectionHeader {
                Layout.fillWidth: true
                visible: sources.visible
                text: i18nd("kcm_pulseaudio", "Recording Devices")
            }

            ListView {
                id: sources
                visible: count > 0
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.topMargin: Kirigami.Units.smallSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing
                Layout.preferredHeight: contentHeight
                interactive: false
                spacing: Kirigami.Units.largeSpacing
                model: sourceRenameModel
                delegate: RenameDelegate {}
            }
        }
    }

    states: [
        State { name: "busy"; when: sinkRenameModel.busy || sourceRenameModel.busy },
        State { name: "" }
    ]
}
