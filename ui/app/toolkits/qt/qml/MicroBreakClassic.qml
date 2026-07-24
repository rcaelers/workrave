// MicroBreakClassic.qml — GTK-faithful classic micro-break window.
// Loaded by MicroBreakShell.qml when bridge.classic is true.
// All data comes from the C++ "bridge" context property (MicroBreakBridge).

import QtQuick

Item {
    id: root

    // Historical Gtk msgids — reuses the existing po translations. The
    // mnemonic underscore ("_Skip") is stripped for display.
    readonly property string txtSkip:     qsTr("_Skip").replace("_", "")
    readonly property string txtPostpone: qsTr("_Postpone").replace("_", "")

    // ── Design tokens ────────────────────────────────────────────────────────
    readonly property color colBg:     "#E8E8E8"
    readonly property color colBar:    "#4A90D9"
    readonly property color colBorder: "#AAAAAA"
    readonly property color colWarn:   "#F08700"
    readonly property color colInk:    "#1A1A1A"
    readonly property color colInk2:   "#444444"
    readonly property color colBtn:    "#D4D0C8"
    readonly property color colBtnTxt: "#1A1A1A"

    // ── Bridge bindings ──────────────────────────────────────────────────────
    readonly property int    blockMode:   bridge != null ? bridge.blockMode      : 1
    readonly property bool   userActive:  bridge != null ? bridge.userActive     : false
    readonly property double barProgress: bridge != null ? 1.0 - bridge.ringProgress : 0.0
    readonly property string timeLeft:    bridge != null ? bridge.timeRemaining  : "0:30"
    readonly property string breakName:   bridge != null ? bridge.breakName      : qsTr("Micro-break")
    readonly property bool   canPostpone: bridge != null ? bridge.canPostpone    : true
    readonly property bool   canSkip:     bridge != null ? bridge.canSkip        : true
    readonly property bool   isLocked:    bridge != null ? bridge.isLocked       : false
    readonly property double lockProg:    bridge != null ? bridge.lockProgress   : 0.0
    readonly property bool   restEnabled: bridge != null ? bridge.restBreakEnabled : false
    readonly property string restInfo:    bridge != null ? bridge.restBreakInfo  : ""

    // ── Flashing border ──────────────────────────────────────────────────────
    property bool flashState: false
    Timer {
        interval: 500; repeat: true
        running: root.userActive
        onTriggered: root.flashState = !root.flashState
        onRunningChanged: if (!running) root.flashState = false
    }
    readonly property color borderCol: root.userActive ? (root.flashState ? colWarn : colBg) : colBorder
    readonly property int   borderW:   root.userActive ? 6 : 1

    // ── Optional dim backdrop (blockMode All) ────────────────────────────────
    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.45)
        visible: root.blockMode === 2
        z: 0
    }

    // ── Card ─────────────────────────────────────────────────────────────────
    Rectangle {
        id: card
        z: 1
        width: Math.min(parent.width - 48, 400)
        color: colBg
        radius: 0
        border.color: root.borderCol
        border.width: root.borderW

        anchors.horizontalCenter: parent.horizontalCenter
        y: root.blockMode === 0
           ? parent.height - height - 12
           : (parent.height - height) / 2

        Behavior on border.color { ColorAnimation { duration: 80 } }

        Column {
            id: cardContent
            anchors { top: parent.top; left: parent.left; right: parent.right }
            padding: 12
            spacing: 0

            // ── Top row: icon + label ────────────────────────────────────────
            Row {
                width: parent.width - 24
                spacing: 12
                anchors.horizontalCenter: parent.horizontalCenter

                Image {
                    source: "qrc:/sanctuary/micro-break.png"
                    width: 64; height: 64
                    fillMode: Image.PreserveAspectFit
                    anchors.verticalCenter: parent.verticalCenter
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 4
                    width: parent.width - 76

                    // Bold title — matches GTK <span weight="bold" size="larger">
                    Text {
                        width: parent.width
                        text: root.breakName
                        font.pixelSize: 15; font.bold: true
                        color: colInk
                    }

                    Text {
                        width: parent.width
                        text: qsTr("Please relax for a few seconds")
                        font.pixelSize: 13
                        color: colInk
                        wrapMode: Text.Wrap
                    }

                    Text {
                        width: parent.width
                        visible: root.restInfo !== ""
                        text: root.restInfo
                        font.pixelSize: 12
                        color: colInk2
                        wrapMode: Text.Wrap
                    }
                }
            }

            Item { width: 1; height: 12 }

            // ── TimeBar — white background, lightgreen fill, black text, like
            // the Gtk TimeBar widget ─────────────────────────────────────────
            Rectangle {
                width: parent.width - 24
                height: 22
                anchors.horizontalCenter: parent.horizontalCenter
                color: "#FFFFFF"
                border.color: "#8F8F8F"; border.width: 1
                clip: true

                Rectangle {
                    x: 1; y: 1
                    width: Math.max(0, (parent.width - 2) * root.barProgress)
                    height: parent.height - 2
                    color: "#90EE90"
                    Behavior on width { NumberAnimation { duration: 500 } }
                }

                Text {
                    anchors.centerIn: parent
                    text: root.breakName + " " + root.timeLeft
                    font.pixelSize: 12
                    color: colInk
                }
            }

            // ── Lock progress bar (bare bar, no label — matches GTK) ─────────
            Item {
                width: parent.width - 24
                height: root.isLocked ? 8 : 0
                anchors.horizontalCenter: parent.horizontalCenter
                clip: true
                Behavior on height { NumberAnimation { duration: 200 } }

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width; height: 4; color: "#C0C0C0"
                    Rectangle {
                        width: Math.max(4, parent.width * root.lockProg)
                        height: parent.height; color: colBar
                        Behavior on width { NumberAnimation { duration: 500 } }
                    }
                }
            }

            Item { width: 1; height: 18 }

            // ── Button row: Rest break (left) | Skip + Postpone (right) ──────
            // Mirrors GTK layout: image button on left, skip/postpone right-aligned.
            Item {
                width: parent.width - 24
                height: 28
                anchors.horizontalCenter: parent.horizontalCenter

                ClassicButton {
                    anchors.left: parent.left
                    visible: root.restEnabled
                    label: qsTr("Rest break")
                    onClicked: { if (bridge != null) bridge.requestRestBreak() }
                }

                Row {
                    anchors.right: parent.right
                    spacing: 6

                    ClassicButton {
                        visible: root.canSkip
                        enabled: root.canSkip
                        label: root.txtSkip
                        onClicked: { if (bridge != null) bridge.requestSkip() }
                    }

                    ClassicButton {
                        visible: root.canPostpone
                        enabled: root.canPostpone
                        label: root.txtPostpone
                        onClicked: { if (bridge != null) bridge.requestPostpone() }
                    }
                }
            }

            Item { width: 1; height: 8 }
        }

        height: cardContent.implicitHeight
    }

    // ── Inline button component ──────────────────────────────────────────────
    component ClassicButton: Rectangle {
        property string label: ""
        signal clicked()
        property bool hovered: false

        height: 28
        width: Math.max(btnText.implicitWidth + 24, 88)
        radius: 0
        color: hovered ? "#C0BBAF" : colBtn
        border.color: "#888888"; border.width: 1

        Text {
            id: btnText
            anchors.centerIn: parent
            text: parent.label
            font.pixelSize: 12
            color: parent.enabled ? colBtnTxt : "#888888"
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true
            onEntered: parent.hovered = true
            onExited:  parent.hovered = false
            onClicked: parent.clicked()
            enabled: parent.enabled
        }
    }
}
