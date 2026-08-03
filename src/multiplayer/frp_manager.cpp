#include "multiplayer/frp_manager.hpp"

#include "settings.hpp"
#include "tool/path_tools.hpp"

#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <qmicroz.h>

FrpManager::FrpManager(QObject* parent)
    : BinaryServiceBase(QStringLiteral("frp"), parent) {
    QString path = default_install_dir();
    if (Settings* s = Settings::instance()) {
        const QString persisted = s->frp_install_path();
        if (!persisted.isEmpty()) {
            path = persisted;
        }
    }
    m_install_path = path;
    detect_install();
}

QString FrpManager::default_install_dir() const {
    return get_default_frp_install_dir();
}

QRegularExpression FrpManager::platform_asset_pattern() const {
#ifdef _WIN32
    return QRegularExpression(R"(^frp_[\d.]+_windows_amd64\.zip$)");
#else
    return QRegularExpression(R"(^frp_[\d.]+_linux_amd64\.tar\.gz$)");
#endif
}

QString FrpManager::archive_extension() const {
#ifdef _WIN32
    return QStringLiteral(".zip");
#else
    return QStringLiteral(".tar.gz");
#endif
}

QString FrpManager::frpc_binary() const {
#ifdef _WIN32
    return m_install_path + "/frpc.exe";
#else
    return m_install_path + "/frpc";
#endif
}

QString FrpManager::toml_path() const { return m_install_path + "/frpc.toml"; }

bool FrpManager::is_installed_impl() const {
    return QFileInfo::exists(frpc_binary()) && QFileInfo::exists(toml_path());
}

QString FrpManager::extract_archive_impl(const QString& archive_path,
                                         const QString& tmp_dir) {
#ifdef _WIN32
    QMicroz zip(archive_path);
    zip.setOutputFolder(tmp_dir);
    if (!zip.extractAll()) {
        return QStringLiteral("Failed to extract zip archive");
    }
    return {};
#else
    QProcess tar;
    tar.setProgram("tar");
    tar.setArguments({"xzf", archive_path, "-C", tmp_dir});
    tar.start();
    if (!tar.waitForStarted() || !tar.waitForFinished(-1)) {
        return QStringLiteral("Failed to run tar: %1").arg(tar.errorString());
    }
    return {};
#endif
}

void FrpManager::install_binaries_impl(const QString& inner_dir,
                                       const QString& tmp_root) {
    QDir().mkpath(m_install_path);

    // AI-generated: frpc ships as frpc.exe inside the Windows archive.
    const QString src_binary =
        inner_dir + "/" + QFileInfo(frpc_binary()).fileName();
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

void FrpManager::reset_install_extra() {
    m_frpc_toml.clear();
    emit frpc_toml_changed();
}

void FrpManager::on_detect_install() { load_toml_into_property(); }

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

Q_INVOKABLE void FrpManager::start() {
    launch_process(frpc_binary(), QStringList() << "-c" << "frpc.toml",
                   QProcessEnvironment::systemEnvironment());
}

Q_INVOKABLE void FrpManager::stop() { stop_process(); }

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
