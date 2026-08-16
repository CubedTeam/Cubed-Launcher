import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import CubedLauncher

ApplicationWindow {
    id: launcherWindow
    // qmllint disable unqualified
    readonly property string appVersion: AppVersion
    // qmllint enable unqualified
    visible: true
    width: 1280
    height: 720
    minimumWidth: 900
    minimumHeight: 600
    title: qsTr("Cubed Launcher")
    color: Theme.surface

    Material.theme: Theme.resolvedDark ? Material.Dark : Material.Light
    Material.primary: Theme.primary
    Material.accent: Theme.primary
    Material.background: Theme.surface
    Material.foreground: Theme.surfaceForeground
    Material.roundedScale: Material.MediumScale

    Sidebar { anchors.fill: parent; appVersion: launcherWindow.appVersion }
    FirstLaunchDialog {}

    Behavior on color { ColorAnimation { duration: Theme.motionNormal } }
}
