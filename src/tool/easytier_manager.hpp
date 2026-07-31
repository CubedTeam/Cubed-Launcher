#pragma once

#include "tool/binary_service_base.hpp"

#include <QQmlEngine>
#include <QString>
#include <QTimer>

class EasyTierManager : public BinaryServiceBase {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString virtualIp READ virtual_ip NOTIFY virtual_ip_changed)

public:
    explicit EasyTierManager(QObject* parent = nullptr);
    ~EasyTierManager() override;

    QString virtual_ip() const { return m_virtual_ip; }

    Q_INVOKABLE void start(const QString& network_name,
                           const QString& network_secret,
                           const QString& peer_address);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void refresh_virtual_ip();
    Q_INVOKABLE void copy_to_clipboard(const QString& text);

signals:
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

private:
    QString core_binary() const;
    QString cli_binary() const;
    void start_ip_polling();
    void stop_ip_polling();
    void on_ip_poll_timeout();
    void parse_virtual_ip(const QByteArray& data);

    QString m_virtual_ip;
    QTimer* m_ip_poll_timer{nullptr};
};
