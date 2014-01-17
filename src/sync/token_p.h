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

    QString token();
    void requestToken();
    void setUserId(const QString& strUserId);
    void setPasswd(const QString& strPasswd);
    const WIZUSERINFO& info();

    int lastErrorCode() const;
    QString lastErrorMessage() const;

private Q_SLOTS:
    void onLoginFinished(const WIZUSERINFO& info);
    void onGetTokenFinished(const QString& strToken);
    void onKeepAliveFinished(bool bOk);

private:
    AsyncApi* m_api;
    WIZUSERINFO m_info;
    QString m_strUserId;
    QString m_strPasswd;
    bool m_bProcess;
    QMutex* m_mutex;

    Token* q;
};

} // namespace Internal
} // namespace WizService



#endif // WIZSERVICE_TOKEN_P_H
