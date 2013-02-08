#ifndef WIZXMLRPCSERVER_H
#define WIZXMLRPCSERVER_H

#include <QtNetwork>
#include <QString>

#include "wizxmlrpc.h"


class CWizXmlRpcServer : public QObject
{
    Q_OBJECT

public:
    explicit CWizXmlRpcServer(const QString& strUrl);

    bool xmlRpcCall(const QString& strMethodName, CWizXmlRpcValue* pParam);

    void setKbUrl(const QString& strUrl) { m_strUrl = strUrl; }
    void setProxy(const QString& host, int port, const QString& userName, const QString& password);
    void abort();

    QByteArray requestData() { return m_requestData; }
    QByteArray replyData() { return m_replyData; }

protected:
    QPointer<QNetworkAccessManager> m_network;
    QString m_strUrl;
    QString m_strMethodName;

    virtual void processError(WizXmlRpcError error, int errorCode, const QString& errorString);
    virtual void processReturn(CWizXmlRpcValue& ret);

private:
    QByteArray m_requestData;
    QByteArray m_replyData;

public Q_SLOTS:
    void on_replyFinished();
    void on_replyError(QNetworkReply::NetworkError);
    void on_replyDownloadProgress(qint64, qint64);

Q_SIGNALS:
    void xmlRpcError(const QString& strMethodName, WizXmlRpcError error, int errorCode, const QString& errorString);
    void xmlRpcReturn(const QString& strMethodName, CWizXmlRpcValue& ret);
    void xmlRpcReadProgress(qint64 bytesReceived, qint64 bytesTotal);
};

#endif // WIZXMLRPCSERVER_H
