import QtQuick

// TimerStylePreview — small schematic preview of a timer display style.
// kind: "rings" | "bars" | "focus" | "classic"
Item {
    property string kind: "rings"

    implicitWidth: 140
    implicitHeight: 60

    PrefTokens { id: tok }

    // Rings: three arcs drawn on Canvas
    Row {
        visible: kind === "rings"
        anchors.centerIn: parent
        spacing: 10

        Repeater {
            model: [
                { ringColor: tok.sage.toString(),     progress: 0.65 },
                { ringColor: tok.clay.toString(),     progress: 0.30 },
                { ringColor: tok.sageDeep.toString(), progress: 0.85 },
            ]

            Canvas {
                required property var modelData
                width: 34; height: 34
                Connections { target: tok; function onTrackChanged() { requestPaint() } }
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)
                    ctx.lineWidth = 4
                    ctx.strokeStyle = tok.track.toString()
                    ctx.beginPath()
                    ctx.arc(17, 17, 13, 0, 2 * Math.PI)
                    ctx.stroke()
                    ctx.strokeStyle = modelData.ringColor
                    ctx.lineCap = "round"
                    ctx.beginPath()
                    ctx.arc(17, 17, 13,
                            -Math.PI / 2,
                            -Math.PI / 2 + 2 * Math.PI * modelData.progress)
                    ctx.stroke()
                }
            }
        }
    }

    // Bars: three rows of icon + bar + time
    Column {
        visible: kind === "bars"
        anchors.centerIn: parent
        spacing: 9

        Repeater {
            model: [
                { barColor: tok.sage.toString(),     pct: 0.65, time: "3:45"  },
                { barColor: tok.clay.toString(),     pct: 0.30, time: "12:20" },
                { barColor: tok.sageDeep.toString(), pct: 0.85, time: "1:05"  },
            ]

            Row {
                required property var modelData
                spacing: 6

                Rectangle {
                    width: 10; height: 10; radius: 2
                    color: modelData.barColor
                    anchors.verticalCenter: parent.verticalCenter
                }

                Item {
                    width: 52; height: 4
                    anchors.verticalCenter: parent.verticalCenter

                    Rectangle {
                        anchors.fill: parent; radius: 2
                        color: tok.track
                    }
                    Rectangle {
                        width: parent.width * modelData.pct; height: parent.height
                        radius: 2; color: modelData.barColor
                    }
                }

                Text {
                    text: modelData.time
                    font.pixelSize: 8; font.family: tok.monoFamily
                    color: tok.ink2
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    // Classic: three text rows mimicking the legacy widget
    Column {
        visible: kind === "classic"
        anchors.centerIn: parent
        spacing: 6

        Repeater {
            model: [
                { dotColor: tok.sage.toString(),     label: "3:45"  },
                { dotColor: tok.clay.toString(),     label: "12:20" },
                { dotColor: tok.sageDeep.toString(), label: "1:05"  },
            ]

            Row {
                required property var modelData
                spacing: 5

                Rectangle {
                    width: 6; height: 6; radius: 3
                    color: modelData.dotColor
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: modelData.label
                    font.pixelSize: 9; font.family: tok.monoFamily
                    color: tok.ink
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    // Focus: large ring for active timer + two small chips
    Row {
        visible: kind === "focus"
        anchors.centerIn: parent
        spacing: 10

        Canvas {
            width: 46; height: 46
            anchors.verticalCenter: parent.verticalCenter
            Connections { target: tok; function onTrackChanged() { requestPaint() } }
            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
                ctx.lineWidth = 5
                ctx.strokeStyle = tok.track.toString()
                ctx.beginPath()
                ctx.arc(23, 23, 18, 0, 2 * Math.PI)
                ctx.stroke()
                ctx.strokeStyle = tok.sage.toString()
                ctx.lineCap = "round"
                ctx.beginPath()
                ctx.arc(23, 23, 18, -Math.PI / 2, -Math.PI / 2 + 2 * Math.PI * 0.65)
                ctx.stroke()
            }
        }

        Column {
            spacing: 7
            anchors.verticalCenter: parent.verticalCenter

            Repeater {
                model: [
                    { chipColor: tok.clay.toString(),     time: "12:20" },
                    { chipColor: tok.sageDeep.toString(), time: "1:05"  },
                ]

                Rectangle {
                    required property var modelData
                    width: 62; height: 18; radius: 9
                    color: tok.panel2
                    border.color: tok.edge
                    border.width: 1

                    Row {
                        anchors { left: parent.left; leftMargin: 6; verticalCenter: parent.verticalCenter }
                        spacing: 4

                        Rectangle {
                            width: 6; height: 6; radius: 3
                            color: modelData.chipColor
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: modelData.time
                            font.pixelSize: 7; font.family: tok.monoFamily
                            color: tok.ink2
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }
        }
    }
}
