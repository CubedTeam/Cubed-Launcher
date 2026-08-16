import QtQuick
import QtQuick.Controls

TextField {
    id: root
    implicitHeight: 48
    leftPadding: 16
    rightPadding: 16
    color: Theme.onSurface
    placeholderTextColor: Theme.onSurfaceVariant
    selectionColor: Theme.primaryContainer
    selectedTextColor: Theme.onPrimaryContainer
    font.pixelSize: Theme.bodySize
    background: Rectangle {
        radius: Theme.radiusSmall
        color: Theme.surfaceContainerHighest
        border.width: root.activeFocus ? 2 : 1
        border.color: root.activeFocus ? Theme.primary : Theme.outline
    }
}
