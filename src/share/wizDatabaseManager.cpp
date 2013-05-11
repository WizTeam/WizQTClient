#include "wizDatabaseManager.h"

CWizDatabaseManager::CWizDatabaseManager(const QString& strUserId)
    : m_strUserId(strUserId)
    , m_dbPrivate(NULL)
{
}

CWizDatabaseManager::~CWizDatabaseManager()
{
    closeAll();
}

bool CWizDatabaseManager::open(const QString& strKbGUID)
{
    Q_ASSERT(!m_strUserId.isEmpty());

    CWizDatabase* db = new CWizDatabase();

    bool ret = false;
    if (strKbGUID.isEmpty()) {
        ret = db->openPrivate(m_strUserId, m_strPasswd);
    } else {
        ret = db->openGroup(m_strUserId, strKbGUID);
    }

    if (!ret) {
        delete db;
        return false;
    }

    initSignals(db);

    if (strKbGUID.isEmpty()) {
        m_dbPrivate = db;
    } else {
        m_dbGroups.append(db);
    }

    Q_EMIT databaseOpened(strKbGUID);

    return true;
}

bool CWizDatabaseManager::openAll()
{
    // first, open private db
    if (!open()) {
        TOLOG("open user private database failed");
        return false;
    }

    // second, get groups info
    CWizGroupDataArray arrayGroup;
    if (!m_dbPrivate->getUserGroupInfo(arrayGroup)) {
        TOLOG("Failed to get user group info");
        return true;
    }

    // third, open groups one by one
    CWizGroupDataArray::const_iterator it;
    for (it = arrayGroup.begin(); it != arrayGroup.end(); it++) {
        WIZGROUPDATA group = *it;
        if (!open(group.strGroupGUID)) {
            TOLOG1("failed to open group: %1", group.strGroupName);
        }
    }

    return true;
}

bool CWizDatabaseManager::isOpened(const QString& strKbGUID)
{
    if (strKbGUID.isEmpty() || m_dbPrivate->kbGUID() == strKbGUID) {
        return true;
    }

    QList<CWizDatabase*>::const_iterator it;
    for (it = m_dbGroups.begin(); it != m_dbGroups.end(); it++) {
        CWizDatabase* db = *it;

        if (db->kbGUID() == strKbGUID) {
            return true;
        }
    }

    return false;
}

bool CWizDatabaseManager::isPrivate(const QString& strKbGUID)
{
    Q_ASSERT(!strKbGUID.isEmpty());

    if (m_dbPrivate->kbGUID() == strKbGUID) {
        return true;
    }

    return false;
}

CWizDatabase& CWizDatabaseManager::db(const QString& strKbGUID)
{
    Q_ASSERT(m_dbPrivate);

    if (strKbGUID.isEmpty() || m_dbPrivate->kbGUID() == strKbGUID) {
        return *m_dbPrivate;
    }

    QList<CWizDatabase*>::const_iterator it;
    for (it = m_dbGroups.begin(); it != m_dbGroups.end(); it++) {
        CWizDatabase* db = *it;

        if (db->kbGUID() == strKbGUID) {
            return *db;
        }
    }

    Q_ASSERT(0);
    return *m_dbPrivate;
}

void CWizDatabaseManager::Guids(QStringList& strings)
{
    QList<CWizDatabase*>::const_iterator it;
    for (it = m_dbGroups.begin(); it != m_dbGroups.end(); it++) {
        CWizDatabase* db = *it;
        strings.append(db->kbGUID());
    }
}

int CWizDatabaseManager::count()
{
    return m_dbGroups.size();
}

CWizDatabase& CWizDatabaseManager::at(int i)
{
    Q_ASSERT(i < count() && i >= 0);

    CWizDatabase* db = m_dbGroups.value(i);
    return *db;
}

bool CWizDatabaseManager::removeKb(const QString& strKbGUID)
{
    Q_UNUSED(strKbGUID);
    return false;
}

bool CWizDatabaseManager::close(const QString& strKbGUID)
{
    // should close all groups db before close user db.
    if (!m_dbGroups.isEmpty()) {
        Q_ASSERT(!strKbGUID.isEmpty());
    }

    bool closed = false;
    QList<CWizDatabase*>::iterator it;
    for (it = m_dbGroups.begin(); it != m_dbGroups.end(); it++) {
        CWizDatabase* db = *it;

        if (db->kbGUID() == strKbGUID) {
            db->Close();
            m_dbGroups.erase(it);
            closed = true;
            break;
        }
    }

    if (!closed && !strKbGUID.isEmpty()) {
        TOLOG("WARNING: nothing closed, guid not found");
        return false;
    }

    Q_EMIT databaseClosed(strKbGUID);
    return true;
}

void CWizDatabaseManager::closeAll()
{
    QList<CWizDatabase*>::iterator it;
    for (it = m_dbGroups.begin(); it != m_dbGroups.end(); it++) {
        CWizDatabase* db = *it;

        close(db->kbGUID());
    }

    // close private db at last
    close();
}

void CWizDatabaseManager::initSignals(CWizDatabase* db)
{
    connect(db, SIGNAL(databaseRename(const QString&)),
            SIGNAL(databaseRename(const QString&)));

    connect(db, SIGNAL(databasePermissionChanged(const QString&)),
            SIGNAL(databasePermissionChanged(const QString&)));

    connect(db, SIGNAL(tagCreated(const WIZTAGDATA&)), SIGNAL(tagCreated(const WIZTAGDATA&)));
    connect(db, SIGNAL(tagModified(const WIZTAGDATA&, const WIZTAGDATA&)),
            SIGNAL(tagModified(const WIZTAGDATA&, const WIZTAGDATA&)));
    connect(db, SIGNAL(tagDeleted(const WIZTAGDATA&)), SIGNAL(tagDeleted(const WIZTAGDATA&)));

    connect(db, SIGNAL(styleCreated(const WIZSTYLEDATA&)), SIGNAL(styleCreated(const WIZSTYLEDATA&)));
    connect(db, SIGNAL(styleModified(const WIZSTYLEDATA&, const WIZSTYLEDATA&)),
            SIGNAL(styleModified(const WIZSTYLEDATA&, const WIZSTYLEDATA&)));
    connect(db, SIGNAL(styleDeleted(const WIZSTYLEDATA&)), SIGNAL(styleDeleted(const WIZSTYLEDATA&)));

    connect(db, SIGNAL(documentCreated(const WIZDOCUMENTDATA&)),
            SIGNAL(documentCreated(const WIZDOCUMENTDATA&)));
    connect(db, SIGNAL(documentModified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)),
            SIGNAL(documentModified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)));
    connect(db, SIGNAL(documentDeleted(const WIZDOCUMENTDATA&)),
            SIGNAL(documentDeleted(const WIZDOCUMENTDATA&)));

    connect(db, SIGNAL(documentTagModified(const WIZDOCUMENTDATA&)),
            SIGNAL(documentTagModified(const WIZDOCUMENTDATA&)));

    connect(db, SIGNAL(documentDataModified(const WIZDOCUMENTDATA&)),
            SIGNAL(documentDataModified(const WIZDOCUMENTDATA&)));
    connect(db, SIGNAL(documentAbstractModified(const WIZDOCUMENTDATA&)),
            SIGNAL(documentAbstractModified(const WIZDOCUMENTDATA&)));

    connect(db, SIGNAL(attachmentCreated(const WIZDOCUMENTATTACHMENTDATA&)),
            SIGNAL(attachmentCreated(const WIZDOCUMENTATTACHMENTDATA&)));
    connect(db, SIGNAL(attachmentModified(const WIZDOCUMENTATTACHMENTDATA&, const WIZDOCUMENTATTACHMENTDATA&)),
            SIGNAL(attachmentModified(const WIZDOCUMENTATTACHMENTDATA&, const WIZDOCUMENTATTACHMENTDATA&)));
    connect(db, SIGNAL(attachmentDeleted(const WIZDOCUMENTATTACHMENTDATA&)),
            SIGNAL(attachmentDeleted(const WIZDOCUMENTATTACHMENTDATA&)));

    connect(db, SIGNAL(folderCreated(const CString&)), SIGNAL(folderCreated(const CString&)));
    connect(db, SIGNAL(folderDeleted(const CString&)), SIGNAL(folderDeleted(const CString&)));
}
