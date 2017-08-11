#include "WizXmlRpcServer.h"
#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QTimer>

#include "share/WizEventLoop.h"
#include "WizDef.h"


WizXmlRpcServer::WizXmlRpcServer(const QString& strUrl, QObject* parent)
    : QObject(parent)
    , m_strUrl(strUrl)
    , m_nLastErrorCode(0)
{
    m_network = new QNetworkAccessManager(this);
}
QString WizXmlRpcServer::getURL() const
{
    return m_strUrl;
}

int WizXmlRpcServer::getLastErrorCode() const
{
    return m_nLastErrorCode;
}
QString WizXmlRpcServer::getLastErrorMessage() const
{
    return m_strLastErrorMessage;
}

bool WizXmlRpcServer::xmlRpcCall(const QString& strMethodName, WizXmlRpcResult& result, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 /*= NULL*/, WizXmlRpcValue* pParam3 /*= NULL*/, WizXmlRpcValue* pParam4 /*= NULL*/)
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
    //
    QUrl url(m_strUrl + "?methodName=" + strMethodName + "&version=" + WIZ_CLIENT_VERSION);

    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/xml"));
    request.setHeader(QNetworkRequest::UserAgentHeader, QVariant(QString("WizQT_") + WIZ_CLIENT_VERSION));

    QNetworkReply* reply = m_network->post(request, data.toData());
    WizXmlRpcEventLoop loop(reply);
//        qDebug() << "[Sync]Start a xml rpc event loop";
    loop.exec();
//        qDebug() << "[Sync]Xml rpc event loop finished";
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
    //
    return true;
}

BOOL WizXmlRpcServer::call(const QString& strMethodName, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 /*= NULL*/, WizXmlRpcValue* pParam3 /*= NULL*/, WizXmlRpcValue* pParam4 /*= NULL*/)
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


BOOL WizXmlRpcServer::call(const QString& strMethodName, WizXmlRpcResult& ret, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 /*= NULL*/, WizXmlRpcValue* pParam3 /*= NULL*/, WizXmlRpcValue* pParam4 /*= NULL*/)
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

BOOL WizXmlRpcServer::call(const QString& strMethodName, std::map<QString, QString>& mapRet, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 /*= NULL*/, WizXmlRpcValue* pParam3 /*= NULL*/, WizXmlRpcValue* pParam4 /*= NULL*/)
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

BOOL WizXmlRpcServer::getReturnValueInStringMap(const QString& strMethodName, std::map<QString, QString>& mapRet, const QString& strName, QString& strValue)
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


BOOL WizXmlRpcServer::call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 /*= NULL*/, WizXmlRpcValue* pParam3 /*= NULL*/, WizXmlRpcValue* pParam4 /*= NULL*/)
{
    std::map<QString, QString> mapRet;
    if (!call(strMethodName, mapRet, pParam1, pParam2, pParam3, pParam4))
        return FALSE;
    //
    return getReturnValueInStringMap(strMethodName, mapRet, strRetName1, strRetValue1);
}
BOOL WizXmlRpcServer::call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, const QString& strRetName2, QString& strRetValue2, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 /*= NULL*/, WizXmlRpcValue* pParam3 /*= NULL*/, WizXmlRpcValue* pParam4 /*= NULL*/)
{
    std::map<QString, QString> mapRet;
    if (!call(strMethodName, mapRet, pParam1, pParam2, pParam3, pParam4))
        return FALSE;
    //
    return getReturnValueInStringMap(strMethodName, mapRet, strRetName1, strRetValue1)
        && getReturnValueInStringMap(strMethodName, mapRet, strRetName2, strRetValue2);
}
BOOL WizXmlRpcServer::call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, const QString& strRetName2, QString& strRetValue2, const QString& strRetName3, QString& strRetValue3, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 /*= NULL*/, WizXmlRpcValue* pParam3 /*= NULL*/, WizXmlRpcValue* pParam4 /*= NULL*/)
{
    std::map<QString, QString> mapRet;
    if (!call(strMethodName, mapRet, pParam1, pParam2, pParam3, pParam4))
        return FALSE;
    //
    return getReturnValueInStringMap(strMethodName, mapRet, strRetName1, strRetValue1)
        && getReturnValueInStringMap(strMethodName, mapRet, strRetName2, strRetValue2)
        && getReturnValueInStringMap(strMethodName, mapRet, strRetName3, strRetValue3);
}
BOOL WizXmlRpcServer::call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, const QString& strRetName2, QString& strRetValue2, const QString& strRetName3, QString& strRetValue3, const QString& strRetName4, QString& strRetValue4, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 /*= NULL*/, WizXmlRpcValue* pParam3 /*= NULL*/, WizXmlRpcValue* pParam4 /*= NULL*/)
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


