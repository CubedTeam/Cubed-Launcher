#pragma once
#include <QString>
class QNetworkRequest;
namespace GitHubAuth {
QString current_token();
bool has_token();
void apply_to_request(QNetworkRequest& req);
} // namespace GitHubAuth
