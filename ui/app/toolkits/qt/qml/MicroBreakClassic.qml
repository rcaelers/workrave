// MicroBreakClassic.qml — GTK-faithful classic micro-break window.
// Loaded by MicroBreakShell.qml when bridge.classic is true.
// All data comes from the C++ "bridge" context property (MicroBreakBridge).

import QtQuick

Item {
    id: root

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
    readonly property int    blockMode:   bridge != null ? bridge.blockMode   : 1
    readonly property bool   userActive:  bridge != null ? bridge.userActive  : false
    readonly property double barProgress: bridge != null ? 1.0 - bridge.ringProgress : 0.0
    readonly property string timeLeft:    bridge != null ? bridge.timeRemaining : "0:30"
    readonly property string breakName:   bridge != null ? bridge.breakName    : qsTr("Micro-break")
    readonly property bool   canPostpone: bridge != null ? bridge.canPostpone  : true
    readonly property bool   canSkip:     bridge != null ? bridge.canSkip      : true
    readonly property bool   isLocked:    bridge != null ? bridge.isLocked     : false
    readonly property double lockProg:    bridge != null ? bridge.lockProgress  : 0.0
    readonly property bool   lockable:    bridge != null ? bridge.lockable     : false
    readonly property bool   restEnabled: bridge != null ? bridge.restBreakEnabled : false
    readonly property string restInfo:    bridge != null ? bridge.restBreakInfo : ""

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
        width: Math.min(parent.width - 48, 520)
        color: colBg
        radius: 2
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

                    Text {
                        width: parent.width
                        text: qsTr("Please relax for a few seconds")
                        font.pixelSize: 13; font.bold: true
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

            Item { width: 1; height: 10 }

            // ── TimeBar ──────────────────────────────────────────────────────
            Rectangle {
                width: parent.width - 24
                height: 20
                anchors.horizontalCenter: parent.horizontalCenter
                color: "#C0C0C0"
                clip: true

                Rectangle {
                    width: Math.max(0, parent.width * root.barProgress)
                    height: parent.height
                    color: colBar
                    Behavior on width { NumberAnimation { duration: 500 } }
                }

                Text {
                    anchors.centerIn: parent
                    text: root.breakName + " " + root.timeLeft
                    font.pixelSize: 11; font.bold: true
                    color: "#FFFFFF"
                    style: Text.Outline; styleColor: "#00000066"
                }
            }

            Item { width: 1; height: 6 }

            // ── Lock progress bar ────────────────────────────────────────────
            Item {
                width: parent.width - 24
                height: root.isLocked ? lockBar.implicitHeight + 4 : 0
                anchors.horizontalCenter: parent.horizontalCenter
                clip: true
                Behavior on height { NumberAnimation { duration: 200 } }

                Column {
                    id: lockBar
                    width: parent.width
                    spacing: 3

                    Text {
                        width: parent.width
                        text: qsTr("Postpone and skip will unlock after resting")
                        font.pixelSize: 10; color: colInk2
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Rectangle {
                        width: parent.width; height: 4; color: "#C0C0C0"
                        Rectangle {
                            width: Math.max(4, parent.width * root.lockProg)
                            height: parent.height; color: colBar
                            Behavior on width { NumberAnimation { duration: 500 } }
                        }
                    }
                }
            }

            Item { width: 1; height: 8 }

            // ── Button row ───────────────────────────────────────────────────
            Row {
                width: parent.width - 24
                anchors.right: parent.right
                anchors.rightMargin: 12
                spacing: 6
                layoutDirection: Qt.RightToLeft

                ClassicButton {
                    visible: root.canSkip
                    enabled: root.canSkip
                    label: qsTr("Skip")
                    onClicked: { if (bridge != null) bridge.requestSkip() }
                }

                ClassicButton {
                    visible: root.canPostpone
                    enabled: root.canPostpone
                    label: qsTr("Postpone")
                    onClicked: { if (bridge != null) bridge.requestPostpone() }
                }

                ClassicButton {
                    visible: root.lockable
                    label: qsTr("Lock")
                    onClicked: { if (bridge != null) bridge.requestLock() }
                }

                ClassicButton {
                    visible: root.restEnabled
                    label: qsTr("Rest break")
                    onClicked: { if (bridge != null) bridge.requestRestBreak() }
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

        height: 26
        width: btnText.implicitWidth + 20
        radius: 2
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
