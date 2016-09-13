#include "WizCreateAccount.h"

WizCreateAccount::WizCreateAccount(const CString& strAccountsApiURL)
    : CWizApiBase(strAccountsApiURL)
{
}

void WizCreateAccount::createAccount(const CString& strUserId, \
                                      const CString& strPassword, \
                                      const CString& strInviteCode)
{
    Q_UNUSED(strInviteCode);

    callCreateAccount(strUserId, strPassword);
}

void WizCreateAccount::onXmlRpcError(const QString& strMethodName, \
                                      WizXmlRpcError err, \
                                      int errorCode, \
                                      const QString& errorMessage)
{
    Q_UNUSED(strMethodName);
    Q_UNUSED(err);
    Q_UNUSED(errorCode);

    emit done(false, errorMessage);
}

void WizCreateAccount::onCreateAccount()
{
    emit done(true, "");
}
