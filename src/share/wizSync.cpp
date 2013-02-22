#include "wizSync.h"

#include "wizDatabase.h"

CWizSync::CWizSync(CWizDatabaseManager& dbMgr, const QString& strKbUrl /* = WIZ_API_URL */)
    : m_dbMgr(dbMgr)
    , m_bStarted(false)
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
    TOLOG(tr("Begin syning"));
    Q_EMIT syncStarted();

    m_bStarted = true;

    QString strUserId = m_kbSync->database()->getUserId();
    QString strPasswd = m_kbSync->database()->getPassword();
    m_kbSync->callClientLogin(strUserId, strPasswd);
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
    TOLOG(tr("Begin syncing user private data"));
    m_kbSync->startSync(WizGlobal()->userInfo().strKbGUID);
}

void CWizSync::on_kbSyncDone(bool bError)
{
    if (bError) {
        m_bStarted = false;
        TOLOG(tr("Error occured while syncing, Try to syncing next time"));
        Q_EMIT syncDone(true);
        return;
    }

    if (!m_arrayGroup.size()) {
        m_bStarted = false;
        TOLOG(tr("Syncing finished"));
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
    TOLOG1(tr("Begin syncing group data: %1"), group.strGroupName);
    m_kbSync->startSync(group.strGroupGUID);
}
