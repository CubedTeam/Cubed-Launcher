#include "tool/json_cache.hpp"

#include "path_tools.hpp"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QVariant>

namespace {
constexpr QLatin1StringView kTs("ts");
constexpr QLatin1StringView kData("data");

QString cache_file_path(const QString& cache_name) {
    const auto hash =
        QString::fromLatin1(QCryptographicHash::hash(cache_name.toUtf8(),
                                                     QCryptographicHash::Sha256)
                                .toHex());
    return JsonCache::cache_dir() + QLatin1Char('/') + hash +
           QStringLiteral(".json");
}
} // namespace

QString JsonCache::cache_dir() {
    return DefaultDir::get_default_cache_dir() + QStringLiteral("/json");
}

std::optional<QJsonObject> JsonCache::read(const QString& cache_name,
                                           qint64 ttl_seconds) {
    QFile f(cache_file_path(cache_name));
    if (!f.exists() || !f.open(QIODevice::ReadOnly)) {

        return std::nullopt;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) {
        return std::nullopt;
    }
    const QJsonObject wrap = doc.object();
    const qint64 ts = wrap.value(kTs).toVariant().toLongLong();
    const qint64 age_ms = QDateTime::currentDateTime().toMSecsSinceEpoch() - ts;
    if (age_ms < 0 || age_ms > ttl_seconds * qint64(1000)) {
        return std::nullopt;
    }
    const QJsonValue data = wrap.value(kData);
    if (!data.isObject()) {
        return std::nullopt;
    }
    return data.toObject();
}

bool JsonCache::write(const QString& key, const QJsonObject& object) {
    QDir().mkpath(cache_dir());

    QJsonObject wrap;
    wrap.insert(kTs, QDateTime::currentDateTime().toMSecsSinceEpoch());
    wrap.insert(kData, object);

    QFile f(cache_file_path(key));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    f.write(QJsonDocument(wrap).toJson(QJsonDocument::Compact));
    f.flush();
    return true;
}

bool JsonCache::remove(const QString& key) {
    QFile f(cache_file_path(key));
    if (!f.exists()) {
        return true;
    }
    return f.remove();
}