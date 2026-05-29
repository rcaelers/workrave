import QtQuick

QtObject {
    property bool   enabled:         true
    property string limitDisplay:    "3:00"
    property double limitNorm:       0.28
    property string durationDisplay: "0:30"
    property double durationNorm:    0.25
    property string snoozeDisplay:   "2:00"
    property double snoozeNorm:      0.18
    property bool   showPostpone:    true
    property bool   showSkip:        false
    property bool   preludeEnabled:  true
    property int    maxPreludes:     3

    function setEnabled(v) {}
    function incrementLimit() {}
    function decrementLimit() {}
    function setLimitNorm(v) {}
    function incrementDuration() {}
    function decrementDuration() {}
    function setDurationNorm(v) {}
    function incrementSnooze() {}
    function decrementSnooze() {}
    function setSnoozeNorm(v) {}
    function setShowPostpone(v) {}
    function setShowSkip(v) {}
    function setPreludeEnabled(v) {}
    function incrementMaxPreludes() {}
    function decrementMaxPreludes() {}
}
