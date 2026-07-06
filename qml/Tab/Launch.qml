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
    Component.onCompleted: {
        CubedInstance.check_version();
    }
    Loader {
        active: LauncherUpdate.hasNewVersion || LauncherUpdate.hasError
        width: 300
        height: 200
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.rightMargin: 20
        source: "qrc:/qt/qml/CubedLauncher/qml/Tool/CheckUpdate.qml"
    }
    Image {
        id: logoImage
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.top
        anchors.verticalCenterOffset: 300
        source: "qrc:/qt/qml/CubedLauncher/resources/logo.png"
    }
    ColumnLayout {

        visible: !CubedInstance.installed
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 30

        Label {
            id: installGameTip

            text: "You need to install the game in the manager tab next to it."

            font.bold: true
            font.pixelSize: 20
            color: Material.color(Material.Red)
        }
        Button {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 350
            Layout.preferredHeight: 60
            font.pixelSize: 20
            enabled: !CubedInstance.installed
            highlighted: !CubedInstance.installed

            text: "Go to Install Game"
            onClicked: {
                SideTool.currentIndex = 1;
            }
        }
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

        Label {
            visible: GameUpdate.hasNewVersion
            text: "New Cubed Version: " + GameUpdate.remoteVersion + " Available!"
            Layout.alignment: Qt.AlignCenter
            font.pixelSize: 18
        }

        Label {
            visible: CubedInstance.installed
            text: "Cubed Version: " + CubedInstance.version
            Layout.alignment: Qt.AlignCenter
            font.pixelSize: 20
            Layout.bottomMargin: 20
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
            enabled: CubedInstance.installed
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
