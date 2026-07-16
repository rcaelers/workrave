// AutoUpdateDialog.qml — Software update notification dialog.
// Loaded by QmlAutoUpdateDialog via QQuickView.
// All data comes from the C++ "updateBridge" context property (AutoUpdateBridge).

import QtQuick
import QtQuick.Controls.Basic

Item {
    id: root

    signal closeRequested()

    PrefTokens { id: tok }

    Rectangle {
        anchors.fill: parent
        color: tok.bg

        // ── Header: logo + version info ───────────────────────────────────────
        Row {
            id: headerRow
            anchors { top: parent.top; left: parent.left; right: parent.right }
            anchors { topMargin: 20; leftMargin: 20; rightMargin: 20 }
            spacing: 16

            Image {
                source: "qrc:/sanctuary/workrave-sheep.svg"
                width: 80; height: 80
                fillMode: Image.PreserveAspectFit
                smooth: true
                anchors.verticalCenter: parent.verticalCenter
            }

            Column {
                width: parent.width - 96
                anchors.verticalCenter: parent.verticalCenter
                spacing: 6

                Text {
                    width: parent.width
                    text: updateBridge != null ? updateBridge.infoTitle : ""
                    font.pixelSize: 16; font.bold: true
                    color: tok.ink
                    wrapMode: Text.WordWrap
                }
                Text {
                    width: parent.width
                    text: updateBridge != null ? updateBridge.infoText : ""
                    font.pixelSize: 12; color: tok.ink2
                    wrapMode: Text.WordWrap
                    lineHeight: 1.4
                }
            }
        }

        // ── Separator ─────────────────────────────────────────────────────────
        Rectangle {
            id: headerSep
            anchors { top: headerRow.bottom; left: parent.left; right: parent.right; topMargin: 12 }
            height: 1; color: tok.edge
        }

        // ── Release notes label ───────────────────────────────────────────────
        Text {
            id: notesLabel
            anchors { top: headerSep.bottom; left: parent.left; leftMargin: 20; topMargin: 8 }
            text: qsTr("Release notes")
            font.pixelSize: 12; font.bold: true
            color: tok.ink2
        }

        // ── Scrollable release notes ──────────────────────────────────────────
        Rectangle {
            id: notesBox
            anchors { top: notesLabel.bottom; left: parent.left; right: parent.right; bottom: statusRow.top }
            anchors { leftMargin: 20; rightMargin: 20; topMargin: 4; bottomMargin: 8 }
            color: tok.panel
            border.color: tok.edge; border.width: 1

            Flickable {
                id: notesFl
                anchors { fill: parent; margins: 10 }
                contentHeight: notesText.implicitHeight
                contentWidth: width
                clip: true
                flickableDirection: Flickable.VerticalFlick

                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                Text {
                    id: notesText
                    width: notesFl.width
                    text: updateBridge != null ? updateBridge.releaseNotes : ""
                    textFormat: Text.MarkdownText
                    font.pixelSize: 12
                    color: tok.ink2
                    wrapMode: Text.WordWrap
                    lineHeight: 1.5
                }
            }
        }

        // ── Status / progress row ─────────────────────────────────────────────
        Item {
            id: statusRow
            anchors { left: parent.left; right: parent.right; bottom: buttonRow.top }
            anchors { leftMargin: 20; rightMargin: 20; bottomMargin: 4 }
            height: (updateBridge != null && (updateBridge.progressVisible || updateBridge.statusText !== "")) ? 28 : 0
            clip: true

            Behavior on height { NumberAnimation { duration: 150 } }

            Text {
                id: statusLabel
                anchors { left: parent.left; verticalCenter: parent.verticalCenter }
                text: updateBridge != null ? updateBridge.statusText : ""
                font.pixelSize: 12; color: tok.ink2
                visible: text !== ""
            }

            // Progress bar track
            Rectangle {
                anchors {
                    left: statusLabel.visible ? statusLabel.right : parent.left
                    right: parent.right; verticalCenter: parent.verticalCenter
                    leftMargin: statusLabel.visible ? 10 : 0
                }
                height: 8; radius: 4
                color: tok.track
                visible: updateBridge != null && updateBridge.progressVisible

                Rectangle {
                    width: parent.width * (updateBridge != null ? updateBridge.progressValue / 100.0 : 0)
                    height: parent.height; radius: parent.radius
                    color: tok.sage
                    Behavior on width { NumberAnimation { duration: 300 } }
                }
            }
        }

        // ── Button row ────────────────────────────────────────────────────────
        Item {
            id: buttonRow
            anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
            anchors { bottomMargin: 12; leftMargin: 20; rightMargin: 20 }
            height: 34

            // Left side: Skip this version
            ActionButton {
                anchors { left: parent.left; verticalCenter: parent.verticalCenter }
                label: qsTr("Skip this version")
                visible: updateBridge != null && updateBridge.showInstallButtons
                onClicked: if (updateBridge != null) updateBridge.skip()
            }

            // Right side: Remind me later / Install update / Close
            Row {
                anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                spacing: 8

                ActionButton {
                    label: qsTr("Remind me later")
                    visible: updateBridge != null && updateBridge.showInstallButtons
                    onClicked: if (updateBridge != null) updateBridge.later()
                }
                ActionButton {
                    label: qsTr("Install update")
                    highlighted: true
                    enabled: updateBridge != null && updateBridge.installEnabled
                    visible: updateBridge != null && !updateBridge.showClose
                    onClicked: if (updateBridge != null) updateBridge.install()
                }
                ActionButton {
                    label: qsTr("Close")
                    visible: updateBridge != null && updateBridge.showClose
                    onClicked: root.closeRequested()
                }
            }
        }
    }

    // ── Button component ──────────────────────────────────────────────────────
    component ActionButton: Rectangle {
        property string label: ""
        property bool highlighted: false
        property bool hovered: false
        property bool enabled: true
        signal clicked()

        height: 30
        width: Math.max(lbl.implicitWidth + 24, 100)
        radius: tok.actionRadius
        color: !enabled ? tok.track
               : hovered ? (highlighted ? tok.sageDeep : tok.sageSoft)
               : (highlighted ? tok.sage : tok.actionBg)
        border.color: highlighted ? tok.sage : tok.actionEdge
        border.width: 1

        Text {
            id: lbl
            anchors.centerIn: parent
            text: parent.label
            font.pixelSize: 12
            color: parent.enabled ? (parent.highlighted ? tok.bg : tok.ink) : tok.mute
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            enabled: parent.enabled
            cursorShape: Qt.PointingHandCursor
            onEntered: parent.hovered = true
            onExited:  parent.hovered = false
            onClicked: parent.clicked()
        }
    }
}
