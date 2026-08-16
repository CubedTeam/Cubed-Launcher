pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import CubedLauncher

MdDialog {
    id: root
    parent: Overlay.overlay
    anchors.centerIn: Overlay.overlay
    width: Math.min(520, Overlay.overlay.width - Theme.space32 * 2)
    modal: true
    closePolicy: Popup.NoAutoClose
    standardButtons: Dialog.NoButton
    padding: Theme.space32
    background: Rectangle { color: Theme.surfaceContainerHigh; radius: Theme.radiusExtraLarge }

    Component.onCompleted: {
        if (!Settings.pathSetted)
            open();
    }

    ColumnLayout {
        width: parent.width
        spacing: Theme.space16
        Image {
            source: "qrc:/qt/qml/CubedLauncher/resources/CubedLauncher.png"
            sourceSize.width: 80
            sourceSize.height: 80
            Layout.preferredWidth: 80
            Layout.preferredHeight: 80
            Layout.alignment: Qt.AlignHCenter
        }
        Label {
            text: qsTr("Welcome to Cubed Launcher")
            color: Theme.surfaceForeground
            font.pixelSize: Theme.headlineSize
            font.weight: Font.Bold
            Layout.alignment: Qt.AlignHCenter
        }
        Label {
            text: qsTr("Choose where to install Cubed game files. You can change this later in Settings.")
            color: Theme.surfaceVariantForeground
            font.pixelSize: Theme.bodySize
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: pathLabel.implicitHeight + Theme.space16 * 2
            radius: Theme.radiusMedium
            color: Theme.surfaceContainer
            RowLayout {
                anchors.fill: parent
                anchors.margins: Theme.space16
                MdIcon { name: "folder"; color: Theme.surfaceVariantForeground }
                Label {
                    id: pathLabel
                    Layout.fillWidth: true
                    text: SystemInfo.defaultGameInstallDir
                    color: Theme.surfaceVariantForeground
                    font.pixelSize: Theme.labelSize
                    wrapMode: Text.WrapAnywhere
                }
            }
        }
        MdButton {
            text: qsTr("Use Default Folder")
            iconName: "check"
            Layout.fillWidth: true
            onClicked: {
                Settings.set_game_dir(SystemInfo.defaultGameInstallDir);
                root.close();
            }
        }
        MdButton {
            text: qsTr("Choose Install Folder")
            iconName: "folder"
            variant: "outlined"
            Layout.fillWidth: true
            onClicked: folderDialog.open()
        }
    }

    FolderDialog {
        id: folderDialog
        title: qsTr("Select Game Install Folder")
        onAccepted: {
            Settings.set_game_dir_url(selectedFolder);
            root.close();
        }
    }
}
