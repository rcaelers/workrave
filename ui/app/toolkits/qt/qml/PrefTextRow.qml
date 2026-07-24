import QtQuick
import QtQuick.Controls.Basic

// PrefTextRow — label/hint on left, text field on right.
// Properties: label, hint, value, placeholder
// Signals: committed(string text)
Item {
    id: root

    property string label:       ""
    property string hint:        ""
    property string value:       ""
    property string placeholder: ""

    signal committed(string text)

    implicitWidth:  parent ? parent.width : 400
    implicitHeight: Math.max(labelCol.implicitHeight, field.height) + tok.rowPad

    PrefTokens { id: tok }

    Rectangle {
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 1
        color: tok.edge2
    }

    Item {
        anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
        height: Math.max(labelCol.implicitHeight, field.height)

        Column {
            id: labelCol
            anchors { left: parent.left; right: field.left; rightMargin: 16; verticalCenter: parent.verticalCenter }
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

        TextField {
            id: field
            anchors { right: parent.right; verticalCenter: parent.verticalCenter }
            width: 260
            text: root.value
            placeholderText: root.placeholder
            font.pixelSize: 13
            leftPadding: 10
            rightPadding: 10

            background: Rectangle {
                radius: 8
                color: tok.panel
                border.color: field.activeFocus ? tok.sage : tok.edge
                border.width: field.activeFocus ? 1.5 : 1
            }

            onEditingFinished: root.committed(text)
        }
    }
}
