#pragma once

#include <QObject>
#include <QQmlEngine>

class CubedInstance : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    Q_INVOKABLE void start_cubed_instance();
    Q_INVOKABLE void set_game_path(const QUrl& game_path);
    Q_INVOKABLE void set_peer(int index);
    Q_INVOKABLE void set_port(const QString& port);
    Q_INVOKABLE void set_ip(const QString& ip);
    Q_INVOKABLE void set_name(const QString& name);

private:
    QString m_game_path;
    QString m_peer_mode{"--host"};
    QString m_port{"25530"};
    QString m_ip{"127.0.0.1"};
    QString m_name{"Unknown"};
};