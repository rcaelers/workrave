// DailyLimitOverlay.qml — Sanctuary-style daily limit window
// Loaded by QmlDailyLimitWindow into a QQuickView.
// All data comes from the C++ "bridge" context property (DailyLimitBridge).

import QtQuick

Item {
    id: root

    // Historical Gtk msgids — reuses the existing po translations. The
    // mnemonic underscore ("_Skip") is stripped for display.
    readonly property string txtSkip:     qsTr("_Skip").replace("_", "")
    readonly property string txtPostpone: qsTr("_Postpone").replace("_", "")
    readonly property string txtShutdown: qsTr("Shutdown")
    readonly property string txtSleep:    qsTr("Suspend")

    PrefTokens { id: tok }

    // ── Bridge bindings ──────────────────────────────────────────────────────
    readonly property int    blockMode:    bridge != null ? bridge.blockMode    : 1
    readonly property bool   canPostpone:  bridge != null ? bridge.canPostpone  : true
    readonly property bool   canSkip:      bridge != null ? bridge.canSkip      : true
    readonly property bool   lockable:     bridge != null ? bridge.lockable     : false
    readonly property bool   shutdownable: bridge != null ? bridge.shutdownable : false
    readonly property bool   sleepable:    bridge != null ? bridge.sleepable    : false
    readonly property bool   isLocked:     bridge != null ? bridge.isLocked     : false
    readonly property double lockProgress: bridge != null ? bridge.lockProgress : 0.0

    // ── Warm background fill (block screen mode only) ────────────────────────
    Rectangle {
        anchors.fill: parent
        color: tok.bg
        visible: root.blockMode === 2
        z: 0
    }

    // ── Card ─────────────────────────────────────────────────────────────────
    Rectangle {
        id: card
        width: 520
        color: tok.panel
        radius: 24
        z: 1

        border.color: tok.edge
        border.width: 1

        x: (root.width  - width)  / 2
        y: (root.height - height) / 2

        scale: 1.0
        opacity: 1.0

        Column {
            id: content
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                topMargin: 32
                leftMargin: 36
                rightMargin: 36
            }
            spacing: 28

            // ── Header row ───────────────────────────────────────────────────
            Item {
                width: parent.width
                height: 28

                // "DAILY LIMIT" pill
                Rectangle {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    height: 22
                    width: headerLabel.implicitWidth + 20
                    radius: 999
                    color: tok.claySoft

                    Text {
                        id: headerLabel
                        anchors.centerIn: parent
                        text: qsTr("Daily limit")
                        font.pixelSize: 10
                        font.weight: Font.DemiBold
                        font.letterSpacing: 1.8
                        font.capitalization: Font.AllUppercase
                        color: tok.clay
                    }
                }

                // Right-side buttons
                Row {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 6

                    // Lock button
                    Rectangle {
                        visible: root.lockable
                        width: 28; height: 28
                        radius: tok.actionRadius
                        color: tok.actionBg
                        border.color: tok.actionEdge
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "🔒"
                            font.pixelSize: 12
                        }

                        Accessible.role: Accessible.Button
                        Accessible.name: qsTr("Lock screen")

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: { if (bridge != null) bridge.requestLock() }
                        }
                    }

                    // Shut down button
                    Rectangle {
                        visible: root.shutdownable
                        width: 28; height: 28; radius: tok.actionRadius
                        color: tok.actionBg; border.color: tok.actionEdge; border.width: 1
                        Text {
                            anchors.centerIn: parent; text: "⏻"
                            font.pixelSize: 15; font.weight: Font.Bold; color: tok.ink2
                        }
                        Accessible.role: Accessible.Button; Accessible.name: root.txtShutdown
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: confirmDlg.ask("shutdown", root.txtShutdown, qsTr("Are you sure you want to shut down the computer?")) }
                    }

                    // Sleep button
                    Rectangle {
                        visible: root.sleepable
                        width: 28; height: 28; radius: tok.actionRadius
                        color: tok.actionBg; border.color: tok.actionEdge; border.width: 1
                        Text {
                            anchors.centerIn: parent; text: "☾"
                            font.pixelSize: 17; font.weight: Font.Bold; color: tok.ink2
                        }
                        Accessible.role: Accessible.Button; Accessible.name: root.txtSleep
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: confirmDlg.ask("sleep", root.txtSleep, qsTr("Are you sure you want to put the computer to sleep?")) }
                    }

                }
            }

            // ── Sun icon ─────────────────────────────────────────────────────
            Canvas {
                id: sunCanvas
                width: 54; height: 54
                anchors.horizontalCenter: parent.horizontalCenter

                onPaint: {
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);
                    ctx.strokeStyle = tok.clay.toString();
                    ctx.lineWidth = 1.4;
                    ctx.lineCap = "round";

                    var cx = width / 2, cy = height / 2;
                    var rInner = 11;
                    var rOuter = 22;
                    var nRays = 8;

                    // Center circle
                    ctx.beginPath();
                    ctx.arc(cx, cy, rInner, 0, 2 * Math.PI);
                    ctx.stroke();

                    // Radiating rays
                    for (var i = 0; i < nRays; i++) {
                        var angle = (i / nRays) * 2 * Math.PI;
                        var x1 = cx + Math.cos(angle) * (rInner + 4);
                        var y1 = cy + Math.sin(angle) * (rInner + 4);
                        var x2 = cx + Math.cos(angle) * rOuter;
                        var y2 = cy + Math.sin(angle) * rOuter;
                        ctx.beginPath();
                        ctx.moveTo(x1, y1);
                        ctx.lineTo(x2, y2);
                        ctx.stroke();
                    }
                }
            }

            // ── Body text ────────────────────────────────────────────────────
            Text {
                width: parent.width
                text: qsTr("You have reached your daily limit. Please stop working\nbehind the computer. If your working day is not over yet,\nfind something else to do, such as reviewing a document.")
                font.pixelSize: 15
                color: tok.ink2
                wrapMode: Text.WordWrap
                lineHeight: 1.5
                horizontalAlignment: Text.AlignHCenter
            }

            // ── Lock indicator (visible when buttons are temporarily locked) ────
            Item {
                width: parent.width
                height: root.isLocked ? lockCol.implicitHeight + 8 : 0
                clip: true

                Behavior on height { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }

                Column {
                    id: lockCol
                    width: parent.width
                    spacing: 8
                    anchors.bottom: parent.bottom

                    Text {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        text: qsTr("Postpone and skip will unlock after resting")
                        font.pixelSize: 11
                        color: tok.mute
                    }

                    Rectangle {
                        width: parent.width
                        height: 4
                        radius: 2
                        color: tok.track

                        Rectangle {
                            anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
                            width: Math.max(4, parent.width * root.lockProgress)
                            radius: 2
                            color: tok.sage
                            Behavior on width { NumberAnimation { duration: 500 } }
                        }
                    }
                }
            }

            // ── Action row ───────────────────────────────────────────────────
            Item {
                width: parent.width
                height: actionRow.implicitHeight

                Row {
                    id: actionRow
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 8

                    Rectangle {
                        visible: root.canSkip
                        enabled: root.canSkip
                        height: 34
                        width: skipLabel.implicitWidth + 28
                        radius: tok.actionRadius
                        color: tok.actionBg
                        border.color: tok.actionEdge
                        border.width: 1
                        opacity: enabled ? 1.0 : 0.4

                        Text {
                            id: skipLabel
                            anchors.centerIn: parent
                            text: root.txtSkip
                            font.pixelSize: 13
                            font.weight: Font.Medium
                            font.letterSpacing: 0.12
                            color: tok.ink2
                        }

                        Accessible.role: Accessible.Button
                        Accessible.name: root.txtSkip

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: { if (bridge != null) bridge.requestSkip() }
                        }
                    }

                    Rectangle {
                        visible: root.canPostpone
                        enabled: root.canPostpone
                        height: 34
                        width: postponeLabel.implicitWidth + 28
                        radius: tok.actionRadius
                        color: tok.actionBg
                        border.color: tok.actionEdge
                        border.width: 1
                        opacity: enabled ? 1.0 : 0.4

                        Text {
                            id: postponeLabel
                            anchors.centerIn: parent
                            text: root.txtPostpone
                            font.pixelSize: 13
                            font.weight: Font.Medium
                            font.letterSpacing: 0.12
                            color: tok.ink2
                        }

                        Accessible.role: Accessible.Button
                        Accessible.name: root.txtPostpone

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: { if (bridge != null) bridge.requestPostpone() }
                        }
                    }
                }
            }

            // bottom spacer
            Item { width: 1; height: 4 }
        }

        height: content.implicitHeight + 64
    }

    ConfirmDialog {
        id: confirmDlg
        anchors.fill: parent
        z: 200
        onConfirmed: (action) => {
            if (action === "shutdown") { if (bridge != null) bridge.requestShutdown() }
            else if (action === "sleep")    { if (bridge != null) bridge.requestSleep() }
        }
    }

    // ── Card enter animation ──────────────────────────────────────────────────
    Component.onCompleted: {
        card.scale   = 0.96
        card.opacity = 0
        enterAnim.start()
    }

    SequentialAnimation {
        id: enterAnim
        ParallelAnimation {
            NumberAnimation { target: card; property: "scale";   to: 1.0; duration: 240; easing.type: Easing.OutCubic }
            NumberAnimation { target: card; property: "opacity"; to: 1.0; duration: 240; easing.type: Easing.OutCubic }
        }
    }
}
