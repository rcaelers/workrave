import QtQuick
import QtQuick.Controls.Basic

// PrefsShell — two-column preferences frame.
// Sidebar (200px) has two-level nav; content area loads page QML via Loader.
// Connect to navigateTo(section, page) to handle nav clicks.
Item {
    id: root

    property string currentSection: "timers"
    property string currentPage:    "microbreak"

    signal navigateTo(string section, string page)
    signal closeRequested()

    readonly property var staticNavModel: [
        { id: "timers", title: "Timers", children: [
            { id: "microbreak",  title: "Microbreak"   },
            { id: "restbreak",   title: "Rest break"   },
            { id: "daily",       title: "Daily limit"  },
            { id: "monitoring",  title: "Monitoring"   },
        ]},
        { id: "ui", title: "User interface", children: [
            { id: "general",     title: "General"        },
            { id: "sounds",      title: "Sounds"         },
            { id: "status",      title: "Status window"  },
            { id: "applet",      title: "Status applet"  },
        ]},
    ]

    // Plugin pages injected by C++ via prefPluginNavEntries context property.
    // Each entry: { id: string, title: string }
    readonly property var pluginEntries: (typeof prefPluginNavEntries !== "undefined") ? prefPluginNavEntries : []

    readonly property var navModel: {
        if (pluginEntries.length === 0)
            return staticNavModel;
        var plugins = [];
        for (var i = 0; i < pluginEntries.length; i++) {
            plugins.push({ id: pluginEntries[i].id, title: pluginEntries[i].title });
        }
        return staticNavModel.concat([{ id: "plugin", title: "Plugins", children: plugins }]);
    }

    PrefTokens { id: tok }

    function pageMetaForRoute(sec, pg) {
        if (sec === "timers") {
            if (pg === "microbreak") return {
                title: qsTr("Microbreak"),
                lede:  qsTr("A short pause to look away, stretch, and reset. Workrave reminds you several times an hour."),
                url:   "MicrobreakPrefPage.qml"
            }
            if (pg === "restbreak") return {
                title: qsTr("Rest break"),
                lede:  qsTr("A longer break with guided exercises. Workrave suggests one every 30–60 minutes of activity."),
                url:   "RestBreakPrefPage.qml"
            }
            if (pg === "daily") return {
                title: qsTr("Daily limit"),
                lede:  qsTr("A cap on how long you'll work at the computer in a day. Workrave nudges you to stop when you reach it."),
                url:   "DailyLimitPrefPage.qml"
            }
            if (pg === "monitoring") return {
                title: qsTr("Monitoring"),
                lede:  qsTr("How Workrave detects your activity — keyboard, mouse, and system idle state."),
                url:   "MonitoringPrefPage.qml"
            }
        }
        if (sec === "ui") {
            if (pg === "general") return {
                title: qsTr("General"),
                lede:  qsTr("How Workrave behaves on your system, and how forcefully it interrupts."),
                url:   "GeneralPrefPage.qml"
            }
            if (pg === "sounds") return {
                title: qsTr("Sounds"),
                lede:  qsTr("Choose which sounds Workrave plays at break events, and adjust the volume."),
                url:   "SoundsPrefPage.qml"
            }
            if (pg === "status") return {
                title: qsTr("Status window"),
                lede:  qsTr("The little always-visible window with your three timers."),
                url:   "StatusWindowPrefPage.qml"
            }
            if (pg === "applet") return {
                title: qsTr("Status applet"),
                lede:  qsTr("The system-tray or panel applet showing your three timers."),
                url:   "AppletPrefPage.qml"
            }
        }
        if (sec === "plugin") {
            var lbl = "";
            for (var i = 0; i < root.pluginEntries.length; i++) {
                if (root.pluginEntries[i].id === pg) { lbl = root.pluginEntries[i].title; break; }
            }
            return { title: lbl, lede: "", url: "PluginPrefPage.qml" }
        }
        return { title: "", lede: "", url: "" }
    }

    readonly property var pageMeta: pageMetaForRoute(currentSection, currentPage)

    // ── Background ───────────────────────────────────────────────────────────
    Rectangle { anchors.fill: parent; color: tok.bg }

    Column {
        anchors.fill: parent

        // ── Title bar ────────────────────────────────────────────────────────
        Rectangle {
            width: parent.width
            height: 38
            color: tok.panel

            Rectangle {
                anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
                height: 1; color: tok.edge
            }

            Row {
                anchors { left: parent.left; leftMargin: 14; verticalCenter: parent.verticalCenter }
                spacing: 10

                Rectangle {
                    width: 7; height: 7; radius: 99; color: tok.sage
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: qsTr("Workrave — Preferences")
                    font.pixelSize: 15
                    font.family: "Georgia"
                    font.italic: true
                    color: tok.ink
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // Close button
            Item {
                id: closeBtn
                anchors { right: parent.right; rightMargin: 12; verticalCenter: parent.verticalCenter }
                width: 22; height: 22

                property bool hovered: false

                Rectangle {
                    anchors.fill: parent
                    radius: 11
                    color: closeBtn.hovered ? tok.edge2 : "transparent"
                }

                Text {
                    anchors.centerIn: parent
                    text: "✕"
                    font.pixelSize: 13
                    color: closeBtn.hovered ? tok.ink : tok.mute
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    onEntered: closeBtn.hovered = true
                    onExited:  closeBtn.hovered = false
                    onClicked: root.closeRequested()
                }
            }
        }

        // ── Body: sidebar + content ──────────────────────────────────────────
        Item {
            id: body
            width: parent.width
            height: parent.height - 38 - 52

            // ── Sidebar ──────────────────────────────────────────────────────
            Rectangle {
                id: sidebar
                width: 200; height: parent.height
                color: tok.panel2
                clip: true

                Rectangle {
                    anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
                    width: 1; color: tok.edge
                }

                Flickable {
                    id: sidebarFlick
                    anchors { fill: parent; topMargin: 14; bottomMargin: 14 }
                    contentWidth: width
                    contentHeight: navCol.implicitHeight
                    clip: true
                    flickableDirection: Flickable.VerticalFlick

                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                    Column {
                        id: navCol
                        width: 184    // sidebar - 8px pad each side
                        x: 8
                        spacing: 0

                        Repeater {
                            model: root.navModel

                            Column {
                                required property var modelData
                                property string secId: modelData.id
                                width: parent.width
                                spacing: 0

                                Text {
                                    width: parent.width
                                    leftPadding: 12; topPadding: 10; bottomPadding: 4
                                    text: modelData.title.toUpperCase()
                                    font.pixelSize: 11
                                    font.weight: Font.DemiBold
                                    font.letterSpacing: 10.5 * 0.16
                                    color: tok.mute
                                }

                                Repeater {
                                    model: modelData.children

                                    delegate: Item {
                                        required property var modelData
                                        required property int index
                                        property string mySection: parent.secId

                                        width: parent.width; height: 32

                                        readonly property bool active:
                                            root.currentSection === mySection
                                            && root.currentPage === modelData.id

                                        Rectangle {
                                            anchors.fill: parent
                                            radius: 6
                                            color: parent.active ? tok.sageSoft : "transparent"
                                        }

                                        Text {
                                            anchors { left: parent.left; leftMargin: 12; verticalCenter: parent.verticalCenter }
                                            text: modelData.title
                                            font.pixelSize: 13
                                            font.weight: parent.active ? Font.DemiBold : Font.Normal
                                            color: parent.active ? tok.sageDeep : tok.ink2
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: root.navigateTo(mySection, modelData.id)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ── Content area ─────────────────────────────────────────────────
            Item {
                id: contentArea
                anchors {
                    left: sidebar.right; right: parent.right
                    top: parent.top;     bottom: parent.bottom
                }
                clip: true

                Flickable {
                    id: contentFlick
                    anchors.fill: parent
                    contentWidth: width
                    contentHeight: contentCol.implicitHeight + 48
                    clip: true
                    flickableDirection: Flickable.VerticalFlick

                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                    Column {
                        id: contentCol
                        width: contentFlick.width - 64
                        x: 32; y: 24
                        spacing: 0

                        Text {
                            width: parent.width
                            text: root.pageMeta.title
                            font.pixelSize: 30
                            font.family: "Georgia"
                            color: tok.ink
                            lineHeight: 1.1
                            wrapMode: Text.WordWrap
                        }

                        Text {
                            visible: root.pageMeta.lede !== ""
                            width: parent.width
                            topPadding: 4
                            text: root.pageMeta.lede
                            font.pixelSize: 13
                            color: tok.mute
                            wrapMode: Text.WordWrap
                            lineHeight: 1.45
                        }

                        Item {
                            width: parent.width
                            height: (pageLoader.item ? pageLoader.item.implicitHeight : 0) + 4

                            Loader {
                                id: pageLoader
                                anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 4 }
                                source: root.pageMeta.url !== ""
                                        ? Qt.resolvedUrl(root.pageMeta.url)
                                        : ""
                            }
                        }
                    }
                }
            }
        }

        // ── Footer ───────────────────────────────────────────────────────────
        Rectangle {
            width: parent.width
            height: 52
            color: tok.panel

            Rectangle {
                anchors { left: parent.left; right: parent.right; top: parent.top }
                height: 1; color: tok.edge
            }

            Item {
                id: footerCloseBtn
                anchors { right: parent.right; rightMargin: 16; verticalCenter: parent.verticalCenter }
                width: 88; height: 32

                property bool hovered: false

                Rectangle {
                    anchors.fill: parent
                    radius: 7
                    color:  footerCloseBtn.hovered ? tok.sage : tok.sageSoft
                    border.color: tok.sage
                    border.width: 1
                }

                Text {
                    anchors.centerIn: parent
                    text: qsTr("Close")
                    font.pixelSize: 13
                    font.weight: Font.Medium
                    color: footerCloseBtn.hovered ? tok.panel : tok.sageDeep
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    onEntered: footerCloseBtn.hovered = true
                    onExited:  footerCloseBtn.hovered = false
                    onClicked: root.closeRequested()
                }
            }
        }
    }
}
