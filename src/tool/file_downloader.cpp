#include "tool/file_downloader.hpp"

#include "tool/user_agent.hpp"

#include <QFile>
#include <QNetworkRequest>

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
        emit has_error_changed();
        emit error_message_changed();
        m_progress = 1.0f;
        emit progress_changed();
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
        emit has_error_changed();
        emit error_message_changed();
        m_progress = 1.0f;
        emit progress_changed();
        end_run();
        return false;
    }

    connect(reply, &QNetworkReply::readyRead, reply,
            [reply, file]() { file->write(reply->readAll()); });
    connect(reply, &QNetworkReply::downloadProgress, this,
            [this](qint64 received, qint64 total) {
                if (total > 0) {
                    m_progress = float(received) / float(total);
                    emit progress_changed();
                }
            });
    connect(reply, &QNetworkReply::finished, this, [this, file, save_path]() {
        file->close();
        m_reply = nullptr;
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
    if (m_cancelling) {
        m_cancelling = false;
        QFile::remove(m_save_path);
        m_progress = 0.0f;
        emit progress_changed();
        end_run();
        emit download_cancelled(m_save_path);
        return;
    }
    m_has_error = true;
    m_error_message = m_reply ? m_reply->errorString()
                              : QStringLiteral("Unknown network error");
    if (m_reply) {
        m_reply->deleteLater();
    }
    QFile::remove(m_save_path);
    emit has_error_changed();
    emit error_message_changed();
    m_progress = 1.0f;
    emit progress_changed();
    end_run();
}

void FileDownloader::mark_succeeded() {
    if (!m_downloading) {
        return;
    }
    m_progress = 1.0f;
    emit progress_changed();
    m_finished = true;
    emit download_finished_changed();
    end_run();
}

void FileDownloader::report_error(const QString& message) {
    if (!m_downloading) {
        return;
    }
    m_has_error = true;
    m_error_message = message;
    emit has_error_changed();
    emit error_message_changed();
    m_progress = 1.0f;
    emit progress_changed();
    end_run();
}

void FileDownloader::set_error_state(const QString& message) {
    m_has_error = true;
    m_error_message = message;
    emit has_error_changed();
    emit error_message_changed();
}

void FileDownloader::clear_error_state() {
    if (!m_has_error && m_error_message.isEmpty()) {
        return;
    }
    m_has_error = false;
    m_error_message.clear();
    emit has_error_changed();
    emit error_message_changed();
}

void FileDownloader::begin_run() {
    m_downloading = true;
    m_cancelling = false;
    m_finished = false;
    m_has_error = false;
    m_error_message.clear();
    m_progress = 0.0f;
    emit downloading_changed();
    emit download_finished_changed();
    emit has_error_changed();
    emit error_message_changed();
    emit progress_changed();
}

void FileDownloader::end_run() {
    m_downloading = false;
    emit downloading_changed();
}
