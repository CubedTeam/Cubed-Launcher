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
        id: aboutLayout
        width: 500
        spacing: 24
        anchors.centerIn: parent
        Rectangle {
            radius: 16
            color: "white"
            border.color: Material.color(Material.Grey, Material.Shade300)
            border.width: 1
            Layout.fillWidth: true
            Layout.preferredHeight: infoLayout.implicitHeight + 20

            ColumnLayout {
                id: infoLayout
                spacing: 24
                anchors.centerIn: parent
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
            }
        }
        Rectangle {
            radius: 16
            color: "white"
            border.color: Material.color(Material.Grey, Material.Shade300)
            border.width: 1
            Layout.fillWidth: true
            Layout.preferredHeight: checkUpdateButton.implicitHeight + 20
            Button {
                id: checkUpdateButton
                text: qsTr("Check Update")
                anchors.centerIn: parent
                width: 300

                font.pixelSize: 24
                onClicked: {
                    LauncherUpdate.check_update("CubedTeam", "Cubed-Launcher");
                }
            }
        }
    }
}
