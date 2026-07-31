#include "tool/easytier_manager.hpp"

#include "settings.hpp"
#include "tool/game_path.hpp"
#include "tool/mirror.hpp"
#include "tool/user_agent.hpp"

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrl>
#include <qmicroz.h>

namespace {
QString platform_asset_pattern() {
#ifdef _WIN32
    return QStringLiteral(R"(^easytier-windows-x86_64-v[\d.]+\.zip$)");
#else
    return QStringLiteral(R"(^easytier-linux-x86_64-v[\d.]+\.zip$)");
#endif
}

QString core_binary_name() {
#ifdef _WIN32
    return QStringLiteral("easytier-core.exe");
#else
    return QStringLiteral("easytier-core");
#endif
}

QString extract_temp_dir() {
    return QDir::temp().filePath(QStringLiteral("easytier_install_%1")
                                     .arg(QDateTime::currentMSecsSinceEpoch()));
}
} // namespace

EasyTierManager::EasyTierManager(QObject* parent)
    : QObject(parent), m_fetcher(&m_manager, this) {
    QString path = get_default_easytier_install_dir();
    if (Settings* s = Settings::instance()) {
        const QString persisted = s->easytier_install_path();
        if (!persisted.isEmpty()) {
            path = persisted;
        }
    }
    m_install_path = path;
    detect_install();
}

EasyTierManager::~EasyTierManager() = default;

bool EasyTierManager::installed() const {
    return QFileInfo::exists(core_binary());
}

bool EasyTierManager::busy() const {
    return m_state == Checking || m_state == Downloading ||
           m_state == Extracting;
}

QString EasyTierManager::core_binary() const {
    return m_install_path + "/" + core_binary_name();
}

void EasyTierManager::set_state(State s) {
    if (m_state == s) {
        return;
    }
    m_state = s;
    emit state_changed();
}

void EasyTierManager::set_error(const QString& message) {
    m_has_error = true;
    m_error_message = message;
    set_state(Error);
    emit has_error_changed();
    emit error_message_changed();
    append_log(QStringLiteral("[error] ") + message);
}

void EasyTierManager::clear_error() {
    if (!m_has_error && m_error_message.isEmpty()) {
        return;
    }
    m_has_error = false;
    m_error_message.clear();
    emit has_error_changed();
    emit error_message_changed();
}

void EasyTierManager::append_log(const QString& line) { emit log_line(line); }

void EasyTierManager::detect_install() {
    if (installed()) {
        set_state(Ready);
    } else {
        set_state(NotInstalled);
    }
    emit installed_changed();
}

void EasyTierManager::check_and_install(int mirror_index) {
    if (busy()) {
        return;
    }
    clear_error();
    set_state(Checking);
    append_log(QStringLiteral("Checking easytier latest release..."));

    m_fetcher.fetch("EasyTier", "EasyTier",
                    QRegularExpression(platform_asset_pattern()),
                    [this, mirror_index](GithubReleaseFetcher::Result r) {
                        on_release_fetched(mirror_index, r);
                    });
}

void EasyTierManager::on_release_fetched(int mirror_index,
                                         GithubReleaseFetcher::Result r) {
    if (!r.ok) {
        set_error(r.errorMessage);
        return;
    }
    m_version = r.version;
    emit version_changed();

    QString url = r.downloadUrl;
    if (mirror_index > 0 && mirror_index < mirror_sources.size()) {
        const QString& prefix = mirror_sources.at(mirror_index).prefix;
        if (!prefix.isEmpty()) {
            url = prefix + url;
        }
    }
    append_log(
        QStringLiteral("Found easytier %1, downloading...").arg(m_version));
    start_download(url);
}

void EasyTierManager::start_download(const QString& url) {

    if (m_download_reply) {
        return;
    }
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, buildUserAgent().toUtf8());
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    auto* reply = m_manager.get(req);
    m_download_reply = reply;

    const QString archive_path =
        QDir::temp().filePath(QStringLiteral("easytier_download_%1.zip")
                                  .arg(QDateTime::currentMSecsSinceEpoch()));
    auto file = std::make_shared<QFile>(archive_path);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        reply->abort();
        reply->deleteLater();
        m_download_reply = nullptr;
        set_error(
            QStringLiteral("Cannot create temp file: %1").arg(archive_path));
        return;
    }

    m_download_progress = 0.0f;
    emit download_progress_changed();
    set_state(Downloading);

    connect(reply, &QNetworkReply::readyRead, reply,
            [reply, file]() { file->write(reply->readAll()); });
    connect(reply, &QNetworkReply::downloadProgress, this,
            [this](qint64 received, qint64 total) {
                if (total > 0) {
                    m_download_progress = float(received) / float(total);
                    emit download_progress_changed();
                }
            });
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, file, archive_path]() {
                file->close();
                m_download_reply = nullptr;
                if (reply->error() != QNetworkReply::NoError) {
                    QFile::remove(archive_path);
                    reply->deleteLater();
                    set_error(reply->errorString());
                    return;
                }
                reply->deleteLater();
                on_download_finished(archive_path);
            });
}

void EasyTierManager::on_download_finished(const QString& archive_path) {
    QFileInfo info(archive_path);
    if (info.size() < 1024) {
        QFile::remove(archive_path);
        set_error(QStringLiteral("Downloaded archive is too small (%1 bytes)")
                      .arg(info.size()));
        return;
    }
    append_log(QStringLiteral("Download finished, extracting..."));
    set_state(Extracting);
    extract_archive(archive_path);
}

void EasyTierManager::extract_archive(const QString& archive_path) {
    const QString tmp_dir = extract_temp_dir();
    if (!QDir().mkpath(tmp_dir)) {
        QFile::remove(archive_path);
        set_error(QStringLiteral("Cannot create temp dir: %1").arg(tmp_dir));
        return;
    }

    QMicroz zip(archive_path);
    zip.setOutputFolder(tmp_dir);
    if (!zip.extractAll()) {
        QFile::remove(archive_path);
        QDir(tmp_dir).removeRecursively();
        set_error(QStringLiteral("Failed to extract zip archive"));
        return;
    }
    QFile::remove(archive_path);

    QDir rootDir(tmp_dir);
    QStringList entries = rootDir.entryList(QStringList() << "easytier-*",
                                            QDir::Dirs | QDir::NoDotAndDotDot);
    QString inner_dir;
    if (!entries.isEmpty()) {
        inner_dir = rootDir.filePath(entries.first());
    } else {
        // No inner directory: files were extracted directly to tmp_dir.
        inner_dir = tmp_dir;
    }
    install_binaries(inner_dir, tmp_dir);
}

void EasyTierManager::install_binaries(const QString& inner_dir,
                                       const QString& tmp_root) {
    QDir().mkpath(m_install_path);

    QDir inner(inner_dir);
    if (!inner.exists()) {
        QDir(tmp_root).removeRecursively();
        set_error(
            QStringLiteral("Inner directory not found: %1").arg(inner_dir));
        return;
    }

    // AI-generated: archive may nest the binaries under an extra directory
    // layer. Recursively walk the extraction root and only pick out the two
    // binaries we need.
    auto find_binary = [](const QDir& root,
                          const QString& baseName) -> QString {
        const QString suffix =
#ifdef _WIN32
            QStringLiteral(".exe");
#else
            QString();
#endif
        QDirIterator it(root.absolutePath(),
                        QStringList() << (baseName + suffix) << baseName,
                        QDir::Files | QDir::NoSymLinks,
                        QDirIterator::Subdirectories);
        while (it.hasNext()) {
            return it.next();
        }
        return {};
    };

    const QString src_core =
        find_binary(inner, QStringLiteral("easytier-core"));
    const QString src_cli = find_binary(inner, QStringLiteral("easytier-cli"));
    if (src_core.isEmpty() || src_cli.isEmpty()) {
        QDir(tmp_root).removeRecursively();
        const QString missing =
            src_core.isEmpty() && src_cli.isEmpty()
                ? QStringLiteral("easytier-core and easytier-cli")
            : src_core.isEmpty() ? QStringLiteral("easytier-core")
                                 : QStringLiteral("easytier-cli");
        set_error(QStringLiteral("%1 not found in archive").arg(missing));
        return;
    }

    auto copy_binary = [&](const QString& src, const QString& dst) -> bool {
        if (QFile::exists(dst)) {
            QFile::remove(dst);
        }
        if (!QFile::copy(src, dst)) {
            return false;
        }
#ifndef _WIN32
        QFile::setPermissions(dst, QFile::permissions(dst) | QFile::ExeOwner |
                                       QFile::ExeGroup | QFile::ExeOther);
#endif
        return true;
    };

    if (!copy_binary(src_core, core_binary())) {
        QDir(tmp_root).removeRecursively();
        set_error(QStringLiteral("Failed to copy easytier-core"));
        return;
    }
#ifdef _WIN32
    const QString cli_binary = m_install_path + "/easytier-cli.exe";
#else
    const QString cli_binary = m_install_path + "/easytier-cli";
#endif
    if (!copy_binary(src_cli, cli_binary)) {
        QDir(tmp_root).removeRecursively();
        set_error(QStringLiteral("Failed to copy easytier-cli"));
        return;
    }

    QDir(tmp_root).removeRecursively();

    if (!installed()) {
        set_error(QStringLiteral(
            "Install completed but easytier-core binary missing"));
        return;
    }
    append_log(QStringLiteral("Installed easytier to %1").arg(m_install_path));
    clear_error();
    set_state(Ready);
    emit installed_changed();
}

void EasyTierManager::reset_install() {
    QDir(m_install_path).removeRecursively();
    QDir().mkpath(m_install_path);
    m_version.clear();
    emit version_changed();
    clear_error();
    set_state(NotInstalled);
    emit installed_changed();
}

Q_INVOKABLE void EasyTierManager::set_install_path(const QString& path) {
    if (path == m_install_path) {
        return;
    }
    m_install_path = path;
    emit install_path_changed();
    detect_install();
}

Q_INVOKABLE void EasyTierManager::install_from_url(const QString& url) {
    if (busy()) {
        return;
    }
    start_download(url);
}
