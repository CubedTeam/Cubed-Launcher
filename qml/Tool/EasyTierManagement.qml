// AI-generated: easytier download / install card. Mirrors FrpManagement but
// without startup controls (not implemented yet).
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts

Item {
    implicitWidth: installLayout.implicitWidth
    implicitHeight: installLayout.implicitHeight
    width: installLayout.width
    height: installLayout.implicitHeight
    ColumnLayout {
        id: installLayout
        width: 560
        spacing: 10

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("EasyTier Manager")
            font.pixelSize: 18
            font.bold: true
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("Not Installed")
            visible: !EasyTierManager.installed && !EasyTierManager.busy
            font.pixelSize: 18
            color: Material.color(Material.Orange)
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("Installed: %1").arg(EasyTierManager.version)
            visible: EasyTierManager.installed && !EasyTierManager.busy
            font.pixelSize: 18
            color: Material.color(Material.Green)
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            visible: EasyTierManager.busy
            text: {
                if (EasyTierManager.state === EasyTierManager.Checking)
                    return qsTr("Checking for updates...");
                if (EasyTierManager.state === EasyTierManager.Downloading)
                    return qsTr("Downloading...");
                if (EasyTierManager.state === EasyTierManager.Extracting)
                    return qsTr("Extracting...");
                return qsTr("Working...");
            }
            font.pixelSize: 18
        }

        ProgressBar {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 400
            from: 0.0
            to: 1.0
            value: EasyTierManager.downloadProgress
            visible: EasyTierManager.state === EasyTierManager.Downloading
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            visible: EasyTierManager.hasError
            text: EasyTierManager.errorMessage
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            color: Material.color(Material.Red)
            font.pixelSize: 16
        }
        Switch {
            id: customDowlaodSwitch
            checked: false
            font.pixelSize: 14
            Layout.alignment: Qt.AlignCenter
            text: qsTr("Custom Link")
        }
        Button {
            id: mirrorButton
            visible: !customDowlaodSwitch.checked
            enabled: visible
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 250
            Layout.preferredHeight: 50
            Material.roundedScale: Material.MediumScale
            highlighted: true
            font.pixelSize: 18
            text: {
                const idx = Settings.mirrorIndex >= 0 ? Settings.mirrorIndex : (SystemInfo.isInChina ? 1 : 0);
                return qsTr("Mirror: ") + MirrorSource.names[idx];
            }
            onClicked: mirrorPopup.open()
        }
        TextField {
            id: customLinkText
            visible: customDowlaodSwitch.checked
            enabled: visible
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 250
            Layout.preferredHeight: 50
            placeholderText: qsTr("Download Link")
        }
        Button {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 250
            Layout.preferredHeight: 50
            font.pixelSize: 18
            Material.roundedScale: Material.MediumScale
            highlighted: true
            enabled: !EasyTierManager.busy
            text: EasyTierManager.installed ? qsTr("Reinstall") : qsTr("Download && Install")
            onClicked: {
                if (customDowlaodSwitch.checked) {
                    EasyTierManager.install_from_url(customLinkText.text);
                } else {
                    EasyTierManager.check_and_install(Settings.mirrorIndex);
                }
            }
        }

        Button {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 250
            Layout.preferredHeight: 50
            font.pixelSize: 16
            Material.roundedScale: Material.MediumScale
            visible: EasyTierManager.installed && !EasyTierManager.busy
            text: qsTr("Uninstall")
            onClicked: EasyTierManager.reset_install()
        }
    }
    MirrorSelect {
        id: mirrorPopup
        parent: Overlay.overlay
    }
}
