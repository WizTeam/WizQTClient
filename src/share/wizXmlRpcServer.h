#ifndef WIZXMLRPCSERVER_H
#define WIZXMLRPCSERVER_H

#include <QtNetwork>
#include <QString>

#include "wizxmlrpc.h"


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

#endif // WIZXMLRPCSERVER_H
