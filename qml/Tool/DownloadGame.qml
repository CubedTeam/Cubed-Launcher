pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    // AI-generated: size to the inner column so Manager.qml's Flickable
    // reports a real height and can scroll correctly.
    implicitWidth: managerColumn.implicitWidth
    implicitHeight: managerColumn.implicitHeight
    width: managerColumn.width
    height: managerColumn.implicitHeight

    ColumnLayout {
        id: managerColumn
        anchors.horizontalCenter: parent.horizontalCenter
        width: 520
        spacing: 10
        Component.onCompleted: {
            GameUpdate.gameInstallPath = Settings.gameDir;
        }

        Card {
            Layout.preferredHeight: sourceLayout.implicitHeight + 20
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            ColumnLayout {
                id: sourceLayout
                anchors.centerIn: parent
                spacing: managerColumn.spacing
                Label {
                    text: qsTr("Download Source")
                    font.pixelSize: 20
                    Layout.alignment: Qt.AlignCenter
                }
                Switch {
                    id: downloadSource
                    text: qsTr("Use Custom Link")
                    checked: false
                    font.pixelSize: 20
                    Layout.alignment: Qt.AlignCenter
                }
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
                        return qsTr("Mirror: ") + MirrorSource.names[idx];
                    }
                    onClicked: mirrorPopup.open()
                }
                TextField {
                    id: downloadLink
                    visible: downloadSource.checked
                    enabled: downloadSource.checked
                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: 400
                    Layout.preferredHeight: 60
                    placeholderText: qsTr("Download Link")
                }
            }
        }

        Card {
            Layout.preferredHeight: actionLayout.implicitHeight + 20
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            ColumnLayout {
                id: actionLayout
                anchors.centerIn: parent
                spacing: managerColumn.spacing
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
                    text: CubedGame.installed ? qsTr("Update Game") : qsTr("Install Game")

                    onClicked: {
                        if (!Settings.pathSetted) {
                            Settings.set_game_dir(SystemInfo.defaultGameInstallDir);
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
                    text: CubedGame.installed ? qsTr("Update Game") : qsTr("Install Game")
                    onClicked: {
                        if (!Settings.pathSetted) {
                            Settings.set_game_dir(SystemInfo.defaultGameInstallDir);
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
                    text: qsTr("Cancel Download")
                    onClicked: {
                        GameUpdate.cancel_download();
                    }
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
                            CubedGame.check_version();
                        }
                    }
                }

                // AI-generated: re-enable Install Game after download ends.
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
            }
        }

        Card {
            Layout.preferredHeight: statusLayout.implicitHeight + 20
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            ColumnLayout {
                id: statusLayout
                anchors.centerIn: parent
                spacing: managerColumn.spacing
                Label {
                    text: qsTr("Checking Update...")
                    visible: GameUpdate.checkingUpdate
                    Layout.alignment: Qt.AlignCenter
                    font.pixelSize: 24
                    font.bold: true
                }
                Label {
                    text: qsTr("Install Finished")
                    visible: GameUpdate.downloadFinish && !GameUpdate.hasError
                    Layout.alignment: Qt.AlignCenter
                    font.pixelSize: 24
                    font.bold: true
                    color: Material.color(Material.Green)

                    onVisibleChanged: {
                        CubedGame.check_version();
                    }
                }

                Label {
                    text: qsTr("A new game version is available")
                    visible: GameUpdate.hasNewVersion && !GameUpdate.checkingUpdate && !GameUpdate.hasError
                    Layout.alignment: Qt.AlignCenter
                    font.pixelSize: 24
                    font.bold: true
                    color: Material.accent
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
                    Layout.preferredWidth: 460
                    horizontalAlignment: Text.AlignHCenter
                }

                Label {
                    text: qsTr("Game Install Directory: ") + GameUpdate.gameInstallPath
                    font.pixelSize: 20
                    Layout.alignment: Qt.AlignCenter
                    wrapMode: Text.WrapAnywhere
                    Layout.preferredWidth: 460
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        Card {
            Layout.preferredHeight: advancedLayout.implicitHeight + 20
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            ColumnLayout {
                id: advancedLayout
                anchors.centerIn: parent
                spacing: managerColumn.spacing
                Switch {
                    id: advancedOpt
                    Layout.alignment: Qt.AlignCenter
                    font.pixelSize: 20
                    text: qsTr("Advanced Option")
                    checked: false
                }
                RowLayout {
                    enabled: advancedOpt.checked
                    visible: advancedOpt.checked
                    Layout.alignment: Qt.AlignCenter
                    spacing: 10
                    Button {
                        id: gamePathButton

                        Material.roundedScale: Material.MediumScale
                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: 250
                        Layout.preferredHeight: 60

                        font.pixelSize: 20

                        highlighted: true
                        text: qsTr("Set Game Folder")
                        onClicked: {
                            gameFolderDialog.open();
                        }
                    }
                    Button {
                        Material.roundedScale: Material.MediumScale
                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: 250
                        Layout.preferredHeight: 60
                        font.pixelSize: 20
                        text: qsTr("Reset Path")
                        onClicked: {
                            CubedGame.set_game_dir(SystemInfo.defaultGameInstallDir);
                            Settings.set_game_dir(SystemInfo.defaultGameInstallDir);
                            GameUpdate.gameInstallPath = SystemInfo.defaultGameInstallDir;
                        }
                    }
                }
            }
        }
    }

    MirrorSelect {
        id: mirrorPopup
        parent: Overlay.overlay
    }

    FolderDialog {
        id: gameFolderDialog
        title: qsTr("Select Game Folder")
        onAccepted: {
            CubedGame.set_game_dir_url(selectedFolder);
            Settings.set_game_dir_url(selectedFolder);
            GameUpdate.gameInstallPath = Settings.gameDir;
        }
    }
}
