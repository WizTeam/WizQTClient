#include "wizSync.h"

#include "wizDatabase.h"

CWizSync::CWizSync(CWizDatabaseManager& dbMgr, const QString& strKbUrl /* = WIZ_API_URL */)
    : m_dbMgr(dbMgr)
    , m_bStarted(false)
    , m_bAborted(false)
{
    qRegisterMetaType<CWizGroupDataArray>("CWizGroupDataArray");

    m_kbSync = new CWizKbSync(dbMgr.db(), strKbUrl);
    connect(m_kbSync, SIGNAL(clientLoginDone()), SLOT(on_clientLoginDone()));
    connect(m_kbSync, SIGNAL(getGroupListDone(const CWizGroupDataArray&)),
            SLOT(on_getGroupListDone(const CWizGroupDataArray&)));
    connect(m_kbSync, SIGNAL(kbSyncDone(bool)), SLOT(on_kbSyncDone(bool)));

    connect(m_kbSync, SIGNAL(processLog(const QString&)),
            SIGNAL(processLog(const QString&)));
    connect(m_kbSync, SIGNAL(processDebugLog(const QString&)),
            SIGNAL(processDebugLog(const QString&)));
    connect(m_kbSync, SIGNAL(processErrorLog(const QString&)),
            SIGNAL(processErrorLog(const QString&)));
}

CWizSync::~CWizSync()
{
    delete m_kbSync;
}

void CWizSync::startSync()
{
    if (m_bStarted)
        return;

    m_bStarted = true;
    m_bAborted = false;

    Q_EMIT processLog(tr("Begin syning"));
    Q_EMIT syncStarted();

    QString strUserId = m_kbSync->database()->getUserId();
    QString strPasswd = m_kbSync->database()->getPassword();
    m_kbSync->callClientLogin(strUserId, strPasswd);
}

void CWizSync::abort()
{
    m_bAborted = true;
    m_kbSync->abort();
}

void CWizSync::on_clientLoginDone()
{
    m_kbSync->database()->setDatabaseInfo(WizGlobal()->userInfo().strKbGUID,
                                          WizGlobal()->userInfo().strDatabaseServer,
                                          "Private", 0);

    Q_EMIT syncLogined();
    m_kbSync->callGetGroupList();
}

void CWizSync::on_getGroupListDone(const CWizGroupDataArray& arrayGroup)
{
    m_arrayGroup.assign(arrayGroup.begin(), arrayGroup.end());

    // reset database info
    CWizGroupDataArray::const_iterator it;
    for (it = m_arrayGroup.begin(); it != m_arrayGroup.end(); it++) {
        const WIZGROUPDATA& group = *it;
        if (!m_dbMgr.isOpened(group.strGroupGUID)) {
            if (!m_dbMgr.open(group.strGroupGUID)) {
                TOLOG("FATAL: can't open group db for sync!");
                continue;
            }
        }

        m_dbMgr.db(group.strGroupGUID).setDatabaseInfo(group.strGroupGUID, group.strDatabaseServer,
                                                       group.strGroupName, group.nUserGroup);
    }

    // close group database not in the list
    QStringList groups;
    m_dbMgr.Guids(groups);
    for (int i = 0; i < groups.size(); i++) {
        bool found = false;
        CWizGroupDataArray::const_iterator it;
        for (it = m_arrayGroup.begin(); it != m_arrayGroup.end(); it++) {
            const WIZGROUPDATA& group = *it;
            if (group.strGroupGUID == groups.at(i)) {
                found = true;
                break;
            }
        }

        if (!found) {
            m_dbMgr.close(groups.at(i));
        }
    }

    // sync user private notes
    Q_EMIT processLog(tr("Begin syncing user private data"));
    m_kbSync->startSync(WizGlobal()->userInfo().strKbGUID);
}

void CWizSync::on_kbSyncDone(bool bError)
{
    if (m_bAborted) {
        m_bStarted = false;
        Q_EMIT syncDone(true);
        return;
    }

    if (bError) {
        Q_EMIT processLog(tr("Error occured while syncing, Try to syncing next time"));
    }

    if (!m_arrayGroup.size()) {
        m_bStarted = false;
        Q_EMIT processLog(tr("Syncing finished"));
        Q_EMIT syncDone(false);
        return;
    }

    WIZGROUPDATA group(m_arrayGroup[0]);
    m_arrayGroup.erase(m_arrayGroup.begin());

    // open group database for syncing
    if (!m_dbMgr.isOpened(group.strGroupGUID)) {
        m_bStarted = false;
        TOLOG(tr("FATAL: Can't open group database!"));
        Q_EMIT syncDone(false);
        return;
    }

    m_kbSync->setDatabase(m_dbMgr.db(group.strGroupGUID));

    // reset db info and start sync group data
    Q_EMIT processLog(tr("Begin syncing group data: %1").arg(group.strGroupName));
    m_kbSync->startSync(group.strGroupGUID);
}
