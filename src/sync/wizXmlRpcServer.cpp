#include "wizXmlRpcServer.h"

//#include "wizdef.h"
#include "../share/wizmisc.h"

#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkProxy>

CWizXmlRpcEventLoop::CWizXmlRpcEventLoop(QNetworkReply* pReply, QObject *parent /*= 0*/)
    : QEventLoop(parent)
    , m_error(QNetworkReply::NoError)
{
    connect(pReply, SIGNAL(finished()), SLOT(on_replyFinished()));
    connect(pReply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(on_replyError(QNetworkReply::NetworkError)));
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
    if (strContentType != "text/xml;charset=UTF-8") {
        m_error = QNetworkReply::ProtocolFailure;
        m_errorString = "Invalid content type of response";
        return;
    }

    m_result = QString::fromUtf8(reply->readAll().constData());

}

void CWizXmlRpcEventLoop::doError(QNetworkReply::NetworkError error)
{
    m_error = error;
}

void CWizXmlRpcEventLoop::on_replyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());
    //
    doFinished(reply);

    reply->deleteLater();
    //
    quit();
}

void CWizXmlRpcEventLoop::on_replyError(QNetworkReply::NetworkError error)
{
    doError(error);
    //
    quit();
}


CWizXmlRpcServerBase::CWizXmlRpcServerBase(const QString& strUrl, QObject* parent)
    : QObject(parent)
    , m_strUrl(strUrl)
    , m_nLastErrorCode(0)
{
    m_network = new QNetworkAccessManager(this);
}
QString CWizXmlRpcServerBase::GetURL() const
{
    return m_strUrl;
}

int CWizXmlRpcServerBase::GetLastErrorCode()
{
    return m_nLastErrorCode;
}
QString CWizXmlRpcServerBase::GetLastErrorMessage()
{
    return m_strLastErrorMessage;
}


bool CWizXmlRpcServerBase::xmlRpcCall(const QString& strMethodName, CWizXmlRpcResult& result, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2 /*= NULL*/, CWizXmlRpcValue* pParam3 /*= NULL*/, CWizXmlRpcValue* pParam4 /*= NULL*/)
{
    CWizXmlRpcRequest data(strMethodName);
    data.addParam(pParam1);
    if (pParam2)
    {
        data.addParam(pParam2);
    }
    if (pParam3)
    {
        data.addParam(pParam3);
    }
    if (pParam4)
    {
        data.addParam(pParam4);
    }

    QNetworkRequest request;
    request.setUrl(QUrl(m_strUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/xml"));
    //

    QNetworkReply* reply = m_network->post(request, data.toData());
    CWizXmlRpcEventLoop loop(reply);
    loop.exec();
    //
    if (loop.error())
    {
        m_nLastErrorCode = loop.error();
        m_strLastErrorMessage = loop.errorString();
        return false;
    }
    //
    QString strXml = loop.result();
    //
    CWizXMLDocument doc;
    if (!doc.LoadXML(strXml)) {
        m_nLastErrorCode = -1;
        m_strLastErrorMessage = "Invalid xml";
        return false;
    }

    CWizXmlRpcValue* pRet = NULL;

    if (!WizXmlRpcResultFromXml(doc, &pRet)) {
        m_nLastErrorCode = -1;
        m_strLastErrorMessage = "Can not parse xmlrpc";
        return false;
    }

    Q_ASSERT(pRet);

    if (CWizXmlRpcFaultValue* pFault = dynamic_cast<CWizXmlRpcFaultValue *>(pRet)) {
        m_nLastErrorCode = pFault->GetFaultCode();
        m_strLastErrorMessage = pFault->GetFaultString();
        return false;
    }
    //
    result.SetResult(strMethodName, pRet);
    //
    return true;
}

BOOL CWizXmlRpcServerBase::Call(const QString& strMethodName, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2 /*= NULL*/, CWizXmlRpcValue* pParam3 /*= NULL*/, CWizXmlRpcValue* pParam4 /*= NULL*/)
{
    CWizXmlRpcResult ret;
    if (!xmlRpcCall(strMethodName, ret, pParam1, pParam2, pParam3, pParam4))
    {
        TOLOG1(_T("Failed to call xml-rpc method: %1"), strMethodName);
        return FALSE;
    }
    //
    return TRUE;
}


BOOL CWizXmlRpcServerBase::Call(const QString& strMethodName, CWizXmlRpcResult& ret, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2 /*= NULL*/, CWizXmlRpcValue* pParam3 /*= NULL*/, CWizXmlRpcValue* pParam4 /*= NULL*/)
{
    if (!xmlRpcCall(strMethodName, ret, pParam1, pParam2, pParam3, pParam4))
    {
        TOLOG1(_T("Failed to call xml-rpc method: %1"), strMethodName);
        return FALSE;
    }
    //
    return TRUE;
}

BOOL CWizXmlRpcServerBase::Call(const QString& strMethodName, std::map<QString, QString>& mapRet, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2 /*= NULL*/, CWizXmlRpcValue* pParam3 /*= NULL*/, CWizXmlRpcValue* pParam4 /*= NULL*/)
{
    CWizXmlRpcResult ret;
    if (!Call(strMethodName, ret, pParam1, pParam2, pParam3, pParam4))
        return FALSE;
    //
    CWizXmlRpcStructValue* pValue = ret.GetResultValue<CWizXmlRpcStructValue>();
    if (!pValue)
    {
        TOLOG1(_T("The return value of XmpRpc method %1 is not a struct!"), strMethodName);
        return FALSE;
    }
    //
    return pValue->ToStringMap(mapRet);
}

BOOL CWizXmlRpcServerBase::GetReturnValueInStringMap(const QString& strMethodName, std::map<QString, QString>& mapRet, const QString& strName, QString& strValue)
{
    std::map<QString, QString>::const_iterator it = mapRet.find(strName);
    if (it == mapRet.end())
    {
        TOLOG2(_T("Can not find member of %1 in the return value of xml-rpc method %2"), strName, strMethodName);
        return FALSE;
    }
    //
    strValue = it->second;
    //
    return TRUE;
}


BOOL CWizXmlRpcServerBase::Call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2 /*= NULL*/, CWizXmlRpcValue* pParam3 /*= NULL*/, CWizXmlRpcValue* pParam4 /*= NULL*/)
{
    std::map<QString, QString> mapRet;
    if (!Call(strMethodName, mapRet, pParam1, pParam2, pParam3, pParam4))
        return FALSE;
    //
    return GetReturnValueInStringMap(strMethodName, mapRet, strRetName1, strRetValue1);
}
BOOL CWizXmlRpcServerBase::Call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, const QString& strRetName2, QString& strRetValue2, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2 /*= NULL*/, CWizXmlRpcValue* pParam3 /*= NULL*/, CWizXmlRpcValue* pParam4 /*= NULL*/)
{
    std::map<QString, QString> mapRet;
    if (!Call(strMethodName, mapRet, pParam1, pParam2, pParam3, pParam4))
        return FALSE;
    //
    return GetReturnValueInStringMap(strMethodName, mapRet, strRetName1, strRetValue1)
        && GetReturnValueInStringMap(strMethodName, mapRet, strRetName2, strRetValue2);
}
BOOL CWizXmlRpcServerBase::Call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, const QString& strRetName2, QString& strRetValue2, const QString& strRetName3, QString& strRetValue3, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2 /*= NULL*/, CWizXmlRpcValue* pParam3 /*= NULL*/, CWizXmlRpcValue* pParam4 /*= NULL*/)
{
    std::map<QString, QString> mapRet;
    if (!Call(strMethodName, mapRet, pParam1, pParam2, pParam3, pParam4))
        return FALSE;
    //
    return GetReturnValueInStringMap(strMethodName, mapRet, strRetName1, strRetValue1)
        && GetReturnValueInStringMap(strMethodName, mapRet, strRetName2, strRetValue2)
        && GetReturnValueInStringMap(strMethodName, mapRet, strRetName3, strRetValue3);
}
BOOL CWizXmlRpcServerBase::Call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, const QString& strRetName2, QString& strRetValue2, const QString& strRetName3, QString& strRetValue3, const QString& strRetName4, QString& strRetValue4, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2 /*= NULL*/, CWizXmlRpcValue* pParam3 /*= NULL*/, CWizXmlRpcValue* pParam4 /*= NULL*/)
{
    std::map<QString, QString> mapRet;
    if (!Call(strMethodName, mapRet, pParam1, pParam2, pParam3, pParam4))
        return FALSE;
    //
    return GetReturnValueInStringMap(strMethodName, mapRet, strRetName1, strRetValue1)
        && GetReturnValueInStringMap(strMethodName, mapRet, strRetName2, strRetValue2)
        && GetReturnValueInStringMap(strMethodName, mapRet, strRetName3, strRetValue3)
        && GetReturnValueInStringMap(strMethodName, mapRet, strRetName4, strRetValue4);
}


#if 0

CWizXmlRpcServer::CWizXmlRpcServer(const QString& strUrl, QObject* parent /* = 0 */)
    : QObject(parent)
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

bool CWizXmlRpcServer::xmlRpcCall(CWizXmlRpcValue* pParam,
                                  const QString& strMethodName,
                                  const QString& arg1 /* = "" */,
                                  const QString& arg2 /* = "" */)
{
    m_strMethodName = strMethodName;
    m_arg1 = arg1;
    m_arg2 = arg2;

    CWizXmlRpcRequest data(strMethodName);
    data.addParam(pParam);

    QNetworkRequest request;
    request.setUrl(QUrl(m_strUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/xml"));

    QNetworkReply* reply = m_network->post(request, data.toData());
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
    Q_EMIT xmlRpcReturn(ret, m_strMethodName, m_arg1, m_arg2);
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

    QString strXml = QString::fromUtf8(reply->readAll().constData());

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
    }

    processReturn(*pRet);

    reply->deleteLater();
}

void CWizXmlRpcServer::on_replyError(QNetworkReply::NetworkError error)
{
    processError(errorNetwork, error, QString());
}

void CWizXmlRpcServer::on_replyDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit xmlRpcReadProgress(bytesReceived, bytesTotal);
}

#endif
