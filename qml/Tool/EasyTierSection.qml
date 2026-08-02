// AI-generated: easytier multiplayer section. Self-contained: install card,
// control card with create-room / join-room mode tab, public/custom server
// sub-tab, network name / network secret inputs, host virtual IP and
// port-forward inputs (join mode), start/stop, info card (virtual IP for
// create, game address for join) with copy-to-clipboard, log card,
// advanced toggle, install path card and folder dialog. Network identity
// (name/secret) and the custom peer address are intentionally NOT
// persisted.
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
    // 0 = create room (host), 1 = join room (client via --port-forward).
    // Local state only, not persisted.
    property int roomMode: 0
    // 0 = public server (chosen from hardcoded list), 1 = custom address.
    // Local state only, not persisted: the persisted signal is the public
    // server index alone.
    property int serverMode: 0

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

    // AI-generated: resolve the peer address that Start will use, based on
    // the current server mode and the persisted public server index.
    function resolvedPeerAddress() {
        if (easytierSection.serverMode === 0) {
            const idx = Settings.easytierPublicServerIndex >= 0 ? Settings.easytierPublicServerIndex : 0;
            return EasyTierManager.public_server_address(idx);
        }
        return customPeerField.text;
    }

    function publicServerLabel() {
        const names = EasyTierManager.publicServerNames;
        const idx = Settings.easytierPublicServerIndex >= 0 ? Settings.easytierPublicServerIndex : 0;
        if (idx < 0 || idx >= names.length) {
            return qsTr("Select...");
        }
        return names[idx];
    }

    function parsePort(text) {
        const n = parseInt(text);
        if (isNaN(n) || n <= 0 || n > 65535) {
            return -1;
        }
        return n;
    }

    function canStart() {
        if (!EasyTierManager.installed || EasyTierManager.running) {
            return false;
        }
        if (networkNameField.text.length === 0) {
            return false;
        }
        if (networkSecretField.text.length === 0) {
            return false;
        }
        if (easytierSection.resolvedPeerAddress().length === 0) {
            return false;
        }
        if (easytierSection.roomMode === 1) {
            if (hostVirtualIpField.text.length === 0) {
                return false;
            }
            if (easytierSection.parsePort(hostPortField.text) < 0) {
                return false;
            }
            if (easytierSection.parsePort(localPortField.text) < 0) {
                return false;
            }
        }
        return true;
    }

    function gameAddress() {
        const port = easytierSection.parsePort(localPortField.text);
        if (port < 0) {
            return "";
        }
        return "127.0.0.1:" + port;
    }

    ColumnLayout {
        id: easytierLayout
        width: 560
        spacing: 12

        Card {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: easytierInstall.implicitHeight + 20
            visible: !EasyTierManager.installed || EasyTierManager.busy
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

                spacing: 10

                Label {
                    Layout.alignment: Qt.AlignCenter
                    text: qsTr("EasyTier Client")
                    font.pixelSize: 18
                    font.bold: true
                }

                Label {
                    Layout.alignment: Qt.AlignCenter
                    text: qsTr("Status: %1").arg(EasyTierManager.running ? qsTr("Running") : qsTr("Stopped"))
                    font.pixelSize: 16
                    color: EasyTierManager.running ? Material.color(Material.Green) : Material.color(Material.Grey)
                }

                TabBar {
                    id: roomModeBar
                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: 36
                    currentIndex: easytierSection.roomMode
                    onCurrentIndexChanged: easytierSection.roomMode = currentIndex
                    Material.elevation: 0
                    Material.background: "white"

                    TabButton {
                        text: qsTr("Create Room")
                        font.pixelSize: 14
                        width: implicitWidth
                    }
                    TabButton {
                        text: qsTr("Join Room")
                        font.pixelSize: 14
                        width: implicitWidth
                    }
                }

                TabBar {
                    id: serverModeBar
                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: 32
                    currentIndex: easytierSection.serverMode
                    onCurrentIndexChanged: easytierSection.serverMode = currentIndex
                    Material.elevation: 0
                    Material.background: "white"

                    TabButton {
                        text: qsTr("Public")
                        font.pixelSize: 12
                        width: implicitWidth
                    }
                    TabButton {
                        text: qsTr("Custom")
                        font.pixelSize: 12
                        width: implicitWidth
                    }
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignCenter
                    spacing: 8

                    Button {
                        id: publicServerButton
                        visible: easytierSection.serverMode === 0
                        Layout.preferredWidth: 400
                        Layout.preferredHeight: 40
                        font.pixelSize: 13
                        Material.roundedScale: Material.MediumScale
                        highlighted: true
                        enabled: !EasyTierManager.running
                        text: qsTr("Public Server: ") + easytierSection.publicServerLabel()
                        onClicked: publicServerPopup.open()
                    }

                    TextField {
                        id: customPeerField
                        visible: easytierSection.serverMode === 1
                        Layout.preferredWidth: 400
                        Layout.preferredHeight: 40
                        font.pixelSize: 13
                        placeholderText: qsTr("Peer address (e.g. tcp://1.2.3.4:1010)")
                        enabled: !EasyTierManager.running
                    }

                    TextField {
                        id: networkNameField
                        Layout.preferredWidth: 400
                        Layout.preferredHeight: 40
                        font.pixelSize: 13
                        placeholderText: qsTr("Network name")
                        enabled: !EasyTierManager.running
                    }

                    TextField {
                        id: networkSecretField
                        Layout.preferredWidth: 400
                        Layout.preferredHeight: 40
                        font.pixelSize: 13
                        placeholderText: qsTr("Network secret")
                        enabled: !EasyTierManager.running
                        echoMode: TextInput.Password
                    }
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignCenter
                    spacing: 8
                    visible: easytierSection.roomMode === 1

                    TextField {
                        id: hostVirtualIpField
                        Layout.preferredWidth: 400
                        Layout.preferredHeight: 40
                        font.pixelSize: 13
                        placeholderText: qsTr("Host virtual IP (e.g. 10.0.0.1)")
                        enabled: !EasyTierManager.running
                    }

                    RowLayout {
                        Layout.alignment: Qt.AlignCenter
                        spacing: 8

                        TextField {
                            id: hostPortField
                            Layout.preferredWidth: 196
                            Layout.preferredHeight: 40
                            font.pixelSize: 13
                            text: "25530"
                            placeholderText: qsTr("Host port (e.g. 25565)")
                            inputMethodHints: Qt.ImhDigitsOnly
                            validator: IntValidator {
                                bottom: 1
                                top: 65535
                            }
                            enabled: !EasyTierManager.running
                        }

                        TextField {
                            id: localPortField
                            Layout.preferredWidth: 196
                            Layout.preferredHeight: 40
                            font.pixelSize: 13
                            text: "25530"
                            placeholderText: qsTr("Local listen port")
                            inputMethodHints: Qt.ImhDigitsOnly
                            validator: IntValidator {
                                bottom: 1
                                top: 65535
                            }
                            enabled: !EasyTierManager.running
                        }
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
                        enabled: easytierSection.canStart()
                        text: qsTr("Start")
                        onClicked: {
                            if (easytierSection.roomMode === 0) {
                                EasyTierManager.start(networkNameField.text, networkSecretField.text, easytierSection.resolvedPeerAddress());
                            } else {
                                EasyTierManager.start_join(networkNameField.text, networkSecretField.text, easytierSection.resolvedPeerAddress(), hostVirtualIpField.text, easytierSection.parsePort(hostPortField.text), easytierSection.parsePort(localPortField.text));
                            }
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
            Layout.preferredHeight: hostInfoLayout.implicitHeight + 20
            visible: EasyTierManager.installed && easytierSection.roomMode === 0
            ColumnLayout {
                id: hostInfoLayout
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

        Card {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: joinInfoLayout.implicitHeight + 20
            visible: EasyTierManager.installed && easytierSection.roomMode === 1
            ColumnLayout {
                id: joinInfoLayout
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8

                Label {
                    Layout.alignment: Qt.AlignCenter
                    text: qsTr("Game Address")
                    font.pixelSize: 18
                    font.bold: true
                }

                RowLayout {
                    Layout.alignment: Qt.AlignCenter
                    spacing: 8

                    Label {
                        id: gameAddressLabel
                        Layout.preferredWidth: 240
                        font.pixelSize: 20
                        font.family: "Monospace"
                        horizontalAlignment: Text.AlignHCenter
                        text: easytierSection.gameAddress().length > 0 ? easytierSection.gameAddress() : qsTr("--")
                        color: easytierSection.gameAddress().length > 0 ? Material.color(Material.Green) : Material.color(Material.Grey)
                    }

                    Button {
                        Layout.preferredHeight: 40
                        Layout.preferredWidth: 80
                        font.pixelSize: 14
                        Material.roundedScale: Material.MediumScale
                        enabled: easytierSection.gameAddress().length > 0
                        text: qsTr("Copy")
                        onClicked: EasyTierManager.copy_to_clipboard(easytierSection.gameAddress())
                    }
                }

                Label {
                    Layout.alignment: Qt.AlignCenter
                    text: qsTr("Enter this address in Cubed to join the host's server.")
                    font.pixelSize: 12
                    color: Material.color(Material.Grey)
                    wrapMode: Text.WordWrap
                    Layout.preferredWidth: 400
                    horizontalAlignment: Text.AlignHCenter
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

    PublicServerSelect {
        id: publicServerPopup
        parent: Overlay.overlay
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
