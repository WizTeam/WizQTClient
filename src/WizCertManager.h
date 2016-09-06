#ifndef CWIZCERTMANAGER_H
#define CWIZCERTMANAGER_H

#if 0

#include "share/wizapi.h"
#include "WizDef.h"

class WizCertManager : public CWizApiBase
{
    Q_OBJECT

public:
    WizCertManager(WizExplorerApp& app, const QString& strAccountsApiURL = WIZ_API_URL);

    void loadUserCert();
    bool downloadUserCert();

private:
    WizExplorerApp& m_app;
    WizDatabase& m_db;

    WIZUSERCERT m_cert;
    bool m_bCertInited;

protected:

    virtual void onGetUserCert(const WIZUSERCERT& ret);

    virtual void onXmlRpcError(const QString& strMethodName, \
                               WizXmlRpcError err, \
                               int errorCode, \
                               const QString& errorMessage);

Q_SIGNALS:
    void done(bool succeed, const QString& errorMessage);

};

#endif // 0

#endif // CWIZCERTMANAGER_H
