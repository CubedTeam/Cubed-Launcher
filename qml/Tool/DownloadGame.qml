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

        Button {
            Material.roundedScale: Material.MediumScale
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 250
            Layout.preferredHeight: 60
            text: "Reset Path"
            onClicked: {
                CubedInstance.set_game_path(SystemInfo.defaultGameFilePath);
                Settings.set_game_path(SystemInfo.defaultGameFilePath);
                VersionUpdate.set_game_dir(SystemInfo.defaultGameFilePath);
            }
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: "Game Path: " + Settings.gamePath
        }
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
            id: downloadGameButton
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
                VersionUpdate.download_from_github();
                downloadProgress.visible = true;
                gamePathButton.enabled = false;
                gamePathButton.highlighted = false;
                enabled = false;
                highlighted = false;
            }
        }
        Label {
            text: "Game Install Directory: " + Settings.gamePath
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
                    downloadGameButton.enabled = true;
                    downloadGameButton.highlighted = true;
                    gamePathButton.enabled = true;
                    gamePathButton.highlighted = true;
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
