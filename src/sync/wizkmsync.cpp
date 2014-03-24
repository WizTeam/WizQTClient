#include "wizkmsync.h"

#include <QDebug>

#include "apientry.h"
#include "token.h"

#include "../share/wizDatabase.h"
#include "sync_p.h"

using namespace WizService;


/* ---------------------------- CWizKMSyncThead ---------------------------- */
void CWizKMSyncEvents::OnSyncProgress(int pos)
{
    Q_UNUSED(pos);
}

HRESULT CWizKMSyncEvents::OnText(WizKMSyncProgressStatusType type, const QString& strStatus)
{
    Q_UNUSED(type);
    qDebug() << "[Sync]" << strStatus;

    Q_EMIT messageReady(strStatus);
    return 0;
}

void CWizKMSyncEvents::SetDatabaseCount(int count)
{
    qDebug() << "[Sync]SetDatabaseCount count = " << count;
}

void CWizKMSyncEvents::SetCurrentDatabase(int index)
{
    qDebug() << "[Sync]SetCurrentDatabase index = " << index;
}

void CWizKMSyncEvents::OnTrafficLimit(IWizSyncableDatabase* pDatabase)
{
}

void CWizKMSyncEvents::OnStorageLimit(IWizSyncableDatabase* pDatabase)
{

}

void CWizKMSyncEvents::OnUploadDocument(const QString& strDocumentGUID, bool bDone)
{
    qDebug() << "[Sync]SetCurrentDatabase guid: " << strDocumentGUID;
}

void CWizKMSyncEvents::OnBeginKb(const QString& strKbGUID)
{
    qDebug() << "[Sync]OnBeginKb kb_guid: " << strKbGUID;
}

void CWizKMSyncEvents::OnEndKb(const QString& strKbGUID)
{
    qDebug() << "[Sync]OnEndKb kb_guid: " << strKbGUID;
}


/* ---------------------------- CWizKMSyncThead ---------------------------- */

#define FULL_SYNC_INTERVAL 15 * 60

static CWizKMSyncThread* g_pSyncThread = NULL;
CWizKMSyncThread::CWizKMSyncThread(CWizDatabase& db, QObject* parent)
    : QThread(parent)
    , m_db(db)
    , m_bNeedSyncAll(false)
    , m_pEvents(NULL)
    , m_bBackground(true)
    , m_mutex(QMutex::Recursive)
{
    int delaySeconds = - 15 * 60 + 10;   //delay for 10 seconds
    m_tLastSyncAll = QDateTime::currentDateTime().addSecs(delaySeconds);
    //
    m_pEvents = new CWizKMSyncEvents();
    //
    connect(m_pEvents, SIGNAL(messageReady(const QString&)), SIGNAL(processLog(const QString&)));
    //
    g_pSyncThread = this;
}
CWizKMSyncThread::~CWizKMSyncThread()
{
    g_pSyncThread = NULL;
}

void CWizKMSyncThread::run()
{
    while (1)
    {
        if (m_pEvents->IsStop())
            return;
        //
        if (doSync())
        {
        }
        else
        {
            sleep(1);    //idle
        }
    }
}

void CWizKMSyncThread::startSyncAll(bool bBackground)
{
    m_bNeedSyncAll = true;
    m_bBackground = bBackground;
}


bool CWizKMSyncThread::prepareToken()
{
    QString token = Token::token();
    if (token.isEmpty())
    {
        Q_EMIT syncFinished(Token::lastErrorCode(), Token::lastErrorMessage());
        return false;
    }
    //
    m_info = Token::info();
    //
    return true;
}

bool CWizKMSyncThread::doSync()
{
    if (needSyncAll())
    {
        qDebug() << "[Sync] syncing all started, thread:" << QThread::currentThreadId();

        syncAll();
        m_tLastSyncAll = QDateTime::currentDateTime();
        return true;
    }
    else if (needQuickSync())
    {
        qDebug() << "[Sync] quick syncing started, thread:" << QThread::currentThreadId();
        //
        quickSync();
        return true;
    }
    //
    return false;
}

bool CWizKMSyncThread::needSyncAll()
{
    if (m_bNeedSyncAll)
        return true;

    QDateTime tNow = QDateTime::currentDateTime();
    int seconds = m_tLastSyncAll.secsTo(tNow);
    if (seconds > FULL_SYNC_INTERVAL)
    {
        m_bNeedSyncAll = true;
    }

    return m_bNeedSyncAll;
}


class CWizKMSyncThreadHelper
{
    CWizKMSyncThread* m_pThread;
public:
    CWizKMSyncThreadHelper(CWizKMSyncThread* pThread, bool syncAll)
        :m_pThread(pThread)
    {
        Q_EMIT m_pThread->syncStarted(syncAll);
    }
    ~CWizKMSyncThreadHelper()
    {
        Q_EMIT m_pThread->syncFinished(m_pThread->m_pEvents->GetLastErrorCode(), "");
    }
};

bool CWizKMSyncThread::syncAll()
{
    m_bNeedSyncAll = false;
    //
    CWizKMSyncThreadHelper helper(this, true);
    Q_UNUSED(helper);
    //
    m_pEvents->SetLastErrorCode(0);
    if (!prepareToken())
        return false;

    syncUserCert();

    ::WizSyncDatabase(m_info, m_pEvents, &m_db, true, m_bBackground);

    return true;
}


bool CWizKMSyncThread::quickSync()
{
    CWizKMSyncThreadHelper helper(this, false);
    //
    Q_UNUSED(helper);
    //
    if (!prepareToken())
        return false;
    //
    QString kbGuid;
    while (peekQuickSyncKb(kbGuid))
    {
        if (kbGuid.isEmpty())
        {
            CWizKMSync syncPrivate(&m_db, m_info, m_pEvents, FALSE, TRUE, NULL);
            //
            if (syncPrivate.Sync())
            {
                m_db.SaveLastSyncTime();
            }
        }
        else
        {
            WIZGROUPDATA group;
            if (m_db.GetGroupData(kbGuid, group))
            {
                IWizSyncableDatabase* pGroupDatabase = m_db.GetGroupDatabase(group);
                //
                WIZUSERINFO userInfo = m_info;
                //userInfo.strDatabaseServer = group.strDatabaseServer;
                QString token = WizService::Token::token();
                userInfo.strToken = token;
                userInfo.strKbGUID = group.strGroupGUID;
                userInfo.strDatabaseServer = WizService::ApiEntry::kUrlFromGuid(token, userInfo.strKbGUID);
                //
                CWizKMSync syncGroup(pGroupDatabase, userInfo, m_pEvents, TRUE, TRUE, NULL);
                //
                if (syncGroup.Sync())
                {
                    pGroupDatabase->SaveLastSyncTime();
                }
                //
                m_db.CloseGroupDatabase(pGroupDatabase);
            }
        }

    }
    //
    //
    return true;
}



// FIXME: remove this to syncing flow
void CWizKMSyncThread::syncUserCert()
{
    QString strN, stre, strd, strHint;

    CWizKMAccountsServer serser(WizService::ApiEntry::syncUrl());
    if (serser.GetCert(m_db.GetUserId(), m_db.GetPassword(), strN, stre, strd, strHint)) {
        m_db.SetUserCert(strN, stre, strd, strHint);
    }
}

bool CWizKMSyncThread::needQuickSync()
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    //
    if (m_setQuickSyncKb.empty())
        return false;
    //
    QDateTime tNow = QDateTime::currentDateTime();
    int seconds = m_tLastKbModified.secsTo(tNow);
    //
    if (seconds >= 3)
        return true;
    //
    return false;
}


void CWizKMSyncThread::stopSync()
{
    if (isRunning() && m_pEvents) {
        m_pEvents->SetStop(true);
    }
}
void CWizKMSyncThread::addQuickSyncKb(const QString& kbGuid)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    //
    m_setQuickSyncKb.insert(kbGuid);
    //
    m_tLastKbModified = QDateTime::currentDateTime();
}

bool CWizKMSyncThread::peekQuickSyncKb(QString& kbGuid)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    //
    if (m_setQuickSyncKb.empty())
        return false;
    //
    kbGuid = *m_setQuickSyncKb.begin();
    m_setQuickSyncKb.erase(m_setQuickSyncKb.begin());
    return true;
}
void CWizKMSyncThread::quickSyncKb(const QString& kbGuid)
{
    if (!g_pSyncThread)
        return;
    //
    g_pSyncThread->addQuickSyncKb(kbGuid);
}
