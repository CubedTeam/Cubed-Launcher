pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CubedLauncher

Item {
    id: root
    implicitHeight: banner.implicitHeight

    InfoBanner {
        id: banner
        anchors.left: parent.left
        anchors.right: parent.right
        tone: LauncherUpdate.hasError ? "error" : "info"
        iconName: LauncherUpdate.hasError ? "warning" : "update"
        text: LauncherUpdate.hasError
            ? LauncherUpdate.errorMessage
            : qsTr("Launcher update available: %1 → %2").arg(LauncherUpdate.localVersion).arg(LauncherUpdate.remoteVersion)
        actionText: LauncherUpdate.hasError ? qsTr("Details") : qsTr("Update")
        onActionClicked: updatePopup.open()
    }

    MdDialog {
        id: updatePopup
        parent: Overlay.overlay
        anchors.centerIn: Overlay.overlay
        width: Math.min(560, Overlay.overlay.width - Theme.space32 * 2)
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        title: qsTr("Update Launcher")
        standardButtons: Dialog.NoButton
        padding: Theme.space24
        background: Rectangle { color: Theme.surfaceContainerHigh; radius: Theme.radiusExtraLarge }

        ColumnLayout {
            width: parent.width
            spacing: Theme.space16

            InfoBanner {
                visible: Qt.platform.os !== "windows"
                Layout.fillWidth: true
                tone: "warning"
                text: qsTr("Automatic launcher updates are currently available on Windows only.")
            }

            Label {
                text: LauncherUpdate.localVersion + " → " + LauncherUpdate.remoteVersion
                color: Theme.surfaceForeground
                font.pixelSize: Theme.titleSize
                font.weight: Font.DemiBold
                Layout.alignment: Qt.AlignHCenter
            }

            MdSwitch {
                id: customDownloadSwitch
                text: qsTr("Use custom download link")
                Layout.alignment: Qt.AlignHCenter
            }

            MdButton {
                id: mirrorButton
                visible: !customDownloadSwitch.checked
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
                id: customLinkText
                visible: customDownloadSwitch.checked
                Layout.fillWidth: true
                placeholderText: qsTr("Download Link")
            }

            MdProgressBar {
                id: launcherProgress
                visible: LauncherUpdate.downloading
                Layout.fillWidth: true
                from: 0
                to: 1
                value: LauncherUpdate.downloadProgress
            }

            Label {
                visible: LauncherUpdate.hasError
                text: LauncherUpdate.errorMessage
                color: Theme.error
                font.pixelSize: Theme.bodySize
                wrapMode: Text.WrapAnywhere
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                MdButton {
                    text: qsTr("Release page")
                    variant: "text"
                    onClicked: Qt.openUrlExternally("https://github.com/CubedTeam/Cubed-Launcher/releases")
                }
                MdButton {
                    visible: LauncherUpdate.downloading
                    text: qsTr("Cancel")
                    variant: "danger"
                    iconName: "stop"
                    onClicked: LauncherUpdate.cancel_download()
                }
                MdButton {
                    visible: !LauncherUpdate.downloading
                    text: qsTr("Update")
                    iconName: "update"
                    enabled: Qt.platform.os === "windows"
                    onClicked: {
                        if (customDownloadSwitch.checked)
                            LauncherUpdate.update_launcher_from_url(customLinkText.text);
                        else
                            LauncherUpdate.update_launcher(Settings.mirrorIndex >= 0 ? Settings.mirrorIndex : (SystemInfo.isInChina ? 1 : 0));
                    }
                }
            }
        }

        MirrorSelect {
            id: mirrorPopup
            parent: Overlay.overlay
        }
    }
}
