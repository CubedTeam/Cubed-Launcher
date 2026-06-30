#pragma once

#include <QObject>
#include <QProcess>
#include <QQmlEngine>
class CubedInstance : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool running READ running NOTIFY running_changed)
    Q_PROPERTY(bool path_selected READ game_path_select NOTIFY path_change)
    Q_PROPERTY(bool logOn WRITE set_log_statue)
public:
    Q_INVOKABLE void start_cubed_instance();
    Q_INVOKABLE void set_game_path(const QUrl& game_path);
    Q_INVOKABLE void set_peer(int index);
    Q_INVOKABLE void set_port(const QString& port);
    Q_INVOKABLE void set_ip(const QString& ip);
    Q_INVOKABLE void set_name(const QString& name);
    Q_INVOKABLE void set_wrapper_command(const QString& wrapper);
    Q_INVOKABLE void kill_all();
    bool running() const;
    bool game_path_select() const;
    void set_log_statue(bool status);
signals:
    void running_changed();
    void path_change();

private:
    QString m_game_path;
    QString m_wrapper_command;
    QString m_peer_mode{"--host"};
    QString m_port{"25530"};
    QString m_ip{"127.0.0.1"};
    QString m_name{"Unknown"};
    QList<QProcess*> m_processes;
    bool m_log_on{false};
};