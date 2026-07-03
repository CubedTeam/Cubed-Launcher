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
        active: VersionUpdate.hasNewVersion
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
            text: "Application: " + Qt.application.name
            font.pixelSize: 24
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: "Version: " + AppVersion
            font.pixelSize: 24
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: "OS: " + Qt.platform.os
            font.pixelSize: 24
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: "ProductType: " + SystemInfo.productType
            font.pixelSize: 24
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: "ProductVersion: " + SystemInfo.productVersion
            font.pixelSize: 24
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: "KernelType: " + SystemInfo.kernelType
            font.pixelSize: 24
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: "KernelVersion: " + SystemInfo.kernelVersion
            font.pixelSize: 24
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: "QtVersion: " + SystemInfo.qtVersion
            font.pixelSize: 24
        }

        Button {
            text: "Check Update"
            Layout.alignment: Qt.AlignCenter
            font.pixelSize: 24
            onClicked: {
                VersionUpdate.check_update("CubedTeam", "Cubed-Launcher");
            }
        }
    }
}
