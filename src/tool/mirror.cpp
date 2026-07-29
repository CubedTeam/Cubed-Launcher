#include "tool/mirror.hpp"

MirrorSource::MirrorSource(QObject* parent) : QObject(parent) {}

QStringList MirrorSource::names() const {
    QStringList list;
    list.reserve(mirror_sources.size());
    for (const auto& entry : mirror_sources) {
        list.append(entry.name);
    }
    return list;
}

QString MirrorSource::apply(const QString& url, int index) const {
    if (index <= 0 || index >= mirror_sources.size()) {
        return url;
    }
    const QString& prefix = mirror_sources.at(index).prefix;
    if (prefix.isEmpty()) {
        return url;
    }
    return prefix + url;
}