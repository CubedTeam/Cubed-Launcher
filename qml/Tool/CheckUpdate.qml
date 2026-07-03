pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts

Item {
    anchors.fill: parent
    ColumnLayout {
        anchors.centerIn: parent
        Button {
            id: updateButton
            Layout.preferredWidth: 250
            Layout.preferredHeight: 60
            highlighted: true
            Material.roundedScale: Material.MediumScale
            contentItem: ColumnLayout {
                spacing: 5
                anchors.top: parent.top
                anchors.topMargin: 5
                anchors.horizontalCenter: parent.horizontalCenter

                Text {
                    Layout.alignment: Qt.AlignCenter
                    text: "Launcher New Version"
                    font.bold: true
                    font.pixelSize: 16
                    color: "white"
                }
                Text {
                    Layout.alignment: Qt.AlignCenter

                    text: VersionUpdate.localVersion + " -> " + VersionUpdate.remoteVersion
                    color: "white"
                    font.bold: true
                    font.pixelSize: 16
                }
            }
            onClicked: {
                Qt.openUrlExternally("https://github.com/CubedTeam/Cubed-Launcher/releases");
            }
        }

        Button {
            id: updateLauncherButton
            Layout.preferredWidth: 250
            Layout.preferredHeight: 60
            Material.roundedScale: Material.MediumScale
            highlighted: true
            text: "Click me to Update"
            font.bold: true
            font.pixelSize: 16
            onClicked: {
                updatePopup.open();
            }
        }
    }

    Popup {
        id: updatePopup
        anchors.centerIn: Overlay.overlay
        width: 300
        height: 300
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 10

            Label {
                text: "Update Launcher"
                Layout.alignment: Qt.AlignCenter

                font.pixelSize: 20
            }

            Label {
                text: VersionUpdate.localVersion + " -> " + VersionUpdate.remoteVersion
                Layout.alignment: Qt.AlignCenter

                font.pixelSize: 20
            }

            Switch {
                id: mirrorSwitch
                text: "Use Mirror"
                checked: SystemInfo.isInChina
                Layout.alignment: Qt.AlignCenter
                font.pixelSize: 20
            }

            Button {
                id: downloadUpdateButton
                text: "Update"
                Layout.alignment: Qt.AlignCenter
                font.pixelSize: 20
                highlighted: true
                onClicked: {
                    VersionUpdate.update_launcher(mirrorSwitch.checked);
                }
            }
        }
    }
}
