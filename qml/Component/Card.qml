// AI-generated: reusable rounded card container with a thin grey border.
// Usage: wrap inner content in a ColumnLayout with id <innerId>, then set
// Layout.preferredHeight: innerId.implicitHeight + 20 inside the Card.
import QtQuick

import CubedLauncher

Rectangle {
    color: "white"
    radius: 16
    border.color: Qt.lighter(Settings.accentColor, 1.4)
    //border.color: Material.color(Material.Grey, Material.Shade300)
    border.width: 2
}
