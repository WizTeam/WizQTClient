#ifndef CWIZCERTMANAGER_H
#define CWIZCERTMANAGER_H

#include "share/wizapi.h"
#include "wizdef.h"

class CWizCertManager : public CWizApiBase
{
    Q_OBJECT

public:
    CWizCertManager(CWizExplorerApp& app, const QString& strAccountsApiURL = WIZ_API_URL);

    void loadUserCert();

private:
    CWizExplorerApp& m_app;
    CWizDatabase& m_db;

    WIZUSERCERT m_cert;
    bool m_bCertInited;

protected:
    bool downloadUserCert();

    virtual void onGetUserCert(CWizXmlRpcValue& ret);

    virtual void onXmlRpcError(const QString& strMethodName, \
                               WizXmlRpcError err, \
                               int errorCode, \
                               const QString& errorMessage);

Q_SIGNALS:
    void done(bool succeed, const QString& errorMessage);

};

#endif // CWIZCERTMANAGER_H
