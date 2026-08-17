pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CubedLauncher

PageScaffold {
    id: launchTab
    title: qsTr("Ready to play?")
    subtitle: qsTr("Launch Cubed and keep your game installation up to date.")

    Component.onCompleted: CubedGame.check_version()

    InfoBanner {
        Layout.fillWidth: true
        visible: !CubedGame.installed
        tone: "warning"
        iconName: "download"
        text: qsTr("Cubed is not installed yet. Install the game before launching.")
        actionText: qsTr("Open Manager")
        onActionClicked: SideTool.currentIndex = 1
    }

    GridLayout {
        Layout.fillWidth: true
        columns: launchTab.width >= 1040 ? 2 : 1
        columnSpacing: Theme.space24
        rowSpacing: Theme.space24

        Card {
            Layout.fillWidth: true
            Layout.preferredHeight: 350

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Theme.space32
                spacing: Theme.space16

                Image {
                    source: "qrc:/qt/qml/CubedLauncher/resources/logo.png"
                    sourceSize.width: 245
                    sourceSize.height: 89
                    fillMode: Image.PreserveAspectFit
                    Layout.preferredWidth: 245
                    Layout.preferredHeight: 89
                    Layout.alignment: Qt.AlignHCenter
                }

                Item {
                    Layout.fillHeight: true
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: qsTr("Game status")
                        color: Theme.surfaceVariantForeground
                        font.pixelSize: Theme.bodySize
                        Layout.fillWidth: true
                    }
                    StatusChip {
                        text: CubedGame.running ? qsTr("Running") : CubedGame.installed ? qsTr("Ready") : qsTr("Not installed")
                        iconName: CubedGame.running ? "play_arrow" : CubedGame.installed ? "check" : "download"
                        tone: CubedGame.running || CubedGame.installed ? "success" : "warning"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 1
                    color: Theme.outlineVariant
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: qsTr("Installed version")
                        color: Theme.surfaceVariantForeground
                        font.pixelSize: Theme.bodySize
                        Layout.fillWidth: true
                    }
                    Label {
                        text: CubedGame.installed ? CubedGame.version : qsTr("—")
                        color: Theme.surfaceForeground
                        font.pixelSize: Theme.bodyLargeSize
                        font.weight: Font.DemiBold
                    }
                }

                StatusChip {
                    visible: GameUpdate.hasNewVersion
                    Layout.alignment: Qt.AlignLeft
                    text: qsTr("Version %1 available").arg(GameUpdate.remoteVersion)
                    iconName: "update"
                    tone: "warning"
                }
            }
        }

        Card {
            Layout.fillWidth: true
            Layout.preferredHeight: 350
            color: Theme.surfaceContainer

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Theme.space32
                spacing: Theme.space16

                MdIcon {
                    name: "sports_esports"
                    iconSize: 36
                    color: Theme.primary
                    Layout.alignment: Qt.AlignLeft
                }
                Label {
                    text: qsTr("Launch game")
                    color: Theme.surfaceForeground
                    font.pixelSize: Theme.titleSize
                    font.weight: Font.DemiBold
                }
                Label {
                    text: qsTr("Choose the player name used by Cubed.")
                    color: Theme.surfaceVariantForeground
                    font.pixelSize: Theme.bodySize
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                MdTextField {
                    id: playerNameText
                    Layout.fillWidth: true
                    text: Settings.playerName
                    placeholderText: qsTr("Enter Player Name")
                    enabled: !CubedGame.running
                    onEditingFinished: {
                        Settings.playerName = text;
                        CubedGame.set_name(text);
                    }
                    Component.onCompleted: CubedGame.set_name(text)
                }

                Item {
                    Layout.fillHeight: true
                }

                MdButton {
                    visible: !CubedGame.running
                    Layout.fillWidth: true
                    text: CubedGame.installed ? qsTr("Start Game") : qsTr("Install Game First")
                    iconName: CubedGame.installed ? "play_arrow" : "download"
                    enabled: CubedGame.installed
                    onClicked: CubedGame.start_cubed_game()
                    Component.onCompleted: {
                        if (Settings.pathSetted)
                            CubedGame.set_game_dir(Settings.gameDir);
                    }
                }
                MdButton {
                    visible: CubedGame.running
                    Layout.fillWidth: true
                    text: qsTr("Stop all game processes")
                    iconName: "stop"
                    variant: "danger"
                    onClicked: CubedGame.kill_all()
                }
            }
        }
    }
}
