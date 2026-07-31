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
        width: 520
        spacing: 10

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("Frp Manager")
            font.pixelSize: 18
            font.bold: true
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("Not Installed")
            visible: !FrpManager.installed && !FrpManager.busy
            font.pixelSize: 18
            color: Material.color(Material.Orange)
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            visible: FrpManager.busy
            text: {
                if (FrpManager.state === FrpManager.Checking)
                    return qsTr("Checking for updates...");
                if (FrpManager.state === FrpManager.Downloading)
                    return qsTr("Downloading...");
                if (FrpManager.state === FrpManager.Extracting)
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
            value: FrpManager.downloadProgress
            visible: FrpManager.state === FrpManager.Downloading
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            visible: FrpManager.hasError
            text: FrpManager.errorMessage
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
            enabled: !FrpManager.busy && !FrpManager.running
            text: FrpManager.installed ? qsTr("Reinstall") : qsTr("Download && Install")
            onClicked: {
                if (customDowlaodSwitch.checked) {
                    FrpManager.install_from_url(customLinkText.text);
                } else {
                    FrpManager.check_and_install(Settings.mirrorIndex);
                }
            }
        }

        Button {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 250
            Layout.preferredHeight: 50
            font.pixelSize: 16
            Material.roundedScale: Material.MediumScale
            visible: FrpManager.installed && !FrpManager.busy
            text: qsTr("Uninstall")
            onClicked: FrpManager.reset_install()
        }
    }
    MirrorSelect {
        id: mirrorPopup
        parent: Overlay.overlay
    }
}
