import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
    id: root
    property string title
    property string subtitle: ""
    property string iconName: "settings"
    spacing: Theme.space12
    MdIcon { name: root.iconName; iconSize: 28; color: Theme.primary }
    ColumnLayout {
        Layout.fillWidth: true
        spacing: 2
        Label {
            text: root.title
            color: Theme.surfaceForeground
            font.pixelSize: Theme.titleSize
            font.weight: Font.DemiBold
        }
        Label {
            visible: root.subtitle.length > 0
            text: root.subtitle
            color: Theme.surfaceVariantForeground
            font.pixelSize: Theme.labelSize
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
}
