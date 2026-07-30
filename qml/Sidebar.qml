pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts

Rectangle {
    anchors.fill: parent

    color: Material.backgroundColor

    RowLayout {
        anchors.fill: parent
        Rectangle {
            Layout.preferredWidth: 140
            Layout.fillHeight: true
            color: Material.color(Material.Amber, Material.Shade50)
            ListView {
                id: navList
                anchors.fill: parent

                // AI-generated: plain string keys only, so model never rebuilds
                // on retranslate and currentIndex stays stable.
                model: ["Launcher", "Manager", "Setting", "About"]
                currentIndex: SideTool.currentIndex

                delegate: ItemDelegate {
                    id: sideDelegate
                    required property int index
                    required property string modelData
                    width: ListView.view.width
                    height: 60
                    font.pixelSize: 20
                    // AI-generated: literal qsTr lookups so retranslate refreshes
                    // the text without touching the stable string-key model.
                    text: {
                        if (modelData === "Launcher") return qsTr("Launcher")
                        if (modelData === "Manager") return qsTr("Manager")
                        if (modelData === "Setting") return qsTr("Setting")
                        if (modelData === "About") return qsTr("About")
                        return modelData
                    }

                    highlighted: ListView.isCurrentItem

                    onClicked: SideTool.currentIndex = index
                }
            }
        }

        StackLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true

            // AI-generated: bind to the C++ singleton so the active page stays put.
            currentIndex: SideTool.currentIndex

            Launch {}
            Manager {}
            Setting {}

            About {}
        }
    }
}
