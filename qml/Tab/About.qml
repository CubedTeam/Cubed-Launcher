import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import QtQuick.Layouts
import CubedLauncher

Item {
    id: aboutTab
    Layout.fillHeight: true
    Layout.fillWidth: true
    Loader {
        active: LauncherUpdate.hasNewVersion || LauncherUpdate.hasError
        width: 300
        height: 200
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.rightMargin: 20
        source: "qrc:/qt/qml/CubedLauncher/qml/Tool/CheckUpdate.qml"
    }
    ColumnLayout {
        anchors.centerIn: parent
        width: 300
        spacing: 24
        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("Application: %L1").arg(Qt.application.name)
            font.pixelSize: 24
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("Version: %L1").arg(AppVersion)
            font.pixelSize: 24
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("OS: %L1").arg(Qt.platform.os)
            font.pixelSize: 24
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("SystemType: ") + SystemInfo.productType
            font.pixelSize: 24
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("SystemVersion: ") + SystemInfo.productVersion
            font.pixelSize: 24
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("KernelType: ") + SystemInfo.kernelType
            font.pixelSize: 24
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("KernelVersion: ") + SystemInfo.kernelVersion
            font.pixelSize: 24
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("QtVersion: ") + SystemInfo.qtVersion
            font.pixelSize: 24
        }

        Button {
            text: qsTr("Check Update")
            Layout.alignment: Qt.AlignCenter
            font.pixelSize: 24
            onClicked: {
                LauncherUpdate.check_update("CubedTeam", "Cubed-Launcher");
            }
        }
    }
}
