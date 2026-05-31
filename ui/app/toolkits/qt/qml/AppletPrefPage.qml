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
                        spacing: tok.labelHintGap

                        Text {
                            text: qsTr("Timer layout")
                            font.pixelSize: tok.labelPx; font.weight: Font.Medium
                            color: tok.ink
                        }
                        Text {
                            width: parent.width
                            text: qsTr("Three ways to render the three timers. Pick whichever reads best at a glance.")
                            font.pixelSize: tok.hintPx; color: tok.mute
                            wrapMode: Text.WordWrap; lineHeight: tok.hintLineH
                        }
                    }

                    Row {
                        width: parent.width
                        spacing: 10

                        TimerStyleCard {
                            width: (parent.width - 30) / 4
                            active: root.bridge ? root.bridge.displayStyle === 0 : true
                            cardTitle: qsTr("A · Rings")
                            cardSub:   qsTr("Three circular rings. Closest to the classic look.")
                            kind: "rings"
                            onClicked: { if (root.bridge) root.bridge.setDisplayStyle(0) }
                        }

                        TimerStyleCard {
                            width: (parent.width - 30) / 4
                            active: root.bridge ? root.bridge.displayStyle === 1 : false
                            cardTitle: qsTr("B · Bars")
                            cardSub:   qsTr("Icon + tiny progress bar + timecode. Compact and scannable.")
                            kind: "bars"
                            onClicked: { if (root.bridge) root.bridge.setDisplayStyle(1) }
                        }

                        TimerStyleCard {
                            width: (parent.width - 30) / 4
                            active: root.bridge ? root.bridge.displayStyle === 2 : false
                            cardTitle: qsTr("C · Focus")
                            cardSub:   qsTr("Big ring on the active timer; the others are chips alongside.")
                            kind: "focus"
                            onClicked: { if (root.bridge) root.bridge.setDisplayStyle(2) }
                        }

                        TimerStyleCard {
                            width: (parent.width - 30) / 4
                            active: root.bridge ? root.bridge.displayStyle === 3 : false
                            cardTitle: qsTr("D · Classic")
                            cardSub:   qsTr("The original Workrave widget — text timers, no transparency.")
                            kind: "classic"
                            onClicked: { if (root.bridge) root.bridge.setDisplayStyle(3) }
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
                        spacing: tok.labelHintGap

                        Text {
                            text: qsTr("Placement")
                            font.pixelSize: tok.labelPx; font.weight: Font.Medium
                            color: tok.ink
                        }
                        Text {
                            width: parent.width
                            text: qsTr("Where each timer appears. Pair timers into the same slot to save space — Workrave will alternate between them.")
                            font.pixelSize: tok.hintPx; color: tok.mute
                            wrapMode: Text.WordWrap; lineHeight: tok.hintLineH
                        }
                    }

                    Row {
                        width: parent.width
                        spacing: 10

                        TimerPlacementCard {
                            width: (parent.width - 30) / 4
                            active: root.bridge ? root.bridge.placement === 0 : true
                            cardTitle: qsTr("Next to each other")
                            pattern: [["M"], ["R"], ["D"]]
                            onClicked: { if (root.bridge) root.bridge.setPlacement(0) }
                        }

                        TimerPlacementCard {
                            width: (parent.width - 30) / 4
                            active: root.bridge ? root.bridge.placement === 1 : false
                            cardTitle: qsTr("Micro + Rest together")
                            pattern: [["M","R"], ["D"]]
                            onClicked: { if (root.bridge) root.bridge.setPlacement(1) }
                        }

                        TimerPlacementCard {
                            width: (parent.width - 30) / 4
                            active: root.bridge ? root.bridge.placement === 2 : false
                            cardTitle: qsTr("Rest + Daily together")
                            pattern: [["M"], ["R","D"]]
                            onClicked: { if (root.bridge) root.bridge.setPlacement(2) }
                        }

                        TimerPlacementCard {
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
                onCommitted:    (secs) => { if (root.bridge) root.bridge.setCycleSeconds(secs) }
            }
        }

        // ── Per-timer visibility ──────────────────────────────────────────────
        PrefGroup {
            width: parent.width
            title: qsTr("Per-timer visibility")

            TimerVisibilityRow {
                width: parent.width
                timerName:  qsTr("Microbreak")
                timerColor: tok.sage
                value:      root.bridge ? root.bridge.microVisibility : 0
                onSelected: (v) => { if (root.bridge) root.bridge.setMicroVisibility(v) }
            }

            TimerVisibilityRow {
                width: parent.width
                timerName:  qsTr("Rest break")
                timerColor: tok.clay
                value:      root.bridge ? root.bridge.restVisibility : 0
                onSelected: (v) => { if (root.bridge) root.bridge.setRestVisibility(v) }
            }

            TimerVisibilityRow {
                width: parent.width
                timerName:  qsTr("Daily limit")
                timerColor: tok.sageDeep
                value:      root.bridge ? root.bridge.dailyVisibility : 0
                isLast:     true
                onSelected: (v) => { if (root.bridge) root.bridge.setDailyVisibility(v) }
            }
        }
    }
}
