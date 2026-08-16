import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property var manager
    property string pathTitle
    property string setButtonText: qsTr("Set Folder")
    property string resetButtonText: qsTr("Reset Path")
    signal setClicked()
    signal resetClicked()
    implicitHeight: pathRow.implicitHeight + Theme.space16 * 2
    radius: Theme.radiusMedium
    color: Theme.surfaceContainer

    RowLayout {
        id: pathRow
        anchors.fill: parent
        anchors.margins: Theme.space16
        spacing: Theme.space12
        MdIcon { name: "folder"; color: Theme.surfaceVariantForeground }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2
            Label { text: root.pathTitle; color: Theme.surfaceForeground; font.pixelSize: Theme.bodySize; font.weight: Font.DemiBold }
            Label {
                Layout.fillWidth: true
                text: root.manager.installPath
                color: Theme.surfaceVariantForeground
                font.pixelSize: Theme.labelSize
                wrapMode: Text.WrapAnywhere
            }
        }
        MdButton { text: root.setButtonText; iconName: "folder"; variant: "outlined"; onClicked: root.setClicked() }
        MdIconButton { iconName: "refresh"; toolTip: root.resetButtonText; onClicked: root.resetClicked() }
    }
}
