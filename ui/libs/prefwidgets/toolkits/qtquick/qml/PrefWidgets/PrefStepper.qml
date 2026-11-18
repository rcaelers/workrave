pragma ComponentBehavior: Bound

import QtQuick

// PrefStepper — the −/value/+ pill used in TimeControl and SpinRow.
// Properties: value (display string), narrow (bool)
// Signals: increment(), decrement()
Item {
    id: root

    property string value:  "0:00"
    property bool   narrow: false

    signal increment()
    signal decrement()

    implicitWidth:  30 + (narrow ? 60 : 96) + 30
    implicitHeight: 34

    readonly property var tok: Theme

    Rectangle {
        anchors.fill: parent
        radius: 10
        color: root.tok.surface
        border.color: root.tok.border
        border.width: 1
        clip: true

        Row {
            anchors.fill: parent

            // ── − button ─────────────────────────────────────────────────────
            Item {
                width: 30; height: parent.height

                Text {
                    anchors.centerIn: parent
                    text: "−"
                    font.pixelSize: root.tok.btnPx
                    font.weight: Font.Medium
                    color: root.tok.textSecondary
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.decrement()
                }
            }

            // ── Value display ─────────────────────────────────────────────────
            Item {
                width: root.narrow ? 60 : 96
                height: parent.height

                Rectangle {
                    anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
                    width: 1; color: root.tok.border
                }
                Rectangle {
                    anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
                    width: 1; color: root.tok.border
                }

                Text {
                    anchors.centerIn: parent
                    text: root.value
                    font.pixelSize: root.tok.stepperPx
                    font.family: root.tok.serifFamily
                    font.features: { "tnum": 1 }
                    color: root.tok.text
                }
            }

            // ── + button ─────────────────────────────────────────────────────
            Item {
                width: 30; height: parent.height

                Text {
                    anchors.centerIn: parent
                    text: "+"
                    font.pixelSize: root.tok.btnPx
                    font.weight: Font.Medium
                    color: root.tok.textSecondary
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.increment()
                }
            }
        }
    }

    Accessible.role: Accessible.SpinBox
    Accessible.name: root.value
}
