// ExercisesDialog.qml — Exercises player.
// Loaded by QmlExercisesDialog via QQuickView.
// Context property: exercisesBridge (ExercisesBridge).

import QtQuick
import QtQuick.Controls.Basic

Item {
    id: root

    signal closeRequested()

    PrefTokens { id: tok }

    readonly property double exProgress: exercisesBridge != null ? exercisesBridge.exerciseProgress : 1.0
    readonly property bool   exPaused:   exercisesBridge != null ? exercisesBridge.isPaused : false

    Rectangle {
        anchors.fill: parent
        color: tok.bg

        // ── Header ────────────────────────────────────────────────────────────
        Rectangle {
            id: header
            anchors { top: parent.top; left: parent.left; right: parent.right }
            height: 48
            color: tok.panel

            Rectangle {
                anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
                height: 1; color: tok.edge
            }

            Text {
                anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: 16 }
                text: qsTr("Exercises")
                font.pixelSize: 16; font.bold: true
                color: tok.ink
            }

            Text {
                anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: 16 }
                text: exercisesBridge != null ? exercisesBridge.exerciseTimeStr : "0:00"
                font.pixelSize: 22; font.bold: true
                color: exPaused ? tok.warn : tok.sage
            }
        }

        // ── Content ───────────────────────────────────────────────────────────
        Item {
            id: contentArea
            anchors { top: header.bottom; left: parent.left; right: parent.right; bottom: bottomBar.top }
            anchors { topMargin: 16; leftMargin: 16; rightMargin: 16; bottomMargin: 16 }

            // ── Image panel (fills available height above the progress bar) ───
            Rectangle {
                id: imgPanel
                anchors { top: parent.top; left: parent.left; bottom: progressBar.top }
                anchors.bottomMargin: 8
                width: 220
                color: tok.panel
                border.color: tok.edge; border.width: 1
                radius: 4
                clip: true

                Image {
                    anchors { fill: parent; margins: 6 }
                    source: exercisesBridge != null ? exercisesBridge.exerciseImage : ""
                    mirror: exercisesBridge != null ? exercisesBridge.exerciseImageMirror : false
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }
            }

            // ── Countdown progress bar ────────────────────────────────────────
            Rectangle {
                id: progressBar
                anchors { bottom: parent.bottom; left: parent.left }
                width: 220; height: 8
                radius: 4
                color: tok.track

                Rectangle {
                    width: parent.width * root.exProgress
                    height: parent.height
                    radius: parent.radius
                    color: exPaused ? tok.warn : tok.sage
                    Behavior on width { NumberAnimation { duration: 500 } }
                }
            }

            // ── Right pane: title + scrollable description ────────────────────
            Item {
                anchors { top: parent.top; left: imgPanel.right; right: parent.right; bottom: parent.bottom }
                anchors.leftMargin: 16

                Text {
                    id: exTitle
                    anchors { top: parent.top; left: parent.left; right: parent.right }
                    text: exercisesBridge != null ? exercisesBridge.exerciseName : ""
                    font.pixelSize: 15; font.bold: true
                    color: tok.ink
                    wrapMode: Text.Wrap
                }

                Rectangle {
                    id: titleSep
                    anchors { top: exTitle.bottom; left: parent.left; right: parent.right; topMargin: 10 }
                    height: 1; color: tok.edge
                }

                Flickable {
                    anchors { top: titleSep.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
                    anchors.topMargin: 10
                    contentHeight: descText.implicitHeight
                    clip: true
                    flickableDirection: Flickable.VerticalFlick

                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                    Text {
                        id: descText
                        width: parent.width
                        text: exercisesBridge != null ? exercisesBridge.exerciseDescription : ""
                        font.pixelSize: 13
                        color: tok.ink2
                        wrapMode: Text.WordWrap
                        lineHeight: 1.45
                    }
                }
            }
        }

        // ── Bottom bar ────────────────────────────────────────────────────────
        Rectangle {
            id: bottomBar
            anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
            height: 52
            color: tok.panel

            Rectangle {
                anchors { top: parent.top; left: parent.left; right: parent.right }
                height: 1; color: tok.edge
            }

            Row {
                anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: 16 }
                spacing: 8

                ActionButton {
                    label: "◀  " + qsTr("Previous")
                    onClicked: if (exercisesBridge != null) exercisesBridge.prevExercise()
                }
                ActionButton {
                    label: exPaused ? ("▶  " + qsTr("Resume")) : ("⏸  " + qsTr("Pause"))
                    onClicked: if (exercisesBridge != null) exercisesBridge.togglePause()
                }
                ActionButton {
                    label: qsTr("Next") + "  ▶"
                    onClicked: if (exercisesBridge != null) exercisesBridge.nextExercise()
                }
            }

            ActionButton {
                anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: 16 }
                label: qsTr("Close")
                onClicked: root.closeRequested()
            }
        }
    }

    // ── ActionButton component ────────────────────────────────────────────────
    component ActionButton: Rectangle {
        property string label: ""
        property bool hovered: false
        signal clicked()

        height: 30
        width: Math.max(lbl.implicitWidth + 24, 100)
        radius: tok.actionRadius
        color: hovered ? tok.sageSoft : tok.actionBg
        border.color: tok.actionEdge; border.width: 1

        Text {
            id: lbl
            anchors.centerIn: parent
            text: parent.label
            font.pixelSize: 12
            color: tok.ink
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
