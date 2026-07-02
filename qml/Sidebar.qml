pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts

Rectangle {
    anchors.fill: parent

    color: Material.backgroundColor

    Loader {
        active: VersionUpdate.hasNewVersion
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.rightMargin: 20
        source: "qrc:/qt/qml/CubedLauncher/qml/Tool/CheckUpdate.qml"
    }

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
                model: ListModel {
                    ListElement {
                        title: "Launcher"
                    }
                    ListElement {
                        title: "Manager"
                    }
                    ListElement {
                        title: "Setting"
                    }
                    ListElement {
                        title: "About"
                    }
                }
                currentIndex: 0

                delegate: ItemDelegate {
                    id: sideDelegate
                    required property int index
                    required property string title
                    width: ListView.view.width
                    height: 60
                    font.pixelSize: 20
                    text: title

                    highlighted: ListView.isCurrentItem

                    onClicked: navList.currentIndex = index
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
