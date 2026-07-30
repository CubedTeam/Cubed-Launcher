pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    anchors.fill: parent
    ColumnLayout {
        anchors.centerIn: parent
        Label {
            Layout.alignment: Qt.AlignCenter
            text: gameFolderDialog.selectedFolder
        }
        Label {
            id: selectMessgae
            Layout.alignment: Qt.AlignCenter
            Material.foreground: Material.Red
            visible: !CubedInstance.path_selected
            text: qsTr("You must select or intstall game program")
        }
        Button {
            id: gamePathButton

            Material.roundedScale: Material.MediumScale
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 250
            Layout.preferredHeight: 60

            font.pixelSize: 20

            highlighted: true
            text: qsTr("Select Game")
            onClicked: {
                gameFolderDialog.open();
            }
        }
    }
    FolderDialog {
        id: gameFolderDialog
        title: qsTr("Select Cubed Game")
        onAccepted: {
            CubedInstance.set_game_dir_url(selectedFolder);
            Settings.set_game_dir_url(selectedFolder);
            GameUpdate.gameInstallPath = Settings.gameDir;
        }
    }
}
