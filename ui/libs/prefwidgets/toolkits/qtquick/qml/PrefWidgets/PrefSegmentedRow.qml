pragma ComponentBehavior: Bound

import QtQuick

// Generic labelled segmented choice with an optional accent badge.
Item {
    id: row

    property string label:  ""
    property color  accentColor: "transparent"
    property int    value:      0
    property bool   isLast:     false
    property var options: []

    signal selected(int v)

    implicitWidth:  parent ? parent.width : 400
    implicitHeight: 52

    readonly property var tok: Theme

    Rectangle {
        visible: !row.isLast
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 1; color: row.tok.borderSubtle
    }

    // Colored dot badge
    Item {
        id: dotBadge
        visible: row.accentColor.a > 0
        anchors { left: parent.left; verticalCenter: parent.verticalCenter }
        width: visible ? 28 : 0; height: 28

        Rectangle {
            anchors.fill: parent; radius: 8
            color: Qt.rgba(row.accentColor.r, row.accentColor.g, row.accentColor.b, 0.12)
        }
        Rectangle {
            anchors.centerIn: parent
            width: 10; height: 10; radius: 5
            color: row.accentColor
        }
    }

    Text {
        anchors { left: dotBadge.right; leftMargin: dotBadge.visible ? 10 : 0; verticalCenter: parent.verticalCenter }
        text: row.label
        font.pixelSize: row.tok.labelPx; font.weight: Font.Medium
        color: row.tok.text
    }

    // Segmented control
    Item {
        id: segControl
        anchors { right: parent.right; verticalCenter: parent.verticalCenter }
        implicitWidth: segRow.implicitWidth + 4
        height: 32

        Rectangle {
            anchors.fill: parent; radius: 8
            color: row.tok.surface; border.color: row.tok.border; border.width: 1
        }

        Row {
            id: segRow
            x: 2; y: 2
            height: parent.height - 4
            spacing: 0

            Repeater {
                model: row.options

                delegate: Item {
                    id: segment
                    required property string modelData
                    required property int index

                    implicitWidth: segLabel.implicitWidth + 24
                    height: segRow.height

                    Rectangle {
                        anchors.fill: parent; radius: 6
                        color:   row.value === segment.index ? row.tok.accent : "transparent"
                        visible: row.value === segment.index
                    }

                    Text {
                        id: segLabel
                        anchors.centerIn: parent
                        text: segment.modelData
                        font.pixelSize: row.tok.hintPx
                        color:       row.value === segment.index ? "#ffffff" : row.tok.textSecondary
                        font.weight: row.value === segment.index ? Font.DemiBold : Font.Normal
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: row.selected(segment.index)
                    }
                }
            }
        }
    }
}
