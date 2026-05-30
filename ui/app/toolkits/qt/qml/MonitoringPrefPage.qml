import QtQuick

// MonitoringPrefPage — activity monitoring options.
// Expects context property: monitoringPrefBridge (MonitoringPrefBridge)
// On macOS the only option is opening the debug window.
Item {
    id: root

    property var bridge: typeof monitoringPrefBridge !== "undefined" ? monitoringPrefBridge : null

    implicitWidth:  parent ? parent.width : 500
    implicitHeight: col.implicitHeight

    PrefTokens { id: tok }

    Column {
        id: col
        anchors { left: parent.left; right: parent.right; top: parent.top }
        spacing: 24

        // ── Activity detection — Windows only ─────────────────────────────────
        PrefGroup {
            width: parent.width
            visible: root.bridge ? root.bridge.hasAlternateMonitor : false
            title: qsTr("Activity detection")

            PrefToggleRow {
                width: parent.width
                label: qsTr("Use alternate monitor")
                hint:  qsTr("Enable if Workrave fails to detect when you are using your computer. Requires a restart of Workrave.")
                checked: root.bridge ? root.bridge.alternateMonitor : false
                onToggled: (v) => { if (root.bridge) root.bridge.setAlternateMonitor(v) }
            }

            // Sensitivity — only relevant when alternate monitor is active
            PrefSpinRow {
                width: parent.width
                visible: root.bridge ? root.bridge.alternateMonitor : false
                label:   qsTr("Mouse sensitivity")
                hint:    qsTr("Number of pixels the mouse must move before Workrave counts it as activity.")
                display: root.bridge ? root.bridge.sensitivity.toString() : "3"
                onIncrement: { if (root.bridge) root.bridge.setSensitivity(root.bridge.sensitivity + 1) }
                onDecrement: { if (root.bridge) root.bridge.setSensitivity(Math.max(1, root.bridge.sensitivity - 1)) }
            }
        }

        // ── Plugin extensions (e.g. WindowsFocusAssist) ───────────────────────
        PrefPluginGroups {
            width: parent.width
            bridge: (typeof pageExtensionBridge !== "undefined") ? pageExtensionBridge : null
        }

        // ── Debug ─────────────────────────────────────────────────────────────
        PrefGroup {
            width: parent.width
            title: qsTr("Debug")

            // Spacer row — button sits inside the group card
            Item {
                width: parent.width
                height: debugRow.implicitHeight + 28

                Rectangle {
                    anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
                    height: 1; color: tok.edge2
                }

                Item {
                    id: debugRow
                    anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
                    implicitHeight: debugLabel.implicitHeight

                    Column {
                        id: debugLabel
                        anchors { left: parent.left; right: debugBtn.left; rightMargin: 16;
                                  verticalCenter: parent.verticalCenter }
                        spacing: 3

                        Text {
                            text: qsTr("Activity monitor")
                            font.pixelSize: 14; font.weight: Font.Medium
                            color: tok.ink
                        }
                        Text {
                            width: parent.width
                            text: qsTr("Open the debug window to inspect raw activity events and monitor state.")
                            font.pixelSize: 12; color: tok.mute
                            wrapMode: Text.WordWrap; lineHeight: 1.45
                        }
                    }

                    // Pill button
                    Item {
                        id: debugBtn
                        anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                        width: btnText.implicitWidth + 24; height: 30

                        Rectangle {
                            anchors.fill: parent; radius: 6
                            color: debugMouse.containsMouse ? tok.sageSoft : tok.panel2
                            border.color: tok.edge; border.width: 1
                            Behavior on color { ColorAnimation { duration: 120 } }
                        }

                        Text {
                            id: btnText
                            anchors.centerIn: parent
                            text: qsTr("Open debug window")
                            font.pixelSize: 13; font.weight: Font.Medium
                            color: tok.ink
                        }

                        MouseArea {
                            id: debugMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: { if (root.bridge) root.bridge.openDebugWindow() }
                        }
                    }
                }
            }
        }
    }
}
