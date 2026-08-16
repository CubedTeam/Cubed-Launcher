#pragma once

#include <QString>
#include <QStringList>
#include <QStringView>
#include <compare>
#include <optional>

class SemanticVersion {
public:
    static std::optional<SemanticVersion> parse(QStringView version);

    QString toString() const;
    bool isPrerelease() const;
    std::strong_ordering comparePrecedence(const SemanticVersion& other) const;

    bool operator==(const SemanticVersion& other) const;
    std::strong_ordering operator<=>(const SemanticVersion& other) const;

private:
    QString m_major;
    QString m_minor;
    QString m_patch;
    QStringList m_prerelease;
    QStringList m_build;
};
