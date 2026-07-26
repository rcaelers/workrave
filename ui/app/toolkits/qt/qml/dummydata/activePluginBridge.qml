import QtQuick

// Simulates ActivePluginPageBridge with a sample plugin (auto-updater style).
// groups is a list of group objects, each with a label and rows list.
// rows items mirror PrefRowBridge properties.

QtObject {
    id: root

    property string label: "Software updates"

    property var groups: [group1]

    property QtObject group1: QtObject {
        property string label: "Auto update"
        property var rows: [row1, row2, row3, row4, row5]

        property QtObject row1: QtObject {
            property int    kind:         1
            property string label:        "Automatically check for updates"
            property bool   enabled:      true
            property bool   checked:      true
            property string timeDisplay:  ""
            property double timeNorm:     0
            property string spinDisplay:  ""
            property var    options:      []
            property int    currentIndex: 0
            property string entryText:    ""
            function setChecked(v) {}
            function increment() {}
            function decrement() {}
            function setTimeNorm(v) {}
            function setCurrentIndex(v) {}
            function setEntryText(v) {}
        }

        property QtObject row2: QtObject {
            property int    kind:         1
            property string label:        "Get updates as soon as they are available"
            property bool   enabled:      true
            property bool   checked:      false
            property string timeDisplay:  ""
            property double timeNorm:     0
            property string spinDisplay:  ""
            property var    options:      []
            property int    currentIndex: 0
            property string entryText:    ""
            function setChecked(v) {}
            function increment() {}
            function decrement() {}
            function setTimeNorm(v) {}
            function setCurrentIndex(v) {}
            function setEntryText(v) {}
        }

        property QtObject row3: QtObject {
            property int    kind:         4
            property string label:        "Release channel"
            property bool   enabled:      true
            property bool   checked:      false
            property string timeDisplay:  ""
            property double timeNorm:     0
            property string spinDisplay:  ""
            property var    options:      ["Stable", "Release Candidate", "Beta"]
            property int    currentIndex: 0
            property string entryText:    ""
            function setChecked(v) {}
            function increment() {}
            function decrement() {}
            function setTimeNorm(v) {}
            function setCurrentIndex(v) {}
            function setEntryText(v) {}
        }

        property QtObject row4: QtObject {
            property int    kind:         4
            property string label:        "Proxy type"
            property bool   enabled:      true
            property bool   checked:      false
            property string timeDisplay:  ""
            property double timeNorm:     0
            property string spinDisplay:  ""
            property var    options:      ["No proxy", "System proxy", "Custom proxy"]
            property int    currentIndex: 0
            property string entryText:    ""
            function setChecked(v) {}
            function increment() {}
            function decrement() {}
            function setTimeNorm(v) {}
            function setCurrentIndex(v) {}
            function setEntryText(v) {}
        }

        property QtObject row5: QtObject {
            property int    kind:         5
            property string label:        "Proxy"
            property bool   enabled:      false
            property bool   checked:      false
            property string timeDisplay:  ""
            property double timeNorm:     0
            property string spinDisplay:  ""
            property var    options:      []
            property int    currentIndex: 0
            property string entryText:    "http://proxy.example.com:8080"
            function setChecked(v) {}
            function increment() {}
            function decrement() {}
            function setTimeNorm(v) {}
            function setCurrentIndex(v) {}
            function setEntryText(v) {}
        }
    }
}
