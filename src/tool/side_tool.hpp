#pragma once
#include <QObject>
#include <QQmlEngine>

class SideTool : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(int currentIndex READ current_index WRITE set_current_index
                   NOTIFY current_index_changed)
public:
    int current_index() const { return m_current_index; }

    void set_current_index(int index) {
        m_current_index = index;
        emit current_index_changed();
    }

signals:
    void current_index_changed();

private:
    int m_current_index = 0;
};