import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CubedLauncher

MdButton {
    id: root
    text: qsTr("Credits")
    iconName: "info"
    variant: "tonal"
    onClicked: creditsDialog.open()

    MdDialog {
        id: creditsDialog
        parent: Overlay.overlay
        anchors.centerIn: Overlay.overlay
        width: Math.min(460, Overlay.overlay.width - 48)
        modal: true
        title: qsTr("Special Thanks")
        standardButtons: Dialog.Close
        palette.text: Theme.surfaceForeground
        background: Rectangle {
            color: Theme.surfaceContainerHigh
            radius: Theme.radiusExtraLarge
        }

        ColumnLayout {
            width: parent.width
            spacing: Theme.space16
            MdIcon {
                name: "badge"
                color: Theme.primary
                iconSize: 40
                Layout.alignment: Qt.AlignHCenter
            }
            Label {
                text: qsTr("Thanks to everyone who helped Cubed Launcher grow.")
                color: Theme.surfaceVariantForeground
                font.pixelSize: Theme.bodySize
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            StatusChip {
                text: "克公"
                iconName: "badge"
                Layout.alignment: Qt.AlignHCenter
            }
            StatusChip {
                text: "阿杰"
                iconName: "badge"
                Layout.alignment: Qt.AlignHCenter
            }
            StatusChip {
                text: "游云"
                iconName: "badge"
                Layout.alignment: Qt.AlignHCenter
            }
        }
    }
}
