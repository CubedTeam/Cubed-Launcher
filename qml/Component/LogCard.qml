import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Card {
    id: root
    property var logLines: []
    property bool showLog: false
    property string title: qsTr("Logs")
    implicitHeight: logColumn.implicitHeight + Theme.space32 * 2

    ColumnLayout {
        id: logColumn
        anchors.fill: parent
        anchors.margins: Theme.space24
        spacing: Theme.space12
        SettingRow {
            title: root.title
            description: qsTr("Show the latest service output.")
            iconName: "terminal"
            MdSwitch {
                checked: root.showLog
                onToggled: root.showLog = checked
            }
        }
        Rectangle {
            visible: root.showLog
            Layout.fillWidth: true
            Layout.preferredHeight: 260
            color: "#101418"
            radius: Theme.radiusMedium
            clip: true
            ScrollView {
                anchors.fill: parent
                anchors.margins: Theme.space8
                clip: true
                TextArea {
                    id: logArea
                    readOnly: true
                    wrapMode: TextArea.NoWrap
                    color: "#B8E8FF"
                    background: null
                    font.family: "Monospace"
                    font.pixelSize: 13
                    selectByMouse: true
                    text: root.logLines.join("\n")
                    onTextChanged: cursorPosition = length
                }
            }
        }
    }
}
