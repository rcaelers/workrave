import QtQuick

// ConfirmDialog — full-parent modal overlay for destructive confirmations.
// Place as a direct child of the root item; set z high enough to cover everything.
// Usage:
//   ConfirmDialog {
//       id: dlg
//       anchors.fill: parent; z: 200
//       onConfirmed: (action) => {
//           if (action === "shutdown") bridge.requestShutdown()
//           else if (action === "sleep") bridge.requestSleep()
//       }
//   }
//   dlg.ask("shutdown", qsTr("Shut down"), qsTr("Shut down the computer?"))
Item {
    id: root

    signal confirmed(string action)

    function ask(action, title, message) {
        root._action   = action
        titleText.text = title
        msgText.text   = message
        root.visible   = true
        root.forceActiveFocus()
    }

    visible: false
    focus: true
    Keys.onEscapePressed: root.visible = false

    property string _action: ""

    PrefTokens { id: tok }

    // Click-blocking scrim
    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.55)
        MouseArea { anchors.fill: parent }
    }

    // Dialog card
    Rectangle {
        anchors.centerIn: parent
        width: Math.min(parent.width - 48, 360)
        height: body.implicitHeight + 40
        radius: 14
        color: tok.panel
        border.color: tok.edge
        border.width: 1

        Column {
            id: body
            anchors {
                left: parent.left; right: parent.right; top: parent.top
                leftMargin: 24; rightMargin: 24; topMargin: 24
            }
            spacing: 8

            Text {
                id: titleText
                width: parent.width
                font.pixelSize: 15; font.weight: Font.SemiBold
                color: tok.ink
                wrapMode: Text.WordWrap
            }

            Text {
                id: msgText
                width: parent.width
                font.pixelSize: 13
                color: tok.ink2
                wrapMode: Text.WordWrap
                lineHeight: tok.hintLineH
            }

            Item { width: 1; height: 8 }

            Row {
                width: parent.width
                spacing: 10
                layoutDirection: Qt.RightToLeft

                // Confirm (danger-colored)
                Rectangle {
                    height: 34
                    width: confirmLbl.implicitWidth + 24
                    radius: 8
                    color: tok.danger

                    Text {
                        id: confirmLbl
                        anchors.centerIn: parent
                        text: qsTr("Confirm")
                        font.pixelSize: 13; font.weight: Font.Medium
                        color: "#FFFFFF"
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.visible = false
                            root.confirmed(root._action)
                        }
                    }
                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("Confirm")
                }

                // Cancel
                Rectangle {
                    height: 34
                    width: cancelLbl.implicitWidth + 24
                    radius: 8
                    color: tok.panel2
                    border.color: tok.edge
                    border.width: 1

                    Text {
                        id: cancelLbl
                        anchors.centerIn: parent
                        text: qsTr("Cancel")
                        font.pixelSize: 13; font.weight: Font.Medium
                        color: tok.ink2
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.visible = false
                    }
                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("Cancel")
                }
            }
        }
    }
}
