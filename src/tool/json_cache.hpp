#pragma once

#include <QJsonObject>
#include <optional>

class JsonCache {
public:
    static std::optional<QJsonObject> read(const QString& cache_name,
                                           qint64 ttl_seconds);

    static bool write(const QString& key, const QJsonObject& object);
    static bool remove(const QString& key);
    static bool clear_all();

    static QString cache_dir();
};