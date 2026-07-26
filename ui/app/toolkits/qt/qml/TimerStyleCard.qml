import QtQuick

// TimerStyleCard — selectable card showing a display style option.
// kind: "rings" | "bars" | "focus"
Item {
    id: card

    property bool   active:    false
    property string cardTitle: ""
    property string cardSub:   ""
    property string kind:      "rings"

    signal clicked()

    implicitHeight: cardCol.implicitHeight + 26

    PrefTokens { id: tok }

    Rectangle {
        anchors.fill: parent; radius: 12
        color:        card.active ? tok.sageSoft : tok.panel
        border.color: card.active ? tok.sage : tok.edge
        border.width: card.active ? 2 : 1
    }

    Column {
        id: cardCol
        anchors {
            left: parent.left; right: parent.right; top: parent.top
            leftMargin: 12; rightMargin: 12; topMargin: 12
        }
        spacing: 8

        // Preview area
        Item {
            width: parent.width; height: 110
            clip: true

            Rectangle {
                anchors.fill: parent; radius: 8
                color: card.active ? Qt.rgba(1, 1, 1, 0.55) : tok.bg
                border.color: tok.edge; border.width: 1
            }

            TimerStylePreview {
                anchors.centerIn: parent
                kind: card.kind
            }
        }

        // Radio dot + title
        Row {
            spacing: 6

            Item {
                width: 14; height: 14
                anchors.verticalCenter: parent.verticalCenter

                Rectangle {
                    anchors.fill: parent; radius: 7
                    color:        card.active ? tok.sage : tok.panel
                    border.color: card.active ? tok.sage : tok.edge
                    border.width: 1
                }
                Rectangle {
                    visible: card.active
                    anchors.centerIn: parent
                    width: 8; height: 8; radius: 4; color: tok.panel
                }
                Rectangle {
                    visible: card.active
                    anchors.centerIn: parent
                    width: 4; height: 4; radius: 2; color: tok.sage
                }
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: card.cardTitle
                font.pixelSize: tok.bodyPx; font.weight: Font.DemiBold
                color: tok.ink
            }
        }

        Text {
            width: parent.width
            text: card.cardSub
            font.pixelSize: tok.hintPx; color: tok.ink2
            wrapMode: Text.WordWrap; lineHeight: tok.hintLineH
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: card.clicked()
    }

    Accessible.role: Accessible.RadioButton
    Accessible.name: card.cardTitle
    Accessible.checked: card.active
}
