import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls

ApplicationWindow {
    id: cbedLauncher
    visible: true
    width: 640
    height: 480
    title: "Cubed Launcher"

    Material.background: Material.color(Material.Orange, Material.Shade50)
    Material.accent: Material.color(Material.Orange)

    Sidebar {}
}
