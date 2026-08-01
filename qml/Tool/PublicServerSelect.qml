pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts

// AI-generated: public EasyTier community server picker dialog. Mirrors
// MirrorSelect.qml in shape but stays simpler: no latency probing, since
// reachability is best verified by actually launching easytier-core.
Dialog {
    id: publicServerPopup
    anchors.centerIn: Overlay.overlay
    width: 480
    height: 380
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    title: qsTr("Select Public Server")
    standardButtons: Dialog.NoButton

    ListModel {
        id: serverModel
        Component.onCompleted: {
            const names = EasyTierManager.publicServerNames;
            for (let i = 0; i < names.length; ++i) {
                serverModel.append({
                    "name": names[i],
                    "address": EasyTierManager.public_server_address(i)
                });
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("Tap a server to select it. Only the index is saved.")
            font.pixelSize: 12
            color: Material.color(Material.Grey)
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }

        ListView {
            id: serverList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: serverModel
            delegate: ItemDelegate {
                id: row
                width: serverList.width
                required property int index
                required property string name
                required property string address

                onClicked: {
                    Settings.easytierPublicServerIndex = row.index;
                    publicServerPopup.close();
                }

                contentItem: ColumnLayout {
                    spacing: 2
                    width: parent.width

                    RadioButton {
                        checked: Settings.easytierPublicServerIndex === row.index
                        onClicked: {
                            Settings.easytierPublicServerIndex = row.index;
                            publicServerPopup.close();
                        }
                        text: row.name
                        font.pixelSize: 18
                        Layout.fillWidth: true
                    }

                    Label {
                        text: row.address
                        font.pixelSize: 12
                        font.family: "Monospace"
                        color: Material.color(Material.Grey)
                        Layout.fillWidth: true
                        leftPadding: 32
                    }
                }
            }
        }
    }
}
