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

    GithubReleaseFetcher(QNetworkAccessManager* manager, QStringView name,
                         QObject* parent = nullptr);
    ~GithubReleaseFetcher() override;

    bool fetch(const QString& owner, const QString& repo,
               const QRegularExpression& assetRegex, Callback callback);

private:
    QNetworkAccessManager* m_manager;
    QNetworkReply* m_reply = nullptr;
    const QString m_name;
    Callback m_callback;
};
