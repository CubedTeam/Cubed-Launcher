import QtQuick
import QtQuick.Controls

Button {
    id: root
    property string iconName
    property string toolTip: ""
    property string variant: "standard"
    implicitWidth: 40
    implicitHeight: 40
    hoverEnabled: true

    contentItem: MdIcon {
        anchors.centerIn: parent
        name: root.iconName
        iconSize: 22
        color: root.variant === "filled" ? Theme.primaryForeground
             : root.variant === "danger" ? Theme.error : Theme.surfaceVariantForeground
    }
    background: Rectangle {
        radius: width / 2
        color: root.variant === "filled" ? Theme.primary
             : root.hovered || root.down ? Theme.surfaceContainerHigh : "transparent"
        border.width: root.activeFocus ? 1 : 0
        border.color: Theme.primary
    }
    ToolTip.visible: hovered && toolTip.length > 0
    ToolTip.text: toolTip
    ToolTip.delay: 450
}
