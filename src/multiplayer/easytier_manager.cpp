#include "multiplayer/easytier_manager.hpp"

#include "settings.hpp"
#include "tool/log.hpp"
#include "tool/path_tools.hpp"

#include <QClipboard>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QStandardPaths>
#include <qmicroz.h>
EasyTierManager::EasyTierManager(QObject* parent)
    : BinaryServiceBase(QStringLiteral("Easytier"), parent) {
    QString path = default_install_dir();
    if (Settings* s = Settings::instance()) {
        const QString persisted = s->easytier_install_path();
        if (!persisted.isEmpty()) {
            path = persisted;
        }
    }
    m_install_path = path;
    detect_install();
}

EasyTierManager::~EasyTierManager() {
#ifndef _WIN32
    if (was_elevated() && running()) {
        const QString core = core_binary();
        if (!core.isEmpty() &&
            !QStandardPaths::findExecutable(QStringLiteral("pkexec"))
                 .isEmpty()) {
            const QString pattern =
                QStringLiteral("^") +
                QRegularExpression::escape(core).replace(QLatin1String("\\/"),
                                                         QLatin1String("/"));
            // Synchronous call: blocks until pkill finishes
            QProcess::execute(QStringLiteral("pkexec"),
                              {QStringLiteral("pkill"), QStringLiteral("-KILL"),
                               QStringLiteral("-f"), pattern});
        }
    }
#endif
    stop_ip_polling();
    delete m_ip_poll_timer;
    m_ip_poll_timer = nullptr;
}

QStringList EasyTierManager::public_server_names() const {
    QStringList list;
    list.reserve(easytier_public_servers.size());
    for (const auto& entry : easytier_public_servers) {
        list.append(entry.name);
    }
    return list;
}

QString EasyTierManager::public_server_address(int index) const {
    if (index < 0 || index >= easytier_public_servers.size()) {
        return {};
    }
    return easytier_public_servers.at(index).address;
}

QString EasyTierManager::default_install_dir() const {
    return DefaultDir::get_default_easytier_install_dir();
}

QRegularExpression EasyTierManager::platform_asset_pattern() const {
#ifdef _WIN32
    return QRegularExpression(R"(^easytier-windows-x86_64-v[\d.]+\.zip$)");
#else
    return QRegularExpression(R"(^easytier-linux-x86_64-v[\d.]+\.zip$)");
#endif
}

QString EasyTierManager::core_binary() const {
#ifdef _WIN32
    return m_install_path + "/easytier-core.exe";
#else
    return m_install_path + "/easytier-core";
#endif
}

QString EasyTierManager::cli_binary() const {
#ifdef _WIN32
    return m_install_path + "/easytier-cli.exe";
#else
    return m_install_path + "/easytier-cli";
#endif
}

bool EasyTierManager::is_installed_impl() const {
    return QFileInfo::exists(core_binary());
}

QString EasyTierManager::extract_archive_impl(const QString& archive_path,
                                              const QString& tmp_dir) {
    QMicroz zip(archive_path);
    zip.setOutputFolder(tmp_dir);
    if (!zip.extractAll()) {
        return QStringLiteral("Failed to extract zip archive");
    }
    return {};
}

void EasyTierManager::install_binaries_impl(const QString& inner_dir,
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
    if (!copy_binary(src_cli, cli_binary())) {
        QDir(tmp_root).removeRecursively();
        set_error(QStringLiteral("Failed to copy easytier-cli"));
        return;
    }

#ifdef _WIN32
    // AI-generated: easytier-core ships with runtime DLLs (vcruntime, wintun,
    // ...); copy all of them so the installed binary can resolve its imports.
    QDirIterator dll_it(
        inner.absolutePath(), QStringList() << QStringLiteral("*.dll"),
        QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (dll_it.hasNext()) {
        const QString src_dll = dll_it.next();
        const QString dst_dll =
            m_install_path + "/" + QFileInfo(src_dll).fileName();
        if (QFile::exists(dst_dll)) {
            QFile::remove(dst_dll);
        }
        if (!QFile::copy(src_dll, dst_dll)) {
            QDir(tmp_root).removeRecursively();
            set_error(QStringLiteral("Failed to copy %1")
                          .arg(QFileInfo(src_dll).fileName()));
            return;
        }
    }
#endif

    QDir(tmp_root).removeRecursively();

    if (!installed()) {
        set_error(QStringLiteral(
            "Install completed but easytier-core binary missing"));
        return;
    }
    append_log(QStringLiteral("Installed easytier to %1").arg(m_install_path));
    clear_error();
    set_state(Ready);
    Q_EMIT installed_changed();
}

void EasyTierManager::on_process_finished(int exit_code) {
    Q_UNUSED(exit_code);
    stop_ip_polling();
    if (!m_virtual_ip.isEmpty()) {
        m_virtual_ip.clear();
        Q_EMIT virtual_ip_changed();
    }
}
void EasyTierManager::reset_install_extra() {
    m_virtual_ip.clear();
    Q_EMIT virtual_ip_changed();
}
Q_INVOKABLE void EasyTierManager::start(const QString& network_name,
                                        const QString& network_secret,
                                        const QString& peer_address) {
    if (running()) {
        return;
    }
    if (!installed()) {
        set_error(QStringLiteral("easytier-core is not installed"));
        return;
    }
    if (network_name.isEmpty() || network_secret.isEmpty() ||
        peer_address.isEmpty()) {
        set_error(QStringLiteral(
            "Network name, secret and peer address are required"));
        return;
    }
    const QStringList args{
        QStringLiteral("--network-name"),   network_name,
        QStringLiteral("--network-secret"), network_secret,
        QStringLiteral("--peers"),          peer_address,
        QStringLiteral("--dhcp"),           QStringLiteral("true"),
    };
    // AI-generated: connect first so the polling kicks in whether the
    // process is already running (Linux pkexec) or starts asynchronously
    // after UAC (Windows runas). SingleShotConnection auto-disconnects
    // after the first signal.
    connect(
        this, &BinaryServiceBase::running_changed, this,
        [this]() {
            if (running())
                start_ip_polling();
        },
        Qt::SingleShotConnection);
    launch_process(core_binary(), args,
                   QProcessEnvironment::systemEnvironment(), /*elevate=*/true);
}

Q_INVOKABLE void EasyTierManager::start_join(const QString& network_name,
                                             const QString& network_secret,
                                             const QString& peer_address) {
    if (running()) {
        return;
    }
    if (!installed()) {
        set_error(QStringLiteral("easytier-core is not installed"));
        return;
    }
    if (network_name.isEmpty() || network_secret.isEmpty() ||
        peer_address.isEmpty()) {
        set_error(QStringLiteral(
            "Network name, secret and peer address are required"));
        return;
    }
    const QStringList args{
        QStringLiteral("--network-name"),   network_name,
        QStringLiteral("--network-secret"), network_secret,
        QStringLiteral("--peers"),          peer_address,
        QStringLiteral("--no-tun"),         QStringLiteral("true"),
        QStringLiteral("--dhcp"),           QStringLiteral("true"),
    };
    launch_process(core_binary(), args,
                   QProcessEnvironment::systemEnvironment());
}

Q_INVOKABLE void EasyTierManager::stop() {
    // AI-generated: on the elevated (pkexec) path the wrapper QProcess
    // does not forward signals to the child, so stop_process() would
    // block the UI thread waiting up to 5s for a process that won't
    // exit on its own. Fire the privileged pkill immediately instead;
    // the QProcess finished handler updates state when the core
    // actually dies.
    if (was_elevated()) {
        kill_core_as_root();
        return;
    }
    stop_process();
}

void EasyTierManager::kill_core_as_root() {
#ifndef _WIN32
    const QString core = core_binary();
    if (core.isEmpty() ||
        QStandardPaths::findExecutable(QStringLiteral("pkexec")).isEmpty()) {
        return;
    }
    // Anchor to the start of the command line so pkill -f only matches
    // the easytier-core binary (and never the pkexec wrapper or an
    // unrelated process whose command line merely contains the path).
    // Drop QRegularExpression::escape's "<\/"; pkill's regex engine
    // treats the slash as a literal even on stricter implementations.
    const QString pattern =
        QStringLiteral("^") + QRegularExpression::escape(core).replace(
                                  QLatin1String("\\/"), QLatin1String("/"));
    auto* p = new QProcess(this);
    p->setProgram(QStringLiteral("pkexec"));
    p->setArguments(QStringList()
                    << QStringLiteral("pkill") << QStringLiteral("-KILL")
                    << QStringLiteral("-f") << pattern);
    connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, p](int exitCode, QProcess::ExitStatus) {
                // pkill exit 0 = killed, 1 = no match (orphan if we know
                // one existed). Other codes typically mean the user
                // canceled the polkit prompt.
                if (exitCode != 0) {
                    append_log(
                        QStringLiteral("pkexec pkill exited with %1 (core may "
                                       "still be running as root)")
                            .arg(exitCode));
                }
                p->deleteLater();
            });
    p->start();
#endif
}

void EasyTierManager::start_ip_polling() {
    if (!m_ip_poll_timer) {
        m_ip_poll_timer = new QTimer(this);
        m_ip_poll_timer->setInterval(2000);
        connect(m_ip_poll_timer, &QTimer::timeout, this,
                &EasyTierManager::on_ip_poll_timeout);
    }
    m_ip_poll_timer->start();
    QTimer::singleShot(0, this, &EasyTierManager::on_ip_poll_timeout);
}

void EasyTierManager::stop_ip_polling() {
    if (m_ip_poll_timer) {
        m_ip_poll_timer->stop();
    }
}

void EasyTierManager::on_ip_poll_timeout() {
    if (!running()) {
        stop_ip_polling();
        return;
    }
    refresh_virtual_ip();
}

void EasyTierManager::refresh_virtual_ip() {
    if (!running()) {
        return;
    }
    if (!QFileInfo::exists(cli_binary())) {
        return;
    }
    auto* cli = new QProcess(this);
    cli->setProgram(cli_binary());
    cli->setArguments(QStringList() << "node");
    connect(cli, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, cli](int exitCode, QProcess::ExitStatus) {
                const QByteArray data = cli->readAllStandardOutput();
                cli->deleteLater();
                if (exitCode == 0) {
                    parse_virtual_ip(data);
                }
            });
    connect(cli, &QProcess::errorOccurred, [cli]() {
        cli->deleteLater();
        Logger::error("Can't Open Easytier Cli");
    });
    cli->start();
}

void EasyTierManager::parse_virtual_ip(const QByteArray& data) {
    static const QRegularExpression ip_re(
        QStringLiteral(R"(\b(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\b)"));
    const auto lines = QString::fromUtf8(data).split('\n');
    for (const auto& line : lines) {
        if (!line.contains(QStringLiteral("Virtual IP"))) {
            continue;
        }
        const auto m = ip_re.match(line);
        if (m.hasMatch()) {
            const QString ip = m.captured(1);
            if (ip != m_virtual_ip) {
                m_virtual_ip = ip;
                Q_EMIT virtual_ip_changed();
            }
            return;
        }
    }
}

Q_INVOKABLE void EasyTierManager::copy_to_clipboard(const QString& text) {
    if (QClipboard* cb = QGuiApplication::clipboard()) {
        cb->setText(text);
    }
}
