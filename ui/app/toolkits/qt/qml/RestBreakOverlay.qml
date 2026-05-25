// RestBreakOverlay.qml — Sanctuary-style rest-break window
// Loaded by QmlRestBreakWindow into a QQuickView.
// All data comes from the C++ "bridge" context property (RestBreakBridge).

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
    readonly property double     breakProgress:   bridge != null ? bridge.breakProgress      : 1.0
    readonly property string     breakTimeShort:  bridge != null ? bridge.breakTimeShort     : "5:00"
    readonly property string     breakMaxStr:     bridge != null ? bridge.breakMaxStr        : "10:00"

    // True while exercises are actively running
    readonly property bool showExercises: root.hasExercises && !root.exercisesDone

    // ── Warm background (all block modes except toast) ────────────────────────
    Rectangle {
        anchors.fill: parent
        color: colBg
        visible: root.blockMode !== 0
        z: 0
    }

    // ── Content area ──────────────────────────────────────────────────────────
    Item {
        id: contentArea
        anchors {
            fill: parent
            leftMargin: 48
            rightMargin: 48
            topMargin: 32
            bottomMargin: 40
        }
        z: 1

        // ── Header strip ──────────────────────────────────────────────────────
        Item {
            id: headerStrip
            anchors { top: parent.top; left: parent.left; right: parent.right }
            height: 36

            // Left: brand dot + breadcrumb
            Row {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                Rectangle {
                    width: 6; height: 6
                    radius: 999
                    color: colClay
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("REST BREAK") + " · " + (root.exerciseIndex + 1) + " " + qsTr("OF") + " " + root.exerciseCount
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                    font.letterSpacing: 0.5
                    color: colMute
                }
            }

            // Right: lock, divider, Postpone, Skip
            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4

                Rectangle {
                    visible: root.lockable
                    width: 28; height: 28
                    radius: 999
                    color: "transparent"
                    border.color: colEdge
                    border.width: 1
                    anchors.verticalCenter: parent.verticalCenter

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

                // Vertical divider
                Rectangle {
                    width: 1; height: 18
                    color: colEdge
                    anchors.verticalCenter: parent.verticalCenter
                    visible: root.lockable && (root.canPostpone || root.canSkip)
                }

                Rectangle {
                    visible: root.canPostpone
                    enabled: root.canPostpone
                    height: 28
                    width: postponeLabel.implicitWidth + 20
                    radius: 999
                    color: "transparent"
                    border.color: colEdge
                    border.width: 1
                    opacity: enabled ? 1.0 : 0.4
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        id: postponeLabel
                        anchors.centerIn: parent
                        text: qsTr("Postpone")
                        font.pixelSize: 12
                        font.weight: Font.Medium
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

                Text {
                    visible: root.canSkip
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

        // ── Body area ─────────────────────────────────────────────────────────
        Item {
            id: bodyArea
            anchors {
                top: headerStrip.bottom
                topMargin: 20
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            // Max-width container so the layout doesn't stretch on wide monitors
            Item {
                id: threeCol
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    top: parent.top
                    bottom: parent.bottom
                }
                width: Math.min(parent.width, 1060)

            // ── Left column: exercise sequence list ───────────────────────────
            Item {
                id: seqPanel
                visible: root.showExercises
                anchors { top: parent.top; left: parent.left; bottom: parent.bottom }
                width: root.showExercises ? 220 : 0

                Column {
                    anchors { verticalCenter: parent.verticalCenter; left: parent.left; right: parent.right }
                    spacing: 2

                    Text {
                        text: qsTr("Exercises")
                        font.pixelSize: 14
                        font.italic: true
                        font.family: "Georgia"
                        color: colMute
                        bottomPadding: 10
                    }

                    Repeater {
                        model: root.exerciseNames

                        delegate: Item {
                            id: exRow
                            width: seqPanel.width
                            height: 52

                            property bool isDone:    root.exercisesDone || index < root.exerciseIndex
                            property bool isCurrent: !root.exercisesDone && index === root.exerciseIndex

                            // Current-row card highlight
                            Rectangle {
                                anchors { fill: parent; leftMargin: -4; rightMargin: -4 }
                                radius: 10
                                color: colPanel
                                border.color: colEdge
                                border.width: 0.5
                                visible: exRow.isCurrent
                            }

                            Row {
                                anchors {
                                    left: parent.left
                                    right: parent.right
                                    verticalCenter: parent.verticalCenter
                                    leftMargin: 8
                                    rightMargin: 8
                                }
                                spacing: 10

                                // Icon box
                                Rectangle {
                                    width: 32; height: 32
                                    radius: 8
                                    color: exRow.isDone ? colSageSoft : (exRow.isCurrent ? colClaySoft : "transparent")
                                    border.color: (!exRow.isDone && !exRow.isCurrent) ? colEdge : "transparent"
                                    border.width: 1
                                    anchors.verticalCenter: parent.verticalCenter

                                    Text {
                                        anchors.centerIn: parent
                                        text: exRow.isDone ? "✓" : (exRow.isCurrent ? "●" : "○")
                                        font.pixelSize: 13
                                        color: exRow.isDone ? colSage : (exRow.isCurrent ? colClay : colMute)
                                    }
                                }

                                // Exercise name
                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: parent.width - 32 - 10
                                    text: modelData
                                    font.pixelSize: 13
                                    font.weight: exRow.isCurrent ? Font.Medium : Font.Normal
                                    font.strikeout: exRow.isDone
                                    color: exRow.isDone ? colMute : (exRow.isCurrent ? colInk : colInk2)
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }
                }
            }

            // ── Right column: total-break timer (hidden when ring view is shown) ─
            Item {
                id: timerPanel
                visible: root.showExercises
                anchors { top: parent.top; right: parent.right; bottom: parent.bottom }
                width: root.showExercises ? 160 : 0

                Column {
                    anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                    spacing: 0

                    Text {
                        anchors.right: parent.right
                        text: qsTr("Total left")
                        font.pixelSize: 13
                        font.italic: true
                        font.family: "Georgia"
                        color: colMute
                        bottomPadding: 4
                    }

                    Text {
                        anchors.right: parent.right
                        text: root.breakTimeShort
                        font.pixelSize: 58
                        font.family: "Georgia"
                        color: colInk
                        lineHeight: 1.0
                    }

                    Text {
                        anchors.right: parent.right
                        text: "OF " + root.breakMaxStr
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                        font.letterSpacing: 1.4
                        color: colMute
                        topPadding: 4
                        bottomPadding: 10
                    }

                    // Progress bar
                    Rectangle {
                        anchors.right: parent.right
                        width: parent.width
                        height: 4
                        radius: 2
                        color: colTrack

                        Rectangle {
                            anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
                            width: Math.max(4, parent.width * root.breakProgress)
                            radius: 2
                            color: colClay
                        }
                    }
                }
            }

            // ── Center column: exercise card ──────────────────────────────────
            Item {
                id: centerCol
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                    left: root.showExercises ? seqPanel.right : parent.left
                    leftMargin: root.showExercises ? 12 : 0
                    right: timerPanel.left
                    rightMargin: 20
                }

                // White exercise card — vertically centered, not full-height
                Rectangle {
                    id: exerciseCard
                    anchors {
                        left: parent.left
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                    }
                    height: Math.min(parent.height - 24, 680)
                    radius: 24
                    color: colPanel
                    border.color: colEdge
                    border.width: 0.5

                    // ── Active exercise view ──────────────────────────────────
                    Item {
                        id: activeView
                        anchors {
                            fill: parent
                            topMargin: 32
                            leftMargin: 40
                            rightMargin: 40
                            bottomMargin: 32
                        }
                        visible: root.showExercises

                        // Dot progress row (top)
                        Row {
                            id: dotRow
                            anchors { top: parent.top; horizontalCenter: parent.horizontalCenter }
                            spacing: 8

                            Repeater {
                                model: root.exerciseCount
                                Rectangle {
                                    width: 8; height: 8
                                    radius: 999
                                    color: index <= root.exerciseIndex ? colClay : colTrack
                                }
                            }
                        }

                        // Navigation row (bottom)
                        Item {
                            id: navRow
                            anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
                            height: 48

                            // Prev button
                            Rectangle {
                                id: prevBtn
                                anchors { left: parent.left; verticalCenter: parent.verticalCenter }
                                width: 36; height: 36
                                radius: 999
                                color: "transparent"
                                border.color: colEdge
                                border.width: 1

                                Text {
                                    anchors.centerIn: parent
                                    text: "◄"
                                    font.pixelSize: 12
                                    color: colInk2
                                }

                                Accessible.role: Accessible.Button
                                Accessible.name: qsTr("Previous exercise")

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: { if (bridge != null) bridge.prevExercise() }
                                }
                            }

                            // Exercise timer (center)
                            Text {
                                anchors.centerIn: parent
                                text: root.exerciseTimeStr
                                font.pixelSize: 34
                                font.family: "Georgia"
                                color: colInk
                            }

                            // Next button
                            Rectangle {
                                id: nextBtn
                                anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                                width: 36; height: 36
                                radius: 999
                                color: "transparent"
                                border.color: colEdge
                                border.width: 1

                                Text {
                                    anchors.centerIn: parent
                                    text: "►"
                                    font.pixelSize: 12
                                    color: colInk2
                                }

                                Accessible.role: Accessible.Button
                                Accessible.name: qsTr("Next exercise")

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: { if (bridge != null) bridge.nextExercise() }
                                }
                            }
                        }

                        // Image + text row (fills space between dots and nav)
                        Item {
                            anchors {
                                top: dotRow.bottom
                                topMargin: 20
                                bottom: navRow.top
                                bottomMargin: 16
                                left: parent.left
                                right: parent.right
                            }

                            // Exercise image (left, 260×260 or fills height)
                            Item {
                                id: imgContainer
                                anchors { left: parent.left; verticalCenter: parent.verticalCenter }
                                width: Math.min(260, parent.height)
                                height: width

                                Image {
                                    id: exImg
                                    anchors.fill: parent
                                    source: root.exerciseImage
                                    mirror: root.exerciseMirror
                                    fillMode: Image.PreserveAspectFit
                                }

                                Rectangle {
                                    anchors.fill: parent
                                    color: colSageSoft
                                    radius: 16
                                    visible: exImg.status !== Image.Ready || root.exerciseImage === ""
                                }
                            }

                            // Name + description (right of image)
                            Column {
                                anchors {
                                    left: imgContainer.right
                                    leftMargin: 32
                                    right: parent.right
                                    verticalCenter: parent.verticalCenter
                                }
                                spacing: 14

                                Text {
                                    width: parent.width
                                    text: root.exerciseName
                                    font.pixelSize: 36
                                    font.family: "Georgia"
                                    color: colInk
                                    wrapMode: Text.Wrap
                                    lineHeight: 1.1
                                }

                                Text {
                                    width: parent.width
                                    text: root.exerciseDesc
                                    font.pixelSize: 15
                                    color: colInk2
                                    wrapMode: Text.WordWrap
                                    lineHeight: 1.6
                                }
                            }
                        }
                    }

                    // ── Done / no-exercises ring view ─────────────────────────
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

                                ctx.beginPath();
                                ctx.arc(cx, cy, r, 0, 2 * Math.PI);
                                ctx.strokeStyle = colTrack.toString();
                                ctx.lineWidth = sw;
                                ctx.stroke();

                                ctx.beginPath();
                                ctx.arc(cx, cy, r, -Math.PI / 2,
                                        -Math.PI / 2 + 2 * Math.PI * Math.max(prog, 0.001));
                                ctx.strokeStyle = colSage.toString();
                                ctx.lineWidth = sw;
                                ctx.lineCap = "round";
                                ctx.stroke();
                            }
                        }

                        Column {
                            anchors.centerIn: parent
                            spacing: 4

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: root.breakTimeShort
                                font.pixelSize: 80
                                font.family: "Georgia"
                                color: colInk
                            }

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                visible: root.exercisesDone
                                text: qsTr("Exercises complete — please relax")
                                font.pixelSize: 13
                                color: colMute
                            }

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                visible: !root.hasExercises
                                text: qsTr("Please stand up and walk away from your computer")
                                font.pixelSize: 13
                                color: colMute
                            }

                            // Skip / end-break button
                            Item { width: 1; height: 16 }

                            Rectangle {
                                anchors.horizontalCenter: parent.horizontalCenter
                                visible: root.canSkip
                                height: 36
                                width: skipLabel.implicitWidth + 28
                                radius: 999
                                color: "transparent"
                                border.color: colEdge
                                border.width: 1

                                Text {
                                    id: skipLabel
                                    anchors.centerIn: parent
                                    text: qsTr("End break early")
                                    font.pixelSize: 13
                                    font.weight: Font.Medium
                                    color: colInk2
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: { if (bridge != null) bridge.requestSkip() }
                                }
                            }
                        }
                    }
                }
            }
            } // threeCol
        }
    }

    // ── Card enter animation ──────────────────────────────────────────────────
    Component.onCompleted: {
        contentArea.opacity = 0
        contentArea.scale   = 0.98
        enterAnim.start()
    }

    SequentialAnimation {
        id: enterAnim
        ParallelAnimation {
            NumberAnimation { target: contentArea; property: "opacity"; to: 1.0; duration: 260; easing.type: Easing.OutCubic }
            NumberAnimation { target: contentArea; property: "scale";   to: 1.0; duration: 260; easing.type: Easing.OutCubic }
        }
    }
}
