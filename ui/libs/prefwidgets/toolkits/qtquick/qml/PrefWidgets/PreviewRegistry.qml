pragma Singleton

import QtQuick

// Engine-local host assets. Definitions refer only to their stable names.
QtObject {
    id: registry

    property var entries: ({})

    function lookup(name) {
        return Object.prototype.hasOwnProperty.call(entries, name) ? entries[name] : null
    }

    function registerImage(name, source, darkSource = "") {
        return registerAsset(name, {
            kind: "image", source: String(source), darkSource: String(darkSource),
        })
    }

    function registerComponent(name, source, properties = ({})) {
        return registerAsset(name, {
            kind: "component", source: String(source), properties: properties,
        })
    }

    function registerAsset(name, entry) {
        if (!name || !entry.source) {
            console.warn("Cannot register a preference preview without a name and source")
            return false
        }
        const updated = Object.assign({}, entries)
        Object.defineProperty(updated, name, { value: entry, enumerable: true, configurable: true })
        entries = updated
        return true
    }

    function unregister(name) {
        if (!lookup(name)) return
        const updated = Object.assign({}, entries)
        delete updated[name]
        entries = updated
    }
}
