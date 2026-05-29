import QtQuick

// AppletPrefPage — content for User interface > Status applet preferences.
// Expects context property: appletPrefBridge (AppletPrefBridge)
// displayStyle values: 0=Rings, 1=Bars, 2=Focus
// placement values: 0=separate, 1=M+R together, 2=R+D together, 3=all-in-one
// visibility values: 0=show, 1=first-due, 2=hide
Item {
    id: root

    property var bridge: typeof appletPrefBridge !== "undefined" ? appletPrefBridge : null

    implicitWidth:  parent ? parent.width : 500
    implicitHeight: col.implicitHeight

    PrefTokens { id: tok }

    Column {
        id: col
        anchors { left: parent.left; right: parent.right; top: parent.top }
        spacing: 24

        // ── Visibility ────────────────────────────────────────────────────────
        PrefGroup {
            width: parent.width
            title: qsTr("Visibility")

            PrefToggleRow {
                width: parent.width
                label: qsTr("Show status applet")
                hint:  qsTr("Turn off to hide the panel applet — Workrave still runs from the tray.")
                checked: root.bridge ? root.bridge.enabled : true
                onToggled: (v) => { if (root.bridge) root.bridge.setEnabled(v) }
            }
        }

        // ── Display style ─────────────────────────────────────────────────────
        PrefGroup {
            width: parent.width
            title: qsTr("Display style")

            Item {
                width: parent.width
                height: styleContent.implicitHeight + 28

                Rectangle {
                    anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
                    height: 1; color: tok.edge2
                }

                Column {
                    id: styleContent
                    anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 14 }
                    spacing: 14

                    Column {
                        width: parent.width
                        spacing: 3

                        Text {
                            text: qsTr("Timer layout")
                            font.pixelSize: 14; font.weight: Font.Medium
                            color: tok.ink
                        }
                        Text {
                            width: parent.width
                            text: qsTr("Three ways to render the three timers. Pick whichever reads best at a glance.")
                            font.pixelSize: 12; color: tok.mute
                            wrapMode: Text.WordWrap; lineHeight: 1.45
                        }
                    }

                    Row {
                        width: parent.width
                        spacing: 12

                        StyleCard {
                            width: (parent.width - 24) / 3
                            active: root.bridge ? root.bridge.displayStyle === 0 : true
                            cardTitle: qsTr("A · Rings")
                            cardSub:   qsTr("Three circular rings. Closest to the classic look.")
                            kind: "rings"
                            onClicked: { if (root.bridge) root.bridge.setDisplayStyle(0) }
                        }

                        StyleCard {
                            width: (parent.width - 24) / 3
                            active: root.bridge ? root.bridge.displayStyle === 1 : false
                            cardTitle: qsTr("B · Bars")
                            cardSub:   qsTr("Icon + tiny progress bar + timecode. Compact and scannable.")
                            kind: "bars"
                            onClicked: { if (root.bridge) root.bridge.setDisplayStyle(1) }
                        }

                        StyleCard {
                            width: (parent.width - 24) / 3
                            active: root.bridge ? root.bridge.displayStyle === 2 : false
                            cardTitle: qsTr("C · Focus")
                            cardSub:   qsTr("Big ring on the active timer; the others are chips alongside.")
                            kind: "focus"
                            onClicked: { if (root.bridge) root.bridge.setDisplayStyle(2) }
                        }
                    }
                }
            }
        }

        // ── Layout ────────────────────────────────────────────────────────────
        PrefGroup {
            width: parent.width
            title: qsTr("Layout")

            // Placement picker
            Item {
                width: parent.width
                height: placementContent.implicitHeight + 28

                Rectangle {
                    anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
                    height: 1; color: tok.edge2
                }

                Column {
                    id: placementContent
                    anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 14 }
                    spacing: 14

                    Column {
                        width: parent.width
                        spacing: 3

                        Text {
                            text: qsTr("Placement")
                            font.pixelSize: 14; font.weight: Font.Medium
                            color: tok.ink
                        }
                        Text {
                            width: parent.width
                            text: qsTr("Where each timer appears. Pair timers into the same slot to save space — Workrave will alternate between them.")
                            font.pixelSize: 12; color: tok.mute
                            wrapMode: Text.WordWrap; lineHeight: 1.45
                        }
                    }

                    Row {
                        width: parent.width
                        spacing: 10

                        PlacementCard {
                            width: (parent.width - 30) / 4
                            active: root.bridge ? root.bridge.placement === 0 : true
                            cardTitle: qsTr("Next to each other")
                            pattern: [["M"], ["R"], ["D"]]
                            onClicked: { if (root.bridge) root.bridge.setPlacement(0) }
                        }

                        PlacementCard {
                            width: (parent.width - 30) / 4
                            active: root.bridge ? root.bridge.placement === 1 : false
                            cardTitle: qsTr("Micro + Rest together")
                            pattern: [["M","R"], ["D"]]
                            onClicked: { if (root.bridge) root.bridge.setPlacement(1) }
                        }

                        PlacementCard {
                            width: (parent.width - 30) / 4
                            active: root.bridge ? root.bridge.placement === 2 : false
                            cardTitle: qsTr("Rest + Daily together")
                            pattern: [["M"], ["R","D"]]
                            onClicked: { if (root.bridge) root.bridge.setPlacement(2) }
                        }

                        PlacementCard {
                            width: (parent.width - 30) / 4
                            active: root.bridge ? root.bridge.placement === 3 : false
                            cardTitle: qsTr("All in one slot")
                            pattern: [["M","R","D"]]
                            onClicked: { if (root.bridge) root.bridge.setPlacement(3) }
                        }
                    }
                }
            }

            // Cycle time — only meaningful when timers share a slot
            PrefTimeControl {
                width: parent.width
                visible: root.bridge ? root.bridge.placement !== 0 : false
                label:       qsTr("Cycle time")
                hint:        qsTr("When timers share a slot, switch between them this often.")
                value:       root.bridge ? root.bridge.cycleDisplay : "0:10"
                sliderValue: root.bridge ? root.bridge.cycleNorm    : 0.286
                sliderColor: tok.sage
                ticks: [
                    { at: 0.000, label: "2s"  },
                    { at: 0.286, label: "10s" },
                    { at: 1.000, label: "30s" },
                ]
                onIncrement:    { if (root.bridge) root.bridge.incrementCycle() }
                onDecrement:    { if (root.bridge) root.bridge.decrementCycle() }
                onSliderMoved:  (v) => { if (root.bridge) root.bridge.setCycleNorm(v) }
            }
        }

        // ── Per-timer visibility ──────────────────────────────────────────────
        PrefGroup {
            width: parent.width
            title: qsTr("Per-timer visibility")

            PerTimerRow {
                width: parent.width
                timerName:  qsTr("Microbreak")
                timerColor: tok.sage
                value:      root.bridge ? root.bridge.microVisibility : 0
                onSelected: (v) => { if (root.bridge) root.bridge.setMicroVisibility(v) }
            }

            PerTimerRow {
                width: parent.width
                timerName:  qsTr("Rest break")
                timerColor: "#C97B4A"
                value:      root.bridge ? root.bridge.restVisibility : 0
                onSelected: (v) => { if (root.bridge) root.bridge.setRestVisibility(v) }
            }

            PerTimerRow {
                width: parent.width
                timerName:  qsTr("Daily limit")
                timerColor: tok.sageDeep
                value:      root.bridge ? root.bridge.dailyVisibility : 0
                isLast:     true
                onSelected: (v) => { if (root.bridge) root.bridge.setDailyVisibility(v) }
            }
        }
    }

    // ── StyleCard ─────────────────────────────────────────────────────────────
    component StyleCard: Item {
        id: card

        property bool   active:    false
        property string cardTitle: ""
        property string cardSub:   ""
        property string kind:      "rings"

        signal clicked()

        implicitHeight: cardCol.implicitHeight + 26

        Rectangle {
            anchors.fill: parent; radius: 12
            color:        card.active ? tok.sageSoft : tok.panel
            border.color: card.active ? tok.sage : tok.edge
            border.width: card.active ? 2 : 1
        }

        Column {
            id: cardCol
            anchors {
                left: parent.left; right: parent.right; top: parent.top
                leftMargin: 12; rightMargin: 12; topMargin: 12
            }
            spacing: 8

            // Preview area
            Item {
                width: parent.width; height: 110
                clip: true

                Rectangle {
                    anchors.fill: parent; radius: 8
                    color: card.active ? Qt.rgba(1, 1, 1, 0.55) : tok.bg
                    border.color: tok.edge; border.width: 1
                }

                StylePreview {
                    anchors.centerIn: parent
                    kind: card.kind
                }
            }

            // Radio dot + title
            Row {
                spacing: 6

                Item {
                    width: 14; height: 14
                    anchors.verticalCenter: parent.verticalCenter

                    Rectangle {
                        anchors.fill: parent; radius: 7
                        color:        card.active ? tok.sage : tok.panel
                        border.color: card.active ? tok.sage : tok.edge
                        border.width: 1
                    }
                    Rectangle {
                        visible: card.active
                        anchors.centerIn: parent
                        width: 8; height: 8; radius: 4; color: tok.panel
                    }
                    Rectangle {
                        visible: card.active
                        anchors.centerIn: parent
                        width: 4; height: 4; radius: 2; color: tok.sage
                    }
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: card.cardTitle
                    font.pixelSize: 13; font.weight: Font.DemiBold
                    color: tok.ink
                }
            }

            Text {
                width: parent.width
                text: card.cardSub
                font.pixelSize: 12; color: tok.ink2
                wrapMode: Text.WordWrap; lineHeight: 1.45
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: card.clicked()
        }

        Accessible.role: Accessible.RadioButton
        Accessible.name: card.cardTitle
        Accessible.checked: card.active
    }

    // ── StylePreview ──────────────────────────────────────────────────────────
    component StylePreview: Item {
        property string kind: "rings"

        implicitWidth: 140
        implicitHeight: 60

        // Rings: three arcs drawn on Canvas
        Row {
            visible: kind === "rings"
            anchors.centerIn: parent
            spacing: 10

            Repeater {
                model: [
                    { ringColor: "#6B8068", progress: 0.65 },
                    { ringColor: "#C97B4A", progress: 0.30 },
                    { ringColor: "#44563F", progress: 0.85 },
                ]

                Canvas {
                    required property var modelData
                    width: 34; height: 34
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        ctx.lineWidth = 4
                        ctx.strokeStyle = "rgba(40,45,38,0.12)"
                        ctx.beginPath()
                        ctx.arc(17, 17, 13, 0, 2 * Math.PI)
                        ctx.stroke()
                        ctx.strokeStyle = modelData.ringColor
                        ctx.lineCap = "round"
                        ctx.beginPath()
                        ctx.arc(17, 17, 13,
                                -Math.PI / 2,
                                -Math.PI / 2 + 2 * Math.PI * modelData.progress)
                        ctx.stroke()
                    }
                }
            }
        }

        // Bars: three rows of icon + bar + time
        Column {
            visible: kind === "bars"
            anchors.centerIn: parent
            spacing: 9

            Repeater {
                model: [
                    { barColor: "#6B8068", pct: 0.65, time: "3:45"  },
                    { barColor: "#C97B4A", pct: 0.30, time: "12:20" },
                    { barColor: "#44563F", pct: 0.85, time: "1:05"  },
                ]

                Row {
                    required property var modelData
                    spacing: 6

                    Rectangle {
                        width: 10; height: 10; radius: 2
                        color: modelData.barColor
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Item {
                        width: 52; height: 4
                        anchors.verticalCenter: parent.verticalCenter

                        Rectangle {
                            anchors.fill: parent; radius: 2
                            color: Qt.rgba(40/255, 45/255, 38/255, 0.12)
                        }
                        Rectangle {
                            width: parent.width * modelData.pct; height: parent.height
                            radius: 2; color: modelData.barColor
                        }
                    }

                    Text {
                        text: modelData.time
                        font.pixelSize: 8; font.family: "Menlo"
                        color: tok.ink2
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }

        // Focus: large ring for active timer + two small chips
        Row {
            visible: kind === "focus"
            anchors.centerIn: parent
            spacing: 10

            Canvas {
                width: 46; height: 46
                anchors.verticalCenter: parent.verticalCenter
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)
                    ctx.lineWidth = 5
                    ctx.strokeStyle = "rgba(40,45,38,0.12)"
                    ctx.beginPath()
                    ctx.arc(23, 23, 18, 0, 2 * Math.PI)
                    ctx.stroke()
                    ctx.strokeStyle = "#6B8068"
                    ctx.lineCap = "round"
                    ctx.beginPath()
                    ctx.arc(23, 23, 18, -Math.PI / 2, -Math.PI / 2 + 2 * Math.PI * 0.65)
                    ctx.stroke()
                }
            }

            Column {
                spacing: 7
                anchors.verticalCenter: parent.verticalCenter

                Repeater {
                    model: [
                        { chipColor: "#C97B4A", time: "12:20" },
                        { chipColor: "#44563F", time: "1:05"  },
                    ]

                    Rectangle {
                        required property var modelData
                        width: 62; height: 18; radius: 9
                        color: Qt.rgba(1, 1, 1, 0.7)
                        border.color: Qt.rgba(40/255, 45/255, 38/255, 0.08)
                        border.width: 1

                        Row {
                            anchors { left: parent.left; leftMargin: 6; verticalCenter: parent.verticalCenter }
                            spacing: 4

                            Rectangle {
                                width: 6; height: 6; radius: 3
                                color: modelData.chipColor
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: modelData.time
                                font.pixelSize: 7; font.family: "Menlo"
                                color: tok.ink2
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }
            }
        }
    }

    // ── PlacementCard ─────────────────────────────────────────────────────────
    component PlacementCard: Item {
        id: plCard

        property bool   active:    false
        property string cardTitle: ""
        property var    pattern:   []

        signal clicked()

        implicitHeight: plCardCol.implicitHeight + 22

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

            // Mini status window
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
                                    color:   modelData === "M" ? "#6B8068"
                                           : modelData === "R" ? "#C97B4A"
                                           :                     "#44563F"
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
                font.pixelSize: 11; font.weight: Font.Medium
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

    // ── PerTimerRow ───────────────────────────────────────────────────────────
    component PerTimerRow: Item {
        id: ptrRow

        property string timerName:  ""
        property color  timerColor: tok.sage
        property int    value:      0
        property bool   isLast:     false

        signal selected(int v)

        implicitWidth:  parent ? parent.width : 400
        implicitHeight: 52

        Rectangle {
            visible: !ptrRow.isLast
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
            height: 1; color: tok.edge2
        }

        Item {
            id: dotBadge
            anchors { left: parent.left; verticalCenter: parent.verticalCenter }
            width: 28; height: 28

            Rectangle {
                anchors.fill: parent; radius: 8
                color: Qt.rgba(ptrRow.timerColor.r, ptrRow.timerColor.g, ptrRow.timerColor.b, 0.12)
            }
            Rectangle {
                anchors.centerIn: parent
                width: 10; height: 10; radius: 5
                color: ptrRow.timerColor
            }
        }

        Text {
            anchors { left: dotBadge.right; leftMargin: 10; verticalCenter: parent.verticalCenter }
            text: ptrRow.timerName
            font.pixelSize: 14; font.weight: Font.Medium
            color: tok.ink
        }

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
                        { val: 0, label: qsTr("Always show")    },
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
                            color:   ptrRow.value === modelData.val ? tok.sage : "transparent"
                            visible: ptrRow.value === modelData.val
                        }

                        Text {
                            id: segLabel
                            anchors.centerIn: parent
                            text: modelData.label
                            font.pixelSize: 12
                            color:       ptrRow.value === modelData.val ? "#ffffff" : tok.ink2
                            font.weight: ptrRow.value === modelData.val ? Font.DemiBold : Font.Normal
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: ptrRow.selected(modelData.val)
                        }
                    }
                }
            }
        }
    }
}
