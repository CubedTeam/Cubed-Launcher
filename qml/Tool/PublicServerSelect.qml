pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CubedLauncher

Dialog {
    id: root
    anchors.centerIn: Overlay.overlay
    width: Math.min(520, Overlay.overlay.width - 48)
    height: Math.min(440, Overlay.overlay.height - 48)
    modal: true
    title: qsTr("Select Public Server")
    standardButtons: Dialog.NoButton
    padding: Theme.space16
    palette.text: Theme.onSurface
    background: Rectangle { color: Theme.surfaceContainerHigh; radius: Theme.radiusExtraLarge }

    ListModel {
        id: serverModel
        Component.onCompleted: {
            const names = EasyTierManager.publicServerNames;
            for (let index = 0; index < names.length; ++index)
                append({ name: names[index], address: EasyTierManager.public_server_address(index) });
        }
    }
    ColumnLayout {
        anchors.fill: parent
        spacing: Theme.space12
        Label {
            Layout.fillWidth: true
            text: qsTr("Select a community relay for the EasyTier room.")
            color: Theme.onSurfaceVariant
            font.pixelSize: Theme.bodySize
        }
        ListView {
            id: serverList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: Theme.space4
            model: serverModel
            delegate: ItemDelegate {
                id: row
                required property int index
                required property string name
                required property string address
                width: serverList.width
                height: 68
                highlighted: Settings.easytierPublicServerIndex === index
                onClicked: { Settings.easytierPublicServerIndex = index; root.close(); }
                contentItem: RowLayout {
                    MdIcon { name: row.highlighted ? "check" : "public"; color: row.highlighted ? Theme.primary : Theme.onSurfaceVariant }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Label { text: row.name; color: Theme.onSurface; font.pixelSize: Theme.bodyLargeSize }
                        Label { text: row.address; color: Theme.onSurfaceVariant; font.family: "Monospace"; font.pixelSize: Theme.labelSize }
                    }
                }
                background: Rectangle {
                    radius: Theme.radiusMedium
                    color: row.highlighted ? Theme.secondaryContainer : row.hovered ? Theme.surfaceContainerHighest : "transparent"
                }
            }
        }
        MdButton { Layout.alignment: Qt.AlignRight; text: qsTr("Close"); variant: "text"; onClicked: root.close() }
    }
}
