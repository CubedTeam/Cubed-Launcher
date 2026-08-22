#include "tool/identity_manager.hpp"

#include "tool/standard_paths.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSaveFile>

namespace {
QString default_data_directory() {
    const Cubed::StandardPaths paths("Cubed");
    const std::filesystem::path data_directory =
        paths.ensure(Cubed::StandardPaths::Location::DATA);
#ifdef _WIN32
    return QString::fromStdWString(data_directory.native());
#else
    return QString::fromUtf8(data_directory.native());
#endif
}
} // namespace

IdentityManager::IdentityManager(QObject* parent) : QObject(parent) {
    try {
        m_identity_path =
            QDir(default_data_directory()).filePath("identity.json");
    } catch (const std::exception& error) {
        set_error(tr("Cannot determine the Cubed data directory: %1")
                      .arg(QString::fromUtf8(error.what())));
    }
}

IdentityManager::IdentityManager(const QString& data_directory, QObject* parent)
    : QObject(parent),
      m_identity_path(QDir(data_directory).filePath("identity.json")) {}

bool IdentityManager::import_identity(const QUrl& source) {
    set_error({});
    if (!has_identity_path()) {
        return false;
    }
    if (!source.isLocalFile()) {
        set_error(tr("The selected path is not a local file."));
        return false;
    }

    QByteArray contents;
    if (!read_identity(source.toLocalFile(), contents)) {
        return false;
    }

    return replace_identity_data(contents);
}

bool IdentityManager::read_identity_data(QByteArray& contents) {
    set_error({});
    if (!has_identity_path() || !read_identity(m_identity_path, contents)) {
        return false;
    }
    return validate_identity(contents);
}

bool IdentityManager::replace_identity_data(const QByteArray& contents) {
    set_error({});
    if (!has_identity_path() || !validate_identity(contents)) {
        return false;
    }

    const QString data_directory = QFileInfo(m_identity_path).absolutePath();
    if (!QDir().mkpath(data_directory)) {
        set_error(tr("Cannot create the Cubed data directory."));
        return false;
    }
    return write_identity(m_identity_path, contents);
}

bool IdentityManager::export_identity(const QUrl& destination) {
    set_error({});
    if (!has_identity_path()) {
        return false;
    }
    if (!destination.isLocalFile()) {
        set_error(tr("The selected path is not a local file."));
        return false;
    }

    QByteArray contents;
    if (!read_identity(m_identity_path, contents)) {
        return false;
    }

    return write_identity(destination.toLocalFile(), contents);
}

bool IdentityManager::has_identity_path() {
    if (!m_identity_path.isEmpty()) {
        return true;
    }
    set_error(tr("The Cubed identity path is unavailable."));
    return false;
}

bool IdentityManager::validate_identity(const QByteArray& contents) {
    QJsonParseError parse_error;
    const QJsonDocument document =
        QJsonDocument::fromJson(contents, &parse_error);
    if (parse_error.error == QJsonParseError::NoError && document.isObject()) {
        return true;
    }
    set_error(tr("The selected file is not a valid identity JSON object."));
    return false;
}

bool IdentityManager::read_identity(const QString& path, QByteArray& contents) {
    const QFileInfo file_info(path);
    if (!file_info.isFile()) {
        set_error(tr("The identity file does not exist or is not a regular "
                     "file."));
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        set_error(
            tr("Cannot read the identity file: %1").arg(file.errorString()));
        return false;
    }

    contents = file.readAll();
    if (file.error() != QFileDevice::NoError) {
        set_error(
            tr("Cannot read the identity file: %1").arg(file.errorString()));
        return false;
    }
    return true;
}

bool IdentityManager::write_identity(const QString& path,
                                     const QByteArray& contents) {
    // AI-generated: Keep authentication files intact when a write fails.
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        set_error(
            tr("Cannot write the identity file: %1").arg(file.errorString()));
        return false;
    }

    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    if (file.write(contents) != contents.size()) {
        set_error(
            tr("Cannot write the identity file: %1").arg(file.errorString()));
        file.cancelWriting();
        return false;
    }

    if (!file.commit()) {
        set_error(tr("Cannot finish writing the identity file: %1")
                      .arg(file.errorString()));
        return false;
    }
    return true;
}

void IdentityManager::set_error(const QString& message) {
    if (m_error_message == message) {
        return;
    }
    m_error_message = message;
    Q_EMIT error_message_changed();
}
