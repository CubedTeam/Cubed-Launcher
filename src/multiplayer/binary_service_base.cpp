#include "multiplayer/binary_service_base.hpp"

#include "tool/log.hpp"
#include "tool/mirror.hpp"
#include "tool/user_agent.hpp"

#include <QStandardPaths>

BinaryServiceBase::BinaryServiceBase(QStringView name, QObject* parent)
    : QObject(parent), m_fetcher(&m_manager, name, this) {}

BinaryServiceBase::~BinaryServiceBase() {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(2000);
    }
    delete m_process;
    m_process = nullptr;
}

bool BinaryServiceBase::installed() const { return is_installed_impl(); }

bool BinaryServiceBase::running() const {
    return m_process && m_process->state() != QProcess::NotRunning;
}

bool BinaryServiceBase::busy() const {
    return m_state == Checking || m_state == Downloading ||
           m_state == Extracting;
}

QString BinaryServiceBase::temp_archive_path() const {
    return QDir::temp().filePath(QStringLiteral("%1_%2%3").arg(
        archive_filename_prefix(),
        QString::number(QDateTime::currentMSecsSinceEpoch()),
        archive_extension()));
}

QString BinaryServiceBase::extract_temp_dir() const {
    return QDir::temp().filePath(
        QStringLiteral("%1_install_%2")
            .arg(archive_filename_prefix(),
                 QString::number(QDateTime::currentMSecsSinceEpoch())));
}

void BinaryServiceBase::set_state(State s) {
    if (m_state == s) {
        return;
    }
    m_state = s;
    Q_EMIT state_changed();
}

void BinaryServiceBase::set_error(const QString& message) {
    m_has_error = true;
    m_error_message = message;
    set_state(Error);
    Q_EMIT has_error_changed();
    Q_EMIT error_message_changed();
    append_log(QStringLiteral("[error] ") + message);
}

void BinaryServiceBase::clear_error() {
    if (!m_has_error && m_error_message.isEmpty()) {
        return;
    }
    m_has_error = false;
    m_error_message.clear();
    Q_EMIT has_error_changed();
    Q_EMIT error_message_changed();
}

void BinaryServiceBase::append_log(const QString& line) {
    Q_EMIT log_line(line);
}

void BinaryServiceBase::detect_install() {
    if (installed()) {
        set_state(Ready);
    } else {
        set_state(NotInstalled);
    }
    Q_EMIT installed_changed();
    on_detect_install();
}

void BinaryServiceBase::check_and_install(int mirror_index) {
    if (busy() || running()) {
        return;
    }
    clear_error();
    set_state(Checking);
    append_log(
        QStringLiteral("Checking %1 latest release...").arg(service_name()));

    m_fetcher.fetch(repo_owner(), repo_name(), platform_asset_pattern(),
                    [this, mirror_index](GithubReleaseFetcher::Result r) {
                        on_release_fetched(mirror_index, r);
                    });
}

void BinaryServiceBase::on_release_fetched(int mirror_index,
                                           GithubReleaseFetcher::Result r) {
    if (!r.ok) {
        set_error(r.errorMessage);
        return;
    }
    m_version = r.version;
    Logger::info("{} Version: {}", service_name().toStdString(),
                 m_version.toStdString());
    Q_EMIT version_changed();

    QString url = r.downloadUrl;
    if (mirror_index > 0 && mirror_index < mirror_sources.size()) {
        const QString& prefix = mirror_sources.at(mirror_index).prefix;
        if (!prefix.isEmpty()) {
            url = prefix + url;
        }
    }
    append_log(QStringLiteral("Found %1 %2, downloading...")
                   .arg(release_name(), m_version));
    start_download(url);
}

void BinaryServiceBase::start_download(const QString& url) {
    if (m_download_reply) {
        return;
    }
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, buildUserAgent().toUtf8());
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    auto* reply = m_manager.get(req);
    m_download_reply = reply;

    const QString archive_path = temp_archive_path();
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
    Q_EMIT download_progress_changed();
    set_state(Downloading);

    connect(reply, &QNetworkReply::readyRead, reply,
            [reply, file]() { file->write(reply->readAll()); });
    connect(reply, &QNetworkReply::downloadProgress, this,
            [this](qint64 received, qint64 total) {
                if (total > 0) {
                    m_download_progress = float(received) / float(total);
                    Q_EMIT download_progress_changed();
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

void BinaryServiceBase::on_download_finished(const QString& archive_path) {
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

void BinaryServiceBase::extract_archive(const QString& archive_path) {
    const QString tmp_dir = extract_temp_dir();
    if (!QDir().mkpath(tmp_dir)) {
        QFile::remove(archive_path);
        set_error(QStringLiteral("Cannot create temp dir: %1").arg(tmp_dir));
        return;
    }

    const QString failure = extract_archive_impl(archive_path, tmp_dir);
    if (!failure.isEmpty()) {
        QFile::remove(archive_path);
        QDir(tmp_dir).removeRecursively();
        set_error(failure);
        return;
    }
    QFile::remove(archive_path);

    QDir rootDir(tmp_dir);
    QString inner_dir = tmp_dir;
    const auto patterns = inner_dir_patterns();
    for (const QString& pattern : patterns) {
        const auto entries = rootDir.entryList(
            QStringList() << pattern, QDir::Dirs | QDir::NoDotAndDotDot);
        if (!entries.isEmpty()) {
            inner_dir = rootDir.filePath(entries.first());
            break;
        }
    }
    install_binaries_impl(inner_dir, tmp_dir);
}

void BinaryServiceBase::install_from_url(const QString& url) {
    if (busy() || running()) {
        return;
    }
    start_download(url);
}

void BinaryServiceBase::set_install_path(const QString& path) {
    if (path == m_install_path) {
        return;
    }
    if (running()) {
        stop_process();
    }
    m_install_path = path;
    Q_EMIT install_path_changed();
    detect_install();
}

void BinaryServiceBase::reset_install() {
    if (running()) {
        stop_process();
    }
    QDir(m_install_path).removeRecursively();
    QDir().mkpath(m_install_path);
    m_version.clear();
    Q_EMIT version_changed();
    reset_install_extra();
    clear_error();
    set_state(NotInstalled);
    Q_EMIT installed_changed();
}

void BinaryServiceBase::stop_process() {
    if (!m_process || m_process->state() == QProcess::NotRunning) {
        return;
    }
    append_log(QStringLiteral("Stopping %1...").arg(process_log_name()));
    m_process->terminate();
    if (!m_process->waitForFinished(3000)) {
        m_process->kill();
        m_process->waitForFinished(2000);
    }
}

void BinaryServiceBase::wire_process(QProcess* p) {
    connect(p, &QProcess::readyReadStandardOutput, this, [this, p]() {
        const QByteArray data = p->readAllStandardOutput();
        const auto lines = data.split('\n');
        for (const auto& line : lines) {
            if (line.isEmpty()) {
                continue;
            }
            append_log(QString::fromUtf8(line));
        }
    });
    connect(p, &QProcess::readyReadStandardError, this, [this, p]() {
        const QByteArray data = p->readAllStandardError();
        const auto lines = data.split('\n');
        for (const auto& line : lines) {
            if (line.isEmpty()) {
                continue;
            }
            append_log(QString::fromUtf8(line));
        }
    });
    connect(p, &QProcess::errorOccurred, this,
            [this, p](QProcess::ProcessError) {
                if (p) {
                    set_error(p->errorString());
                }
            });
    connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, p](int exitCode, QProcess::ExitStatus) {
                append_log(QStringLiteral("%1 exited with code %2")
                               .arg(process_log_name())
                               .arg(exitCode));
                if (m_process == p) {
                    m_process = nullptr;
                }
                p->deleteLater();
                Q_EMIT running_changed();
                if (m_state == Running) {
                    set_state(Ready);
                }
                on_process_finished(exitCode);
            });
}

void BinaryServiceBase::launch_process(const QString& program,
                                       const QStringList& arguments,
                                       const QProcessEnvironment& env) {
    if (running()) {
        return;
    }
    if (!installed()) {
        set_error(
            QStringLiteral("%1 is not installed").arg(process_log_name()));
        return;
    }
    clear_error();

    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
    m_process = new QProcess(this);
    m_process->setWorkingDirectory(m_install_path);
    m_process->setProgram(program);
    m_process->setArguments(arguments);
    m_process->setProcessEnvironment(env);

    wire_process(m_process);

    m_process->start();
    if (!m_process->waitForStarted(5000)) {
        const QString err = m_process->errorString();
        m_process->deleteLater();
        m_process = nullptr;
        set_error(QStringLiteral("Failed to start %1: %2")
                      .arg(process_log_name(), err));
        return;
    }
    append_log(QStringLiteral("%1 started (pid %2)")
                   .arg(process_log_name())
                   .arg(m_process->processId()));
    set_state(Running);
    Q_EMIT running_changed();
}
