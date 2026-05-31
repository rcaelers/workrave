import QtQuick

// PrefTimeStepper — editable time stepper for use in PrefTimeControl.
// Displays time as "m:ss" or "h:mm:ss". Click the value to type directly.
//
// Parse rules (mirrors formatTime):
//   "ss"       → seconds
//   "mm:ss"    → m*60 + s  (s must be 0–59)
//   "h:mm:ss"  → h*3600 + m*60 + s  (m, s must be 0–59)
//
// Signals:
//   increment()          — + button pressed
//   decrement()          — − button pressed
//   committed(int secs)  — user typed a valid time and confirmed it
Item {
    id: root

    property string value: "0:00"

    signal increment()
    signal decrement()
    signal committed(int seconds)

    implicitWidth:  170   // 30 + 110 + 30
    implicitHeight: 34

    PrefTokens { id: tok }

    function parseTime(text) {
        var parts = text.trim().split(":")
        if (parts.length === 1) {
            var s = parseInt(parts[0], 10)
            return (isNaN(s) || s < 0) ? -1 : s
        }
        if (parts.length === 2) {
            var m = parseInt(parts[0], 10)
            var s = parseInt(parts[1], 10)
            if (isNaN(m) || isNaN(s) || m < 0 || s < 0 || s > 59) return -1
            return m * 60 + s
        }
        if (parts.length === 3) {
            var h = parseInt(parts[0], 10)
            var m = parseInt(parts[1], 10)
            var s = parseInt(parts[2], 10)
            if (isNaN(h) || isNaN(m) || isNaN(s) || h < 0 || m < 0 || m > 59 || s < 0 || s > 59) return -1
            return h * 3600 + m * 60 + s
        }
        return -1
    }

    Rectangle {
        anchors.fill: parent
        radius: 10
        color: tok.panel
        border.color: valueInput.activeFocus ? tok.sage : tok.edge
        border.width:  valueInput.activeFocus ? 2 : 1
        clip: true

        Row {
            anchors.fill: parent

            // ── − button ──────────────────────────────────────────────────────
            Item {
                width: 30; height: parent.height

                Text {
                    anchors.centerIn: parent
                    text: "−"
                    font.pixelSize: tok.btnPx; font.weight: Font.Medium
                    color: tok.ink2
                }

                MouseArea {
                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (!valueInput.readOnly) valueInput.cancelEdit()
                        root.decrement()
                    }
                }
            }

            // ── Editable value ────────────────────────────────────────────────
            Item {
                id: valueCell
                width: 110; height: parent.height

                Rectangle {
                    anchors { left:  parent.left;  top: parent.top; bottom: parent.bottom }
                    width: 1; color: tok.edge
                }
                Rectangle {
                    anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
                    width: 1; color: tok.edge
                }

                TextInput {
                    id: valueInput
                    anchors.centerIn: parent
                    width: parent.width - 10

                    text: root.value
                    readOnly: true
                    horizontalAlignment: TextInput.AlignHCenter

                    font.pixelSize: tok.stepperPx
                    font.family: tok.serifFamily
                    font.features: { "tnum": 1 }
                    color: tok.ink
                    selectionColor: tok.sageSoft
                    selectedTextColor: tok.ink
                    cursorVisible: activeFocus && !readOnly

                    // Keep display in sync when bridge pushes an update
                    Connections {
                        target: root
                        function onValueChanged() {
                            if (valueInput.readOnly)
                                valueInput.text = root.value
                        }
                    }

                    function beginEdit() {
                        readOnly = false
                        forceActiveFocus()
                        selectAll()
                    }

                    function cancelEdit() {
                        text = root.value
                        readOnly = true
                    }

                    function commitEdit() {
                        var secs = root.parseTime(text)
                        if (secs >= 1) {
                            root.committed(secs)
                        } else {
                            text = root.value    // revert invalid input
                        }
                        readOnly = true
                    }

                    Keys.onReturnPressed: commitEdit()
                    Keys.onEnterPressed:  commitEdit()
                    Keys.onTabPressed:    commitEdit()
                    Keys.onEscapePressed: cancelEdit()

                    onActiveFocusChanged: {
                        if (!activeFocus && !readOnly)
                            commitEdit()
                    }

                    // Click to enter edit mode
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.IBeamCursor
                        visible: parent.readOnly
                        onClicked: parent.beginEdit()
                    }
                }
            }

            // ── + button ──────────────────────────────────────────────────────
            Item {
                width: 30; height: parent.height

                Text {
                    anchors.centerIn: parent
                    text: "+"
                    font.pixelSize: tok.btnPx; font.weight: Font.Medium
                    color: tok.ink2
                }

                MouseArea {
                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (!valueInput.readOnly) valueInput.cancelEdit()
                        root.increment()
                    }
                }
            }
        }
    }

    Accessible.role: Accessible.SpinBox
    Accessible.name: root.value
    Accessible.editable: true
}
