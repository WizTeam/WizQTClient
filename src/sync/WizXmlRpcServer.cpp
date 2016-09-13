#include "WizXmlRpcServer.h"
#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QTimer>

#include "share/WizEventLoop.h"


WizXmlRpcServerBase::WizXmlRpcServerBase(const QString& strUrl, QObject* parent)
    : QObject(parent)
    , m_strUrl(strUrl)
    , m_nLastErrorCode(0)
{
    m_network = new QNetworkAccessManager(this);
}
QString WizXmlRpcServerBase::getURL() const
{
    return m_strUrl;
}

int WizXmlRpcServerBase::getLastErrorCode() const
{
    return m_nLastErrorCode;
}
QString WizXmlRpcServerBase::getLastErrorMessage() const
{
    return m_strLastErrorMessage;
}

bool WizXmlRpcServerBase::xmlRpcCall(const QString& strMethodName, WizXmlRpcResult& result, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 /*= NULL*/, WizXmlRpcValue* pParam3 /*= NULL*/, WizXmlRpcValue* pParam4 /*= NULL*/)
{
    WizXmlRpcRequest data(strMethodName);
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

    int nCounter = 0;
    while (true)
    {
        QNetworkReply* reply = m_network->post(request, data.toData());
        WizXmlRpcEventLoop loop(reply);
//        qDebug() << "[Sync]Start a xml rpc event loop";
        loop.exec();
//        qDebug() << "[Sync]Xml rpc event loop finished";
        //
        if (loop.error() && nCounter == 0)
        {
            nCounter ++;
            continue;
        }
        //
        if (loop.error())
        {
            m_nLastErrorCode = loop.error();
            m_strLastErrorMessage = loop.errorString();
            return false;
        }
        //
        QString strXml = QString::fromUtf8(loop.result().constData());
        //
        WizXMLDocument doc;
        if (!doc.loadXML(strXml)) {
            m_nLastErrorCode = -1;
            m_strLastErrorMessage = "Invalid xml";
            return false;
        }

        WizXmlRpcValue* pRet = NULL;

        if (!WizXmlRpcResultFromXml(doc, &pRet)) {
            m_nLastErrorCode = -1;
            m_strLastErrorMessage = "Can not parse xmlrpc";
            return false;
        }

        Q_ASSERT(pRet);

        if (WizXmlRpcFaultValue* pFault = dynamic_cast<WizXmlRpcFaultValue *>(pRet)) {
            m_nLastErrorCode = pFault->getFaultCode();
            m_strLastErrorMessage = pFault->getFaultString();
            TOLOG2("XmlRpcCall failed : %1, %2", QString::number(m_nLastErrorCode), m_strLastErrorMessage);
            return false;
        }
        //
        result.setResult(strMethodName, pRet);
        break;
    }
    //
    return true;
}

BOOL WizXmlRpcServerBase::call(const QString& strMethodName, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 /*= NULL*/, WizXmlRpcValue* pParam3 /*= NULL*/, WizXmlRpcValue* pParam4 /*= NULL*/)
{
    WizXmlRpcResult ret;
    if (!xmlRpcCall(strMethodName, ret, pParam1, pParam2, pParam3, pParam4))
    {
        TOLOG3("Failed to call xml-rpc method: %1 , error code : %2 , error message : %3"
               , strMethodName, QString::number(m_nLastErrorCode), m_strLastErrorMessage);
        return FALSE;
    }
    //
    return TRUE;
}


BOOL WizXmlRpcServerBase::call(const QString& strMethodName, WizXmlRpcResult& ret, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 /*= NULL*/, WizXmlRpcValue* pParam3 /*= NULL*/, WizXmlRpcValue* pParam4 /*= NULL*/)
{
    if (!xmlRpcCall(strMethodName, ret, pParam1, pParam2, pParam3, pParam4))
    {
        TOLOG3("Failed to call xml-rpc method: %1 , error code : %2 , error message : %3"
               , strMethodName, QString::number(m_nLastErrorCode), m_strLastErrorMessage);
        return FALSE;
    }
    //
    return TRUE;
}

BOOL WizXmlRpcServerBase::call(const QString& strMethodName, std::map<QString, QString>& mapRet, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 /*= NULL*/, WizXmlRpcValue* pParam3 /*= NULL*/, WizXmlRpcValue* pParam4 /*= NULL*/)
{
    WizXmlRpcResult ret;
    if (!call(strMethodName, ret, pParam1, pParam2, pParam3, pParam4))
        return FALSE;
    //
    WizXmlRpcStructValue* pValue = ret.getResultValue<WizXmlRpcStructValue>();
    if (!pValue)
    {
        TOLOG1("The return value of XmpRpc method %1 is not a struct!", strMethodName);
        return FALSE;
    }
    //
    return pValue->toStringMap(mapRet);
}

BOOL WizXmlRpcServerBase::getReturnValueInStringMap(const QString& strMethodName, std::map<QString, QString>& mapRet, const QString& strName, QString& strValue)
{
    std::map<QString, QString>::const_iterator it = mapRet.find(strName);
    if (it == mapRet.end())
    {
        TOLOG2("Can not find member of %1 in the return value of xml-rpc method %2", strName, strMethodName);
        return FALSE;
    }
    //
    strValue = it->second;
    //
    return TRUE;
}


BOOL WizXmlRpcServerBase::call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 /*= NULL*/, WizXmlRpcValue* pParam3 /*= NULL*/, WizXmlRpcValue* pParam4 /*= NULL*/)
{
    std::map<QString, QString> mapRet;
    if (!call(strMethodName, mapRet, pParam1, pParam2, pParam3, pParam4))
        return FALSE;
    //
    return getReturnValueInStringMap(strMethodName, mapRet, strRetName1, strRetValue1);
}
BOOL WizXmlRpcServerBase::call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, const QString& strRetName2, QString& strRetValue2, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 /*= NULL*/, WizXmlRpcValue* pParam3 /*= NULL*/, WizXmlRpcValue* pParam4 /*= NULL*/)
{
    std::map<QString, QString> mapRet;
    if (!call(strMethodName, mapRet, pParam1, pParam2, pParam3, pParam4))
        return FALSE;
    //
    return getReturnValueInStringMap(strMethodName, mapRet, strRetName1, strRetValue1)
        && getReturnValueInStringMap(strMethodName, mapRet, strRetName2, strRetValue2);
}
BOOL WizXmlRpcServerBase::call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, const QString& strRetName2, QString& strRetValue2, const QString& strRetName3, QString& strRetValue3, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 /*= NULL*/, WizXmlRpcValue* pParam3 /*= NULL*/, WizXmlRpcValue* pParam4 /*= NULL*/)
{
    std::map<QString, QString> mapRet;
    if (!call(strMethodName, mapRet, pParam1, pParam2, pParam3, pParam4))
        return FALSE;
    //
    return getReturnValueInStringMap(strMethodName, mapRet, strRetName1, strRetValue1)
        && getReturnValueInStringMap(strMethodName, mapRet, strRetName2, strRetValue2)
        && getReturnValueInStringMap(strMethodName, mapRet, strRetName3, strRetValue3);
}
BOOL WizXmlRpcServerBase::call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, const QString& strRetName2, QString& strRetValue2, const QString& strRetName3, QString& strRetValue3, const QString& strRetName4, QString& strRetValue4, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 /*= NULL*/, WizXmlRpcValue* pParam3 /*= NULL*/, WizXmlRpcValue* pParam4 /*= NULL*/)
{
    std::map<QString, QString> mapRet;
    if (!call(strMethodName, mapRet, pParam1, pParam2, pParam3, pParam4))
        return FALSE;
    //
    return getReturnValueInStringMap(strMethodName, mapRet, strRetName1, strRetValue1)
        && getReturnValueInStringMap(strMethodName, mapRet, strRetName2, strRetValue2)
        && getReturnValueInStringMap(strMethodName, mapRet, strRetName3, strRetValue3)
        && getReturnValueInStringMap(strMethodName, mapRet, strRetName4, strRetValue4);
}


#if 0

WizXmlRpcServer::WizXmlRpcServer(const QString& strUrl, QObject* parent /* = 0 */)
    : QObject(parent)
{
    m_strUrl = strUrl;
    m_network = new QNetworkAccessManager(this);
}

void WizXmlRpcServer::setProxy(const QString& host, int port, const QString& userName, const QString& password)
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

void WizXmlRpcServer::abort()
{
    m_network->disconnect();
}

bool WizXmlRpcServer::xmlRpcCall(WizXmlRpcValue* pParam,
                                  const QString& strMethodName,
                                  const QString& arg1 /* = "" */,
                                  const QString& arg2 /* = "" */)
{
    m_strMethodName = strMethodName;
    m_arg1 = arg1;
    m_arg2 = arg2;

    WizXmlRpcRequest data(strMethodName);
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

void WizXmlRpcServer::processError(WizXmlRpcError error, int errorCode, const QString& errorString)
{
    Q_ASSERT(!m_strMethodName.isEmpty());
    Q_EMIT xmlRpcError(m_strMethodName, error, errorCode, errorString);
}

void WizXmlRpcServer::processReturn(WizXmlRpcValue& ret)
{
    Q_ASSERT(!m_strMethodName.isEmpty());
    Q_EMIT xmlRpcReturn(ret, m_strMethodName, m_arg1, m_arg2);
}

void WizXmlRpcServer::on_replyFinished()
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

    WizXMLDocument doc;
    if (!doc.loadXML(strXml)) {
        processError(errorXmlFormat, 0, "Invalid xml");
        reply->deleteLater();
        return;
    }

    WizXmlRpcValue* pRet = NULL;

    if (!WizXmlRpcResultFromXml(doc, &pRet)) {
        processError(errorXmlRpcFormat, 0, "Can not parse xmlrpc");
        reply->deleteLater();
        return;
    }

    Q_ASSERT(pRet);

    if (WizXmlRpcFaultValue* pFault = dynamic_cast<WizXmlRpcFaultValue *>(pRet)) {
        processError(errorXmlRpcFault, pFault->getFaultCode(), pFault->getFaultString());
    }

    processReturn(*pRet);

    reply->deleteLater();
}

void WizXmlRpcServer::on_replyError(QNetworkReply::NetworkError error)
{
    processError(errorNetwork, error, QString());
}

void WizXmlRpcServer::on_replyDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit xmlRpcReadProgress(bytesReceived, bytesTotal);
}

#endif
