// AI-generated: shared install/management card for FrpManager and
// EasyTierManager. Parameterised on the manager instance and a few labels
// so the same UI can render either service.
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts

Item {
    id: root
    property var manager
    property string title: ""
    // Some services want to advertise the installed version inside the
    // install card; frp prefers to keep the version only on the control
    // card.
    property bool showInstalledVersion: true
    // When true, the primary action button is also disabled while the
    // service is running (matches frp). EasyTier historically allowed
    // reinstall while running, so this defaults to false.
    property bool blockWhileRunning: false

    implicitWidth: installLayout.implicitWidth
    implicitHeight: installLayout.implicitHeight
    width: installLayout.width
    height: installLayout.implicitHeight

    ColumnLayout {
        id: installLayout
        width: 560
        spacing: 10

        Label {
            Layout.alignment: Qt.AlignCenter
            text: root.title
            font.pixelSize: 18
            font.bold: true
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("Not Installed")
            visible: !root.manager.installed && !root.manager.busy
            font.pixelSize: 18
            color: Material.color(Material.Orange)
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("Installed: %1").arg(root.manager.version)
            visible: root.showInstalledVersion && root.manager.installed && !root.manager.busy
            font.pixelSize: 18
            color: Material.color(Material.Green)
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            visible: root.manager.busy
            text: {
                // BinaryServiceBase::State values: NotInstalled=0, Checking=1,
                // Downloading=2, Extracting=3. Compared as integers because
                // the manager is bound through a `var` property and QML
                // cannot resolve Q_ENUM values from `var`.
                const s = root.manager.state;
                if (s === 1) return qsTr("Checking for updates...");
                if (s === 2) return qsTr("Downloading...");
                if (s === 3) return qsTr("Extracting...");
                return qsTr("Working...");
            }
            font.pixelSize: 18
        }

        ProgressBar {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 400
            from: 0.0
            to: 1.0
            value: root.manager.downloadProgress
            visible: root.manager.state === 2
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            visible: root.manager.hasError
            text: root.manager.errorMessage
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            color: Material.color(Material.Red)
            font.pixelSize: 16
        }
        Switch {
            id: customDowlaodSwitch
            checked: false
            font.pixelSize: 14
            Layout.alignment: Qt.AlignCenter
            text: qsTr("Custom Link")
        }
        Button {
            id: mirrorButton
            visible: !customDowlaodSwitch.checked
            enabled: visible
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 250
            Layout.preferredHeight: 50
            Material.roundedScale: Material.MediumScale
            highlighted: true
            font.pixelSize: 18
            text: {
                const idx = Settings.mirrorIndex >= 0 ? Settings.mirrorIndex : (SystemInfo.isInChina ? 1 : 0);
                return qsTr("Mirror: ") + MirrorSource.names[idx];
            }
            onClicked: mirrorPopup.open()
        }
        TextField {
            id: customLinkText
            visible: customDowlaodSwitch.checked
            enabled: visible
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 250
            Layout.preferredHeight: 50
            placeholderText: qsTr("Download Link")
        }
        Button {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 250
            Layout.preferredHeight: 50
            font.pixelSize: 18
            Material.roundedScale: Material.MediumScale
            highlighted: true
            enabled: !root.manager.busy && (!root.blockWhileRunning || !root.manager.running)
            text: root.manager.installed ? qsTr("Reinstall") : qsTr("Download && Install")
            onClicked: {
                if (customDowlaodSwitch.checked) {
                    root.manager.install_from_url(customLinkText.text);
                } else {
                    root.manager.check_and_install(Settings.mirrorIndex);
                }
            }
        }

        Button {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 250
            Layout.preferredHeight: 50
            font.pixelSize: 16
            Material.roundedScale: Material.MediumScale
            visible: root.manager.installed && !root.manager.busy
            text: qsTr("Uninstall")
            onClicked: root.manager.reset_install()
        }
    }
    MirrorSelect {
        id: mirrorPopup
        parent: Overlay.overlay
    }
}
