pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import CubedLauncher

Card {
    id: root
    implicitHeight: managerColumn.implicitHeight + Theme.space32 * 2

    ColumnLayout {
        id: managerColumn
        anchors.fill: parent
        anchors.margins: Theme.space32
        spacing: Theme.space16

        Component.onCompleted: GameUpdate.gameInstallPath = Settings.gameDir

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.space12
            MdIcon {
                name: "download"
                color: Theme.primary
                iconSize: 32
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Label {
                    text: CubedGame.installed ? qsTr("Update Cubed") : qsTr("Install Cubed")
                    color: Theme.surfaceForeground
                    font.pixelSize: Theme.titleSize
                    font.weight: Font.DemiBold
                }
                Label {
                    text: CubedGame.installed ? qsTr("Installed version: %1").arg(CubedGame.version) : qsTr("The game has not been installed on this device.")
                    color: Theme.surfaceVariantForeground
                    font.pixelSize: Theme.bodySize
                }
            }
            StatusChip {
                text: GameUpdate.downloading ? qsTr("Downloading") : GameUpdate.checkingUpdate ? qsTr("Checking") : CubedGame.installed ? qsTr("Installed") : qsTr("Not installed")
                iconName: GameUpdate.downloading ? "download" : CubedGame.installed ? "check" : "info"
                tone: CubedGame.installed ? "success" : "neutral"
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 1
            color: Theme.outlineVariant
        }

        GridLayout {
            Layout.fillWidth: true
            columns: root.width >= 760 ? 2 : 1
            columnSpacing: Theme.space24
            rowSpacing: Theme.space16

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Theme.space12
                Label {
                    text: qsTr("Download source")
                    color: Theme.surfaceForeground
                    font.pixelSize: Theme.bodyLargeSize
                    font.weight: Font.DemiBold
                }
                MdSwitch {
                    id: downloadSource
                    text: qsTr("Use custom link")
                }
                MdButton {
                    visible: !downloadSource.checked
                    Layout.fillWidth: true
                    variant: "tonal"
                    iconName: "public"
                    text: {
                        const index = Settings.mirrorIndex >= 0 ? Settings.mirrorIndex : (SystemInfo.isInChina ? 1 : 0);
                        return qsTr("Mirror: ") + MirrorSource.names[index];
                    }
                    onClicked: mirrorPopup.open()
                }
                MdTextField {
                    id: downloadLink
                    visible: downloadSource.checked
                    Layout.fillWidth: true
                    placeholderText: qsTr("Download Link")
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Theme.space12
                Label {
                    text: qsTr("Install location")
                    color: Theme.surfaceForeground
                    font.pixelSize: Theme.bodyLargeSize
                    font.weight: Font.DemiBold
                }
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: pathLabel.implicitHeight + Theme.space16 * 2
                    radius: Theme.radiusMedium
                    color: Theme.surfaceContainer
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: Theme.space16
                        spacing: Theme.space12
                        MdIcon {
                            name: "folder"
                            color: Theme.surfaceVariantForeground
                        }
                        Label {
                            id: pathLabel
                            Layout.fillWidth: true
                            text: GameUpdate.gameInstallPath.length > 0 ? GameUpdate.gameInstallPath : SystemInfo.defaultGameInstallDir
                            color: Theme.surfaceVariantForeground
                            font.pixelSize: Theme.labelSize
                            wrapMode: Text.WrapAnywhere
                        }
                    }
                }
                MdSwitch {
                    id: advancedOption
                    text: qsTr("Advanced folder options")
                }
                RowLayout {
                    visible: advancedOption.checked
                    Layout.fillWidth: true
                    MdButton {
                        id: gamePathButton
                        Layout.fillWidth: true
                        variant: "outlined"
                        iconName: "folder"
                        text: qsTr("Choose Folder")
                        enabled: !GameUpdate.downloading
                        onClicked: gameFolderDialog.open()
                    }
                    MdIconButton {
                        iconName: "refresh"
                        toolTip: qsTr("Reset path")
                        enabled: !GameUpdate.downloading
                        onClicked: {
                            CubedGame.set_game_dir(SystemInfo.defaultGameInstallDir);
                            Settings.set_game_dir(SystemInfo.defaultGameInstallDir);
                            GameUpdate.gameInstallPath = SystemInfo.defaultGameInstallDir;
                        }
                    }
                }
            }
        }

        MdProgressBar {
            visible: GameUpdate.downloading
            Layout.fillWidth: true
            from: 0
            to: 1
            value: GameUpdate.downloadProgress
        }

        InfoBanner {
            visible: GameUpdate.hasError
            Layout.fillWidth: true
            tone: "error"
            text: GameUpdate.errorMessage
        }
        InfoBanner {
            visible: GameUpdate.downloadFinish && !GameUpdate.hasError
            Layout.fillWidth: true
            iconName: "check"
            text: qsTr("Installation finished successfully.")
        }
        InfoBanner {
            visible: GameUpdate.hasNewVersion && !GameUpdate.checkingUpdate && !GameUpdate.hasError
            Layout.fillWidth: true
            iconName: "update"
            text: qsTr("A new game version is available.")
        }

        RowLayout {
            Layout.fillWidth: true
            Item {
                Layout.fillWidth: true
            }
            MdButton {
                visible: GameUpdate.downloading
                variant: "danger"
                iconName: "stop"
                text: qsTr("Cancel Download")
                onClicked: GameUpdate.cancel_download()
            }
            MdButton {
                visible: !GameUpdate.downloading
                iconName: CubedGame.installed ? "update" : "download"
                text: CubedGame.installed ? qsTr("Update Game") : qsTr("Install Game")
                enabled: !GameUpdate.checkingUpdate && Qt.platform.os === "windows"
                onClicked: {
                    if (!Settings.pathSetted)
                        Settings.set_game_dir(SystemInfo.defaultGameInstallDir);
                    if (downloadSource.checked)
                        GameUpdate.download_game(downloadLink.text);
                    else
                        GameUpdate.download_from_github(Settings.mirrorIndex >= 0 ? Settings.mirrorIndex : (SystemInfo.isInChina ? 1 : 0));
                }
            }
        }
    }

    Connections {
        target: GameUpdate
        function onDownloadingChanged() {
            if (!GameUpdate.downloading && GameUpdate.downloadFinish)
                CubedGame.check_version();
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
