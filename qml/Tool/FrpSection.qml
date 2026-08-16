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

    Component.onCompleted: {
        if (Settings.frpInstallPath.length > 0)
            FrpManager.set_install_path(Settings.frpInstallPath);
    }
    Connections {
        target: Settings
        function onFrp_install_path_changed() { FrpManager.set_install_path(Settings.frpInstallPath); }
    }
    Connections {
        target: FrpManager
        function onLog_line(line) {
            const lines = root.logLines.slice();
            lines.push(line);
            if (lines.length > 2000)
                lines.splice(0, lines.length - 2000);
            root.logLines = lines;
        }
    }

    ColumnLayout {
        id: content
        width: parent.width
        spacing: Theme.space24

        Card {
            visible: !FrpManager.installed || FrpManager.busy
            Layout.fillWidth: true
            implicitHeight: installer.implicitHeight + Theme.space32 * 2
            FrpManagement {
                id: installer
                anchors.fill: parent
                anchors.margins: Theme.space24
            }
        }

        Card {
            visible: FrpManager.installed
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
                        title: qsTr("Frp client")
                        subtitle: qsTr("Connect through a configured Frp server without elevated privileges.")
                        iconName: "network"
                    }
                    StatusChip {
                        text: FrpManager.running ? qsTr("Running") : qsTr("Stopped")
                        iconName: FrpManager.running ? "play_arrow" : "stop"
                        tone: FrpManager.running ? "success" : "neutral"
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    MdButton {
                        text: qsTr("Edit frpc.toml")
                        iconName: "edit"
                        variant: "outlined"
                        enabled: !FrpManager.running
                        onClicked: tomlDialog.open()
                    }
                    Item { Layout.fillWidth: true }
                    MdButton {
                        visible: !FrpManager.running
                        text: qsTr("Start")
                        iconName: "play_arrow"
                        enabled: FrpManager.installed
                        onClicked: FrpManager.start()
                    }
                    MdButton {
                        visible: FrpManager.running
                        text: qsTr("Stop")
                        iconName: "stop"
                        variant: "danger"
                        onClicked: FrpManager.stop()
                    }
                }
            }
        }

        LogCard {
            visible: FrpManager.installed
            Layout.fillWidth: true
            logLines: root.logLines
            title: qsTr("Frp logs")
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
                    title: qsTr("Advanced Frp options")
                    description: qsTr("Reinstall the service or change its installation folder.")
                    iconName: "settings"
                    MdSwitch { id: advancedToggle }
                }
                FrpManagement {
                    visible: advancedToggle.checked && FrpManager.installed
                    Layout.fillWidth: true
                }
                InstallPathCard {
                    visible: advancedToggle.checked
                    Layout.fillWidth: true
                    manager: FrpManager
                    pathTitle: qsTr("Frp Install Directory")
                    setButtonText: qsTr("Set Frp Folder")
                    onSetClicked: frpFolderDialog.open()
                    onResetClicked: {
                        Settings.frpInstallPath = SystemInfo.defaultFrpInstallDir;
                        FrpManager.set_install_path(SystemInfo.defaultFrpInstallDir);
                    }
                }
            }
        }
    }

    MdDialog {
        id: tomlDialog
        parent: Overlay.overlay
        anchors.centerIn: Overlay.overlay
        width: Math.min(800, Overlay.overlay.width - 64)
        height: Math.min(600, Overlay.overlay.height - 64)
        modal: true
        title: qsTr("Edit frpc.toml")
        standardButtons: Dialog.Save | Dialog.Cancel
        palette.text: Theme.surfaceForeground
        background: Rectangle { color: Theme.surfaceContainerHigh; radius: Theme.radiusExtraLarge }
        onOpened: tomlEditor.text = FrpManager.read_toml()
        onAccepted: FrpManager.save_toml(tomlEditor.text)
        ScrollView {
            anchors.fill: parent
            TextArea {
                id: tomlEditor
                color: Theme.surfaceForeground
                selectionColor: Theme.primaryContainer
                selectedTextColor: Theme.primaryContainerForeground
                font.family: "Monospace"
                font.pixelSize: Theme.bodySize
                wrapMode: TextArea.NoWrap
                selectByMouse: true
                background: Rectangle { color: Theme.surfaceContainer; radius: Theme.radiusMedium }
            }
        }
    }
    FolderDialog {
        id: frpFolderDialog
        title: qsTr("Select Frp Folder")
        onAccepted: {
            Settings.set_frp_install_path_url(selectedFolder);
            FrpManager.set_install_path(Settings.frpInstallPath);
        }
    }
}
