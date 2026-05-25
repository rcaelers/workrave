// PreludeOverlay.qml — Sanctuary break-warning card
// Three-column layout: [icon badge] [heading + countdown] [Skip button]
// Loaded by QmlPreludeWindow into a transparent QQuickView (CARD_W × CARD_H).
// Data comes from "bridge" context property (PreludeBridge).

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
    readonly property color colEdge:     Qt.rgba(40/255, 45/255, 38/255, 0.10)
    readonly property color colWarn:     "#D4872A"
    readonly property color colAlert:    "#B85A4A"

    // ── Bridge bindings ───────────────────────────────────────────────────────
    readonly property int    stage:    bridge != null ? bridge.stage    : 0
    readonly property int    breakType: bridge != null ? bridge.breakType : 0
    readonly property string heading:  bridge != null ? bridge.heading  : qsTr("Time for a micro-break?")
    readonly property string countdown: bridge != null
                                        ? (bridge.countdownText + " " + bridge.timeLabel)
                                        : "Break in 0:30"

    // stage: 0=Initial, 1=Warn, 2=Alert, 3=MoveOut
    readonly property color stageAccent: stage === 1 ? colWarn
                                       : stage === 2 ? colAlert
                                       :               colSage

    // ── Icon per break type ───────────────────────────────────────────────────
    function breakIcon(t) { return t === 0 ? "✋" : (t === 1 ? "☕" : "☀") }


    // ── Card ──────────────────────────────────────────────────────────────────
    Rectangle {
        id: card
        anchors.fill: parent
        color: colPanel
        radius: 16
        opacity: 0
        scale: 0.94
        border.width: 1.5
        border.color: colEdge

        // Pulse border width + colour on Warn/Alert — same corner curve, no separate ring needed
        SequentialAnimation {
            running: root.stage === 1 || root.stage === 2
            loops: Animation.Infinite
            ParallelAnimation {
                ColorAnimation  { target: card; property: "border.color"; to: root.stageAccent; duration: root.stage === 1 ? 600 : 280; easing.type: Easing.InOutSine }
                NumberAnimation { target: card; property: "border.width"; to: 5;               duration: root.stage === 1 ? 600 : 280; easing.type: Easing.InOutSine }
            }
            ParallelAnimation {
                ColorAnimation  { target: card; property: "border.color"; to: colEdge; duration: root.stage === 1 ? 600 : 280; easing.type: Easing.InOutSine }
                NumberAnimation { target: card; property: "border.width"; to: 1.5;     duration: root.stage === 1 ? 600 : 280; easing.type: Easing.InOutSine }
            }
            onRunningChanged: {
                if (!running) {
                    card.border.color = root.stage >= 1 && root.stage <= 2 ? root.stageAccent : colEdge
                    card.border.width = 1.5
                }
            }
        }

        // ── Content: icon | text | skip ───────────────────────────────────────
        Item {
            id: contentArea
            anchors {
                fill: parent
                leftMargin: 22; rightMargin: 22
                topMargin: 18;  bottomMargin: 18
            }

            // Icon badge (left)
            Rectangle {
                id: iconBadge
                width: 42; height: 42
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                radius: 999
                color: colSageSoft

                Text {
                    anchors.centerIn: parent
                    text: root.breakIcon(root.breakType)
                    font.pixelSize: 20
                }
            }

            // Text column: heading + countdown (fills space right of icon)
            Column {
                anchors.left: iconBadge.right
                anchors.leftMargin: 18
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 3

                Text {
                    width: parent.width
                    text: root.heading
                    font.pixelSize: 17
                    font.family: "Georgia"
                    font.weight: Font.DemiBold
                    color: colInk
                    elide: Text.ElideRight
                    renderType: Text.NativeRendering
                }

                Text {
                    width: parent.width
                    text: root.countdown
                    font.pixelSize: 12
                    color: colMute
                    elide: Text.ElideRight
                }
            }
        }

        // ── Countdown bar (bottom of card) ────────────────────────────────────
        // Thin pill that drains left→right as time runs out.
        Item {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottomMargin: 10
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            height: 3

            Rectangle {
                anchors.fill: parent
                radius: 999
                color: colEdge
            }

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: parent.width * (bridge != null ? bridge.ringProgress : 1.0)
                radius: 999
                color: root.stageAccent
                Behavior on color { ColorAnimation { duration: 300 } }
            }
        }
    }

    // ── Enter animation ───────────────────────────────────────────────────────
    Component.onCompleted: {
        enterAnim.start()
    }

    SequentialAnimation {
        id: enterAnim
        ParallelAnimation {
            NumberAnimation { target: card; property: "opacity"; to: 1.0; duration: 220; easing.type: Easing.OutCubic }
            NumberAnimation { target: card; property: "scale";   to: 1.0; duration: 220; easing.type: Easing.OutCubic }
        }
    }
}
