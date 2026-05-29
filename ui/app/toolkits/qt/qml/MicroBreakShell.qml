// MicroBreakShell.qml — root of the QmlMicroBreakWindow QQuickView.
// Switches live between MicroBreakOverlay.qml (Sanctuary) and MicroBreakClassic.qml
// (GTK-style) via bridge.classic, driven by GUIConfig::sanctuary_ui_enabled.

import QtQuick

Item {
    id: root

    readonly property bool useClassic: bridge != null ? bridge.classic : false

    Loader {
        anchors.fill: parent
        source: root.useClassic ? Qt.resolvedUrl("MicroBreakClassic.qml")
                                : Qt.resolvedUrl("MicroBreakOverlay.qml")
    }
}
