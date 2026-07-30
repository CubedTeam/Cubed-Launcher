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

    Flickable {
        id: settingScroll
        anchors.fill: parent
        contentWidth: width
        contentHeight: settingLayout.implicitHeight + 40
        clip: true
        boundsMovement: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {}

        ColumnLayout {
            id: settingLayout
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 20
            width: 500
            spacing: 10

            Card {
                Layout.preferredHeight: languageLayout.implicitHeight + 20
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                ColumnLayout {
                    id: languageLayout
                    anchors.centerIn: parent
                    spacing: settingLayout.spacing
                    Label {
                        text: qsTr("Language")
                        font.pixelSize: 20
                        Layout.alignment: Qt.AlignCenter
                    }
                    ComboBox {
                        id: languageCombo
                        model: ["简体中文", "English"]
                        currentIndex: Settings.language === "en" ? 1 : 0
                        font.pixelSize: 20
                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: 300
                        onActivated: {
                            Settings.language = currentIndex === 1 ? "en" : "zh_CN";
                        }
                    }
                }
            }

            Card {
                Layout.preferredHeight: colorLayout.implicitHeight + 20
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                ColumnLayout {
                    id: colorLayout
                    anchors.centerIn: parent
                    spacing: settingLayout.spacing
                    Label {
                        text: qsTr("Theme Color")
                        font.pixelSize: 20
                        Layout.alignment: Qt.AlignCenter
                    }
                    RowLayout {

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
            }
            Card {
                Layout.preferredHeight: colorfulBorder.implicitHeight + 20
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                Switch {
                    id: colorfulBorder
                    anchors.centerIn: parent
                    font.pixelSize: 20
                    text: qsTr("Colorful Card Border")
                    checked: Settings.cardColorfulBorder
                    onCheckedChanged: Settings.cardColorfulBorder = checked
                }
            }

            Card {
                Layout.preferredHeight: advancedSetting.implicitHeight + 20
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                Switch {
                    id: advancedSetting
                    anchors.centerIn: parent
                    font.pixelSize: 20
                    text: qsTr("Advanced Setting")
                    checked: false
                }
            }

            Card {
                Layout.preferredHeight: wrapperLayout.implicitHeight + 20
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                visible: advancedSetting.checked
                ColumnLayout {
                    id: wrapperLayout
                    anchors.centerIn: parent
                    spacing: settingLayout.spacing
                    Label {
                        text: qsTr("Wrapper Command")
                        font.pixelSize: 20
                        Layout.alignment: Qt.AlignCenter
                    }
                    TextField {
                        id: wrapperCommand
                        Layout.preferredWidth: 300
                        Layout.alignment: Qt.AlignCenter
                        placeholderText: qsTr("Wrapper Command")
                        onEditingFinished: {
                            CubedInstance.set_wrapper_command(wrapperCommand.text);
                        }
                    }
                }
            }

            Card {
                Layout.preferredHeight: logLayout.implicitHeight + 20
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                visible: advancedSetting.checked
                ColumnLayout {
                    id: logLayout
                    anchors.centerIn: parent
                    spacing: settingLayout.spacing
                    Label {
                        text: qsTr("Log")
                        font.pixelSize: 20
                        Layout.alignment: Qt.AlignCenter
                    }
                    Switch {
                        id: logStatus
                        checked: false
                        font.pixelSize: 20
                        Layout.alignment: Qt.AlignCenter
                        onCheckedChanged: {
                            CubedInstance.logOn = logStatus.checked;
                        }
                        text: checked ? qsTr("On") : qsTr("Off")
                    }
                }
            }

            Card {
                Layout.preferredHeight: networkLayout.implicitHeight + 20
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                visible: advancedSetting.checked
                ColumnLayout {
                    id: networkLayout
                    anchors.centerIn: parent
                    spacing: settingLayout.spacing
                    Label {
                        text: qsTr("Network")
                        font.pixelSize: 20
                        Layout.alignment: Qt.AlignCenter
                    }
                    ComboBox {
                        id: peerMode
                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: 300
                        font.pixelSize: 20
                        model: [qsTr("Host"), qsTr("Client")]
                        currentIndex: 0
                        onCurrentIndexChanged: {
                            CubedInstance.set_peer(peerMode.currentIndex);
                        }
                    }
                    TextField {
                        id: hostPort
                        visible: peerMode.currentIndex == 0
                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: 300
                        placeholderText: qsTr("Port")
                        onEditingFinished: {
                            CubedInstance.set_port(hostPort.text);
                        }
                    }
                    RowLayout {
                        visible: peerMode.currentIndex == 1
                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: 300
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
        }
    }
}
