pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import CubedLauncher

PageScaffold {
    id: managerTab
    title: qsTr("Game Manager")
    subtitle: qsTr("Install Cubed, choose a download source, and manage the game folder.")

    InfoBanner {
        visible: Qt.platform.os !== "windows"
        Layout.fillWidth: true
        tone: "warning"
        text: qsTr("Automatic game installation is currently available on Windows only.")
    }

    DownloadGame { Layout.fillWidth: true }

    Connections {
        target: CubedGame
        property bool checked: false
        function onVersion_changed() {
            if (checked)
                return;
            checked = true;
            GameUpdate.check_update(CubedGame.version);
        }
    }
}
