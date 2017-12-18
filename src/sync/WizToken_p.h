#ifndef WIZSERVICE_TOKEN_P_H
#define WIZSERVICE_TOKEN_P_H

#include <QObject>
#include "../share/WizObject.h"

class QString;
class QDateTime;
class QMutex;

struct WIZUSERINFO;

class WizToken;
class WizAsyncApi;


class WizTokenPrivate: QObject
{
    Q_OBJECT

public:
    explicit WizTokenPrivate(WizToken* token);
    ~WizTokenPrivate();

    QString token();
    void requestToken();
    void clearToken();
    void clearLastError();
    void setUserId(const QString& strUserId);
    void setPasswd(const QString& strPasswd);
    WIZUSERINFO userInfo();

    int lastErrorCode() const;
    QString lastErrorMessage() const;
    bool lastIsNetworkError() const;

private:
    WIZUSERINFO m_info;
    QString m_strUserId;
    QString m_strPasswd;
    bool m_bProcessing;
    QMutex* m_mutex;
    int m_lastErrorCode;
    QString m_lastErrorMessage;
    bool m_bLastIsNetworkError;

    WizToken* q;
};



#endif // WIZSERVICE_TOKEN_P_H
