import QtQuick
import QtQuick.Controls

Switch {
    id: root
    spacing: Theme.space12
    font.pixelSize: Theme.bodySize
    palette.text: Theme.onSurface

    indicator: Rectangle {
        implicitWidth: 52
        implicitHeight: 32
        x: root.leftPadding
        y: parent.height / 2 - height / 2
        radius: height / 2
        color: root.checked ? Theme.primary : Theme.surfaceContainerHighest
        border.width: root.checked ? 0 : 2
        border.color: Theme.outline

        Rectangle {
            x: root.checked ? parent.width - width - 6 : 6
            anchors.verticalCenter: parent.verticalCenter
            width: root.checked ? 24 : 16
            height: width
            radius: width / 2
            color: root.checked ? Theme.onPrimary : Theme.outline
            Behavior on x { NumberAnimation { duration: Theme.motionFast } }
            Behavior on width { NumberAnimation { duration: Theme.motionFast } }
        }
    }
}
