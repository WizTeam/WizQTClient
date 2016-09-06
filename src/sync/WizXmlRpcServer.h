#ifndef WIZXMLRPCSERVER_H
#define WIZXMLRPCSERVER_H

#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "share/WizXmlRpc.h"
#include "utils/WizLogger.h"
#include "share/WizMisc.h"



class WizXmlRpcServerBase : public QObject
{
    Q_OBJECT

public:
    WizXmlRpcServerBase(const QString& url, QObject* parent);
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
    virtual void onXmlRpcError() {}
protected:
    BOOL getReturnValueInStringMap(const QString& strMethodName, std::map<QString, QString>& mapRet, const QString& strName, QString& strValue);
    //
    bool xmlRpcCall(const QString& strMethodName, WizXmlRpcResult& result, WizXmlRpcValue* pParam1, WizXmlRpcValue* pParam2 = NULL, WizXmlRpcValue* pParam3 = NULL, WizXmlRpcValue* pParam4 = NULL);
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
            TOLOG1(_T("Failed to call xml-rpc method: %1"), strMethodName);
            return FALSE;
        }
        //
        WizXmlRpcValue* pValue = result.getResultValue<WizXmlRpcValue>();
        if (!pValue)
        {
            TOLOG1(_T("Failed to call xml-rpc method, can not get result value: %1"), strMethodName);
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
            TOLOG1(_T("Failed to call xml-rpc method: %1"), strMethodName);
            return FALSE;
        }
        //
        WizXmlRpcArrayValue* pArray = result.getResultValue<WizXmlRpcArrayValue>();
        if (!pArray)
        {
            TOLOG1(_T("Failed to call xml-rpc method, can not get result value: %1"), strMethodName);
            return FALSE;
        }
        //
        return pArray->toArray<TData>(arrayRet);
    }
};


#if 0

class WizXmlRpcServer : public QObject
{
    Q_OBJECT

public:
    explicit WizXmlRpcServer(const QString& strUrl, QObject* parent = 0);

    bool xmlRpcCall(WizXmlRpcValue* pParam, const QString& strMethodName,
                    const QString& arg1 = "", const QString& arg2 = "");

    void setKbUrl(const QString& strUrl) { m_strUrl = strUrl; }
    void setProxy(const QString& host, int port, const QString& userName, const QString& password);
    void abort();

protected:
    QNetworkAccessManager* m_network;

    QString m_strUrl;
    QString m_strMethodName;
    QString m_arg1;
    QString m_arg2;

    virtual void processError(WizXmlRpcError error, int errorCode, const QString& errorString);
    virtual void processReturn(WizXmlRpcValue& ret);

public Q_SLOTS:
    void on_replyFinished();
    void on_replyError(QNetworkReply::NetworkError);
    void on_replyDownloadProgress(qint64, qint64);

Q_SIGNALS:
    void xmlRpcError(const QString& strMethodName, WizXmlRpcError error,
                     int errorCode, const QString& errorString);
    void xmlRpcReturn(WizXmlRpcValue& ret, const QString& strMethodName,
                      const QString& arg1, const QString& arg2);
    void xmlRpcReadProgress(qint64 bytesReceived, qint64 bytesTotal);
};

#endif

#endif // WIZXMLRPCSERVER_H
