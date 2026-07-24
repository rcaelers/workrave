import QtQuick

QtObject {
    property bool enabled: true
    property bool hasVolume: true
    property int  volume: 80
    property bool hasMute: true
    property bool mute: false
    property string currentThemeId: "default"

    property var themes: [
        { id: "default",  name: "Default" },
        { id: "classic",  name: "Classic" },
    ]

    property QtObject ev1:  QtObject { property string id: "break-prelude";     property string name: "Break prelude";       property bool enabled: true;  property string filename: "" }
    property QtObject ev2:  QtObject { property string id: "break-ignored";     property string name: "Break ignored";       property bool enabled: true;  property string filename: "" }
    property QtObject ev3:  QtObject { property string id: "micro-break-started"; property string name: "Microbreak started"; property bool enabled: true;  property string filename: "" }
    property QtObject ev4:  QtObject { property string id: "micro-break-ended"; property string name: "Microbreak ended";    property bool enabled: true;  property string filename: "" }
    property QtObject ev5:  QtObject { property string id: "rest-break-started"; property string name: "Rest break started"; property bool enabled: true;  property string filename: "" }
    property QtObject ev6:  QtObject { property string id: "rest-break-ended";  property string name: "Rest break ended";   property bool enabled: true;  property string filename: "/custom/sound.wav" }
    property QtObject ev7:  QtObject { property string id: "daily-limit";       property string name: "Daily limit";         property bool enabled: false; property string filename: "" }
    property QtObject ev8:  QtObject { property string id: "exercise-ended";    property string name: "Exercise ended";      property bool enabled: true;  property string filename: "" }
    property QtObject ev9:  QtObject { property string id: "exercises-ended";   property string name: "Exercises ended";     property bool enabled: true;  property string filename: "" }
    property QtObject ev10: QtObject { property string id: "exercise-step";     property string name: "Exercise step";       property bool enabled: false; property string filename: "" }

    property var events: [ev1, ev2, ev3, ev4, ev5, ev6, ev7, ev8, ev9, ev10]

    function setEnabled(v)                      { console.log("setEnabled", v) }
    function setVolume(v)                       { console.log("setVolume", v) }
    function setMute(v)                         { console.log("setMute", v) }
    function setTheme(id)                       { console.log("setTheme", id) }
    function setEventEnabled(id, v)             { console.log("setEventEnabled", id, v) }
    function pickEventFile(id)                  { console.log("pickEventFile", id) }
    function playEvent(id)                      { console.log("playEvent", id) }
    function clearEventFile(id)                 { console.log("clearEventFile", id) }
}
