import QtQuick

// Design token object — instantiate as `PrefTokens { id: tok }` in each component.
// Qt::ColorScheme::Dark == 2; binding updates live when the user changes the setting.
QtObject {
    readonly property bool isDark: Qt.styleHints.colorScheme === 2

    // ── Colors ────────────────────────────────────────────────────────────────
    readonly property color bg:         isDark ? "#1B1D1A" : "#F5F1EA"
    readonly property color panel:      isDark ? "#23261F" : "#FFFFFF"
    readonly property color panel2:     isDark ? "#2A2D26" : "#FAF6EE"
    readonly property color edge:       isDark ? Qt.rgba(220/255, 215/255, 200/255, 0.10)
                                               : Qt.rgba( 40/255,  45/255,  38/255, 0.10)
    readonly property color edge2:      isDark ? Qt.rgba(220/255, 215/255, 200/255, 0.06)
                                               : Qt.rgba( 40/255,  45/255,  38/255, 0.06)
    readonly property color ink:        isDark ? "#E8E2D2" : "#2A2D29"
    readonly property color ink2:       isDark ? "#C7C2B3" : "#4A4D46"
    readonly property color mute:       isDark ? "#86887E" : "#8A8B82"
    readonly property color sage:       isDark ? "#8FA88B" : "#6B8068"
    readonly property color sageDeep:   isDark ? "#B4C9B0" : "#44563F"
    readonly property color sageSoft:   isDark ? "#384034" : "#D9E1D2"
    readonly property color clay:       isDark ? "#E0A07A" : "#C97B4A"
    readonly property color claySoft:   isDark ? "#4A3325" : "#F2D9C5"
    readonly property color track:      isDark ? "#2F322B" : "#E7E1D2"
    readonly property color danger:     isDark ? "#D77F6F" : "#B85A4A"
    readonly property color dangerSoft: isDark ? "#3D1F1A" : "#FFE8E5"
    readonly property color warn:       isDark ? "#E8A050" : "#D4872A"
    readonly property color rest:       isDark ? "#8FC99A" : "#7FAF88"

    // ── Typography ────────────────────────────────────────────────────────────
    readonly property int labelPx:    14    // primary row label
    readonly property int hintPx:     12    // secondary / hint text
    readonly property int bodyPx:     13    // body / nav items / lede
    readonly property int captionPx:  11    // section headers, small badges
    readonly property int stepperPx:  19    // large time value in stepper
    readonly property int btnPx:      16    // +/− button glyphs
    readonly property int tickPx:     10    // slider tick labels

    readonly property string serifFamily: "Georgia"
    readonly property string monoFamily:  "Menlo"

    // ── Layout ────────────────────────────────────────────────────────────────
    readonly property int  labelHintGap: 3     // spacing between label and hint
    readonly property real hintLineH:    1.45  // hint / body text line height
    readonly property int  rowPad:       28    // vertical padding in toggle/choice rows
    readonly property int  rowPadLg:     36    // vertical padding in stepper/time rows
}
