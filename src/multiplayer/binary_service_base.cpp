#include "multiplayer/binary_service_base.hpp"

#include "tool/log.hpp"
#include "tool/mirror.hpp"
#include "tool/user_agent.hpp"

#include <QStandardPaths>
#include <QTimer>

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <shellapi.h>
#include <thread>
// clang-format on
#endif

BinaryServiceBase::BinaryServiceBase(QStringView name, QObject* parent)
    : QObject(parent), m_fetcher(&m_manager, name, this) {}

BinaryServiceBase::~BinaryServiceBase() {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(2000);
    }
    delete m_process;
    m_process = nullptr;
#ifdef _WIN32
    if (m_elevated_poll_timer) {
        m_elevated_poll_timer->stop();
    }
    if (m_elevated_handle) {
        HANDLE h = reinterpret_cast<HANDLE>(m_elevated_handle);
        TerminateProcess(h, 1);
        WaitForSingleObject(h, 2000);
        CloseHandle(h);
        m_elevated_handle = nullptr;
        m_elevated_pid = 0;
    }
#endif
}

bool BinaryServiceBase::installed() const { return is_installed_impl(); }

bool BinaryServiceBase::running() const {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        return true;
    }
#ifdef _WIN32
    if (m_elevated_handle) {
        DWORD code = 0;
        if (GetExitCodeProcess(reinterpret_cast<HANDLE>(m_elevated_handle),
                               &code)) {
            return code == STILL_ACTIVE;
        }
    }
#endif
    return false;
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
    if (m_process && m_process->state() != QProcess::NotRunning) {
        append_log(QStringLiteral("Stopping %1...").arg(process_log_name()));
        m_process->terminate();
        if (!m_process->waitForFinished(3000)) {
            m_process->kill();
            m_process->waitForFinished(2000);
        }
    }
#ifdef _WIN32
    if (m_elevate_pending) {
        // AI-generated: user requested stop while the UAC dialog is still up.
        // The worker thread will check this flag when ShellExecuteEx returns
        // and immediately terminate the just-created process.
        m_elevate_stop_requested = true;
        return;
    }
    if (m_elevated_handle) {
        HANDLE h = reinterpret_cast<HANDLE>(m_elevated_handle);
        append_log(QStringLiteral("Stopping %1...").arg(process_log_name()));
        if (m_elevated_poll_timer) {
            m_elevated_poll_timer->stop();
        }
        TerminateProcess(h, 1);
        WaitForSingleObject(h, 3000);
        CloseHandle(h);
        m_elevated_handle = nullptr;
        m_elevated_pid = 0;
        Q_EMIT running_changed();
        if (m_state == Running) {
            set_state(Ready);
        }
    }
#endif
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
                                       const QProcessEnvironment& env,
                                       bool elevate) {
    if (running()) {
        return;
    }
    if (!installed()) {
        set_error(
            QStringLiteral("%1 is not installed").arg(process_log_name()));
        return;
    }
    clear_error();

    // AI-generated: when elevate is requested, route the launch through a
    // privilege-elevation helper so the OS prompts the user for credentials
    // and runs the target with elevated rights. The launcher itself stays
    // unprivileged.
    //   - Linux: `pkexec` + polkit authentication dialog.
    //   - Windows: `ShellExecuteEx` with the `runas` verb + UAC consent
    //     prompt. The actual launch runs on a worker thread because UAC
    //     blocks the calling thread.
    QString effective_program = program;
    QStringList effective_arguments = arguments;
    if (elevate) {
#ifdef _WIN32
        launch_elevated_windows(program, arguments);
        return;
#else
        if (QStandardPaths::findExecutable(QStringLiteral("pkexec"))
                .isEmpty()) {
            set_error(QStringLiteral(
                "pkexec not found; install polkit/policykit to run as root"));
            return;
        }
        effective_program = QStringLiteral("pkexec");
        effective_arguments.prepend(program);
#endif
    }

    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
    m_process = new QProcess(this);
    m_process->setWorkingDirectory(m_install_path);
    m_process->setProgram(effective_program);
    m_process->setArguments(effective_arguments);
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
    on_process_started();
}

#ifdef _WIN32

// AI-generated: quote a single argument for the ShellExecuteExW lpParameters
// string. Wraps in double quotes only when the argument contains whitespace
// or a double quote; embedded double quotes are escaped with a backslash.
// This is a pragmatic subset of the CommandLineToArgvW rules that covers
// network names/secrets with spaces.
static QString quoteWinArg(const QString& arg) {
    if (arg.isEmpty()) {
        return QStringLiteral("\"\"");
    }
    const bool needsQuote = arg.contains(QLatin1Char(' ')) ||
                            arg.contains(QLatin1Char('\t')) ||
                            arg.contains(QLatin1Char('"'));
    if (!needsQuote) {
        return arg;
    }
    QString escaped = arg;
    escaped.replace(QLatin1Char('"'), QLatin1String("\\\""));
    return QLatin1Char('"') + escaped + QLatin1Char('"');
}

void BinaryServiceBase::launch_elevated_windows(const QString& program,
                                                const QStringList& arguments) {
    if (m_elevate_pending || m_elevated_handle) {
        set_error(QStringLiteral("%1 is already starting/running")
                      .arg(process_log_name()));
        return;
    }
    clear_error();

    QString params;
    for (const QString& arg : arguments) {
        if (!params.isEmpty()) {
            params += QLatin1Char(' ');
        }
        params += quoteWinArg(arg);
    }

    const std::wstring file = program.toStdWString();
    const std::wstring dir = m_install_path.toStdWString();
    const std::wstring args = params.toStdWString();

    m_elevate_pending = true;
    m_elevate_stop_requested = false;

    // AI-generated: ShellExecuteExW with the `runas` verb blocks the calling
    // thread until the UAC prompt is answered. Run it on a detached worker
    // thread so the Qt event loop keeps spinning and the UI stays
    // responsive. COM is initialized on the worker thread because Shell
    // APIs require it.
    std::thread([this, file, dir, args]() {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        SHELLEXECUTEINFOW sei{};
        sei.cbSize = sizeof(sei);
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.lpVerb = L"runas";
        sei.lpFile = file.c_str();
        sei.lpParameters = args.c_str();
        sei.lpDirectory = dir.c_str();
        sei.nShow = SW_HIDE;
        const BOOL ok = ShellExecuteExW(&sei);
        const DWORD err = ok ? ERROR_SUCCESS : GetLastError();
        void* handle = ok ? reinterpret_cast<void*>(sei.hProcess) : nullptr;
        const qint64 pid =
            ok ? static_cast<qint64>(GetProcessId(sei.hProcess)) : 0;
        CoUninitialize();

        QMetaObject::invokeMethod(
            this,
            [this, ok, err, handle, pid]() {
                if (ok) {
                    if (m_elevate_stop_requested) {
                        // User clicked Stop while the UAC dialog was up;
                        // terminate the just-created process immediately.
                        if (handle) {
                            HANDLE h = reinterpret_cast<HANDLE>(handle);
                            TerminateProcess(h, 1);
                            CloseHandle(h);
                        }
                        m_elevate_pending = false;
                        return;
                    }
                    m_elevate_pending = false;
                    m_elevated_handle = handle;
                    m_elevated_pid = pid;
                    append_log(QStringLiteral("%1 started elevated (pid %2)")
                                   .arg(process_log_name())
                                   .arg(pid));
                    start_elevated_polling();
                    set_state(Running);
                    Q_EMIT running_changed();
                    on_process_started();
                } else {
                    m_elevate_pending = false;
                    set_error(
                        QStringLiteral("Failed to start %1 elevated (error %2)")
                            .arg(process_log_name())
                            .arg(err));
                }
            },
            Qt::QueuedConnection);
    }).detach();
}

void BinaryServiceBase::start_elevated_polling() {
    if (!m_elevated_poll_timer) {
        m_elevated_poll_timer = new QTimer(this);
        m_elevated_poll_timer->setInterval(1000);
        connect(m_elevated_poll_timer, &QTimer::timeout, this, [this]() {
            if (!m_elevated_handle) {
                return;
            }
            DWORD code = 0;
            HANDLE h = reinterpret_cast<HANDLE>(m_elevated_handle);
            const BOOL gotCode = GetExitCodeProcess(h, &code);
            if (!gotCode || code != STILL_ACTIVE) {
                m_elevated_poll_timer->stop();
                CloseHandle(h);
                m_elevated_handle = nullptr;
                m_elevated_pid = 0;
                append_log(QStringLiteral("%1 exited with code %2")
                               .arg(process_log_name())
                               .arg(static_cast<int>(code)));
                Q_EMIT running_changed();
                if (m_state == Running) {
                    set_state(Ready);
                }
                on_process_finished(static_cast<int>(code));
            }
        });
    }
    m_elevated_poll_timer->start();
}

#endif // _WIN32
