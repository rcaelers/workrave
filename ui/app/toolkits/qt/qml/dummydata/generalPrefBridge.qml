import QtQuick

QtObject {
    property int  blockMode:          1
    property bool sanctuaryEnabled:   true
    property bool trayIconEnabled:    true
    property bool autostartEnabled:   false

    property bool hasDarkMode: true
    property int  darkMode:    2        // 0=Light, 1=Dark, 2=Auto

    property var    languages: [
        { id: "",      localizedName: "System default", nativeName: "" },
        { id: "de",    localizedName: "German",         nativeName: "Deutsch" },
        { id: "en",    localizedName: "English",        nativeName: "English" },
        { id: "fr",    localizedName: "French",         nativeName: "français" },
        { id: "ja",    localizedName: "Japanese",       nativeName: "日本語" },
        { id: "nl",    localizedName: "Dutch",          nativeName: "Nederlands" },
        { id: "zh_CN", localizedName: "Chinese",        nativeName: "中文（简体）" },
    ]
    property string currentLanguage: ""

    // Unix — set hasForceX11/hasGnomeShellPreludes: true to preview those rows
    property bool hasForceX11:           false
    property bool forceX11:              false
    property bool hasGnomeShellPreludes: false
    property bool gnomeShellPreludes:    false

    // Icon theme — populate to preview the picker
    property var    iconThemes:       []
    property string currentIconTheme: ""

    function setBlockMode(v)          { console.log("setBlockMode", v) }
    function setSanctuaryEnabled(v)   { console.log("setSanctuaryEnabled", v) }
    function setTrayIconEnabled(v)    { console.log("setTrayIconEnabled", v) }
    function setAutostartEnabled(v)   { console.log("setAutostartEnabled", v) }
    function setDarkMode(v)           { console.log("setDarkMode", v) }
    function setLanguage(id)          { console.log("setLanguage", id) }
    function setForceX11(v)           { console.log("setForceX11", v) }
    function setGnomeShellPreludes(v) { console.log("setGnomeShellPreludes", v) }
    function setIconTheme(id)         { console.log("setIconTheme", id) }
}
