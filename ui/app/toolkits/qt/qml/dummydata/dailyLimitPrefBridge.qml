import QtQuick

QtObject {
    property bool   enabled:                true
    property string limitDisplay:           "8:00"
    property double limitNorm:              0.42
    property bool   useMicroBreakActivity: false

    function setEnabled(v) {}
    function incrementLimit() {}
    function decrementLimit() {}
    function setLimitNorm(v) {}
    function setUseMicroBreakActivity(v) {}
}
