#ifndef WIZSERVICE_ASYNCAPI_H
#define WIZSERVICE_ASYNCAPI_H

#include <QObject>

class QString;
struct WIZUSERINFO;
class QNetworkAccessManager;


class WizAsyncApi : public QObject
{
    Q_OBJECT

public:
    explicit WizAsyncApi(QObject *parent = 0);
    ~WizAsyncApi();

    void registerAccount(const QString& strUserId, const QString& strPasswd, const QString& strInviteCode,
                         const QString& strCaptchaID = "", const QString& strCaptcha = "");
    void setMessageReadStatus(const QString& ids, bool bRead);
    void setMessageDeleteStatus(const QString& ids, bool bDelete);

    int lastErrorCode() { return m_nErrorCode; }
    QString lastErrorMessage() { return m_strErrorMessage; }

private:
    int m_nErrorCode;
    QString m_strErrorMessage;
    QNetworkAccessManager* m_networkManager;

    bool registerAccount_impl(const QString& strUserId, const QString& strPasswd, const QString& strInviteCode, const QString& strCaptchaID, const QString& strCaptcha);
    void setMessageReadStatus_impl(const QString& ids, bool bRead);
    void setMessageDeleteStatus_impl(const QString& ids, bool bDelete);

Q_SIGNALS:
    void registerAccountFinished(bool bOk);    
    void uploadMessageReadStatusFinished(const QString& ids);
    void uploadMessageDeleteStatusFinished(const QString& ids);
};

#endif // WIZSERVICE_ASYNCAPI_H
