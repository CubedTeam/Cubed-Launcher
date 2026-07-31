// AI-generated: frp install/management card. Delegates to the shared
// ServiceInstallCard with frp-specific labels.
pragma ComponentBehavior: Bound
import QtQuick
import CubedLauncher

ServiceInstallCard {
    manager: FrpManager
    title: qsTr("Frp Manager")
    showInstalledVersion: true
    blockWhileRunning: true
}
