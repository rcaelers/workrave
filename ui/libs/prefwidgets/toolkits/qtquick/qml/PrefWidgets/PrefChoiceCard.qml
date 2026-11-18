pragma ComponentBehavior: Bound

import QtQuick

// Selectable choice card with an optional registered preview.
Item {
    id: card

    property bool   active:    false
    property string cardTitle: ""
    property var    preview:   ({})
    property int    previewHeight: 52

    signal clicked()

    implicitHeight: cardCol.implicitHeight + 22

    readonly property var tok: Theme

    Rectangle {
        anchors.fill: parent; radius: 10
        color:        card.active ? card.tok.accentSoft : card.tok.surface
        border.color: card.active ? card.tok.accent : card.tok.border
        border.width: card.active ? 2 : 1
    }

    Column {
        id: cardCol
        anchors {
            left: parent.left; right: parent.right; top: parent.top
            leftMargin: 10; rightMargin: 10; topMargin: 10
        }
        spacing: 8

        // Optional application-supplied preview
        Item {
            visible: illustration.available
            width: parent.width; height: visible ? card.previewHeight : 0

            Rectangle {
                anchors.fill: parent; radius: 5
                color: card.tok.surface
                border.color: card.tok.border; border.width: 1
            }

            Preview {
                id: illustration
                anchors.fill: parent
                anchors.margins: 4
                asset: card.preview.asset || ""
            }
        }

        Text {
            width: parent.width
            text: card.cardTitle
            font.pixelSize: card.tok.captionPx; font.weight: Font.Medium
            color: card.tok.text; lineHeight: 1.3
            wrapMode: Text.WordWrap
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: card.clicked()
    }

    Accessible.role: Accessible.RadioButton
    Accessible.name: card.cardTitle
    Accessible.description: card.preview.description || ""
    Accessible.checked: card.active
}
