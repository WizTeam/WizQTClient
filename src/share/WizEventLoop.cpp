#include "WizEventLoop.h"

#include <QTimer>
#include <QDebug>

const int TIMEOUT_WAIT_SECONDS = 120;

WizAutoTimeOutEventLoop::WizAutoTimeOutEventLoop(QNetworkReply* pReply, QObject *parent /*= 0*/)
    : QEventLoop(parent)
    , m_error(QNetworkReply::NoError)
    , m_timeOut(false)
    , m_timeOutSeconds(TIMEOUT_WAIT_SECONDS)
    , m_downloadBytes(0)
    , m_lastDownloadBytes(-1)
    , m_uploadBytes(0)
    , m_lastUploadBytes(-1)
    , m_reply(pReply)
    , m_finished(false)
{
    m_url = pReply->request().url();
    connect(pReply, SIGNAL(finished()), SLOT(on_replyFinished()));
    connect(pReply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(on_replyError(QNetworkReply::NetworkError)));
    connect(pReply, SIGNAL(downloadProgress(qint64,qint64)), SLOT(on_downloadProgress(qint64,qint64)));
    connect(pReply, SIGNAL(uploadProgress(qint64,qint64)), SLOT(on_uploadProgress(qint64,qint64)));
    connect(&m_timer, SIGNAL(timeout()), SLOT(on_timeOut()));
}

WizAutoTimeOutEventLoop::~WizAutoTimeOutEventLoop()
{
    if (m_reply)
    {
        m_reply->close();
        m_reply->deleteLater();
    }
}

QNetworkReply::NetworkError WizAutoTimeOutEventLoop::error() const
{
    if (m_error != QNetworkReply::NoError)
        return m_error;
    //
    if (!m_finished) {
        return QNetworkReply::OperationCanceledError;
    }
    //
    return m_error;
}
QString WizAutoTimeOutEventLoop::errorString() const
{
    return m_errorString;
}

void WizAutoTimeOutEventLoop::setTimeoutWaitSeconds(int seconds)
{
    m_timeOutSeconds = seconds;
}

QNetworkReply*WizAutoTimeOutEventLoop::networkReply() const
{
    return m_reply;
}

void WizAutoTimeOutEventLoop::doFinished(QNetworkReply* reply)
{
    m_finished = true;
    if (m_error == QNetworkReply::NoError) {
        m_error = reply->error();
        if (m_error) {
            m_errorString = reply->errorString();
            return;
        }
        m_result = reply->readAll();
    }
}

void WizAutoTimeOutEventLoop::doError(QNetworkReply::NetworkError error)
{
    m_error = error;
}

int WizAutoTimeOutEventLoop::exec(QEventLoop::ProcessEventsFlags flags)
{
    m_timer.start(m_timeOutSeconds * 1000);
    return QEventLoop::exec(flags);
}

void WizAutoTimeOutEventLoop::on_replyFinished()
{
    m_timer.stop();
    //
    doFinished(m_reply);
    //
    quit();
}

void WizAutoTimeOutEventLoop::on_replyError(QNetworkReply::NetworkError error)
{
    m_timer.stop();
    doError(error);
    //
    quit();
}

void WizAutoTimeOutEventLoop::on_timeOut()
{
    qDebug() << "network status check, " << "downloaded bytes : " << m_downloadBytes << "  last time : " << m_lastDownloadBytes
             << " uploaded bytes : " << m_uploadBytes << " last time : " << m_lastUploadBytes;
    if (m_downloadBytes != m_lastDownloadBytes)
    {
        m_lastDownloadBytes = m_downloadBytes;
    }
    else if (m_uploadBytes != m_lastUploadBytes)
    {
        m_lastUploadBytes = m_uploadBytes;
    }
    else
    {
        m_timeOut = true;
        m_error = QNetworkReply::TimeoutError;
        m_errorString = "Event loop time out, can not get response from network reply";
        qWarning() << "[sync]Xml rpc event loop time out";
        m_timer.stop();
        m_reply->abort();
        //
        quit();
    }
}

void WizAutoTimeOutEventLoop::on_downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    m_downloadBytes = bytesReceived;
//    qDebug() << "download progress changed  " << bytesReceived << "  totoal  : " << bytesTotal;
    emit downloadProgress(m_url, bytesReceived, bytesTotal);
}

void WizAutoTimeOutEventLoop::on_uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    m_uploadBytes = bytesSent;
//    qDebug() << "upload progress changed  " << bytesSent << "  totoal  : " << bytesTotal;
}
