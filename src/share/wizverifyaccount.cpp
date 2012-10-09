#include "wizverifyaccount.h"

CWizVerifyAccount::CWizVerifyAccount(const CString& strAccountsApiURL)
    : CWizApiBase(strAccountsApiURL, m_events)
{

}

void CWizVerifyAccount::verifyAccount(const CString& strUserId, const CString& strPassword)
{
    callClientLogin(strUserId, strPassword);
}

void CWizVerifyAccount::onXmlRpcError(const CString& strMethodName, WizXmlRpcError err, int errorCode, const CString& errorMessage)
{
    Q_UNUSED(strMethodName);
    Q_UNUSED(err);
    Q_UNUSED(errorCode);

    emit done(false, errorMessage);
}

void CWizVerifyAccount::onClientLogin()
{
    emit done(true, "");
}

void CWizVerifyAccount::addErrorLog(const CString& str)
{
    m_strErrorMessage += str;
    m_strErrorMessage += "\n";
}
