import QtQuick
import QtQuick.Controls.Basic

// PrefLanguageRow — label/hint + language dropdown.
// Each entry in 'languages' must have: id (string), localizedName (string), nativeName (string).
// 'currentId' identifies the selected language; empty string means "System default".
// Emits selected(id) when the user picks a language.
Item {
    id: root

    property string label:     ""
    property string hint:      ""
    property var    languages: []
    property string currentId: ""

    signal selected(string id)

    implicitWidth:  parent ? parent.width : 400
    implicitHeight: Math.max(labelCol.implicitHeight, pill.height) + tok.rowPad

    PrefTokens { id: tok }

    // ── Bottom divider ───────────────────────────────────────────────────────
    Rectangle {
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 1
        color:  tok.edge2
    }

    // ── Row ──────────────────────────────────────────────────────────────────
    Item {
        anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
        height: Math.max(labelCol.implicitHeight, pill.height)

        Column {
            id: labelCol
            anchors { left: parent.left; right: pill.left; rightMargin: 16; verticalCenter: parent.verticalCenter }
            spacing: tok.labelHintGap

            Text {
                width: parent.width
                text:  root.label
                font.pixelSize: tok.labelPx
                font.weight: Font.Medium
                color: tok.ink
            }

            Text {
                visible: root.hint !== ""
                width: parent.width
                text:  root.hint
                font.pixelSize: tok.hintPx
                color: tok.mute
                wrapMode: Text.WordWrap
                lineHeight: tok.hintLineH
            }
        }

        // ── Dropdown pill ────────────────────────────────────────────────────
        ComboBox {
            id: pill
            anchors { right: parent.right; verticalCenter: parent.verticalCenter }

            model: root.languages
            textRole: "nativeName"
            valueRole: "id"

            implicitHeight: 34
            implicitWidth: 200

            // Keep pill in sync with currentId from outside
            currentIndex: {
                for (var i = 0; i < root.languages.length; i++) {
                    if (root.languages[i].id === root.currentId) return i
                }
                return 0
            }

            onActivated: (idx) => root.selected(root.languages[idx].id)

            // ── Pill background ──────────────────────────────────────────────
            background: Rectangle {
                radius: 10
                color: tok.panel
                border.color: tok.edge
                border.width: 1
            }

            // ── Pill label — shows native name of selection ──────────────────
            contentItem: Column {
                leftPadding: 14
                rightPadding: 28
                anchors.verticalCenter: parent.verticalCenter
                spacing: 1

                Text {
                    width: parent.width - parent.leftPadding - parent.rightPadding
                    text: {
                        var idx = pill.currentIndex
                        if (idx >= 0 && idx < root.languages.length)
                            return root.languages[idx].nativeName || root.languages[idx].localizedName
                        return ""
                    }
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    color: tok.ink
                    elide: Text.ElideRight
                }

                Text {
                    width: parent.width - parent.leftPadding - parent.rightPadding
                    visible: {
                        var idx = pill.currentIndex
                        return idx >= 0 && idx < root.languages.length &&
                               root.languages[idx].nativeName !== "" &&
                               root.languages[idx].localizedName !== root.languages[idx].nativeName
                    }
                    text: {
                        var idx = pill.currentIndex
                        if (idx >= 0 && idx < root.languages.length)
                            return root.languages[idx].localizedName
                        return ""
                    }
                    font.pixelSize: 10
                    color: tok.mute
                    elide: Text.ElideRight
                }
            }

            // ── Chevron ──────────────────────────────────────────────────────
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

            // ── Popup ────────────────────────────────────────────────────────
            popup: Popup {
                y: pill.height + 2
                width: Math.max(pill.width, 260)
                height: Math.min(listView.contentHeight + 2, 320)
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
                    model: pill.popup.visible ? pill.delegateModel : null
                    currentIndex: pill.highlightedIndex
                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                }
            }

            // ── Delegate — two lines per language ────────────────────────────
            delegate: ItemDelegate {
                required property var model
                required property int index

                width: pill.popup.width - 2
                implicitHeight: 52
                highlighted: pill.highlightedIndex === index

                background: Rectangle {
                    color: parent.highlighted ? tok.sageSoft : "transparent"
                    radius: 6
                }

                contentItem: Column {
                    anchors { left: parent.left; right: parent.right; leftMargin: 14; rightMargin: 14 }
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 1

                    Text {
                        width: parent.width
                        text:  parent.parent.model.nativeName !== ""
                               ? parent.parent.model.nativeName
                               : parent.parent.model.localizedName
                        font.pixelSize: 13
                        font.weight: Font.Medium
                        color: tok.ink
                        elide: Text.ElideRight
                    }

                    Text {
                        width: parent.width
                        visible: parent.parent.model.nativeName !== "" &&
                                 parent.parent.model.localizedName !== parent.parent.model.nativeName
                        text:  parent.parent.model.localizedName
                        font.pixelSize: 11
                        color: tok.mute
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }
}
