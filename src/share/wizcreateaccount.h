#ifndef WIZCREATEACCOUNT_H
#define WIZCREATEACCOUNT_H

#include "wizapi.h"

class CWizCreateAccount : public CWizApiBase
{
    Q_OBJECT
public:
    CWizCreateAccount(const CString& strAccountsApiURL);

    void createAccount(const CString& strUserId, const CString& strPassword, const CString& strInviteCode);

protected:
    virtual void onXmlRpcError(const QString& strMethodName, WizXmlRpcError err, int errorCode, const QString& errorMessage);
    virtual void onCreateAccount();

Q_SIGNALS:
    void done(bool succeeded, const CString& strErrorMessage);
};

#endif // WIZCREATEACCOUNT_H
