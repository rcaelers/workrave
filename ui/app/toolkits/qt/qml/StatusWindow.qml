// StatusWindow.qml — main timer window
// Three display style variants: Rings (0), Bars (1), Focus (2).
// Loaded into a QQuickWidget hosted by QmlTimerBoxView.
// Data source: "bridge" context property (StatusWindowBridge).

import QtQuick

Item {
    id: root

    PrefTokens { id: tok }

    // ── Bridge bindings (safe fallbacks while bridge initialises) ─────────────
    // Use != null (loose equality) to guard against both null and undefined.
    readonly property int    displayStyle:    (bridge != null) ? bridge.displayStyle    : 0

    readonly property double microProg:       (bridge != null) ? bridge.microProgress      : 0.7
    readonly property double microIdleProg:   (bridge != null) ? bridge.microIdleProgress  : 0.0
    readonly property string microTime:       (bridge != null) ? bridge.microTime          : "5:00"
    readonly property bool   microVis:        (bridge != null) ? bridge.microVisible       : true
    readonly property bool   microOverdue:    (bridge != null) ? bridge.microOverdue       : false

    readonly property double restProg:        (bridge != null) ? bridge.restProgress       : 0.4
    readonly property double restIdleProg:    (bridge != null) ? bridge.restIdleProgress   : 0.0
    readonly property string restTime:        (bridge != null) ? bridge.restTime           : "10:00"
    readonly property bool   restVis:         (bridge != null) ? bridge.restVisible        : true
    readonly property bool   restOverdue:     (bridge != null) ? bridge.restOverdue        : false

    readonly property double dailyProg:       (bridge != null) ? bridge.dailyProgress      : 0.6
    readonly property double dailyIdleProg:   (bridge != null) ? bridge.dailyIdleProgress  : 0.0
    readonly property string dailyTime:       (bridge != null) ? bridge.dailyTime          : "3:00:00"
    readonly property bool   dailyVis:        (bridge != null) ? bridge.dailyVisible       : true
    readonly property bool   dailyOverdue:    (bridge != null) ? bridge.dailyOverdue       : false

    readonly property int    visibleCount: (microVis ? 1 : 0) + (restVis ? 1 : 0) + (dailyVis ? 1 : 0)

    // ── State → colour helper ─────────────────────────────────────────────────
    function timerColor(overdue, progress) {
        if (overdue)          return tok.danger
        if (progress < 0.25)  return tok.clay
        return tok.sage
    }

    // ── Per-style sizing ─────────────────────────────────────────────────────
    // Rings: 60px per visible timer. Bars: fixed width, height from row count.
    // Focus: fixed 200×100 (always shows all timers).
    readonly property int _n: Math.max(visibleCount, 1)
    // Rings: 60px per visible timer + 12px for 6px margins on each side
    implicitWidth:  displayStyle === 0 ? _n * 60 + 12
                  : (displayStyle === 1 ? 190 : 200)
    // Bars: title(22) + topPad(6) + rows(26 each) + spacing(4 between) + bottomPad(6)
    implicitHeight: displayStyle === 1 ? 22 + 6 + _n * 26 + Math.max(_n - 1, 0) * 4 + 6
                  : (displayStyle === 0 ? 100 : 100)

    // ── Window background card ────────────────────────────────────────────────
    Rectangle {
        anchors.fill: parent
        color: tok.panel
        radius: 12
        border.color: tok.edge
        border.width: 1
    }

    // ── Title chrome (22 px) ──────────────────────────────────────────────────
    Item {
        id: titleBar
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: 22

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            onPressed:         { if (bridge != null) bridge.startWindowDrag() }
            onPositionChanged: { if (pressedButtons & Qt.LeftButton && bridge != null) bridge.continueWindowDrag() }
            cursorShape: Qt.SizeAllCursor
        }

        Rectangle {
            width: 5; height: 5; radius: 999; color: tok.sage
            anchors { left: parent.left; leftMargin: 8; verticalCenter: parent.verticalCenter }
        }

        Text {
            anchors { left: parent.left; leftMargin: 18; verticalCenter: parent.verticalCenter }
            text: "Workrave"
            font.pixelSize: 11; font.italic: true
            font.family: "Georgia"
            color: tok.ink
        }

        Text {
            id: closeX
            anchors { right: parent.right; rightMargin: 8; verticalCenter: parent.verticalCenter }
            text: "×"; font.pixelSize: 13; color: tok.mute; z: 1
            Accessible.role: Accessible.Button; Accessible.name: qsTr("Close")
            MouseArea {
                anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                onClicked: { if (bridge != null) bridge.requestClose() }
            }
            HoverHandler { onHoveredChanged: parent.color = hovered ? tok.ink : tok.mute }
        }

        Rectangle {
            anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
            height: 1; color: tok.edge2
        }
    }

    // ── Content (below title bar) ─────────────────────────────────────────────
    Item {
        id: contentArea
        anchors { top: titleBar.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }

        Loader {
            anchors.fill: parent
            sourceComponent: displayStyle === 0 ? ringsComp : (displayStyle === 1 ? barsComp : focusComp)
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Variant A · Rings
    // ═══════════════════════════════════════════════════════════════════════════
    Component {
        id: ringsComp

        Row {
            anchors { fill: parent; topMargin: 4; bottomMargin: 4; leftMargin: 6; rightMargin: 6 }
            spacing: 0

            Item {
                width: 60; height: parent.height
                visible: microVis
                Column {
                    anchors.centerIn: parent; spacing: 3
                    Text {
                        text: "MICRO"; font.pixelSize: 8; font.letterSpacing: 1.1
                        font.capitalization: Font.AllUppercase; color: tok.mute
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    MiniRing {
                        anchors.horizontalCenter: parent.horizontalCenter
                        accent: root.timerColor(microOverdue, microProg)
                        progress: microProg; idleProgress: microIdleProg
                    }
                    Text {
                        text: microTime; font.pixelSize: 13
                        font.family: "Georgia"
                        color: microOverdue ? tok.danger : tok.ink
                        anchors.horizontalCenter: parent.horizontalCenter
                        renderType: Text.NativeRendering; font.features: { "tnum": 1 }
                    }
                }
            }

            Item {
                width: 60; height: parent.height
                visible: restVis
                Column {
                    anchors.centerIn: parent; spacing: 3
                    Text {
                        text: "REST"; font.pixelSize: 8; font.letterSpacing: 1.1
                        font.capitalization: Font.AllUppercase; color: tok.mute
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    MiniRing {
                        anchors.horizontalCenter: parent.horizontalCenter
                        accent: root.timerColor(restOverdue, restProg)
                        progress: restProg; idleProgress: restIdleProg
                    }
                    Text {
                        text: restTime; font.pixelSize: 13
                        font.family: "Georgia"
                        color: restOverdue ? tok.danger : tok.ink
                        anchors.horizontalCenter: parent.horizontalCenter
                        renderType: Text.NativeRendering; font.features: { "tnum": 1 }
                    }
                }
            }

            Item {
                width: 60; height: parent.height
                visible: dailyVis
                Column {
                    anchors.centerIn: parent; spacing: 3
                    Text {
                        text: "TODAY"; font.pixelSize: 8; font.letterSpacing: 1.1
                        font.capitalization: Font.AllUppercase; color: tok.mute
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    MiniRing {
                        anchors.horizontalCenter: parent.horizontalCenter
                        accent: root.timerColor(dailyOverdue, dailyProg)
                        progress: dailyProg; idleProgress: dailyIdleProg
                    }
                    Text {
                        text: dailyTime; font.pixelSize: 13
                        font.family: "Georgia"
                        color: dailyOverdue ? tok.danger : tok.ink
                        anchors.horizontalCenter: parent.horizontalCenter
                        renderType: Text.NativeRendering; font.features: { "tnum": 1 }
                    }
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Variant B · Bars
    // ═══════════════════════════════════════════════════════════════════════════
    Component {
        id: barsComp

        Column {
            anchors { fill: parent; topMargin: 6; bottomMargin: 6; leftMargin: 12; rightMargin: 12 }
            spacing: 4

            BarRow {
                width: parent.width; iconText: "✋"
                accent: root.timerColor(microOverdue, microProg)
                progress: microProg; idleProgress: microIdleProg
                timeText: microTime; visible: microVis; overdue: microOverdue
            }
            BarRow {
                width: parent.width; iconText: "☕"
                accent: root.timerColor(restOverdue, restProg)
                progress: restProg; idleProgress: restIdleProg
                timeText: restTime; visible: restVis; overdue: restOverdue
                iconClickable: true
                onIconClicked: { if (bridge != null) bridge.forceRestBreak() }
            }
            BarRow {
                width: parent.width; iconText: "☀"
                accent: root.timerColor(dailyOverdue, dailyProg)
                progress: dailyProg; idleProgress: dailyIdleProg
                timeText: dailyTime; visible: dailyVis; overdue: dailyOverdue
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Variant C · Focus
    // ═══════════════════════════════════════════════════════════════════════════
    Component {
        id: focusComp

        Item {
            anchors { fill: parent; topMargin: 10; bottomMargin: 10; leftMargin: 12; rightMargin: 12 }

            Item {
                id: heroRing
                width: 58; height: parent.height
                anchors { left: parent.left; top: parent.top; bottom: parent.bottom }

                Canvas {
                    anchors.fill: parent
                    property color strokeCol: root.timerColor(microOverdue, microProg)
                    property double prog:      microProg
                    property double idleProg:  microIdleProg

                    onStrokeColChanged: requestPaint()
                    onProgChanged:      requestPaint()
                    onIdleProgChanged:  requestPaint()
                    Component.onCompleted: requestPaint()

                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.clearRect(0, 0, width, height);
                        var cx = width / 2, cy = height / 2;

                        var r = 25, sw = 4;
                        ctx.beginPath(); ctx.arc(cx, cy, r, 0, 2 * Math.PI);
                        ctx.strokeStyle = tok.track; ctx.lineWidth = sw; ctx.stroke();

                        ctx.beginPath();
                        ctx.arc(cx, cy, r, -Math.PI / 2,
                                -Math.PI / 2 + 2 * Math.PI * Math.max(prog, 0.001));
                        ctx.strokeStyle = strokeCol.toString();
                        ctx.lineWidth = sw; ctx.lineCap = "round"; ctx.stroke();

                        if (idleProg > 0) {
                            var ri = 17, swi = 2.5;
                            ctx.beginPath(); ctx.arc(cx, cy, ri, 0, 2 * Math.PI);
                            ctx.strokeStyle = tok.track; ctx.lineWidth = swi; ctx.stroke();

                            ctx.beginPath();
                            ctx.arc(cx, cy, ri, -Math.PI / 2,
                                    -Math.PI / 2 + 2 * Math.PI * Math.max(idleProg, 0.001));
                            ctx.strokeStyle = tok.rest;
                            ctx.lineWidth = swi; ctx.lineCap = "round"; ctx.stroke();
                        }
                    }
                }

                Column {
                    anchors.centerIn: parent; spacing: 1
                    Text {
                        text: microTime
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: 17
                        font.family: "Georgia"
                        color: microOverdue ? tok.danger : tok.ink
                        renderType: Text.NativeRendering; font.features: { "tnum": 1 }
                    }
                    Text {
                        text: "MICRO"
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: 8; font.letterSpacing: 1.1
                        font.capitalization: Font.AllUppercase; color: tok.mute
                    }
                }
            }

            Item {
                anchors {
                    left: heroRing.right; leftMargin: 10
                    right: parent.right; top: parent.top; bottom: parent.bottom
                }

                Item {
                    id: restChip
                    anchors { top: parent.top; left: parent.left; right: parent.right }
                    height: parent.height / 2

                    Rectangle {
                        width: 5; height: 5; radius: 999
                        color: root.timerColor(restOverdue, restProg)
                        anchors { left: parent.left; verticalCenter: parent.verticalCenter }
                    }
                    Text {
                        anchors { left: parent.left; leftMargin: 9; verticalCenter: parent.verticalCenter }
                        text: "REST"; font.pixelSize: 9; font.letterSpacing: 1.2
                        font.capitalization: Font.AllUppercase; color: tok.mute
                    }
                    Text {
                        anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                        text: restTime; font.pixelSize: 13
                        font.family: "Georgia"
                        color: restOverdue ? tok.danger : tok.ink
                        renderType: Text.NativeRendering; font.features: { "tnum": 1 }
                    }
                }

                Rectangle {
                    anchors { top: restChip.bottom; left: parent.left; right: parent.right }
                    height: 1; color: tok.edge2
                }

                Item {
                    anchors { top: restChip.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }

                    Rectangle {
                        width: 5; height: 5; radius: 999
                        color: root.timerColor(dailyOverdue, dailyProg)
                        anchors { left: parent.left; verticalCenter: parent.verticalCenter }
                    }
                    Text {
                        anchors { left: parent.left; leftMargin: 9; verticalCenter: parent.verticalCenter }
                        text: "TODAY"; font.pixelSize: 9; font.letterSpacing: 1.2
                        font.capitalization: Font.AllUppercase; color: tok.mute
                    }
                    Text {
                        anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                        text: dailyTime; font.pixelSize: 13
                        font.family: "Georgia"
                        color: dailyOverdue ? tok.danger : tok.ink
                        renderType: Text.NativeRendering; font.features: { "tnum": 1 }
                    }
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Inline reusable components
    // ═══════════════════════════════════════════════════════════════════════════

    component MiniRing: Canvas {
        width: 34; height: 34
        property color accent: tok.sage
        property double progress: 0.5
        property double idleProgress: 0.0

        onAccentChanged:       requestPaint()
        onProgressChanged:     requestPaint()
        onIdleProgressChanged: requestPaint()
        Component.onCompleted: requestPaint()

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            var cx = 17, cy = 17;

            var r = 14.1, sw = 2.8;
            ctx.beginPath(); ctx.arc(cx, cy, r, 0, 2 * Math.PI);
            ctx.strokeStyle = tok.track; ctx.lineWidth = sw; ctx.stroke();

            ctx.beginPath();
            ctx.arc(cx, cy, r, -Math.PI / 2,
                    -Math.PI / 2 + 2 * Math.PI * Math.max(progress, 0.001));
            ctx.strokeStyle = accent.toString();
            ctx.lineWidth = sw; ctx.lineCap = "round"; ctx.stroke();

            if (idleProgress > 0) {
                var ri = 9.0, swi = 2.0;
                ctx.beginPath(); ctx.arc(cx, cy, ri, 0, 2 * Math.PI);
                ctx.strokeStyle = tok.track; ctx.lineWidth = swi; ctx.stroke();

                ctx.beginPath();
                ctx.arc(cx, cy, ri, -Math.PI / 2,
                        -Math.PI / 2 + 2 * Math.PI * Math.max(idleProgress, 0.001));
                ctx.strokeStyle = tok.rest;
                ctx.lineWidth = swi; ctx.lineCap = "round"; ctx.stroke();
            }
        }
    }

    component BarRow: Item {
        height: 26
        property color  accent:        tok.sage
        property string iconText:      "✋"
        property double progress:      0.5
        property double idleProgress:  0.0
        property string timeText:      "5:00"
        property bool   overdue:       false
        property bool   iconClickable: false

        signal iconClicked()

        Text {
            id: barIcon
            anchors { left: parent.left; top: parent.top; topMargin: 3 }
            text: iconText; font.pixelSize: 14; color: accent; width: 20
            horizontalAlignment: Text.AlignHCenter
            opacity: iconArea.containsMouse && iconClickable ? 0.65 : 1.0
            Behavior on opacity { NumberAnimation { duration: 120 } }
        }

        MouseArea {
            id: iconArea
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
            width: 24
            enabled: iconClickable
            hoverEnabled: iconClickable
            cursorShape: iconClickable ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: parent.iconClicked()
        }

        Text {
            id: barTime
            anchors { right: parent.right; top: parent.top; topMargin: 3 }
            text: timeText; font.pixelSize: 13
            font.family: "Georgia"
            color: overdue ? tok.danger : tok.ink
            width: 46; horizontalAlignment: Text.AlignRight
            renderType: Text.NativeRendering; font.features: { "tnum": 1 }
        }

        Item {
            id: primaryBar
            anchors {
                left: barIcon.right; leftMargin: 6
                right: barTime.left; rightMargin: 6
                top: parent.top; topMargin: 5
            }
            height: 4

            Rectangle { anchors.fill: parent; radius: 99; color: tok.track }
            Rectangle {
                anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
                width: Math.max(2, parent.width * progress); radius: 99; color: accent
                Behavior on width { NumberAnimation { duration: 800; easing.type: Easing.OutQuad } }
            }
        }

        Item {
            visible: idleProgress > 0
            anchors {
                left: barIcon.right; leftMargin: 6
                right: barTime.left; rightMargin: 6
                top: primaryBar.bottom; topMargin: 3
            }
            height: 2

            Rectangle { anchors.fill: parent; radius: 99; color: tok.track }
            Rectangle {
                anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
                width: Math.max(1, parent.width * idleProgress); radius: 99; color: tok.rest
                Behavior on width { NumberAnimation { duration: 800; easing.type: Easing.OutQuad } }
            }
        }
    }
}
