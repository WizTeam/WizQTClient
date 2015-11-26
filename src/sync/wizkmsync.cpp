#include "wizkmsync.h"

#include <QDebug>
#include <QApplication>

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

HRESULT CWizKMSyncEvents::OnText(WizKMSyncProgressMessageType type, const QString& strStatus)
{
    Q_UNUSED(type);
    qInfo() << "[Sync]"  << strStatus;

    Q_EMIT messageReady(strStatus);
    return 0;
}

HRESULT CWizKMSyncEvents::OnMessage(WizKMSyncProgressMessageType type, const QString& strTitle, const QString& strMessage)
{
    emit promptMessageRequest(type, strTitle, strMessage);
    return S_OK;
}

HRESULT CWizKMSyncEvents::OnBubbleNotification(const QVariant& param)
{
    emit bubbleNotificationRequest(param);
    return S_OK;
}

void CWizKMSyncEvents::SetDatabaseCount(int count)
{
    OnStatus(QObject::tr("Set database count: %1").arg(count));
}

void CWizKMSyncEvents::SetCurrentDatabase(int index)
{
    OnStatus(QObject::tr("Set current database index: %1").arg(index));
}

void CWizKMSyncEvents::ClearLastSyncError(IWizSyncableDatabase* pDatabase)
{
    // FIXME
    Q_UNUSED(pDatabase);
}

void CWizKMSyncEvents::OnTrafficLimit(IWizSyncableDatabase* pDatabase)
{
    // FIXME
    Q_UNUSED(pDatabase);
}

void CWizKMSyncEvents::OnStorageLimit(IWizSyncableDatabase* pDatabase)
{
    // FIXME
    Q_UNUSED(pDatabase);
}

void CWizKMSyncEvents::OnBizServiceExpr(IWizSyncableDatabase *pDatabase)
{
    // FIXME
    Q_UNUSED(pDatabase);
}

void CWizKMSyncEvents::OnBizNoteCountLimit(IWizSyncableDatabase* pDatabase)
{
    // FIXME
    Q_UNUSED(pDatabase);
}

void CWizKMSyncEvents::OnUploadDocument(const QString& strDocumentGUID, bool bDone)
{
    if (bDone)
    {
        OnStatus(QObject::tr("Upload document: %1 finished").arg(strDocumentGUID));
    }
    else
    {
        OnStatus(QObject::tr("Upload document: %1 start").arg(strDocumentGUID));
    }
}

void CWizKMSyncEvents::OnBeginKb(const QString& strKbGUID)
{
    OnStatus(QObject::tr("OnBeginKb kb_guid: %1").arg(strKbGUID));
}

void CWizKMSyncEvents::OnEndKb(const QString& strKbGUID)
{
    OnStatus(QObject::tr("OnEndKb kb_guid: %1").arg(strKbGUID));
}

/* ---------------------------- CWizKMSyncThead ---------------------------- */

#define DEFAULT_FULL_SYNC_INTERVAL 15 * 60

static CWizKMSyncThread* g_pSyncThread = NULL;
CWizKMSyncThread::CWizKMSyncThread(CWizDatabase& db, QObject* parent)
    : QThread(parent)
    , m_db(db)
    , m_bNeedSyncAll(false)
    , m_bNeedDownloadMessages(false)
    , m_pEvents(NULL)
    , m_bBackground(true)
    , m_mutex(QMutex::Recursive)
    , m_nfullSyncInterval(DEFAULT_FULL_SYNC_INTERVAL)
{
    m_tLastSyncAll = QDateTime::currentDateTime();
    //
    m_pEvents = new CWizKMSyncEvents();
    //
    connect(m_pEvents, SIGNAL(messageReady(const QString&)), SIGNAL(processLog(const QString&)));
    connect(m_pEvents, SIGNAL(promptMessageRequest(int, QString, QString)), SIGNAL(promptMessageRequest(int, QString, QString)));
    connect(m_pEvents, SIGNAL(bubbleNotificationRequest(const QVariant&)), SIGNAL(bubbleNotificationRequest(const QVariant&)));
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
        {
            return;
        }
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

void CWizKMSyncThread::syncAfterStart()
{
#ifndef QT_DEBUG
    if (m_tLastSyncAll.secsTo(QDateTime::currentDateTime()) < 5)
        return;

    startSyncAll(false);
#endif
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
        m_bNeedSyncAll = false;
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
    else if (needDownloadMessage())
    {
        qDebug() <<  "[Sync] quick download messages started, thread:" << QThread::currentThreadId();
        //
        downloadMesages();
        return true;
    }
    //
    return false;
}

bool CWizKMSyncThread::clearCurrentToken()
{
    Token::clearToken();
    Token::clearLastError();
    return true;
}

void CWizKMSyncThread::waitForDone()
{
    stopSync();
    //
    ::WizWaitForThread(this);
}

bool CWizKMSyncThread::needSyncAll()
{
    if (m_bNeedSyncAll)
        return true;

#ifdef QT_DEBUG
    return false;
#endif

    QDateTime tNow = QDateTime::currentDateTime();
    int seconds = m_tLastSyncAll.secsTo(tNow);
    if (m_nfullSyncInterval > 0 && seconds > m_nfullSyncInterval)
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

    if (m_db.kbGUID().isEmpty()) {
        m_db.setKbGUID(Token::info().strKbGUID);
    }

    syncUserCert();

    ::WizSyncDatabase(m_info, m_pEvents, &m_db, m_bBackground);

    return true;
}


bool CWizKMSyncThread::quickSync()
{
    CWizKMSyncThreadHelper helper(this, false);
    //
    Q_UNUSED(helper);
    //
    QString kbGuid;
    while (peekQuickSyncKb(kbGuid))
    {
        if (!prepareToken())
            return false;

        if (kbGuid.isEmpty() || m_db.kbGUID() == kbGuid)
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
                userInfo.strKbGUID = group.strGroupGUID;
                userInfo.strDatabaseServer = group.strDatabaseServer;
                if (userInfo.strDatabaseServer.isEmpty())
                {
                    userInfo.strDatabaseServer = WizService::CommonApiEntry::kUrlFromGuid(userInfo.strToken, userInfo.strKbGUID);
                }
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

bool CWizKMSyncThread::downloadMesages()
{
    if (!prepareToken())
        return false;

    ::WizQuickDownloadMessage(m_info, m_pEvents, &m_db);

    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    m_bNeedDownloadMessages = false;
    return true;
}



// FIXME: remove this to syncing flow
void CWizKMSyncThread::syncUserCert()
{
    QString strN, stre, strd, strHint;

    CWizKMAccountsServer serser(WizService::CommonApiEntry::syncUrl());
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

bool CWizKMSyncThread::needDownloadMessage()
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    //
    if (m_bNeedDownloadMessages)
    {
        m_bNeedDownloadMessages = false;
        return true;
    }
    return false;
}


void CWizKMSyncThread::stopSync()
{
    if (isRunning() && m_pEvents) {
        m_pEvents->SetStop(true);
    }
}

void CWizKMSyncThread::setFullSyncInterval(int nMinutes)
{
    m_nfullSyncInterval = nMinutes * 60;
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

void CWizKMSyncThread::quickDownloadMesages()
{
    // 一分钟中内不重复查询，防止过于频繁的请求
    static QTime time = QTime::currentTime().addSecs(-61);
    if (time.secsTo(QTime::currentTime()) < 60)
        return;
    time = QTime::currentTime();

    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    m_bNeedDownloadMessages = true;
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
