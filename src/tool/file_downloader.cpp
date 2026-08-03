#include "tool/file_downloader.hpp"

#include "tool/user_agent.hpp"

#include <QFile>
#include <QNetworkRequest>
#include <QProcessEnvironment>
FileDownloader::FileDownloader(QNetworkAccessManager* manager, QObject* parent)
    : QObject(parent), m_manager(manager) {}

FileDownloader::~FileDownloader() {
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
}

bool FileDownloader::start(const QString& url, const QString& save_path) {
    if (m_downloading) {
        return false;
    }
    begin_run();
    m_save_path = save_path;

    if (url.isEmpty()) {
        m_has_error = true;
        m_error_message = QStringLiteral("Download Url is Empty!");
        Q_EMIT has_error_changed();
        Q_EMIT error_message_changed();
        m_progress = 1.0f;
        Q_EMIT progress_changed();
        end_run();
        return false;
    }

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, buildUserAgent().toUtf8());
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    auto* reply = m_manager->get(req);
    m_reply = reply;

    auto file = std::make_shared<QFile>(save_path);
    if (!file->open(QIODevice::WriteOnly)) {
        m_reply = nullptr;
        reply->abort();
        reply->deleteLater();
        m_has_error = true;
        m_error_message = QStringLiteral("Can't open file");
        Q_EMIT has_error_changed();
        Q_EMIT error_message_changed();
        m_progress = 1.0f;
        Q_EMIT progress_changed();
        end_run();
        return false;
    }

    connect(reply, &QNetworkReply::readyRead, reply,
            [reply, file]() { file->write(reply->readAll()); });
    connect(reply, &QNetworkReply::downloadProgress, this,
            [this](qint64 received, qint64 total) {
                if (total > 0) {
                    m_progress = float(received) / float(total);
                    Q_EMIT progress_changed();
                }
            });
    connect(reply, &QNetworkReply::finished, this, [this, file, save_path]() {
        file->close();
        on_reply_finished();
    });
    return true;
}

void FileDownloader::cancel() {
    if (!m_downloading || m_cancelling || !m_reply) {
        return;
    }
    m_cancelling = true;
    m_reply->abort();
}

void FileDownloader::on_reply_finished() {
    // AI-generated: snapshot reply, then null m_reply before deleteLater
    // so reentrant access sees a clean state and the reply is always freed.
    QPointer<QNetworkReply> reply = m_reply;
    m_reply = nullptr;
    if (reply) {
        reply->deleteLater();
    }

    if (m_cancelling) {
        m_cancelling = false;
        QFile::remove(m_save_path);
        m_progress = 0.0f;
        Q_EMIT progress_changed();
        end_run();
        Q_EMIT download_cancelled(m_save_path);
        return;
    }

    if (reply && reply->error() == QNetworkReply::NoError) {
        m_progress = 1.0f;
        Q_EMIT progress_changed();
        Q_EMIT download_complete(m_save_path);
        return;
    }

    m_has_error = true;
    m_error_message =
        reply ? reply->errorString() : QStringLiteral("Unknown network error");
    QFile::remove(m_save_path);
    Q_EMIT has_error_changed();
    Q_EMIT error_message_changed();
    m_progress = 1.0f;
    Q_EMIT progress_changed();
    end_run();
}

void FileDownloader::mark_succeeded() {
    if (!m_downloading) {
        return;
    }
    m_progress = 1.0f;
    Q_EMIT progress_changed();
    m_finished = true;
    Q_EMIT download_finished_changed();
    end_run();
}

void FileDownloader::report_error(const QString& message) {
    if (!m_downloading) {
        return;
    }
    m_has_error = true;
    m_error_message = message;
    Q_EMIT has_error_changed();
    Q_EMIT error_message_changed();
    m_progress = 1.0f;
    Q_EMIT progress_changed();
    end_run();
}

void FileDownloader::set_error_state(const QString& message) {
    m_has_error = true;
    m_error_message = message;
    Q_EMIT has_error_changed();
    Q_EMIT error_message_changed();
}

void FileDownloader::clear_error_state() {
    if (!m_has_error && m_error_message.isEmpty()) {
        return;
    }
    m_has_error = false;
    m_error_message.clear();
    Q_EMIT has_error_changed();
    Q_EMIT error_message_changed();
}

void FileDownloader::begin_run() {
    m_downloading = true;
    m_cancelling = false;
    m_finished = false;
    m_has_error = false;
    m_error_message.clear();
    m_progress = 0.0f;
    Q_EMIT downloading_changed();
    Q_EMIT download_finished_changed();
    Q_EMIT has_error_changed();
    Q_EMIT error_message_changed();
    Q_EMIT progress_changed();
}

void FileDownloader::end_run() {
    m_downloading = false;
    Q_EMIT downloading_changed();
}
