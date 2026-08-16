#pragma once

#include "multiplayer/binary_service_base.hpp"

#include <QQmlEngine>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariantMap>
#include <QVector>

// AI-generated: hardcoded public EasyTier community servers. Mirrors the
// mirror_sources pattern in mirror.hpp: only the index is persisted in
// Settings, the actual addresses are baked in here.
struct PublicServerEntry {
    QString name;
    QString address;
};

inline const QVector<PublicServerEntry> easytier_public_servers{
    {QStringLiteral("225284.xyz"), QStringLiteral("tcp://225284.xyz:11010")},
    {QStringLiteral("183.230.36.171"),
     QStringLiteral("tcp://183.230.36.171:11010")},
    {QStringLiteral("easytier.weiai.org.cn"),
     QStringLiteral("tcp://easytier.weiai.org.cn:11010")},
};

class EasyTierManager : public BinaryServiceBase {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString virtualIp READ virtual_ip NOTIFY virtual_ip_changed)
    Q_PROPERTY(QStringList publicServerNames READ public_server_names CONSTANT)

public:
    explicit EasyTierManager(QObject* parent = nullptr);
    ~EasyTierManager() override;

    QString virtual_ip() const { return m_virtual_ip; }
    QStringList public_server_names() const;
    Q_INVOKABLE QString public_server_address(int index) const;

    // AI-generated: room code helpers. The code is a short random
    // identifier; the actual network name + secret are derived
    // deterministically so host and players converge on the same
    // EasyTier network without sharing long strings.
    Q_INVOKABLE QString generate_room_code() const;
    Q_INVOKABLE QVariantMap credentials_for_code(const QString& code) const;
    Q_INVOKABLE bool is_valid_room_code(const QString& code) const;

    Q_INVOKABLE void start(const QString& network_name,
                           const QString& network_secret,
                           const QString& peer_address);
    Q_INVOKABLE void start_join(const QString& network_name,
                                const QString& network_secret,
                                const QString& peer_address);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void refresh_virtual_ip();
    Q_INVOKABLE void copy_to_clipboard(const QString& text);

Q_SIGNALS:
    void virtual_ip_changed();

protected:
    QString repo_owner() const override { return QStringLiteral("EasyTier"); }
    QString repo_name() const override { return QStringLiteral("EasyTier"); }
    QRegularExpression platform_asset_pattern() const override;
    QString service_name() const override { return QStringLiteral("easytier"); }
    QString release_name() const override { return QStringLiteral("easytier"); }
    QString default_install_dir() const override;
    QString archive_filename_prefix() const override {
        return QStringLiteral("easytier_download");
    }
    QString archive_extension() const override {
        return QStringLiteral(".zip");
    }
    QStringList inner_dir_patterns() const override {
        return {QStringLiteral("easytier-*")};
    }
    bool is_installed_impl() const override;
    void install_binaries_impl(const QString& inner_dir,
                               const QString& tmp_root) override;
    QString extract_archive_impl(const QString& archive_path,
                                 const QString& tmp_dir) override;
    QString process_log_name() const override {
        return QStringLiteral("easytier-core");
    }
    void on_process_finished(int exit_code) override;
    void reset_install_extra() override;

private:
    QString core_binary() const;
    QString cli_binary() const;
    void start_ip_polling();
    void stop_ip_polling();
    void on_ip_poll_timeout();
    void parse_virtual_ip(const QByteArray& data);
    // AI-generated: when easytier-core is launched via pkexec, terminating the
    // pkexec QProcess may leave the root easytier-core child orphaned (pkexec
    // does not reliably forward SIGTERM). This helper spawns a detached
    // `pkexec pkill -f <core path>` to guarantee the root process dies.
    void kill_core_as_root();

    QString m_virtual_ip;
    QTimer* m_ip_poll_timer{nullptr};
};
