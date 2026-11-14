pragma Singleton

import QtQuick

// Engine-local, typed presentations for complete preference controls.
QtObject {
    property var entries: ({})
    readonly property var widgetKinds: ({ toggle: 1, time: 2, value: 3, choice: 4, entry: 5 })

    function contains(name) {
        return Object.prototype.hasOwnProperty.call(entries, name)
    }

    function lookup(name, kind) {
        const entry = contains(name) ? entries[name] : null
        return entry !== null && entry.kind === kind ? entry : null
    }

    function registerComponent(name, widgetType, source) {
        const kind = Object.prototype.hasOwnProperty.call(widgetKinds, widgetType) ? widgetKinds[widgetType] : 0
        if (!name || !kind || !source) {
            console.warn("Cannot register a preference presentation without a name, supported widget type and source")
            return false
        }
        const previous = lookup(name, kind)
        if (previous !== null && previous.source === String(source)) return true
        const updated = Object.assign({}, entries)
        Object.defineProperty(updated, name, {
            value: { kind: kind, source: String(source) }, enumerable: true, configurable: true,
        })
        entries = updated
        return true
    }

    function unregister(name) {
        if (!contains(name)) return
        const updated = Object.assign({}, entries)
        delete updated[name]
        entries = updated
    }
}
