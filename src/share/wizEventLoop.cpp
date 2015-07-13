#include "wizEventLoop.h"

#include <QTimer>
#include <QDebug>

const int TIMEOUT_WAIT_SECONDS = 30;

CWizAutoTimeOutEventLoop::CWizAutoTimeOutEventLoop(QNetworkReply* pReply, QObject *parent /*= 0*/)
    : QEventLoop(parent)
    , m_error(QNetworkReply::NoError)
//    , m_timeOut(false)
//    , m_timeOutWaitSeconds(TIMEOUT_WAIT_SECONDS * 1000)
{
    connect(pReply, SIGNAL(finished()), SLOT(on_replyFinished()));
    connect(pReply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(on_replyError(QNetworkReply::NetworkError)));
}

//void CWizAutoTimeOutEventLoop::setTimeoutWaitSeconds(int seconds)
//{
//    m_timeOutWaitSeconds = seconds * 1000;
//}


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
//    QTimer::singleShot(m_timeOutWaitSeconds, this, SLOT(on_timeOut()));
    return QEventLoop::exec(flags);
}

void CWizAutoTimeOutEventLoop::on_replyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());
    //
    doFinished(reply);

    reply->deleteLater();
    //
    quit();
}

void CWizAutoTimeOutEventLoop::on_replyError(QNetworkReply::NetworkError error)
{
    doError(error);
    //
    quit();
}

//void CWizAutoTimeOutEventLoop::on_timeOut()
//{
//    m_timeOut = true;
//    m_error = QNetworkReply::TimeoutError;
//    m_errorString = "Event loop time out, can not get response from network reply";
//    qDebug() << "[sync]Xml rpc event loop time out";
//    //
//    quit();
//}



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
