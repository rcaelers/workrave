import QtQuick
import PrefWidgets 1.0

// Application-owned assets shared by built-in pages and prefwidget definitions.
QtObject {
    function registerPreview(name, source, properties) {
        if (PreviewRegistry.lookup(name) === null)
            PreviewRegistry.registerComponent(name, source, properties)
    }

    Component.onCompleted: {
        PresentationRegistry.registerComponent("workrave.timer-grouping", "choice",
            Qt.resolvedUrl("TimerPlacementPicker.qml"))
        const grouping = Qt.resolvedUrl("TimerGroupingPreview.qml")
        registerPreview("workrave.timer-grouping.separate", grouping,
            { groups: [["M"], ["R"], ["D"]] })
        registerPreview("workrave.timer-grouping.micro-rest", grouping,
            { groups: [["M", "R"], ["D"]] })
        registerPreview("workrave.timer-grouping.rest-daily", grouping,
            { groups: [["M"], ["R", "D"]] })
        registerPreview("workrave.timer-grouping.all", grouping,
            { groups: [["M", "R", "D"]] })

        const style = Qt.resolvedUrl("TimerStylePreview.qml")
        for (const kind of ["rings", "bars", "focus", "classic"])
            registerPreview("workrave.timer-style." + kind, style, { kind: kind })
    }
}
