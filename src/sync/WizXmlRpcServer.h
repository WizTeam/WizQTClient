#ifndef WIZXMLRPCSERVER_H
#define WIZXMLRPCSERVER_H

#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "share/WizXmlRpc.h"
#include "utils/WizLogger.h"
#include "share/WizMisc.h"



class WizXmlRpcServer : public QObject
{
    Q_OBJECT

public:
    WizXmlRpcServer(const QString& url, QObject* parent);
protected:
    QNetworkAccessManager* m_network;
    QString m_strUrl;
    //
    int m_nLastErrorCode;
    QString m_strLastErrorMessage;

public:
    QString getURL() const;
    //
    int getLastErrorCode() const;
    QString getLastErrorMessage() const;
    //
    void setLastError(int code, QString message) { m_nLastErrorCode = code; m_strLastErrorMessage = message; }
    //
    virtual void onXmlRpcError() {}
protected:
    BOOL getReturnValueInStringMap(const QString& strMethodName, std::map<QString, QString>& mapRet, const QString& strName, QString& strValue);
    //
    bool xmlRpcCall(const QString& strMethodName, WizXmlRpcResult& result, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 = NULL, WizXmlRpcValue* pParam3 = NULL, WizXmlRpcValue* pParam4 = NULL);
    //
public:
    BOOL call(const QString& strMethodName, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 = NULL, WizXmlRpcValue* pParam3 = NULL, WizXmlRpcValue* pParam4 = NULL);
    BOOL call(const QString& strMethodName, WizXmlRpcResult& ret, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 = NULL, WizXmlRpcValue* pParam3 = NULL, WizXmlRpcValue* pParam4 = NULL);
    BOOL call(const QString& strMethodName, std::map<QString, QString>& mapRet, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 = NULL, WizXmlRpcValue* pParam3 = NULL, WizXmlRpcValue* pParam4 = NULL);
    BOOL call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 = NULL, WizXmlRpcValue* pParam3 = NULL, WizXmlRpcValue* pParam4 = NULL);
    BOOL call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, const QString& strRetName2, QString& strRetValue2, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 = NULL, WizXmlRpcValue* pParam3 = NULL, WizXmlRpcValue* pParam4 = NULL);
    BOOL call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, const QString& strRetName2, QString& strRetValue2, const QString& strRetName3, QString& strRetValue3, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 = NULL, WizXmlRpcValue* pParam3 = NULL, WizXmlRpcValue* pParam4 = NULL);
    BOOL call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, const QString& strRetName2, QString& strRetValue2, const QString& strRetName3, QString& strRetValue3, const QString& strRetName4, QString& strRetValue4, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 = NULL, WizXmlRpcValue* pParam3 = NULL, WizXmlRpcValue* pParam4 = NULL);
    //
    template <class TData>
    BOOL call(const QString& strMethodName, TData& ret, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 = NULL, WizXmlRpcValue* pParam3 = NULL, WizXmlRpcValue* pParam4 = NULL)
    {
        WizXmlRpcResult result;
        if (!call(strMethodName, result, pParam1, pParam2, pParam3, pParam4))
        {
            TOLOG1("Failed to call xml-rpc method: %1", strMethodName);
            return FALSE;
        }
        //
        WizXmlRpcValue* pValue = result.getResultValue<WizXmlRpcValue>();
        if (!pValue)
        {
            TOLOG1("Failed to call xml-rpc method, can not get result value: %1", strMethodName);
            return FALSE;
        }
        //
        return pValue->toData<TData>(ret);
    }
    template <class TData>
    BOOL call(const QString& strMethodName, std::deque<TData>& arrayRet, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 = NULL, WizXmlRpcValue* pParam3 = NULL, WizXmlRpcValue* pParam4 = NULL)
    {
        WizXmlRpcResult result;
        if (!call(strMethodName, result, pParam1, pParam2, pParam3, pParam4))
        {
            TOLOG1("Failed to call xml-rpc method: %1", strMethodName);
            return FALSE;
        }
        //
        WizXmlRpcArrayValue* pArray = result.getResultValue<WizXmlRpcArrayValue>();
        if (!pArray)
        {
            TOLOG1("Failed to call xml-rpc method, can not get result value: %1", strMethodName);
            return FALSE;
        }
        //
        return pArray->toArray<TData>(arrayRet);
    }
};

#endif // WIZXMLRPCSERVER_H
