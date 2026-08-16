import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property string text
    property string iconName: "info"
    property string tone: "neutral"
    implicitWidth: chipRow.implicitWidth + 20
    implicitHeight: 32
    radius: height / 2
    color: tone === "success" ? Theme.primaryContainer
         : tone === "error" ? Theme.errorContainer
         : tone === "warning" ? Theme.tertiaryContainer : Theme.secondaryContainer

    RowLayout {
        id: chipRow
        anchors.centerIn: parent
        spacing: Theme.space4
        MdIcon {
            name: root.iconName
            iconSize: 16
            color: root.tone === "success" ? Theme.onPrimaryContainer
                 : root.tone === "error" ? Theme.onErrorContainer
                 : root.tone === "warning" ? Theme.onTertiaryContainer : Theme.onSecondaryContainer
        }
        Label {
            text: root.text
            font.pixelSize: Theme.labelSize
            font.weight: Font.DemiBold
            color: root.tone === "success" ? Theme.onPrimaryContainer
                 : root.tone === "error" ? Theme.onErrorContainer
                 : root.tone === "warning" ? Theme.onTertiaryContainer : Theme.onSecondaryContainer
        }
    }
}
