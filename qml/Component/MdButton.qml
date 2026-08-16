import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Button {
    id: root
    property string iconName: ""
    property string variant: "filled"

    implicitHeight: Theme.controlHeight
    implicitWidth: Math.max(96, contentRow.implicitWidth + 32)
    leftPadding: 16
    rightPadding: 16
    hoverEnabled: true

    function foregroundColor() {
        if (!enabled)
            return Theme.alpha(Theme.surfaceForeground, 0.38);
        if (variant === "filled")
            return Theme.primaryForeground;
        if (variant === "tonal")
            return Theme.secondaryContainerForeground;
        if (variant === "danger")
            return Theme.errorForeground;
        return variant === "outlined" || variant === "text" ? Theme.primary : Theme.surfaceForeground;
    }

    function backgroundColor() {
        if (!enabled)
            return variant === "text" || variant === "outlined" ? "transparent" : Theme.alpha(Theme.surfaceForeground, 0.12);
        if (variant === "filled")
            return Theme.primary;
        if (variant === "tonal")
            return Theme.secondaryContainer;
        if (variant === "danger")
            return Theme.error;
        return "transparent";
    }

    contentItem: RowLayout {
        id: contentRow
        spacing: Theme.space8
        MdIcon {
            visible: root.iconName.length > 0
            name: root.iconName
            color: root.foregroundColor()
            iconSize: 20
        }
        Label {
            text: root.text
            color: root.foregroundColor()
            font.pixelSize: Theme.labelSize
            font.weight: Font.DemiBold
            Layout.alignment: Qt.AlignVCenter
        }
    }

    background: Rectangle {
        radius: root.height / 2
        color: root.backgroundColor()
        border.width: root.activeFocus || root.variant === "outlined" ? 1 : 0
        border.color: root.activeFocus ? Theme.primary : Theme.outline

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: root.down ? Theme.alpha(root.foregroundColor(), 0.12)
                             : root.hovered ? Theme.alpha(root.foregroundColor(), 0.08) : "transparent"
            Behavior on color { ColorAnimation { duration: Theme.motionFast } }
        }
    }
}
