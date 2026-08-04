#pragma once

#include "multiplayer/binary_service_base.hpp"

#include <QQmlEngine>
#include <QString>

class FrpManager : public BinaryServiceBase {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString frpcToml READ frpc_toml NOTIFY frpc_toml_changed)

public:
    explicit FrpManager(QObject* parent = nullptr);

    QString frpc_toml() const { return m_frpc_toml; }

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void save_toml(const QString& content);
    Q_INVOKABLE QString read_toml() const;

Q_SIGNALS:
    void frpc_toml_changed();

protected:
    QString repo_owner() const override { return QStringLiteral("fatedier"); }
    QString repo_name() const override { return QStringLiteral("frp"); }
    QRegularExpression platform_asset_pattern() const override;
    QString service_name() const override { return QStringLiteral("frp"); }
    QString release_name() const override { return QStringLiteral("frp"); }
    QString default_install_dir() const override;
    QString archive_filename_prefix() const override {
        return QStringLiteral("frp_download");
    }
    QString archive_extension() const override;
    QStringList inner_dir_patterns() const override {
        return {QStringLiteral("frp_*")};
    }
    bool is_installed_impl() const override;
    void install_binaries_impl(const QString& inner_dir,
                               const QString& tmp_root) override;
    QString extract_archive_impl(const QString& archive_path,
                                 const QString& tmp_dir) override;
    QString process_log_name() const override { return QStringLiteral("frpc"); }
    void reset_install_extra() override;
    void on_detect_install() override;

private:
    QString frpc_binary() const;
    QString toml_path() const;
    void load_toml_into_property();

    QString m_frpc_toml;
};
