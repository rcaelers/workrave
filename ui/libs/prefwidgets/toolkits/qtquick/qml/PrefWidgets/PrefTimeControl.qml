pragma ComponentBehavior: Bound

import QtQuick

// PrefTimeControl — label/hint row + stepper pill + optional ticked slider.
// Signals: increment(), decrement(), sliderMoved(real value)
Item {
    id: root

    property string label:         ""
    property string hint:          ""
    property string value:         "0:00"
    property bool   sliderVisible: true
    property real   sliderValue:   0.5
    property color  sliderColor:   root.tok.accent
    property var    ticks:         []
    property int    secondsStep:   1

    signal increment()
    signal decrement()
    signal sliderMoved(real value)
    signal committed(int seconds)

    implicitWidth:  parent ? parent.width : 400
    implicitHeight: topRow.height
                    + (root.sliderVisible ? sliderItem.implicitHeight + 14 : 0)
                    + root.tok.rowPadLg

    readonly property var tok: Theme

    // ── Bottom divider ───────────────────────────────────────────────────────
    Rectangle {
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 1
        color: root.tok.borderSubtle
    }

    // ── Top row: label/hint + stepper ────────────────────────────────────────
    Item {
        id: topRow
        anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 18 }
        height: Math.max(labelCol.implicitHeight, stepper.implicitHeight)

        Column {
            id: labelCol
            anchors { left: parent.left; right: stepper.left; rightMargin: 24; verticalCenter: parent.verticalCenter }
            spacing: root.tok.labelHintGap

            Text {
                width: parent.width
                text: root.label
                font.pixelSize: root.tok.labelPx
                font.weight: Font.Medium
                color: root.tok.text
            }

            Text {
                visible: root.hint !== ""
                width: parent.width
                text: root.hint
                font.pixelSize: root.tok.hintPx
                color: root.tok.textMuted
                wrapMode: Text.WordWrap
                lineHeight: root.tok.hintLineH
            }
        }

        PrefTimeStepper {
            id: stepper
            anchors { right: parent.right; verticalCenter: parent.verticalCenter }
            value: root.value
            secondsStep: root.secondsStep
            onIncrement:  root.increment()
            onDecrement:  root.decrement()
            onCommitted: (secs) => root.committed(secs)
        }
    }

    // ── Ticked slider ────────────────────────────────────────────────────────
    PrefSliderRow {
        id: sliderItem
        visible: root.sliderVisible
        anchors {
            left: parent.left; top: topRow.bottom; topMargin: 14
            right: parent.right; rightMargin: stepper.implicitWidth + 24
        }
        value: root.sliderValue
        sliderColor: root.sliderColor
        ticks: root.ticks
        onMoved: (v) => { root.sliderValue = v; root.sliderMoved(v) }
    }
}
