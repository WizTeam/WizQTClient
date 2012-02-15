#include "wizcreateaccount.h"

CWizCreateAccount::CWizCreateAccount(const CString& strAccountsApiURL)
    : CWizApiBase(strAccountsApiURL, m_events)
{

}

void CWizCreateAccount::createAccount(const CString& strUserId, const CString& strPassword, const CString& strInviteCode)
{
    Q_UNUSED(strInviteCode);
    //
    callCreateAccount(strUserId, strPassword);
}

void CWizCreateAccount::onXmlRpcError(const CString& strMethodName, WizXmlRpcError err, int errorCode, const CString& errorMessage)
{
    Q_UNUSED(strMethodName);
    Q_UNUSED(err);
    Q_UNUSED(errorCode);
    //
    emit done(false, errorMessage);
}

void CWizCreateAccount::onCreateAccount()
{
    emit done(true, "");
}
