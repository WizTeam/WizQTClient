#ifndef WIZCREATEACCOUNT_H
#define WIZCREATEACCOUNT_H

#include "wizapi.h"

class WizCreateAccount : public CWizApiBase
{
    Q_OBJECT
public:
    WizCreateAccount(const CString& strAccountsApiURL);

    void createAccount(const CString& strUserId, const CString& strPassword, const CString& strInviteCode);

protected:
    virtual void onCreateAccount();

Q_SIGNALS:
    void done(bool succeeded, const CString& strErrorMessage);
};

#endif // WIZCREATEACCOUNT_H
