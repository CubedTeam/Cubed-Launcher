import QtQuick.Controls
import QtQuick.Controls.Material

Dialog {
    Material.theme: Theme.resolvedDark ? Material.Dark : Material.Light
    Material.primary: Theme.primary
    Material.accent: Theme.primary
    Material.foreground: Theme.surfaceForeground
    Material.background: Theme.surfaceContainerHigh

    palette.window: Theme.surfaceContainerHigh
    palette.windowText: Theme.surfaceForeground
    palette.text: Theme.surfaceForeground
    palette.buttonText: Theme.surfaceForeground
}
