import QtQuick

// GeneralPrefPage — content for User interface > General preferences.
// Expects context property: generalPrefBridge (GeneralPrefBridge)
// Block-mode values: 0=Off (no blocking), 1=Input, 2=All (input+screen)
Item {
    id: root

    property var bridge: typeof generalPrefBridge !== "undefined" ? generalPrefBridge : null

    implicitWidth:  parent ? parent.width : 500
    implicitHeight: col.implicitHeight

    PrefTokens { id: tok }

    Column {
        id: col
        anchors { left: parent.left; right: parent.right; top: parent.top }
        spacing: 24

        // ── During breaks ─────────────────────────────────────────────────────
        PrefGroup {
            width: parent.width
            title: qsTr("During breaks")

            // Block-mode row (bespoke card picker)
            Item {
                width: parent.width
                height: blockModeContent.implicitHeight + 28

                // Bottom divider
                Rectangle {
                    anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
                    height: 1; color: tok.edge2
                }

                Column {
                    id: blockModeContent
                    anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 14 }
                    spacing: 14

                    Column {
                        width: parent.width
                        spacing: tok.labelHintGap

                        Text {
                            text: qsTr("Block mode")
                            font.pixelSize: tok.labelPx; font.weight: Font.Medium
                            color: tok.ink
                        }
                        Text {
                            width: parent.width
                            text: qsTr("What Workrave does to the rest of your screen when a break starts.")
                            font.pixelSize: tok.hintPx; color: tok.mute
                            wrapMode: Text.WordWrap; lineHeight: tok.hintLineH
                        }
                    }

                    // Three-card grid: Input | All | Off
                    Row {
                        width: parent.width
                        spacing: 12

                        BlockModeCard {
                            width: (parent.width - 24) / 3
                            active: root.bridge ? root.bridge.blockMode === 1 : true
                            cardTitle: qsTr("Block input")
                            cardDesc:  qsTr("Keyboard and mouse are blocked. You can still see your desktop.")
                            previewKind: "input"
                            onClicked: { if (root.bridge) root.bridge.setBlockMode(1) }
                        }

                        BlockModeCard {
                            width: (parent.width - 24) / 3
                            active: root.bridge ? root.bridge.blockMode === 2 : false
                            cardTitle: qsTr("Block input and screen")
                            cardDesc:  qsTr("Break window covers everything but your wallpaper.")
                            previewKind: "screen"
                            onClicked: { if (root.bridge) root.bridge.setBlockMode(2) }
                        }

                        BlockModeCard {
                            width: (parent.width - 24) / 3
                            active: root.bridge ? root.bridge.blockMode === 0 : false
                            cardTitle: qsTr("No blocking")
                            cardDesc:  qsTr("Workrave shows the break window but doesn't interrupt input.")
                            previewKind: "none"
                            onClicked: { if (root.bridge) root.bridge.setBlockMode(0) }
                        }
                    }
                }
            }
        }

        // ── Theme ─────────────────────────────────────────────────────────────
        PrefGroup {
            width: parent.width
            title: qsTr("Theme")

            PrefToggleRow {
                width: parent.width
                label: qsTr("Use Sanctuary UI")
                hint:  qsTr("When enabled, break windows and the status window use the new QML-based design. Restart Workrave after changing this setting.")
                checked: root.bridge ? root.bridge.sanctuaryEnabled : true
                onToggled: (v) => { if (root.bridge) root.bridge.setSanctuaryEnabled(v) }
                isLast: true
            }
        }

        // ── System ────────────────────────────────────────────────────────────
        PrefGroup {
            width: parent.width
            title: qsTr("System")

            PrefToggleRow {
                width: parent.width
                label: qsTr("Show system tray icon")
                hint:  qsTr("Adds a Workrave icon to the system tray for quick access.")
                checked: root.bridge ? root.bridge.trayIconEnabled : true
                onToggled: (v) => { if (root.bridge) root.bridge.setTrayIconEnabled(v) }
            }

            PrefToggleRow {
                width: parent.width
                label: qsTr("Start Workrave on logon")
                hint:  qsTr("Launches Workrave automatically when you sign in.")
                checked: root.bridge ? root.bridge.autostartEnabled : false
                onToggled: (v) => { if (root.bridge) root.bridge.setAutostartEnabled(v) }
            }

            PrefLanguageRow {
                width: parent.width
                label:     qsTr("Language")
                hint:      qsTr("Language used for Workrave's interface.")
                languages: root.bridge ? root.bridge.languages : []
                currentId: root.bridge ? root.bridge.currentLanguage : ""
                onSelected: (id) => { if (root.bridge) root.bridge.setLanguage(id) }
            }

            PrefChoiceRow {
                width: parent.width
                visible: root.bridge ? root.bridge.hasDarkMode : false
                label:   qsTr("Dark mode")
                hint:    qsTr("Light or dark colour scheme for the Workrave windows.")
                options: [qsTr("Light"), qsTr("Dark"), qsTr("Auto")]
                currentIndex: root.bridge ? root.bridge.darkMode : 0
                onSelected: (idx) => { if (root.bridge) root.bridge.setDarkMode(idx) }
            }

            // Icon theme — Linux only (row hidden when list is empty)
            PrefChoiceRow {
                width: parent.width
                visible: root.bridge ? root.bridge.iconThemes.length > 0 : false
                label:   qsTr("Icon theme")
                hint:    qsTr("Visual style used for Workrave's icons.")
                options: {
                    if (!root.bridge) return [qsTr("Default")]
                    var names = [qsTr("Default")]
                    for (var i = 0; i < root.bridge.iconThemes.length; i++)
                        names.push(root.bridge.iconThemes[i].name)
                    return names
                }
                currentIndex: {
                    if (!root.bridge) return 0
                    var cur = root.bridge.currentIconTheme
                    for (var i = 0; i < root.bridge.iconThemes.length; i++) {
                        if (root.bridge.iconThemes[i].id === cur) return i + 1
                    }
                    return 0
                }
                onSelected: (idx) => {
                    if (!root.bridge) return
                    if (idx === 0)
                        root.bridge.setIconTheme("")
                    else
                        root.bridge.setIconTheme(root.bridge.iconThemes[idx - 1].id)
                }
            }
        }

        // ── Wayland / X11 ─────────────────────────────────────────────────────
        PrefGroup {
            width: parent.width
            visible: root.bridge ? (root.bridge.hasForceX11 || root.bridge.hasGnomeShellPreludes) : false
            title: qsTr("Linux display server")

            PrefToggleRow {
                width: parent.width
                visible: root.bridge ? root.bridge.hasForceX11 : false
                label: qsTr("Force the use of X11 on Wayland (requires restart of Workrave)")
                hint:  qsTr("Runs Workrave under XWayland instead of native Wayland. Requires a restart of Workrave.")
                checked: root.bridge ? root.bridge.forceX11 : false
                onToggled: (v) => { if (root.bridge) root.bridge.setForceX11(v) }
            }

            PrefToggleRow {
                width: parent.width
                visible: root.bridge ? root.bridge.hasGnomeShellPreludes : false
                label: qsTr("Use GNOME Shell extension for showing break prompts on Wayland (EXPERIMENTAL)")
                hint:  qsTr("Uses the Workrave GNOME Shell extension to show break prompts on Wayland. Experimental.")
                checked: root.bridge ? root.bridge.gnomeShellPreludes : false
                onToggled: (v) => { if (root.bridge) root.bridge.setGnomeShellPreludes(v) }
            }
        }
    }

    // ── BlockModeCard component ───────────────────────────────────────────────
    component BlockModeCard: Item {
        id: card

        property bool   active:      false
        property string cardTitle:   ""
        property string cardDesc:    ""
        property string previewKind: "input"

        signal clicked()

        implicitHeight: cardCol.implicitHeight + 26

        Rectangle {
            anchors.fill: parent
            radius: 10
            color:  card.active ? tok.sageSoft : tok.panel
            border.color: card.active ? tok.sage : tok.edge
            border.width: card.active ? 2 : 1
        }

        Column {
            id: cardCol
            anchors {
                left: parent.left; right: parent.right
                top: parent.top
                leftMargin: 12; rightMargin: 12; topMargin: 12
            }
            spacing: 8

            // Desktop preview thumbnail
            BlockPreview {
                width: parent.width
                height: 88
                kind: card.previewKind
            }

            // Radio dot + title
            Row {
                spacing: 6

                // Active: sage outer ring + white gap + sage center
                // Inactive: white fill + edge border
                Item {
                    width: 14; height: 14
                    anchors.verticalCenter: parent.verticalCenter

                    Rectangle {
                        anchors.fill: parent
                        radius: 7
                        color:  card.active ? tok.sage : tok.panel
                        border.color: card.active ? tok.sage : tok.edge
                        border.width: 1
                    }

                    // White ring (only shown when active to create inset-shadow effect)
                    Rectangle {
                        visible: card.active
                        anchors.centerIn: parent
                        width: 8; height: 8; radius: 4
                        color: tok.panel
                    }

                    // Sage center dot (only shown when active)
                    Rectangle {
                        visible: card.active
                        anchors.centerIn: parent
                        width: 4; height: 4; radius: 2
                        color: tok.sage
                    }
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: card.cardTitle
                    font.pixelSize: 13; font.weight: Font.DemiBold
                    color: tok.ink
                }
            }

            // Description
            Text {
                width: parent.width
                text: card.cardDesc
                font.pixelSize: 12
                color: tok.ink2
                wrapMode: Text.WordWrap
                lineHeight: 1.45
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: card.clicked()
        }

        Accessible.role: Accessible.RadioButton
        Accessible.name: card.cardTitle
        Accessible.checked: card.active
    }

    // ── BlockPreview component ────────────────────────────────────────────────
    component BlockPreview: Item {
        id: preview

        property string kind: "input"

        clip: true

        // Green desktop gradient background
        Rectangle {
            anchors.fill: parent
            radius: 6
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop { position: 0.0; color: tok.isDark ? "#2E3A2A" : "#c9d7c0" }
                GradientStop { position: 1.0; color: tok.isDark ? "#253020" : "#b6c5a8" }
            }
            border.color: tok.edge
            border.width: 1
        }

        // Faux app windows (shown for input and none modes)
        Rectangle {
            visible: preview.kind !== "screen"
            x: 6; y: 10; width: 64; height: 38
            radius: 3; color: Qt.rgba(1, 1, 1, 0.90)
        }
        Rectangle {
            visible: preview.kind !== "screen"
            x: 38; y: 30; width: 80; height: 48
            radius: 3; color: Qt.rgba(1, 1, 1, 0.92)
        }

        // Break window — "input" mode: card on the right
        Rectangle {
            visible: preview.kind === "input"
            anchors { right: parent.right; rightMargin: 8; verticalCenter: parent.verticalCenter }
            width: 72; height: 50; radius: 5
            color: tok.panel
            border.color: tok.edge; border.width: 1

            Text {
                anchors.centerIn: parent
                text: "0:18"
                font.pixelSize: 14; font.family: "Georgia"
                color: tok.ink
            }
        }

        // Fullscreen dark overlay — "screen" mode
        Rectangle {
            visible: preview.kind === "screen"
            anchors.fill: parent
            radius: 6
            color: Qt.rgba(40/255, 45/255, 38/255, 0.60)

            Rectangle {
                anchors.centerIn: parent
                width: parent.width * 0.70; height: parent.height * 0.70
                radius: 6; color: tok.panel

                Text {
                    anchors.centerIn: parent
                    text: "0:18"
                    font.pixelSize: 18; font.family: "Georgia"
                    color: tok.ink
                }
            }
        }

        // Toast notification — "none" mode: small card top-right
        Rectangle {
            visible: preview.kind === "none"
            anchors { right: parent.right; rightMargin: 6; top: parent.top; topMargin: 6 }
            width: 54; height: 34; radius: 4
            color: tok.panel
            border.color: tok.edge; border.width: 1

            Text {
                anchors.centerIn: parent
                text: "0:18"
                font.pixelSize: 11; font.family: "Georgia"
                color: tok.ink
            }
        }
    }
}
