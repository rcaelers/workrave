import QtQuick

// RestBreakPrefPage — content for Timers > Rest break preferences.
// Expects context property: restBreakPrefBridge (RestBreakPrefBridge)
Item {
    id: root

    property var bridge: typeof restBreakPrefBridge !== "undefined" ? restBreakPrefBridge : null

    implicitWidth:  parent ? parent.width : 500
    implicitHeight: col.implicitHeight

    Column {
        id: col
        anchors { left: parent.left; right: parent.right; top: parent.top }
        spacing: 24

        PrefToggleRow {
            width: parent.width
            label: qsTr("Enable timer")
            hint:  qsTr("When off, no rest break prompts will be shown.")
            checked: root.bridge ? root.bridge.enabled : true
            onToggled: (v) => { if (root.bridge) root.bridge.setEnabled(v) }
        }

        PrefGroup {
            width: parent.width
            title: qsTr("Timing")

            PrefTimeControl {
                width: parent.width
                label: qsTr("Time between breaks")
                hint:  qsTr("Active time required before a rest break is prompted.")
                value: root.bridge ? root.bridge.limitDisplay : "45:00"
                sliderValue: root.bridge ? root.bridge.limitNorm : 0.5
                sliderColor: "#6B8068"
                ticks: [
                    { at: 0.000, label: "15m" },
                    { at: 0.333, label: "30m" },
                    { at: 0.500, label: "45m" },
                    { at: 0.667, label: "60m" },
                    { at: 1.000, label: "90m" },
                ]
                onIncrement: { if (root.bridge) root.bridge.incrementLimit() }
                onDecrement: { if (root.bridge) root.bridge.decrementLimit() }
                onSliderMoved: (v) => { if (root.bridge) root.bridge.setLimitNorm(v) }
                onCommitted:   (secs) => { if (root.bridge) root.bridge.setLimitSeconds(secs) }
            }

            PrefTimeControl {
                width: parent.width
                label: qsTr("Break duration")
                hint:  qsTr("How long each rest break lasts.")
                value: root.bridge ? root.bridge.durationDisplay : "10:00"
                sliderValue: root.bridge ? root.bridge.durationNorm : 0.444
                sliderColor: "#6B8068"
                ticks: [
                    { at: 0.000, label: "5m"  },
                    { at: 0.444, label: "10m" },
                    { at: 0.667, label: "15m" },
                    { at: 1.000, label: "20m" },
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
                value: root.bridge ? root.bridge.snoozeDisplay : "3:00"
                sliderValue: root.bridge ? root.bridge.snoozeNorm : 0.222
                sliderColor: "#6B8068"
                ticks: [
                    { at: 0.000, label: "1m"  },
                    { at: 0.222, label: "3m"  },
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
                narrow:  true
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

            PrefToggleRow {
                width: parent.width
                label: qsTr("Start restbreak when screen is locked")
                hint:  qsTr("Counts time the screen is locked as part of your break. Prevents being prompted right after you unlock.")
                checked: root.bridge ? root.bridge.autoNatural : true
                onToggled: (v) => { if (root.bridge) root.bridge.setAutoNatural(v) }
            }

            PrefToggleRow {
                width: parent.width
                label: qsTr("Enable shutting down the computer from the rest screen")
                hint:  qsTr("Adds a power button to the rest break window so you can stop work and shut down or sleep the computer.")
                checked: root.bridge ? root.bridge.enableShutdown : false
                onToggled: (v) => { if (root.bridge) root.bridge.setEnableShutdown(v) }
            }

            PrefTextRow {
                width: parent.width
                visible: root.bridge ? root.bridge.enableShutdown : false
                label:   qsTr("Custom lock command")
                hint:    qsTr("Shell command for locking the screen. When set, appears as 'Custom command' in the lock method list.")
                value:   root.bridge ? root.bridge.customLockCommand : ""
                placeholder: qsTr("e.g. /usr/bin/pmset displaysleepnow")
                onCommitted: (text) => { if (root.bridge) root.bridge.setCustomLockCommand(text) }
            }

            PrefChoiceRow {
                width: parent.width
                visible: (root.bridge ? root.bridge.enableShutdown : false) && (root.bridge ? root.bridge.hasLockMethods : false)
                label:   qsTr("Lock method")
                hint:    qsTr("Which screen-lock backend to use when the Lock button is pressed.")
                options: root.bridge ? root.bridge.lockMethodOptions : []
                currentIndex: root.bridge ? root.bridge.lockMethodIndex : 0
                onSelected: (idx) => { if (root.bridge) root.bridge.setLockMethodIndex(idx) }
            }

            PrefChoiceRow {
                width: parent.width
                visible: (root.bridge ? root.bridge.enableShutdown : false) && (root.bridge ? root.bridge.hasSleepOperations : false)
                label:   qsTr("Sleep action")
                hint:    qsTr("What the Sleep button does — suspend, hibernate, or hybrid sleep.")
                options: root.bridge ? root.bridge.sleepOperationOptions : []
                currentIndex: root.bridge ? root.bridge.sleepOperationIndex : 0
                onSelected: (idx) => { if (root.bridge) root.bridge.setSleepOperationIndex(idx) }
            }
        }

        PrefGroup {
            width: parent.width
            title: qsTr("Exercises")

            PrefSpinRow {
                width: parent.width
                label: qsTr("Number of exercises")
                hint:  qsTr("Workrave picks this many at random from its library each rest break.")
                display: root.bridge ? root.bridge.exercises.toString() : "4"
                narrow: true
                onIncrement: { if (root.bridge) root.bridge.incrementExercises() }
                onDecrement: { if (root.bridge) root.bridge.decrementExercises() }
            }
        }
    }
}
