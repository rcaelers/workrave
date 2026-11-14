pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root
    property var groups: []

    PrefTokens { id: tok }

    Row {
        anchors.centerIn: parent
        spacing: 10

        Repeater {
            model: root.groups
            delegate: Item {
                id: groupItem
                required property var modelData
                width: 22; height: 22

                Repeater {
                    model: groupItem.modelData
                    delegate: Rectangle {
                        id: badge
                        required property string modelData
                        required property int index
                        anchors.fill: parent
                        radius: 11
                        color: modelData === "M" ? tok.sage : modelData === "R" ? tok.clay : tok.sageDeep
                        opacity: index === 0 ? 1 : 0.35
                    }
                }
                Text {
                    visible: groupItem.modelData.length > 1
                    anchors { bottom: parent.bottom; right: parent.right; bottomMargin: -2; rightMargin: -3 }
                    text: "↻" + groupItem.modelData.length
                    font.pixelSize: 7
                    color: tok.mute
                }
            }
        }
    }
}
