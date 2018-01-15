#ifndef WIZSERVICE_TOKEN_H
#define WIZSERVICE_TOKEN_H

#include <QObject>
#include <QDateTime>

#include "share/WizObject.h"

class WizTokenPrivate;

class WizToken : public QObject
{
    Q_OBJECT

public:
    static WizToken* instance();

    WizToken(const QString& strUserId = 0, const QString& strPasswd = 0);
    ~WizToken();

    static WIZUSERINFO userInfo();
    static QString token(); // sync
    static void requestToken(); // async
    static void clearToken();
    static void clearLastError();
    static void setUserId(const QString& strUserId);
    static void setPasswd(const QString& strPasswd);

    static QString lastErrorMessage();
    static int lastErrorCode();
    static bool lastIsNetworkError();

Q_SIGNALS:
    void tokenAcquired(QString strToken);

    friend class WizTokenPrivate;
};


#endif // WIZSERVICE_TOKEN_H
