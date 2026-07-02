pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts

Button {
    id: updateButton
    width: 160
    height: 60
    highlighted: VersionUpdate.hasNewVersion
    enabled: VersionUpdate.hasNewVersion

    contentItem: ColumnLayout {
        spacing: 2
        anchors.centerIn: parent
        Material.roundedScale: Material.FullScale
        Text {
            Layout.alignment: Qt.AlignCenter
            text: "New Version"
            font.bold: true
            font.pixelSize: 16
            color: updateButton.Material.foreground
        }
        Text {
            Layout.alignment: Qt.AlignCenter
            text: VersionUpdate.localVersion + " -> " + VersionUpdate.remoteVersion
            color: updateButton.Material.foreground
            font.pixelSize: 12
        }
    }
    onClicked: {
        Qt.openUrlExternally("https://github.com/CubedTeam/Cubed-Launcher/releases");
    }
}
