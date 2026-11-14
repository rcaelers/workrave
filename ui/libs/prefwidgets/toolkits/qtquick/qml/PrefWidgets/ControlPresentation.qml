pragma ComponentBehavior: Bound

import QtQuick

// Registered controls receive the same live preference model as built-in rows.
Item {
    id: root

    property var preference: null
    readonly property var entry: preference
        ? PresentationRegistry.lookup(preference.presentationName, preference.kind) : null
    readonly property Item contentItem: controlLoader.item as Item
    readonly property bool available: entry !== null && controlLoader.status === Loader.Ready && contentItem !== null

    implicitHeight: available ? contentItem.implicitHeight : 0

    Loader {
        id: controlLoader
        anchors.fill: parent

        function reload() {
            setSource(root.entry !== null ? root.entry.source : "", {
                preference: Qt.binding(function() { return root.preference }),
            })
        }

        Connections {
            target: root
            function onEntryChanged() { controlLoader.reload() }
        }
        Component.onCompleted: reload()
    }
}
