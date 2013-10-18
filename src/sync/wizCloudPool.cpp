#include "wizCloudPool.h"

#include <QApplication>
#include <QThread>

#include "wizkmxmlrpc.h"
#include "../share/wizDatabaseManager.h"


CWizCloudPool* CWizCloudPool::_instance = 0;

CWizCloudPool* CWizCloudPool::instance()
{
    if (0 == _instance) {
        _instance = new CWizCloudPool();
        QThread* td = new QThread();
        _instance->moveToThread(td);
        td->start();
    }

    return _instance;
}

void CWizCloudPool::init(CWizDatabaseManager* dbMgr)
{
    Q_ASSERT(!m_bInited);

    m_dbMgr = dbMgr;
    m_bInited = true;
}

void CWizCloudPool::getToken()
{
    Q_ASSERT(m_bInited);
    Q_ASSERT(QThread::currentThread() != qApp->thread());

    if (!m_aServer) {
        m_aServer = new CWizKMAccountsServer(::WizKMGetAccountsServerURL(true));
    }

    QString strToken;
    if (!m_aServer->GetToken(m_dbMgr->db().GetUserId(), m_dbMgr->db().GetPassword(), strToken)) {
        TOLOG("[CloudPool]Failed to get token");
    }

    Q_EMIT tokenAcquired(strToken);
}
