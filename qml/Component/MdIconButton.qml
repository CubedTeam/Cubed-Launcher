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
    scale: down ? 0.98 : 1

    Behavior on scale {
        NumberAnimation { duration: Theme.motionFast; easing.type: Theme.motionEasing }
    }

    contentItem: MdIcon {
        anchors.centerIn: parent
        name: root.iconName
        iconSize: 22
        color: root.variant === "filled" ? Theme.primaryForeground
             : root.variant === "danger" ? Theme.error : Theme.surfaceVariantForeground
    }
    background: Rectangle {
        property color outlineColor: Theme.primary
        radius: width / 2
        color: root.variant === "filled" ? Theme.primary
             : root.hovered || root.down ? Theme.surfaceContainerHigh : "transparent"
        border.width: root.activeFocus ? 1 : 0
        border.color: outlineColor

        Behavior on color { ColorAnimation { duration: Theme.motionFast } }
        Behavior on outlineColor { ColorAnimation { duration: Theme.motionFast } }
    }
    MdToolTip {
        target: root
        visible: root.hovered && root.toolTip.length > 0
        text: root.toolTip
    }
}
