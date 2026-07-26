// DailyLimitShell.qml — root of the QmlDailyLimitWindow QQuickView.
// Switches live between DailyLimitOverlay.qml (Sanctuary) and DailyLimitClassic.qml
// (GTK-style) via bridge.classic, driven by GUIConfig::sanctuary_ui_enabled.

import QtQuick

Item {
    id: root

    readonly property bool useClassic: bridge != null ? bridge.classic : false

    Loader {
        anchors.fill: parent
        source: root.useClassic ? Qt.resolvedUrl("DailyLimitClassic.qml")
                                : Qt.resolvedUrl("DailyLimitOverlay.qml")
    }
}
