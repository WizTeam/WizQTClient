#include "wizXmlRpcServer.h"

#include "wizdef.h"


CWizXmlRpcServer::CWizXmlRpcServer(const QString& strUrl)
{
    m_strUrl = strUrl;
    m_network = new QNetworkAccessManager(this);
}

void CWizXmlRpcServer::setProxy(const QString& host, int port, const QString& userName, const QString& password)
{
    QNetworkProxy proxy = m_network->proxy();

    if (host.isEmpty()) {
        proxy.setType(QNetworkProxy::DefaultProxy);
    } else {
        proxy.setHostName(host);
        proxy.setPort(port);
        proxy.setUser(userName);
        proxy.setPassword(password);
        proxy.setType(QNetworkProxy::HttpProxy);
    }

    m_network->setProxy(proxy);
}

void CWizXmlRpcServer::abort()
{
    m_network->disconnect();
}

bool CWizXmlRpcServer::xmlRpcCall(const QString& strMethodName, CWizXmlRpcValue* pParam)
{
    m_strMethodName = strMethodName;

    CWizXmlRpcRequest data(strMethodName);
    data.addParam(pParam);

    m_requestData = data.toData();

    QNetworkRequest request;
    request.setUrl(QUrl(m_strUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/xml"));

    QNetworkReply* reply = m_network->post(request, m_requestData);
    connect(reply, SIGNAL(finished()), SLOT(on_replyFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(on_replyError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), SLOT(on_replyDownloadProgress(qint64, qint64)));

    return true;
}

void CWizXmlRpcServer::processError(WizXmlRpcError error, int errorCode, const QString& errorString)
{
    Q_ASSERT(!m_strMethodName.isEmpty());
    Q_EMIT xmlRpcError(m_strMethodName, error, errorCode, errorString);
}

void CWizXmlRpcServer::processReturn(CWizXmlRpcValue& ret)
{
    Q_ASSERT(!m_strMethodName.isEmpty());
    Q_EMIT xmlRpcReturn(m_strMethodName, ret);
}

void CWizXmlRpcServer::on_replyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());

    if (reply->error()) {
        reply->deleteLater();
        return;
    }

    QString strContentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    if (strContentType != "text/xml;charset=UTF-8") {
        processError(errorContentType, 0, "Invalid content type of response");
        reply->deleteLater();
        return;
    }

    m_replyData = reply->readAll();

#ifdef WIZNOTE_DEBUG
    QString requestFile = QDir::tempPath() + "/WizNote/" + m_strMethodName;
    QString replyFile = QDir::tempPath() + "/WizNote/on_" + m_strMethodName;
    WizSaveUnicodeTextToUtf8File(requestFile, QString::fromUtf8(m_requestData.constData()));
    WizSaveUnicodeTextToUtf8File(replyFile, QString::fromUtf8(m_replyData.constData()));
#endif

    QString strXml = QString::fromUtf8(m_replyData.constData());

    CWizXMLDocument doc;
    if (!doc.LoadXML(strXml)) {
        processError(errorXmlFormat, 0, "Invalid xml");
        reply->deleteLater();
        return;
    }

    CWizXmlRpcValue* pRet = NULL;

    if (!WizXmlRpcResultFromXml(doc, &pRet)) {
        processError(errorXmlRpcFormat, 0, "Can not parse xmlrpc");
        reply->deleteLater();
        return;
    }

    Q_ASSERT(pRet);

    if (CWizXmlRpcFaultValue* pFault = dynamic_cast<CWizXmlRpcFaultValue *>(pRet)) {
        processError(errorXmlRpcFault, pFault->GetFaultCode(), pFault->GetFaultString());
        //delete pRet;
        //return;
    }

    reply->deleteLater();
    processReturn(*pRet);
}

void CWizXmlRpcServer::on_replyError(QNetworkReply::NetworkError error)
{
    processError(errorNetwork, error, QString());
}

void CWizXmlRpcServer::on_replyDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit xmlRpcReadProgress(bytesReceived, bytesTotal);
}
