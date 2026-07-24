import QtQuick

// PrefSpinRow — label/hint + stepper pill (no slider).
// For integer or narrow values. Signals: increment(), decrement()
Item {
    id: root

    property string label:   ""
    property string hint:    ""
    property string display: "0"
    property bool   narrow:  true

    signal increment()
    signal decrement()

    implicitWidth:  parent ? parent.width : 400
    implicitHeight: Math.max(labelCol.implicitHeight, stepper.implicitHeight) + tok.rowPadLg

    PrefTokens { id: tok }

    // ── Bottom divider ───────────────────────────────────────────────────────
    Rectangle {
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 1
        color: tok.edge2
    }

    // ── Row ──────────────────────────────────────────────────────────────────
    Item {
        anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
        height: Math.max(labelCol.implicitHeight, stepper.implicitHeight)

        Column {
            id: labelCol
            anchors { left: parent.left; right: stepper.left; rightMargin: 24; verticalCenter: parent.verticalCenter }
            spacing: tok.labelHintGap

            Text {
                width: parent.width
                text: root.label
                font.pixelSize: tok.labelPx
                font.weight: Font.Medium
                color: tok.ink
            }

            Text {
                visible: root.hint !== ""
                width: parent.width
                text: root.hint
                font.pixelSize: tok.hintPx
                color: tok.mute
                wrapMode: Text.WordWrap
                lineHeight: tok.hintLineH
            }
        }

        PrefStepper {
            id: stepper
            anchors { right: parent.right; verticalCenter: parent.verticalCenter }
            value: root.display
            narrow: root.narrow
            onIncrement: root.increment()
            onDecrement: root.decrement()
        }
    }
}
