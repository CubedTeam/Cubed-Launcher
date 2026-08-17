pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CubedLauncher

MdDialog {
    id: root
    anchors.centerIn: parent
    width: Math.min(520, parent ? parent.width - 48 : 520)
    height: Math.min(440, parent ? parent.height - 48 : 440)
    modal: true
    title: qsTr("Select Public Server")
    standardButtons: Dialog.NoButton
    padding: Theme.space16
    palette.text: Theme.surfaceForeground
    background: Rectangle {
        color: Theme.surfaceContainerHigh
        radius: Theme.radiusExtraLarge
    }

    ListModel {
        id: serverModel
        Component.onCompleted: {
            const names = EasyTierManager.publicServerNames;
            for (let index = 0; index < names.length; ++index)
                append({
                    name: names[index],
                    address: EasyTierManager.public_server_address(index)
                });
        }
    }
    ColumnLayout {
        anchors.fill: parent
        spacing: Theme.space12
        Label {
            Layout.fillWidth: true
            text: qsTr("Select a community relay for the EasyTier room.")
            color: Theme.surfaceVariantForeground
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
                leftPadding: Theme.space16
                rightPadding: Theme.space16
                topPadding: 0
                bottomPadding: 0
                onClicked: {
                    Settings.easytierPublicServerIndex = index;
                    root.close();
                }
                contentItem: RowLayout {
                    spacing: Theme.space12
                    Item {
                        Layout.preferredWidth: 24
                        Layout.fillHeight: true
                        MdIcon {
                            anchors.centerIn: parent
                            name: row.highlighted ? "check" : "public"
                            color: row.highlighted ? Theme.primary : Theme.surfaceVariantForeground
                        }
                    }
                    Label {
                        Layout.preferredWidth: 160
                        text: row.name
                        color: Theme.surfaceForeground
                        font.pixelSize: Theme.bodyLargeSize
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                    }
                    Label {
                        Layout.fillWidth: true
                        text: row.address
                        color: Theme.surfaceVariantForeground
                        font.family: "Monospace"
                        font.pixelSize: Theme.labelSize
                        elide: Text.ElideMiddle
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                background: Rectangle {
                    radius: Theme.radiusMedium
                    color: row.highlighted ? Theme.secondaryContainer : row.hovered ? Theme.surfaceContainerHighest : "transparent"
                }
            }
        }
        MdButton {
            Layout.alignment: Qt.AlignRight
            text: qsTr("Close")
            variant: "text"
            onClicked: root.close()
        }
    }
}
