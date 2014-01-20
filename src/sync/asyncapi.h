#ifndef WIZSERVICE_ASYNCAPI_H
#define WIZSERVICE_ASYNCAPI_H

#include <QObject>

class QString;
struct WIZUSERINFO;

namespace WizService {

class AsyncApi : public QObject
{
    Q_OBJECT

public:
    explicit AsyncApi(QObject *parent = 0);
    ~AsyncApi();

    void login(const QString& strUserId, const QString& strPasswd);
    void getToken(const QString& strUserId, const QString& strPasswd);
    void keepAlive(const QString& strToken, const QString& strKbGUID);
    void registerAccount(const QString& strUserId, const QString& strPasswd, const QString& strInviteCode);
    void getCommentsCount(const QString& strUrl);
    void setMessageStatus(const QString& ids, bool bRead);

    int lastErrorCode() { return m_nErrorCode; }
    QString lastErrorMessage() { return m_strErrorMessage; }

private:
    int m_nErrorCode;
    QString m_strErrorMessage;

    bool login_impl(const QString& strUserId, const QString& strPasswd);
    bool getToken_impl(const QString& strUserId, const QString& strPasswd);
    bool keepAlive_impl(const QString& strToken, const QString &strKbGUID);
    bool registerAccount_impl(const QString& strUserId, const QString& strPasswd, const QString& strInviteCode);
    void getCommentsCount_impl(const QString& strUrl);
    void setMessageStatus_impl(const QString& ids, bool bRead);

Q_SIGNALS:
    void loginFinished(const WIZUSERINFO& info);
    void getTokenFinished(const QString& strToken);
    void keepAliveFinished(bool bOk);
    void registerAccountFinished(bool bOk);
    void getCommentsCountFinished(int i);
};

} // namespace WizService

#endif // WIZSERVICE_ASYNCAPI_H
