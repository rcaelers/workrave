// DailyLimitClassic.qml — GTK-faithful classic daily-limit window.
// Loaded by DailyLimitShell.qml when bridge.classic is true.
// All data comes from the C++ "bridge" context property (DailyLimitBridge).

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
    readonly property int    blockMode:   bridge != null ? bridge.blockMode    : 1
    readonly property bool   userActive:  bridge != null ? bridge.userActive   : false
    readonly property bool   canPostpone: bridge != null ? bridge.canPostpone  : true
    readonly property bool   canSkip:     bridge != null ? bridge.canSkip      : true
    readonly property bool   isLocked:    bridge != null ? bridge.isLocked     : false
    readonly property double lockProg:    bridge != null ? bridge.lockProgress : 0.0
    readonly property bool   lockable:    bridge != null ? bridge.lockable     : false
    readonly property bool   shutdownable: bridge != null ? bridge.shutdownable : false
    readonly property bool   sleepable:   bridge != null ? bridge.sleepable    : false

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

    // ── Dim backdrop (blockMode All) ─────────────────────────────────────────
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
        width: Math.min(parent.width - 48, 440)
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

            // ── Top row: icon + text ─────────────────────────────────────────
            Row {
                width: parent.width - 24
                spacing: 12
                anchors.horizontalCenter: parent.horizontalCenter

                Image {
                    source: "qrc:/sanctuary/daily-limit.png"
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
                        text: qsTr("Daily limit")
                        font.pixelSize: 15; font.bold: true
                        color: colInk
                    }

                    Text {
                        width: parent.width
                        text: qsTr("You have reached your daily limit. Please stop working behind the computer. If your working day is not over yet, find something else to do, such as reviewing a document.")
                        font.pixelSize: 13
                        color: colInk
                        wrapMode: Text.WordWrap
                        lineHeight: 1.4
                    }
                }
            }

            // ── Lock progress bar (bare bar, no label — matches GTK) ─────────
            Item {
                width: parent.width - 24
                height: root.isLocked ? 16 : 0
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

            Item { width: 1; height: 8 }

            // ── Button row ───────────────────────────────────────────────────
            Item {
                width: parent.width - 24
                height: 28 + 8
                anchors.horizontalCenter: parent.horizontalCenter

                // Left-aligned: Lock / Shut down / Sleep
                Row {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 6

                    ClassicButton {
                        visible: root.lockable
                        label: qsTr("Lock")
                        onClicked: { if (bridge != null) bridge.requestLock() }
                    }
                    ClassicButton {
                        visible: root.shutdownable
                        label: qsTr("Shut down")
                        onClicked: confirmDlg.ask("shutdown", qsTr("Shut down"), qsTr("Are you sure you want to shut down the computer?"))
                    }
                    ClassicButton {
                        visible: root.sleepable
                        label: qsTr("Sleep")
                        onClicked: confirmDlg.ask("sleep", qsTr("Sleep"), qsTr("Are you sure you want to put the computer to sleep?"))
                    }
                }

                // Right-aligned: Postpone / Skip
                Row {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 6

                    ClassicButton {
                        visible: root.canPostpone
                        enabled: root.canPostpone
                        label: qsTr("Postpone")
                        onClicked: { if (bridge != null) bridge.requestPostpone() }
                    }
                    ClassicButton {
                        visible: root.canSkip
                        enabled: root.canSkip
                        label: qsTr("Skip")
                        onClicked: { if (bridge != null) bridge.requestSkip() }
                    }
                }
            }

            Item { width: 1; height: 8 }
        }

        height: cardContent.implicitHeight
    }

    ConfirmDialog {
        id: confirmDlg
        anchors.fill: parent
        z: 200
        onConfirmed: (action) => {
            if (action === "shutdown") { if (bridge != null) bridge.requestShutdown() }
            else if (action === "sleep")    { if (bridge != null) bridge.requestSleep() }
        }
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
