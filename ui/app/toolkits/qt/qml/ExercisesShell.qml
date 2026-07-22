// ExercisesShell.qml — root of the exercises QQuickView.
// Switches between ExercisesDialog.qml (Sanctuary) and ExercisesClassic.qml
// (Gtk-style) via a Loader driven by exercisesBridge.classic.
// Switching is live — no restart needed.

import QtQuick

Item {
    id: root

    signal closeRequested()

    readonly property bool useClassic: exercisesBridge != null ? exercisesBridge.classic : false

    Loader {
        anchors.fill: parent
        source: root.useClassic ? Qt.resolvedUrl("ExercisesClassic.qml")
                                : Qt.resolvedUrl("ExercisesDialog.qml")
        onLoaded: item.closeRequested.connect(root.closeRequested)
    }
}
