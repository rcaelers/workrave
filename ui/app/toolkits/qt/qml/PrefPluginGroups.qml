import QtQuick
import PrefWidgets 1.0 as Widgets

// Embeds a library-rendered page in an application preference panel.
Widgets.Page {
    property var bridge: null
    model: bridge
    visible: bridge !== null && bridge.groups.length > 0
    PreferencePresentations { }
    PrefTokens { }
}
