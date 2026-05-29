import QtQuick
import QtQuick.Controls.Basic

// PrefChoiceRow — label/hint + inline dropdown pill on the right.
// Properties: label, hint, model (list of strings), currentIndex
// Signals: activated(int index)
Item {
    id: root

    property string label:        ""
    property string hint:         ""
    property var    options:      []
    property int    currentIndex: 0

    signal selected(int index)

    implicitWidth:  parent ? parent.width : 400
    implicitHeight: Math.max(labelCol.implicitHeight, pill.height) + 28

    PrefTokens { id: tok }

    // ── Bottom divider ───────────────────────────────────────────────────────
    Rectangle {
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 1
        color: tok.edge2
    }

    // ── Row ──────────────────────────────────────────────────────────────────
    Item {
        anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
        height: Math.max(labelCol.implicitHeight, pill.height)

        Column {
            id: labelCol
            anchors { left: parent.left; right: pill.left; rightMargin: 16; verticalCenter: parent.verticalCenter }
            spacing: 3

            Text {
                width: parent.width
                text: root.label
                font.pixelSize: 14
                font.weight: Font.Medium
                color: tok.ink
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

        // Dropdown pill
        ComboBox {
            id: pill
            anchors { right: parent.right; verticalCenter: parent.verticalCenter }
            model: root.options
            currentIndex: root.currentIndex
            onActivated: (idx) => root.selected(idx)

            implicitHeight: 34
            implicitWidth: Math.max(contentItem.implicitWidth + 48, 160)

            background: Rectangle {
                radius: 10
                color: tok.panel
                border.color: tok.edge
                border.width: 1
            }

            contentItem: Text {
                leftPadding: 14
                rightPadding: 28
                text: pill.displayText
                font.pixelSize: 13
                color: tok.ink
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            indicator: Canvas {
                x: pill.width - width - 10
                y: (pill.height - height) / 2
                width: 10; height: 6
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)
                    ctx.beginPath()
                    ctx.moveTo(1, 1); ctx.lineTo(5, 5); ctx.lineTo(9, 1)
                    ctx.strokeStyle = tok.ink2.toString()
                    ctx.lineWidth = 1.4
                    ctx.lineCap = "round"; ctx.lineJoin = "round"
                    ctx.stroke()
                }
            }

            popup: Popup {
                y: pill.height + 2
                width: pill.width
                implicitHeight: listView.implicitHeight + 2
                padding: 1

                background: Rectangle {
                    radius: 8
                    color: tok.panel
                    border.color: tok.edge
                    border.width: 1
                }

                contentItem: ListView {
                    id: listView
                    clip: true
                    implicitHeight: contentHeight
                    model: pill.popup.visible ? pill.delegateModel : null
                    currentIndex: pill.highlightedIndex
                }
            }

            delegate: ItemDelegate {
                required property var model
                required property int index
                width: pill.width
                highlighted: pill.highlightedIndex === index

                background: Rectangle {
                    color: parent.highlighted ? tok.sageSoft : "transparent"
                    radius: 6
                }

                contentItem: Text {
                    leftPadding: 14
                    text: parent.model.modelData !== undefined
                          ? parent.model.modelData
                          : ""
                    font.pixelSize: 13
                    color: tok.ink
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
}
