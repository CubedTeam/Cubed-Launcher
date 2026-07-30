pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts

// AI-generated: mirror source picker popup with a per-mirror latency tester.
Popup {
    id: mirrorPopup
    anchors.centerIn: Overlay.overlay
    width: 520
    height: 540
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    ListModel {
        id: mirrorModel
        Component.onCompleted: {
            const names = MirrorSource.names;
            for (let i = 0; i < names.length; ++i) {
                mirrorModel.append({
                    "name": names[i],
                    "latency": -1,
                    "testing": false
                });
            }
        }
    }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Component.onCompleted: {
            MirrorSource.test_all_latency();
        }

        Label {
            text: qsTr("Select Mirror Source")
            Layout.alignment: Qt.AlignCenter
            font.pixelSize: 24
            font.bold: true
        }

        ListView {
            id: mirrorList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: mirrorModel
            delegate: ItemDelegate {
                id: row
                width: mirrorList.width
                // AI-generated: required properties so ComponentBehavior: Bound
                // does not break the model binding.
                required property int index
                required property string name
                required property int latency
                required property bool testing

                onClicked: Settings.mirrorIndex = row.index

                contentItem: RowLayout {
                    width: parent.width
                    spacing: 10

                    RadioButton {
                        checked: Settings.mirrorIndex === row.index
                        onClicked: Settings.mirrorIndex = row.index
                        text: row.name
                        font.pixelSize: 18
                        Layout.fillWidth: true
                    }

                    Label {
                        id: latencyLabel
                        Layout.preferredWidth: 120
                        horizontalAlignment: Text.AlignRight
                        font.pixelSize: 18
                        font.bold: true
                        color: {
                            if (row.testing) {
                                return Material.color(Material.Grey);
                            }
                            if (row.latency < 0) {
                                return Material.color(Material.Red);
                            }
                            if (row.latency < 300) {
                                return Material.color(Material.Green);
                            }
                            if (row.latency < 1000) {
                                return Material.color(Material.Orange);
                            }
                            return Material.color(Material.Red);
                        }
                        text: {
                            if (row.testing) {
                                return qsTr("testing...");
                            }
                            if (row.latency < 0) {
                                return qsTr("timeout");
                            }
                            return row.latency + " " + qsTr("ms");
                        }

                        BusyIndicator {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: -28
                            visible: row.testing
                            running: row.testing
                            implicitWidth: 18
                            implicitHeight: 18
                        }
                    }
                }
            }
        }

        Button {
            id: testButton
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 250
            Layout.preferredHeight: 50
            font.pixelSize: 18
            Material.roundedScale: Material.MediumScale
            highlighted: enabled
            text: qsTr("Test Latency")
            enabled: mirrorPopup.pendingTests === 0
            onClicked: {
                mirrorPopup.pendingTests = mirrorModel.count;
                for (let i = 0; i < mirrorModel.count; ++i) {
                    mirrorModel.setProperty(i, "testing", true);
                    mirrorModel.setProperty(i, "latency", -1);
                }
                MirrorSource.test_all_latency();
            }
        }
    }

    // AI-generated: probes in flight, gates the Test Latency button.
    property int pendingTests: 0

    Connections {
        target: MirrorSource
        function onLatencyReady(index, ms) {
            if (index < 0 || index >= mirrorModel.count) {
                return;
            }
            mirrorModel.setProperty(index, "testing", false);
            mirrorModel.setProperty(index, "latency", Number(ms));
            mirrorPopup.pendingTests = Math.max(0, mirrorPopup.pendingTests - 1);
        }
    }
}
