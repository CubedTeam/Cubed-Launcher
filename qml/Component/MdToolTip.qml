import QtQuick
import QtQuick.Controls

ToolTip {
    id: root
    required property Item target
    property bool showOnRight: false

    parent: target
    x: showOnRight ? target.width + Theme.space8
                   : Math.round((target.width - implicitWidth) / 2)
    y: showOnRight ? Math.round((target.height - implicitHeight) / 2)
                   : -implicitHeight - Theme.space4
    delay: 450
    leftPadding: Theme.space12
    rightPadding: Theme.space12
    topPadding: Theme.space8
    bottomPadding: Theme.space8

    contentItem: Label {
        text: root.text
        color: Theme.surfaceForeground
        font.pixelSize: Theme.labelSize
    }

    background: Rectangle {
        color: Theme.surfaceContainerHighest
        radius: Theme.radiusSmall
        border.width: 1
        border.color: Theme.outlineVariant
    }
}
