import QtQuick

QtObject {
    // Windows — set hasAlternateMonitor: true to preview those rows
    property bool hasAlternateMonitor: false
    property bool alternateMonitor:    false
    property int  sensitivity:         3

    function setAlternateMonitor(v) { console.log("setAlternateMonitor", v) }
    function setSensitivity(v)      { console.log("setSensitivity", v) }
    function openDebugWindow()      { console.log("openDebugWindow") }
}
