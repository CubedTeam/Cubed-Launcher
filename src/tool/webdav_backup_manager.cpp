#include "tool/webdav_backup_manager.hpp"

#include "tool/backup_crypto.hpp"
#include "tool/secret_store.hpp"

#include <QUrl>
#include <algorithm>

namespace {
// AI-generated: Keep WebDAV secrets outside launcher settings.
constexpr QLatin1StringView kServerUrlKey("webdav/server_url");
constexpr QLatin1StringView kUsernameKey("webdav/username");
constexpr QLatin1StringView kPasswordKey("webdav_password");
constexpr QLatin1StringView kPassphraseKey("webdav_backup_passphrase");
constexpr qsizetype kMinimumPassphraseLength = 12;
} // namespace

WebDavBackupManager::WebDavBackupManager(QObject* parent)
    : QObject(parent), m_settings(QSettings::IniFormat, QSettings::UserScope,
                                  "Cubed", "Launcher") {
    m_server_url = m_settings.value(kServerUrlKey).toString();
    m_username = m_settings.value(kUsernameKey).toString();
    refresh_stored_state();
    connect(&m_client, &WebDavClient::completed, this,
            &WebDavBackupManager::on_client_completed);
}

WebDavBackupManager::~WebDavBackupManager() { clear_active_secrets(); }

bool WebDavBackupManager::connection_ready() const {
    return !m_server_url.trimmed().isEmpty() &&
           !m_username.trimmed().isEmpty() && m_password_stored;
}

bool WebDavBackupManager::configuration_ready() const {
    return connection_ready() && m_passphrase_stored;
}

void WebDavBackupManager::set_server_url(const QString& url) {
    const QString trimmed = url.trimmed();
    if (m_server_url == trimmed) {
        return;
    }
    m_server_url = trimmed;
    m_settings.setValue(kServerUrlKey, m_server_url);
    Q_EMIT server_url_changed();
    Q_EMIT configuration_changed();
}

void WebDavBackupManager::set_username(const QString& username) {
    const QString trimmed = username.trimmed();
    if (m_username == trimmed) {
        return;
    }
    m_username = trimmed;
    m_settings.setValue(kUsernameKey, m_username);
    Q_EMIT username_changed();
    Q_EMIT configuration_changed();
}

bool WebDavBackupManager::store_webdav_password(const QString& password) {
    if (password.isEmpty()) {
        set_error(tr("The WebDAV password cannot be empty."));
        return false;
    }
    if (!SecretStore::save(kPasswordKey, password.toUtf8())) {
        set_error(tr("Cannot save the WebDAV password in the system keyring."));
        return false;
    }
    refresh_stored_state();
    finish(ResultState::Success, tr("The WebDAV password was saved securely."));
    return true;
}

void WebDavBackupManager::clear_webdav_password() {
    if (!SecretStore::remove(kPasswordKey)) {
        set_error(
            tr("Cannot remove the WebDAV password from the system keyring."));
        return;
    }
    refresh_stored_state();
    finish(ResultState::Idle, {});
}

bool WebDavBackupManager::store_backup_passphrase(const QString& passphrase) {
    if (passphrase.size() < kMinimumPassphraseLength) {
        set_error(
            tr("The backup passphrase must contain at least 12 characters."));
        return false;
    }
    if (!SecretStore::save(kPassphraseKey, passphrase.toUtf8())) {
        set_error(
            tr("Cannot save the backup passphrase in the system keyring."));
        return false;
    }
    refresh_stored_state();
    finish(ResultState::Success,
           tr("The backup passphrase was saved securely."));
    return true;
}

void WebDavBackupManager::clear_backup_passphrase() {
    if (!SecretStore::remove(kPassphraseKey)) {
        set_error(
            tr("Cannot remove the backup passphrase from the system keyring."));
        return;
    }
    refresh_stored_state();
    finish(ResultState::Idle, {});
}

void WebDavBackupManager::test_connection() {
    if (m_busy) {
        return;
    }
    QUrl directory_url;
    if (!validate_directory_url(directory_url) || !load_credentials(false)) {
        return;
    }
    begin(Operation::TestConnection);
    if (!m_client.test_directory(directory_url, m_username,
                                 m_active_password)) {
        set_error(tr("Another WebDAV operation is already running."));
    }
}

void WebDavBackupManager::backup_identity() {
    if (m_busy) {
        return;
    }
    QUrl directory_url;
    if (!validate_directory_url(directory_url) || !load_credentials(true)) {
        return;
    }

    QByteArray identity;
    if (!m_identity_manager.read_identity_data(identity)) {
        set_error(m_identity_manager.error_message());
        return;
    }
    const BackupCrypto::Result encrypted =
        BackupCrypto::encrypt(identity, m_active_passphrase);
    std::fill(m_active_passphrase.begin(), m_active_passphrase.end(), '\0');
    m_active_passphrase.clear();
    if (!encrypted) {
        set_error(tr("Cannot encrypt the player identity for backup."));
        return;
    }

    begin(Operation::Backup);
    if (!m_client.upload(remote_file_url(directory_url), m_username,
                         m_active_password, encrypted.data)) {
        set_error(tr("Another WebDAV operation is already running."));
    }
}

void WebDavBackupManager::restore_identity() {
    if (m_busy) {
        return;
    }
    QUrl directory_url;
    if (!validate_directory_url(directory_url) || !load_credentials(true)) {
        return;
    }
    begin(Operation::Restore);
    if (!m_client.download(remote_file_url(directory_url), m_username,
                           m_active_password)) {
        set_error(tr("Another WebDAV operation is already running."));
    }
}

bool WebDavBackupManager::validate_directory_url(QUrl& directory_url) {
    if (m_server_url.trimmed().isEmpty()) {
        set_error(tr("Enter a WebDAV directory URL."));
        return false;
    }
    directory_url = QUrl(m_server_url, QUrl::StrictMode);
    if (!directory_url.isValid() ||
        directory_url.scheme().compare(QStringLiteral("https"),
                                       Qt::CaseInsensitive) != 0 ||
        directory_url.host().isEmpty() || !directory_url.userInfo().isEmpty() ||
        directory_url.hasQuery() || directory_url.hasFragment()) {
        set_error(tr("Enter a valid HTTPS WebDAV directory URL without "
                     "embedded credentials, a query, or a fragment."));
        return false;
    }
    QString path = directory_url.path();
    if (!path.endsWith(QLatin1Char('/'))) {
        path.append(QLatin1Char('/'));
        directory_url.setPath(path);
    }
    return true;
}

QUrl WebDavBackupManager::remote_file_url(const QUrl& directory_url) const {
    return directory_url.resolved(QUrl(QString::fromLatin1(kRemoteFileName)));
}

bool WebDavBackupManager::load_credentials(bool include_passphrase) {
    if (m_username.trimmed().isEmpty()) {
        set_error(tr("Enter a WebDAV username."));
        return false;
    }
    const auto password = SecretStore::load(kPasswordKey);
    if (!password || password->isEmpty()) {
        refresh_stored_state();
        set_error(tr("Save the WebDAV password before continuing."));
        return false;
    }
    m_active_password = *password;

    if (!include_passphrase) {
        return true;
    }
    const auto passphrase = SecretStore::load(kPassphraseKey);
    if (!passphrase || passphrase->isEmpty()) {
        clear_active_secrets();
        refresh_stored_state();
        set_error(tr("Save the backup passphrase before continuing."));
        return false;
    }
    m_active_passphrase = *passphrase;
    return true;
}

void WebDavBackupManager::begin(Operation operation) {
    m_busy = true;
    m_operation = operation;
    m_result_state = ResultState::Idle;
    m_status_message.clear();
    Q_EMIT busy_changed();
    Q_EMIT operation_changed();
    Q_EMIT result_changed();
}

void WebDavBackupManager::finish(ResultState state, const QString& message) {
    const bool was_busy = m_busy;
    const bool operation_was_active = m_operation != Operation::None;
    m_busy = false;
    m_operation = Operation::None;
    m_result_state = state;
    m_status_message = message;
    clear_active_secrets();
    if (was_busy) {
        Q_EMIT busy_changed();
    }
    if (operation_was_active) {
        Q_EMIT operation_changed();
    }
    Q_EMIT result_changed();
}

void WebDavBackupManager::set_error(const QString& message) {
    finish(ResultState::Error, message);
}

void WebDavBackupManager::on_client_completed(WebDavClient::Operation operation,
                                              WebDavClient::Error error,
                                              const QByteArray& contents,
                                              const QString& detail) {
    if (error != WebDavClient::Error::None) {
        set_error(client_error_message(error, detail));
        return;
    }

    if (operation == WebDavClient::Operation::Test) {
        finish(ResultState::Success, tr("The WebDAV directory is reachable."));
        return;
    }
    if (operation == WebDavClient::Operation::Upload) {
        finish(ResultState::Success,
               tr("The player identity was backed up successfully."));
        return;
    }

    const BackupCrypto::Result decrypted =
        BackupCrypto::decrypt(contents, m_active_passphrase);
    if (!decrypted) {
        if (decrypted.error == BackupCrypto::Error::AuthenticationFailed) {
            set_error(tr("The backup passphrase is incorrect or the backup was "
                         "modified."));
        } else {
            set_error(tr(
                "The remote file is not a supported Cubed identity backup."));
        }
        return;
    }
    if (!m_identity_manager.replace_identity_data(decrypted.data)) {
        set_error(m_identity_manager.error_message());
        return;
    }
    finish(ResultState::Success, tr("The player identity was restored and will "
                                    "be used next time Cubed starts."));
}

QString WebDavBackupManager::client_error_message(WebDavClient::Error error,
                                                  const QString& detail) const {
    switch (error) {
    case WebDavClient::Error::Authentication:
        return tr(
            "WebDAV authentication failed. Check the username and password.");
    case WebDavClient::Error::DirectoryMissing:
        return tr("The WebDAV directory or backup file does not exist.");
    case WebDavClient::Error::ResponseTooLarge:
        return tr("The remote backup is too large.");
    case WebDavClient::Error::Busy:
        return tr("Another WebDAV operation is already running.");
    case WebDavClient::Error::Network:
        return detail.isEmpty()
                   ? tr("The WebDAV request failed.")
                   : tr("The WebDAV request failed: %1").arg(detail);
    case WebDavClient::Error::None:
        break;
    }
    return tr("The WebDAV request failed.");
}

void WebDavBackupManager::clear_active_secrets() {
    std::fill(m_active_password.begin(), m_active_password.end(), '\0');
    std::fill(m_active_passphrase.begin(), m_active_passphrase.end(), '\0');
    m_active_password.clear();
    m_active_passphrase.clear();
}

void WebDavBackupManager::refresh_stored_state() {
    const bool old_password_stored = m_password_stored;
    const bool old_passphrase_stored = m_passphrase_stored;
    const auto password = SecretStore::load(kPasswordKey);
    const auto passphrase = SecretStore::load(kPassphraseKey);
    m_password_stored = password && !password->isEmpty();
    m_passphrase_stored = passphrase && !passphrase->isEmpty();
    if (old_password_stored != m_password_stored ||
        old_passphrase_stored != m_passphrase_stored) {
        Q_EMIT credentials_changed();
        Q_EMIT configuration_changed();
    }
}
