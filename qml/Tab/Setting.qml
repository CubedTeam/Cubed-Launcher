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

    Loader {
        id: pathSetLoader

        anchors.bottom: advancedSetting.top
        anchors.horizontalCenter: advancedSetting.horizontalCenter
        anchors.bottomMargin: 50

        source: "qrc:/qt/qml/CubedLauncher/qml/Tool/GamePathSet.qml"
    }

    Switch {
        id: advancedSetting
        anchors.centerIn: parent
        font.pixelSize: 20
        text: qsTr("Advanced Setting")
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
            placeholderText: qsTr("Wrapper Command")
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
            text: qsTr("Log")
            onCheckedChanged: {
                CubedInstance.logOn = logStatus.checked;
            }
        }

        ComboBox {
            id: peerMode
            visible: advancedSetting.checked
            Layout.alignment: Qt.AlignCenter
            Layout.fillWidth: true
            font.pixelSize: 20
            // AI-generated: host/client labels translated for display.
            model: [qsTr("Host"), qsTr("Client")]
            currentIndex: 0
            onCurrentIndexChanged: {
                CubedInstance.set_peer(peerMode.currentIndex);
            }
        }

        TextField {
            id: hostPort
            visible: advancedSetting.checked && peerMode.currentIndex == 0
            Layout.fillWidth: true
            placeholderText: qsTr("Port")
            onEditingFinished: {
                CubedInstance.set_port(hostPort.text);
            }
        }

        RowLayout {
            visible: advancedSetting.checked && peerMode.currentIndex == 1
            Layout.fillWidth: true
            spacing: 10
            TextField {
                id: serverIp
                Layout.fillWidth: true
                placeholderText: qsTr("Ip")
                onEditingFinished: {
                    CubedInstance.set_ip(serverIp.text);
                }
            }
            TextField {
                id: serverPort
                Layout.fillWidth: true
                placeholderText: qsTr("Port")
                onEditingFinished: {
                    CubedInstance.set_port(serverPort.text);
                }
            }
        }
    }
}
