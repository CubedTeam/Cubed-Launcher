import QtQuick

Rectangle {
    color: Theme.surfaceContainerLow
    radius: Theme.radiusLarge
    border.color: Theme.outlineVariant
    border.width: 0

    Behavior on color {
        ColorAnimation { duration: Theme.motionNormal }
    }
}
