#include "tool/secret_store.hpp"

#include "tool/log.hpp"

#include <QSettings>
#include <QString>

#if defined(_WIN32)
// clang-format off
#include <windows.h>
#include <dpapi.h>
// clang-format on
#else
#include <libsecret/secret.h>

namespace {
const SecretSchema kSecretSchema = {
    "org.cubedlauncher.Generic",
    SECRET_SCHEMA_NONE,
    {
        {"name", SECRET_SCHEMA_ATTRIBUTE_STRING},
        {nullptr, SECRET_SCHEMA_ATTRIBUTE_STRING},
    },
    0,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};
} // namespace
#endif

namespace SecretStore {

#if defined(_WIN32)
constexpr QLatin1StringView kDpapiPrefix("DPAPI:");

void write_dpapi_blob(const QString& key, const QByteArray& blob) {
    QSettings s;
    s.setValue(QStringLiteral("secret_store/") + key, blob);
    s.sync();
}

QByteArray read_dpapi_blob(const QString& key) {
    return QSettings()
        .value(QStringLiteral("secret_store/") + key)
        .toByteArray();
}

void delete_dpapi_blob(const QString& key) {
    QSettings s;
    s.remove(QStringLiteral("secret_store/") + key);
    s.sync();
}
#endif

bool available() { return true; }

bool save(const QString& key, const QByteArray& secret) {
    if (key.isEmpty()) {
        return false;
    }
#if defined(_WIN32)
    const std::wstring wkey = key.toStdWString();
    DATA_BLOB input{};
    input.pbData =
        reinterpret_cast<BYTE*>(const_cast<char*>(secret.constData()));
    input.cbData = static_cast<DWORD>(secret.size());

    DATA_BLOB output{};
    if (!CryptProtectData(&input, wkey.c_str(), nullptr, nullptr, nullptr,
                          CRYPTPROTECT_UI_FORBIDDEN, &output)) {
        Logger::warn("DPAPI CryptProtectData failed: {}", GetLastError());
        return false;
    }
    QByteArray blob(reinterpret_cast<const char*>(output.pbData),
                    static_cast<int>(output.cbData));
    LocalFree(output.pbData);

    QByteArray wrapped;
    wrapped.append(kDpapiPrefix.data(), kDpapiPrefix.size());
    wrapped.append(blob.toBase64());
    write_dpapi_blob(key, wrapped);
    return true;
#else
    GError* err = nullptr;
    const gboolean ok = secret_password_store_sync(
        &kSecretSchema, nullptr,
        qPrintable(QStringLiteral("CubedLauncher ") + key), secret.constData(),
        nullptr, &err, "name", qPrintable(key), nullptr);
    if (err) {
        Logger::warn("libsecret store failed: {}", err->message);
        g_error_free(err);
    }
    return ok == TRUE;
#endif
}

std::optional<QByteArray> load(const QString& key) {
    if (key.isEmpty()) {
        return std::nullopt;
    }
#if defined(_WIN32)
    const QByteArray wrapped = read_dpapi_blob(key);
    if (wrapped.isEmpty() || !wrapped.startsWith(QByteArrayView(
                                 kDpapiPrefix.data(), kDpapiPrefix.size()))) {
        return std::nullopt;
    }
    const QByteArray b64 = wrapped.mid(kDpapiPrefix.size());
    const QByteArray blob = QByteArray::fromBase64(b64);
    if (blob.isEmpty()) {
        return std::nullopt;
    }

    DATA_BLOB input{};
    input.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(blob.constData()));
    input.cbData = static_cast<DWORD>(blob.size());

    DATA_BLOB output{};
    if (!CryptUnprotectData(&input, nullptr, nullptr, nullptr, nullptr,
                            CRYPTPROTECT_UI_FORBIDDEN, &output)) {
        Logger::warn("DPAPI CryptUnprotectData failed: {}", GetLastError());
        return std::nullopt;
    }
    QByteArray plain(reinterpret_cast<const char*>(output.pbData),
                     static_cast<int>(output.cbData));
    LocalFree(output.pbData);
    return plain;
#else
    GError* err = nullptr;
    gchar* password = secret_password_lookup_sync(
        &kSecretSchema, nullptr, &err, "name", qPrintable(key), nullptr);
    if (err) {
        Logger::warn("libsecret lookup failed: {}", err->message);
        g_error_free(err);
        return std::nullopt;
    }
    if (!password) {
        return std::nullopt;
    }
    QByteArray plain(password);
    secret_password_free(password);
    return plain;
#endif
}

bool remove(const QString& key) {
    if (key.isEmpty()) {
        return false;
    }
#if defined(_WIN32)
    delete_dpapi_blob(key);
    return true;
#else
    GError* err = nullptr;
    const gboolean ok = secret_password_clear_sync(
        &kSecretSchema, nullptr, &err, "name", qPrintable(key), nullptr);
    if (err) {
        Logger::warn("libsecret clear failed: {}", err->message);
        g_error_free(err);
    }
    return ok == TRUE;
#endif
}

} // namespace SecretStore
