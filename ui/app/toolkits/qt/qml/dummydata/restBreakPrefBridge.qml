import QtQuick

QtObject {
    property bool   enabled:         true
    property string limitDisplay:    "45:00"
    property double limitNorm:       0.44
    property string durationDisplay: "10:00"
    property double durationNorm:    0.35
    property string snoozeDisplay:   "2:00"
    property double snoozeNorm:      0.18
    property int    exercises:       3
    property bool   autoNatural:     true
    property bool   enableShutdown:  false
    property bool   showPostpone:    true
    property bool   showSkip:        false
    property bool   preludeEnabled:  true
    property bool   hasMaxPreludes:  true
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
    function incrementExercises() {}
    function decrementExercises() {}
    function setAutoNatural(v) {}
    function setEnableShutdown(v) {}
    function setShowPostpone(v) {}
    function setShowSkip(v) {}
    function setPreludeEnabled(v) {}
    function setHasMaxPreludes(v) {}
    function incrementMaxPreludes() {}
    function decrementMaxPreludes() {}
}
