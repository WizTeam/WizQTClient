#ifndef WIZSERVICE_ASYNCAPI_H
#define WIZSERVICE_ASYNCAPI_H

#include <QObject>
//#include "../share/wizobject.h"

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

private:
    //WIZUSERINFO m_info;
    //QString m_strToken;
    int m_nErrorCode;
    QString m_strErrorMessage;

    bool login_impl(const QString& strUserId, const QString& strPasswd);
    bool getToken_impl(const QString& strUserId, const QString& strPasswd);
    bool keepAlive_impl(const QString& strToken, const QString &strKbGUID);

private Q_SLOTS:
    //void onLoginFinished();
    //void on_getToken_finished();
    //void on_keepAlive_finished();

Q_SIGNALS:
    void loginFinished(const WIZUSERINFO& info);
    void getTokenFinished(const QString& strToken, const QString& strMsg = 0);
    void keepAliveFinished(bool bOk, const QString& strMsg = 0);
};

} // namespace WizService

#endif // WIZSERVICE_ASYNCAPI_H
