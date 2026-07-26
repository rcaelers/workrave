import QtQuick

// TimerVisibilityRow — label + color dot + three-segment visibility selector.
// value: 0=always show, 1=when first due, 2=hide
Item {
    id: row

    property string timerName:  ""
    property color  timerColor: tok.sage
    property int    value:      0
    property bool   isLast:     false

    signal selected(int v)

    implicitWidth:  parent ? parent.width : 400
    implicitHeight: 52

    PrefTokens { id: tok }

    Rectangle {
        visible: !row.isLast
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 1; color: tok.edge2
    }

    // Colored dot badge
    Item {
        id: dotBadge
        anchors { left: parent.left; verticalCenter: parent.verticalCenter }
        width: 28; height: 28

        Rectangle {
            anchors.fill: parent; radius: 8
            color: Qt.rgba(row.timerColor.r, row.timerColor.g, row.timerColor.b, 0.12)
        }
        Rectangle {
            anchors.centerIn: parent
            width: 10; height: 10; radius: 5
            color: row.timerColor
        }
    }

    Text {
        anchors { left: dotBadge.right; leftMargin: 10; verticalCenter: parent.verticalCenter }
        text: row.timerName
        font.pixelSize: tok.labelPx; font.weight: Font.Medium
        color: tok.ink
    }

    // Segmented control
    Item {
        id: segControl
        anchors { right: parent.right; verticalCenter: parent.verticalCenter }
        implicitWidth: segRow.implicitWidth + 4
        height: 32

        Rectangle {
            anchors.fill: parent; radius: 8
            color: tok.panel; border.color: tok.edge; border.width: 1
        }

        Row {
            id: segRow
            x: 2; y: 2
            height: parent.height - 4
            spacing: 0

            Repeater {
                model: [
                    { val: 0, label: qsTr("Show")           },
                    { val: 1, label: qsTr("When first due") },
                    { val: 2, label: qsTr("Hide")           },
                ]

                delegate: Item {
                    required property var modelData
                    required property int index

                    implicitWidth: segLabel.implicitWidth + 24
                    height: segRow.height

                    Rectangle {
                        anchors.fill: parent; radius: 6
                        color:   row.value === modelData.val ? tok.sage : "transparent"
                        visible: row.value === modelData.val
                    }

                    Text {
                        id: segLabel
                        anchors.centerIn: parent
                        text: modelData.label
                        font.pixelSize: tok.hintPx
                        color:       row.value === modelData.val ? "#ffffff" : tok.ink2
                        font.weight: row.value === modelData.val ? Font.DemiBold : Font.Normal
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: row.selected(modelData.val)
                    }
                }
            }
        }
    }
}
