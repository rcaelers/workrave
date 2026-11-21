import QtQuick
import PrefWidgets 1.0

// Application-specific names and choices; rendering remains generic.
PrefSegmentedRow {
    id: root

    property alias timerName: root.label
    property alias timerColor: root.accentColor

    options: [qsTr("Show"), qsTr("When first due"), qsTr("Hide")]
}
