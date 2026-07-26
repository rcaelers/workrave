import QtQuick

QtObject {
    property bool   enabled:         true
    property bool   alwaysOnTop:     false
    property int    displayStyle:    0
    property int    placement:       0
    property string cycleDisplay:    "5"
    property double cycleNorm:       0.23
    property int    microVisibility: 0
    property int    restVisibility:  0
    property int    dailyVisibility: 0

    function setEnabled(v) {}
    function setAlwaysOnTop(v) {}
    function setDisplayStyle(v) {}
    function setPlacement(v) {}
    function incrementCycle() {}
    function decrementCycle() {}
    function setCycleNorm(v) {}
    function setMicroVisibility(v) {}
    function setRestVisibility(v) {}
    function setDailyVisibility(v) {}
}
