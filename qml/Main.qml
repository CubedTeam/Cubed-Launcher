import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher

ApplicationWindow {
    id: cbedLauncher
    visible: true
    width: 1280
    height: 720
    title: qsTr("Cubed Launcher")

    Material.background: "#F5F5F5"

    Material.accent: Settings.accentColor
    Material.roundedScale: Material.MediumScale
    Sidebar {}
    FirstLaunchDialog {}
}
