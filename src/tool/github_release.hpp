#pragma once
#include <QObject>
#include <QRegularExpression>
#include <QString>
#include <functional>

class QNetworkAccessManager;
class QNetworkReply;

class GithubReleaseFetcher : public QObject {
    Q_OBJECT
public:
    struct Result {
        bool ok = false;
        QString version;
        QString downloadUrl;
        QString errorMessage;
    };
    using Callback = std::function<void(Result)>;

    explicit GithubReleaseFetcher(QNetworkAccessManager* manager,
                                  QObject* parent = nullptr);
    ~GithubReleaseFetcher() override;

    bool fetch(const QString& owner, const QString& repo,
               const QRegularExpression& assetRegex, Callback callback);

private:
    QNetworkAccessManager* m_manager;
    QNetworkReply* m_reply = nullptr;
    Callback m_callback;
};
