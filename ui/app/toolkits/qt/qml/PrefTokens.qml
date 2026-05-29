import QtQuick

// Design token object — instantiate as `PrefTokens { id: tok }` in each component.
QtObject {
    // Light theme colors
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
}
