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
    }
    indicator: MdIcon {
        name: "expand_more"
        color: Theme.surfaceVariantForeground
        iconSize: 20
        x: root.width - width - 14
        anchors.verticalCenter: parent.verticalCenter
    }
    background: Rectangle {
        radius: Theme.radiusSmall
        color: Theme.surfaceContainerHighest
        border.width: root.activeFocus ? 2 : 1
        border.color: root.activeFocus ? Theme.primary : Theme.outline
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
        background: Rectangle {
            color: Theme.surfaceContainerHigh
            radius: Theme.radiusMedium
            border.width: 1
            border.color: Theme.outlineVariant
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
