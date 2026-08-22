#pragma once

#include "tool/identity_manager.hpp"
#include "tool/webdav_client.hpp"

#include <QObject>
#include <QQmlEngine>
#include <QSettings>

class WebDavBackupManager : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString serverUrl READ server_url WRITE set_server_url NOTIFY
                   server_url_changed)
    Q_PROPERTY(QString username READ username WRITE set_username NOTIFY
                   username_changed)
    Q_PROPERTY(
        bool passwordStored READ password_stored NOTIFY credentials_changed)
    Q_PROPERTY(
        bool passphraseStored READ passphrase_stored NOTIFY credentials_changed)
    Q_PROPERTY(
        bool connectionReady READ connection_ready NOTIFY configuration_changed)
    Q_PROPERTY(bool configurationReady READ configuration_ready NOTIFY
                   configuration_changed)
    Q_PROPERTY(bool busy READ busy NOTIFY busy_changed)
    Q_PROPERTY(Operation operation READ operation NOTIFY operation_changed)
    Q_PROPERTY(ResultState resultState READ result_state NOTIFY result_changed)
    Q_PROPERTY(QString statusMessage READ status_message NOTIFY result_changed)

public:
    enum class Operation { None, TestConnection, Backup, Restore };
    Q_ENUM(Operation)

    enum class ResultState { Idle, Success, Error };
    Q_ENUM(ResultState)

    explicit WebDavBackupManager(QObject* parent = nullptr);
    ~WebDavBackupManager() override;

    QString server_url() const { return m_server_url; }
    QString username() const { return m_username; }
    bool password_stored() const { return m_password_stored; }
    bool passphrase_stored() const { return m_passphrase_stored; }
    bool connection_ready() const;
    bool configuration_ready() const;
    bool busy() const { return m_busy; }
    Operation operation() const { return m_operation; }
    ResultState result_state() const { return m_result_state; }
    QString status_message() const { return m_status_message; }

    void set_server_url(const QString& url);
    void set_username(const QString& username);

    Q_INVOKABLE bool store_webdav_password(const QString& password);
    Q_INVOKABLE void clear_webdav_password();
    Q_INVOKABLE bool store_backup_passphrase(const QString& passphrase);
    Q_INVOKABLE void clear_backup_passphrase();
    Q_INVOKABLE void test_connection();
    Q_INVOKABLE void backup_identity();
    Q_INVOKABLE void restore_identity();

Q_SIGNALS:
    void server_url_changed();
    void username_changed();
    void credentials_changed();
    void configuration_changed();
    void busy_changed();
    void operation_changed();
    void result_changed();

private:
    static constexpr QLatin1StringView kRemoteFileName{
        "cubed-identity-backup.dat"};

    bool validate_directory_url(QUrl& directory_url);
    QUrl remote_file_url(const QUrl& directory_url) const;
    bool load_credentials(bool include_passphrase);
    void begin(Operation operation);
    void finish(ResultState state, const QString& message);
    void set_error(const QString& message);
    void on_client_completed(WebDavClient::Operation operation,
                             WebDavClient::Error error,
                             const QByteArray& contents, const QString& detail);
    QString client_error_message(WebDavClient::Error error,
                                 const QString& detail) const;
    void clear_active_secrets();
    void refresh_stored_state();

    QSettings m_settings;
    WebDavClient m_client;
    IdentityManager m_identity_manager;
    QString m_server_url;
    QString m_username;
    bool m_password_stored{false};
    bool m_passphrase_stored{false};
    bool m_busy{false};
    Operation m_operation{Operation::None};
    ResultState m_result_state{ResultState::Idle};
    QString m_status_message;
    QByteArray m_active_password;
    QByteArray m_active_passphrase;
};
