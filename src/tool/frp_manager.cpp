#include "tool/frp_manager.hpp"

#include "settings.hpp"
#include "tool/game_path.hpp"
#include "tool/mirror.hpp"
#include "tool/user_agent.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>
#include <qmicroz.h>

namespace {
QString platform_asset_pattern() {
#ifdef _WIN32
    return QStringLiteral(R"(^frp_[\d.]+_windows_amd64\.zip$)");
#else
    return QStringLiteral(R"(^frp_[\d.]+_linux_amd64\.tar\.gz$)");
#endif
}

QString archive_extension() {
#ifdef _WIN32
    return QStringLiteral(".zip");
#else
    return QStringLiteral(".tar.gz");
#endif
}

QString frpc_binary_name() {
#ifdef _WIN32
    return QStringLiteral("frpc.exe");
#else
    return QStringLiteral("frpc");
#endif
}

QString extract_temp_dir() {
    return QDir::temp().filePath(QStringLiteral("frp_install_%1")
                                     .arg(QDateTime::currentMSecsSinceEpoch()));
}
} // namespace

FrpManager::FrpManager(QObject* parent)
    : QObject(parent), m_fetcher(&m_manager, this) {
    QString path = get_default_frp_install_dir();
    if (Settings* s = Settings::instance()) {
        const QString persisted = s->frp_install_path();
        if (!persisted.isEmpty()) {
            path = persisted;
        }
    }
    m_install_path = path;
    detect_install();
}

FrpManager::~FrpManager() {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(2000);
    }
}

bool FrpManager::installed() const {
    return QFileInfo::exists(frpc_binary()) && QFileInfo::exists(toml_path());
}

bool FrpManager::running() const {
    return m_process && m_process->state() != QProcess::NotRunning;
}

bool FrpManager::busy() const {
    return m_state == Checking || m_state == Downloading ||
           m_state == Extracting;
}

QString FrpManager::frpc_binary() const {
#ifdef _WIN32
    return m_install_path + "/frpc.exe";
#else
    return m_install_path + "/frpc";
#endif
}

QString FrpManager::toml_path() const { return m_install_path + "/frpc.toml"; }

void FrpManager::set_state(State s) {
    if (m_state == s) {
        return;
    }
    m_state = s;
    emit state_changed();
}

void FrpManager::set_error(const QString& message) {
    m_has_error = true;
    m_error_message = message;
    set_state(Error);
    emit has_error_changed();
    emit error_message_changed();
    append_log(QStringLiteral("[error] ") + message);
}

void FrpManager::clear_error() {
    if (!m_has_error && m_error_message.isEmpty()) {
        return;
    }
    m_has_error = false;
    m_error_message.clear();
    emit has_error_changed();
    emit error_message_changed();
}

void FrpManager::append_log(const QString& line) { emit log_line(line); }

void FrpManager::detect_install() {
    if (installed()) {
        set_state(Ready);
    } else {
        set_state(NotInstalled);
    }
    load_toml_into_property();
    emit installed_changed();
}

void FrpManager::load_toml_into_property() {
    QFile f(toml_path());
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_frpc_toml.clear();
        emit frpc_toml_changed();
        return;
    }
    m_frpc_toml = QString::fromUtf8(f.readAll());
    f.close();
    emit frpc_toml_changed();
}

void FrpManager::check_and_install(int mirror_index) {
    if (busy() || running()) {
        return;
    }
    clear_error();
    set_state(Checking);
    append_log(QStringLiteral("Checking frp latest release..."));

    m_fetcher.fetch("fatedier", "frp",
                    QRegularExpression(platform_asset_pattern()),
                    [this, mirror_index](GithubReleaseFetcher::Result r) {
                        on_release_fetched(mirror_index, r);
                    });
}

void FrpManager::on_release_fetched(int mirror_index,
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
    append_log(QStringLiteral("Found frp %1, downloading...").arg(m_version));
    start_download(url);
}

void FrpManager::start_download(const QString& url) {

    if (m_download_reply) {
        m_download_reply->abort();
        m_download_reply->deleteLater();
        m_download_reply = nullptr;
    }
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, buildUserAgent().toUtf8());
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    auto* reply = m_manager.get(req);
    m_download_reply = reply;

    const QString archive_path =
        QDir::temp().filePath("frp_download" + archive_extension());
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

void FrpManager::on_download_finished(const QString& archive_path) {
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

void FrpManager::extract_archive(const QString& archive_path) {
    const QString tmp_dir = extract_temp_dir();
    if (!QDir().mkpath(tmp_dir)) {
        QFile::remove(archive_path);
        set_error(QStringLiteral("Cannot create temp dir: %1").arg(tmp_dir));
        return;
    }

#ifdef _WIN32
    QMicroz zip(archive_path);
    zip.setOutputFolder(tmp_dir);
    if (!zip.extractAll()) {
        QFile::remove(archive_path);
        QDir(tmp_dir).removeRecursively();
        set_error(QStringLiteral("Failed to extract zip archive"));
        return;
    }
    QFile::remove(archive_path);
#else
    QProcess tar;
    tar.setProgram("tar");
    tar.setArguments({"xzf", archive_path, "-C", tmp_dir});
    tar.start();
    if (!tar.waitForStarted() || !tar.waitForFinished(-1)) {
        QFile::remove(archive_path);
        QDir(tmp_dir).removeRecursively();
        set_error(
            QStringLiteral("Failed to run tar: %1").arg(tar.errorString()));
        return;
    }
    QFile::remove(archive_path);
#endif

    QDir rootDir(tmp_dir);
    const auto entries = rootDir.entryList(QStringList() << "frp_*",
                                           QDir::Dirs | QDir::NoDotAndDotDot);
    if (entries.isEmpty()) {
        QDir(tmp_dir).removeRecursively();
        set_error(QStringLiteral("No frp_* directory after extraction"));
        return;
    }
    const QString inner_dir = rootDir.filePath(entries.first());
    install_frpc_binary(inner_dir, tmp_dir);
}

void FrpManager::install_frpc_binary(const QString& inner_dir,
                                     const QString& tmp_root) {
    QDir().mkpath(m_install_path);

    const QString src_binary = inner_dir + "/" + frpc_binary_name();
    if (!QFile::exists(src_binary)) {
        QDir(tmp_root).removeRecursively();
        set_error(QStringLiteral("frpc binary not found in archive"));
        return;
    }

    const QString dst_binary = frpc_binary();
    if (QFile::exists(dst_binary)) {
        QFile::remove(dst_binary);
    }
    if (!QFile::copy(src_binary, dst_binary)) {
        QDir(tmp_root).removeRecursively();
        set_error(QStringLiteral("Failed to copy frpc binary"));
        return;
    }
#ifndef _WIN32
    QFile::setPermissions(dst_binary, QFile::permissions(dst_binary) |
                                          QFile::ExeOwner | QFile::ExeGroup |
                                          QFile::ExeOther);
#endif

    // Only seed frpc.toml on first install; preserve user config on reinstall.
    const QString dst_toml = toml_path();
    if (!QFile::exists(dst_toml)) {
        const QString src_toml = inner_dir + "/frpc.toml";
        if (QFile::exists(src_toml)) {
            QFile::copy(src_toml, dst_toml);
        }
    }

    QDir(tmp_root).removeRecursively();

    if (!installed()) {
        set_error(
            QStringLiteral("Install completed but frpc/frpc.toml missing"));
        return;
    }
    append_log(QStringLiteral("Installed frpc to %1").arg(m_install_path));
    clear_error();
    set_state(Ready);
    emit installed_changed();
    load_toml_into_property();
}

Q_INVOKABLE void FrpManager::start() {
    if (running()) {
        return;
    }
    if (!installed()) {
        set_error(QStringLiteral("frpc is not installed"));
        return;
    }
    clear_error();
    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
    m_process = new QProcess(this);
    m_process->setWorkingDirectory(m_install_path);
    m_process->setProgram(frpc_binary());
    m_process->setArguments(QStringList() << "-c" << "frpc.toml");

    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        const QByteArray data = m_process->readAllStandardOutput();
        const auto lines = data.split('\n');
        for (const auto& line : lines) {
            if (line.isEmpty()) {
                continue;
            }
            append_log(QString::fromUtf8(line));
        }
    });
    connect(m_process, &QProcess::readyReadStandardError, this, [this]() {
        const QByteArray data = m_process->readAllStandardError();
        const auto lines = data.split('\n');
        for (const auto& line : lines) {
            if (line.isEmpty()) {
                continue;
            }
            append_log(QString::fromUtf8(line));
        }
    });
    connect(m_process, &QProcess::errorOccurred, this,
            [this](QProcess::ProcessError) {
                if (m_process) {
                    set_error(m_process->errorString());
                }
            });
    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this](int exitCode, QProcess::ExitStatus) {
                append_log(
                    QStringLiteral("frpc exited with code %1").arg(exitCode));
                if (m_process) {
                    m_process->deleteLater();
                    m_process = nullptr;
                }
                emit running_changed();
                if (m_state == Running) {
                    set_state(Ready);
                }
            });

    m_process->start();
    if (!m_process->waitForStarted(5000)) {
        const QString err = m_process->errorString();
        m_process->deleteLater();
        m_process = nullptr;
        set_error(QStringLiteral("Failed to start frpc: %1").arg(err));
        return;
    }
    append_log(
        QStringLiteral("frpc started (pid %1)").arg(m_process->processId()));
    set_state(Running);
    emit running_changed();
}

void FrpManager::stop() {
    if (!m_process || m_process->state() == QProcess::NotRunning) {
        return;
    }
    append_log(QStringLiteral("Stopping frpc..."));
    m_process->terminate();
    if (!m_process->waitForFinished(3000)) {
        m_process->kill();
        m_process->waitForFinished(2000);
    }
}

void FrpManager::reset_install() {
    if (running()) {
        stop();
    }
    QDir(m_install_path).removeRecursively();
    QDir().mkpath(m_install_path);
    m_version.clear();
    emit version_changed();
    m_frpc_toml.clear();
    emit frpc_toml_changed();
    clear_error();
    set_state(NotInstalled);
    emit installed_changed();
}

Q_INVOKABLE void FrpManager::set_install_path(const QString& path) {
    if (path == m_install_path) {
        return;
    }
    if (running()) {
        stop();
    }
    m_install_path = path;
    emit install_path_changed();
    detect_install();
}

Q_INVOKABLE void FrpManager::save_toml(const QString& content) {
    QDir().mkpath(m_install_path);
    QFile f(toml_path());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        set_error(QStringLiteral("Cannot write frpc.toml"));
        return;
    }
    f.write(content.toUtf8());
    f.close();
    m_frpc_toml = content;
    emit frpc_toml_changed();
    append_log(QStringLiteral("frpc.toml saved"));
}

Q_INVOKABLE QString FrpManager::read_toml() const {
    QFile f(toml_path());
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(f.readAll());
}

Q_INVOKABLE void FrpManager::install_from_url(const QString& url) {
    start_download(url);
}
