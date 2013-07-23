#include "wizSync.h"

#include "wizDatabase.h"
#include "wizApiEntry.h"

CWizSync::CWizSync(CWizDatabaseManager& dbMgr, const QString& strKbUrl /* = WIZ_API_URL */)
    : m_dbMgr(dbMgr)
    , m_bStarted(false)
    , m_bAborted(false)
{
    m_kbSync = new CWizKbSync(dbMgr.db(), strKbUrl);

    // signals from CWizApiBase
    connect(m_kbSync, SIGNAL(clientLoginDone()), SLOT(on_clientLoginDone()));
    connect(m_kbSync, SIGNAL(getGroupListDone(const CWizGroupDataArray&)),
            SLOT(on_getGroupListDone(const CWizGroupDataArray&)));

    // signals for syncing user folders
    connect(m_kbSync, SIGNAL(folderGetVersionDone(qint64)),
            SLOT(on_folderGetVersionDone(qint64)));
    connect(m_kbSync, SIGNAL(folderGetListDone(const QStringList&, qint64)),
            SLOT(on_folderGetListDone(const QStringList&, qint64)));
    connect(m_kbSync, SIGNAL(folderPostListDone(qint64)),
            SLOT(on_folderPostListDone(qint64)));

    // signals from CWizKbSync
    connect(m_kbSync, SIGNAL(kbSyncDone(bool)), SLOT(on_kbSyncDone(bool)));

    // obsolete
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

void CWizSync::abort()
{
    m_bAborted = true;
    m_kbSync->abort();
}

void CWizSync::startSync()
{
    if (m_bStarted)
        return;

    m_bStarted = true;
    m_bAborted = false;

    Q_EMIT processLog(tr("Begin syning"));
    Q_EMIT syncStarted();

    // 1. login
    QString strUserId = m_kbSync->database()->getUserId();
    QString strPasswd = m_kbSync->database()->getPassword();
    m_kbSync->callClientLogin(strUserId, strPasswd);
}

void CWizSync::on_clientLoginDone()
{
    qDebug() << "[Syncing]logined...";

    WIZDATABASEINFO dbInfo;
    dbInfo.name = "Private";
    dbInfo.nPermission = 0;
    dbInfo.kbGUID = WizGlobal()->userInfo().strKbGUID;
    dbInfo.serverUrl = WizGlobal()->userInfo().strDatabaseServer;
    m_kbSync->database()->setDatabaseInfo(dbInfo);

    Q_EMIT syncLogined();

    // 2. sync group info
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

        WIZDATABASEINFO dbInfo;
        dbInfo.name = group.strGroupName;
        dbInfo.nPermission = group.nUserGroup;
        dbInfo.kbGUID = group.strGroupGUID;
        dbInfo.serverUrl = group.strDatabaseServer;
        dbInfo.bizGUID = group.bizGUID;
        dbInfo.bizName = group.bizName;
        m_dbMgr.db(group.strGroupGUID).setDatabaseInfo(dbInfo);
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

    syncUserFoldersStart();
}

void CWizSync::syncUserFoldersStart()
{
    qDebug() << "[Syncing]start syncing folders...";

    // 3. sync user folders, use ks url
    m_kbSync->setKbUrl(WizGlobal()->userInfo().strDatabaseServer);

    qint64 nVersionLocal = m_dbMgr.db().GetObjectVersion("folder");
    if (nVersionLocal == 1) {
        // dirty
        uploadUserFolders();
    } else {
        downloadUserFolders();
    }
}

void CWizSync::uploadUserFolders()
{
    qDebug() << "[Syncing]upload folders...";

    CWizStdStringArray arrayFolder;
    m_dbMgr.db().GetExtraFolder(arrayFolder);
    m_kbSync->callFolderPostList(arrayFolder);
}

void CWizSync::on_folderPostListDone(qint64 nVersion)
{
    qDebug() << "[Syncing]upload folders done, version: " << nVersion;

    m_dbMgr.db().SetObjectVersion("folder", nVersion);
    syncUserFoldersEnd();
}

void CWizSync::downloadUserFolders()
{
    m_kbSync->callFolderGetVersion();
}

void CWizSync::on_folderGetVersionDone(qint64 nVersion)
{
    qint64 nVersionLocal = m_dbMgr.db().GetObjectVersion("folder");

    qDebug() << "[Syncing]sync folders, local: " << nVersionLocal
             << " remote: " << nVersion;

    if (nVersionLocal < nVersion) {
        m_kbSync->callFolderGetList();
    } else {
        syncUserFoldersEnd();
    }
}

void CWizSync::on_folderGetListDone(const QStringList& listFolder, qint64 nVersion)
{
    CWizStdStringArray arrayFolder;
    m_dbMgr.db().GetExtraFolder(arrayFolder);

    // if remote have folders that local not exist, create it.
    for (int i = 0; i < listFolder.size(); i++) {
        QString strFolder = listFolder.at(i);
        if (::WizFindInArray(arrayFolder, strFolder) == -1) {
            m_dbMgr.db().AddExtraFolder(strFolder);

            qDebug() << "[Syncing]create new folder: " << strFolder;
        }
    }

    // if folder exist on local but not exist on remote and it's empty
    // maybe means that folder should be deleted
    CWizStdStringArray::const_iterator it;
    for (it = arrayFolder.begin(); it != arrayFolder.end(); it++) {
        if (!listFolder.contains(*it)) {
            int nSize = 0;
            m_dbMgr.db().GetDocumentsSizeByLocation(*it, nSize, true);

            if (!nSize) {
                m_dbMgr.db().LogDeletedFolder(*it);

                qDebug() << "[Syncing]delete old folder: " << *it;
            }
        }
    }

    // reset
    arrayFolder.clear();

    for (int i = 0; i < listFolder.size(); i++) {
        arrayFolder.push_back(listFolder.at(i));
    }

    m_dbMgr.db().SetExtraFolder(arrayFolder);
    m_dbMgr.db().SetObjectVersion("folder", nVersion);

    syncUserFoldersEnd();
}

void CWizSync::syncUserFoldersEnd()
{
    qDebug() << "[Syncing]end syncing folders...";

    // 4. sync user private data
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

    // 5. reset db info and start sync group data one by one
    Q_EMIT processLog(tr("Begin syncing group data: %1").arg(group.strGroupName));
    m_kbSync->startSync(group.strGroupGUID);
}
