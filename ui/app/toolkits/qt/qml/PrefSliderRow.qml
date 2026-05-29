import QtQuick

// PrefSliderRow — ticked slider bar. Used standalone or inside PrefTimeControl.
// Properties: value (0..1), sliderColor, ticks (list of {at, label})
// Signals: moved(real value)
Item {
    id: root

    property real   value:       0.5
    property color  sliderColor: tok.sage
    property var    ticks:       []

    signal moved(real value)

    implicitWidth:  parent ? parent.width : 400
    implicitHeight: 46   // 30px track area + 16px tick labels

    PrefTokens { id: tok }

    Item {
        id: trackArea
        anchors { left: parent.left; right: parent.right; top: parent.top }
        height: 30

        // Track background
        Rectangle {
            id: trackBg
            anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
            height: 3
            radius: 99
            color: tok.track
        }

        // Track fill
        Rectangle {
            anchors { left: trackBg.left; top: trackBg.top; bottom: trackBg.bottom }
            width: Math.max(0, trackBg.width * root.value)
            radius: 99
            color: root.sliderColor
        }

        // Thumb
        Rectangle {
            width: 16; height: 16
            radius: 99
            color: "#FFFFFF"
            anchors.verticalCenter: trackBg.verticalCenter
            x: trackBg.x + trackBg.width * root.value - width / 2
            border.color: root.sliderColor
            border.width: 4
        }

        // Tick marks
        Repeater {
            model: root.ticks
            delegate: Rectangle {
                required property var modelData
                x: trackBg.x + trackBg.width * modelData.at - width / 2
                anchors.top: parent.top
                width: 1; height: 6
                color: tok.edge
            }
        }

        // Drag interaction
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.SizeHorCursor
            onPressed:         (mouse) => updateValue(mouse.x)
            onPositionChanged: (mouse) => { if (pressed) updateValue(mouse.x) }
            function updateValue(mx) {
                var v = (mx - trackBg.x) / trackBg.width
                root.value = Math.max(0, Math.min(1, v))
                root.moved(root.value)
            }
        }
    }

    // Tick labels
    Item {
        anchors { left: parent.left; right: parent.right; top: trackArea.bottom }
        height: 16

        Repeater {
            model: root.ticks
            delegate: Text {
                required property var modelData
                x: trackArea.width * modelData.at - implicitWidth / 2
                anchors.top: parent.top
                text: modelData.label
                font.pixelSize: tok.tickPx
                font.family: tok.monoFamily
                font.letterSpacing: 9.5 * 0.04
                color: tok.mute
            }
        }
    }
}
