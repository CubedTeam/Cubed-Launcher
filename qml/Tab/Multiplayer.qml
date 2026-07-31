// AI-generated: multiplayer tab shell. Hosts a horizontal EasyTier/Frp
// mode toggle and a Loader that swaps in the matching section file. The
// frp install / control / log UI and the easytier install UI live in
// their own files (FrpSection.qml, EasyTierSection.qml).
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import CubedLauncher
import QtQuick.Layouts

Item {
    id: multiplayerTab
    Layout.fillHeight: true
    Layout.fillWidth: true

    // 0 = EasyTier, 1 = Frp.
    property int mode: 0

    Flickable {
        anchors.fill: parent
        contentWidth: width
        contentHeight: mainColumn.implicitHeight + 40
        clip: true
        boundsMovement: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {}

        ColumnLayout {
            id: mainColumn
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 20
            width: 560
            spacing: 12

            // First tab = EasyTier, second = Frp.

            Card {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                Layout.preferredHeight: modeRow.implicitHeight + 20

                RowLayout {
                    id: modeRow
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 0

                    TabBar {
                        id: modeBar
                        Layout.preferredWidth: implicitWidth
                        Layout.preferredHeight: 44
                        Layout.alignment: Qt.AlignCenter
                        currentIndex: multiplayerTab.mode
                        onCurrentIndexChanged: multiplayerTab.mode = currentIndex
                        Material.elevation: 0
                        Material.background: "white"

                        TabButton {
                            text: qsTr("EasyTier")
                            font.pixelSize: 16
                            width: implicitWidth
                        }
                        TabButton {
                            text: qsTr("Frp")
                            font.pixelSize: 16
                            width: implicitWidth
                        }
                    }
                }
            }

            Loader {
                id: sectionLoader
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                source: multiplayerTab.mode === 0 ? "qrc:/qt/qml/CubedLauncher/qml/Tool/EasyTierSection.qml" : "qrc:/qt/qml/CubedLauncher/qml/Tool/FrpSection.qml"
            }
        }
    }
}
