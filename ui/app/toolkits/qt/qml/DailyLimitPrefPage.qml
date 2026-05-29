import QtQuick

// DailyLimitPrefPage — content for Timers > Daily limit preferences.
// Expects context property: dailyLimitPrefBridge (DailyLimitPrefBridge)
Item {
    id: root

    property var bridge: typeof dailyLimitPrefBridge !== "undefined" ? dailyLimitPrefBridge : null

    implicitWidth:  parent ? parent.width : 500
    implicitHeight: col.implicitHeight

    Column {
        id: col
        anchors { left: parent.left; right: parent.right; top: parent.top }
        spacing: 24

        PrefToggleRow {
            width: parent.width
            label: qsTr("Enable daily limit")
            hint:  qsTr("When off, no daily limit prompts will be shown.")
            checked: root.bridge ? root.bridge.enabled : true
            onToggled: (v) => { if (root.bridge) root.bridge.setEnabled(v) }
        }

        PrefGroup {
            width: parent.width
            title: qsTr("Timing")

            PrefTimeControl {
                width: parent.width
                label: qsTr("Daily limit")
                hint:  qsTr("When today's active time crosses this, the daily-limit window appears.")
                value: root.bridge ? root.bridge.limitDisplay : "8:00"
                sliderValue: root.bridge ? root.bridge.limitNorm : 0.6
                sliderColor: "#44563F"
                ticks: [
                    { at: 0.000, label: "2h"  },
                    { at: 0.400, label: "6h"  },
                    { at: 0.600, label: "8h"  },
                    { at: 0.800, label: "10h" },
                    { at: 1.000, label: "12h" },
                ]
                onIncrement: { if (root.bridge) root.bridge.incrementLimit() }
                onDecrement: { if (root.bridge) root.bridge.decrementLimit() }
                onSliderMoved: (v) => { if (root.bridge) root.bridge.setLimitNorm(v) }
            }

            PrefTimeControl {
                width: parent.width
                label: qsTr("Postpone time")
                hint:  qsTr("When postponed, Workrave reminds you again after this long.")
                value: root.bridge ? root.bridge.snoozeDisplay : "3:00"
                sliderValue: root.bridge ? root.bridge.snoozeNorm : 0.222
                sliderColor: "#44563F"
                ticks: [
                    { at: 0.000, label: "1m"  },
                    { at: 0.222, label: "3m"  },
                    { at: 0.444, label: "5m"  },
                    { at: 1.000, label: "10m" },
                ]
                onIncrement: { if (root.bridge) root.bridge.incrementSnooze() }
                onDecrement: { if (root.bridge) root.bridge.decrementSnooze() }
                onSliderMoved: (v) => { if (root.bridge) root.bridge.setSnoozeNorm(v) }
            }
        }

        PrefGroup {
            width: parent.width
            title: qsTr("Counting")

            PrefToggleRow {
                width: parent.width
                label: qsTr("Count microbreaks as activity")
                hint:  qsTr("During a microbreak you're usually still seated. Turn on to limit the time spent at the computer; turn off to limit only active use.")
                checked: root.bridge ? root.bridge.useMicroBreakActivity : false
                onToggled: (v) => { if (root.bridge) root.bridge.setUseMicroBreakActivity(v) }
            }
        }

        PrefGroup {
            width: parent.width
            title: qsTr("Break window")

            PrefToggleRow {
                width: parent.width
                label: qsTr("Show 'Postpone' button")
                checked: root.bridge ? root.bridge.showPostpone : true
                onToggled: (v) => { if (root.bridge) root.bridge.setShowPostpone(v) }
            }

            PrefToggleRow {
                width: parent.width
                label: qsTr("Show 'Skip' button")
                checked: root.bridge ? root.bridge.showSkip : true
                onToggled: (v) => { if (root.bridge) root.bridge.setShowSkip(v) }
            }

            PrefToggleRow {
                width: parent.width
                label: qsTr("Prompt before enforcing limit")
                checked: root.bridge ? root.bridge.preludeEnabled : true
                onToggled: (v) => { if (root.bridge) root.bridge.setPreludeEnabled(v) }
            }

            PrefSpinRow {
                width: parent.width
                visible: root.bridge ? root.bridge.preludeEnabled : true
                label:   qsTr("Max prompts before forcing break")
                hint:    qsTr("0 means unlimited — Workrave keeps prompting until you comply.")
                display: root.bridge ? root.bridge.maxPreludes.toString() : "0"
                narrow:  true
                onIncrement: { if (root.bridge) root.bridge.incrementMaxPreludes() }
                onDecrement: { if (root.bridge) root.bridge.decrementMaxPreludes() }
            }
        }
    }
}
