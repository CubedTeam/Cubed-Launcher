import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Dialogs
import QtQuick.Layouts

ApplicationWindow {
    id: cbedLauncher
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
            enabled: CubedInstance.path_selected
            highlighted: enabled

            Material.roundedScale: Material.MediumScale
            onClicked: {
                CubedInstance.start_cubed_instance();
            }
        }
        Label {
            id: selectMessgae
            Layout.alignment: Qt.AlignCenter
            Material.foreground: Material.Red
            visible: !CubedInstance.path_selected
            text: "You must select game program"
        }
        Button {
            id: gamePathButton

            Material.roundedScale: Material.MediumScale
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 250
            Layout.preferredHeight: 60

            font.pixelSize: 20

            highlighted: true
            text: "Select Game"
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
        Button {
            text: "Kill All Process"
            visible: CubedInstance.running

            Material.roundedScale: Material.MediumScale
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 250
            Layout.preferredHeight: 60
            font.pixelSize: 20

            highlighted: true

            onClicked: {
                CubedInstance.kill_all();
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
        Switch {
            id: advancedSetting
            Layout.alignment: Qt.AlignCenter
            font.pixelSize: 20
            text: "Advanced Setting"
            checked: false
        }
        TextField {
            id: wrapperCommand
            visible: advancedSetting.checked
            Layout.fillWidth: true
            placeholderText: "Wrapper Command"
            onEditingFinished: {
                CubedInstance.set_wrapper_command(wrapperCommand.text);
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
