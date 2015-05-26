#ifndef WIZXMLRPCSERVER_H
#define WIZXMLRPCSERVER_H

#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "share/wizxmlrpc.h"
#include "utils/logger.h"
#include "share/wizmisc.h"



class CWizXmlRpcServerBase : public QObject
{
    Q_OBJECT

public:
    CWizXmlRpcServerBase(const QString& url, QObject* parent);
protected:
    QNetworkAccessManager* m_network;
    QString m_strUrl;
    //
    int m_nLastErrorCode;
    QString m_strLastErrorMessage;

public:
    QString GetURL() const;
    //
    int GetLastErrorCode() const;
    QString GetLastErrorMessage() const;
    //
    virtual void OnXmlRpcError() {}
protected:
    BOOL GetReturnValueInStringMap(const QString& strMethodName, std::map<QString, QString>& mapRet, const QString& strName, QString& strValue);
    //
    bool xmlRpcCall(const QString& strMethodName, CWizXmlRpcResult& result, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2 = NULL, CWizXmlRpcValue* pParam3 = NULL, CWizXmlRpcValue* pParam4 = NULL);
    BOOL Call(const QString& strMethodName, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2 = NULL, CWizXmlRpcValue* pParam3 = NULL, CWizXmlRpcValue* pParam4 = NULL);
    BOOL Call(const QString& strMethodName, CWizXmlRpcResult& ret, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2 = NULL, CWizXmlRpcValue* pParam3 = NULL, CWizXmlRpcValue* pParam4 = NULL);
    BOOL Call(const QString& strMethodName, std::map<QString, QString>& mapRet, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2 = NULL, CWizXmlRpcValue* pParam3 = NULL, CWizXmlRpcValue* pParam4 = NULL);
    BOOL Call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2 = NULL, CWizXmlRpcValue* pParam3 = NULL, CWizXmlRpcValue* pParam4 = NULL);
    BOOL Call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, const QString& strRetName2, QString& strRetValue2, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2 = NULL, CWizXmlRpcValue* pParam3 = NULL, CWizXmlRpcValue* pParam4 = NULL);
    BOOL Call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, const QString& strRetName2, QString& strRetValue2, const QString& strRetName3, QString& strRetValue3, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2 = NULL, CWizXmlRpcValue* pParam3 = NULL, CWizXmlRpcValue* pParam4 = NULL);
    BOOL Call(const QString& strMethodName, const QString& strRetName1, QString& strRetValue1, const QString& strRetName2, QString& strRetValue2, const QString& strRetName3, QString& strRetValue3, const QString& strRetName4, QString& strRetValue4, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2 = NULL, CWizXmlRpcValue* pParam3 = NULL, CWizXmlRpcValue* pParam4 = NULL);
    //
    template <class TData>
    BOOL Call(const QString& strMethodName, TData& ret, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2 = NULL, CWizXmlRpcValue* pParam3 = NULL, CWizXmlRpcValue* pParam4 = NULL)
    {
        CWizXmlRpcResult result;
        if (!Call(strMethodName, result, pParam1, pParam2, pParam3, pParam4))
        {
            TOLOG1(_T("Failed to call xml-rpc method: %1"), strMethodName);
            return FALSE;
        }
        //
        CWizXmlRpcValue* pValue = result.GetResultValue<CWizXmlRpcValue>();
        if (!pValue)
        {
            TOLOG1(_T("Failed to call xml-rpc method, can not get result value: %1"), strMethodName);
            return FALSE;
        }
        //
        return pValue->ToData<TData>(ret);
    }
    template <class TData>
    BOOL Call(const QString& strMethodName, std::deque<TData>& arrayRet, CWizXmlRpcValue* pParam1, CWizXmlRpcValue* pParam2 = NULL, CWizXmlRpcValue* pParam3 = NULL, CWizXmlRpcValue* pParam4 = NULL)
    {
        CWizXmlRpcResult result;
        if (!Call(strMethodName, result, pParam1, pParam2, pParam3, pParam4))
        {
            TOLOG1(_T("Failed to call xml-rpc method: %1"), strMethodName);
            return FALSE;
        }
        //
        CWizXmlRpcArrayValue* pArray = result.GetResultValue<CWizXmlRpcArrayValue>();
        if (!pArray)
        {
            TOLOG1(_T("Failed to call xml-rpc method, can not get result value: %1"), strMethodName);
            return FALSE;
        }
        //
        return pArray->ToArray<TData>(arrayRet);
    }
};


#if 0

class CWizXmlRpcServer : public QObject
{
    Q_OBJECT

public:
    explicit CWizXmlRpcServer(const QString& strUrl, QObject* parent = 0);

    bool xmlRpcCall(CWizXmlRpcValue* pParam, const QString& strMethodName,
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
    virtual void processReturn(CWizXmlRpcValue& ret);

public Q_SLOTS:
    void on_replyFinished();
    void on_replyError(QNetworkReply::NetworkError);
    void on_replyDownloadProgress(qint64, qint64);

Q_SIGNALS:
    void xmlRpcError(const QString& strMethodName, WizXmlRpcError error,
                     int errorCode, const QString& errorString);
    void xmlRpcReturn(CWizXmlRpcValue& ret, const QString& strMethodName,
                      const QString& arg1, const QString& arg2);
    void xmlRpcReadProgress(qint64 bytesReceived, qint64 bytesTotal);
};

#endif

#endif // WIZXMLRPCSERVER_H
