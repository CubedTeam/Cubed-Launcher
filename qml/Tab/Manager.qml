pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: mamagerTab
    Layout.fillHeight: true
    Layout.fillWidth: true

    Flickable {
        id: managerScroll
        anchors.fill: parent
        contentWidth: width
        contentHeight: managerContent.implicitHeight + 40
        clip: true
        boundsMovement: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {}

        ColumnLayout {
            id: managerContent
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 20
            width: 520
            spacing: 10

            Label {
                text: qsTr("Intalling Game only be available in Windows")
                visible: Qt.platform.os != "windows"
                font.pixelSize: 24
                Layout.alignment: Qt.AlignCenter
            }

            Loader {
                id: gameDownload
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                source: "qrc:/qt/qml/CubedLauncher/qml/Tool/DownloadGame.qml"
            }
        }
    }

    Connections {
        id: checkConnections
        target: CubedGame
        property bool checked: false
        function onVersion_changed() {
            if (checked) {
                return;
            }
            checked = true;
            console.log("version changed:", CubedGame.version);
            GameUpdate.check_update(CubedGame.version);
        }
    }
}
