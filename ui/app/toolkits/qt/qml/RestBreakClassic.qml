// RestBreakClassic.qml — GTK-faithful classic rest-break window.
// Loaded by RestBreakShell.qml when bridge.classic is true.
// All data comes from the C++ "bridge" context property (RestBreakBridge).

import QtQuick

Item {
    id: root

    // ── Design tokens ────────────────────────────────────────────────────────
    readonly property color colBg:      "#E8E8E8"
    readonly property color colBar:     "#4A90D9"
    readonly property color colTimeBar: "#6ABF69"   // green, matches GTK Adwaita
    readonly property color colBorder:  "#AAAAAA"
    readonly property color colWarn:    "#F08700"
    readonly property color colInk:     "#1A1A1A"
    readonly property color colInk2:    "#444444"
    readonly property color colBtn:     "#D4D0C8"
    readonly property color colBtnTxt:  "#1A1A1A"

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
    readonly property bool   shutdownable: bridge != null ? bridge.shutdownable     : false
    readonly property bool   sleepable:    bridge != null ? bridge.sleepable        : false
    // barProgress = elapsed fraction (0→1); timebar fills left-to-right
    readonly property double barProgress:  bridge != null ? bridge.breakProgress    : 0.0
    readonly property string timeLeft:     bridge != null ? bridge.breakTimeShort   : "5:00"
    // exProgress = remaining fraction (1.0=start → 0.0=end)
    readonly property double exProgress:   bridge != null ? bridge.exerciseProgress : 1.0

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
        width: root.showEx ? Math.min(parent.width - 48, 640)
                           : Math.min(parent.width - 48, 440)
        color: colBg
        radius: 0
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
                height: root.showEx ? exercisePanel.height : infoPanel.height
                anchors.horizontalCenter: parent.horizontalCenter

                // ── Exercise panel ───────────────────────────────────────────
                // Layout: [image 200×200][vert-bar 8×200][title+desc+controls]
                Row {
                    id: exercisePanel
                    visible: root.showEx
                    width: parent.width
                    spacing: 0

                    // Exercise image
                    Rectangle {
                        width: 200; height: 200
                        color: "#D0D0D0"

                        Image {
                            id: exImg
                            anchors.fill: parent
                            source: bridge != null ? bridge.exerciseImage : ""
                            mirror: bridge != null ? bridge.exerciseImageMirror : false
                            fillMode: Image.PreserveAspectFit
                        }
                        Rectangle {
                            anchors.fill: parent
                            color: "#D0D0D0"
                            visible: exImg.status !== Image.Ready
                                     || (bridge != null && bridge.exerciseImage === "")
                        }
                    }

                    // Vertical countdown bar — remaining fraction fills from top, shrinks downward
                    Rectangle {
                        width: 8; height: 200
                        color: "#C0C0C0"

                        Rectangle {
                            width: parent.width
                            height: parent.height * root.exProgress
                            anchors.top: parent.top
                            color: colBar
                            Behavior on height { NumberAnimation { duration: 500 } }
                        }
                    }

                    // Title, description, and exercise player controls
                    Item {
                        id: textCol
                        width: parent.width - 208
                        // Grow tall enough for text + controls; never less than the image height
                        height: Math.max(textBlock.implicitHeight + 8 + exControls.height + 8, 200)

                        Column {
                            id: textBlock
                            anchors { top: parent.top; left: parent.left; right: parent.right }
                            topPadding: 4; leftPadding: 8; rightPadding: 8
                            spacing: 6

                            Text {
                                width: parent.width - 16
                                text: bridge != null ? bridge.exerciseName : ""
                                font.pixelSize: 15; font.bold: true
                                color: colInk; wrapMode: Text.Wrap
                            }

                            Text {
                                width: parent.width - 16
                                text: bridge != null ? bridge.exerciseDescription : ""
                                font.pixelSize: 12; color: colInk2
                                wrapMode: Text.WordWrap; lineHeight: 1.4
                            }
                        }

                        // "Exercises player: |◄ ‖ ►| ×" — anchored to bottom-left
                        Row {
                            id: exControls
                            anchors { bottom: parent.bottom; left: parent.left
                                      leftMargin: 8; bottomMargin: 4 }
                            spacing: 3

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: qsTr("Exercises player:")
                                font.pixelSize: 11; color: colInk2
                            }

                            SmallButton {
                                label: "|◄"
                                onClicked: { if (bridge != null) bridge.prevExercise() }
                            }
                            SmallButton {
                                label: (bridge != null && bridge.isPaused) ? "▶" : "‖"
                                onClicked: { if (bridge != null) bridge.togglePause() }
                            }
                            SmallButton {
                                label: "►|"
                                onClicked: { if (bridge != null) bridge.nextExercise() }
                            }
                            SmallButton {
                                label: "×"
                                onClicked: { if (bridge != null) bridge.endExercises() }
                            }
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
                            font.pixelSize: 15; font.bold: true; color: colInk
                        }

                        Text {
                            width: parent.width
                            text: root.isNatural
                                  ? qsTr("This is your natural rest break.")
                                  : qsTr("This is your rest break. Make sure you stand up and walk away from your computer on a regular basis. Just walk around for a few minutes, stretch, and relax.")
                            font.pixelSize: 13; color: colInk
                            wrapMode: Text.WordWrap; lineHeight: 1.4
                        }
                    }
                }
            }

            Item { width: 1; height: 8 }

            // ── TimeBar (green) ──────────────────────────────────────────────
            Rectangle {
                width: parent.width - 24
                height: 22
                anchors.horizontalCenter: parent.horizontalCenter
                color: "#C0C0C0"
                clip: true

                Rectangle {
                    width: Math.max(0, parent.width * root.barProgress)
                    height: parent.height
                    color: colTimeBar
                    Behavior on width { NumberAnimation { duration: 500 } }
                }

                Text {
                    anchors.centerIn: parent
                    text: bridge != null ? bridge.breakTime : qsTr("Rest break for {}")
                    font.pixelSize: 11; font.bold: true
                    color: "#FFFFFF"
                    style: Text.Outline; styleColor: "#00000066"
                }
            }

            // ── Lock progress bar (bare bar, no label) ───────────────────────
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
                        onClicked: { if (bridge != null) bridge.requestShutdown() }
                    }
                    ClassicButton {
                        visible: root.sleepable
                        label: qsTr("Sleep")
                        onClicked: { if (bridge != null) bridge.requestSleep() }
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

    // ── Small button for exercise player controls ─────────────────────────────
    component SmallButton: Rectangle {
        property string label: ""
        signal clicked()
        property bool hovered: false

        height: 22
        width: Math.max(lbl.implicitWidth + 10, 30)
        radius: 0
        color: hovered ? "#C0BBAF" : colBtn
        border.color: "#888888"; border.width: 1

        Text {
            id: lbl
            anchors.centerIn: parent
            text: parent.label
            font.pixelSize: 11
            color: colBtnTxt
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true
            onEntered: parent.hovered = true
            onExited:  parent.hovered = false
            onClicked: parent.clicked()
        }
    }

    // ── Standard button ──────────────────────────────────────────────────────
    component ClassicButton: Rectangle {
        property string label: ""
        signal clicked()
        property bool hovered: false

        height: 28
        width: Math.max(btnLbl.implicitWidth + 24, 88)
        radius: 0
        color: hovered ? "#C0BBAF" : colBtn
        border.color: "#888888"; border.width: 1

        Text {
            id: btnLbl
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
