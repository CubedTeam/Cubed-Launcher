pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts

Item {
    anchors.fill: parent
    ColumnLayout {
        anchors.centerIn: parent

        Button {
            id: updateButton
            Layout.preferredWidth: 250
            Layout.preferredHeight: 60
            highlighted: true
            Material.roundedScale: Material.MediumScale
            contentItem: ColumnLayout {
                spacing: 5
                anchors.top: parent.top
                anchors.topMargin: 5
                anchors.horizontalCenter: parent.horizontalCenter

                Text {
                    Layout.alignment: Qt.AlignCenter
                    text: "Launcher New Version"
                    font.bold: true
                    font.pixelSize: 16
                    color: "white"
                }
                Text {
                    Layout.alignment: Qt.AlignCenter

                    text: LauncherUpdate.localVersion + " -> " + LauncherUpdate.remoteVersion
                    color: "white"
                    font.bold: true
                    font.pixelSize: 16
                }
            }
            onClicked: {
                Qt.openUrlExternally("https://github.com/CubedTeam/Cubed-Launcher/releases");
            }
        }

        Button {
            id: updateLauncherButton
            Layout.preferredWidth: 250
            Layout.preferredHeight: 60
            Material.roundedScale: Material.MediumScale
            highlighted: true
            text: "Click me to Update"
            font.bold: true
            font.pixelSize: 16
            onClicked: {
                updatePopup.open();
            }
        }

        Label {
            enabled: LauncherUpdate.hasError
            visible: LauncherUpdate.hasError
            text: LauncherUpdate.errorMessage
            color: Material.color(Material.Red)
            Layout.alignment: Qt.AlignCenter
            font.pixelSize: 20
            font.bold: true
            wrapMode: Text.WrapAnywhere
            Layout.preferredWidth: 500
            horizontalAlignment: Text.AlignHCenter
        }
    }

    Popup {
        id: updatePopup
        anchors.centerIn: Overlay.overlay
        width: 500
        height: 300
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 10

            Label {
                text: "Update Launcher(Only Support Windows)"
                Layout.alignment: Qt.AlignCenter

                font.pixelSize: 20
            }

            Label {
                text: LauncherUpdate.localVersion + " -> " + LauncherUpdate.remoteVersion
                Layout.alignment: Qt.AlignCenter

                font.pixelSize: 20
            }

            // AI-generated: mirror selector, same as DownloadGame.
            ComboBox {
                id: mirrorCombo
                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: 250
                font.pixelSize: 20
                model: MirrorSource.names
                currentIndex: Settings.mirrorIndex >= 0 ? Settings.mirrorIndex
                                                        : (SystemInfo.isInChina ? 1 : 0)
                onActivated: Settings.mirrorIndex = currentIndex
            }

            Button {
                id: downloadUpdateButton
                text: "Update"
                enabled: !LauncherUpdate.downloading
                Layout.alignment: Qt.AlignCenter
                font.pixelSize: 20
                Material.roundedScale: Material.MediumScale
                Layout.preferredWidth: 250
                Layout.preferredHeight: 60
                highlighted: enabled
                onClicked: {
                    launcherProgress.visible = true;
                    cancelLauncherButton.visible = true;
                    downloadUpdateButton.enabled = false;
                    downloadUpdateButton.highlighted = false;
                    LauncherUpdate.update_launcher(mirrorCombo.currentIndex);
                }
            }

            // AI-generated: abort the running launcher update download.
            Button {
                id: cancelLauncherButton
                visible: false
                enabled: LauncherUpdate.downloading
                Layout.alignment: Qt.AlignCenter
                Material.roundedScale: Material.MediumScale
                Layout.preferredWidth: 250
                Layout.preferredHeight: 60
                Material.background: Material.color(Material.Red)
                font.pixelSize: 20
                text: "Cancel Download"
                onClicked: {
                    LauncherUpdate.cancel_download();
                }
            }
            ProgressBar {
                id: launcherProgress
                visible: false
                Layout.alignment: Qt.AlignCenter
                from: 0.0
                to: 1.0
                Layout.preferredHeight: 20
                Layout.preferredWidth: 400
                value: LauncherUpdate.downloadProgress
                onValueChanged: {
                    if (value >= to && !LauncherUpdate.hasError) {
                        console.log("Launcher Download Finish");
                        visible = false;
                    }
                }
            }

            // AI-generated: recover the Update button on error/finish.
            Connections {
                target: LauncherUpdate
                function onHasErrorChanged() {
                    if (LauncherUpdate.hasError) {
                        launcherProgress.visible = false;
                        downloadUpdateButton.enabled = true;
                        downloadUpdateButton.highlighted = true;
                        cancelLauncherButton.visible = false;
                    }
                }
                function onDownloadFinishChanged() {
                    if (LauncherUpdate.downloadFinish) {
                        downloadUpdateButton.enabled = true;
                        downloadUpdateButton.highlighted = true;
                        cancelLauncherButton.visible = false;
                    }
                }
                // AI-generated: hide cancel when download ends.
                function onDownloadingChanged() {
                    if (!LauncherUpdate.downloading) {
                        cancelLauncherButton.visible = false;
                        launcherProgress.visible = false;
                    }
                }
            }

            Label {
                enabled: LauncherUpdate.hasError
                visible: LauncherUpdate.hasError
                text: LauncherUpdate.errorMessage
                Layout.alignment: Qt.AlignCenter
                font.pixelSize: 20
                color: Material.color(Material.Red)
                wrapMode: Text.WrapAnywhere
                Layout.preferredWidth: 500
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
