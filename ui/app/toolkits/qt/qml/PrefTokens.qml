import QtQuick

// Design token object — instantiate as `PrefTokens { id: tok }` in each component.
QtObject {
    // ── Colors ────────────────────────────────────────────────────────────────
    readonly property color bg:        "#F5F1EA"
    readonly property color panel:     "#FFFFFF"
    readonly property color panel2:    "#FAF6EE"
    readonly property color edge:      Qt.rgba(40/255, 45/255, 38/255, 0.10)
    readonly property color edge2:     Qt.rgba(40/255, 45/255, 38/255, 0.06)
    readonly property color ink:       "#2A2D29"
    readonly property color ink2:      "#4A4D46"
    readonly property color mute:      "#8A8B82"
    readonly property color sage:      "#6B8068"
    readonly property color sageDeep:  "#44563F"
    readonly property color sageSoft:  "#D9E1D2"
    readonly property color clay:      "#C97B4A"
    readonly property color claySoft:  "#F2D9C5"
    readonly property color track:     "#E7E1D2"
    readonly property color danger:    "#B85A4A"

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
