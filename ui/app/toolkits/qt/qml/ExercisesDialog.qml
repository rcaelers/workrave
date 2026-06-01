// ExercisesDialog.qml — Standalone exercises player.
// Loaded by QmlExercisesDialog via QQuickView.
// All data comes from the C++ "exercisesBridge" context property (ExercisesBridge).

import QtQuick

Item {
    id: root

    signal closeRequested()

    // ── Design tokens ────────────────────────────────────────────────────────
    readonly property color colBg:     "#E8E8E8"
    readonly property color colBar:    "#4A90D9"
    readonly property color colInk:    "#1A1A1A"
    readonly property color colInk2:   "#444444"
    readonly property color colBtn:    "#D4D0C8"
    readonly property color colBtnTxt: "#1A1A1A"
    readonly property color colSep:    "#C0C0C0"

    // ── Bridge bindings ──────────────────────────────────────────────────────
    readonly property double exProgress: exercisesBridge != null ? exercisesBridge.exerciseProgress : 1.0

    // ── Background ───────────────────────────────────────────────────────────
    Rectangle {
        anchors.fill: parent
        color: colBg

        // ── Button row (anchored to bottom) ──────────────────────────────────
        Row {
            id: buttonRow
            anchors {
                bottom: parent.bottom
                horizontalCenter: parent.horizontalCenter
                bottomMargin: 12
            }
            spacing: 6

            DialogButton {
                label: "← " + qsTr("Previous")
                onClicked: { if (exercisesBridge != null) exercisesBridge.prevExercise() }
            }
            DialogButton {
                label: (exercisesBridge != null && exercisesBridge.isPaused)
                       ? "▶ " + qsTr("Resume") : "⏸ " + qsTr("Pause")
                minWidth: 110
                onClicked: { if (exercisesBridge != null) exercisesBridge.togglePause() }
            }
            DialogButton {
                label: qsTr("Next") + " →"
                onClicked: { if (exercisesBridge != null) exercisesBridge.nextExercise() }
            }
            DialogButton {
                label: qsTr("Close")
                onClicked: root.closeRequested()
            }
        }

        // ── Content area (image + bar + text) ────────────────────────────────
        Item {
            id: contentArea
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                bottom: buttonRow.top
                topMargin: 12; leftMargin: 12; rightMargin: 12; bottomMargin: 8
            }

            // Exercise image — vertically centered
            Rectangle {
                id: imgBox
                width: 250; height: 250
                color: "#D0D0D0"
                anchors { left: parent.left; verticalCenter: parent.verticalCenter }

                Image {
                    id: exImg
                    anchors.fill: parent
                    source: exercisesBridge != null ? exercisesBridge.exerciseImage : ""
                    mirror: exercisesBridge != null ? exercisesBridge.exerciseImageMirror : false
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }
                Rectangle {
                    anchors.fill: parent
                    color: "#D0D0D0"
                    visible: exImg.status !== Image.Ready
                             || (exercisesBridge != null && exercisesBridge.exerciseImage === "")
                }
            }

            // Vertical exercise countdown bar — remaining fraction, fills from top
            Rectangle {
                id: vertBar
                width: 8; height: 250
                color: colSep
                anchors { left: imgBox.right; verticalCenter: parent.verticalCenter }

                Rectangle {
                    width: parent.width
                    height: parent.height * root.exProgress
                    anchors.top: parent.top
                    color: colBar
                    Behavior on height { NumberAnimation { duration: 500 } }
                }
            }

            // Title + scrollable description
            Item {
                anchors {
                    left: vertBar.right
                    right: parent.right
                    top: parent.top
                    bottom: parent.bottom
                    leftMargin: 10
                }

                Column {
                    anchors { top: parent.top; left: parent.left; right: parent.right }
                    spacing: 8

                    Text {
                        width: parent.width
                        text: exercisesBridge != null ? exercisesBridge.exerciseName : ""
                        font.pixelSize: 15; font.bold: true
                        color: colInk
                        wrapMode: Text.Wrap
                    }

                    // Scrollable description
                    Flickable {
                        width: parent.width
                        height: Math.min(descText.implicitHeight, contentArea.height - 30)
                        contentWidth: parent.width
                        contentHeight: descText.implicitHeight
                        clip: true
                        flickableDirection: Flickable.VerticalFlick

                        Text {
                            id: descText
                            width: parent.width
                            text: exercisesBridge != null ? exercisesBridge.exerciseDescription : ""
                            font.pixelSize: 13
                            color: colInk2
                            wrapMode: Text.WordWrap
                            lineHeight: 1.45
                        }
                    }
                }
            }
        }
    }

    // ── Button component ──────────────────────────────────────────────────────
    component DialogButton: Rectangle {
        property string label: ""
        property int    minWidth: 100
        signal clicked()
        property bool hovered: false

        height: 30
        width: Math.max(lbl.implicitWidth + 24, minWidth)
        radius: 2
        color: hovered ? "#C0BBAF" : colBtn
        border.color: "#888888"; border.width: 1

        Text {
            id: lbl
            anchors.centerIn: parent
            text: parent.label
            font.pixelSize: 12
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
}
