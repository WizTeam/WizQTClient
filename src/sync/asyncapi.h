#ifndef WIZSERVICE_ASYNCAPI_H
#define WIZSERVICE_ASYNCAPI_H

#include <QObject>

class QString;
struct WIZUSERINFO;
class QNetworkAccessManager;

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
    void registerAccount(const QString& strUserId, const QString& strPasswd, const QString& strInviteCode,
                         const QString& strCaptchaID = "", const QString& strCaptcha = "");
    void getCommentsCount(const QString& strUrl);
    void setMessageReadStatus(const QString& ids, bool bRead);
    void setMessageDeleteStatus(const QString& ids, bool bDelete);

    int lastErrorCode() { return m_nErrorCode; }
    QString lastErrorMessage() { return m_strErrorMessage; }

private:
    int m_nErrorCode;
    QString m_strErrorMessage;
    QNetworkAccessManager* m_networkManager;

    bool login_impl(const QString& strUserId, const QString& strPasswd);
    bool getToken_impl(const QString& strUserId, const QString& strPasswd);
    bool keepAlive_impl(const QString& strToken, const QString &strKbGUID);
    bool registerAccount_impl(const QString& strUserId, const QString& strPasswd, const QString& strInviteCode, const QString& strCaptchaID, const QString& strCaptcha);
    void setMessageReadStatus_impl(const QString& ids, bool bRead);
    void setMessageDeleteStatus_impl(const QString& ids, bool bDelete);

public slots:
    void on_comments_finished();

Q_SIGNALS:
    void loginFinished(const WIZUSERINFO& info);
    void getTokenFinished(const QString& strToken);
    void keepAliveFinished(bool bOk);
    void registerAccountFinished(bool bOk);
    void getCommentsCountFinished(int i);
    void uploadMessageReadStatusFinished(const QString& ids);
    void uploadMessageDeleteStatusFinished(const QString& ids);
};

} // namespace WizService

#endif // WIZSERVICE_ASYNCAPI_H
