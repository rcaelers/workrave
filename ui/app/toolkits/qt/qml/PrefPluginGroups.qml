import QtQuick
import QtQuick.Controls.Basic

// PrefPluginGroups — renders groups from a plugin bridge into an existing pref page.
// Set bridge to an ActivePluginPageBridge/PluginPageBridge exposing .groups.
Item {
    id: root

    property var bridge: null

    implicitWidth:  parent ? parent.width : 500
    implicitHeight: col.implicitHeight
    visible:        bridge !== null && bridge.groups.length > 0

    PrefTokens { id: tok }

    Column {
        id: col
        width: parent.width
        spacing: 24

        Repeater {
            model: root.bridge ? root.bridge.groups : []

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

                        width: parent.width

                        // Collapse disabled rows — matches native pref page behaviour.
                        visible: rowItem.modelData.enabled

                        // Use the height of whichever row type is active. Math.max across
                        // all types would pull in PrefTimeControl's slider height (~100px)
                        // and create huge empty space in toggle/choice rows.
                        implicitHeight: {
                            var k = rowItem.modelData.kind
                            if (k === 1) return toggleRow.implicitHeight
                            if (k === 2) return timeRow.implicitHeight
                            if (k === 3) return spinRow.implicitHeight
                            if (k === 4) return choiceRow.implicitHeight
                            if (k === 5) return entryRow.implicitHeight
                            return 48
                        }

                        PrefToggleRow {
                            id: toggleRow
                            anchors { left: parent.left; right: parent.right }
                            visible: rowItem.modelData.kind === 1
                            label:   rowItem.modelData.label
                            checked: rowItem.modelData.checked
                            onToggled: rowItem.modelData.setChecked(!rowItem.modelData.checked)
                        }

                        PrefTimeControl {
                            id: timeRow
                            anchors { left: parent.left; right: parent.right }
                            visible:       rowItem.modelData.kind === 2
                            label:         rowItem.modelData.label
                            hint:          rowItem.modelData.timeDisplay
                            sliderValue:   rowItem.modelData.timeNorm
                            sliderVisible: true
                            onIncrement:   rowItem.modelData.increment()
                            onDecrement:   rowItem.modelData.decrement()
                            onSliderMoved: function(v) { rowItem.modelData.setTimeNorm(v) }
                        }

                        PrefSpinRow {
                            id: spinRow
                            anchors { left: parent.left; right: parent.right }
                            visible: rowItem.modelData.kind === 3
                            label:   rowItem.modelData.label
                            display: rowItem.modelData.spinDisplay
                            onIncrement: rowItem.modelData.increment()
                            onDecrement: rowItem.modelData.decrement()
                        }

                        PrefChoiceRow {
                            id: choiceRow
                            anchors { left: parent.left; right: parent.right }
                            visible:      rowItem.modelData.kind === 4
                            label:        rowItem.modelData.label
                            options:      rowItem.modelData.options
                            currentIndex: rowItem.modelData.currentIndex
                            onSelected: function(idx) { rowItem.modelData.setCurrentIndex(idx) }
                        }

                        Item {
                            id: entryRow
                            anchors { left: parent.left; right: parent.right }
                            visible:        rowItem.modelData.kind === 5
                            implicitHeight: visible ? 48 : 0

                            Text {
                                anchors { left: parent.left; leftMargin: 16; verticalCenter: parent.verticalCenter }
                                text: rowItem.modelData.label
                                font.pixelSize: 13
                                color: tok.ink
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
