import QtQuick
import QtQuick.Controls

TextField {
    id: root
    implicitHeight: 48
    leftPadding: 16
    rightPadding: 16
    color: Theme.surfaceForeground
    placeholderTextColor: Theme.surfaceVariantForeground
    selectionColor: Theme.primaryContainer
    selectedTextColor: Theme.primaryContainerForeground
    font.pixelSize: Theme.bodySize
    background: Rectangle {
        property color outlineColor: root.activeFocus ? Theme.primary : Theme.outline
        radius: Theme.radiusSmall
        color: Theme.surfaceContainerHighest
        border.width: root.activeFocus ? 2 : 1
        border.color: outlineColor

        Behavior on color { ColorAnimation { duration: Theme.motionNormal } }
        Behavior on outlineColor { ColorAnimation { duration: Theme.motionFast } }
    }
}
