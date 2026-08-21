pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import CubedLauncher

PageScaffold {
    id: managerTab
    title: qsTr("Game Manager")
    subtitle: qsTr("Install Cubed, choose a download source, and manage the game folder.")

    DownloadGame {
        Layout.fillWidth: true
    }

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
