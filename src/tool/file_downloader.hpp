#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QPointer>
#include <QString>

// AI-generated: shared download state machine + QNetworkReply plumbing for
// GameUpdate and LauncherUpdate. The user provides a save path; on success
// the download_complete() signal carries the path so the caller can do
// post-processing (extract / install) and then call mark_succeeded() or
// report_error() to clear the busy state.
class FileDownloader : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloading_changed)
    Q_PROPERTY(float progress READ progress NOTIFY progress_changed)
    Q_PROPERTY(bool downloadFinished READ download_finished NOTIFY
                   download_finished_changed)
    Q_PROPERTY(bool hasError READ has_error NOTIFY has_error_changed)
    Q_PROPERTY(
        QString errorMessage READ error_message NOTIFY error_message_changed)

public:
    explicit FileDownloader(QNetworkAccessManager* manager,
                            QObject* parent = nullptr);
    ~FileDownloader() override;

    // Begin a download. Returns false if already downloading, if the URL is
    // empty, or if the local file cannot be opened. On the empty-URL /
    // open-failure paths the error / progress state is set and the UI
    // will see downloading pulse to refresh controls.
    bool start(const QString& url, const QString& save_path);

    // Abort the in-flight download; finished handler clears state.
    void cancel();

    bool downloading() const { return m_downloading; }
    bool cancelling() const { return m_cancelling; }
    float progress() const { return m_progress; }
    bool download_finished() const { return m_finished; }
    bool has_error() const { return m_has_error; }
    QString error_message() const { return m_error_message; }

Q_SIGNALS:
    void downloading_changed();
    void progress_changed();
    void download_finished_changed();
    void has_error_changed();
    void error_message_changed();
    // The save path is fully written and the response has no network error.
    // Caller must invoke mark_succeeded() or report_error() to clear the
    // busy state once post-processing is done.
    void download_complete(const QString& save_path);
    // The save path has been removed and state has been cleared.
    void download_cancelled(const QString& save_path);

public Q_SLOTS:
    // Clear busy state after a successful post-processing step.
    void mark_succeeded();
    // Report a post-processing failure; clears busy state and sets error.
    void report_error(const QString& message);
    // Set the error message without affecting the downloading state. Used
    // to surface errors from non-download phases (e.g. version check).
    void set_error_state(const QString& message);
    // Clear any pending error message. begin_run() already does this on
    // a new download, so callers usually only need this when bailing out
    // before starting one.
    void clear_error_state();

private:
    void on_reply_finished();
    void begin_run();
    void end_run();

    QNetworkAccessManager* m_manager;
    QPointer<QNetworkReply> m_reply;
    QString m_save_path;
    bool m_downloading = false;
    bool m_cancelling = false;
    bool m_finished = false;
    bool m_has_error = false;
    QString m_error_message;
    float m_progress = 0.0f;
};
