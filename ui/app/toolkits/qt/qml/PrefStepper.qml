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

    PrefTokens { id: tok }

    Rectangle {
        anchors.fill: parent
        radius: 10
        color: tok.panel
        border.color: tok.edge
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
                    font.pixelSize: tok.btnPx
                    font.weight: Font.Medium
                    color: tok.ink2
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
                    width: 1; color: tok.edge
                }
                Rectangle {
                    anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
                    width: 1; color: tok.edge
                }

                Text {
                    anchors.centerIn: parent
                    text: root.value
                    font.pixelSize: tok.stepperPx
                    font.family: tok.serifFamily
                    font.features: { "tnum": 1 }
                    color: tok.ink
                }
            }

            // ── + button ─────────────────────────────────────────────────────
            Item {
                width: 30; height: parent.height

                Text {
                    anchors.centerIn: parent
                    text: "+"
                    font.pixelSize: tok.btnPx
                    font.weight: Font.Medium
                    color: tok.ink2
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
