pragma ComponentBehavior: Bound

import QtQuick

// Generic illustrated choice grid. Option indices retain their original meaning.
Item {
    id: root

    property string label: ""
    property string hint: ""
    property int currentIndex: 0
    property var options: []
    property var previews: []
    property int previewHeight: 52

    signal selected(int index)

    implicitWidth: parent ? parent.width : 500
    implicitHeight: content.implicitHeight + 28

    readonly property var tok: Theme

    Rectangle {
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 1
        color: root.tok.borderSubtle
    }

    Column {
        id: content
        anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 14 }
        spacing: 14

        Column {
            width: parent.width
            spacing: root.tok.labelHintGap

            Text {
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

        Grid {
            id: cards
            columns: Math.max(1, Math.min(4, root.options.length))
            width: parent.width
            spacing: 10

            Repeater {
                model: root.options

                delegate: PrefChoiceCard {
                    id: card
                    required property string modelData
                    required property int index
                    width: (cards.width - ((cards.columns - 1) * cards.spacing)) / cards.columns
                    active: root.currentIndex === card.index
                    cardTitle: card.modelData
                    preview: root.previews[card.index] || ({})
                    previewHeight: root.previewHeight
                    onClicked: root.selected(card.index)
                }
            }
        }
    }
}
