// RestBreakOverlay.qml — Sanctuary-style rest-break window
// Loaded by QmlRestBreakWindow into a QQuickView.
// All data comes from the C++ "bridge" context property (RestBreakBridge).
//
// Two visual modes:
//   blockMode 2 (All)   — warm full-screen background, three-column exercise layout
//   blockMode 0/1       — transparent full-screen view, floating white card (centered)

import QtQuick

Item {
    id: root

    // ── Design tokens ────────────────────────────────────────────────────────
    readonly property color colBg:       "#F5F1EA"
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

    // ── Bridge bindings ──────────────────────────────────────────────────────
    readonly property int        blockMode:       bridge != null ? bridge.blockMode          : 1
    readonly property bool       hasExercises:    bridge != null ? bridge.hasExercises       : false
    readonly property bool       exercisesDone:   bridge != null ? bridge.exercisesDone      : false
    readonly property int        exerciseCount:   bridge != null ? bridge.exerciseCount      : 4
    readonly property int        exerciseIndex:   bridge != null ? bridge.exerciseIndex      : 0
    readonly property var        exerciseNames:   bridge != null ? bridge.exerciseNames      : []
    readonly property string     exerciseName:    bridge != null ? bridge.exerciseName       : ""
    readonly property string     exerciseDesc:    bridge != null ? bridge.exerciseDescription : ""
    readonly property string     exerciseImage:   bridge != null ? bridge.exerciseImage      : ""
    readonly property bool       exerciseMirror:  bridge != null ? bridge.exerciseImageMirror : false
    readonly property string     exerciseTimeStr: bridge != null ? bridge.exerciseTimeStr    : "0:45"
    readonly property bool       isPaused:        bridge != null ? bridge.isPaused           : false
    readonly property bool       canPostpone:     bridge != null ? bridge.canPostpone        : true
    readonly property bool       canSkip:         bridge != null ? bridge.canSkip            : true
    readonly property bool       lockable:        bridge != null ? bridge.lockable           : false
    readonly property bool       isLocked:        bridge != null ? bridge.isLocked           : false
    readonly property double     lockProgress:    bridge != null ? bridge.lockProgress       : 0.0
    readonly property double     breakProgress:   bridge != null ? bridge.breakProgress      : 1.0
    readonly property string     breakTimeShort:  bridge != null ? bridge.breakTimeShort     : "5:00"
    readonly property string     breakMaxStr:     bridge != null ? bridge.breakMaxStr        : "10:00"

    readonly property bool showExercises: root.hasExercises && !root.exercisesDone

    // ════════════════════════════════════════════════════════════════════════
    // CARD LAYOUT  (blockMode 0 = Off, 1 = Input)
    // Transparent full-screen view; a white rounded card is centered inside.
    // ════════════════════════════════════════════════════════════════════════
    Item {
        id: cardLayout
        anchors.fill: parent
        visible: root.blockMode !== 2

        Rectangle {
            id: card
            anchors.centerIn: parent
            width: Math.min(parent.width - 48, 1040)
            height: cardCol.implicitHeight
            color: colPanel
            radius: 24
            border.color: colEdge
            border.width: 0.5

            Column {
                id: cardCol
                width: parent.width

                // ── Compact header ───────────────────────────────────────────
                Item {
                    width: parent.width
                    height: 52

                    // Left: ● breadcrumb · divider · timer
                    Row {
                        anchors { left: parent.left; leftMargin: 20; verticalCenter: parent.verticalCenter }
                        spacing: 0

                        Rectangle {
                            width: 6; height: 6; radius: 999; color: colClay
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Item { width: 8; height: 1 }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("REST BREAK") + " · "
                                  + (root.exerciseIndex + 1) + " " + qsTr("OF") + " " + root.exerciseCount
                                  + (root.blockMode === 1 ? " · " + qsTr("INPUT BLOCKED") : "")
                            font.pixelSize: 11; font.weight: Font.DemiBold
                            font.letterSpacing: 0.5; color: colMute
                        }
                        Item { width: 12; height: 1 }
                        Rectangle { width: 1; height: 14; color: colEdge; anchors.verticalCenter: parent.verticalCenter }
                        Item { width: 12; height: 1 }
                        Row {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 5
                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: root.breakTimeShort
                                font.pixelSize: 18; font.family: "Georgia"; color: colInk
                            }
                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: qsTr("OF") + " " + root.breakMaxStr
                                font.pixelSize: 11; font.weight: Font.DemiBold
                                font.letterSpacing: 0.5; color: colMute
                            }
                        }
                    }

                    // Right: lock | divider | Postpone | Skip
                    Row {
                        anchors { right: parent.right; rightMargin: 16; verticalCenter: parent.verticalCenter }
                        spacing: 4

                        Rectangle {
                            visible: root.lockable
                            width: 28; height: 28; radius: 999
                            color: "transparent"; border.color: colEdge; border.width: 1
                            anchors.verticalCenter: parent.verticalCenter
                            Text { anchors.centerIn: parent; text: "🔒"; font.pixelSize: 12 }
                            Accessible.role: Accessible.Button
                            Accessible.name: qsTr("Lock screen")
                            MouseArea {
                                anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                onClicked: { if (bridge != null) bridge.requestLock() }
                            }
                        }

                        // Postpone / Skip only in header when exercises are visible;
                        // when ring view is shown they move to the bottom of the card.
                        Rectangle {
                            width: 1; height: 18; color: colEdge
                            anchors.verticalCenter: parent.verticalCenter
                            visible: root.showExercises && root.lockable && (root.canPostpone || root.canSkip)
                        }

                        Rectangle {
                            visible: root.showExercises && root.canPostpone
                            height: 28; width: cardPostponeLbl.implicitWidth + 20
                            radius: 999; color: "transparent"; border.color: colEdge; border.width: 1
                            opacity: root.canPostpone ? 1.0 : 0.4
                            anchors.verticalCenter: parent.verticalCenter
                            Text {
                                id: cardPostponeLbl; anchors.centerIn: parent
                                text: qsTr("Postpone"); font.pixelSize: 12; font.weight: Font.Medium; color: colInk2
                            }
                            Accessible.role: Accessible.Button
                            Accessible.name: qsTr("Postpone")
                            MouseArea {
                                anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                onClicked: { if (bridge != null) bridge.requestPostpone() }
                            }
                        }

                        Text {
                            visible: root.showExercises && root.canSkip
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Skip"); font.pixelSize: 12; color: colInk2
                            leftPadding: 4; rightPadding: 4
                            MouseArea {
                                anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                onClicked: { if (bridge != null) bridge.requestSkip() }
                            }
                        }
                    }
                }

                // Separator
                Rectangle { width: parent.width; height: 1; color: colEdge }

                // ── Lock strip ───────────────────────────────────────────────
                Item {
                    width: parent.width
                    height: root.isLocked ? cardLockCol.implicitHeight + 16 : 0
                    clip: true
                    Behavior on height { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }

                    Column {
                        id: cardLockCol
                        anchors {
                            left: parent.left; right: parent.right
                            leftMargin: 24; rightMargin: 24
                            bottom: parent.bottom; bottomMargin: 8
                        }
                        spacing: 6
                        Text {
                            width: parent.width; horizontalAlignment: Text.AlignHCenter
                            text: qsTr("Postpone and skip will unlock after resting")
                            font.pixelSize: 11; color: colMute
                        }
                        Rectangle {
                            width: parent.width; height: 4; radius: 2; color: colTrack
                            Rectangle {
                                anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
                                width: Math.max(4, parent.width * root.lockProgress)
                                radius: 2; color: colSage
                                Behavior on width { NumberAnimation { duration: 500 } }
                            }
                        }
                    }
                }

                // ── Body — exercise or ring ──────────────────────────────────
                Item {
                    id: cardBody
                    width: parent.width
                    height: root.showExercises ? cardExCol.implicitHeight : cardRingCol.implicitHeight

                    // Exercise content
                    Column {
                        id: cardExCol
                        visible: root.showExercises
                        width: parent.width - 56
                        x: 28
                        topPadding: 24; bottomPadding: 28; spacing: 20

                        // Progress dots
                        Row {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 8
                            Repeater {
                                model: root.exerciseCount
                                Rectangle { width: 8; height: 8; radius: 999; color: index <= root.exerciseIndex ? colClay : colTrack }
                            }
                        }

                        // Image + title / description
                        Row {
                            width: parent.width; spacing: 28

                            Item {
                                id: cardImgContainer
                                width: 200; height: 200

                                Image {
                                    id: cardExImg
                                    anchors.fill: parent
                                    source: root.exerciseImage
                                    mirror: root.exerciseMirror
                                    fillMode: Image.PreserveAspectFit
                                }
                                Rectangle {
                                    anchors.fill: parent; color: colSageSoft; radius: 16
                                    visible: cardExImg.status !== Image.Ready || root.exerciseImage === ""
                                }
                            }

                            Column {
                                width: parent.width - cardImgContainer.width - 28
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 14
                                Text {
                                    width: parent.width
                                    text: root.exerciseName
                                    font.pixelSize: 36; font.family: "Georgia"; color: colInk
                                    wrapMode: Text.Wrap; lineHeight: 1.1
                                }
                                Text {
                                    width: parent.width
                                    text: root.exerciseDesc
                                    font.pixelSize: 15; color: colInk2
                                    wrapMode: Text.WordWrap; lineHeight: 1.6
                                }
                            }
                        }

                        // Navigation
                        Row {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 16

                            Rectangle {
                                width: 36; height: 36; radius: 999
                                color: "transparent"; border.color: colEdge; border.width: 1
                                Text { anchors.centerIn: parent; text: "◄"; font.pixelSize: 12; color: colInk2 }
                                Accessible.role: Accessible.Button; Accessible.name: qsTr("Previous exercise")
                                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { if (bridge != null) bridge.prevExercise() } }
                            }

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: root.exerciseTimeStr
                                font.pixelSize: 34; font.family: "Georgia"; color: colInk
                                horizontalAlignment: Text.AlignHCenter
                                font.features: {"tnum": 1}
                            }

                            Rectangle {
                                width: 36; height: 36; radius: 999
                                color: "transparent"; border.color: colEdge; border.width: 1
                                Text { anchors.centerIn: parent; text: "►"; font.pixelSize: 12; color: colInk2 }
                                Accessible.role: Accessible.Button; Accessible.name: qsTr("Next exercise")
                                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { if (bridge != null) bridge.nextExercise() } }
                            }
                        }
                    }

                    // Ring view (no exercises or all done)
                    Column {
                        id: cardRingCol
                        visible: !root.showExercises
                        width: parent.width
                        topPadding: 32; bottomPadding: 36; spacing: 4

                        Canvas {
                            id: cardRingCanvas
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: 280; height: 280

                            property double prog: root.breakProgress
                            onProgChanged: requestPaint()
                            Component.onCompleted: requestPaint()

                            onPaint: {
                                var ctx = getContext("2d");
                                ctx.clearRect(0, 0, width, height);
                                var cx = 140, cy = 140, r = 133, sw = 8;
                                ctx.beginPath();
                                ctx.arc(cx, cy, r, 0, 2 * Math.PI);
                                ctx.strokeStyle = colTrack.toString();
                                ctx.lineWidth = sw; ctx.stroke();
                                ctx.beginPath();
                                ctx.arc(cx, cy, r, -Math.PI / 2,
                                        -Math.PI / 2 + 2 * Math.PI * Math.max(prog, 0.001));
                                ctx.strokeStyle = colSage.toString();
                                ctx.lineWidth = sw; ctx.lineCap = "round"; ctx.stroke();
                            }

                            Text {
                                anchors.centerIn: parent
                                text: root.breakTimeShort
                                font.pixelSize: 70; font.family: "Georgia"; color: colInk
                            }
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            visible: root.exercisesDone
                            text: qsTr("Exercises complete — please relax")
                            font.pixelSize: 13; color: colMute
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            visible: !root.hasExercises
                            text: qsTr("Please stand up and walk away from your computer")
                            font.pixelSize: 13; color: colMute
                        }

                        Item { width: 1; height: 16 }

                        // Action buttons — mirroring micro-break card layout
                        Row {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 8

                            Rectangle {
                                visible: root.canPostpone
                                height: 34; width: ringPostponeLbl.implicitWidth + 28
                                radius: 999; color: "transparent"; border.color: colEdge; border.width: 1
                                opacity: root.canPostpone ? 1.0 : 0.4
                                anchors.verticalCenter: parent.verticalCenter
                                Text {
                                    id: ringPostponeLbl; anchors.centerIn: parent
                                    text: qsTr("Postpone"); font.pixelSize: 13; font.weight: Font.Medium; color: colInk2
                                }
                                Accessible.role: Accessible.Button; Accessible.name: qsTr("Postpone")
                                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { if (bridge != null) bridge.requestPostpone() } }
                            }

                            Text {
                                visible: root.canSkip
                                anchors.verticalCenter: parent.verticalCenter
                                text: qsTr("Skip"); font.pixelSize: 12; color: colInk2
                                leftPadding: 4; rightPadding: 4
                                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { if (bridge != null) bridge.requestSkip() } }
                            }
                        }
                    }
                }
            }
        }

        // Card enter animation
        Component.onCompleted: {
            card.opacity = 0
            card.scale   = 0.98
            cardEnterAnim.start()
        }
        SequentialAnimation {
            id: cardEnterAnim
            ParallelAnimation {
                NumberAnimation { target: card; property: "opacity"; to: 1.0; duration: 260; easing.type: Easing.OutCubic }
                NumberAnimation { target: card; property: "scale";   to: 1.0; duration: 260; easing.type: Easing.OutCubic }
            }
        }
    }

    // ════════════════════════════════════════════════════════════════════════
    // FULL-SCREEN LAYOUT  (blockMode 2 = Block input + screen)
    // Warm background fills the screen; three-column exercise layout.
    // ════════════════════════════════════════════════════════════════════════
    Item {
        id: fullScreenLayout
        anchors.fill: parent
        visible: root.blockMode === 2

        Rectangle { anchors.fill: parent; color: colBg; z: 0 }

        Item {
            id: contentArea
            anchors { fill: parent; leftMargin: 48; rightMargin: 48; topMargin: 32; bottomMargin: 40 }
            z: 1

            // ── Header strip ──────────────────────────────────────────────────
            Item {
                id: headerStrip
                anchors { top: parent.top; left: parent.left; right: parent.right }
                height: 36

                Row {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 8
                    Rectangle { width: 6; height: 6; radius: 999; color: colClay; anchors.verticalCenter: parent.verticalCenter }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("REST BREAK") + " · " + (root.exerciseIndex + 1) + " " + qsTr("OF") + " " + root.exerciseCount
                        font.pixelSize: 12; font.weight: Font.DemiBold; font.letterSpacing: 0.5; color: colMute
                    }
                }

                Row {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 4

                    Rectangle {
                        visible: root.lockable
                        width: 28; height: 28; radius: 999
                        color: "transparent"; border.color: colEdge; border.width: 1
                        anchors.verticalCenter: parent.verticalCenter
                        Text { anchors.centerIn: parent; text: "🔒"; font.pixelSize: 12 }
                        Accessible.role: Accessible.Button; Accessible.name: qsTr("Lock screen")
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { if (bridge != null) bridge.requestLock() } }
                    }

                    Rectangle {
                        width: 1; height: 18; color: colEdge
                        anchors.verticalCenter: parent.verticalCenter
                        visible: root.lockable && (root.canPostpone || root.canSkip)
                    }

                    Rectangle {
                        visible: root.canPostpone
                        height: 28; width: postponeLabel.implicitWidth + 20
                        radius: 999; color: "transparent"; border.color: colEdge; border.width: 1
                        opacity: root.canPostpone ? 1.0 : 0.4
                        anchors.verticalCenter: parent.verticalCenter
                        Text { id: postponeLabel; anchors.centerIn: parent; text: qsTr("Postpone"); font.pixelSize: 12; font.weight: Font.Medium; color: colInk2 }
                        Accessible.role: Accessible.Button; Accessible.name: qsTr("Postpone")
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { if (bridge != null) bridge.requestPostpone() } }
                    }

                    Text {
                        visible: root.canSkip
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("Skip"); font.pixelSize: 12; color: colInk2
                        leftPadding: 4; rightPadding: 4
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { if (bridge != null) bridge.requestSkip() } }
                    }
                }
            }

            // ── Lock strip ────────────────────────────────────────────────────
            Item {
                id: lockStrip
                anchors { top: headerStrip.bottom; left: parent.left; right: parent.right }
                height: root.isLocked ? lockStripCol.implicitHeight + 12 : 0
                clip: true
                Behavior on height { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }

                Column {
                    id: lockStripCol
                    anchors { left: parent.left; right: parent.right; bottom: parent.bottom; bottomMargin: 6 }
                    spacing: 6
                    Text {
                        width: parent.width; horizontalAlignment: Text.AlignHCenter
                        text: qsTr("Postpone and skip will unlock after resting")
                        font.pixelSize: 11; color: colMute
                    }
                    Rectangle {
                        width: parent.width; height: 4; radius: 2; color: colTrack
                        Rectangle {
                            anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
                            width: Math.max(4, parent.width * root.lockProgress)
                            radius: 2; color: colSage
                            Behavior on width { NumberAnimation { duration: 500 } }
                        }
                    }
                }
            }

            // ── Body area ─────────────────────────────────────────────────────
            Item {
                id: bodyArea
                anchors { top: lockStrip.bottom; topMargin: 20; left: parent.left; right: parent.right; bottom: parent.bottom }

                Item {
                    id: threeCol
                    anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; bottom: parent.bottom }
                    width: Math.min(parent.width, 1060)

                    // Left column: exercise sequence list
                    Item {
                        id: seqPanel
                        visible: root.showExercises
                        anchors { top: parent.top; left: parent.left; bottom: parent.bottom }
                        width: root.showExercises ? 220 : 0

                        Column {
                            anchors { verticalCenter: parent.verticalCenter; left: parent.left; right: parent.right }
                            spacing: 2

                            Text { text: qsTr("Exercises"); font.pixelSize: 14; font.italic: true; font.family: "Georgia"; color: colMute; bottomPadding: 10 }

                            Repeater {
                                model: root.exerciseNames
                                delegate: Item {
                                    id: exRow
                                    width: seqPanel.width; height: 52
                                    property bool isDone:    root.exercisesDone || index < root.exerciseIndex
                                    property bool isCurrent: !root.exercisesDone && index === root.exerciseIndex

                                    Rectangle {
                                        anchors { fill: parent; leftMargin: -4; rightMargin: -4 }
                                        radius: 10; color: colPanel; border.color: colEdge; border.width: 0.5
                                        visible: exRow.isCurrent
                                    }
                                    Row {
                                        anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; leftMargin: 8; rightMargin: 8 }
                                        spacing: 10
                                        Rectangle {
                                            width: 32; height: 32; radius: 8
                                            color: exRow.isDone ? colSageSoft : (exRow.isCurrent ? colClaySoft : "transparent")
                                            border.color: (!exRow.isDone && !exRow.isCurrent) ? colEdge : "transparent"; border.width: 1
                                            anchors.verticalCenter: parent.verticalCenter
                                            Text { anchors.centerIn: parent; text: exRow.isDone ? "✓" : (exRow.isCurrent ? "●" : "○"); font.pixelSize: 13; color: exRow.isDone ? colSage : (exRow.isCurrent ? colClay : colMute) }
                                        }
                                        Text {
                                            anchors.verticalCenter: parent.verticalCenter
                                            width: parent.width - 32 - 10
                                            text: modelData; font.pixelSize: 13; font.weight: exRow.isCurrent ? Font.Medium : Font.Normal
                                            font.strikeout: exRow.isDone; color: exRow.isDone ? colMute : (exRow.isCurrent ? colInk : colInk2); elide: Text.ElideRight
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Right column: total-break timer
                    Item {
                        id: timerPanel
                        visible: root.showExercises
                        anchors { top: parent.top; right: parent.right; bottom: parent.bottom }
                        width: root.showExercises ? 160 : 0

                        Column {
                            anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                            spacing: 0
                            Text { anchors.right: parent.right; text: qsTr("Total left"); font.pixelSize: 13; font.italic: true; font.family: "Georgia"; color: colMute; bottomPadding: 4 }
                            Text { anchors.right: parent.right; text: root.breakTimeShort; font.pixelSize: 58; font.family: "Georgia"; color: colInk; lineHeight: 1.0 }
                            Text { anchors.right: parent.right; text: "OF " + root.breakMaxStr; font.pixelSize: 11; font.weight: Font.DemiBold; font.letterSpacing: 1.4; color: colMute; topPadding: 4; bottomPadding: 10 }
                            Rectangle {
                                anchors.right: parent.right; width: parent.width; height: 4; radius: 2; color: colTrack
                                Rectangle { anchors.left: parent.left; anchors.top: parent.top; anchors.bottom: parent.bottom; width: Math.max(4, parent.width * root.breakProgress); radius: 2; color: colClay }
                            }
                        }
                    }

                    // Center column: exercise card or ring
                    Item {
                        id: centerCol
                        anchors {
                            top: parent.top; bottom: parent.bottom
                            left: root.showExercises ? seqPanel.right : parent.left; leftMargin: root.showExercises ? 12 : 0
                            right: timerPanel.left; rightMargin: 20
                        }

                        // Exercise card
                        Rectangle {
                            id: exerciseCard
                            anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
                            height: Math.min(parent.height - 24, 680)
                            radius: 24; color: colPanel; border.color: colEdge; border.width: 0.5

                            Item {
                                id: activeView
                                anchors { fill: parent; topMargin: 32; leftMargin: 40; rightMargin: 40; bottomMargin: 32 }
                                visible: root.showExercises

                                Row {
                                    id: dotRow
                                    anchors { top: parent.top; horizontalCenter: parent.horizontalCenter }
                                    spacing: 8
                                    Repeater {
                                        model: root.exerciseCount
                                        Rectangle { width: 8; height: 8; radius: 999; color: index <= root.exerciseIndex ? colClay : colTrack }
                                    }
                                }

                                Item {
                                    id: navRow
                                    anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
                                    height: 48

                                    Rectangle {
                                        id: prevBtn
                                        anchors { left: parent.left; verticalCenter: parent.verticalCenter }
                                        width: 36; height: 36; radius: 999; color: "transparent"; border.color: colEdge; border.width: 1
                                        Text { anchors.centerIn: parent; text: "◄"; font.pixelSize: 12; color: colInk2 }
                                        Accessible.role: Accessible.Button; Accessible.name: qsTr("Previous exercise")
                                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { if (bridge != null) bridge.prevExercise() } }
                                    }
                                    Text {
                                        anchors.centerIn: parent; text: root.exerciseTimeStr
                                        font.pixelSize: 34; font.family: "Georgia"; color: colInk
                                    }
                                    Rectangle {
                                        id: nextBtn
                                        anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                                        width: 36; height: 36; radius: 999; color: "transparent"; border.color: colEdge; border.width: 1
                                        Text { anchors.centerIn: parent; text: "►"; font.pixelSize: 12; color: colInk2 }
                                        Accessible.role: Accessible.Button; Accessible.name: qsTr("Next exercise")
                                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { if (bridge != null) bridge.nextExercise() } }
                                    }
                                }

                                Item {
                                    anchors { top: dotRow.bottom; topMargin: 20; bottom: navRow.top; bottomMargin: 16; left: parent.left; right: parent.right }

                                    Item {
                                        id: fsImgContainer
                                        anchors { left: parent.left; verticalCenter: parent.verticalCenter }
                                        width: Math.min(260, parent.height); height: width

                                        Image {
                                            id: fsExImg; anchors.fill: parent
                                            source: root.exerciseImage; mirror: root.exerciseMirror
                                            fillMode: Image.PreserveAspectFit
                                        }
                                        Rectangle {
                                            anchors.fill: parent; color: colSageSoft; radius: 16
                                            visible: fsExImg.status !== Image.Ready || root.exerciseImage === ""
                                        }
                                    }

                                    Column {
                                        anchors { left: fsImgContainer.right; leftMargin: 32; right: parent.right; verticalCenter: parent.verticalCenter }
                                        spacing: 14
                                        Text { width: parent.width; text: root.exerciseName; font.pixelSize: 36; font.family: "Georgia"; color: colInk; wrapMode: Text.Wrap; lineHeight: 1.1 }
                                        Text { width: parent.width; text: root.exerciseDesc; font.pixelSize: 15; color: colInk2; wrapMode: Text.WordWrap; lineHeight: 1.6 }
                                    }
                                }
                            }

                            // Ring view
                            Item {
                                anchors.fill: parent
                                visible: !root.showExercises

                                Canvas {
                                    id: ringCanvas
                                    anchors.centerIn: parent
                                    width: 320; height: 320

                                    property double prog: root.breakProgress
                                    onProgChanged: requestPaint()
                                    Component.onCompleted: requestPaint()

                                    onPaint: {
                                        var ctx = getContext("2d");
                                        ctx.clearRect(0, 0, width, height);
                                        var cx = 160, cy = 160, r = 152, sw = 9;
                                        ctx.beginPath(); ctx.arc(cx, cy, r, 0, 2 * Math.PI); ctx.strokeStyle = colTrack.toString(); ctx.lineWidth = sw; ctx.stroke();
                                        ctx.beginPath(); ctx.arc(cx, cy, r, -Math.PI / 2, -Math.PI / 2 + 2 * Math.PI * Math.max(prog, 0.001)); ctx.strokeStyle = colSage.toString(); ctx.lineWidth = sw; ctx.lineCap = "round"; ctx.stroke();
                                    }
                                }

                                Column {
                                    anchors.centerIn: parent
                                    spacing: 4
                                    Text { anchors.horizontalCenter: parent.horizontalCenter; text: root.breakTimeShort; font.pixelSize: 80; font.family: "Georgia"; color: colInk }
                                    Text { anchors.horizontalCenter: parent.horizontalCenter; visible: root.exercisesDone; text: qsTr("Exercises complete — please relax"); font.pixelSize: 13; color: colMute }
                                    Text { anchors.horizontalCenter: parent.horizontalCenter; visible: !root.hasExercises; text: qsTr("Please stand up and walk away from your computer"); font.pixelSize: 13; color: colMute }
                                    Item { width: 1; height: 16 }
                                    Rectangle {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        visible: root.canSkip; height: 36; width: fsSkipLabel.implicitWidth + 28
                                        radius: 999; color: "transparent"; border.color: colEdge; border.width: 1
                                        Text { id: fsSkipLabel; anchors.centerIn: parent; text: qsTr("End break early"); font.pixelSize: 13; font.weight: Font.Medium; color: colInk2 }
                                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { if (bridge != null) bridge.requestSkip() } }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Full-screen enter animation
        Component.onCompleted: {
            contentArea.opacity = 0
            contentArea.scale   = 0.98
            fsEnterAnim.start()
        }
        SequentialAnimation {
            id: fsEnterAnim
            ParallelAnimation {
                NumberAnimation { target: contentArea; property: "opacity"; to: 1.0; duration: 260; easing.type: Easing.OutCubic }
                NumberAnimation { target: contentArea; property: "scale";   to: 1.0; duration: 260; easing.type: Easing.OutCubic }
            }
        }
    }
}
