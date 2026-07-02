pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: mamagerTab
    Layout.fillHeight: true
    Layout.fillWidth: true
    Label {
        text: "Intalling Game only be available in Windows"
        visible: {
            return Qt.platform.os != "windows";
        }
        font.pixelSize: 24
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 100
    }
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
