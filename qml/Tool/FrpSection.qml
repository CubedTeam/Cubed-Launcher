// AI-generated: frp multiplayer section. Self-contained: install card,
// control card, logs, advanced toggle, path setting, toml dialog and
// folder dialog. Sourced from Multiplayer.qml to keep the multiplayer tab
// file itself thin.
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    id: frpSection
    implicitWidth: frpLayout.implicitWidth
    implicitHeight: frpLayout.implicitHeight
    width: frpLayout.width
    height: frpLayout.implicitHeight

    property var logLines: []
    property bool showLog: true

    Component.onCompleted: {
        if (Settings.frpInstallPath && Settings.frpInstallPath.length > 0) {
            FrpManager.set_install_path(Settings.frpInstallPath);
        }
    }

    Connections {
        target: Settings
        function onFrp_install_path_changed() {
            FrpManager.set_install_path(Settings.frpInstallPath);
        }
    }

    Connections {
        target: FrpManager
        function onLog_line(line) {
            const lines = frpSection.logLines.slice();
            lines.push(line);
            if (lines.length > 2000) {
                lines.splice(0, lines.length - 2000);
            }
            frpSection.logLines = lines;
        }
    }

    ColumnLayout {
        id: frpLayout
        width: 560
        spacing: 12

        Card {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: installLayout.implicitHeight + 20
            visible: !FrpManager.installed || FrpManager.busy
            FrpManagement {
                id: installLayout
                width: parent.width
                anchors.centerIn: parent
            }
        }
        // frp control
        Card {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: controlLayout.implicitHeight + 20
            visible: FrpManager.installed
            ColumnLayout {
                id: controlLayout
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Label {
                    Layout.alignment: Qt.AlignCenter
                    text: qsTr("Frp Client: %1").arg(FrpManager.version)
                    font.pixelSize: 18
                    font.bold: true
                }

                Label {
                    Layout.alignment: Qt.AlignCenter
                    text: qsTr("Status: %1").arg(FrpManager.running ? qsTr("Running") : qsTr("Stopped"))
                    font.pixelSize: 16
                    color: FrpManager.running ? Material.color(Material.Green) : Material.color(Material.Grey)
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
                        enabled: FrpManager.installed && !FrpManager.running
                        text: qsTr("Start")
                        onClicked: FrpManager.start()
                    }
                    Button {
                        id: stopButton
                        Layout.preferredWidth: 160
                        Layout.preferredHeight: 50
                        font.pixelSize: 18
                        Material.roundedScale: Material.MediumScale
                        Material.background: Material.color(Material.Red)
                        enabled: FrpManager.running
                        text: qsTr("Stop")
                        onClicked: FrpManager.stop()
                    }
                }

                Button {
                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: 250
                    Layout.preferredHeight: 50
                    font.pixelSize: 16
                    Material.roundedScale: Material.MediumScale
                    text: qsTr("Edit frpc.toml")
                    onClicked: tomlDialog.open()
                }
            }
        }
        // log card
        LogCard {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: implicitHeight
            visible: FrpManager.installed
            logLines: frpSection.logLines
            showLog: frpSection.showLog
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
            visible: advancedButton.checked && FrpManager.installed
            FrpManagement {
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

    Dialog {
        id: tomlDialog
        anchors.centerIn: Overlay.overlay
        width: Math.min(parent.width - 60, 800)
        height: Math.min(parent.height - 60, 600)
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        title: qsTr("Edit frpc.toml")
        standardButtons: Dialog.Save | Dialog.Cancel

        onOpened: tomlEditor.text = FrpManager.read_toml()

        onAccepted: FrpManager.save_toml(tomlEditor.text)

        ScrollView {
            anchors.fill: parent
            TextArea {
                id: tomlEditor
                font.family: "Monospace"
                font.pixelSize: 14
                wrapMode: TextArea.NoWrap
                selectByMouse: true
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
