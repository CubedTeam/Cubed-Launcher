#pragma once

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QQmlEngine>
#include <qmicroz.h>
class VersionUpdate : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(
        bool hasNewVersion READ has_new_version NOTIFY new_version_changed)
    Q_PROPERTY(QString localVersion READ local_version CONSTANT)
    Q_PROPERTY(
        QString remoteVersion READ remote_version NOTIFY remote_version_changed)
    Q_PROPERTY(float downloadProgress READ download_progress NOTIFY
                   download_progress_changed)
    Q_PROPERTY(QString gameInstallPath READ game_install_path NOTIFY
                   game_install_path_changed)
    Q_PROPERTY(
        bool downloadFinish READ download_finish NOTIFY download_finish_changed)

    Q_PROPERTY(
        bool launcherFinish READ launcher_finish NOTIFY launcher_finish_changed)

    Q_PROPERTY(float launcherProgress READ launcher_progress NOTIFY
                   launcher_progress_changed)

public:
    VersionUpdate();

    bool has_new_version() const;
    bool download_finish() const;
    float download_progress() const;
    QString local_version() const;
    QString remote_version() const;
    QString game_install_path() const;

    bool launcher_finish() const;
    float launcher_progress() const;

    Q_INVOKABLE void check_update(const QString& onwer, const QString& repo);

    Q_INVOKABLE void download_from_github(bool use_mirror);

    Q_INVOKABLE void download_game(const QString& download_url);

    Q_INVOKABLE void set_game_dir(const QString& game_dir);

    Q_INVOKABLE void update_launcher(bool use_mirror);

signals:

    void new_version_changed();
    void remote_version_changed();
    void download_progress_changed();
    void download_finish_changed();
    void game_install_path_changed();
    void launcher_finish_changed();
    void launcher_progress_changed();

private:
    bool m_new_version{false};
    float m_download_progress = 0.0f;
    bool m_downloading{false};
    bool m_download_finish{false};

    float m_launcher_progress = 0.0f;
    bool m_launcher_downloading{false};
    bool m_launcher_finish{false};

    QVersionNumber m_local_version;
    QVersionNumber m_remote_version;
    QNetworkAccessManager m_manager;

    QString m_game_dir;
    QString m_latest_launcher_link;
    QString buildUserAgent();
};