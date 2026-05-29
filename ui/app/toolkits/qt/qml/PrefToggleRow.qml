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

    signal toggled(bool checked)

    implicitWidth:  parent ? parent.width : 400
    implicitHeight: rowContent.implicitHeight + 28

    PrefTokens { id: tok }

    // ── Bottom divider ───────────────────────────────────────────────────────
    Rectangle {
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 1
        color: tok.edge2
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
            spacing: 3

            Item {
                width: parent.width
                height: labelText.implicitHeight

                Text {
                    id: labelText
                    text: root.label
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    color: tok.ink
                }

                Rectangle {
                    visible: root.badge !== ""
                    anchors { left: labelText.right; leftMargin: 8; verticalCenter: labelText.verticalCenter }
                    height: badgeText.implicitHeight + 4
                    width:  badgeText.implicitWidth  + 12
                    radius: 4
                    color:  tok.panel2
                    border.color: tok.edge
                    border.width: 1

                    Text {
                        id: badgeText
                        anchors.centerIn: parent
                        text: root.badge
                        font.pixelSize: 10
                        font.weight: Font.DemiBold
                        font.letterSpacing: 9.5 * 0.14
                        color: tok.mute
                    }
                }
            }

            Text {
                visible: root.hint !== ""
                width: parent.width
                text: root.hint
                font.pixelSize: 12
                color: tok.mute
                wrapMode: Text.WordWrap
                lineHeight: 1.45
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
                color: root.checked ? tok.sage : tok.track
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
