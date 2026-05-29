// RestBreakClassic.qml — GTK-faithful classic rest-break window.
// Loaded by RestBreakShell.qml when bridge.classic is true.
// All data comes from the C++ "bridge" context property (RestBreakBridge).
//
// Two body modes (driven by bridge.hasExercises / bridge.exercisesDone):
//   Exercise panel — image, title, description, timer, navigation
//   Info panel     — rest-break.png + instructional text

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
    readonly property int    blockMode:    bridge != null ? bridge.blockMode        : 1
    readonly property bool   userActive:   bridge != null ? bridge.userActive       : false
    readonly property bool   hasExercises: bridge != null ? bridge.hasExercises     : false
    readonly property bool   exDone:       bridge != null ? bridge.exercisesDone    : false
    readonly property bool   showEx:       root.hasExercises && !root.exDone
    readonly property bool   isNatural:    bridge != null ? bridge.isNatural        : false
    readonly property bool   canPostpone:  bridge != null ? bridge.canPostpone      : true
    readonly property bool   canSkip:      bridge != null ? bridge.canSkip          : true
    readonly property bool   isLocked:     bridge != null ? bridge.isLocked         : false
    readonly property double lockProg:     bridge != null ? bridge.lockProgress     : 0.0
    readonly property bool   lockable:     bridge != null ? bridge.lockable         : false
    readonly property double barProgress:  bridge != null ? 1.0 - bridge.breakProgress : 0.0
    readonly property string timeLeft:     bridge != null ? bridge.breakTimeShort   : "5:00"
    readonly property string breakMax:     bridge != null ? bridge.breakMaxStr      : "10:00"

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
        width: root.showEx ? Math.min(parent.width - 48, 760)
                           : Math.min(parent.width - 48, 560)
        color: colBg
        radius: 2
        border.color: root.borderCol
        border.width: root.borderW

        anchors.horizontalCenter: parent.horizontalCenter
        y: root.blockMode === 2 ? (parent.height - height) / 2
                                 : Math.max(24, (parent.height - height) / 2)

        Behavior on border.color { ColorAnimation { duration: 80 } }

        Column {
            id: cardContent
            anchors { top: parent.top; left: parent.left; right: parent.right }
            padding: 12
            spacing: 0

            // ── Body: exercise panel OR info panel ───────────────────────────
            Item {
                width: parent.width - 24
                height: root.showEx ? exercisePanel.implicitHeight : infoPanel.implicitHeight
                anchors.horizontalCenter: parent.horizontalCenter

                // ── Exercise panel ───────────────────────────────────────────
                Column {
                    id: exercisePanel
                    visible: root.showEx
                    width: parent.width
                    spacing: 10

                    // Progress dots
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 6
                        Repeater {
                            model: bridge != null ? bridge.exerciseCount : 0
                            Rectangle {
                                width: 8; height: 8; radius: 999
                                color: index <= (bridge != null ? bridge.exerciseIndex : 0)
                                       ? colBar : "#BBBBBB"
                            }
                        }
                    }

                    // Image + title/description
                    Row {
                        width: parent.width
                        spacing: 16

                        // Exercise image
                        Rectangle {
                            width: 200; height: 200
                            color: "#D0D0D0"
                            anchors.verticalCenter: parent.verticalCenter

                            Image {
                                id: exImg
                                anchors.fill: parent
                                source: bridge != null ? bridge.exerciseImage : ""
                                mirror: bridge != null ? bridge.exerciseImageMirror : false
                                fillMode: Image.PreserveAspectFit
                            }
                            Rectangle {
                                anchors.fill: parent; color: "#D0D0D0"
                                visible: exImg.status !== Image.Ready || (bridge != null && bridge.exerciseImage === "")
                            }
                        }

                        Column {
                            width: parent.width - 216
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 8

                            Text {
                                width: parent.width
                                text: bridge != null ? bridge.exerciseName : ""
                                font.pixelSize: 18; font.bold: true
                                color: colInk; wrapMode: Text.Wrap
                            }

                            Text {
                                width: parent.width
                                text: bridge != null ? bridge.exerciseDescription : ""
                                font.pixelSize: 12; color: colInk2
                                wrapMode: Text.WordWrap; lineHeight: 1.5
                            }
                        }
                    }

                    // Navigation row: ◄  timer  ►  [Pause]
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 16

                        ClassicButton {
                            label: "◄"
                            onClicked: { if (bridge != null) bridge.prevExercise() }
                        }

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: bridge != null ? bridge.exerciseTimeStr : "0:45"
                            font.pixelSize: 22; font.bold: true; color: colInk
                        }

                        ClassicButton {
                            label: "►"
                            onClicked: { if (bridge != null) bridge.nextExercise() }
                        }

                        ClassicButton {
                            label: (bridge != null && bridge.isPaused) ? qsTr("Resume") : qsTr("Pause")
                            onClicked: { if (bridge != null) bridge.togglePause() }
                        }
                    }
                }

                // ── Info panel ───────────────────────────────────────────────
                Row {
                    id: infoPanel
                    visible: !root.showEx
                    width: parent.width
                    spacing: 12

                    Image {
                        source: "qrc:/sanctuary/rest-break.png"
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
                            text: root.isNatural ? qsTr("Natural rest break") : qsTr("Rest break")
                            font.pixelSize: 14; font.bold: true; color: colInk
                        }

                        Text {
                            width: parent.width
                            text: root.isNatural
                                  ? qsTr("This is your natural rest break.")
                                  : qsTr("This is your rest break. Make sure you stand up and walk away from your computer on a regular basis. Just walk around for a few minutes, stretch, and relax.")
                            font.pixelSize: 12; color: colInk2
                            wrapMode: Text.WordWrap; lineHeight: 1.4
                        }
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
                    text: qsTr("Rest break for") + " " + root.timeLeft
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
