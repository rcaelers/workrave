import QtQuick
import PrefWidgets 1.0

// Application-specific defaults supplied to the generic choice grid.
PrefCardsRow {
    id: root

    // Optional prefwidget model; built-in pages may bind currentIndex/selected directly.
    property var preference: null
    currentIndex: preference ? preference.currentIndex : 0
    onSelected: (index) => { if (preference) preference.setCurrentIndex(index) }

    label: qsTr("Placement")
    hint: qsTr("Where each timer appears. Pair timers into the same slot to save space — Workrave will alternate between them.")
    options: [
        qsTr("Next to each other"),
        qsTr("Micro + Rest together"),
        qsTr("Rest + Daily together"),
        qsTr("All in one slot"),
    ]
    previews: ["separate", "micro-rest", "rest-daily", "all"].map(name => ({
        asset: "workrave.timer-grouping." + name,
    }))

    PreferencePresentations { }

    PrefTokens { id: tok }
}
