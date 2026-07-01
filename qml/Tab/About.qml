import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import QtQuick.Layouts
import CubedLauncher

Item {
    id: aboutTab
    Layout.fillHeight: true
    Layout.fillWidth: true
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
    }
}
