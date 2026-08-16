import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property string title
    property string description: ""
    property string iconName: "settings"
    default property alias trailing: trailingSlot.data
    implicitHeight: Math.max(64, row.implicitHeight + 20)
    Layout.fillWidth: true

    RowLayout {
        id: row
        anchors.fill: parent
        anchors.leftMargin: Theme.space16
        anchors.rightMargin: Theme.space16
        spacing: Theme.space16
        MdIcon { name: root.iconName; color: Theme.surfaceVariantForeground }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2
            Label {
                text: root.title
                color: Theme.surfaceForeground
                font.pixelSize: Theme.bodyLargeSize
                Layout.fillWidth: true
            }
            Label {
                visible: root.description.length > 0
                text: root.description
                color: Theme.surfaceVariantForeground
                font.pixelSize: Theme.labelSize
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }
        RowLayout {
            id: trailingSlot
            spacing: Theme.space8
        }
    }
}
