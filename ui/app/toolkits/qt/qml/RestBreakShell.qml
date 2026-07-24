// RestBreakShell.qml — root of the QmlRestBreakWindow QQuickView.
// Switches live between RestBreakOverlay.qml (Sanctuary) and RestBreakClassic.qml
// (GTK-style) via bridge.classic, driven by GUIConfig::sanctuary_ui_enabled.

import QtQuick

Item {
    id: root

    readonly property bool useClassic: bridge != null ? bridge.classic : false

    Loader {
        anchors.fill: parent
        source: root.useClassic ? Qt.resolvedUrl("RestBreakClassic.qml")
                                : Qt.resolvedUrl("RestBreakOverlay.qml")
    }
}
