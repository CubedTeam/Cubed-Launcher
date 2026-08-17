import QtQuick
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
    transformOrigin: Item.Center

    // AI-generated: Keep popup motion consistent.
    enter: Transition {
        ParallelAnimation {
            NumberAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: Theme.motionEmphasized
                easing.type: Theme.motionEasing
            }
            NumberAnimation {
                property: "scale"
                from: 0.96
                to: 1
                duration: Theme.motionEmphasized
                easing.type: Theme.motionEasing
            }
        }
    }

    exit: Transition {
        ParallelAnimation {
            NumberAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: Theme.motionFast
                easing.type: Theme.motionExitEasing
            }
            NumberAnimation {
                property: "scale"
                from: 1
                to: 0.96
                duration: Theme.motionFast
                easing.type: Theme.motionExitEasing
            }
        }
    }
}
