import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property string text
    property string iconName: tone === "error" ? "warning" : "info"
    property string tone: "info"
    property alias actionText: actionButton.text
    signal actionClicked

    implicitHeight: Math.max(56, bannerRow.implicitHeight + 20)
    radius: Theme.radiusLarge
    color: tone === "error" ? Theme.errorContainer : tone === "warning" ? Theme.tertiaryContainer : Theme.secondaryContainer

    Behavior on color {
        ColorAnimation {
            duration: Theme.motionNormal
        }
    }

    RowLayout {
        id: bannerRow
        anchors.fill: parent
        anchors.margins: Theme.space12
        spacing: Theme.space12
        MdIcon {
            name: root.iconName
            color: root.tone === "error" ? Theme.errorContainerForeground : root.tone === "warning" ? Theme.tertiaryContainerForeground : Theme.secondaryContainerForeground
        }
        Label {
            text: root.text
            color: root.tone === "error" ? Theme.errorContainerForeground : root.tone === "warning" ? Theme.tertiaryContainerForeground : Theme.secondaryContainerForeground
            font.pixelSize: Theme.bodySize
            wrapMode: Text.WordWrap
            Layout.fillWidth: true

            Behavior on color {
                ColorAnimation {
                    duration: Theme.motionFast
                }
            }
        }
        MdButton {
            id: actionButton
            visible: text.length > 0
            variant: "text"
            onClicked: root.actionClicked()
        }
    }
}
