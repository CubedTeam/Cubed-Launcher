import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import QtQuick.Layouts
import CubedLauncher

Card {
    id: creditsCard
    implicitHeight: creditsButton.implicitHeight + 20

    Button {
        id: creditsButton
        text: qsTr("Special Thanks")
        anchors.centerIn: parent
        width: 300
        font.pixelSize: 24
        onClicked: creditsDialog.open()
    }

    Dialog {
        id: creditsDialog
        parent: Overlay.overlay
        anchors.centerIn: Overlay.overlay
        width: 480
        height: 360
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        title: qsTr("Special Thanks")
        standardButtons: Dialog.Close

        ScrollView {
            anchors.fill: parent
            clip: true

            ColumnLayout {
                width: creditsDialog.availableWidth
                spacing: 12

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: "克公"
                    font.pixelSize: 20
                }
                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: "阿杰"
                    font.pixelSize: 20
                }
            }
        }
    }
}
