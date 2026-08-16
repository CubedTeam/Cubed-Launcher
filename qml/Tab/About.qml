pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CubedLauncher

PageScaffold {
    id: aboutTab
    property string appVersion
    title: qsTr("About Cubed Launcher")
    subtitle: qsTr("Version information, system details, and project credits.")

    Card {
        Layout.fillWidth: true
        implicitHeight: brandRow.implicitHeight + Theme.space32 * 2
        color: Theme.primaryContainer
        RowLayout {
            id: brandRow
            anchors.fill: parent
            anchors.margins: Theme.space32
            spacing: Theme.space24
            Image {
                source: "qrc:/qt/qml/CubedLauncher/resources/CubedLauncher.png"
                sourceSize.width: 88
                sourceSize.height: 88
                Layout.preferredWidth: 88
                Layout.preferredHeight: 88
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Theme.space4
                Label {
                    text: "Cubed Launcher"
                    color: Theme.onPrimaryContainer
                    font.pixelSize: Theme.headlineSize
                    font.weight: Font.Bold
                }
                Label {
                    text: qsTr("A lightweight launcher and multiplayer companion for Cubed.")
                    color: Theme.onPrimaryContainer
                    opacity: 0.8
                    font.pixelSize: Theme.bodySize
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
            StatusChip {
                text: aboutTab.appVersion
                iconName: "info"
            }
        }
    }

    GridLayout {
        Layout.fillWidth: true
        columns: aboutTab.width >= 1040 ? 2 : 1
        columnSpacing: Theme.space24
        rowSpacing: Theme.space24

        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true
            implicitHeight: appInfo.implicitHeight + Theme.space32 * 2
            ColumnLayout {
                id: appInfo
                anchors.fill: parent
                anchors.margins: Theme.space24
                spacing: Theme.space12
                SectionHeader { title: qsTr("Application"); iconName: "sports_esports"; Layout.fillWidth: true }
                Repeater {
                    model: [
                        { label: qsTr("Name"), value: Qt.application.name },
                        { label: qsTr("Version"), value: aboutTab.appVersion },
                        { label: qsTr("Qt version"), value: SystemInfo.qtVersion }
                    ]
                    delegate: RowLayout {
                        id: appInfoRow
                        required property var modelData
                        Layout.fillWidth: true
                        Label { text: appInfoRow.modelData.label; color: Theme.onSurfaceVariant; font.pixelSize: Theme.bodySize; Layout.fillWidth: true }
                        Label { text: appInfoRow.modelData.value; color: Theme.onSurface; font.pixelSize: Theme.bodySize; font.weight: Font.DemiBold }
                    }
                }
                Item { Layout.fillHeight: true }
                RowLayout {
                    Layout.fillWidth: true
                    MdButton {
                        Layout.fillWidth: true
                        text: qsTr("Check for updates")
                        iconName: "update"
                        onClicked: LauncherUpdate.check_update("CubedTeam", "Cubed-Launcher")
                    }
                    SpecialThanks {}
                }
            }
        }

        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true
            implicitHeight: systemInfo.implicitHeight + Theme.space32 * 2
            ColumnLayout {
                id: systemInfo
                anchors.fill: parent
                anchors.margins: Theme.space24
                spacing: Theme.space12
                SectionHeader { title: qsTr("System"); iconName: "desktop"; Layout.fillWidth: true }
                Repeater {
                    model: [
                        { label: qsTr("Platform"), value: Qt.platform.os },
                        { label: qsTr("Product"), value: SystemInfo.productType },
                        { label: qsTr("System version"), value: SystemInfo.productVersion },
                        { label: qsTr("Kernel"), value: SystemInfo.kernelType },
                        { label: qsTr("Kernel version"), value: SystemInfo.kernelVersion }
                    ]
                    delegate: RowLayout {
                        id: systemInfoRow
                        required property var modelData
                        Layout.fillWidth: true
                        Label { text: systemInfoRow.modelData.label; color: Theme.onSurfaceVariant; font.pixelSize: Theme.bodySize; Layout.fillWidth: true }
                        Label {
                            text: systemInfoRow.modelData.value.length > 0 ? systemInfoRow.modelData.value : qsTr("Unknown")
                            color: Theme.onSurface
                            font.pixelSize: Theme.bodySize
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                            Layout.maximumWidth: 220
                        }
                    }
                }
            }
        }
    }
}
