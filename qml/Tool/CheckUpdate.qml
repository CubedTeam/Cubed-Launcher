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
                    text: qsTr("Launcher New Version")
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
            text: qsTr("Click me to Update")
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
                text: qsTr("Update Launcher(Only Support Windows)")
                Layout.alignment: Qt.AlignCenter

                font.pixelSize: 20
            }

            Label {
                text: LauncherUpdate.localVersion + " -> " + LauncherUpdate.remoteVersion
                Layout.alignment: Qt.AlignCenter

                font.pixelSize: 20
            }

            Button {
                id: mirrorButton
                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: 400
                Layout.preferredHeight: 60
                Material.roundedScale: Material.MediumScale
                highlighted: true
                font.pixelSize: 20
                text: {
                    const idx = Settings.mirrorIndex >= 0 ? Settings.mirrorIndex : (SystemInfo.isInChina ? 1 : 0);
                    return qsTr("Mirror: ") + MirrorSource.names[idx];
                }
                onClicked: mirrorPopup.open()
            }

            MirrorSelect {
                id: mirrorPopup
                parent: Overlay.overlay
            }

            Button {
                id: downloadUpdateButton
                text: qsTr("Update")
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
                    const idx = Settings.mirrorIndex >= 0 ? Settings.mirrorIndex : (SystemInfo.isInChina ? 1 : 0);
                    LauncherUpdate.update_launcher(idx);
                }
            }

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
                text: qsTr("Cancel Download")
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

            // AI-generated: re-enable Update button once download ends any way.
            Connections {
                target: LauncherUpdate
                function onDownloadingChanged() {
                    if (!LauncherUpdate.downloading) {
                        downloadUpdateButton.enabled = true && !LauncherUpdate.downloading;
                        downloadUpdateButton.highlighted = downloadUpdateButton.enabled;
                        cancelLauncherButton.visible = false;
                        if (!LauncherUpdate.downloadFinish || LauncherUpdate.hasError) {
                            launcherProgress.visible = false;
                        }
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
