#include "game-mgmt/cubed_instance.hpp"

#include <QDebug>
#include <QFileInfo>

Q_INVOKABLE void CubedInstance::start_cubed_instance() {
    qDebug() << "Game Start";

    QProcess* process = new QProcess(this);

    QStringList argument;
    QString program;
    if (m_wrapper_command.isEmpty()) {
        program = m_game_path;
    } else {
        program = m_wrapper_command;
        argument.append(m_game_path);
    }
    argument.append(
        {"--player", m_name, m_peer_mode, "--ip", m_ip, "-p", m_port});
    QFileInfo info(m_game_path);

    process->setWorkingDirectory(info.absolutePath());
    process->setProgram(program);
    process->setArguments(argument);
    connect(process, &QProcess::finished, this,
            [process, this](int exitCode, QProcess::ExitStatus status) {
                qDebug() << "Process exit, exit code: " << exitCode;
                m_processes.removeAll(process);
                emit running_changed();
                process->deleteLater();
            });

    connect(process, &QProcess::errorOccurred, this,
            [](QProcess::ProcessError error) { qDebug() << error; });

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
        emit running_changed();
    }
}
Q_INVOKABLE void CubedInstance::set_game_path(const QUrl& game_path_url) {
    m_game_path = game_path_url.toLocalFile();
    emit path_change();
}

Q_INVOKABLE void CubedInstance::set_peer(int index) {
    if (index == 0) {
        m_peer_mode = "--host";
    } else if (index == 1) {
        m_peer_mode = "--client";
    }
}

Q_INVOKABLE void CubedInstance::set_port(const QString& port) { m_port = port; }
Q_INVOKABLE void CubedInstance::set_ip(const QString& ip) { m_ip = ip; }
Q_INVOKABLE void CubedInstance::set_name(const QString& name) { m_name = name; }
Q_INVOKABLE void CubedInstance::kill_all() {
    for (auto* proc : std::as_const(m_processes)) {
        if (proc->state() != QProcess::NotRunning)
            proc->kill();
    }
}

Q_INVOKABLE void CubedInstance::set_wrapper_command(const QString& wrapper) {
    m_wrapper_command = wrapper;
}

bool CubedInstance::running() const { return !m_processes.isEmpty(); }
bool CubedInstance::game_path_select() const { return !m_game_path.isEmpty(); }
void CubedInstance::set_log_statue(bool status) { m_log_on = status; }