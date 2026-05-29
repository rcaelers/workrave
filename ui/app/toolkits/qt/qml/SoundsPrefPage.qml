import QtQuick
import QtQuick.Controls.Basic

// SoundsPrefPage — audio settings.
// Expects context property: soundsPrefBridge (SoundsPrefBridge)
// events model entries: { id, name, enabled, filename }
// themes model entries: { id, name }
Item {
    id: root

    property var bridge: typeof soundsPrefBridge !== "undefined" ? soundsPrefBridge : null

    implicitWidth:  parent ? parent.width : 500
    implicitHeight: col.implicitHeight

    PrefTokens { id: tok }

    Column {
        id: col
        anchors { left: parent.left; right: parent.right; top: parent.top }
        spacing: 24

        // ── Options ───────────────────────────────────────────────────────────
        PrefGroup {
            width: parent.width
            title: qsTr("Options")

            PrefToggleRow {
                width: parent.width
                label:   qsTr("Enable sounds")
                hint:    qsTr("Play a sound when a break starts or ends.")
                checked: root.bridge ? root.bridge.enabled : true
                onToggled: (v) => { if (root.bridge) root.bridge.setEnabled(v) }
            }

            // Volume row — only shown when the audio backend supports it
            Item {
                visible: root.bridge ? root.bridge.hasVolume : false
                width: parent.width
                height: visible ? volumeContent.implicitHeight + 28 : 0
                clip: true

                Rectangle {
                    anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
                    height: 1; color: tok.edge2
                }

                Item {
                    id: volumeContent
                    anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
                    implicitHeight: Math.max(volLabel.implicitHeight, volSlider.implicitHeight)

                    Text {
                        id: volLabel
                        anchors { left: parent.left; verticalCenter: parent.verticalCenter }
                        text: qsTr("Volume")
                        font.pixelSize: 14; font.weight: Font.Medium
                        color: root.bridge && root.bridge.enabled ? tok.ink : tok.mute
                    }

                    Text {
                        id: volPct
                        anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                        text: (root.bridge ? root.bridge.volume : 100) + "%"
                        font.pixelSize: 13
                        color: tok.mute
                        width: 36
                        horizontalAlignment: Text.AlignRight
                    }

                    Slider {
                        id: volSlider
                        anchors { left: volLabel.right; right: volPct.left;
                                  leftMargin: 16; rightMargin: 8;
                                  verticalCenter: parent.verticalCenter }
                        from: 0; to: 100; stepSize: 1
                        value: root.bridge ? root.bridge.volume : 100
                        enabled: root.bridge ? root.bridge.enabled : false
                        onMoved: { if (root.bridge) root.bridge.setVolume(Math.round(value)) }

                        background: Rectangle {
                            x: volSlider.leftPadding
                            y: volSlider.topPadding + volSlider.availableHeight / 2 - height / 2
                            width: volSlider.availableWidth; height: 4; radius: 2
                            color: tok.track
                            Rectangle {
                                width: volSlider.visualPosition * parent.width
                                height: parent.height; radius: 2
                                color: volSlider.enabled ? tok.sage : tok.mute
                            }
                        }
                        handle: Rectangle {
                            x: volSlider.leftPadding + volSlider.visualPosition * volSlider.availableWidth - width / 2
                            y: volSlider.topPadding + volSlider.availableHeight / 2 - height / 2
                            width: 16; height: 16; radius: 8
                            color: "#FFFFFF"
                            border.color: volSlider.enabled ? tok.sage : tok.track; border.width: 2
                        }
                    }
                }
            }

            // Mute row — only shown when the audio backend supports it
            Item {
                visible: root.bridge ? root.bridge.hasMute : false
                width: parent.width
                height: visible ? muteContent.implicitHeight + 28 : 0
                clip: true

                Rectangle {
                    anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
                    height: 1; color: tok.edge2
                }

                Item {
                    id: muteContent
                    anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
                    implicitHeight: muteLabel.implicitHeight

                    Text {
                        id: muteLabel
                        anchors { left: parent.left; verticalCenter: parent.verticalCenter }
                        text: qsTr("Mute")
                        font.pixelSize: 14; font.weight: Font.Medium
                        color: root.bridge && root.bridge.enabled ? tok.ink : tok.mute
                    }

                    Item {
                        anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                        width: 36; height: 21

                        Rectangle {
                            anchors.fill: parent; radius: 999
                            color: (root.bridge && root.bridge.mute) ? tok.sage : tok.track
                            Behavior on color { ColorAnimation { duration: 150 } }
                        }
                        Rectangle {
                            width: 17; height: 17; radius: 999; color: "#FFFFFF"
                            anchors.verticalCenter: parent.verticalCenter
                            x: (root.bridge && root.bridge.mute) ? 17 : 2
                            Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
                        }
                        MouseArea {
                            anchors.fill: parent
                            enabled: root.bridge ? root.bridge.enabled : false
                            cursorShape: Qt.PointingHandCursor
                            onClicked: { if (root.bridge) root.bridge.setMute(!root.bridge.mute) }
                        }
                    }
                }
            }

            // Theme row
            Item {
                width: parent.width
                height: themeContent.implicitHeight + 28

                Rectangle {
                    anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
                    height: 1; color: tok.edge2
                }

                Item {
                    id: themeContent
                    anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
                    implicitHeight: themeLabel.implicitHeight

                    Text {
                        id: themeLabel
                        anchors { left: parent.left; verticalCenter: parent.verticalCenter }
                        text: qsTr("Sound theme")
                        font.pixelSize: 14; font.weight: Font.Medium
                        color: root.bridge && root.bridge.enabled ? tok.ink : tok.mute
                    }

                    // Theme selector
                    ComboBox {
                        id: themeCombo
                        anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                        width: 200
                        enabled: root.bridge ? root.bridge.enabled : false
                        model: root.bridge ? root.bridge.themes : []
                        textRole: "name"
                        valueRole: "id"
                        currentIndex: {
                            if (!root.bridge) return 0;
                            var tid = root.bridge.currentThemeId;
                            var themes = root.bridge.themes;
                            for (var i = 0; i < themes.length; i++) {
                                if (themes[i].id === tid) return i;
                            }
                            return 0;
                        }
                        onActivated: {
                            if (root.bridge) root.bridge.setTheme(currentValue)
                        }

                        contentItem: Text {
                            leftPadding: 8
                            text: themeCombo.displayText
                            font.pixelSize: 13
                            color: themeCombo.enabled ? tok.ink : tok.mute
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            radius: 6
                            color: tok.panel2
                            border.color: tok.edge; border.width: 1
                        }
                    }
                }
            }
        }

        // ── Sound events ──────────────────────────────────────────────────────
        PrefGroup {
            width: parent.width
            title: qsTr("Sound events")

            Repeater {
                model: root.bridge ? root.bridge.events : []

                delegate: Item {
                    id: eventItem
                    required property var modelData
                    required property int index

                    width: parent.width
                    height: eventContent.implicitHeight + 28

                    opacity: root.bridge && root.bridge.enabled ? 1.0 : 0.45

                    Rectangle {
                        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
                        height: 1; color: tok.edge2
                    }

                    Item {
                        id: eventContent
                        anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
                        implicitHeight: Math.max(eventName.implicitHeight, eventActions.implicitHeight)

                        // Toggle dot
                        Item {
                            id: eventToggle
                            anchors { left: parent.left; verticalCenter: parent.verticalCenter }
                            width: 36; height: 21

                            Rectangle {
                                anchors.fill: parent; radius: 999
                                color: eventItem.modelData.enabled ? tok.sage : tok.track
                                Behavior on color { ColorAnimation { duration: 150 } }
                            }
                            Rectangle {
                                width: 17; height: 17; radius: 999; color: "#FFFFFF"
                                anchors.verticalCenter: parent.verticalCenter
                                x: eventItem.modelData.enabled ? 17 : 2
                                Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
                            }
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (root.bridge)
                                        root.bridge.setEventEnabled(eventItem.modelData.id,
                                                                    !eventItem.modelData.enabled)
                                }
                            }
                        }

                        // Event name
                        Text {
                            id: eventName
                            anchors { left: eventToggle.right; leftMargin: 10;
                                      right: eventActions.left; rightMargin: 8;
                                      verticalCenter: parent.verticalCenter }
                            text: eventItem.modelData.name
                            font.pixelSize: 13; font.weight: Font.Medium
                            color: tok.ink
                            elide: Text.ElideRight
                        }

                        // Action buttons: Play | Choose | Clear
                        Row {
                            id: eventActions
                            anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                            spacing: 6

                            // Play button
                            Item {
                                width: playText.implicitWidth + 20; height: 28

                                Rectangle {
                                    anchors.fill: parent; radius: 6
                                    color: playMouse.containsMouse ? tok.sageSoft : tok.panel2
                                    border.color: tok.edge; border.width: 1
                                    Behavior on color { ColorAnimation { duration: 120 } }
                                }
                                Text {
                                    id: playText
                                    anchors.centerIn: parent
                                    text: qsTr("Play")
                                    font.pixelSize: 12
                                    color: tok.ink
                                }
                                MouseArea {
                                    id: playMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: { if (root.bridge) root.bridge.playEvent(eventItem.modelData.id) }
                                }
                            }

                            // Choose file button
                            Item {
                                width: chooseText.implicitWidth + 20; height: 28

                                Rectangle {
                                    anchors.fill: parent; radius: 6
                                    color: chooseMouse.containsMouse ? tok.sageSoft : tok.panel2
                                    border.color: tok.edge; border.width: 1
                                    Behavior on color { ColorAnimation { duration: 120 } }
                                }
                                Text {
                                    id: chooseText
                                    anchors.centerIn: parent
                                    text: qsTr("Choose…")
                                    font.pixelSize: 12
                                    color: tok.ink
                                }
                                MouseArea {
                                    id: chooseMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: { if (root.bridge) root.bridge.pickEventFile(eventItem.modelData.id) }
                                }
                            }

                            // Clear button — only shown when a custom file is set
                            Item {
                                visible: eventItem.modelData.filename !== ""
                                width: visible ? clearText.implicitWidth + 20 : 0; height: 28

                                Rectangle {
                                    anchors.fill: parent; radius: 6
                                    color: clearMouse.containsMouse ? "#ffe5e5" : tok.panel2
                                    border.color: tok.edge; border.width: 1
                                    Behavior on color { ColorAnimation { duration: 120 } }
                                }
                                Text {
                                    id: clearText
                                    anchors.centerIn: parent
                                    text: qsTr("Clear")
                                    font.pixelSize: 12
                                    color: "#c0392b"
                                }
                                MouseArea {
                                    id: clearMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: { if (root.bridge) root.bridge.clearEventFile(eventItem.modelData.id) }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
