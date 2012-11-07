#ifndef WIZVERIFYACCOUNT_H
#define WIZVERIFYACCOUNT_H

#include "wizapi.h"

class CWizVerifyAccount : public CWizApiBase
{
    Q_OBJECT

public:
    CWizVerifyAccount(const CString& strAccountsApiURL);

    void verifyAccount(const CString& strUserId, const CString& strPassword);

private:
    CString m_strErrorMessage;

public:
    virtual void onXmlRpcError(const QString& strMethodName, \
                               WizXmlRpcError err, \
                               int errorCode, \
                               const QString& errorMessage);

    virtual void onClientLogin();

    virtual void addErrorLog(const CString& str);

Q_SIGNALS:
    void done(bool succeeded, const CString& errorMessage);
};


#endif // WIZVERIFYACCOUNT_H
