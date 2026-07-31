// AI-generated: easytier multiplayer section. Self-contained: install card,
// control card with peer address / network name / network secret inputs and
// start/stop, virtual IP card with copy-to-clipboard, log card, advanced
// toggle, install path card and folder dialog.
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    id: easytierSection
    implicitWidth: easytierLayout.implicitWidth
    implicitHeight: easytierLayout.implicitHeight
    width: easytierLayout.width
    height: easytierLayout.implicitHeight

    property var logLines: []
    property bool showLog: true

    Component.onCompleted: {
        if (Settings.easytierInstallPath && Settings.easytierInstallPath.length > 0) {
            EasyTierManager.set_install_path(Settings.easytierInstallPath);
        }
    }

    Connections {
        target: Settings
        function onEasytier_install_path_changed() {
            EasyTierManager.set_install_path(Settings.easytierInstallPath);
        }
    }

    Connections {
        target: EasyTierManager
        function onLog_line(line) {
            const lines = easytierSection.logLines.slice();
            lines.push(line);
            if (lines.length > 2000) {
                lines.splice(0, lines.length - 2000);
            }
            easytierSection.logLines = lines;
        }
    }

    ColumnLayout {
        id: easytierLayout
        width: 560
        spacing: 12

        Card {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: easytierInstall.implicitHeight + 20
            visible: !EasyTierManager.installed
            EasyTierManagement {
                id: easytierInstall
                width: parent.width
                anchors.centerIn: parent
            }
        }

        Card {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: controlLayout.implicitHeight + 20
            visible: EasyTierManager.installed
            ColumnLayout {
                id: controlLayout
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Label {
                    Layout.alignment: Qt.AlignCenter
                    text: qsTr("EasyTier Client: %1").arg(EasyTierManager.version)
                    font.pixelSize: 18
                    font.bold: true
                }

                Label {
                    Layout.alignment: Qt.AlignCenter
                    text: qsTr("Status: %1").arg(EasyTierManager.running ? qsTr("Running") : qsTr("Stopped"))
                    font.pixelSize: 16
                    color: EasyTierManager.running ? Material.color(Material.Green) : Material.color(Material.Grey)
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignCenter
                    spacing: 8

                    TextField {
                        id: peerAddressField
                        Layout.preferredWidth: 400
                        Layout.preferredHeight: 44
                        font.pixelSize: 14
                        placeholderText: qsTr("Peer address (e.g. tcp://1.2.3.4:1010)")
                        text: Settings.easytierPeerAddress
                        onEditingFinished: Settings.easytierPeerAddress = text
                        enabled: !EasyTierManager.running
                    }

                    TextField {
                        id: networkNameField
                        Layout.preferredWidth: 400
                        Layout.preferredHeight: 44
                        font.pixelSize: 14
                        placeholderText: qsTr("Network name")
                        text: Settings.easytierNetworkName
                        onEditingFinished: Settings.easytierNetworkName = text
                        enabled: !EasyTierManager.running
                    }

                    TextField {
                        id: networkSecretField
                        Layout.preferredWidth: 400
                        Layout.preferredHeight: 44
                        font.pixelSize: 14
                        placeholderText: qsTr("Network secret")
                        text: Settings.easytierNetworkSecret
                        onEditingFinished: Settings.easytierNetworkSecret = text
                        enabled: !EasyTierManager.running
                        echoMode: TextInput.Password
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignCenter
                    spacing: 12
                    Button {
                        id: startButton
                        Layout.preferredWidth: 160
                        Layout.preferredHeight: 50
                        font.pixelSize: 18
                        Material.roundedScale: Material.MediumScale
                        highlighted: true
                        enabled: EasyTierManager.installed && !EasyTierManager.running && peerAddressField.text.length > 0 && networkNameField.text.length > 0 && networkSecretField.text.length > 0
                        text: qsTr("Start")
                        onClicked: {
                            Settings.easytierPeerAddress = peerAddressField.text;
                            Settings.easytierNetworkName = networkNameField.text;
                            Settings.easytierNetworkSecret = networkSecretField.text;
                            EasyTierManager.start(networkNameField.text, networkSecretField.text, peerAddressField.text);
                        }
                    }
                    Button {
                        id: stopButton
                        Layout.preferredWidth: 160
                        Layout.preferredHeight: 50
                        font.pixelSize: 18
                        Material.roundedScale: Material.MediumScale
                        Material.background: Material.color(Material.Red)
                        enabled: EasyTierManager.running
                        text: qsTr("Stop")
                        onClicked: EasyTierManager.stop()
                    }
                }
            }
        }

        Card {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: ipLayout.implicitHeight + 20
            visible: EasyTierManager.installed
            ColumnLayout {
                id: ipLayout
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8

                Label {
                    Layout.alignment: Qt.AlignCenter
                    text: qsTr("Virtual IP")
                    font.pixelSize: 18
                    font.bold: true
                }

                RowLayout {
                    Layout.alignment: Qt.AlignCenter
                    spacing: 8

                    Label {
                        id: virtualIpLabel
                        Layout.preferredWidth: 240
                        font.pixelSize: 20
                        font.family: "Monospace"
                        horizontalAlignment: Text.AlignHCenter
                        text: EasyTierManager.virtualIp.length > 0 ? EasyTierManager.virtualIp : qsTr("--")
                        color: EasyTierManager.virtualIp.length > 0 ? Material.color(Material.Green) : Material.color(Material.Grey)
                    }

                    Button {
                        Layout.preferredHeight: 40
                        Layout.preferredWidth: 80
                        font.pixelSize: 14
                        Material.roundedScale: Material.MediumScale
                        enabled: EasyTierManager.virtualIp.length > 0
                        text: qsTr("Copy")
                        onClicked: EasyTierManager.copy_to_clipboard(EasyTierManager.virtualIp)
                    }

                    Button {
                        Layout.preferredHeight: 40
                        Layout.preferredWidth: 80
                        font.pixelSize: 14
                        Material.roundedScale: Material.MediumScale
                        enabled: EasyTierManager.running
                        text: qsTr("Refresh")
                        onClicked: EasyTierManager.refresh_virtual_ip()
                    }
                }

                Label {
                    Layout.alignment: Qt.AlignCenter
                    visible: EasyTierManager.running && EasyTierManager.virtualIp.length === 0
                    text: qsTr("Waiting for easytier to assign IP...")
                    font.pixelSize: 12
                    color: Material.color(Material.Grey)
                }
            }
        }

        LogCard {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: implicitHeight
            visible: EasyTierManager.installed
            logLines: easytierSection.logLines
            showLog: easytierSection.showLog
        }

        Card {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: advancedButton.implicitHeight + 20

            Switch {
                id: advancedButton
                font.pixelSize: 14
                anchors.centerIn: parent
                text: qsTr("Advanced")
                checked: false
            }
        }

        Card {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: advancedSetting.implicitHeight + 20
            visible: advancedButton.checked && EasyTierManager.installed
            EasyTierManagement {
                id: advancedSetting
                width: parent.width
                anchors.centerIn: parent
            }
        }

        InstallPathCard {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: implicitHeight
            visible: advancedButton.checked
            manager: EasyTierManager
            pathTitle: qsTr("EasyTier Install Directory")
            setButtonText: qsTr("Set EasyTier Folder")
            onSetClicked: easytierFolderDialog.open()
            onResetClicked: {
                Settings.easytierInstallPath = SystemInfo.defaultEasyTierInstallDir;
                EasyTierManager.set_install_path(SystemInfo.defaultEasyTierInstallDir);
            }
        }
    }

    FolderDialog {
        id: easytierFolderDialog
        title: qsTr("Select EasyTier Folder")
        onAccepted: {
            Settings.set_easytier_install_path_url(selectedFolder);
            EasyTierManager.set_install_path(Settings.easytierInstallPath);
        }
    }
}
