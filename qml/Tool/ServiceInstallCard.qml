pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CubedLauncher

Item {
    id: root
    property var manager
    property string title: ""
    property bool showInstalledVersion: true
    property bool blockWhileRunning: true
    implicitHeight: installColumn.implicitHeight

    ColumnLayout {
        id: installColumn
        width: parent.width
        spacing: Theme.space16
        RowLayout {
            Layout.fillWidth: true
            SectionHeader {
                Layout.fillWidth: true
                title: root.title
                subtitle: root.manager.installed
                    ? qsTr("Installed version: %1").arg(root.manager.version)
                    : qsTr("Download and install this multiplayer service.")
                iconName: "download"
            }
            StatusChip {
                text: root.manager.busy ? qsTr("Working")
                      : root.manager.installed ? qsTr("Installed") : qsTr("Not installed")
                iconName: root.manager.busy ? "update" : root.manager.installed ? "check" : "download"
                tone: root.manager.installed ? "success" : "neutral"
            }
        }

        Label {
            visible: root.manager.busy
            Layout.fillWidth: true
            text: root.manager.state === 1 ? qsTr("Checking for updates…")
                : root.manager.state === 2 ? qsTr("Downloading…")
                : root.manager.state === 3 ? qsTr("Extracting…") : qsTr("Working…")
            color: Theme.surfaceVariantForeground
            font.pixelSize: Theme.bodySize
        }
        MdProgressBar {
            visible: root.manager.state === 2
            Layout.fillWidth: true
            from: 0
            to: 1
            value: root.manager.downloadProgress
        }
        InfoBanner {
            visible: root.manager.hasError
            Layout.fillWidth: true
            tone: "error"
            text: root.manager.errorMessage
        }

        MdSwitch {
            id: customDownloadSwitch
            text: qsTr("Use custom download link")
            enabled: !root.manager.busy
        }
        MdButton {
            visible: !customDownloadSwitch.checked
            Layout.fillWidth: true
            variant: "outlined"
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

        RowLayout {
            Layout.fillWidth: true
            MdButton {
                visible: root.manager.installed && !root.manager.busy
                text: qsTr("Uninstall")
                iconName: "delete"
                variant: "outlined"
                enabled: !root.manager.running
                onClicked: root.manager.reset_install()
            }
            Item { Layout.fillWidth: true }
            MdButton {
                text: root.manager.installed ? qsTr("Reinstall") : qsTr("Download & Install")
                iconName: "download"
                enabled: !root.manager.busy && (!root.blockWhileRunning || !root.manager.running)
                onClicked: {
                    if (customDownloadSwitch.checked)
                        root.manager.install_from_url(customLinkText.text);
                    else
                        root.manager.check_and_install(Settings.mirrorIndex);
                }
            }
        }
    }
    MirrorSelect { id: mirrorPopup; parent: Overlay.overlay }
}
