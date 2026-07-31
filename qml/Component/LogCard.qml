// AI-generated: log viewer card. Used by the multiplayer service sections
// (frp / easytier) and bound to a JS array of log lines.
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts

Card {
    id: root
    property var logLines: []
    property bool showLog: true
    property string title: qsTr("Logs")

    implicitWidth: logLayout.implicitWidth + 20
    implicitHeight: logLayout.implicitHeight + 20

    ColumnLayout {
        id: logLayout
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: root.title
                font.pixelSize: 18
                font.bold: true
                Layout.alignment: Qt.AlignLeft
            }
            Item {
                Layout.fillWidth: true
            }
            Switch {
                id: logToggle
                font.pixelSize: 14
                text: qsTr("ShowLog")
                checked: root.showLog
                onCheckedChanged: root.showLog = checked
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 300
            color: "#101418"
            visible: root.showLog
            radius: 8
            border.color: Qt.rgba(0, 0, 0, 0.12)
            border.width: 1
            clip: true

            ScrollView {
                anchors.fill: parent
                anchors.margins: 8
                clip: true

                TextArea {
                    id: logArea
                    readOnly: true
                    wrapMode: TextArea.NoWrap
                    color: "#9CDCFE"
                    background: null
                    font.family: root.showLog ? "Monospace" : "Sans"
                    font.pixelSize: root.showLog ? 13 : 14
                    selectByMouse: true
                    text: root.logLines.join("\n")
                    onTextChanged: cursorPosition = length
                }
            }
        }
    }
}
