import QtQuick

// RemoteControlPrefPage — content for Remote control > gRPC preferences.
// Expects context property: remoteControlPrefBridge (RemoteControlPrefBridge)
Item {
    id: root

    property var bridge: typeof remoteControlPrefBridge !== "undefined" ? remoteControlPrefBridge : null

    implicitWidth: parent ? parent.width : 500
    implicitHeight: col.implicitHeight

    Column {
        id: col
        anchors { left: parent.left; right: parent.right; top: parent.top }
        spacing: 24

        PrefGroup {
            width: parent.width
            title: qsTr("Connection")

            PrefToggleRow {
                width: parent.width
                label: qsTr("Enable gRPC")
                hint: qsTr("Allows local applications to control Workrave through gRPC. Disabling this closes the server immediately.")
                checked: root.bridge ? root.bridge.grpcEnabled : true
                onToggled: (v) => { if (root.bridge) root.bridge.setGrpcEnabled(v) }
            }

            PrefChoiceRow {
                width: parent.width
                visible: root.bridge ? root.bridge.grpcEnabled : false
                label: qsTr("Connection type")
                hint: qsTr("Use a local Unix-domain socket, or a TCP/IP port bound to this computer only.")
                options: [qsTr("Unix domain socket"), qsTr("TCP/IP")]
                currentIndex: root.bridge ? root.bridge.grpcTransport : 0
                onSelected: (idx) => { if (root.bridge) root.bridge.setGrpcTransport(idx) }
            }

            PrefTextRow {
                width: parent.width
                visible: root.bridge ? (root.bridge.grpcEnabled && root.bridge.grpcTransport === 0) : false
                label: qsTr("Socket filename")
                hint: qsTr("Filesystem path of the Unix-domain socket used by Workrave.")
                value: root.bridge ? root.bridge.grpcSocket : ""
                readOnly: true
            }

            PrefTextRow {
                width: parent.width
                visible: root.bridge ? (root.bridge.grpcEnabled && root.bridge.grpcTransport === 1) : false
                label: qsTr("TCP port")
                hint: qsTr("Port on the loopback interface (127.0.0.1).")
                value: root.bridge ? String(root.bridge.grpcPort) : "50051"
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator { bottom: 1; top: 65535 }
                onCommitted: (text) => { if (root.bridge) root.bridge.setGrpcPort(Number(text)) }
            }
        }
    }
}
