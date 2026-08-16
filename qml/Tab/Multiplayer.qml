pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import CubedLauncher

PageScaffold {
    id: multiplayerTab
    property int mode: 0
    title: qsTr("Multiplayer")
    subtitle: qsTr("Create or join private Cubed sessions with EasyTier or Frp.")

    Rectangle {
        Layout.alignment: Qt.AlignHCenter
        implicitWidth: modeRow.implicitWidth + Theme.space8
        implicitHeight: 52
        radius: 26
        color: Theme.surfaceContainer
        RowLayout {
            id: modeRow
            anchors.fill: parent
            anchors.margins: Theme.space4
            spacing: Theme.space4
            MdButton {
                text: qsTr("EasyTier")
                iconName: "hub"
                variant: multiplayerTab.mode === 0 ? "tonal" : "text"
                onClicked: multiplayerTab.mode = 0
            }
            MdButton {
                text: qsTr("Frp")
                iconName: "network"
                variant: multiplayerTab.mode === 1 ? "tonal" : "text"
                onClicked: multiplayerTab.mode = 1
            }
        }
    }

    Loader {
        Layout.fillWidth: true
        source: multiplayerTab.mode === 0
            ? "qrc:/qt/qml/CubedLauncher/qml/Tool/EasyTierSection.qml"
            : "qrc:/qt/qml/CubedLauncher/qml/Tool/FrpSection.qml"
    }
}
