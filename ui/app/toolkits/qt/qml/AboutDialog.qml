// AboutDialog.qml — About window for Workrave.
// Loaded by QmlAboutDialog via QQuickView.
// Context properties: aboutVersion, aboutCopyright, aboutAuthors, aboutTranslators.

import QtQuick

Item {
    id: root

    signal closeRequested()

    PrefTokens { id: tok }

    Rectangle {
        anchors.fill: parent
        color: tok.bg

        // ── Header: sheep logo + app info ────────────────────────────────────
        Row {
            id: headerRow
            anchors { top: parent.top; left: parent.left; right: parent.right }
            anchors { topMargin: 20; leftMargin: 20; rightMargin: 20 }
            spacing: 18

            Image {
                id: sheepImg
                source: "qrc:/sanctuary/workrave-sheep.svg"
                width: 110; height: 110
                fillMode: Image.PreserveAspectFit
                smooth: true
                anchors.verticalCenter: parent.verticalCenter
            }

            Column {
                width: parent.width - sheepImg.width - parent.spacing
                anchors.verticalCenter: parent.verticalCenter
                spacing: 5

                Text {
                    text: "Workrave " + aboutVersion
                    font.pixelSize: 18; font.bold: true
                    color: tok.ink
                }
                Text {
                    width: parent.width
                    text: qsTr("This program assists in the prevention and recovery of Repetitive Strain Injury (RSI).")
                    font.pixelSize: 12; color: tok.ink2
                    wrapMode: Text.WordWrap
                    lineHeight: 1.4
                }
                Text {
                    text: aboutCopyright
                    font.pixelSize: 11; color: tok.mute
                    topPadding: 4
                }
                Text {
                    text: "www.workrave.org"
                    font.pixelSize: 11; color: tok.sage
                    font.underline: siteArea.containsMouse

                    MouseArea {
                        id: siteArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("https://www.workrave.org/")
                    }
                }
            }
        }

        // ── Separator ─────────────────────────────────────────────────────────
        Rectangle {
            id: headerSep
            anchors { top: headerRow.bottom; left: parent.left; right: parent.right; topMargin: 12 }
            height: 1; color: tok.edge
        }

        // ── Tab bar ───────────────────────────────────────────────────────────
        Row {
            id: tabBar
            anchors { top: headerSep.bottom; left: parent.left; leftMargin: 20 }
            spacing: 0
            property int selected: 0

            Repeater {
                model: [ qsTr("Authors"), qsTr("Translators") ]
                delegate: Rectangle {
                    required property int index
                    required property string modelData
                    width: 120; height: 32
                    color: tabBar.selected === index ? tok.panel : "transparent"
                    border.color: tok.edge; border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        font.pixelSize: 12
                        color: tabBar.selected === index ? tok.ink : tok.ink2
                        font.bold: tabBar.selected === index
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: tabBar.selected = index
                    }
                }
            }
        }

        // ── Credits content ───────────────────────────────────────────────────
        Rectangle {
            id: contentBox
            anchors { top: tabBar.bottom; left: parent.left; right: parent.right; bottom: buttonRow.top }
            anchors { leftMargin: 20; rightMargin: 20; bottomMargin: 10 }
            color: tok.panel
            border.color: tok.edge; border.width: 1

            Flickable {
                anchors { fill: parent; margins: 10 }
                contentHeight: creditsText.implicitHeight
                contentWidth: width
                clip: true
                flickableDirection: Flickable.VerticalFlick

                Text {
                    id: creditsText
                    width: parent.width
                    text: tabBar.selected === 0 ? aboutAuthors : aboutTranslators
                    font.pixelSize: 12; font.family: tok.monoFamily
                    color: tok.ink2
                    wrapMode: Text.WordWrap
                    lineHeight: 1.6
                }
            }
        }

        // ── Close button ──────────────────────────────────────────────────────
        Row {
            id: buttonRow
            anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter; bottomMargin: 12 }

            Rectangle {
                height: 30; width: 100; radius: tok.actionRadius
                property bool hovered: false
                color: hovered ? tok.sageSoft : tok.actionBg
                border.color: tok.actionEdge; border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: qsTr("Close")
                    font.pixelSize: 13
                    color: tok.ink
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onEntered: parent.hovered = true
                    onExited:  parent.hovered = false
                    onClicked: root.closeRequested()
                }
            }
        }
    }
}
