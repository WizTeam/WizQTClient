#include "WizVerifyAccount.h"

void WizVerifyAccount::verifyAccount(const QString& strUserId,
                                      const QString& strPassword)
{
    callClientLogin(strUserId, strPassword);
}

void WizVerifyAccount::onClientLogin(const WIZUSERINFO& userInfo)
{
    Q_UNUSED(userInfo);

    emit done(true, 0, "");
}

void WizVerifyAccount::onXmlRpcError(const QString& strMethodName, \
                                      WizXmlRpcError err, \
                                      int errorCode, \
                                      const QString& errorMessage)
{
    Q_UNUSED(strMethodName);
    Q_UNUSED(err);
    Q_UNUSED(errorCode);

    emit done(false, errorCode, errorMessage);
}
