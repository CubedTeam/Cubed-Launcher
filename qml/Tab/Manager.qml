pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: mamagerTab
    Layout.fillHeight: true
    Layout.fillWidth: true
    Label {
        text: qsTr("Intalling Game only be available in Windows")
        visible: {
            return Qt.platform.os != "windows";
        }
        font.pixelSize: 24
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 100
    }
    Connections {
        id: checkConnections
        target: CubedInstance
        property bool checked: false
        function onVersion_changed() {
            if (checked) {
                return;
            }
            checked = true;
            console.log("version changed:", CubedInstance.version);
            GameUpdate.check_update(CubedInstance.version);
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        Component.onCompleted: {
            CubedInstance.check_version();
        }
        Loader {
            id: gameDownload
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            source: "qrc:/qt/qml/CubedLauncher/qml/Tool/DownloadGame.qml"
        }
    }
}
