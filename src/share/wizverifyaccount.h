#ifndef WIZVERIFYACCOUNT_H
#define WIZVERIFYACCOUNT_H

#include "wizapi.h"

class CWizVerifyAccount : public CWizApiBase
{
    Q_OBJECT

public:
    void verifyAccount(const QString& strUserId, const QString& strPassword);

protected:
    virtual void onClientLogin(const WIZUSERINFO& userInfo);
    virtual void onXmlRpcError(const QString& strMethodName,
                               WizXmlRpcError err,
                               int errorCode,
                               const QString& errorMessage);

Q_SIGNALS:
    void done(bool succeeded, int errorCode, const QString& errorMessage);
};


#endif // WIZVERIFYACCOUNT_H
