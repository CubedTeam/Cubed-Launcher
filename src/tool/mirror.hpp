#pragma once
#include <QQmlEngine>
#include <QString>
#include <QStringList>
#include <QVector>
#include <qtmetamacros.h>

// AI-generated: a single mirror source entry.
// name:    human-readable label shown in the UI ComboBox.
// prefix:  URL prefix prepended to a GitHub download URL (empty for direct).
struct MirrorEntry {
    QString name;
    QString prefix;
};

// AI-generated: built-in mirror catalogue.
// Index 0 is always the direct GitHub source (empty prefix) and acts as the
// fallback when no mirror is chosen. The remaining entries are public GitHub
// proxy mirrors; they may go offline at any time, so the list is intentionally
// generous. The order is part of the persisted Settings contract (the selected
// index is stored by position), so do not reorder existing entries.
inline const QVector<MirrorEntry> mirror_sources{
    {QStringLiteral("Direct (GitHub)"), QStringLiteral("")},
    {QStringLiteral("gh-proxy.org"), QStringLiteral("https://gh-proxy.org/")},
    {QStringLiteral("ghproxy.net"), QStringLiteral("https://ghproxy.net/")},
    {QStringLiteral("ghps.cc"), QStringLiteral("https://ghps.cc/")},
    {QStringLiteral("github.moeyy.xyz"),
     QStringLiteral("https://github.moeyy.xyz/")},
    {QStringLiteral("gh.api.99988866.xyz"),
     QStringLiteral("https://gh.api.99988866.xyz/")},
    {QStringLiteral("mirror.ghproxy.com"),
     QStringLiteral("https://mirror.ghproxy.com/")},
};

// AI-generated: QML singleton exposing the mirror catalogue to the UI and a
// helper that prepends the selected mirror prefix to a GitHub download URL.
class MirrorSource : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QStringList names READ names CONSTANT)
public:
    explicit MirrorSource(QObject* parent = nullptr);
    QStringList names() const;
    // AI-generated: returns url unchanged for the direct source (index 0) or
    // when index is out of range, otherwise prepends the mirror prefix.
    Q_INVOKABLE QString apply(const QString& url, int index) const;
};