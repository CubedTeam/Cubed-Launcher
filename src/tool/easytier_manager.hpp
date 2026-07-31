#pragma once

#include "tool/github_release.hpp"

#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QProcess>
#include <QQmlEngine>
#include <QString>
#include <QStringList>
#include <QTimer>

class EasyTierManager : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(State state READ state NOTIFY state_changed)
    Q_PROPERTY(
        QString installPath READ install_path NOTIFY install_path_changed)
    Q_PROPERTY(bool installed READ installed NOTIFY installed_changed)
    Q_PROPERTY(bool running READ running NOTIFY running_changed)
    Q_PROPERTY(float downloadProgress READ download_progress NOTIFY
                   download_progress_changed)
    Q_PROPERTY(QString version READ version NOTIFY version_changed)
    Q_PROPERTY(
        QString errorMessage READ error_message NOTIFY error_message_changed)
    Q_PROPERTY(bool hasError READ has_error NOTIFY has_error_changed)
    Q_PROPERTY(bool busy READ busy NOTIFY state_changed)
    Q_PROPERTY(QString virtualIp READ virtual_ip NOTIFY virtual_ip_changed)

public:
    enum State {
        NotInstalled = 0,
        Checking,
        Downloading,
        Extracting,
        Ready,
        Running,
        Error
    };
    Q_ENUM(State)

    explicit EasyTierManager(QObject* parent = nullptr);
    ~EasyTierManager() override;

    State state() const { return m_state; }
    QString install_path() const { return m_install_path; }
    bool installed() const;
    bool running() const;
    float download_progress() const { return m_download_progress; }
    QString version() const { return m_version; }
    QString error_message() const { return m_error_message; }
    bool has_error() const { return m_has_error; }
    bool busy() const;
    QString virtual_ip() const { return m_virtual_ip; }

    Q_INVOKABLE void check_and_install(int mirror_index);
    Q_INVOKABLE void reset_install();
    Q_INVOKABLE void install_from_url(const QString& url);
    Q_INVOKABLE void set_install_path(const QString& path);
    Q_INVOKABLE void start(const QString& network_name,
                           const QString& network_secret,
                           const QString& peer_address);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void refresh_virtual_ip();
    Q_INVOKABLE void copy_to_clipboard(const QString& text);

signals:
    void state_changed();
    void install_path_changed();
    void installed_changed();
    void running_changed();
    void download_progress_changed();
    void version_changed();
    void error_message_changed();
    void has_error_changed();
    void virtual_ip_changed();
    void log_line(const QString& line);

private:
    void detect_install();
    void set_state(State s);
    void set_error(const QString& message);
    void clear_error();
    void append_log(const QString& line);
    void on_release_fetched(int mirror_index, GithubReleaseFetcher::Result r);
    void start_download(const QString& url);
    void on_download_finished(const QString& archive_path);
    void extract_archive(const QString& archive_path);
    void install_binaries(const QString& inner_dir, const QString& tmp_root);
    void start_ip_polling();
    void stop_ip_polling();
    void on_ip_poll_timeout();
    void parse_virtual_ip(const QByteArray& data);

    QString core_binary() const;
    QString cli_binary() const;

    QNetworkAccessManager m_manager;
    GithubReleaseFetcher m_fetcher;
    QPointer<QNetworkReply> m_download_reply;

    State m_state{NotInstalled};
    QString m_install_path;
    QString m_version;
    float m_download_progress{0.0f};
    QString m_error_message;
    bool m_has_error{false};
    QString m_virtual_ip;
    QProcess* m_process{nullptr};
    QTimer* m_ip_poll_timer{nullptr};
};
