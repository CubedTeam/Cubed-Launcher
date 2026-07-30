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

                //clip: true
                // AI-generated: JS array model so nav titles can pass through qsTr.
                model: [
                    { title: qsTr("Launcher") },
                    { title: qsTr("Manager") },
                    { title: qsTr("Setting") },
                    { title: qsTr("About") }
                ]
                currentIndex: SideTool.currentIndex

                delegate: ItemDelegate {
                    id: sideDelegate
                    required property int index
                    required property string title
                    width: ListView.view.width
                    height: 60
                    font.pixelSize: 20
                    // AI-generated: translate nav entry at render time.
                    text: qsTr(title)

                    highlighted: ListView.isCurrentItem

                    onClicked: SideTool.currentIndex = index
                }
            }
        }

        StackLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true

            currentIndex: navList.currentIndex

            Launch {}
            Manager {}
            Setting {}

            About {}
        }
    }
}
