import QtQuick

// TimerPlacementCard — selectable card showing a timer slot arrangement.
// pattern: array of arrays, e.g. [["M","R"], ["D"]]
Item {
    id: plCard

    property bool   active:    false
    property string cardTitle: ""
    property var    pattern:   []

    signal clicked()

    implicitHeight: plCardCol.implicitHeight + 22

    PrefTokens { id: tok }

    Rectangle {
        anchors.fill: parent; radius: 10
        color:        plCard.active ? tok.sageSoft : tok.panel
        border.color: plCard.active ? tok.sage : tok.edge
        border.width: plCard.active ? 2 : 1
    }

    Column {
        id: plCardCol
        anchors {
            left: parent.left; right: parent.right; top: parent.top
            leftMargin: 10; rightMargin: 10; topMargin: 10
        }
        spacing: 8

        // Mini status window diagram
        Item {
            width: parent.width; height: 52

            Rectangle {
                anchors.fill: parent; radius: 5
                color: tok.panel
                border.color: tok.edge; border.width: 1
            }

            Row {
                anchors.centerIn: parent
                spacing: 10

                Repeater {
                    model: plCard.pattern

                    delegate: Item {
                        id: slotItem
                        required property var modelData
                        required property int index
                        width: 22; height: 22

                        Repeater {
                            model: slotItem.modelData

                            delegate: Rectangle {
                                required property string modelData
                                required property int    index
                                anchors.fill: parent; radius: 11
                                color:   modelData === "M" ? tok.sage
                                       : modelData === "R" ? tok.clay
                                       :                     tok.sageDeep
                                opacity: index === 0 ? 1.0 : 0.35
                            }
                        }

                        Text {
                            visible: slotItem.modelData.length > 1
                            anchors { bottom: parent.bottom; right: parent.right; bottomMargin: -2; rightMargin: -3 }
                            text: "↻" + slotItem.modelData.length
                            font.pixelSize: 7
                            color: tok.mute
                        }
                    }
                }
            }
        }

        Text {
            width: parent.width
            text: plCard.cardTitle
            font.pixelSize: tok.captionPx; font.weight: Font.Medium
            color: tok.ink; lineHeight: 1.3
            wrapMode: Text.WordWrap
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: plCard.clicked()
    }

    Accessible.role: Accessible.RadioButton
    Accessible.name: plCard.cardTitle
    Accessible.checked: plCard.active
}
