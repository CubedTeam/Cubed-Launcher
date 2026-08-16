#include "tool/github_release_selector.hpp"

#include "tool/semantic_version.hpp"

#include <QJsonObject>

std::optional<GithubReleaseSelection>
selectGithubRelease(const QJsonArray& releases,
                    const QRegularExpression& assetRegex,
                    bool includePrereleases) {
    std::optional<SemanticVersion> selectedVersion;
    std::optional<GithubReleaseSelection> selected;

    for (const QJsonValue& value : releases) {
        const QJsonObject release = value.toObject();
        if (release.value("draft").toBool()) {
            continue;
        }

        const auto version =
            SemanticVersion::parse(release.value("tag_name").toString());
        // AI-generated: enforce the selected channel from both data sources.
        if (!version ||
            (!includePrereleases && (release.value("prerelease").toBool() ||
                                     version->isPrerelease()))) {
            continue;
        }

        QString downloadUrl;
        for (const QJsonValue& assetValue : release.value("assets").toArray()) {
            const QJsonObject asset = assetValue.toObject();
            if (assetRegex.match(asset.value("name").toString()).hasMatch()) {
                downloadUrl = asset.value("browser_download_url").toString();
                break;
            }
        }
        if (downloadUrl.isEmpty()) {
            continue;
        }

        if (!selectedVersion || *version > *selectedVersion) {
            selectedVersion = version;
            selected = GithubReleaseSelection{version->toString(), downloadUrl};
        }
    }
    return selected;
}
