pragma ComponentBehavior: Bound

import QtQuick

// Shared asset renderer, usable in cards and directly by the host application.
Item {
    id: root

    property string asset: ""
    readonly property var entry: PreviewRegistry.lookup(asset)
    readonly property bool available: entry !== null

    Image {
        anchors.fill: parent
        visible: root.entry !== null && root.entry.kind === "image"
        source: visible ? (Theme.isDark && root.entry.darkSource
            ? root.entry.darkSource : root.entry.source) : ""
        fillMode: Image.PreserveAspectFit
    }

    Loader {
        id: componentLoader
        anchors.fill: parent

        function reload() {
            const entry = root.entry
            setSource(entry !== null && entry.kind === "component" ? entry.source : "",
                entry !== null && entry.kind === "component" ? entry.properties : ({}))
        }

        Connections {
            target: root
            function onEntryChanged() { componentLoader.reload() }
        }
        Component.onCompleted: reload()
    }
}
