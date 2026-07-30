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
        //active: true
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
        anchors.verticalCenterOffset: 250
        source: "qrc:/qt/qml/CubedLauncher/resources/logo.png"
    }
    ColumnLayout {
        id: installTip
        visible: !CubedInstance.installed
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 30
        spacing: 10
        width: 400
        Card {
            Layout.fillWidth: true
            Layout.preferredHeight: installGameLayout.implicitHeight + 20
            ColumnLayout {
                id: installGameLayout
                anchors.centerIn: parent
                spacing: installTip.spacing
                Label {
                    id: installGameTip
                    Layout.alignment: Qt.AlignCenter
                    text: qsTr("You need to install the game in the manager tab next to it.")

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
                    Material.roundedScale: Material.MediumScale
                    text: qsTr("Go to Install Game")
                    onClicked: {
                        SideTool.currentIndex = 1;
                    }
                }
            }
        }
    }

    ColumnLayout {
        id: gameLayout
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.bottomMargin: 10
        anchors.rightMargin: 25

        width: 400
        spacing: 10

        Card {
            Layout.preferredHeight: statusLayout.implicitHeight + 20
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            visible: CubedInstance.running
            ColumnLayout {
                id: statusLayout
                anchors.centerIn: parent
                spacing: gameLayout.spacing
                Button {
                    text: qsTr("Kill All Process")
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
            }
        }
        Card {
            Layout.preferredHeight: versionLayout.implicitHeight + 20
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            visible: CubedInstance.installed
            ColumnLayout {
                id: versionLayout
                anchors.centerIn: parent
                spacing: gameLayout.spacing
                Label {
                    visible: GameUpdate.hasNewVersion
                    text: qsTr("New Cubed Version: %1 Available!").arg(GameUpdate.remoteVersion)
                    Layout.alignment: Qt.AlignCenter
                    font.pixelSize: 18
                }

                Label {
                    visible: CubedInstance.installed
                    text: qsTr("Cubed Version: %1").arg(CubedInstance.version)
                    Layout.alignment: Qt.AlignCenter
                    font.pixelSize: 20
                }
            }
        }
        Card {
            Layout.preferredHeight: launchLayout.implicitHeight + 20
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            ColumnLayout {
                id: launchLayout
                anchors.centerIn: parent
                spacing: gameLayout.spacing
                TextField {
                    id: playerNameText
                    Component.onCompleted: {
                        CubedInstance.set_name(playerNameText.text);
                    }
                    text: Settings.playerName
                    Layout.fillWidth: true
                    placeholderText: qsTr("Enter Player Name")
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
                    text: qsTr("Start Game")
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
            }
        }
    }
}
