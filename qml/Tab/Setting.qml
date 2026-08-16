// @checkPropertyInstance Settings C++
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore

import CubedLauncher
import QtQuick.Layouts

Item {
    id: settingTab
    Layout.fillHeight: true
    Layout.fillWidth: true
    property url pendingIdentityImport

    Flickable {
        id: settingScroll
        anchors.fill: parent
        contentWidth: width
        contentHeight: settingLayout.implicitHeight + 40
        clip: true
        boundsMovement: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {}

        ColumnLayout {
            id: settingLayout
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 20
            width: 500
            spacing: 10

            Card {
                Layout.preferredHeight: languageLayout.implicitHeight + 20
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                ColumnLayout {
                    id: languageLayout
                    anchors.centerIn: parent
                    spacing: settingLayout.spacing
                    Label {
                        text: qsTr("Language")
                        font.pixelSize: 20
                        Layout.alignment: Qt.AlignCenter
                    }
                    ComboBox {
                        id: languageCombo
                        model: ["简体中文", "English"]
                        currentIndex: Settings.language === "en" ? 1 : 0
                        font.pixelSize: 20
                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: 300
                        onActivated: {
                            Settings.language = currentIndex === 1 ? "en" : "zh_CN";
                        }
                    }
                }
            }

            Card {
                Layout.preferredHeight: colorLayout.implicitHeight + 20
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                ColumnLayout {
                    id: colorLayout
                    anchors.centerIn: parent
                    spacing: settingLayout.spacing
                    Label {
                        text: qsTr("Theme Color")
                        font.pixelSize: 20
                        Layout.alignment: Qt.AlignCenter
                    }
                    RowLayout {

                        spacing: 12

                        Repeater {
                            model: [Material.Red, Material.Pink, Material.Purple, Material.Indigo, Material.Blue, Material.Cyan, Material.Teal, Material.Green, Material.Orange, Material.DeepOrange]

                            delegate: Rectangle {
                                id: colorRect
                                required property int modelData
                                width: 32
                                height: 32
                                radius: 16
                                color: Qt.lighter(Material.color(modelData), 1.2)
                                border.width: Settings.accentColor === Qt.lighter(Material.color(modelData), 1.2) ? 3 : 0
                                border.color: Material.color(Material.Grey, Material.Shade700)

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: Settings.accentColor = Qt.lighter(Material.color(colorRect.modelData), 1.2)
                                }
                            }
                        }
                    }
                }
            }
            Card {
                Layout.preferredHeight: colorfulBorder.implicitHeight + 20
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                Switch {
                    id: colorfulBorder
                    anchors.centerIn: parent
                    font.pixelSize: 20
                    text: qsTr("Colorful Card Border")
                    checked: Settings.cardColorfulBorder
                    onCheckedChanged: Settings.cardColorfulBorder = checked
                }
            }

            Card {
                Layout.preferredHeight: prereleaseUpdates.implicitHeight + 20
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                Switch {
                    id: prereleaseUpdates
                    anchors.centerIn: parent
                    font.pixelSize: 20
                    text: qsTr("Receive Pre-release Updates")
                    checked: Settings.prereleaseUpdates
                    onToggled: {
                        Settings.prereleaseUpdates = checked;
                        LauncherUpdate.check_update("CubedTeam", "Cubed-Launcher");
                        GameUpdate.check_update(CubedGame.installed ? CubedGame.version : "");
                    }
                }
            }

            // AI-generated: Manage the player's authentication identity.
            Card {
                Layout.preferredHeight: identityLayout.implicitHeight + 24
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                ColumnLayout {
                    id: identityLayout
                    anchors.centerIn: parent
                    width: parent.width - 30
                    spacing: settingLayout.spacing
                    Label {
                        text: qsTr("Player Identity")
                        font.pixelSize: 20
                        Layout.alignment: Qt.AlignCenter
                    }
                    Label {
                        text: IdentityManager.identityPath
                        font.pixelSize: 12
                        color: Material.color(Material.Grey)
                        wrapMode: Text.WrapAnywhere
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                    }
                    Label {
                        text: qsTr("identity.json contains your player authentication credentials. Do not share it with anyone or upload it publicly.")
                        font.pixelSize: 14
                        font.bold: true
                        color: Material.color(Material.DeepOrange, Material.Shade700)
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                    }
                    RowLayout {
                        Layout.alignment: Qt.AlignCenter
                        spacing: 10
                        Button {
                            Layout.preferredWidth: 200
                            Layout.preferredHeight: 50
                            font.pixelSize: 16
                            Material.roundedScale: Material.MediumScale
                            text: qsTr("Import Identity File")
                            enabled: !CubedGame.running
                            onClicked: importIdentityDialog.open()
                        }
                        Button {
                            Layout.preferredWidth: 200
                            Layout.preferredHeight: 50
                            font.pixelSize: 16
                            Material.roundedScale: Material.MediumScale
                            text: qsTr("Export Identity File")
                            enabled: !CubedGame.running
                            onClicked: exportIdentityDialog.open()
                        }
                    }
                    Label {
                        visible: CubedGame.running
                        text: qsTr("Exit Cubed before importing or exporting the identity file.")
                        font.pixelSize: 13
                        color: Material.color(Material.Red, Material.Shade700)
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                    }
                }
            }

            Card {
                Layout.preferredHeight: advancedSetting.implicitHeight + 20
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                Switch {
                    id: advancedSetting
                    anchors.centerIn: parent
                    font.pixelSize: 20
                    text: qsTr("Advanced Setting")
                    checked: false
                }
            }

            Card {
                Layout.preferredHeight: wrapperLayout.implicitHeight + 20
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                visible: advancedSetting.checked
                ColumnLayout {
                    id: wrapperLayout
                    anchors.centerIn: parent
                    spacing: settingLayout.spacing
                    Label {
                        text: qsTr("Wrapper Command")
                        font.pixelSize: 20
                        Layout.alignment: Qt.AlignCenter
                    }
                    TextField {
                        id: wrapperCommand
                        Layout.preferredWidth: 300
                        Layout.alignment: Qt.AlignCenter
                        placeholderText: qsTr("Wrapper Command")
                        text: Settings.wrapperCommand
                        onEditingFinished: {
                            Settings.wrapperCommand = wrapperCommand.text;
                        }
                    }
                }
            }

            Card {
                Layout.preferredHeight: logLayout.implicitHeight + 20
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                visible: advancedSetting.checked
                ColumnLayout {
                    id: logLayout
                    anchors.centerIn: parent
                    spacing: settingLayout.spacing
                    Label {
                        text: qsTr("Log")
                        font.pixelSize: 20
                        Layout.alignment: Qt.AlignCenter
                    }
                    Switch {
                        id: logStatus
                        checked: false
                        font.pixelSize: 20
                        Layout.alignment: Qt.AlignCenter
                        onCheckedChanged: {
                            CubedGame.logOn = logStatus.checked;
                        }
                        text: checked ? qsTr("On") : qsTr("Off")
                    }
                }
            }

            Card {
                Layout.preferredHeight: cacheLayout.implicitHeight + 20
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                visible: advancedSetting.checked
                ColumnLayout {
                    id: cacheLayout
                    anchors.centerIn: parent
                    spacing: settingLayout.spacing
                    Label {
                        text: qsTr("Cache")
                        font.pixelSize: 20
                        Layout.alignment: Qt.AlignCenter
                    }
                    Button {
                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: 250
                        Layout.preferredHeight: 50
                        font.pixelSize: 18
                        Material.roundedScale: Material.MediumScale
                        text: qsTr("Clear Cache")
                        onClicked: clearCacheDialog.open()
                    }
                }
            }

            Card {
                Layout.preferredHeight: tokenLayout.implicitHeight + 20
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                visible: advancedSetting.checked
                ColumnLayout {
                    id: tokenLayout
                    anchors.centerIn: parent
                    spacing: settingLayout.spacing
                    Label {
                        text: qsTr("GitHub Token")
                        font.pixelSize: 20
                        Layout.alignment: Qt.AlignCenter
                    }
                    TextField {
                        id: githubTokenField
                        Layout.preferredWidth: 300
                        Layout.alignment: Qt.AlignCenter
                        echoMode: TextInput.Password
                        placeholderText: qsTr("Enter GitHub Token")
                        text: Settings.githubToken
                        onEditingFinished: {
                            Settings.githubToken = githubTokenField.text;
                        }
                    }
                    Label {
                        text: qsTr("Token stored in system keyring. Environment variable takes precedence.")
                        font.pixelSize: 12
                        Layout.alignment: Qt.AlignCenter
                        color: Material.color(Material.Grey)
                        wrapMode: Text.WordWrap
                        Layout.maximumWidth: 320
                    }
                    Button {
                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: 250
                        Layout.preferredHeight: 40
                        font.pixelSize: 16
                        Material.roundedScale: Material.MediumScale
                        visible: Settings.githubToken.length > 0
                        text: qsTr("Remove Token")
                        onClicked: Settings.githubToken = ""
                    }
                }
            }

            Card {
                Layout.preferredHeight: networkLayout.implicitHeight + 20
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                visible: advancedSetting.checked
                ColumnLayout {
                    id: networkLayout
                    anchors.centerIn: parent
                    spacing: settingLayout.spacing
                    Label {
                        text: qsTr("Network")
                        font.pixelSize: 20
                        Layout.alignment: Qt.AlignCenter
                    }
                    ComboBox {
                        id: peerMode
                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: 300
                        font.pixelSize: 20
                        model: [qsTr("Host"), qsTr("Client")]
                        currentIndex: 0
                        onCurrentIndexChanged: {
                            CubedGame.set_peer(peerMode.currentIndex);
                        }
                    }
                    TextField {
                        id: hostPort
                        visible: peerMode.currentIndex == 0
                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: 300
                        placeholderText: qsTr("Port")
                        onEditingFinished: {
                            CubedGame.set_port(hostPort.text);
                        }
                    }
                    RowLayout {
                        visible: peerMode.currentIndex == 1
                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: 300
                        spacing: 10
                        TextField {
                            id: serverIp
                            Layout.fillWidth: true
                            placeholderText: qsTr("Ip")
                            onEditingFinished: {
                                CubedGame.set_ip(serverIp.text);
                            }
                        }
                        TextField {
                            id: serverPort
                            Layout.fillWidth: true
                            placeholderText: qsTr("Port")
                            onEditingFinished: {
                                CubedGame.set_port(serverPort.text);
                            }
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: clearCacheDialog
        anchors.centerIn: Overlay.overlay
        width: 250
        height: 150
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        title: qsTr("Clear Cache")
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: Settings.clear_cache()
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
        anchors.centerIn: Overlay.overlay
        width: 460
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape
        title: qsTr("Replace Player Identity?")
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: {
            identityResultDialog.importOperation = true;
            identityResultDialog.succeeded = IdentityManager.import_identity(settingTab.pendingIdentityImport);
            identityResultDialog.open();
        }

        Label {
            width: parent.width
            text: qsTr("Importing this file will replace your current player identity and authentication credentials. Continue?")
            wrapMode: Text.WordWrap
        }
    }

    Dialog {
        id: identityResultDialog
        property bool importOperation: true
        property bool succeeded: false
        anchors.centerIn: Overlay.overlay
        width: 420
        modal: true
        focus: true
        title: succeeded ? qsTr("Identity File Updated") : qsTr("Identity File Operation Failed")
        standardButtons: Dialog.Ok

        Label {
            width: parent.width
            text: {
                if (!identityResultDialog.succeeded) {
                    return qsTr("The identity file operation failed: %1").arg(IdentityManager.errorMessage);
                }
                return identityResultDialog.importOperation
                    ? qsTr("The player identity was imported. It will take effect the next time Cubed starts.")
                    : qsTr("The player identity was exported successfully.");
            }
            wrapMode: Text.WordWrap
        }
    }
}
