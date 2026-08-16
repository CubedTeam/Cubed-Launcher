import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property string title
    property string subtitle: ""
    property int contentMaximumWidth: 1040
    default property alias content: contentColumn.data

    Flickable {
        anchors.fill: parent
        contentWidth: width
        contentHeight: contentColumn.implicitHeight + Theme.space32 * 2
        clip: true
        boundsMovement: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar {}

        ColumnLayout {
            id: contentColumn
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: Theme.space32
            width: Math.min(root.contentMaximumWidth, parent.width - Theme.space32 * 2)
            spacing: Theme.space24

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Theme.space4
                Label {
                    text: root.title
                    color: Theme.surfaceForeground
                    font.pixelSize: Theme.headlineSize
                    font.weight: Font.DemiBold
                    Layout.fillWidth: true
                }
                Label {
                    visible: root.subtitle.length > 0
                    text: root.subtitle
                    color: Theme.surfaceVariantForeground
                    font.pixelSize: Theme.bodySize
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }
    }
}
