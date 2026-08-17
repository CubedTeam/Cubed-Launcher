import QtQuick
import QtQuick.Controls

ProgressBar {
    id: root
    implicitHeight: 8
    background: Rectangle {
        implicitHeight: 8
        radius: 4
        color: Theme.secondaryContainer
    }
    contentItem: Item {
        implicitHeight: 8
        Rectangle {
            width: root.visualPosition * parent.width
            height: parent.height
            radius: 4
            color: Theme.primary
            Behavior on width {
                NumberAnimation {
                    duration: Theme.motionFast
                }
            }
        }
    }
}
