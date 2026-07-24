// PreludeOverlay.qml — Sanctuary break-warning card
// Fills its parent (positioned by PreludeShell.qml via Loader).
// Data comes from "bridge" context property (PreludeBridge).

import QtQuick

Item {
    id: root

    PrefTokens { id: tok }

    // ── Bridge bindings ───────────────────────────────────────────────────────
    readonly property int    stage:    bridge != null ? bridge.stage    : 0
    readonly property int    breakType: bridge != null ? bridge.breakType : 0
    readonly property string heading:  bridge != null ? bridge.heading  : qsTr("Time for a micro-break?")
    readonly property string countdown: bridge != null ? bridge.countdownText : qsTr("Break in {}")

    // stage: 0=Initial, 1=Warn, 2=Alert, 3=MoveOut
    readonly property color stageAccent: stage === 1 ? tok.warn
                                       : stage === 2 ? tok.danger
                                       :               tok.sage

    // ── Icon per break type ───────────────────────────────────────────────────
    function breakIcon(t) { return t === 0 ? "✋" : (t === 1 ? "☕" : "☀") }

    // ── Card ──────────────────────────────────────────────────────────────────
    Rectangle {
        id: card
        anchors.fill: parent
        color: tok.panel
        radius: 16
        opacity: 0
        scale: 0.94
        border.width: 1.5
        border.color: tok.edge

        // Pulse border width + colour on Warn/Alert
        SequentialAnimation {
            running: root.stage === 1 || root.stage === 2
            loops: Animation.Infinite
            ParallelAnimation {
                ColorAnimation  { target: card; property: "border.color"; to: root.stageAccent; duration: root.stage === 1 ? 600 : 280; easing.type: Easing.InOutSine }
                NumberAnimation { target: card; property: "border.width"; to: 5;               duration: root.stage === 1 ? 600 : 280; easing.type: Easing.InOutSine }
            }
            ParallelAnimation {
                ColorAnimation  { target: card; property: "border.color"; to: tok.edge; duration: root.stage === 1 ? 600 : 280; easing.type: Easing.InOutSine }
                NumberAnimation { target: card; property: "border.width"; to: 1.5;     duration: root.stage === 1 ? 600 : 280; easing.type: Easing.InOutSine }
            }
            onRunningChanged: {
                if (!running) {
                    card.border.color = root.stage >= 1 && root.stage <= 2 ? root.stageAccent : tok.edge
                    card.border.width = 1.5
                }
            }
        }

        // ── Content: icon | text ──────────────────────────────────────────────
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
                color: tok.sageSoft

                Text {
                    anchors.centerIn: parent
                    text: root.breakIcon(root.breakType)
                    font.pixelSize: 20
                }
            }

            // Text column: heading + countdown
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
                    color: tok.ink
                    elide: Text.ElideRight
                    renderType: Text.NativeRendering
                }

                Text {
                    width: parent.width
                    text: root.countdown
                    font.pixelSize: 12
                    color: tok.mute
                    elide: Text.ElideRight
                }
            }
        }

        // ── Countdown bar (bottom of card) ────────────────────────────────────
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
                color: tok.edge
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
