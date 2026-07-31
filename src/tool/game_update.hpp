#pragma once
#include "tool/github_release.hpp"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QQmlEngine>
#include <qtmetamacros.h>
class GameUpdate : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString gameInstallPath READ game_install_path WRITE
                   set_game_install_path NOTIFY game_install_path_changed)

    Q_PROPERTY(
        bool hasNewVersion READ has_new_version NOTIFY new_version_changed)
    Q_PROPERTY(
        QString localVersion READ local_version NOTIFY local_version_changed)
    Q_PROPERTY(
        QString remoteVersion READ remote_version NOTIFY remote_version_changed)
    Q_PROPERTY(float downloadProgress READ download_progress NOTIFY
                   download_progress_changed)
    Q_PROPERTY(
        bool downloadFinish READ download_finish NOTIFY download_finish_changed)
    Q_PROPERTY(bool hasError READ has_error NOTIFY has_error_changed)
    Q_PROPERTY(
        QString errorMessage READ error_message NOTIFY error_message_changed)
    Q_PROPERTY(
        bool checkingUpdate READ checking_update NOTIFY checking_update_changed)
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloading_changed)
public:
    GameUpdate();

    Q_INVOKABLE void check_update(const QString& local_version);
    Q_INVOKABLE void download_from_github(int mirror_index);
    Q_INVOKABLE void download_game(const QString& url);
    Q_INVOKABLE void cancel_download();
    QString game_install_path() const;

    bool has_new_version() const;
    bool checking_update() const;
    bool downloading() const;
    QString local_version() const;
    QString remote_version() const;
    float download_progress() const;
    bool download_finish() const;
    QString error_message() const;
    bool has_error() const;

    void set_game_install_path(const QString& game_dir);

signals:

    void game_install_path_changed();
    void new_version_changed();
    void remote_version_changed();
    void download_progress_changed();
    void download_finish_changed();
    void local_version_changed();
    void error_message_changed();
    void has_error_changed();
    void checking_update_changed();
    void downloading_changed();

private:
    QNetworkAccessManager m_manager;
    GithubReleaseFetcher m_fetcher;

    QString m_download_url;
    QVersionNumber m_local_version;
    QVersionNumber m_remote_version;

    bool m_new_version{false};
    float m_download_progress = 0.0f;
    bool m_downloading{false};
    bool m_download_finish{false};
    bool m_checking_update = false;
    bool m_cancelling{false};
    QString m_game_install_path;
    QNetworkReply* m_download_reply{nullptr};

    bool m_has_error{false};
    QString m_error_message;
};