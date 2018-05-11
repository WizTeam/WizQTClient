#ifndef WIZVERIFYACCOUNT_H
#define WIZVERIFYACCOUNT_H

#include "wizapi.h"

class WizVerifyAccount : public CWizApiBase
{
    Q_OBJECT

public:
    void verifyAccount(const QString& strUserId, const QString& strPassword);

protected:
    virtual void onClientLogin(const WIZUSERINFO& userInfo);

Q_SIGNALS:
    void done(bool succeeded, int errorCode, const QString& errorMessage);
};


#endif // WIZVERIFYACCOUNT_H
