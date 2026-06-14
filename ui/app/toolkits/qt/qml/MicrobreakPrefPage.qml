import QtQuick

// MicrobreakPrefPage — content for Timers > Microbreak preferences.
// Expects context property: microbreakPrefBridge (MicrobreakPrefBridge)
Item {
    id: root

    property var bridge: typeof microbreakPrefBridge !== "undefined" ? microbreakPrefBridge : null

    implicitWidth:  parent ? parent.width : 500
    implicitHeight: col.implicitHeight

    Column {
        id: col
        anchors { left: parent.left; right: parent.right; top: parent.top }
        spacing: 24

        PrefToggleRow {
            width: parent.width
            label: qsTr("Enable timer")
            hint:  qsTr("When off, no microbreak prompts will be shown.")
            checked: root.bridge ? root.bridge.enabled : true
            onToggled: (v) => { if (root.bridge) root.bridge.setEnabled(v) }
        }

        PrefGroup {
            width: parent.width
            title: qsTr("Timing")

            PrefTimeControl {
                width: parent.width
                label: qsTr("Time between breaks")
                hint:  qsTr("How long you've been using the keyboard or mouse before being prompted.")
                value: root.bridge ? root.bridge.limitDisplay : "3:00"
                sliderValue: root.bridge ? root.bridge.limitNorm : 0.3
                sliderColor: "#6B8068"
                secondsStep: 5
                ticks: [
                    { at: 0.000, label: "1m" },
                    { at: 0.111, label: "2m" },
                    { at: 0.222, label: "3m" },
                    { at: 0.444, label: "5m" },
                    { at: 1.000, label: "10m" },
                ]
                onIncrement:   { if (root.bridge) root.bridge.incrementLimit() }
                onDecrement:   { if (root.bridge) root.bridge.decrementLimit() }
                onSliderMoved: (v)    => { if (root.bridge) root.bridge.setLimitNorm(v) }
                onCommitted:   (secs) => { if (root.bridge) root.bridge.setLimitSeconds(secs) }
            }

            PrefTimeControl {
                width: parent.width
                label: qsTr("Break duration")
                hint:  qsTr("How long each microbreak lasts.")
                value: root.bridge ? root.bridge.durationDisplay : "0:30"
                sliderValue: root.bridge ? root.bridge.durationNorm : 0.143
                sliderColor: "#6B8068"
                secondsStep: 5
                ticks: [
                    { at: 0.000, label: "15s" },
                    { at: 0.143, label: "30s" },
                    { at: 0.429, label: "1m"  },
                    { at: 1.000, label: "2m"  },
                ]
                onIncrement: { if (root.bridge) root.bridge.incrementDuration() }
                onDecrement: { if (root.bridge) root.bridge.decrementDuration() }
                onSliderMoved: (v) => { if (root.bridge) root.bridge.setDurationNorm(v) }
                onCommitted:   (secs) => { if (root.bridge) root.bridge.setDurationSeconds(secs) }
            }

            PrefTimeControl {
                width: parent.width
                label: qsTr("Postpone time")
                hint:  qsTr("When postponed, Workrave reminds you again after this long.")
                value: root.bridge ? root.bridge.snoozeDisplay : "5:00"
                sliderValue: root.bridge ? root.bridge.snoozeNorm : 0.444
                sliderColor: "#6B8068"
                ticks: [
                    { at: 0.000, label: "1m"  },
                    { at: 0.444, label: "5m"  },
                    { at: 1.000, label: "10m" },
                ]
                onIncrement: { if (root.bridge) root.bridge.incrementSnooze() }
                onDecrement: { if (root.bridge) root.bridge.decrementSnooze() }
                onSliderMoved: (v) => { if (root.bridge) root.bridge.setSnoozeNorm(v) }
                onCommitted:   (secs) => { if (root.bridge) root.bridge.setSnoozeSeconds(secs) }
            }
        }

        PrefGroup {
            width: parent.width
            title: qsTr("Break prompting")

            PrefToggleRow {
                width: parent.width
                label: qsTr("Prompt before breaking")
                hint:  qsTr("Show a gentle reminder a few seconds before the break starts.")
                checked: root.bridge ? root.bridge.preludeEnabled : true
                onToggled: (v) => { if (root.bridge) root.bridge.setPreludeEnabled(v) }
            }

            PrefToggleRow {
                width: parent.width
                visible: root.bridge ? root.bridge.preludeEnabled : true
                label:   qsTr("Limit number of prompts")
                hint:    qsTr("When off, Workrave keeps reminding you until the break starts.")
                checked: root.bridge ? root.bridge.hasMaxPreludes : false
                onToggled: (v) => { if (root.bridge) root.bridge.setHasMaxPreludes(v) }
            }

            PrefSpinRow {
                width: parent.width
                visible: root.bridge ? (root.bridge.preludeEnabled && root.bridge.hasMaxPreludes) : false
                label:   qsTr("Maximum number of prompts")
                display: root.bridge ? root.bridge.maxPreludes.toString() : "3"
                narrow: true
                onIncrement: { if (root.bridge) root.bridge.incrementMaxPreludes() }
                onDecrement: { if (root.bridge) root.bridge.decrementMaxPreludes() }
            }
        }

        PrefGroup {
            width: parent.width
            title: qsTr("Options")

            PrefToggleRow {
                width: parent.width
                label: qsTr("Show 'Postpone' button")
                hint:  qsTr("Whether the postpone button appears on the break window.")
                checked: root.bridge ? root.bridge.showPostpone : true
                onToggled: (v) => { if (root.bridge) root.bridge.setShowPostpone(v) }
            }

            PrefToggleRow {
                width: parent.width
                label: qsTr("Show 'Skip' button")
                hint:  qsTr("Whether the skip button appears on the break window.")
                checked: root.bridge ? root.bridge.showSkip : true
                onToggled: (v) => { if (root.bridge) root.bridge.setShowSkip(v) }
            }
        }
    }
}
