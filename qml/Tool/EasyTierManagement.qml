// AI-generated: easytier install/management card. Delegates to the shared
// ServiceInstallCard with easytier-specific labels.
pragma ComponentBehavior: Bound
import QtQuick
import CubedLauncher

ServiceInstallCard {
    manager: EasyTierManager
    title: qsTr("EasyTier Manager")
    showInstalledVersion: true
    blockWhileRunning: true
}
