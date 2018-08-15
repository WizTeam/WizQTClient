#ifndef CWIZEVENTLOOP_H
#define CWIZEVENTLOOP_H

#include <QEventLoop>
#include <QNetworkReply>
#include <QTimer>

/**
 * NOTE: CWizAutoTimeOutEventLoop would delete network reply at destruct,
 * should not delete network reply again
 */

class WizAutoTimeOutEventLoop : public QEventLoop
{
    Q_OBJECT
public:
    explicit WizAutoTimeOutEventLoop(QNetworkReply* pReply, QObject *parent = 0);
    ~WizAutoTimeOutEventLoop();
    void setTimeoutWaitSeconds(int seconds);

public:
    QNetworkReply::NetworkError error() const;
    QString errorString() const;
    const QByteArray& result() const { return m_result; }
    bool timeOut() const { return m_timeOut; }
    bool isFinished() const { return m_finished; }
    QNetworkReply* networkReply() const;

    int exec(ProcessEventsFlags flags = AllEvents);

public Q_SLOTS:
    void on_replyFinished();
    void on_replyError(QNetworkReply::NetworkError);
    void on_timeOut();
    void on_downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void on_uploadProgress(qint64 bytesSent, qint64 bytesTotal);

Q_SIGNALS:
    void downloadProgress(QUrl url, qint64 bytesReceived, qint64 bytesTotal);
protected:
    virtual void doFinished(QNetworkReply* reply);
    virtual void doError(QNetworkReply::NetworkError error);

    QByteArray m_result;
    QNetworkReply* m_reply;
    QNetworkReply::NetworkError m_error;
    QUrl m_url;
    QString m_errorString;
    bool m_timeOut;
    int m_timeOutSeconds;
    qint64 m_downloadBytes;
    qint64 m_lastDownloadBytes;
    qint64 m_uploadBytes;
    qint64 m_lastUploadBytes;
    QTimer m_timer;
    bool m_finished;
};



#endif // CWIZEVENTLOOP_H
