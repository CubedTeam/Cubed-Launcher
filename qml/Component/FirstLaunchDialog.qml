pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts
import QtQuick.Dialogs

Dialog {
    id: firstLaunchDialog
    title: qsTr("Welcome to Cubed Launcher")
    anchors.centerIn: Overlay.overlay
    width: 480
    modal: true
    closePolicy: Popup.NoAutoClose
    standardButtons: Dialog.NoButton

    Component.onCompleted: {
        if (!Settings.pathSetted) {
            open();
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 14

        Label {
            text: qsTr("Choose where to install Cubed game files. You can change this later in Settings.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Label {
            text: qsTr("Default location:")
            font.pixelSize: 14
            opacity: 0.7
        }

        Label {
            text: SystemInfo.defaultGameInstallDir
            wrapMode: Text.WrapAnywhere
            font.pixelSize: 13
            opacity: 0.7
            Layout.fillWidth: true
        }

        Button {
            text: qsTr("Use Default Folder")
            Layout.fillWidth: true
            Material.roundedScale: Material.MediumScale
            highlighted: true
            onClicked: {
                Settings.set_game_dir(SystemInfo.defaultGameInstallDir);
                firstLaunchDialog.close();
            }
        }

        Button {
            text: qsTr("Choose Install Folder")
            Layout.fillWidth: true
            Material.roundedScale: Material.MediumScale
            onClicked: folderDialog.open()
        }
    }

    FolderDialog {
        id: folderDialog
        title: qsTr("Select Game Install Folder")
        onAccepted: {
            Settings.set_game_dir_url(selectedFolder);
            firstLaunchDialog.close();
        }
    }
}
