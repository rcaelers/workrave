import QtQuick

QtObject {
    property int  blockMode:        1
    property bool trayIconEnabled:  true
    property bool autostartEnabled: false

    // Windows — set hasDarkMode: true to preview that row
    property bool hasDarkMode: false
    property int  darkMode:    0        // 0=Light, 1=Dark, 2=Auto

    // Unix — set hasForceX11/hasGnomeShellPreludes: true to preview those rows
    property bool hasForceX11:           false
    property bool forceX11:              false
    property bool hasGnomeShellPreludes: false
    property bool gnomeShellPreludes:    false

    // Icon theme — populate to preview the picker
    property var    iconThemes:       []
    property string currentIconTheme: ""

    function setBlockMode(v)          { console.log("setBlockMode", v) }
    function setTrayIconEnabled(v)    { console.log("setTrayIconEnabled", v) }
    function setAutostartEnabled(v)   { console.log("setAutostartEnabled", v) }
    function setDarkMode(v)           { console.log("setDarkMode", v) }
    function setForceX11(v)           { console.log("setForceX11", v) }
    function setGnomeShellPreludes(v) { console.log("setGnomeShellPreludes", v) }
    function setIconTheme(id)         { console.log("setIconTheme", id) }
}
