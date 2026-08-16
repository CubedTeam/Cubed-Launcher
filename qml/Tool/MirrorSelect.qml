pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CubedLauncher

Dialog {
    id: root
    anchors.centerIn: Overlay.overlay
    width: Math.min(560, Overlay.overlay.width - 48)
    height: Math.min(560, Overlay.overlay.height - 48)
    modal: true
    title: qsTr("Select Mirror Source")
    standardButtons: Dialog.NoButton
    padding: Theme.space16
    property int pendingTests: 0
    palette.text: Theme.onSurface
    background: Rectangle { color: Theme.surfaceContainerHigh; radius: Theme.radiusExtraLarge }

    function testMirrors(force) {
        pendingTests = mirrorModel.count;
        for (let index = 0; index < mirrorModel.count; ++index) {
            mirrorModel.setProperty(index, "testing", true);
            mirrorModel.setProperty(index, "latency", -1);
        }
        MirrorSource.test_all_latency(force);
    }
    onOpened: testMirrors(false)

    ListModel {
        id: mirrorModel
        Component.onCompleted: {
            const names = MirrorSource.names;
            for (let index = 0; index < names.length; ++index)
                append({ name: names[index], latency: -1, testing: false });
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Theme.space12
        Label {
            Layout.fillWidth: true
            text: qsTr("Choose the fastest available source. Latency results are estimates.")
            color: Theme.onSurfaceVariant
            font.pixelSize: Theme.bodySize
            wrapMode: Text.WordWrap
        }
        ListView {
            id: mirrorList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: Theme.space4
            model: mirrorModel
            delegate: ItemDelegate {
                id: row
                required property int index
                required property string name
                required property int latency
                required property bool testing
                width: mirrorList.width
                height: 64
                highlighted: Settings.mirrorIndex === index
                onClicked: { Settings.mirrorIndex = index; root.close(); }
                contentItem: RowLayout {
                    MdIcon { name: row.highlighted ? "check" : "public"; color: row.highlighted ? Theme.primary : Theme.onSurfaceVariant }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Label { text: row.name; color: Theme.onSurface; font.pixelSize: Theme.bodyLargeSize }
                        Label {
                            text: row.testing ? qsTr("Testing latency…")
                                  : row.latency < 0 ? qsTr("Timed out") : row.latency + " " + qsTr("ms")
                            color: row.latency >= 0 && row.latency < 300 ? Theme.primary : Theme.onSurfaceVariant
                            font.pixelSize: Theme.labelSize
                        }
                    }
                    BusyIndicator { visible: row.testing; running: row.testing; implicitWidth: 24; implicitHeight: 24 }
                }
                background: Rectangle {
                    radius: Theme.radiusMedium
                    color: row.highlighted ? Theme.secondaryContainer : row.hovered ? Theme.surfaceContainerHighest : "transparent"
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            MdButton { text: qsTr("Close"); variant: "text"; onClicked: root.close() }
            Item { Layout.fillWidth: true }
            MdButton {
                text: qsTr("Test Latency")
                iconName: "refresh"
                enabled: root.pendingTests === 0
                onClicked: root.testMirrors(true)
            }
        }
    }

    Connections {
        target: MirrorSource
        function onLatencyReady(index, ms) {
            if (index < 0 || index >= mirrorModel.count)
                return;
            mirrorModel.setProperty(index, "testing", false);
            mirrorModel.setProperty(index, "latency", Number(ms));
            root.pendingTests = Math.max(0, root.pendingTests - 1);
        }
    }
}
