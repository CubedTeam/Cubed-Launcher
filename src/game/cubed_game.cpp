#include "game/cubed_game.hpp"

#include "settings.hpp"
#include "tool/log.hpp"
#include "tool/path_tools.hpp"

#include <QFileInfo>

CubedGame::CubedGame() {}

Q_INVOKABLE void CubedGame::start_cubed_game() {
    if (m_game_install_dir.isEmpty()) {
        m_game_install_dir = DefaultDir::get_default_game_install_dir();
    }
    const QString program_path = m_game_install_dir + "/" +
                                 DefaultDir::get_default_game_executable_name();
    Logger::info("Game Start, dir {} exec {}", m_game_install_dir.toStdString(),
                 program_path.toStdString());

    QProcess* process = new QProcess(this);

    QStringList argument;
    QString program;
    const QString wrapper = Settings::instance()
                                ? Settings::instance()->wrapper_command()
                                : QString();
    if (wrapper.isEmpty()) {
        program = program_path;
    } else {
        program = wrapper;
        argument.append(program_path);
    }
    argument.append(
        {"--player", m_name, m_peer_mode, "--ip", m_ip, "-p", m_port});

    process->setWorkingDirectory(m_game_install_dir);
    process->setProgram(program);
    process->setArguments(argument);
    connect(process, &QProcess::finished, this,
            [process, this](int exitCode, QProcess::ExitStatus status) {
                Logger::info("Process exit, exit code: {}", exitCode);
                m_processes.removeAll(process);
                Q_EMIT running_changed();
                process->deleteLater();
            });

    connect(process, &QProcess::errorOccurred, this,
            [process](QProcess::ProcessError error) {
                Logger::error("Process error: {}", static_cast<int>(error));
                process->deleteLater();
            });

    if (m_log_on) {
        connect(process, &QProcess::readyReadStandardOutput, this, [process]() {
            QByteArray data = process->readAllStandardOutput();
            if (!data.isEmpty()) {
                fwrite(data.constData(), 1, data.size(), stdout);
                fflush(stdout);
            }
        });
        connect(process, &QProcess::readyReadStandardError, this, [process]() {
            QByteArray data = process->readAllStandardError();
            if (!data.isEmpty()) {
                fwrite(data.constData(), 1, data.size(), stderr);
                fflush(stderr);
            }
        });
    }

    process->start();
    if (process->waitForStarted()) {
        m_processes.append(process);
        Q_EMIT running_changed();
    }
}

Q_INVOKABLE void CubedGame::set_game_dir_url(const QUrl& game_dir_url) {
    m_game_install_dir = game_dir_url.toLocalFile();
    Logger::info("Url Change Game Install Dir {}",
                 m_game_install_dir.toStdString());
    check_version();
    Q_EMIT path_change();
}

Q_INVOKABLE void CubedGame::set_game_dir(const QString& game_dir) {
    m_game_install_dir = game_dir;
    Logger::info("Path: Change Game Install Dir {}",
                 m_game_install_dir.toStdString());
    check_version();
    Q_EMIT path_change();
}

Q_INVOKABLE void CubedGame::set_peer(int index) {
    if (index == 0) {
        m_peer_mode = "--host";
    } else if (index == 1) {
        m_peer_mode = "--client";
    }
}

Q_INVOKABLE void CubedGame::set_port(const QString& port) { m_port = port; }
Q_INVOKABLE void CubedGame::set_ip(const QString& ip) { m_ip = ip; }
Q_INVOKABLE void CubedGame::set_name(const QString& name) { m_name = name; }
Q_INVOKABLE void CubedGame::kill_all() {
    for (auto* proc : std::as_const(m_processes)) {
        if (proc->state() != QProcess::NotRunning)
            proc->kill();
    }
}

Q_INVOKABLE void CubedGame::check_version() {
    if (m_game_install_dir.isEmpty()) {
        m_game_install_dir = DefaultDir::get_default_game_install_dir();
    }
    const QString program_path = m_game_install_dir + "/" +
                                 DefaultDir::get_default_game_executable_name();

    auto info = QFileInfo(program_path);
    if (!info.isFile()) {
        m_installed = false;
        Q_EMIT installed_changed();
        Q_EMIT version_changed();
        Logger::warn("{} is not a file", program_path.toStdString());
        return;
    }
    QProcess* process = new QProcess(this);
    process->setProgram(program_path);
    process->setArguments(QStringList("-V"));

    connect(process, &QProcess::finished, this,
            [process](int exitCode, QProcess::ExitStatus status) {
                if (exitCode != 0) {
                    Logger::error("Cubed Game error, can't get version");
                }

                process->deleteLater();
            });

    connect(process, &QProcess::errorOccurred, this,
            [](QProcess::ProcessError error) {
                Logger::error("check_version fail");
                Logger::error("Process error: {}", static_cast<int>(error));
            });

    connect(process, &QProcess::readyReadStandardOutput, this,
            [process, this]() {
                QByteArray data = process->readAllStandardOutput();
                QString str = QString(data);
                str = str.trimmed();
                if (!str.isEmpty()) {
                    m_version = str;
                    m_installed = true;
                    Logger::info("Cubed Version: {}", m_version.toStdString());
                    Q_EMIT version_changed();
                    Q_EMIT installed_changed();
                } else {
                    m_installed = false;
                    Q_EMIT installed_changed();
                    Q_EMIT version_changed();
                }
            });

    process->start();
}

bool CubedGame::running() const { return !m_processes.isEmpty(); }
bool CubedGame::game_path_select() const {
    return !m_game_install_dir.isEmpty();
}
bool CubedGame::installed() const { return m_installed; }
void CubedGame::set_log_statue(bool status) { m_log_on = status; }
QString CubedGame::version() const { return m_version; }
bool CubedGame::log_on() const { return m_log_on; }
