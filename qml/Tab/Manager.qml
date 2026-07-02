pragma ComponentBehavior: Bound
import QtQuick

import QtQuick.Layouts

Item {
    id: mamagerTab
    Layout.fillHeight: true
    Layout.fillWidth: true

    ColumnLayout {
        anchors.centerIn: parent

        Loader {
            id: gameDownload
            Layout.fillWidth: true
            height: 100
            Layout.alignment: Qt.AlignCenter
            source: "qrc:/qt/qml/CubedLauncher/qml/Tool/DownloadGame.qml"
        }
    }
}
