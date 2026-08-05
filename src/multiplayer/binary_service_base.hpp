#pragma once

#include "tool/github_release.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QPointer>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

class QTimer;

#ifdef _WIN32
#include <atomic>
#include <memory>
#endif

#ifdef _WIN32
// AI-generated: result buffer for the elevated (UAC) launch path. The worker
// thread writes this struct and signals completion via `done`; the main
// thread consumes it from the poll timer. `handle` ownership is transferred
// to the manager on successful consumption (the manager sets it to null so
// the destructor here does not double-close).
struct ElevatedLaunchState {
    std::atomic<bool> done{false};
    bool ok{false};
    unsigned long error{0};
    qint64 pid{0};
    void* handle{nullptr};
    ~ElevatedLaunchState();
};
#endif

// AI-generated: shared base for "fetch GitHub release -> download archive ->
// extract -> install binaries -> run process" services. FrpManager and
// EasyTierManager share the same state machine and process management;
// subclasses only override the install-specific hooks.
class BinaryServiceBase : public QObject {
    Q_OBJECT

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

    explicit BinaryServiceBase(QStringView name, QObject* parent = nullptr);
    ~BinaryServiceBase() override;

    State state() const { return m_state; }
    QString install_path() const { return m_install_path; }
    bool installed() const;
    bool running() const;
    float download_progress() const { return m_download_progress; }
    QString version() const { return m_version; }
    QString error_message() const { return m_error_message; }
    bool has_error() const { return m_has_error; }
    bool busy() const;

    Q_INVOKABLE void check_and_install(int mirror_index);
    Q_INVOKABLE void install_from_url(const QString& url);
    Q_INVOKABLE void set_install_path(const QString& path);
    Q_INVOKABLE void reset_install();
    Q_INVOKABLE void stop_process();

    // True while the managed process was launched with elevation
    // (Linux pkexec path). Reset on stop / natural exit. Used to decide
    // whether the stop sequence needs a privileged cleanup.
    bool was_elevated() const { return m_launch_elevated; }

    // Launch the managed process with the given program and arguments.
    // Subclasses expose their own Q_INVOKABLE start() that gathers
    // arguments and forwards here.
    // When elevate is true:
    //   - On Linux the program is launched through `pkexec`, which triggers
    //     the system polkit authentication dialog and runs the target as
    //     root. The launcher itself stays unprivileged.
    //   - On Windows the program is launched through `ShellExecuteEx` with
    //     the `runas` verb, which triggers the UAC consent prompt and runs
    //     the target as administrator. The UAC dialog blocks the calling
    //     thread, so the call is dispatched to a worker thread to keep the
    //     UI responsive. The returned process handle is tracked for stop()
    //     and exit detection. easytier-core's console output is not
    //     captured on this path (acceptable, see plan).
    void launch_process(const QString& program, const QStringList& arguments,
                        const QProcessEnvironment& env, bool elevate = false);

Q_SIGNALS:
    void state_changed();
    void install_path_changed();
    void installed_changed();
    void running_changed();
    void download_progress_changed();
    void version_changed();
    void error_message_changed();
    void has_error_changed();
    void log_line(const QString& line);

protected:
    // --- Service identity / release lookup ---
    virtual QString repo_owner() const = 0;
    virtual QString repo_name() const = 0;
    virtual QRegularExpression platform_asset_pattern() const = 0;
    // Log-friendly short names: "frp" / "easytier-core".
    virtual QString service_name() const = 0;
    virtual QString release_name() const = 0;

    // --- Install directory & archive layout ---
    virtual QString default_install_dir() const = 0;
    virtual QString archive_filename_prefix() const = 0;
    virtual QString archive_extension() const = 0;
    virtual QStringList inner_dir_patterns() const = 0;

    // --- Filesystem / install logic ---
    virtual bool is_installed_impl() const = 0;
    virtual void install_binaries_impl(const QString& inner_dir,
                                       const QString& tmp_root) = 0;
    // Returns the message used when extraction fails; subclasses can pick
    // "zip" vs "tar" wording.
    virtual QString extract_archive_impl(const QString& archive_path,
                                         const QString& tmp_dir) = 0;

    // --- Process management ---
    virtual QString process_log_name() const = 0;

    // Hook fired after the managed process exits; default is a no-op.
    virtual void on_process_finished(int exit_code) { Q_UNUSED(exit_code); }

    // Hook fired once the managed process has successfully started on
    // either the QProcess path or the (Windows) elevated path. Subclasses
    // can override to e.g. kick off polling. Default is a no-op.
    virtual void on_process_started() {}

    // Hook fired at the end of reset_install() so subclasses can clear
    // additional state (e.g. cached toml content, virtual IP).
    virtual void reset_install_extra() {}

    // Hook fired at the end of detect_install() so subclasses can refresh
    // cached state derived from the install directory.
    virtual void on_detect_install() {}

    // State / log / error helpers, callable from subclasses.
    void set_state(State s);
    void set_error(const QString& message);
    void clear_error();
    void append_log(const QString& line);

    QProcess* process() const { return m_process; }
    void set_process(QProcess* p) { m_process = p; }

    QString temp_archive_path() const;
    QString extract_temp_dir() const;

protected:
    void detect_install();

private:
    void on_release_fetched(int mirror_index, GithubReleaseFetcher::Result r);
    void start_download(const QString& url);
    void on_download_finished(const QString& archive_path);
    void extract_archive(const QString& archive_path);
    void wire_process(QProcess* p);

    QNetworkAccessManager m_manager;
    GithubReleaseFetcher m_fetcher;
    QPointer<QNetworkReply> m_download_reply;

    State m_state{NotInstalled};
    QString m_version;
    float m_download_progress{0.0f};
    QString m_error_message;
    bool m_has_error{false};
    QProcess* m_process{nullptr};
    // True while the managed process was launched through pkexec (Linux).
    // Drives whether the stop sequence needs a privileged cleanup.
    bool m_launch_elevated{false};

    // AI-generated: Windows-only state for the UAC-elevated launch path
    // (ShellExecuteEx + runas). The handle is a void* to avoid pulling
    // windows.h into the header; reinterpret_cast as HANDLE in the .cpp.
    // On non-Windows these members are conditionally compiled out.
#ifdef _WIN32
    void* m_elevated_handle{nullptr};
    qint64 m_elevated_pid{0};
    QTimer* m_elevated_poll_timer{nullptr};
    bool m_elevate_pending{false};
    bool m_elevate_stop_requested{false};
    std::shared_ptr<ElevatedLaunchState> m_elevated_state;

    void launch_elevated_windows(const QString& program,
                                 const QStringList& arguments);
    // Unified tick: drives both "consume UAC launch result" (while pending)
    // and "monitor the elevated process for natural exit" (while running).
    void on_elevated_tick();
#endif

protected:
    // Set by derived constructor before detect_install(); exposed for
    // direct access by install code that needs the resolved install path.
    QString m_install_path;
};
