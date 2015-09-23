#include "wizEventLoop.h"

#include <QTimer>
#include <QDebug>

const int TIMEOUT_WAIT_SECONDS = 60;

CWizAutoTimeOutEventLoop::CWizAutoTimeOutEventLoop(QNetworkReply* pReply, QObject *parent /*= 0*/)
    : QEventLoop(parent)
    , m_error(QNetworkReply::NoError)
    , m_timeOut(false)
    , m_timeOutSeconds(TIMEOUT_WAIT_SECONDS)
    , m_downloadBytes(0)
    , m_lastDownloadBytes(-1)
    , m_uploadBytes(0)
    , m_lastUploadBytes(-1)
{
    connect(pReply, SIGNAL(finished()), SLOT(on_replyFinished()));
    connect(pReply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(on_replyError(QNetworkReply::NetworkError)));
    connect(pReply, SIGNAL(downloadProgress(qint64,qint64)), SLOT(on_downloadProgress(qint64,qint64)));
    connect(pReply, SIGNAL(uploadProgress(qint64,qint64)), SLOT(on_uploadProgress(qint64,qint64)));
    connect(&m_timer, SIGNAL(timeout()), SLOT(on_timeOut()));
}

void CWizAutoTimeOutEventLoop::setTimeoutWaitSeconds(int seconds)
{
    m_timeOutSeconds = seconds;
}


void CWizAutoTimeOutEventLoop::doFinished(QNetworkReply* reply)
{
    m_error = reply->error();
    if (m_error) {
        m_errorString = reply->errorString();
        return;
    }
    m_result = QString::fromUtf8(reply->readAll().constData());
}

void CWizAutoTimeOutEventLoop::doError(QNetworkReply::NetworkError error)
{
    m_error = error;
}

int CWizAutoTimeOutEventLoop::exec(QEventLoop::ProcessEventsFlags flags)
{
    m_timer.start(m_timeOutSeconds * 1000);
    return QEventLoop::exec(flags);
}

void CWizAutoTimeOutEventLoop::on_replyFinished()
{
    m_timer.stop();
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());
    //
    doFinished(reply);

    reply->deleteLater();
    //
    quit();
}

void CWizAutoTimeOutEventLoop::on_replyError(QNetworkReply::NetworkError error)
{
    m_timer.stop();
    doError(error);
    //
    quit();
}

void CWizAutoTimeOutEventLoop::on_timeOut()
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
        //
        m_timer.stop();
        quit();
    }
}

void CWizAutoTimeOutEventLoop::on_downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    m_downloadBytes = bytesReceived;
//    qDebug() << "download progress changed  " << bytesReceived << "  totoal  : " << bytesTotal;
}

void CWizAutoTimeOutEventLoop::on_uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    m_uploadBytes = bytesSent;
//    qDebug() << "upload progress changed  " << bytesSent << "  totoal  : " << bytesTotal;
}


CWizXmlRpcEventLoop::CWizXmlRpcEventLoop(QNetworkReply* pReply, QObject* parent)
    : CWizAutoTimeOutEventLoop(pReply, parent)
{
}

void CWizXmlRpcEventLoop::doFinished(QNetworkReply* reply)
{
    m_error = reply->error();
    if (m_error) {
        m_errorString = reply->errorString();
        return;
    }

    //TODO: modify content type checker
    QString strContentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    if (strContentType.remove(' ').toLower() != "text/xml;charset=utf-8") {
        m_error = QNetworkReply::ProtocolFailure;
        m_errorString = "Invalid content type of response";
        return;
    }

    m_result = QString::fromUtf8(reply->readAll().constData());
}
