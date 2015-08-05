#ifndef CWIZEVENTLOOP_H
#define CWIZEVENTLOOP_H

#include <QEventLoop>
#include <QNetworkReply>
#include <QTimer>

class CWizAutoTimeOutEventLoop : public QEventLoop
{
    Q_OBJECT
public:
    explicit CWizAutoTimeOutEventLoop(QNetworkReply* pReply, QObject *parent = 0);
    void setTimeoutWaitSeconds(int seconds);

public:
    QNetworkReply::NetworkError error() { return m_error; }
    QString errorString() { return m_errorString; }
    QString result() { return m_result; }
    bool timeOut() { return m_timeOut; }

    int exec(ProcessEventsFlags flags = AllEvents);

public Q_SLOTS:
    void on_replyFinished();
    void on_replyError(QNetworkReply::NetworkError);
    void on_timeOut();
    void on_downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void on_uploadProgress(qint64 bytesSent, qint64 bytesTotal);


protected:
    virtual void doFinished(QNetworkReply* reply);
    virtual void doError(QNetworkReply::NetworkError error);

    QString m_result;
    QNetworkReply::NetworkError m_error;
    QString m_errorString;
    bool m_timeOut;
    int m_timeOutSeconds;
    qint64 m_downloadBytes;
    qint64 m_lastDownloadBytes;
    qint64 m_uploadBytes;
    qint64 m_lastUploadBytes;
    QTimer m_timer;
};


class CWizXmlRpcEventLoop : public CWizAutoTimeOutEventLoop
{
    Q_OBJECT
public:
    explicit CWizXmlRpcEventLoop(QNetworkReply* pReply, QObject *parent = 0);

protected:
    virtual void doFinished(QNetworkReply* reply);
};

#endif // CWIZEVENTLOOP_H
