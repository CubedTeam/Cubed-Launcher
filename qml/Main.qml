import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Dialogs
import QtQuick.Layouts

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: "Cubed Launcher"

    ColumnLayout {
        anchors.centerIn: parent
        width: 300
        spacing: 10
        Label {
            Layout.alignment: Qt.AlignCenter
            text: gameFileDialog.selectedFile
        }
        TextField {
            id: playerName
            Layout.fillWidth: true
            placeholderText: "Enter Player Name"
            onEditingFinished: {
                CubedInstance.set_name(playerName.text);
            }
        }

        Button {
            id: startGameButton
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 250
            Layout.preferredHeight: 60
            font.pixelSize: 20
            text: "Start Game"
            //anchors.centerIn: parent
            highlighted: true

            Material.roundedScale: Material.MediumScale
            onClicked: {
                CubedInstance.start_cubed_instance();
            }
        }
        Button {
            id: gamePathButton

            Material.roundedScale: Material.MediumScale
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 250
            Layout.preferredHeight: 60

            font.pixelSize: 20

            highlighted: true
            text: "Select Game Path"
            onClicked: {
                gameFileDialog.open();
            }
        }

        ComboBox {
            id: peerMode
            Layout.alignment: Qt.AlignCenter
            model: ["Host", "Client"]
            currentIndex: 0
            onCurrentIndexChanged: {
                CubedInstance.set_peer(peerMode.currentIndex);
            }
        }
        Loader {
            Layout.fillWidth: true
            height: 100
            sourceComponent: {
                if (peerMode.currentIndex == 0) {
                    return hostMode;
                } else if (peerMode.currentIndex == 1) {
                    return clientMode;
                }
            }
        }

        Component {
            id: hostMode
            Item {
                anchors.fill: parent
                Row {
                    spacing: 10
                    anchors.centerIn: parent
                    TextField {
                        id: hostPort
                        Layout.fillWidth: true
                        placeholderText: "Port"
                        onEditingFinished: {
                            CubedInstance.set_port(hostPort.text);
                        }
                    }
                }
            }
        }
        Component {
            id: clientMode
            Item {
                anchors.fill: parent
                Row {
                    anchors.centerIn: parent
                    spacing: 10
                    TextField {
                        id: serverIp
                        width: 250
                        Layout.fillWidth: true
                        placeholderText: "Ip"
                        onEditingFinished: {
                            CubedInstance.set_ip(serverIp.text);
                        }
                    }
                    TextField {
                        id: serverPort
                        Layout.fillWidth: true
                        placeholderText: "Port"
                        onEditingFinished: {
                            CubedInstance.set_port(serverPort.text);
                        }
                    }
                }
            }
        }
    }

    FileDialog {
        id: gameFileDialog
        title: "Select Cubed Game"
        // currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
        onAccepted: {
            CubedInstance.set_game_path(selectedFile);
        }
    }
}
