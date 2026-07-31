// AI-generated: install path card shared by frp / easytier sections. The
// caller wires up the actual folder dialog and reset action via signals,
// so the card only owns the static layout.
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts

Card {
    id: root
    property var manager
    property string pathTitle
    property string setButtonText: qsTr("Set Folder")
    property string resetButtonText: qsTr("Reset Path")

    signal setClicked()
    signal resetClicked()

    implicitWidth: pathLayout.implicitWidth + 20
    implicitHeight: pathLayout.implicitHeight + 20

    ColumnLayout {
        id: pathLayout
        anchors.fill: parent
        anchors.margins: 10
        spacing: 12

        Label {
            text: root.pathTitle + ": " + root.manager.installPath
            font.pixelSize: 16
            Layout.alignment: Qt.AlignCenter
            wrapMode: Text.WrapAnywhere
            Layout.preferredWidth: 500
            horizontalAlignment: Text.AlignHCenter
        }
        RowLayout {
            Layout.alignment: Qt.AlignCenter
            spacing: 10
            Button {
                Material.roundedScale: Material.MediumScale
                Layout.preferredWidth: 250
                Layout.preferredHeight: 50
                font.pixelSize: 20
                highlighted: true
                text: root.setButtonText
                onClicked: root.setClicked()
            }
            Button {
                Material.roundedScale: Material.MediumScale
                Layout.preferredWidth: 250
                Layout.preferredHeight: 50
                font.pixelSize: 20
                text: root.resetButtonText
                onClicked: root.resetClicked()
            }
        }
    }
}
