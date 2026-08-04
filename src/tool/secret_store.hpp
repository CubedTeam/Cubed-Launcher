#pragma once

#include <QByteArray>
#include <QString>
#include <optional>

namespace SecretStore {
bool available();
bool save(const QString& key, const QByteArray& secret);
std::optional<QByteArray> load(const QString& key);
bool remove(const QString& key);
} // namespace SecretStore
