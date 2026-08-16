// @checkPropertyInstance Settings C++
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QtCore
import CubedLauncher

PageScaffold {
    id: settingTab
    property url pendingIdentityImport
    title: qsTr("Settings")
    subtitle: qsTr("Personalize the launcher and manage game, identity, and network preferences.")

    Card {
        Layout.fillWidth: true
        implicitHeight: appearanceColumn.implicitHeight + Theme.space32 * 2
        ColumnLayout {
            id: appearanceColumn
            anchors.fill: parent
            anchors.margins: Theme.space24
            spacing: Theme.space8
            SectionHeader {
                Layout.fillWidth: true
                title: qsTr("Appearance")
                subtitle: qsTr("Choose how Cubed Launcher looks on this device.")
                iconName: "palette"
            }
            SettingRow {
                title: qsTr("Theme mode")
                description: qsTr("Follow the system or select a fixed light or dark theme.")
                iconName: Settings.themeMode === 2 ? "dark_mode" : Settings.themeMode === 1 ? "light_mode" : "desktop"
                Repeater {
                    model: [qsTr("System"), qsTr("Light"), qsTr("Dark")]
                    delegate: MdButton {
                        required property int index
                        required property string modelData
                        text: modelData
                        variant: Settings.themeMode === index ? "tonal" : "text"
                        onClicked: Settings.themeMode = index
                    }
                }
            }
            Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: Theme.outlineVariant }
            SettingRow {
                title: qsTr("Color palette")
                description: qsTr("Each palette includes accessible light and dark color roles.")
                iconName: "palette"
                Repeater {
                    model: [
                        { id: "cubed", color: "#4B8003", label: qsTr("Cubed") },
                        { id: "blue", color: "#0B57D0", label: qsTr("Blue") },
                        { id: "violet", color: "#6750A4", label: qsTr("Violet") },
                        { id: "teal", color: "#006A6A", label: qsTr("Teal") },
                        { id: "orange", color: "#C25A00", label: qsTr("Orange") }
                    ]
                    delegate: Button {
                        id: paletteButton
                        required property var modelData
                        implicitWidth: 40
                        implicitHeight: 40
                        hoverEnabled: true
                        Accessible.name: modelData.label
                        contentItem: Item {}
                        background: Rectangle {
                            anchors.centerIn: parent
                            width: 32
                            height: 32
                            radius: 16
                            color: paletteButton.modelData.color
                            border.width: Settings.themePalette === paletteButton.modelData.id ? 3 : 1
                            border.color: Settings.themePalette === paletteButton.modelData.id ? Theme.onSurface : Theme.outline
                            MdIcon {
                                visible: Settings.themePalette === paletteButton.modelData.id
                                anchors.centerIn: parent
                                name: "check"
                                iconSize: 17
                                color: "white"
                            }
                        }
                        ToolTip.visible: hovered
                        ToolTip.text: modelData.label
                        onClicked: Settings.themePalette = modelData.id
                    }
                }
            }
        }
    }

    Card {
        Layout.fillWidth: true
        implicitHeight: generalColumn.implicitHeight + Theme.space32 * 2
        ColumnLayout {
            id: generalColumn
            anchors.fill: parent
            anchors.margins: Theme.space24
            spacing: Theme.space8
            SectionHeader {
                Layout.fillWidth: true
                title: qsTr("General")
                subtitle: qsTr("Language and update preferences.")
                iconName: "settings"
            }
            SettingRow {
                title: qsTr("Language")
                description: qsTr("The interface updates immediately after selection.")
                iconName: "translate"
                MdComboBox {
                    model: ["简体中文", "English"]
                    currentIndex: Settings.language === "en" ? 1 : 0
                    implicitWidth: 180
                    onActivated: Settings.language = currentIndex === 1 ? "en" : "zh_CN"
                }
            }
            Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: Theme.outlineVariant }
            SettingRow {
                title: qsTr("Receive pre-release updates")
                description: qsTr("Include preview releases when checking for launcher and game updates.")
                iconName: "update"
                MdSwitch {
                    checked: Settings.prereleaseUpdates
                    onToggled: {
                        Settings.prereleaseUpdates = checked;
                        LauncherUpdate.check_update("CubedTeam", "Cubed-Launcher");
                        GameUpdate.check_update(CubedGame.installed ? CubedGame.version : "");
                    }
                }
            }
        }
    }

    Card {
        Layout.fillWidth: true
        implicitHeight: identityColumn.implicitHeight + Theme.space32 * 2
        ColumnLayout {
            id: identityColumn
            anchors.fill: parent
            anchors.margins: Theme.space24
            spacing: Theme.space16
            SectionHeader {
                Layout.fillWidth: true
                title: qsTr("Identity & security")
                subtitle: qsTr("Protect and transfer your Cubed player identity.")
                iconName: "badge"
            }
            InfoBanner {
                Layout.fillWidth: true
                tone: "warning"
                iconName: "key"
                text: qsTr("identity.json contains authentication credentials. Never share it or upload it publicly.")
            }
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: identityPath.implicitHeight + Theme.space16 * 2
                radius: Theme.radiusMedium
                color: Theme.surfaceContainer
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.space16
                    MdIcon { name: "folder"; color: Theme.onSurfaceVariant }
                    Label {
                        id: identityPath
                        Layout.fillWidth: true
                        text: IdentityManager.identityPath
                        color: Theme.onSurfaceVariant
                        font.pixelSize: Theme.labelSize
                        wrapMode: Text.WrapAnywhere
                    }
                }
            }
            InfoBanner {
                visible: CubedGame.running
                Layout.fillWidth: true
                tone: "error"
                text: qsTr("Exit Cubed before importing or exporting the identity file.")
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                MdButton {
                    text: qsTr("Import Identity")
                    iconName: "download"
                    variant: "outlined"
                    enabled: !CubedGame.running
                    onClicked: importIdentityDialog.open()
                }
                MdButton {
                    text: qsTr("Export Identity")
                    iconName: "folder"
                    variant: "tonal"
                    enabled: !CubedGame.running
                    onClicked: exportIdentityDialog.open()
                }
            }
        }
    }

    Card {
        Layout.fillWidth: true
        implicitHeight: advancedColumn.implicitHeight + Theme.space32 * 2
        ColumnLayout {
            id: advancedColumn
            anchors.fill: parent
            anchors.margins: Theme.space24
            spacing: Theme.space8
            SectionHeader {
                Layout.fillWidth: true
                title: qsTr("Advanced")
                subtitle: qsTr("Developer options, cache, and GitHub authentication.")
                iconName: "terminal"
            }
            SettingRow {
                title: qsTr("Show advanced options")
                description: qsTr("These settings are intended for troubleshooting and custom setups.")
                iconName: "settings"
                MdSwitch { id: advancedSetting }
            }
            Rectangle { visible: advancedSetting.checked; Layout.fillWidth: true; implicitHeight: 1; color: Theme.outlineVariant }
            SettingRow {
                visible: advancedSetting.checked
                title: qsTr("Wrapper command")
                description: qsTr("Run Cubed through a custom wrapper command.")
                iconName: "terminal"
                MdTextField {
                    implicitWidth: 300
                    placeholderText: qsTr("Wrapper Command")
                    text: Settings.wrapperCommand
                    onEditingFinished: Settings.wrapperCommand = text
                }
            }
            SettingRow {
                visible: advancedSetting.checked
                title: qsTr("Game log")
                description: qsTr("Enable additional launcher logging for Cubed.")
                iconName: "terminal"
                MdSwitch {
                    checked: CubedGame.logOn
                    onToggled: CubedGame.logOn = checked
                }
            }
            SettingRow {
                visible: advancedSetting.checked
                title: qsTr("Cached network data")
                description: qsTr("Clear cached release and mirror responses.")
                iconName: "storage"
                MdButton {
                    text: qsTr("Clear Cache")
                    iconName: "delete"
                    variant: "outlined"
                    onClicked: clearCacheDialog.open()
                }
            }
            SettingRow {
                visible: advancedSetting.checked
                title: qsTr("GitHub token")
                description: qsTr("Stored in the system keyring; environment variables take precedence.")
                iconName: "key"
                MdTextField {
                    id: githubTokenField
                    implicitWidth: 260
                    echoMode: TextInput.Password
                    placeholderText: qsTr("Enter GitHub Token")
                    text: Settings.githubToken
                    onEditingFinished: Settings.githubToken = text
                }
                MdIconButton {
                    visible: Settings.githubToken.length > 0
                    iconName: "delete"
                    variant: "danger"
                    toolTip: qsTr("Remove token")
                    onClicked: Settings.githubToken = ""
                }
            }
        }
    }

    Card {
        Layout.fillWidth: true
        implicitHeight: networkColumn.implicitHeight + Theme.space32 * 2
        ColumnLayout {
            id: networkColumn
            anchors.fill: parent
            anchors.margins: Theme.space24
            spacing: Theme.space16
            SectionHeader {
                Layout.fillWidth: true
                title: qsTr("Network")
                subtitle: qsTr("Configure direct host or client mode for Cubed.")
                iconName: "network"
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.space12
                MdComboBox {
                    id: peerMode
                    implicitWidth: 180
                    model: [qsTr("Host"), qsTr("Client")]
                    onCurrentIndexChanged: CubedGame.set_peer(currentIndex)
                }
                MdTextField {
                    id: hostPort
                    visible: peerMode.currentIndex === 0
                    Layout.fillWidth: true
                    placeholderText: qsTr("Port")
                    inputMethodHints: Qt.ImhDigitsOnly
                    onEditingFinished: CubedGame.set_port(text)
                }
                MdTextField {
                    id: serverIp
                    visible: peerMode.currentIndex === 1
                    Layout.fillWidth: true
                    placeholderText: qsTr("IP address")
                    onEditingFinished: CubedGame.set_ip(text)
                }
                MdTextField {
                    id: serverPort
                    visible: peerMode.currentIndex === 1
                    Layout.preferredWidth: 150
                    placeholderText: qsTr("Port")
                    inputMethodHints: Qt.ImhDigitsOnly
                    onEditingFinished: CubedGame.set_port(text)
                }
            }
        }
    }

    Dialog {
        id: clearCacheDialog
        parent: Overlay.overlay
        anchors.centerIn: Overlay.overlay
        width: 360
        modal: true
        title: qsTr("Clear Cache")
        standardButtons: Dialog.Ok | Dialog.Cancel
        palette.text: Theme.onSurface
        background: Rectangle { color: Theme.surfaceContainerHigh; radius: Theme.radiusExtraLarge }
        onAccepted: Settings.clear_cache()
        Label {
            width: parent.width
            text: qsTr("Clear all cached release and mirror data?")
            color: Theme.onSurface
            wrapMode: Text.WordWrap
        }
    }

    FileDialog {
        id: importIdentityDialog
        title: qsTr("Select Identity File")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("JSON files (*.json)"), qsTr("All files (*)")]
        onAccepted: {
            settingTab.pendingIdentityImport = selectedFile;
            confirmIdentityImportDialog.open();
        }
    }
    FileDialog {
        id: exportIdentityDialog
        title: qsTr("Export Identity File")
        fileMode: FileDialog.SaveFile
        currentFile: StandardPaths.writableLocation(StandardPaths.DocumentsLocation) + "/identity.json"
        defaultSuffix: "json"
        nameFilters: [qsTr("JSON files (*.json)"), qsTr("All files (*)")]
        onAccepted: {
            identityResultDialog.importOperation = false;
            identityResultDialog.succeeded = IdentityManager.export_identity(selectedFile);
            identityResultDialog.open();
        }
    }

    Dialog {
        id: confirmIdentityImportDialog
        parent: Overlay.overlay
        anchors.centerIn: Overlay.overlay
        width: Math.min(480, Overlay.overlay.width - 48)
        modal: true
        closePolicy: Popup.CloseOnEscape
        title: qsTr("Replace Player Identity?")
        standardButtons: Dialog.Ok | Dialog.Cancel
        palette.text: Theme.onSurface
        background: Rectangle { color: Theme.surfaceContainerHigh; radius: Theme.radiusExtraLarge }
        onAccepted: {
            identityResultDialog.importOperation = true;
            identityResultDialog.succeeded = IdentityManager.import_identity(settingTab.pendingIdentityImport);
            identityResultDialog.open();
        }
        InfoBanner {
            width: parent.width
            tone: "warning"
            text: qsTr("Importing this file replaces your current player identity and authentication credentials.")
        }
    }

    Dialog {
        id: identityResultDialog
        property bool importOperation: true
        property bool succeeded: false
        parent: Overlay.overlay
        anchors.centerIn: Overlay.overlay
        width: Math.min(440, Overlay.overlay.width - 48)
        modal: true
        title: succeeded ? qsTr("Identity File Updated") : qsTr("Identity File Operation Failed")
        standardButtons: Dialog.Ok
        palette.text: Theme.onSurface
        background: Rectangle { color: Theme.surfaceContainerHigh; radius: Theme.radiusExtraLarge }
        InfoBanner {
            width: parent.width
            tone: identityResultDialog.succeeded ? "info" : "error"
            iconName: identityResultDialog.succeeded ? "check" : "warning"
            text: {
                if (!identityResultDialog.succeeded)
                    return qsTr("The identity file operation failed: %1").arg(IdentityManager.errorMessage);
                return identityResultDialog.importOperation
                    ? qsTr("The player identity was imported and will be used next time Cubed starts.")
                    : qsTr("The player identity was exported successfully.");
            }
        }
    }
}
