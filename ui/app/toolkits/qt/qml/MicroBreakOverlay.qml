// MicroBreakOverlay.qml — Sanctuary-style micro-break window
// Loaded by QmlMicroBreakWindow into a QQuickView.
// All data comes from the C++ "bridge" context property (MicroBreakBridge).

import QtQuick

Item {
    id: root

    // ── Design tokens ────────────────────────────────────────────────────────
    readonly property color colPanel:    "#FFFFFF"
    readonly property color colInk:      "#2A2D29"
    readonly property color colInk2:     "#4A4D46"
    readonly property color colMute:     "#8A8B82"
    readonly property color colSage:     "#6B8068"
    readonly property color colSageSoft: "#D9E1D2"
    readonly property color colClay:     "#C97B4A"
    readonly property color colClaySoft: "#F2D9C5"
    readonly property color colTrack:    "#E7E1D2"
    readonly property color colEdge:     Qt.rgba(40/255, 45/255, 38/255, 0.10)
    readonly property color colEdge2:    Qt.rgba(40/255, 45/255, 38/255, 0.06)
    readonly property color colWarn:     "#D4872A"

    // blockMode: 0=Off (toast), 1=Input (centered, no dim), 2=All (fullscreen + dim)
    readonly property int    blockMode:    bridge != null ? bridge.blockMode    : 1
    readonly property double ringProgress: bridge != null ? bridge.ringProgress : 1.0
    readonly property string timeRemaining: bridge != null ? bridge.timeRemaining : "0:30"
    readonly property bool   userActive:   bridge != null ? bridge.userActive   : false
    readonly property bool   isLocked:     bridge != null ? bridge.isLocked     : false
    readonly property double lockProgress: bridge != null ? bridge.lockProgress  : 0.0

    // Ring colour: clay/orange when the user is active during the break, sage otherwise
    readonly property color ringColor: userActive ? colWarn : colSage

    // ── Dim backdrop (block_input_and_screen only) ────────────────────────────
    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.50)
        visible: root.blockMode === 2
        z: 0
    }

    // ── Card ─────────────────────────────────────────────────────────────────
    Rectangle {
        id: card
        width: 480
        color: colPanel
        radius: 24
        z: 1

        border.color: root.userActive ? colWarn : colEdge
        border.width: root.userActive ? 1.5 : 1
        Behavior on border.color { ColorAnimation { duration: 300 } }

        // Position: centered for modes 1 & 2, top-left origin for toast (mode 0)
        x: root.blockMode === 0 ? 0 : (root.width  - width)  / 2
        y: root.blockMode === 0 ? 0 : (root.height - height) / 2

        scale: 1.0
        opacity: 1.0

        // Content column
        Column {
            id: content
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                topMargin: 32
                leftMargin: 36
                rightMargin: 36
                bottomMargin: 32
            }
            spacing: 20

            // ── Top row: header pill + corner lock ───────────────────────────
            Item {
                width: parent.width
                height: 28

                // "MICRO-BREAK" pill
                Rectangle {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    height: 22
                    width: headerLabel.implicitWidth + 20
                    radius: 999
                    color: colSageSoft

                    Text {
                        id: headerLabel
                        anchors.centerIn: parent
                        text: bridge != null ? bridge.breakName : qsTr("Micro-break")
                        font.pixelSize: 10
                        font.weight: Font.DemiBold
                        font.letterSpacing: 1.8
                        font.capitalization: Font.AllUppercase
                        color: colSage
                    }
                }

                // Lock button (top-right)
                Rectangle {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    visible: bridge != null ? bridge.lockable : false
                    width: 28; height: 28
                    radius: 999
                    color: "transparent"
                    border.color: colEdge
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "🔒"
                        font.pixelSize: 12
                        color: colMute
                    }

                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("Lock screen")

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: { if (bridge != null) bridge.requestLock() }
                    }
                }
            }

            // ── Hero ring timer ───────────────────────────────────────────────
            Item {
                width: parent.width
                height: 200

                Canvas {
                    id: ringCanvas
                    anchors.centerIn: parent
                    width: 200; height: 200

                    property double prog:     root.ringProgress
                    property color  ringCol:  root.ringColor

                    onProgChanged:    requestPaint()
                    onRingColChanged: requestPaint()
                    Component.onCompleted: requestPaint()

                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.clearRect(0, 0, width, height);
                        var cx = 100, cy = 100, r = 96.5, sw = 7;

                        ctx.beginPath();
                        ctx.arc(cx, cy, r, 0, 2 * Math.PI);
                        ctx.strokeStyle = colTrack;
                        ctx.lineWidth = sw;
                        ctx.stroke();

                        ctx.beginPath();
                        ctx.arc(cx, cy, r, -Math.PI / 2,
                                -Math.PI / 2 + 2 * Math.PI * Math.max(prog, 0.001));
                        ctx.strokeStyle = ringCol.toString();
                        ctx.lineWidth = sw;
                        ctx.lineCap = "round";
                        ctx.stroke();
                    }
                }

                Column {
                    anchors.centerIn: parent
                    spacing: 4

                    Text {
                        text: root.timeRemaining
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: 52
                        font.family: "Georgia"
                        color: colInk
                        renderType: Text.NativeRendering
                        font.features: { "tnum": 1 }
                    }

                    // Activity warning — appears when user keeps working
                    Text {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        visible: root.userActive
                        text: qsTr("Please stop and relax")
                        font.pixelSize: 11
                        color: colWarn
                        opacity: root.userActive ? 1.0 : 0.0
                        Behavior on opacity { NumberAnimation { duration: 300 } }
                    }

                    // Idle hint — visible in blocking modes when user is resting
                    Text {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        visible: root.blockMode > 0 && !root.userActive
                        text: qsTr("Please relax for a few seconds")
                        font.pixelSize: 11
                        color: colMute
                    }
                }
            }

            // ── Rest-break info chip ──────────────────────────────────────────
            Item {
                width: parent.width
                height: restChip.visible ? restChip.height : 0

                Rectangle {
                    id: restChip
                    anchors.horizontalCenter: parent.horizontalCenter
                    visible: bridge != null && bridge.restBreakEnabled && bridge.restBreakInfo !== ""
                    height: 30
                    width: restChipRow.implicitWidth + 28
                    radius: 999
                    color: colClaySoft

                    Row {
                        id: restChipRow
                        anchors.centerIn: parent
                        spacing: 6

                        Text {
                            text: "☕"
                            font.pixelSize: 12
                            color: colClay
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: bridge != null ? bridge.restBreakInfo : ""
                            font.pixelSize: 12
                            color: colClay
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            // ── Action buttons ────────────────────────────────────────────────
            Item {
                width: parent.width
                height: actionRow.implicitHeight

                Row {
                    id: actionRow
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 8

                    // Rest break now
                    Rectangle {
                        visible: bridge != null ? bridge.restBreakEnabled : false
                        height: 34
                        width: restBtnLabel.implicitWidth + 28
                        radius: 999
                        color: colClay

                        Text {
                            id: restBtnLabel
                            anchors.centerIn: parent
                            text: qsTr("Rest break")
                            font.pixelSize: 13
                            font.weight: Font.Medium
                            font.letterSpacing: 0.12
                            color: "#FFFFFF"
                        }

                        Accessible.role: Accessible.Button
                        Accessible.name: qsTr("Rest break")

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: { if (bridge != null) bridge.requestRestBreak() }
                        }
                    }

                    // Postpone
                    Rectangle {
                        visible: bridge != null ? bridge.canPostpone : true
                        enabled: bridge != null ? bridge.canPostpone : true
                        height: 34
                        width: postponeLabel.implicitWidth + 28
                        radius: 999
                        color: "transparent"
                        border.color: colEdge
                        border.width: 1
                        opacity: enabled ? 1.0 : 0.4

                        Text {
                            id: postponeLabel
                            anchors.centerIn: parent
                            text: qsTr("Postpone")
                            font.pixelSize: 13
                            font.weight: Font.Medium
                            font.letterSpacing: 0.12
                            color: colInk2
                        }

                        Accessible.role: Accessible.Button
                        Accessible.name: qsTr("Postpone")

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: { if (bridge != null) bridge.requestPostpone() }
                        }
                    }

                    // Skip
                    Text {
                        visible: bridge != null ? bridge.canSkip : true
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("Skip")
                        font.pixelSize: 12
                        color: colMute
                        leftPadding: 4
                        rightPadding: 4

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: { if (bridge != null) bridge.requestSkip() }
                        }
                    }
                }
            }
            // ── Lock indicator (visible when postpone/skip are temporarily locked) ──
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
                        color: colMute
                    }

                    Rectangle {
                        width: parent.width
                        height: 4
                        radius: 2
                        color: colTrack

                        Rectangle {
                            anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
                            width: Math.max(4, parent.width * root.lockProgress)
                            radius: 2
                            color: colSage
                            Behavior on width { NumberAnimation { duration: 500 } }
                        }
                    }
                }
            }
        }

        height: content.implicitHeight + 64  // 32px top + 32px bottom padding
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
