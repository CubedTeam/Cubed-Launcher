pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    anchors.fill: parent

    ColumnLayout {
        anchors.centerIn: parent

        RowLayout {
            Layout.alignment: Qt.AlignCenter
            Button {
                id: gamePathButton

                Material.roundedScale: Material.MediumScale
                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: 250
                Layout.preferredHeight: 60

                font.pixelSize: 20

                highlighted: true
                text: "Set Game Path"
                onClicked: {
                    gameFileDialog.open();
                }
            }
            Button {
                Material.roundedScale: Material.MediumScale
                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: 250
                Layout.preferredHeight: 60
                font.pixelSize: 20
                text: "Reset Path"
                onClicked: {
                    CubedInstance.set_game_path(SystemInfo.defaultGameFilePath);
                    Settings.set_game_path(SystemInfo.defaultGameFilePath);
                    VersionUpdate.set_game_dir(SystemInfo.defaultGameFilePath);
                }
            }
        }

        Switch {
            id: downloadSource
            text: "Use Custom Link"
            checked: false
            Layout.alignment: Qt.AlignCenter
        }

        Button {
            id: downloadGameGithubButton
            visible: !downloadSource.checked
            enabled: !downloadSource.checked
            Layout.alignment: Qt.AlignCenter
            Material.roundedScale: Material.MediumScale
            Layout.preferredWidth: 250
            Layout.preferredHeight: 60
            highlighted: true

            font.pixelSize: 20
            text: "Intall Game"

            Component.onCompleted: {
                VersionUpdate.set_game_dir(Settings.gamePath);
            }

            onClicked: {
                if (!Settings.pathSetted) {
                    Settings.set_game_path(SystemInfo.defaultGameFilePath);
                }

                downloadProgress.visible = true;
                gamePathButton.enabled = false;
                gamePathButton.highlighted = false;
                enabled = false;
                highlighted = false;
                VersionUpdate.download_from_github(useMirror.checked);
            }
        }
        Switch {
            id: useMirror
            visible: !downloadSource.checked
            enabled: !downloadSource.checked
            Layout.alignment: Qt.AlignCenter
            checked: SystemInfo.isInChina
            text: "Use Github Mirror"
        }
        TextField {
            id: downloadLink
            visible: downloadSource.checked
            enabled: downloadSource.checked
            Layout.alignment: Qt.AlignCenter
            Layout.fillWidth: true
            placeholderText: "Download Link"
        }

        Button {
            id: downloadGameCustomButton
            visible: downloadSource.checked
            enabled: downloadSource.checked
            Layout.alignment: Qt.AlignCenter
            Material.roundedScale: Material.MediumScale
            Layout.preferredWidth: 250
            Layout.preferredHeight: 60
            highlighted: true

            font.pixelSize: 20
            text: "Intall Game"
            Component.onCompleted: {
                VersionUpdate.set_game_dir(Settings.gamePath);
            }
            onClicked: {
                if (!Settings.pathSetted) {
                    Settings.set_game_path(SystemInfo.defaultGameFilePath);
                }

                downloadProgress.visible = true;
                gamePathButton.enabled = false;
                gamePathButton.highlighted = false;
                enabled = false;
                highlighted = false;
                VersionUpdate.download_game(downloadLink.text);
            }
        }

        Label {
            text: "Game Install Directory: " + VersionUpdate.gameInstallPath
            font.pixelSize: 16
        }
        ProgressBar {
            id: downloadProgress
            Layout.alignment: Qt.AlignCenter
            visible: false
            from: 0.0
            to: 1.0
            height: 10
            value: VersionUpdate.downloadProgress
            onValueChanged: {
                if (value >= to) {
                    console.log("Download Finish");
                    downloadProgress.visible = true;
                    downloadGameGithubButton.enabled = true;
                    downloadGameGithubButton.highlighted = true;
                    downloadGameCustomButton.enabled = true;
                    downloadGameCustomButton.highlighted = true;
                    gamePathButton.enabled = true;
                    gamePathButton.highlighted = true;
                    visible = false;
                }
            }
        }
    }
    FileDialog {
        id: gameFileDialog
        title: "Select Cubed Game"
        onAccepted: {
            CubedInstance.set_game_path_url(selectedFile);
            Settings.gamePath = selectedFile;
            VersionUpdate.set_game_dir(Settings.gamePath);
        }
    }
}
