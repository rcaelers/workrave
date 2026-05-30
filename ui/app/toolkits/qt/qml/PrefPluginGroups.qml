import QtQuick
import QtQuick.Controls.Basic

// PrefPluginGroups — renders groups from a plugin bridge into an existing pref page.
// Set bridge to a PluginPageBridge/ActivePluginPageBridge exposing .groups.
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

            delegate: Item {
                id: groupItem
                required property var modelData
                width: col.width
                implicitHeight: groupCol.implicitHeight

                Column {
                    id: groupCol
                    width: parent.width
                    spacing: 0

                    Text {
                        visible: modelData.label !== ""
                        width: parent.width
                        text: modelData.label
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                        font.letterSpacing: 11 * 0.12
                        color: tok.mute
                        topPadding: 4
                        bottomPadding: 8
                    }

                    Rectangle {
                        width: parent.width
                        implicitHeight: rowsCol.implicitHeight
                        radius: 10
                        color: tok.panel
                        border.color: tok.edge
                        border.width: 1

                        Column {
                            id: rowsCol
                            width: parent.width
                            spacing: 0

                            Repeater {
                                model: modelData.rows

                                delegate: Item {
                                    id: rowItem
                                    required property var modelData
                                    required property int index

                                    width: rowsCol.width
                                    implicitHeight: rowContent.implicitHeight

                                    Rectangle {
                                        visible: index > 0
                                        anchors { left: parent.left; right: parent.right; top: parent.top; leftMargin: 16 }
                                        height: 1
                                        color: tok.edge2
                                    }

                                    Item {
                                        id: rowContent
                                        width: parent.width
                                        implicitHeight: Math.max(toggleRow.implicitHeight,
                                                                 timeRow.implicitHeight,
                                                                 spinRow.implicitHeight,
                                                                 choiceRow.implicitHeight,
                                                                 entryRow.implicitHeight)

                                        PrefToggleRow {
                                            id: toggleRow
                                            anchors { left: parent.left; right: parent.right }
                                            visible: rowItem.modelData.kind === 1
                                            enabled: rowItem.modelData.enabled
                                            label:   rowItem.modelData.label
                                            checked: rowItem.modelData.checked
                                            onToggled: rowItem.modelData.setChecked(!rowItem.modelData.checked)
                                        }

                                        PrefTimeControl {
                                            id: timeRow
                                            anchors { left: parent.left; right: parent.right }
                                            visible:      rowItem.modelData.kind === 2
                                            enabled:      rowItem.modelData.enabled
                                            label:        rowItem.modelData.label
                                            hint:         rowItem.modelData.timeDisplay
                                            sliderValue:  rowItem.modelData.timeNorm
                                            sliderVisible: true
                                            onIncrement:  rowItem.modelData.increment()
                                            onDecrement:  rowItem.modelData.decrement()
                                            onSliderMoved: function(v) { rowItem.modelData.setTimeNorm(v) }
                                        }

                                        PrefSpinRow {
                                            id: spinRow
                                            anchors { left: parent.left; right: parent.right }
                                            visible: rowItem.modelData.kind === 3
                                            enabled: rowItem.modelData.enabled
                                            label:   rowItem.modelData.label
                                            display: rowItem.modelData.spinDisplay
                                            onIncrement: rowItem.modelData.increment()
                                            onDecrement: rowItem.modelData.decrement()
                                        }

                                        PrefChoiceRow {
                                            id: choiceRow
                                            anchors { left: parent.left; right: parent.right }
                                            visible:      rowItem.modelData.kind === 4
                                            enabled:      rowItem.modelData.enabled
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
                                            opacity:        rowItem.modelData.enabled ? 1.0 : 0.45

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
                                                enabled: rowItem.modelData.enabled
                                                onEditingFinished: rowItem.modelData.setEntryText(text)
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
