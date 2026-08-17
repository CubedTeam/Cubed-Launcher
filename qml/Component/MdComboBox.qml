pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls

ComboBox {
    id: root
    implicitHeight: 48
    leftPadding: 16
    rightPadding: 44
    font.pixelSize: Theme.bodySize

    contentItem: Text {
        text: root.displayText
        color: Theme.surfaceForeground
        font: root.font
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight

        Behavior on color { ColorAnimation { duration: Theme.motionFast } }
    }
    indicator: MdIcon {
        name: "expand_more"
        color: Theme.surfaceVariantForeground
        iconSize: 20
        x: root.width - width - 14
        anchors.verticalCenter: parent.verticalCenter
        rotation: root.popup.visible ? 180 : 0

        Behavior on rotation {
            NumberAnimation { duration: Theme.motionFast; easing.type: Theme.motionEasing }
        }
    }
    background: Rectangle {
        property color outlineColor: root.activeFocus ? Theme.primary : Theme.outline
        radius: Theme.radiusSmall
        color: Theme.surfaceContainerHighest
        border.width: root.activeFocus ? 2 : 1
        border.color: outlineColor

        Behavior on color { ColorAnimation { duration: Theme.motionNormal } }
        Behavior on outlineColor { ColorAnimation { duration: Theme.motionFast } }
    }
    delegate: ItemDelegate {
        id: comboDelegate
        required property var modelData
        required property int index
        width: ListView.view.width
        text: root.textRole.length > 0 ? modelData[root.textRole] : modelData
        highlighted: root.highlightedIndex === index
        palette.text: Theme.surfaceForeground
        background: Rectangle {
            color: comboDelegate.highlighted ? Theme.secondaryContainer : Theme.surfaceContainerHigh
        }
    }
    popup: Popup {
        y: root.height + 4
        width: root.width
        implicitHeight: Math.min(contentItem.implicitHeight + 16, 320)
        padding: 8
        transformOrigin: Item.Top

        enter: Transition {
            ParallelAnimation {
                NumberAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: Theme.motionEmphasized
                    easing.type: Theme.motionEasing
                }
                NumberAnimation {
                    property: "scale"
                    from: 0.96
                    to: 1
                    duration: Theme.motionEmphasized
                    easing.type: Theme.motionEasing
                }
            }
        }

        exit: Transition {
            ParallelAnimation {
                NumberAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: Theme.motionFast
                    easing.type: Theme.motionExitEasing
                }
                NumberAnimation {
                    property: "scale"
                    from: 1
                    to: 0.96
                    duration: Theme.motionFast
                    easing.type: Theme.motionExitEasing
                }
            }
        }

        background: Rectangle {
            color: Theme.surfaceContainerHigh
            radius: Theme.radiusMedium
            border.width: 1
            border.color: Theme.outlineVariant

            Behavior on color { ColorAnimation { duration: Theme.motionNormal } }
        }
        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator {}
        }
    }
}
