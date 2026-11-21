import QtQuick
import PrefWidgets 1.0 as Widgets

Widgets.Page {
    property var bridge: typeof activePluginBridge !== "undefined" ? activePluginBridge : null
    model: bridge
    PreferencePresentations { }
    PrefTokens { }
}
