pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts

Item {
    id: settingTab
    Layout.fillHeight: true
    Layout.fillWidth: true
    Switch {
        id: advancedSetting
        anchors.centerIn: parent
        Layout.alignment: Qt.AlignCenter
        font.pixelSize: 20
        text: "Advanced Setting"
        checked: false
    }
    ColumnLayout {

        anchors.horizontalCenter: advancedSetting.horizontalCenter
        anchors.top: advancedSetting.bottom
        anchors.topMargin: 10

        width: 300
        spacing: 10

        TextField {
            id: wrapperCommand
            visible: advancedSetting.checked
            Layout.fillWidth: true
            placeholderText: "Wrapper Command"
            onEditingFinished: {
                CubedInstance.set_wrapper_command(wrapperCommand.text);
            }
        }
        Switch {
            id: logStatus
            visible: advancedSetting.checked
            Layout.alignment: Qt.AlignCenter
            checked: false
            font.pixelSize: 20
            text: "Log"
            onCheckedChanged: {
                CubedInstance.logOn = logStatus.checked;
            }
        }
    }
}
