import QtQuick

QtObject {
    property bool   enabled:               true
    property string limitDisplay:          "8:00"
    property double limitNorm:             0.42
    property string snoozeDisplay:         "3:00"
    property double snoozeNorm:            0.22
    property bool   useMicroBreakActivity: false
    property bool   showPostpone:          true
    property bool   showSkip:              false
    property bool   preludeEnabled:        true
    property bool   hasMaxPreludes:        true
    property int    maxPreludes:           3

    function setEnabled(v) {}
    function incrementLimit() {}
    function decrementLimit() {}
    function setLimitNorm(v) {}
    function incrementSnooze() {}
    function decrementSnooze() {}
    function setSnoozeNorm(v) {}
    function setUseMicroBreakActivity(v) {}
    function setShowPostpone(v) {}
    function setShowSkip(v) {}
    function setPreludeEnabled(v) {}
    function setHasMaxPreludes(v) {}
    function incrementMaxPreludes() {}
    function decrementMaxPreludes() {}
}
