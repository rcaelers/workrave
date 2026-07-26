// PreludeClassic.qml — GTK-faithful break-warning card
// Replicates the original Workrave prelude window: icon + bold heading +
// orange progress bar with overlaid text. Border flashes orange (Warn) or
// red (Alert) at 500 ms intervals, matching the original Frame behaviour.
// Fills its parent (positioned by PreludeShell.qml via Loader).

import QtQuick

Item {
    id: root

    // ── Bridge bindings ───────────────────────────────────────────────────────
    readonly property int stage: bridge != null ? bridge.stage : 0

    // ── Flash state ───────────────────────────────────────────────────────────
    // Binary on/off every 500 ms — matches the original GTK Frame widget.
    property bool flashOn: true

    Timer {
        interval: 500
        repeat: true
        running: root.stage === 1 || root.stage === 2
        onTriggered: root.flashOn = !root.flashOn
        onRunningChanged: if (!running) root.flashOn = true
    }

    // ── Card ──────────────────────────────────────────────────────────────────
    Rectangle {
        id: card
        anchors.fill: parent

        color:  "#E8E8E8"
        radius: 2

        // 1 px neutral border in Initial/MoveOut.
        // Thick coloured border flashing on/off in Warn/Alert.
        border.width: (root.stage === 1 || root.stage === 2) && root.flashOn ? 6 : 1
        border.color: root.stage === 1 ? "#F08000"
                    : root.stage === 2 ? "#CC2222"
                    :                    "#8A8885"

        // ── Row: icon | heading + progress bar ───────────────────────────────
        // Gtk metrics: 6px frame + 6px border = 12px padding, 6px box spacing.
        Row {
            anchors {
                fill: parent
                leftMargin: 12; rightMargin: 12
                topMargin:  12; bottomMargin: 12
            }
            spacing: 6

            // Icon: happy in Initial, sad in Warn / Alert / MoveOut
            Image {
                id: hintIcon
                source: root.stage >= 1 ? "qrc:/sanctuary/prelude-hint-sad.png"
                                        : "qrc:/sanctuary/prelude-hint.png"
                width:  48
                height: 48
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            // Right column: heading + progress bar
            Column {
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - hintIcon.width - parent.spacing
                spacing: 6

                Text {
                    width: parent.width
                    text: bridge != null ? bridge.heading : qsTr("Time for a break?")
                    font.bold: true
                    font.pixelSize: 15
                    color: "#1A1A1A"
                    elide: Text.ElideRight
                    renderType: Text.NativeRendering
                }

                // Progress bar — like the Gtk TimeBar: bordered track, "orange"
                // fill, plain black text centred over the whole bar.
                Item {
                    width: parent.width
                    height: 20

                    Rectangle {
                        anchors.fill: parent
                        color: "#D0CBC6"
                        border.color: "#8F8F8F"; border.width: 1
                    }

                    // Elapsed = 1 − remaining (bridge.ringProgress = remaining/max)
                    Rectangle {
                        x: 1; y: 1
                        width: Math.max(0, (parent.width - 2) * (bridge != null ? (1.0 - bridge.ringProgress) : 0.0))
                        height: parent.height - 2
                        color: "#FFA500"
                    }

                    Text {
                        anchors.centerIn: parent
                        text: bridge != null ? bridge.countdownText : qsTr("Break in {}")
                        font.pixelSize: 12
                        color: "#1A1A1A"
                        renderType: Text.NativeRendering
                    }
                }
            }
        }
    }

    // Subtle fade-in when loaded
    Component.onCompleted: {
        card.opacity = 0
        fadeIn.start()
    }

    NumberAnimation {
        id: fadeIn
        target: card
        property: "opacity"
        to: 1.0
        duration: 120
        easing.type: Easing.OutCubic
    }
}
