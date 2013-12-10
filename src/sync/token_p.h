#ifndef WIZSERVICE_TOKEN_P_H
#define WIZSERVICE_TOKEN_P_H

#include <QObject>
#include "../share/wizobject.h"

class QString;
class QDateTime;
class QMutex;

struct WIZUSERINFO;

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
    const WIZUSERINFO& info();

private Q_SLOTS:
    void onLoginFinished(const WIZUSERINFO& info);
    void onGetTokenFinished(const QString& strToken, const QString& strMsg);
    void onKeepAliveFinished(bool bOk, const QString& strMsg);

private:
    AsyncApi* m_api;
    WIZUSERINFO m_info;
    QString m_strUserId;
    QString m_strPasswd;
    QMutex* m_mutex;

    Token* q;
};

} // namespace Internal
} // namespace WizService



#endif // WIZSERVICE_TOKEN_P_H
