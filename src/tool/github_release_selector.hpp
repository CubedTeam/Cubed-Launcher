#pragma once

#include <QJsonArray>
#include <QRegularExpression>
#include <QString>
#include <optional>

struct GithubReleaseSelection {
    QString version;
    QString downloadUrl;
};

std::optional<GithubReleaseSelection>
selectGithubRelease(const QJsonArray& releases,
                    const QRegularExpression& assetRegex,
                    bool includePrereleases);
