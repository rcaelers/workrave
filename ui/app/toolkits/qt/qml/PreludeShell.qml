// PreludeShell.qml — root of the prelude QQuickView.
// Owns window positioning (normal/fullscreen) and switches between
// PreludeOverlay.qml (Sanctuary) and PreludeClassic.qml (GTK-style)
// via a Loader driven by bridge.classic.  Switching is live — no restart needed.

import QtQuick

Item {
    id: root

    // ── Bridge-driven layout properties ──────────────────────────────────────
    readonly property bool useClassic:   bridge != null ? bridge.classic     : false
    readonly property bool isFullscreen: bridge != null ? bridge.fullscreen  : false
    readonly property bool cardAtBottom: bridge != null ? bridge.cardAtBottom : false

    // Style-dependent card size, provided by PreludeBridge.
    readonly property int cardW:      bridge != null ? bridge.cardW : 480
    readonly property int cardH:      bridge != null ? bridge.cardH : 80
    readonly property int cardMargin: 20

    // ── Content loader ────────────────────────────────────────────────────────
    // Normal mode  : Loader fills the view (which is already CARD_W × CARD_H).
    // Fullscreen   : Loader is fixed-size, floating at the top or bottom centre
    //               of the transparent full-screen window (Wayland).
    Loader {
        id: contentLoader

        x:      root.isFullscreen ? (root.width - root.cardW) / 2 : 0
        y:      root.isFullscreen
                ? (root.cardAtBottom ? root.height - root.cardH - root.cardMargin
                                     : root.cardMargin)
                : 0
        width:  root.isFullscreen ? root.cardW : root.width
        height: root.isFullscreen ? root.cardH : root.height

        Behavior on y { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }

        source: root.useClassic ? Qt.resolvedUrl("PreludeClassic.qml")
                                : Qt.resolvedUrl("PreludeOverlay.qml")
    }
}
