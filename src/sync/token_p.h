#ifndef WIZSERVICE_TOKEN_P_H
#define WIZSERVICE_TOKEN_P_H

#include <QObject>

class QString;
class QMutex;
class QWaitCondition;

namespace WizService {
class Token;
class AsyncApi;

namespace Internal {

class TokenPrivate: QObject
{
    Q_OBJECT

public:
    explicit TokenPrivate(Token* token);
    ~TokenPrivate();

    void requestToken();
    void setUserId(const QString& strUserId);
    void setPasswd(const QString& strPasswd);

private Q_SLOTS:
    void onGetTokenFinished(const QString& strToken, const QString& strMsg);
    void onKeepAliveFinished(bool bOk, const QString& strMsg);

private:
    AsyncApi* m_api;
    QString m_strToken;
    QString m_strUserId;
    QString m_strPasswd;
    QMutex* m_mutex;
    QWaitCondition* m_waiter;

    Token* q;
};

} // namespace Internal
} // namespace WizService



#endif // WIZSERVICE_TOKEN_P_H
