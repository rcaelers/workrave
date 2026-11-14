pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic

// Renders library page models. Widget and presentation dispatch live here.
Item {
    id: root

    property var model: null

    implicitWidth:  parent ? parent.width : 500
    implicitHeight: col.implicitHeight

    readonly property var tok: Theme

    Column {
        id: col
        width: parent.width
        spacing: 24

        Repeater {
            model: root.model ? root.model.groups : []

            delegate: PrefGroup {
                id: groupItem
                required property var modelData
                width: col.width
                title: modelData.label

                Repeater {
                    model: groupItem.modelData.rows

                    delegate: Item {
                        id: rowItem
                        required property var modelData
                        required property int index

                        readonly property bool cardsChoice: rowItem.modelData.kind === 4 && rowItem.modelData.presentation === "cards"
                        readonly property bool segmentedChoice: rowItem.modelData.kind === 4 && rowItem.modelData.presentation === "segmented"
                        readonly property bool usesTimeControl: rowItem.modelData.kind === 2
                            || (rowItem.modelData.kind === 3 && rowItem.modelData.presentation === "duration")

                        width: parent ? parent.width : 0

                        // Collapse disabled rows — matches native pref page behaviour.
                        visible: rowItem.modelData.enabled

                        // Use the height of whichever row type is active. Math.max across
                        // all types would pull in PrefTimeControl's slider height (~100px)
                        // and create huge empty space in toggle/choice rows.
                        implicitHeight: {
                            if (customRow.available) return customRow.implicitHeight
                            var k = rowItem.modelData.kind
                            if (k === 1) return toggleRow.implicitHeight
                            if (usesTimeControl) return timeRow.implicitHeight
                            if (k === 3) return spinRow.implicitHeight
                            if (cardsChoice) return cardsRow.implicitHeight
                            if (segmentedChoice) return segmentedRow.implicitHeight
                            if (k === 4) return choiceRow.implicitHeight
                            if (k === 5) return entryRow.implicitHeight
                            return 48
                        }

                        ControlPresentation {
                            id: customRow
                            anchors { left: parent.left; right: parent.right }
                            preference: rowItem.modelData
                        }

                        // kind == 1 (Toggle)
                        PrefToggleRow {
                            id: toggleRow
                            anchors { left: parent.left; right: parent.right }
                            visible: !customRow.available && rowItem.modelData.kind === 1
                            label:   rowItem.modelData.label
                            hint:    rowItem.modelData.hint
                            checked: rowItem.modelData.checked
                            onToggled: rowItem.modelData.setChecked(!rowItem.modelData.checked)
                        }

                        // kind == 2 (Time)
                        PrefTimeControl {
                            id: timeRow
                            anchors { left: parent.left; right: parent.right }
                            visible: !customRow.available && rowItem.usesTimeControl
                            label:         rowItem.modelData.label
                            hint:          rowItem.modelData.hint
                            value:         rowItem.modelData.timeDisplay
                            ticks:         rowItem.modelData.timeTicks
                            sliderValue:   rowItem.modelData.timeNorm
                            sliderVisible: true
                            onIncrement:   rowItem.modelData.increment()
                            onDecrement:   rowItem.modelData.decrement()
                            onSliderMoved: function(v) { rowItem.modelData.setTimeNorm(v) }
                            onCommitted: function(seconds) { rowItem.modelData.setSeconds(seconds) }
                        }

                        // kind == 3 (Spin / Value)
                        PrefSpinRow {
                            id: spinRow
                            anchors { left: parent.left; right: parent.right }
                            visible: !customRow.available && rowItem.modelData.kind === 3 && !rowItem.usesTimeControl
                            label:   rowItem.modelData.label
                            hint:    rowItem.modelData.hint
                            display: rowItem.modelData.spinDisplay
                            onIncrement: rowItem.modelData.increment()
                            onDecrement: rowItem.modelData.decrement()
                        }

                        // kind == 4 (Choice)
                        PrefChoiceRow {
                            id: choiceRow
                            anchors { left: parent.left; right: parent.right }
                            visible: !customRow.available && rowItem.modelData.kind === 4 && !rowItem.cardsChoice && !rowItem.segmentedChoice
                            label:        rowItem.modelData.label
                            hint:         rowItem.modelData.hint
                            options:      rowItem.modelData.options
                            currentIndex: rowItem.modelData.currentIndex
                            onSelected: function(idx) { rowItem.modelData.setCurrentIndex(idx) }
                        }

                        PrefCardsRow {
                            id: cardsRow
                            anchors { left: parent.left; right: parent.right }
                            previews: rowItem.modelData.previews
                            previewHeight: rowItem.modelData.previewHeight
                            visible: !customRow.available && rowItem.cardsChoice
                            label: rowItem.modelData.label
                            hint: rowItem.modelData.hint
                            options: rowItem.modelData.options
                            currentIndex: rowItem.modelData.currentIndex
                            onSelected: (idx) => rowItem.modelData.setCurrentIndex(idx)
                        }

                        PrefSegmentedRow {
                            id: segmentedRow
                            anchors { left: parent.left; right: parent.right }
                            visible: !customRow.available && rowItem.segmentedChoice
                            label: rowItem.modelData.label
                            accentColor: root.tok.resolveColor(rowItem.modelData.accent)
                            options: rowItem.modelData.options
                            value: rowItem.modelData.currentIndex
                            isLast: rowItem.index === groupItem.modelData.rows.length - 1
                            onSelected: (idx) => rowItem.modelData.setCurrentIndex(idx)
                        }

                        // kind == 5 (Entry)
                        Item {
                            id: entryRow
                            anchors { left: parent.left; right: parent.right }
                            visible: !customRow.available && rowItem.modelData.kind === 5
                            implicitHeight: visible ? 48 : 0

                            Text {
                                anchors { left: parent.left; leftMargin: 16; verticalCenter: parent.verticalCenter }
                                text: rowItem.modelData.label
                                font.pixelSize: 13
                                color: root.tok.text
                            }

                            TextField {
                                anchors { right: parent.right; rightMargin: 16; verticalCenter: parent.verticalCenter }
                                width: 200
                                text: rowItem.modelData.entryText
                                font.pixelSize: 13
                                onEditingFinished: rowItem.modelData.setEntryText(text)
                            }
                        }
                    }
                }
            }
        }
    }
}
