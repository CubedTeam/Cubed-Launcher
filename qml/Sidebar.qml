pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts
import QtQuick.Controls.Material.impl

Rectangle {
    anchors.fill: parent

    color: Material.backgroundColor

    RowLayout {
        anchors.fill: parent
        Rectangle {
            Layout.preferredWidth: 140
            Layout.fillHeight: true
            color: Material.backgroundColor
            ListView {
                id: navList
                anchors.fill: parent

                // AI-generated: plain string keys so retranslate does not rebuild
                // the model and currentIndex stays stable.
                model: ["Launcher", "Manager", "Multiplayer", "Setting", "About"]
                currentIndex: SideTool.currentIndex

                delegate: ItemDelegate {
                    id: sideDelegate
                    required property int index
                    required property string modelData
                    width: ListView.view.width
                    height: 60
                    font.pixelSize: 20

                    contentItem: Label {
                        text: sideDelegate.text
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }

                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        visible: sideDelegate.index < navList.count - 1
                        height: 1
                        color: Qt.rgba(0, 0, 0, 0.12)
                    }

                    background: Rectangle {
                        id: bg
                        anchors.fill: parent
                        anchors.margins: 8
                        radius: 16
                        color: sideDelegate.highlighted ? Qt.lighter(Material.accent, 1.4) : "transparent"
                        clip: true

                        Ripple {
                            clipRadius: bg.radius
                            width: parent.width
                            height: parent.height
                            pressed: sideDelegate.pressed
                            active: sideDelegate.pressed
                            anchor: sideDelegate
                            color: Qt.lighter(Material.accent, 1.2)
                        }
                        Rectangle {
                            anchors.fill: parent
                            radius: parent.radius
                            color: sideDelegate.pressed ? Qt.rgba(0, 0, 0, 0.1) : (sideDelegate.hovered && !sideDelegate.highlighted ? Qt.rgba(0, 0, 0, 0.05) : "transparent")
                        }
                    }

                    text: {
                        if (modelData === "Launcher")
                            return qsTr("Launcher");
                        if (modelData === "Manager")
                            return qsTr("Manager");
                        if (modelData === "Multiplayer")
                            return qsTr("Multiplayer");
                        if (modelData === "Setting")
                            return qsTr("Setting");
                        if (modelData === "About")
                            return qsTr("About");
                        return modelData;
                    }

                    highlighted: ListView.isCurrentItem

                    onClicked: SideTool.currentIndex = index
                }
            }
        }

        StackLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true

            currentIndex: SideTool.currentIndex

            Launch {}
            Manager {}
            Multiplayer {}
            Setting {}

            About {}
        }
    }
}
