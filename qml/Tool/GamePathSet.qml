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
            text: gameFileDialog.selectedFile
        }
        Label {
            id: selectMessgae
            Layout.alignment: Qt.AlignCenter
            Material.foreground: Material.Red
            visible: !CubedInstance.path_selected
            text: "You must select game program"
        }
        Button {
            id: gamePathButton

            Material.roundedScale: Material.MediumScale
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 250
            Layout.preferredHeight: 60

            font.pixelSize: 20

            highlighted: true
            text: "Select Game"
            onClicked: {
                gameFileDialog.open();
            }
        }
    }
    FileDialog {
        id: gameFileDialog
        title: "Select Cubed Game"
        onAccepted: {
            CubedInstance.set_game_path_url(selectedFile);
            Settings.gamePath = selectedFile;
        }
    }
}
