pragma ComponentBehavior: Bound

import QtQuick

// PrefGroup — uppercase section header + 1px bottom border + child rows below.
// Usage: PrefGroup { title: "Timing"; PrefToggleRow { ... } ... }
Item {
    id: root

    property string title: ""
    default property alias content: contentColumn.data

    implicitWidth:  parent ? parent.width : 400
    implicitHeight: header.height + 4 + contentColumn.implicitHeight

    readonly property var tok: Theme

    // ── Section header ───────────────────────────────────────────────────────
    Item {
        id: header
        width: parent.width
        height: headerText.implicitHeight + 6

        Text {
            id: headerText
            anchors { left: parent.left; right: parent.right; top: parent.top }
            text: root.title.toUpperCase()
            font.pixelSize: root.tok.captionPx
            font.weight: Font.DemiBold
            font.letterSpacing: 10.5 * 0.18
            color: root.tok.textMuted
        }

        Rectangle {
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
            height: 1
            color: root.tok.border
        }
    }

    // ── Child rows ───────────────────────────────────────────────────────────
    Column {
        id: contentColumn
        anchors { top: header.bottom; topMargin: 4; left: parent.left; right: parent.right }
    }
}
