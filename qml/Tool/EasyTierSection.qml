pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import CubedLauncher

Item {
    id: root
    implicitHeight: content.implicitHeight
    property var logLines: []
    property int roomMode: 0
    property int serverMode: 0
    property string roomCode: ""
    property string joinCode: ""
    property bool joinCodeValid: false

    Component.onCompleted: {
        if (Settings.easytierInstallPath.length > 0)
            EasyTierManager.set_install_path(Settings.easytierInstallPath);
        regenerateRoomCode();
    }
    Connections {
        target: Settings
        function onEasytier_install_path_changed() { EasyTierManager.set_install_path(Settings.easytierInstallPath); }
    }
    Connections {
        target: EasyTierManager
        function onLog_line(line) {
            const lines = root.logLines.slice();
            lines.push(line);
            if (lines.length > 2000)
                lines.splice(0, lines.length - 2000);
            root.logLines = lines;
        }
    }

    function resolvedPeerAddress() {
        if (serverMode === 0) {
            const index = Settings.easytierPublicServerIndex >= 0 ? Settings.easytierPublicServerIndex : 0;
            return EasyTierManager.public_server_address(index);
        }
        return customPeerField.text;
    }
    function publicServerLabel() {
        const names = EasyTierManager.publicServerNames;
        const index = Settings.easytierPublicServerIndex >= 0 ? Settings.easytierPublicServerIndex : 0;
        return index >= 0 && index < names.length ? names[index] : qsTr("Select…");
    }
    function regenerateRoomCode() { roomCode = EasyTierManager.generate_room_code(); }
    function resolvedCredentials() {
        if (advancedToggle.checked && networkNameField.text.length > 0 && networkSecretField.text.length > 0)
            return { name: networkNameField.text, secret: networkSecretField.text };
        const code = roomMode === 0 ? roomCode : joinCode;
        if (code.length === 0)
            return { name: "", secret: "" };
        const credentials = EasyTierManager.credentials_for_code(code);
        return { name: credentials.name, secret: credentials.secret };
    }
    function canStart() {
        if (!EasyTierManager.installed || EasyTierManager.running || resolvedPeerAddress().length === 0)
            return false;
        const credentials = resolvedCredentials();
        if (credentials.name.length === 0 || credentials.secret.length === 0)
            return false;
        if (roomMode === 1 && !(advancedToggle.checked && networkNameField.text.length > 0 && networkSecretField.text.length > 0))
            return joinCodeValid;
        return true;
    }

    ColumnLayout {
        id: content
        width: parent.width
        spacing: Theme.space24

        Card {
            visible: !EasyTierManager.installed || EasyTierManager.busy
            Layout.fillWidth: true
            implicitHeight: installer.implicitHeight + Theme.space32 * 2
            EasyTierManagement {
                id: installer
                anchors.fill: parent
                anchors.margins: Theme.space24
            }
        }

        Card {
            visible: EasyTierManager.installed
            Layout.fillWidth: true
            implicitHeight: controlColumn.implicitHeight + Theme.space32 * 2
            ColumnLayout {
                id: controlColumn
                anchors.fill: parent
                anchors.margins: Theme.space24
                spacing: Theme.space16

                RowLayout {
                    Layout.fillWidth: true
                    SectionHeader {
                        Layout.fillWidth: true
                        title: qsTr("EasyTier client")
                        subtitle: qsTr("Share a room code to create a private virtual network.")
                        iconName: "hub"
                    }
                    StatusChip {
                        text: EasyTierManager.running ? qsTr("Running") : qsTr("Stopped")
                        iconName: EasyTierManager.running ? "play_arrow" : "stop"
                        tone: EasyTierManager.running ? "success" : "neutral"
                    }
                }

                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    implicitWidth: roomModeRow.implicitWidth + Theme.space8
                    implicitHeight: 52
                    radius: 26
                    color: Theme.surfaceContainer
                    RowLayout {
                        id: roomModeRow
                        anchors.fill: parent
                        anchors.margins: Theme.space4
                        spacing: Theme.space4
                        MdButton {
                            text: qsTr("Create Room")
                            variant: root.roomMode === 0 ? "tonal" : "text"
                            onClicked: { root.roomMode = 0; if (root.roomCode.length === 0) root.regenerateRoomCode(); }
                        }
                        MdButton {
                            text: qsTr("Join Room")
                            variant: root.roomMode === 1 ? "tonal" : "text"
                            onClicked: root.roomMode = 1
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.space12
                    Label {
                        text: qsTr("Relay server")
                        color: Theme.surfaceForeground
                        font.pixelSize: Theme.bodyLargeSize
                        font.weight: Font.DemiBold
                        Layout.fillWidth: true
                    }
                    MdButton {
                        text: qsTr("Public")
                        variant: root.serverMode === 0 ? "tonal" : "text"
                        onClicked: root.serverMode = 0
                    }
                    MdButton {
                        text: qsTr("Custom")
                        variant: root.serverMode === 1 ? "tonal" : "text"
                        onClicked: root.serverMode = 1
                    }
                }
                MdButton {
                    visible: root.serverMode === 0
                    Layout.fillWidth: true
                    variant: "outlined"
                    iconName: "public"
                    text: qsTr("Public server: %1").arg(root.publicServerLabel())
                    enabled: !EasyTierManager.running
                    onClicked: publicServerPopup.open()
                }
                MdTextField {
                    id: customPeerField
                    visible: root.serverMode === 1
                    Layout.fillWidth: true
                    placeholderText: qsTr("Peer address, for example tcp://1.2.3.4:1010")
                    enabled: !EasyTierManager.running
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: roomCodeColumn.implicitHeight + Theme.space16 * 2
                    radius: Theme.radiusLarge
                    color: Theme.primaryContainer
                    ColumnLayout {
                        id: roomCodeColumn
                        anchors.fill: parent
                        anchors.margins: Theme.space16
                        spacing: Theme.space12
                        Label {
                            text: root.roomMode === 0 ? qsTr("Room code") : qsTr("Enter room code")
                            color: Theme.primaryContainerForeground
                            font.pixelSize: Theme.bodySize
                            font.weight: Font.DemiBold
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                visible: root.roomMode === 0
                                Layout.fillWidth: true
                                text: root.roomCode
                                color: Theme.primaryContainerForeground
                                font.family: "Monospace"
                                font.pixelSize: Theme.headlineSize
                                font.weight: Font.Bold
                                horizontalAlignment: Text.AlignHCenter
                                font.letterSpacing: 3
                            }
                            MdTextField {
                                id: joinCodeField
                                visible: root.roomMode === 1
                                Layout.fillWidth: true
                                placeholderText: qsTr("6-character code")
                                font.family: "Monospace"
                                maximumLength: 6
                                enabled: !EasyTierManager.running
                                onTextChanged: {
                                    root.joinCode = text.toUpperCase();
                                    root.joinCodeValid = EasyTierManager.is_valid_room_code(root.joinCode);
                                }
                            }
                            MdIconButton {
                                visible: root.roomMode === 0
                                iconName: "copy"
                                toolTip: qsTr("Copy room code")
                                enabled: root.roomCode.length > 0
                                onClicked: EasyTierManager.copy_to_clipboard(root.roomCode)
                            }
                            MdIconButton {
                                visible: root.roomMode === 0
                                iconName: "refresh"
                                toolTip: qsTr("Generate a new code")
                                enabled: !EasyTierManager.running
                                onClicked: root.regenerateRoomCode()
                            }
                        }
                        Label {
                            visible: root.roomMode === 1 && root.joinCode.length > 0 && !root.joinCodeValid
                            text: qsTr("Enter a valid 6-character room code.")
                            color: Theme.error
                            font.pixelSize: Theme.labelSize
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Item { Layout.fillWidth: true }
                    MdButton {
                        visible: !EasyTierManager.running
                        text: qsTr("Start")
                        iconName: "play_arrow"
                        enabled: root.canStart()
                        onClicked: {
                            const credentials = root.resolvedCredentials();
                            if (root.roomMode === 0)
                                EasyTierManager.start(credentials.name, credentials.secret, root.resolvedPeerAddress());
                            else
                                EasyTierManager.start_join(credentials.name, credentials.secret, root.resolvedPeerAddress());
                        }
                    }
                    MdButton {
                        visible: EasyTierManager.running
                        text: qsTr("Stop")
                        iconName: "stop"
                        variant: "danger"
                        onClicked: EasyTierManager.stop()
                    }
                }
            }
        }

        InfoBanner {
            visible: EasyTierManager.installed
            Layout.fillWidth: true
            tone: "error"
            iconName: "warning"
            text: qsTr("EasyTier requests administrator or root privileges to create a TUN/TAP interface and can control this machine's network stack. Only continue if you trust easytier-core. Frp does not require elevated privileges.")
        }

        Card {
            visible: EasyTierManager.installed && root.roomMode === 0
            Layout.fillWidth: true
            implicitHeight: virtualIpRow.implicitHeight + Theme.space32 * 2
            RowLayout {
                id: virtualIpRow
                anchors.fill: parent
                anchors.margins: Theme.space24
                spacing: Theme.space12
                MdIcon { name: "network"; color: Theme.primary; iconSize: 28 }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Label { text: qsTr("Virtual IP"); color: Theme.surfaceVariantForeground; font.pixelSize: Theme.labelSize }
                    Label {
                        text: EasyTierManager.virtualIp.length > 0 ? EasyTierManager.virtualIp : qsTr("Waiting for address…")
                        color: Theme.surfaceForeground
                        font.family: "Monospace"
                        font.pixelSize: Theme.titleSize
                        font.weight: Font.DemiBold
                    }
                }
                MdIconButton {
                    iconName: "copy"
                    toolTip: qsTr("Copy virtual IP")
                    enabled: EasyTierManager.virtualIp.length > 0
                    onClicked: EasyTierManager.copy_to_clipboard(EasyTierManager.virtualIp)
                }
                MdIconButton {
                    iconName: "refresh"
                    toolTip: qsTr("Refresh virtual IP")
                    enabled: EasyTierManager.running
                    onClicked: EasyTierManager.refresh_virtual_ip()
                }
            }
        }

        LogCard {
            visible: EasyTierManager.installed
            Layout.fillWidth: true
            logLines: root.logLines
            title: qsTr("EasyTier logs")
        }

        Card {
            Layout.fillWidth: true
            implicitHeight: advancedColumn.implicitHeight + Theme.space32 * 2
            ColumnLayout {
                id: advancedColumn
                anchors.fill: parent
                anchors.margins: Theme.space24
                spacing: Theme.space16
                SettingRow {
                    title: qsTr("Advanced EasyTier options")
                    description: qsTr("Override network credentials, reinstall the service, or change its folder.")
                    iconName: "settings"
                    MdSwitch { id: advancedToggle }
                }
                EasyTierManagement {
                    visible: advancedToggle.checked && EasyTierManager.installed
                    Layout.fillWidth: true
                }
                ColumnLayout {
                    visible: advancedToggle.checked && EasyTierManager.installed
                    Layout.fillWidth: true
                    spacing: Theme.space8
                    Label {
                        text: qsTr("Manual network identity")
                        color: Theme.surfaceForeground
                        font.pixelSize: Theme.bodyLargeSize
                        font.weight: Font.DemiBold
                    }
                    Label {
                        text: qsTr("When both fields are filled, they override the room code.")
                        color: Theme.surfaceVariantForeground
                        font.pixelSize: Theme.labelSize
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        MdTextField {
                            id: networkNameField
                            Layout.fillWidth: true
                            placeholderText: qsTr("Network name")
                            enabled: !EasyTierManager.running
                        }
                        MdTextField {
                            id: networkSecretField
                            Layout.fillWidth: true
                            placeholderText: qsTr("Network secret")
                            echoMode: TextInput.Password
                            enabled: !EasyTierManager.running
                        }
                    }
                }
                InstallPathCard {
                    visible: advancedToggle.checked
                    Layout.fillWidth: true
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
        }
    }

    PublicServerSelect { id: publicServerPopup; parent: Overlay.overlay }
    FolderDialog {
        id: easytierFolderDialog
        title: qsTr("Select EasyTier Folder")
        onAccepted: {
            Settings.set_easytier_install_path_url(selectedFolder);
            EasyTierManager.set_install_path(Settings.easytierInstallPath);
        }
    }
}
