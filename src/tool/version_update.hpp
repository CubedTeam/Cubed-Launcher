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

public:
    VersionUpdate();

    bool has_new_version() const;
    float download_progress() const;
    QString local_version() const;
    QString remote_version() const;
    QString game_install_path() const;
    Q_INVOKABLE void check_update(const QString& onwer, const QString& repo);

    Q_INVOKABLE void download_from_github(bool use_mirror);

    Q_INVOKABLE void download_game(const QString& download_url);

    Q_INVOKABLE void set_game_dir(const QString& game_dir);

signals:

    void new_version_changed();
    void remote_version_changed();
    void download_progress_changed();
    void game_install_path_changed();

private:
    bool m_new_version{false};
    float m_download_progress = 0.0f;
    bool m_downloading{false};
    QVersionNumber m_local_version;
    QVersionNumber m_remote_version;
    QNetworkAccessManager m_manager;

    QString m_game_dir{QCoreApplication::applicationDirPath() + "/game"};

    QString buildUserAgent();

    void on_reply_finished();
};