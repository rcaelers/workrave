// CrashDialog.qml — Crash reporter dialog for WorkraveCrashHandler.
// Context properties: crashBridge (CrashBridge), detailsBridge (CrashDetailsBridge).

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Window

Item {
    id: root

    PrefTokens { id: tok }

    // ── Details window ────────────────────────────────────────────────────────
    Window {
        id: detailsWindow
        title: qsTr("Crash details")
        width: 950; height: 620
        minimumWidth: 800; minimumHeight: 500
        modality: Qt.ApplicationModal

        onVisibleChanged: {
            if (visible) {
                x = Screen.desktopAvailableWidth  / 2 - width  / 2
                y = Screen.desktopAvailableHeight / 2 - height / 2
            }
        }

        Rectangle {
            anchors.fill: parent
            color: tok.bg

            // ── Left panel: attachment list ───────────────────────────────────
            Rectangle {
                id: detailsLeft
                anchors { top: parent.top; left: parent.left; bottom: detailsButtons.top }
                anchors { topMargin: 8; leftMargin: 8; bottomMargin: 8; rightMargin: 4 }
                width: 220
                color: tok.panel
                border.color: tok.edge; border.width: 1
                clip: true

                ListView {
                    id: attachList
                    anchors { fill: parent; margins: 4 }
                    model: detailsBridge ? detailsBridge.items : []
                    clip: true

                    delegate: Item {
                        required property var modelData
                        required property int index

                        width: attachList.width
                        height: 28

                        Rectangle {
                            anchors.fill: parent
                            color: detailsBridge && detailsBridge.selectedIndex === index
                                   ? tok.sageSoft : "transparent"
                            radius: 2
                        }

                        Row {
                            anchors { verticalCenter: parent.verticalCenter; left: parent.left; right: parent.right }
                            anchors { leftMargin: 6; rightMargin: 4 }
                            spacing: 4

                            CheckBox {
                                visible: modelData.isAttachment
                                checked: modelData.enabled
                                width: 22; height: 22
                                onToggled: if (detailsBridge) detailsBridge.toggleAttachment(modelData.attachIndex, checked)
                            }

                            Text {
                                text: modelData.name
                                font.pixelSize: 12
                                color: tok.ink
                                elide: Text.ElideMiddle
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width - (modelData.isAttachment ? 26 : 0) - 10
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: if (detailsBridge) detailsBridge.selectItem(index)
                        }
                    }
                }
            }

            // ── Right panel: content viewer ───────────────────────────────────
            Rectangle {
                id: detailsRight
                anchors { top: parent.top; left: detailsLeft.right; right: parent.right; bottom: detailsButtons.top }
                anchors { topMargin: 8; leftMargin: 4; rightMargin: 8; bottomMargin: 8 }
                color: tok.panel
                border.color: tok.edge; border.width: 1
                clip: true

                Flickable {
                    id: contentFl
                    anchors { fill: parent; margins: 8 }
                    contentWidth: Math.max(contentText.implicitWidth, width)
                    contentHeight: Math.max(contentText.implicitHeight, height)
                    clip: true

                    ScrollBar.vertical: ScrollBar   { policy: ScrollBar.AsNeeded }
                    ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }

                    Text {
                        id: contentText
                        width: Math.max(contentFl.width, implicitWidth)
                        text: detailsBridge ? detailsBridge.selectedContent : ""
                        textFormat: Text.RichText
                        font.family: "Courier New"
                        font.pixelSize: 11
                        color: tok.ink
                        wrapMode: Text.NoWrap
                    }
                }
            }

            // ── Details close button ──────────────────────────────────────────
            Item {
                id: detailsButtons
                anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
                anchors { bottomMargin: 10; leftMargin: 8; rightMargin: 8 }
                height: 34

                ActionButton {
                    anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                    label: qsTr("Close")
                    onClicked: detailsWindow.visible = false
                }
            }
        }
    }

    // ── Main dialog ───────────────────────────────────────────────────────────
    Rectangle {
        anchors.fill: parent
        color: tok.bg

        // ── Header: crashed sheep + title + description ───────────────────────
        Rectangle {
            id: headerRect
            anchors { top: parent.top; left: parent.left; right: parent.right }
            color: tok.dangerSoft
            height: headerRow.implicitHeight + 24

            Row {
                id: headerRow
                anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
                anchors { leftMargin: 16; rightMargin: 16 }
                spacing: 16

                Column {
                    width: parent.width - sheepImg.width - parent.spacing
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 6

                    Text {
                        text: qsTr("Workrave has crashed.")
                        font.pixelSize: 16; font.bold: true
                        color: tok.danger
                    }
                    Text {
                        width: parent.width
                        text: qsTr("Workrave encountered a problem and crashed. Please help us diagnose and fix this problem by sending a crash report.")
                        font.pixelSize: 12; color: tok.ink
                        wrapMode: Text.WordWrap; lineHeight: 1.4
                    }
                }

                Image {
                    id: sheepImg
                    source: "qrc:/crash/workrave-sheep-crashed.svg"
                    width: 120; height: 88
                    fillMode: Image.PreserveAspectFit; smooth: true
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        Rectangle {
            id: headerSep
            anchors { top: headerRect.bottom; left: parent.left; right: parent.right }
            height: 1; color: tok.edge
        }

        // ── Submit checkbox ───────────────────────────────────────────────────
        CheckBox {
            id: submitCheck
            anchors { top: headerSep.bottom; left: parent.left; topMargin: 12; leftMargin: 16 }
            text: qsTr("Submit crash report to the Workrave developers")
            checked: true
            font.pixelSize: 12
            onCheckedChanged: if (crashBridge) crashBridge.setSubmitEnabled(checked)
        }

        // ── Indented section (greyed when submit unchecked) ───────────────────
        Item {
            id: indented
            anchors { top: submitCheck.bottom; left: parent.left; right: parent.right; bottom: mainButtons.top }
            anchors { topMargin: 6; leftMargin: 36; rightMargin: 16; bottomMargin: 8 }
            enabled: submitCheck.checked
            opacity: enabled ? 1.0 : 0.45

            Row {
                id: detailsRow
                anchors { top: parent.top; left: parent.left; topMargin: 4 }

                ActionButton {
                    label: qsTr("Details...")
                    onClicked: {
                        detailsWindow.x = Screen.desktopAvailableWidth  / 2 - detailsWindow.width  / 2
                        detailsWindow.y = Screen.desktopAvailableHeight / 2 - detailsWindow.height / 2
                        detailsWindow.show()
                    }
                }
            }

            Text {
                id: hintText
                anchors { top: detailsRow.bottom; left: parent.left; topMargin: 4 }
                text: qsTr("Shows the crash information and files that will be submitted.")
                font.pixelSize: 11; font.italic: true; color: tok.mute
            }

            Rectangle {
                id: contentSep
                anchors { top: hintText.bottom; left: parent.left; right: parent.right; topMargin: 8 }
                height: 1; color: tok.edge
            }

            Text {
                id: commentsLabel
                anchors { top: contentSep.bottom; left: parent.left; topMargin: 8 }
                text: qsTr("Additional comments (optional):")
                font.pixelSize: 12; color: tok.ink
            }

            Rectangle {
                anchors { top: commentsLabel.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
                anchors { topMargin: 4 }
                color: tok.panel
                border.color: tok.edge; border.width: 1
                radius: 2

                TextArea {
                    id: userTextArea
                    anchors { fill: parent; margins: 4 }
                    font.pixelSize: 12
                    color: tok.ink
                    background: null
                    wrapMode: TextEdit.WordWrap
                }
            }
        }

        // ── Close button ──────────────────────────────────────────────────────
        Item {
            id: mainButtons
            anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
            anchors { bottomMargin: 12; leftMargin: 16; rightMargin: 16 }
            height: 34

            ActionButton {
                anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                label: qsTr("Close")
                onClicked: {
                    if (crashBridge) {
                        crashBridge.setUserText(userTextArea.text)
                        crashBridge.emitClose()
                    }
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
