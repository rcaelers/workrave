// ExercisesClassic.qml — Gtk-faithful standalone exercises player.
// Loaded by ExercisesShell.qml when exercisesBridge.classic is true.
// Mirrors the Gtk ExercisesPanel: image + vertical countdown bar + text,
// with Previous/Pause/Next/Close buttons bottom-right.

import QtQuick

Item {
    id: root

    signal closeRequested()

    readonly property double exProgress: exercisesBridge != null ? exercisesBridge.exerciseProgress : 1.0
    readonly property bool   exPaused:   exercisesBridge != null ? exercisesBridge.isPaused : false

    readonly property color colBg:     "#EFEFEF"
    readonly property color colBar:    "#4A90D9"
    readonly property color colInk:    "#1A1A1A"
    readonly property color colInk2:   "#444444"

    Rectangle {
        anchors.fill: parent
        color: colBg

        // ── Content: [image][countdown bar][title + description] ─────────────
        Item {
            anchors { top: parent.top; left: parent.left; right: parent.right; bottom: buttonRow.top }
            anchors { topMargin: 16; leftMargin: 16; rightMargin: 16; bottomMargin: 12 }

            // Exercise image in a bordered panel
            Rectangle {
                id: imgPanel
                anchors { top: parent.top; left: parent.left; bottom: parent.bottom }
                width: 260
                color: "#FFFFFF"
                border.color: "#B0B0B0"; border.width: 1

                Image {
                    anchors { fill: parent; margins: 1 }
                    source: exercisesBridge != null ? exercisesBridge.exerciseImage : ""
                    mirror: exercisesBridge != null ? exercisesBridge.exerciseImageMirror : false
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }
            }

            // Vertical countdown bar — remaining fraction fills from top
            Rectangle {
                id: countdownBar
                anchors { top: parent.top; left: imgPanel.right; bottom: parent.bottom; leftMargin: 8 }
                width: 8
                color: "#C0C0C0"

                Rectangle {
                    width: parent.width
                    height: parent.height * root.exProgress
                    anchors.top: parent.top
                    color: colBar
                    Behavior on height { NumberAnimation { duration: 500 } }
                }
            }

            // Title + description
            Column {
                anchors { top: parent.top; left: countdownBar.right; right: parent.right; leftMargin: 16 }
                spacing: 10

                Text {
                    width: parent.width
                    text: exercisesBridge != null ? exercisesBridge.exerciseName : ""
                    font.pixelSize: 17; font.bold: true
                    color: colInk
                    wrapMode: Text.Wrap
                }

                Text {
                    width: parent.width
                    text: exercisesBridge != null ? exercisesBridge.exerciseDescription : ""
                    font.pixelSize: 13
                    color: colInk2
                    wrapMode: Text.WordWrap
                    lineHeight: 1.45
                }
            }
        }

        // ── Bottom-right button row, like the Gtk dialog ─────────────────────
        Row {
            id: buttonRow
            anchors { bottom: parent.bottom; right: parent.right; margins: 16 }
            spacing: 8

            ClassicButton {
                label: "|◄◄  " + qsTr("Previous")
                onClicked: if (exercisesBridge != null) exercisesBridge.prevExercise()
            }
            ClassicButton {
                label: root.exPaused ? ("▶  " + qsTr("Resume")) : ("‖  " + qsTr("Pause"))
                onClicked: if (exercisesBridge != null) exercisesBridge.togglePause()
            }
            ClassicButton {
                label: "►►|  " + qsTr("Next")
                onClicked: if (exercisesBridge != null) exercisesBridge.nextExercise()
            }
            ClassicButton {
                label: "×  " + qsTr("Close")
                onClicked: root.closeRequested()
            }
        }
    }

    component ClassicButton: Rectangle {
        property string label: ""
        property bool hovered: false
        signal clicked()

        height: 30
        width: Math.max(lbl.implicitWidth + 24, 96)
        radius: 0
        color: hovered ? "#E4E4E4" : "#FAFAFA"
        border.color: "#B0B0B0"; border.width: 1

        Text {
            id: lbl
            anchors.centerIn: parent
            text: parent.label
            font.pixelSize: 12
            color: "#1A1A1A"
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onEntered: parent.hovered = true
            onExited:  parent.hovered = false
            onClicked: parent.clicked()
        }
    }
}
