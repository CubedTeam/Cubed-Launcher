// AI-generated: rounded card container; border color follows
// Settings.cardColorfulBorder. Usage: wrap content in a ColumnLayout, then
// set Layout.preferredHeight: innerId.implicitHeight + 20.
import QtQuick
import QtQuick.Controls.Material

import CubedLauncher

Rectangle {
    color: "white"
    radius: 16
    border.color: Settings.cardColorfulBorder
        ? Qt.lighter(Settings.accentColor, 1.4)
        : Material.color(Material.Grey, Material.Shade300)
    border.width: 2
}
