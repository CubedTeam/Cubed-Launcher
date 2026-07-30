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
        id: managerColumn
        anchors.centerIn: parent
        Component.onCompleted: {
            GameUpdate.gameInstallPath = Settings.gamePath;
        }
        Switch {
            id: downloadSource
            text: "Use Custom Link"
            checked: false
            font.pixelSize: 20
            Layout.alignment: Qt.AlignCenter
        }
        // AI-generated: opens the mirror picker popup. The chosen index is read
        // back from Settings.mirrorIndex at click time.
        Button {
            id: mirrorButton
            visible: !downloadSource.checked
            enabled: !downloadSource.checked
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 400
            Layout.preferredHeight: 60
            Material.roundedScale: Material.MediumScale
            highlighted: enabled
            font.pixelSize: 20
            text: {
                const idx = Settings.mirrorIndex >= 0 ? Settings.mirrorIndex : (SystemInfo.isInChina ? 1 : 0);
                return "Mirror: " + MirrorSource.names[idx];
            }
            onClicked: mirrorPopup.open()
        }

        MirrorSelect {
            id: mirrorPopup
            parent: Overlay.overlay
        }
        Button {
            id: downloadGameGithubButton
            visible: !downloadSource.checked
            enabled: !downloadSource.checked && !GameUpdate.checkingUpdate && !GameUpdate.downloading
            Layout.alignment: Qt.AlignCenter
            Material.roundedScale: Material.MediumScale
            Layout.preferredWidth: 250
            Layout.preferredHeight: 60
            highlighted: enabled

            font.pixelSize: 20
            text: CubedInstance.installed ? "Update Game" : "Install Game"

            onClicked: {
                if (!Settings.pathSetted) {
                    Settings.set_game_path(SystemInfo.defaultGameFilePath);
                }

                downloadProgress.visible = true;
                cancelButton.visible = true;
                gamePathButton.enabled = false;
                gamePathButton.highlighted = false;
                enabled = false;
                highlighted = false;
                const idx = Settings.mirrorIndex >= 0 ? Settings.mirrorIndex : (SystemInfo.isInChina ? 1 : 0);
                GameUpdate.download_from_github(idx);
            }
        }

        // AI-generated: abort the running download; shown only while downloading.
        Button {
            id: cancelButton
            visible: false
            enabled: GameUpdate.downloading
            Layout.alignment: Qt.AlignCenter
            Material.roundedScale: Material.MediumScale
            Layout.preferredWidth: 250
            Layout.preferredHeight: 60
            highlighted: enabled
            Material.background: Material.color(Material.Red)

            font.pixelSize: 20
            text: "Cancel Download"
            onClicked: {
                GameUpdate.cancel_download();
            }
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
            text: CubedInstance.installed ? "Update Game" : "Install Game"
            onClicked: {
                if (!Settings.pathSetted) {
                    Settings.set_game_path(SystemInfo.defaultGameFilePath);
                }

                downloadProgress.visible = true;
                cancelButton.visible = true;
                gamePathButton.enabled = false;
                gamePathButton.highlighted = false;
                enabled = false;
                highlighted = false;
                GameUpdate.download_game(downloadLink.text);
            }
        }
        Label {
            text: "Checking Update..."
            visible: GameUpdate.checkingUpdate
            Layout.alignment: Qt.AlignCenter
            font.pixelSize: 24
            font.bold: true
        }
        Label {
            text: "Install Finished"
            visible: GameUpdate.downloadFinish && !GameUpdate.hasError
            Layout.alignment: Qt.AlignCenter
            font.pixelSize: 24
            font.bold: true
            color: Material.color(Material.Green)

            onVisibleChanged: {
                CubedInstance.check_version();
            }
        }

        Label {
            text: GameUpdate.errorMessage
            visible: GameUpdate.hasError
            enabled: GameUpdate.hasError
            font.bold: true
            Layout.alignment: Qt.AlignCenter
            font.pixelSize: 24
            color: Material.color(Material.Red)
            wrapMode: Text.WrapAnywhere
            Layout.preferredWidth: 600
            horizontalAlignment: Text.AlignHCenter
        }

        Label {
            text: "Game Install Directory: " + GameUpdate.gameInstallPath
            font.pixelSize: 20
        }

        ProgressBar {
            id: downloadProgress
            Layout.alignment: Qt.AlignCenter
            visible: false
            from: 0.0
            to: 1.0
            Layout.preferredHeight: 20
            Layout.preferredWidth: 400
            value: GameUpdate.downloadProgress
            onValueChanged: {
                if (value >= to && !GameUpdate.hasError) {
                    console.log("Download Finish");
                    visible = false;
                    CubedInstance.check_version();
                }
            }
        }

        // AI-generated: re-enable controls once any download ends (finish,
        // error, or cancel) so Install Game never stays disabled.
        Connections {
            target: GameUpdate
            function onDownloadingChanged() {
                if (!GameUpdate.downloading) {
                    downloadGameGithubButton.enabled = true && !downloadSource.checked && !GameUpdate.checkingUpdate;
                    downloadGameGithubButton.highlighted = downloadGameGithubButton.enabled;
                    downloadGameCustomButton.enabled = true && downloadSource.checked;
                    downloadGameCustomButton.highlighted = downloadGameCustomButton.enabled;
                    gamePathButton.enabled = true;
                    gamePathButton.highlighted = true;
                    cancelButton.visible = false;
                    if (!GameUpdate.downloadFinish || GameUpdate.hasError) {
                        downloadProgress.visible = false;
                    }
                }
            }
        }

        Switch {
            id: advancedOpt
            Layout.alignment: Qt.AlignCenter
            font.pixelSize: 20
            text: "Advanced Option"
            checked: false
        }
    }
    RowLayout {
        enabled: advancedOpt.checked
        visible: advancedOpt.checked
        anchors.top: managerColumn.bottom
        anchors.horizontalCenter: managerColumn.horizontalCenter
        anchors.topMargin: 20
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
                GameUpdate.gameInstallPath = SystemInfo.defaultGameFilePath;
            }
        }
    }
    FileDialog {
        id: gameFileDialog
        title: "Select Cubed Game"
        onAccepted: {
            CubedInstance.set_game_path_url(selectedFile);
            Settings.gamePath = selectedFile;
            GameUpdate.gameInstallPath = Settings.gamePath;
        }
    }
}
