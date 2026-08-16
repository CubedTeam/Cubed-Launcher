#pragma once

#include <QCoreApplication>
#include <QObject>
#include <QProcess>
#include <QQmlEngine>
class CubedGame : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool running READ running NOTIFY running_changed)
    Q_PROPERTY(bool path_selected READ game_path_select NOTIFY path_change)
    Q_PROPERTY(
        bool logOn READ log_on WRITE set_log_statue NOTIFY log_on_changed)
    Q_PROPERTY(bool installed READ installed NOTIFY installed_changed)
    Q_PROPERTY(QString version READ version NOTIFY version_changed)
public:
    CubedGame();
    Q_INVOKABLE void start_cubed_game();
    Q_INVOKABLE void set_game_dir_url(const QUrl& game_dir);
    Q_INVOKABLE void set_game_dir(const QString& game_dir);
    Q_INVOKABLE void set_peer(int index);
    Q_INVOKABLE void set_port(const QString& port);
    Q_INVOKABLE void set_ip(const QString& ip);
    Q_INVOKABLE void set_name(const QString& name);
    Q_INVOKABLE void kill_all();
    Q_INVOKABLE void check_version();
    bool running() const;
    bool game_path_select() const;
    bool installed() const;
    void set_log_statue(bool status);
    QString version() const;
    bool log_on() const;
Q_SIGNALS:
    void running_changed();
    void path_change();
    void log_on_changed();
    void installed_changed();
    void version_changed();

private:
    QString m_game_install_dir;
    QString m_peer_mode{"--host"};
    QString m_port{"25530"};
    QString m_ip{"127.0.0.1"};
    QString m_name{"Unknown"};
    QString m_version;
    QList<QProcess*> m_processes;
    bool m_log_on{false};
    bool m_installed{false};
};
