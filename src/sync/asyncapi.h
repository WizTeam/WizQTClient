#ifndef WIZSERVICE_ASYNCAPI_H
#define WIZSERVICE_ASYNCAPI_H

#include <QObject>

class QString;

namespace WizService {

class AsyncApi : public QObject
{
    Q_OBJECT

public:
    explicit AsyncApi(QObject *parent = 0);
    ~AsyncApi();

    void getToken(const QString& strUserId, const QString& strPasswd);
    void keepAlive(const QString& strToken);

private:
    QString m_strToken;
    int m_nErrorCode;
    QString m_strErrorMessage;

    bool getToken_impl(const QString& strUserId, const QString& strPasswd);
    bool keepAlive_impl(const QString& strToken);

private Q_SLOTS:
    void on_getToken_finished();
    void on_keepAlive_finished();

Q_SIGNALS:
    void getTokenFinished(const QString& strToken, const QString& strMsg);
    void keepAliveFinished(bool bOk, const QString& strMsg);
};

} // namespace WizService

#endif // WIZSERVICE_ASYNCAPI_H
