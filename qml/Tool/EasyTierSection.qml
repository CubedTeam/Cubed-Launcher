// AI-generated: easytier multiplayer section. Self-contained: install card
// and install path setting + folder dialog. Startup UI is not implemented
// yet, so no control/log cards here.
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

    ColumnLayout {
        id: easytierLayout
        width: 560

        spacing: 12

        Card {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: easytierInstall.implicitHeight + 20
            Loader {
                id: easytierInstall
                width: parent.width
                anchors.centerIn: parent
                source: "qrc:/qt/qml/CubedLauncher/qml/Tool/EasyTierManagement.qml"
            }
        }

        Card {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: easytierPath.implicitHeight + 20
            ColumnLayout {
                id: easytierPath
                anchors.centerIn: parent
                spacing: 12
                Label {
                    text: qsTr("EasyTier Install Directory: ") + EasyTierManager.installPath
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
                        text: qsTr("Set EasyTier Folder")
                        onClicked: easytierFolderDialog.open()
                    }
                    Button {
                        Material.roundedScale: Material.MediumScale
                        Layout.preferredWidth: 250
                        Layout.preferredHeight: 50
                        font.pixelSize: 20
                        text: qsTr("Reset Path")
                        onClicked: {
                            Settings.easytierInstallPath = SystemInfo.defaultEasyTierInstallDir;
                            EasyTierManager.set_install_path(SystemInfo.defaultEasyTierInstallDir);
                        }
                    }
                }
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
