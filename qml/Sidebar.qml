pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CubedLauncher

Item {
    id: root
    property string appVersion
    readonly property bool expanded: width >= 1180
    readonly property int navigationWidth: expanded ? 216 : 88

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.preferredWidth: root.navigationWidth
            Layout.fillHeight: true
            color: Theme.surfaceContainerLow

            Behavior on Layout.preferredWidth {
                NumberAnimation {
                    duration: Theme.motionNormal
                    easing.type: Theme.motionEasing
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Theme.space12
                spacing: Theme.space8

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 64
                    RowLayout {
                        anchors.fill: parent
                        spacing: Theme.space12
                        Image {
                            source: "qrc:/qt/qml/CubedLauncher/resources/CubedLauncher.png"
                            sourceSize.width: 44
                            sourceSize.height: 44
                            Layout.preferredWidth: 44
                            Layout.preferredHeight: 44
                            smooth: true
                        }
                        ColumnLayout {
                            opacity: root.expanded ? 1 : 0
                            visible: opacity > 0
                            Layout.fillWidth: true
                            spacing: 0

                            Behavior on opacity {
                                NumberAnimation {
                                    duration: Theme.motionNormal
                                    easing.type: Theme.motionEasing
                                }
                            }

                            Label {
                                text: "Cubed"
                                color: Theme.surfaceForeground
                                font.pixelSize: Theme.titleSize
                                font.weight: Font.Bold
                            }
                            Label {
                                text: qsTr("Launcher")
                                color: Theme.surfaceVariantForeground
                                font.pixelSize: Theme.labelSize
                            }
                        }
                    }
                }

                ListView {
                    id: navList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: Theme.space4
                    clip: true
                    currentIndex: SideTool.currentIndex
                    model: [
                        {
                            key: "launcher",
                            icon: "sports_esports"
                        },
                        {
                            key: "manager",
                            icon: "download"
                        },
                        {
                            key: "multiplayer",
                            icon: "hub"
                        },
                        {
                            key: "setting",
                            icon: "settings"
                        },
                        {
                            key: "about",
                            icon: "info"
                        }
                    ]

                    function labelFor(key) {
                        if (key === "launcher")
                            return qsTr("Launcher");
                        if (key === "manager")
                            return qsTr("Manager");
                        if (key === "multiplayer")
                            return qsTr("Multiplayer");
                        if (key === "setting")
                            return qsTr("Setting");
                        return qsTr("About");
                    }

                    delegate: ItemDelegate {
                        id: navDelegate
                        required property int index
                        required property var modelData
                        width: navList.width
                        height: 56
                        hoverEnabled: true
                        highlighted: ListView.isCurrentItem
                        leftPadding: root.expanded ? Theme.space16 : 0
                        rightPadding: root.expanded ? Theme.space16 : 0

                        contentItem: Item {
                            MdIcon {
                                x: root.expanded ? 0 : (parent.width - width) / 2
                                anchors.verticalCenter: parent.verticalCenter
                                name: navDelegate.modelData.icon
                                color: navDelegate.highlighted ? Theme.secondaryContainerForeground : Theme.surfaceVariantForeground

                                Behavior on x {
                                    NumberAnimation {
                                        duration: Theme.motionNormal
                                        easing.type: Theme.motionEasing
                                    }
                                }
                            }
                            Label {
                                opacity: root.expanded ? 1 : 0
                                visible: opacity > 0
                                anchors.fill: parent
                                text: navList.labelFor(navDelegate.modelData.key)
                                color: navDelegate.highlighted ? Theme.secondaryContainerForeground : Theme.surfaceVariantForeground
                                font.pixelSize: Theme.bodySize
                                font.weight: navDelegate.highlighted ? Font.DemiBold : Font.Normal
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter

                                Behavior on opacity {
                                    NumberAnimation {
                                        duration: Theme.motionNormal
                                        easing.type: Theme.motionEasing
                                    }
                                }
                            }
                        }

                        background: Rectangle {
                            radius: height / 2
                            color: navDelegate.highlighted ? Theme.secondaryContainer : navDelegate.down ? Theme.surfaceContainerHighest : navDelegate.hovered ? Theme.surfaceContainerHigh : "transparent"
                            border.width: navDelegate.activeFocus ? 1 : 0
                            border.color: Theme.primary
                            Behavior on color {
                                ColorAnimation {
                                    duration: Theme.motionFast
                                }
                            }
                        }

                        MdToolTip {
                            target: navDelegate
                            showOnRight: true
                            visible: !root.expanded && navDelegate.hovered
                            text: navList.labelFor(navDelegate.modelData.key)
                        }
                        onClicked: SideTool.currentIndex = index
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.surface

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                CheckUpdate {
                    Layout.fillWidth: true
                    Layout.leftMargin: Theme.space24
                    Layout.rightMargin: Theme.space24
                    Layout.topMargin: visible ? Theme.space16 : 0
                    visible: LauncherUpdate.hasNewVersion || LauncherUpdate.hasError
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: SideTool.currentIndex
                    Launch {}
                    Manager {}
                    Multiplayer {}
                    Setting {}
                    About {
                        appVersion: root.appVersion
                    }
                }
            }
        }
    }
}
