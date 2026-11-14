pragma Singleton
import QtQuick

// Reusable theme roles. Hosts may override values without replacing controls.
QtObject {
    id: theme
    property var values: ({})
    property int colorScheme: Qt.styleHints.colorScheme
    readonly property bool isDark: colorScheme === 2
    property SystemPalette systemPalette: SystemPalette { colorGroup: SystemPalette.Active }

    function value(name, fallback) {
        return values[name] !== undefined ? values[name] : fallback
    }

    // Resolve host-defined roles before optional literal color fallbacks.
    function resolveColor(spec, fallback = "transparent") {
        if (spec.role) {
            const color = value(spec.role, theme[spec.role])
            if (color !== undefined) return color
        }
        return isDark && spec.dark ? spec.dark : (spec.light || fallback)
    }

    readonly property color background: value("background", systemPalette.window)
    readonly property color surface: value("surface", systemPalette.base)
    readonly property color surfaceVariant: value("surfaceVariant", systemPalette.alternateBase)
    readonly property color border: value("border", Qt.rgba(systemPalette.text.r, systemPalette.text.g, systemPalette.text.b, 0.2))
    readonly property color borderSubtle: value("borderSubtle", Qt.rgba(systemPalette.text.r, systemPalette.text.g, systemPalette.text.b, 0.08))
    readonly property color text: value("text", systemPalette.text)
    readonly property color textSecondary: value("textSecondary", systemPalette.windowText)
    readonly property color textMuted: value("textMuted", systemPalette.mid)
    readonly property color accent: value("accent", systemPalette.highlight)
    readonly property color accentStrong: value("accentStrong", Qt.darker(systemPalette.highlight, 1.3))
    readonly property color accentSoft: value("accentSoft", Qt.rgba(accent.r, accent.g, accent.b, 0.16))
    readonly property color secondary: value("secondary", systemPalette.highlight)
    readonly property color secondarySoft: value("secondarySoft", Qt.rgba(secondary.r, secondary.g, secondary.b, 0.16))
    readonly property color track: value("track", systemPalette.midlight)
    readonly property color error: value("error", "#b00020")
    readonly property color errorSoft: value("errorSoft", Qt.rgba(error.r, error.g, error.b, 0.16))
    readonly property color warning: value("warning", "#b36b00")
    readonly property color positive: value("positive", "#287c37")
    readonly property color actionSurface: value("actionSurface", systemPalette.button)
    readonly property color actionBorder: value("actionBorder", systemPalette.mid)
    readonly property int labelPx: value("labelPx", 14)
    readonly property int hintPx: value("hintPx", 12)
    readonly property int bodyPx: value("bodyPx", 13)
    readonly property int captionPx: value("captionPx", 11)
    readonly property int stepperPx: value("stepperPx", 19)
    readonly property int btnPx: value("btnPx", 16)
    readonly property int tickPx: value("tickPx", 10)
    readonly property string serifFamily: value("serifFamily", "")
    readonly property string monoFamily: value("monoFamily", "monospace")
    readonly property int labelHintGap: value("labelHintGap", 3)
    readonly property real hintLineH: value("hintLineH", 1.45)
    readonly property int rowPad: value("rowPad", 28)
    readonly property int rowPadLg: value("rowPadLg", 36)
    readonly property int actionRadius: value("actionRadius", 8)
}
