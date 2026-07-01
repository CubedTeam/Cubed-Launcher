pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher

import QtQuick.Layouts

Item {
    id: launchTab
    Layout.fillHeight: true
    Layout.fillWidth: true

    Image {
        id: logoImage
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.top
        anchors.verticalCenterOffset: 300
        source: "qrc:/qt/qml/CubedLauncher/resources/logo.png"
    }

    ColumnLayout {
        id: gameLayout
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.bottomMargin: 10
        anchors.rightMargin: 25

        width: 300
        spacing: 10

        Button {
            text: "Kill All Process"
            visible: CubedInstance.running
            Layout.margins: 15
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

        TextField {
            id: playerNameText
            Component.onCompleted: {
                CubedInstance.set_name(playerNameText.text);
            }
            text: Settings.playerName
            Layout.fillWidth: true
            placeholderText: "Enter Player Name"
            onEditingFinished: {
                Settings.playerName = playerNameText.text;
                CubedInstance.set_name(playerNameText.text);
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
            Component.onCompleted: {
                if (Settings.pathSetted) {
                    console.log("Set Cubed Instance game path sucessful");
                    CubedInstance.set_game_path(Settings.gamePath);
                }
            }
            Material.roundedScale: Material.MediumScale
            onClicked: {
                CubedInstance.start_cubed_instance();
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
}
