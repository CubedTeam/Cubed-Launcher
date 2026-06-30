#include "cubed_instance.hpp"

#include <QDebug>
#include <QFileInfo>
#include <QProcess>
Q_INVOKABLE void CubedInstance::start_cubed_instance() {
    qDebug() << "Game Start";

    QStringList list{"--player", m_name, m_peer_mode, "--ip",
                     m_ip,       "-p",   m_port};

    if (!QProcess::startDetached(m_game_path, list)) {
        qWarning() << "Failed to start process:" << m_game_path;
        if (!QFileInfo::exists(m_game_path))
            qWarning() << " -> file not found";
        else if (!QFileInfo(m_game_path).isExecutable())
            qWarning() << " -> not executable";
    }
}
Q_INVOKABLE void CubedInstance::set_game_path(const QUrl& game_path_url) {
    m_game_path = game_path_url.toLocalFile();
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