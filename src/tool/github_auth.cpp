#include "tool/github_auth.hpp"

#include "settings.hpp"

#include <QByteArray>
#include <QNetworkRequest>
#include <QProcessEnvironment>
#include <QString>

namespace GitHubAuth {

QString current_token() {
    const QString env = QProcessEnvironment::systemEnvironment()
                            .value(QStringLiteral("CUBED_GITHUB_TOKEN"))
                            .trimmed();
    if (!env.isEmpty()) {
        return env;
    }
    if (Settings* s = Settings::instance()) {
        return s->github_token().trimmed();
    }
    return {};
}

bool has_token() { return !current_token().isEmpty(); }

void apply_to_request(QNetworkRequest& req) {
    const QString token = current_token();
    if (token.isEmpty()) {
        return;
    }
    req.setRawHeader(QByteArrayLiteral("Authorization"),
                     "Bearer " + token.toUtf8());
}

} // namespace GitHubAuth
