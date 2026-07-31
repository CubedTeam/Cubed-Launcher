// AI-generated: frp-based multiplayer tab. State-driven UI: install card is
// the only one visible until frpc is installed, then start/stop and logs.
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    id: multiplayerTab
    Layout.fillHeight: true
    Layout.fillWidth: true

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

    Flickable {
        anchors.fill: parent
        contentWidth: width
        contentHeight: mainColumn.implicitHeight + 40
        clip: true
        boundsMovement: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {}

        ColumnLayout {
            id: mainColumn
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 20
            width: 560
            spacing: 12

            Card {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                Layout.preferredHeight: installLayout.implicitHeight + 20
                visible: !FrpManager.installed || FrpManager.busy
                Loader {
                    id: installLayout
                    width: parent.width
                    anchors.centerIn: parent
                    source: "qrc:/qt/qml/CubedLauncher/qml/Tool/FrpManagement.qml"
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
            Card {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                Layout.preferredHeight: logLayout.implicitHeight + 20
                visible: FrpManager.installed
                ColumnLayout {
                    id: logLayout
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        Label {
                            text: qsTr("Logs")
                            font.pixelSize: 18
                            font.bold: true
                            Layout.alignment: Qt.AlignLeft
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Switch {
                            id: logToggle
                            font.pixelSize: 14
                            text: qsTr("ShowLog")
                            checked: multiplayerTab.showLog
                            onCheckedChanged: multiplayerTab.showLog = checked
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 300
                        color: "#101418"
                        visible: multiplayerTab.showLog
                        radius: 8
                        border.color: Qt.rgba(0, 0, 0, 0.12)
                        border.width: 1
                        clip: true

                        ScrollView {
                            id: logScroll
                            anchors.fill: parent
                            anchors.margins: 8
                            clip: true

                            TextArea {
                                id: logArea
                                readOnly: true
                                wrapMode: TextArea.NoWrap
                                color: "#9CDCFE"
                                background: null
                                font.family: multiplayerTab.showLog ? "Monospace" : "Sans"
                                font.pixelSize: multiplayerTab.showLog ? 13 : 14
                                selectByMouse: true
                                text: multiplayerTab.logLines.join("\n")
                                onTextChanged: cursorPosition = length
                            }
                        }
                    }
                }
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
                Loader {
                    id: advancedSetting
                    width: parent.width
                    anchors.centerIn: parent
                    source: "qrc:/qt/qml/CubedLauncher/qml/Tool/FrpManagement.qml"
                }
            }
            Card {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                Layout.preferredHeight: frpPathLayout.implicitHeight + 20
                visible: advancedButton.checked
                ColumnLayout {
                    id: frpPathLayout
                    anchors.centerIn: parent
                    spacing: mainColumn.spacing
                    Label {
                        text: qsTr("Frp Install Directory: ") + FrpManager.installPath
                        font.pixelSize: 16
                        Layout.alignment: Qt.AlignCenter
                        wrapMode: Text.WrapAnywhere
                        Layout.preferredWidth: 500
                        horizontalAlignment: Text.AlignHCenter
                    }
                    RowLayout {
                        Layout.alignment: Qt.AlignCenter
                        spacing: 10
                        Button {
                            Material.roundedScale: Material.MediumScale
                            Layout.preferredWidth: 250
                            Layout.preferredHeight: 50
                            font.pixelSize: 20
                            highlighted: true
                            text: qsTr("Set Frp Folder")
                            onClicked: frpFolderDialog.open()
                        }
                        Button {
                            Material.roundedScale: Material.MediumScale
                            Layout.preferredWidth: 250
                            Layout.preferredHeight: 50
                            font.pixelSize: 20
                            text: qsTr("Reset Path")
                            onClicked: {
                                Settings.frpInstallPath = SystemInfo.defaultFrpInstallDir;
                                FrpManager.set_install_path(SystemInfo.defaultFrpInstallDir);
                            }
                        }
                    }
                }
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

    Connections {
        target: FrpManager
        function onLog_line(line) {
            const lines = multiplayerTab.logLines.slice();
            lines.push(line);
            // Keep the buffer bounded to avoid unbounded growth.
            if (lines.length > 2000) {
                lines.splice(0, lines.length - 2000);
            }
            multiplayerTab.logLines = lines;
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
