pragma ComponentBehavior: Bound

import QtQuick

// PrefToggleRow — label + optional hint on the left, iOS-style toggle on the right.
// Properties: label, hint, checked, badge (optional tag text)
// Signals: toggled(bool checked)
Item {
    id: root

    property string label:   ""
    property string hint:    ""
    property string badge:   ""
    property bool   checked: false
    property bool   isLast:  false

    signal toggled(bool checked)

    implicitWidth:  parent ? parent.width : 400
    implicitHeight: rowContent.implicitHeight + root.tok.rowPad

    readonly property var tok: Theme

    // ── Bottom divider ───────────────────────────────────────────────────────
    Rectangle {
        visible: !root.isLast
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 1
        color: root.tok.borderSubtle
    }

    // ── Row content ──────────────────────────────────────────────────────────
    Item {
        id: rowContent
        anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
        implicitHeight: labelCol.implicitHeight

        // Left: label + optional hint
        Column {
            id: labelCol
            anchors { left: parent.left; right: toggle.left; rightMargin: 16; verticalCenter: parent.verticalCenter }
            spacing: root.tok.labelHintGap

            Item {
                width: parent.width
                height: labelText.implicitHeight

                Text {
                    id: labelText
                    text: root.label
                    font.pixelSize: root.tok.labelPx
                    font.weight: Font.Medium
                    color: root.tok.text
                }

                Rectangle {
                    visible: root.badge !== ""
                    anchors { left: labelText.right; leftMargin: 8; verticalCenter: labelText.verticalCenter }
                    height: badgeText.implicitHeight + 4
                    width:  badgeText.implicitWidth  + 12
                    radius: 4
                    color:  root.tok.surfaceVariant
                    border.color: root.tok.border
                    border.width: 1

                    Text {
                        id: badgeText
                        anchors.centerIn: parent
                        text: root.badge
                        font.pixelSize: root.tok.captionPx
                        font.weight: Font.DemiBold
                        font.letterSpacing: 9.5 * 0.14
                        color: root.tok.textMuted
                    }
                }
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

        // Right: toggle pill
        Item {
            id: toggle
            anchors { right: parent.right; verticalCenter: parent.verticalCenter }
            width: 36; height: 21

            Rectangle {
                anchors.fill: parent
                radius: 999
                color: root.checked ? root.tok.accent : root.tok.track
                Behavior on color { ColorAnimation { duration: 150 } }
            }

            Rectangle {
                width: 17; height: 17
                radius: 999
                color: "#FFFFFF"
                anchors.verticalCenter: parent.verticalCenter
                x: root.checked ? 17 : 2
                Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    root.checked = !root.checked
                    root.toggled(root.checked)
                }
            }

            Accessible.role: Accessible.CheckBox
            Accessible.name: root.label
            Accessible.checked: root.checked
        }
    }
}
