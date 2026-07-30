#pragma once

#include "tool/github_release.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <qmicroz.h>
class LauncherUpdate : public QObject {
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
    Q_PROPERTY(
        bool downloadFinish READ download_finish NOTIFY download_finish_changed)
    Q_PROPERTY(bool hasError READ has_error NOTIFY has_error_changed)
    Q_PROPERTY(
        QString errorMessage READ error_message NOTIFY error_message_changed)
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloading_changed)

public:
    LauncherUpdate();

    bool has_new_version() const;
    bool download_finish() const;
    float download_progress() const;
    QString local_version() const;
    QString remote_version() const;
    QString error_message() const;
    bool has_error() const;

    Q_INVOKABLE void check_update(const QString& onwer, const QString& repo);

    Q_INVOKABLE void update_launcher(int mirror_index);

    Q_INVOKABLE void cancel_download();

    bool downloading() const;
signals:
    void new_version_changed();
    void remote_version_changed();
    void download_progress_changed();
    void download_finish_changed();
    void game_install_path_changed();
    void error_message_changed();
    void has_error_changed();
    void downloading_changed();

private:
    bool m_new_version{false};
    float m_download_progress = 0.0f;
    bool m_downloading{false};
    bool m_download_finish{false};
    bool m_cancelling{false};

    QVersionNumber m_local_version;
    QVersionNumber m_remote_version;
    QNetworkAccessManager m_manager;
    GithubReleaseFetcher m_fetcher;
    QPointer<QNetworkReply> m_download_reply;

    QString m_latest_launcher_link;

    bool m_has_error{false};
    QString m_error_message;
};