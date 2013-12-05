#include "wizcertmanager.h"

#if 0

#include "share/wizDatabaseManager.h"

CWizCertManager::CWizCertManager(CWizExplorerApp& app, const QString& strAccountsApiURL /* = WIZ_API_URL*/)
    : CWizApiBase(strAccountsApiURL)
    , m_app(app)
    , m_db(app.databaseManager().db())
    , m_bCertInited(false)
{
}

void CWizCertManager::onXmlRpcError(const QString& strMethodName, \
                                    WizXmlRpcError err, \
                                    int errorCode, \
                                    const QString& errorMessage)
{
    Q_UNUSED(strMethodName);
    Q_UNUSED(err);
    Q_UNUSED(errorCode);
    Q_UNUSED(errorMessage);

    Q_EMIT done(false, errorMessage);
}

void CWizCertManager::onGetUserCert(const WIZUSERCERT& data)
{
    // save to database
    if(!m_db.SetUserCert(data.strN, \
                         data.stre, \
                         data.strd, \
                         data.strHint)) {

        TOLOG("update user cert failed");
        return;
    }

    m_bCertInited = true;
    Q_EMIT done(true, QString());
}

void CWizCertManager::loadUserCert()
{
    if (m_bCertInited) {
        Q_EMIT done(true, QString());
        return;
    }

    bool ret = m_db.GetUserCert(m_cert.strN, m_cert.stre, m_cert.strd, m_cert.strHint);

    if (!ret) {
        downloadUserCert();
    } else {
        m_bCertInited = true;
        Q_EMIT done(true, QString());
    }
}

bool CWizCertManager::downloadUserCert()
{
    return callGetUserCert(m_db.getUserId(), m_db.GetPassword());
}

#endif // 0
