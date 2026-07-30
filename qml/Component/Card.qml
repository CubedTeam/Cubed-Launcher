// AI-generated: reusable rounded card container, border color follows the cardColorfulBorder setting.
// Usage: wrap inner content in a ColumnLayout with id <innerId>, then set
// Layout.preferredHeight: innerId.implicitHeight + 20 inside the Card.
import QtQuick
import QtQuick.Controls.Material

import CubedLauncher

Rectangle {
    color: "white"
    radius: 16
    // AI-generated: accent-colored border when enabled, neutral grey otherwise.
    border.color: Settings.cardColorfulBorder
        ? Qt.lighter(Settings.accentColor, 1.4)
        : Material.color(Material.Grey, Material.Shade300)
    border.width: 2
}
