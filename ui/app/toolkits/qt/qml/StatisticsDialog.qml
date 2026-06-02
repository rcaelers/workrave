// StatisticsDialog.qml — Statistics history browser.
// Loaded by QmlStatisticsDialog via QQuickView.
// All data comes from the C++ "statsBridge" context property (StatisticsBridge).

import QtQuick
import QtQuick.Controls.Basic

Item {
    id: root

    signal closeRequested()

    PrefTokens { id: tok }

    // ── Confirm overlay (z:100) ───────────────────────────────────────────────
    ConfirmDialog {
        id: confirmDlg
        anchors.fill: parent
        z: 100
        onConfirmed: (action) => {
            if (action === "delete") {
                statsBridge.deleteAllHistory()
            }
        }
    }

    // ── Root background ───────────────────────────────────────────────────────
    Rectangle {
        anchors.fill: parent
        color: tok.bg

        // ── Layout: sidebar | vertical separator | main content ───────────────
        Row {
            id: bodyRow
            anchors { top: parent.top; left: parent.left; right: parent.right; bottom: bottomBar.top }

            // ── Left sidebar ──────────────────────────────────────────────────
            Rectangle {
                id: sidebar
                width: 256
                height: parent.height
                color: tok.panel

                Column {
                    anchors { fill: parent; margins: 8 }
                    spacing: 0

                    // ── Month navigation header ───────────────────────────────
                    Item {
                        width: parent.width
                        height: 36

                        NavButton {
                            id: prevMonthBtn
                            anchors { left: parent.left; verticalCenter: parent.verticalCenter }
                            label: "◀"
                            onClicked: statsBridge.prevMonth()
                        }

                        Text {
                            anchors.centerIn: parent
                            text: statsBridge != null ? statsBridge.calendarMonthYearText : ""
                            font.pixelSize: 13
                            font.weight: Font.Medium
                            color: tok.ink
                        }

                        NavButton {
                            id: nextMonthBtn
                            anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                            label: "▶"
                            onClicked: statsBridge.nextMonth()
                        }
                    }

                    // ── Day-of-week header row ────────────────────────────────
                    Item {
                        width: parent.width
                        height: 24

                        Row {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 0

                            Repeater {
                                model: [ "M", "T", "W", "T", "F", "S", "S" ]
                                delegate: Text {
                                    required property string modelData
                                    width: Math.floor((sidebar.width - 16) / 7)
                                    height: 24
                                    text: modelData
                                    font.pixelSize: 11
                                    font.weight: Font.DemiBold
                                    color: tok.mute
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                        }
                    }

                    // ── Calendar grid ─────────────────────────────────────────
                    Grid {
                        id: calGrid
                        width: parent.width
                        columns: 7

                        property int cellW: Math.floor((sidebar.width - 16) / 7)
                        property int cellH: 30

                        Repeater {
                            model: statsBridge != null ? statsBridge.calendarCells : []

                            delegate: Item {
                                required property var modelData
                                width: calGrid.cellW
                                height: calGrid.cellH

                                // Skip empty cells
                                visible: modelData.day > 0

                                Rectangle {
                                    anchors.centerIn: parent
                                    width: calGrid.cellW - 2
                                    height: calGrid.cellH - 2
                                    radius: 4
                                    color: modelData.isSelected ? tok.sageSoft : "transparent"

                                    // Data indicator dot
                                    Rectangle {
                                        visible: modelData.hasData && !modelData.isSelected
                                        anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter; bottomMargin: 2 }
                                        width: 3; height: 3; radius: 2
                                        color: tok.sage
                                    }

                                    Text {
                                        anchors.centerIn: parent
                                        text: modelData.day > 0 ? modelData.day : ""
                                        font.pixelSize: 12
                                        font.weight: modelData.isSelected ? Font.DemiBold : Font.Normal
                                        color: modelData.isSelected ? tok.sageDeep : (modelData.hasData ? tok.ink : tok.mute)
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: modelData.hasData ? Qt.PointingHandCursor : Qt.ArrowCursor
                                    enabled: modelData.day > 0
                                    onClicked: {
                                        if (statsBridge != null) {
                                            statsBridge.selectDate(
                                                statsBridge.calendarYear,
                                                statsBridge.calendarMonth,
                                                modelData.day)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // ── Separator ─────────────────────────────────────────────
                    Rectangle {
                        width: parent.width
                        height: 1
                        color: tok.edge
                        topPadding: 4
                        bottomPadding: 4
                    }

                    Item { width: 1; height: 8 }

                    // ── History navigation buttons ────────────────────────────
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 6

                        NavButton {
                            label: "⏮"
                            enabled: statsBridge != null && statsBridge.canGoFirst
                            onClicked: if (statsBridge != null) statsBridge.goFirst()
                        }
                        NavButton {
                            label: "◀"
                            enabled: statsBridge != null && statsBridge.canGoBack
                            onClicked: if (statsBridge != null) statsBridge.goBack()
                        }
                        NavButton {
                            label: "▶"
                            enabled: statsBridge != null && statsBridge.canGoForward
                            onClicked: if (statsBridge != null) statsBridge.goForward()
                        }
                        NavButton {
                            label: "⏭"
                            enabled: statsBridge != null && statsBridge.canGoLast
                            onClicked: if (statsBridge != null) statsBridge.goLast()
                        }
                    }

                    Item { width: 1; height: 8 }

                    // ── Separator ─────────────────────────────────────────────
                    Rectangle {
                        width: parent.width
                        height: 1
                        color: tok.edge
                    }

                    Item { width: 1; height: 8 }

                    // ── Delete history button ─────────────────────────────────
                    Rectangle {
                        id: deleteBtn
                        anchors.horizontalCenter: parent.horizontalCenter
                        height: 30
                        width: Math.max(deleteLbl.implicitWidth + 24, parent.width - 16)
                        radius: 2
                        color: deleteBtnArea.containsMouse ? tok.dangerSoft : tok.panel2
                        border.color: tok.danger
                        border.width: 1

                        Text {
                            id: deleteLbl
                            anchors.centerIn: parent
                            text: qsTr("Delete history")
                            font.pixelSize: 12
                            color: tok.danger
                        }

                        MouseArea {
                            id: deleteBtnArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                confirmDlg.ask("delete",
                                               qsTr("Delete history"),
                                               qsTr("You have chosen to delete your statistics history. Continue?"))
                            }
                        }
                    }
                }

                // Right border of sidebar
                Rectangle {
                    anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
                    width: 1
                    color: tok.edge
                }
            }

            // ── Main content area ─────────────────────────────────────────────
            Item {
                id: mainContent
                width: parent.width - sidebar.width
                height: parent.height

                Column {
                    anchors { fill: parent; margins: 16 }
                    spacing: 8

                    // ── Title ─────────────────────────────────────────────────
                    Text {
                        text: qsTr("Statistics")
                        font.pixelSize: 18
                        font.bold: true
                        color: tok.ink
                    }

                    Text {
                        id: dateText
                        width: parent.width
                        text: (statsBridge != null && statsBridge.selectedDateText !== "")
                              ? statsBridge.selectedDateText
                              : "—"
                        font.pixelSize: 12
                        color: tok.ink2
                        elide: Text.ElideRight
                    }

                    // ── Separator ─────────────────────────────────────────────
                    Rectangle {
                        width: parent.width
                        height: 1
                        color: tok.edge
                    }

                    // ── Breaks content ────────────────────────────────────
                    Item {
                        id: tabContent
                        width: parent.width
                        height: mainContent.height - dateText.y - dateText.height - 42

                        Column {
                                anchors.fill: parent
                                spacing: 0

                                // Header row: labels + break type icons
                                Row {
                                    id: breaksHeader
                                    spacing: 0
                                    width: parent.width

                                    Item { width: 180; height: 28 } // label column spacer

                                    Repeater {
                                        model: [
                                            "qrc:/sanctuary/micro-break.png",
                                            "qrc:/sanctuary/rest-break.png",
                                            "qrc:/sanctuary/daily-limit.png"
                                        ]
                                        delegate: Item {
                                            required property string modelData
                                            width: 80; height: 28
                                            Image {
                                                anchors.centerIn: parent
                                                source: modelData
                                                width: 20; height: 20
                                                fillMode: Image.PreserveAspectFit
                                                smooth: true
                                            }
                                        }
                                    }
                                }

                                // Data rows
                                Repeater {
                                    model: statsBridge != null ? statsBridge.breakStats : []

                                    delegate: Rectangle {
                                        required property var modelData
                                        required property int index
                                        width: tabContent.width
                                        height: 26
                                        color: (index % 2 === 1)
                                               ? Qt.rgba(tok.edge.r, tok.edge.g, tok.edge.b, 0.30)
                                               : "transparent"

                                        Row {
                                            anchors { fill: parent; leftMargin: 0 }
                                            spacing: 0

                                            // Label + tooltip
                                            Item {
                                                width: 180; height: parent.height

                                                Text {
                                                    id: rowLabel
                                                    anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: 4 }
                                                    text: modelData.label
                                                    font.pixelSize: 12
                                                    color: tok.ink2
                                                    elide: Text.ElideRight
                                                }

                                                MouseArea {
                                                    id: labelHover
                                                    anchors.fill: parent
                                                    hoverEnabled: true
                                                }

                                                ToolTip.visible: labelHover.containsMouse
                                                ToolTip.text: modelData.tooltip
                                                ToolTip.delay: 500
                                            }

                                            // micro
                                            Text {
                                                width: 80; height: parent.height
                                                text: modelData.micro
                                                font.pixelSize: 12
                                                color: tok.ink
                                                horizontalAlignment: Text.AlignRight
                                                verticalAlignment: Text.AlignVCenter
                                                rightPadding: 8
                                            }

                                            // rest
                                            Text {
                                                width: 80; height: parent.height
                                                text: modelData.rest
                                                font.pixelSize: 12
                                                color: tok.ink
                                                horizontalAlignment: Text.AlignRight
                                                verticalAlignment: Text.AlignVCenter
                                                rightPadding: 8
                                            }

                                            // daily
                                            Text {
                                                width: 80; height: parent.height
                                                text: modelData.daily
                                                font.pixelSize: 12
                                                color: tok.ink
                                                horizontalAlignment: Text.AlignRight
                                                verticalAlignment: Text.AlignVCenter
                                                rightPadding: 8
                                            }
                                        }
                                    }
                                }

                                // Separator
                                Rectangle {
                                    width: parent.width
                                    height: 1
                                    color: tok.edge
                                }

                                Item { width: 1; height: 6 }

                                // Usage header
                                Row {
                                    spacing: 0
                                    width: parent.width
                                    height: 24

                                    Text {
                                        width: 180; height: parent.height
                                        text: qsTr("Usage")
                                        font.pixelSize: 12
                                        font.bold: true
                                        color: tok.ink
                                        verticalAlignment: Text.AlignVCenter
                                        leftPadding: 4
                                    }

                                    Repeater {
                                        model: [ qsTr("Daily"), qsTr("Weekly"), qsTr("Monthly") ]
                                        delegate: Text {
                                            required property string modelData
                                            width: 80; height: parent.height
                                            text: modelData
                                            font.pixelSize: 12
                                            font.bold: true
                                            color: tok.ink
                                            horizontalAlignment: Text.AlignRight
                                            verticalAlignment: Text.AlignVCenter
                                            rightPadding: 8
                                        }
                                    }
                                }

                                // Usage data row
                                Row {
                                    spacing: 0
                                    width: parent.width
                                    height: 26

                                    Item { width: 180; height: parent.height }

                                    Text {
                                        width: 80; height: parent.height
                                        text: statsBridge != null ? statsBridge.dailyUsage : ""
                                        font.pixelSize: 12
                                        color: tok.ink
                                        horizontalAlignment: Text.AlignRight
                                        verticalAlignment: Text.AlignVCenter
                                        rightPadding: 8
                                    }
                                    Text {
                                        width: 80; height: parent.height
                                        text: statsBridge != null ? statsBridge.weeklyUsage : ""
                                        font.pixelSize: 12
                                        color: tok.ink
                                        horizontalAlignment: Text.AlignRight
                                        verticalAlignment: Text.AlignVCenter
                                        rightPadding: 8
                                    }
                                    Text {
                                        width: 80; height: parent.height
                                        text: statsBridge != null ? statsBridge.monthlyUsage : ""
                                        font.pixelSize: 12
                                        color: tok.ink
                                        horizontalAlignment: Text.AlignRight
                                        verticalAlignment: Text.AlignVCenter
                                        rightPadding: 8
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // ── Bottom bar ────────────────────────────────────────────────────────
        Rectangle {
            id: bottomBar
            anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
            height: 48
            color: tok.panel

            Rectangle {
                anchors { top: parent.top; left: parent.left; right: parent.right }
                height: 1
                color: tok.edge
            }

            ActionButton {
                anchors { right: parent.right; rightMargin: 16; verticalCenter: parent.verticalCenter }
                label: qsTr("Close")
                onClicked: root.closeRequested()
            }
        }
    }

    // ── ActionButton component ────────────────────────────────────────────────
    component ActionButton: Rectangle {
        property string label: ""
        property bool highlighted: false
        property bool hovered: false
        property bool enabled: true
        signal clicked()

        height: 30
        width: Math.max(lbl.implicitWidth + 24, 100)
        radius: 2
        color: !enabled ? tok.track
               : hovered ? (highlighted ? tok.sageDeep : tok.sageSoft)
               : (highlighted ? tok.sage : tok.panel2)
        border.color: highlighted ? tok.sage : tok.edge
        border.width: 1

        Text {
            id: lbl
            anchors.centerIn: parent
            text: parent.label
            font.pixelSize: 12
            color: parent.enabled ? (parent.highlighted ? tok.bg : tok.ink) : tok.mute
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            enabled: parent.enabled
            cursorShape: Qt.PointingHandCursor
            onEntered: parent.hovered = true
            onExited:  parent.hovered = false
            onClicked: parent.clicked()
        }
    }

    // ── NavButton component ───────────────────────────────────────────────────
    component NavButton: Rectangle {
        property string label: ""
        property bool hovered: false
        property bool enabled: true
        signal clicked()

        width: 30; height: 30
        radius: 4
        color: !enabled ? "transparent"
               : hovered ? tok.sageSoft
               : "transparent"
        border.color: enabled ? tok.edge : "transparent"
        border.width: 1

        Text {
            anchors.centerIn: parent
            text: parent.label
            font.pixelSize: 13
            color: parent.enabled ? tok.ink : tok.mute
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            enabled: parent.enabled
            cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
            onEntered: parent.hovered = true
            onExited:  parent.hovered = false
            onClicked: parent.clicked()
        }
    }
}
