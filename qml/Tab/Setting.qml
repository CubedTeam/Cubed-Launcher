// @checkPropertyInstance Settings C++
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

    // AI-generated: always-visible language picker near the page top.
    ColumnLayout {
        id: languageLayout

        anchors.bottom: advancedSetting.top
        anchors.horizontalCenter: advancedSetting.horizontalCenter
        anchors.bottomMargin: 50
        spacing: 10

        Label {
            text: qsTr("Language")
            font.pixelSize: 20
            Layout.alignment: Qt.AlignCenter
        }

        ComboBox {
            id: languageCombo
            // AI-generated: index 0 -> zh_CN, index 1 -> en. "English" is left
            // untranslated on purpose per the spec.
            model: ["简体中文", "English"]
            currentIndex: Settings.language === "en" ? 1 : 0
            font.pixelSize: 20
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 300
            onActivated: {
                Settings.language = currentIndex === 1 ? "en" : "zh_CN";
            }
        }

        // AI-generated: accent color picker. Each swatch stores its 1.2x
        // lighter MD color into Settings.accentColor; Main.qml binds the
        // global Material.accent to it.
        Label {
            text: qsTr("Theme Color")
            font.pixelSize: 20
            Layout.alignment: Qt.AlignCenter
        }

        RowLayout {
            Layout.alignment: Qt.AlignCenter
            spacing: 12

            Repeater {
                model: [Material.Red, Material.Pink, Material.Purple, Material.Indigo, Material.Blue, Material.Cyan, Material.Teal, Material.Green, Material.Orange, Material.DeepOrange]

                delegate: Rectangle {
                    required property int modelData
                    width: 32
                    height: 32
                    radius: 16
                    color: Qt.lighter(Material.color(modelData), 1.2)
                    border.width: Settings.accentColor === Qt.lighter(Material.color(modelData), 1.2) ? 3 : 0
                    border.color: Material.color(Material.Grey, Material.Shade700)

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Settings.accentColor = Qt.lighter(Material.color(modelData), 1.2)
                    }
                }
            }
        }
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
