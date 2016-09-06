#include "WizKMSync.h"

#include <QDebug>
#include <QApplication>

#include "WizApiEntry.h"
#include "WizToken.h"

#include "../share/WizDatabase.h"
#include "WizKMSync_p.h"


/* ---------------------------- CWizKMSyncThead ---------------------------- */
void WizKMSyncEvents::onSyncProgress(int pos)
{
    Q_UNUSED(pos);
}

HRESULT WizKMSyncEvents::onText(WizKMSyncProgressMessageType type, const QString& strStatus)
{
    Q_UNUSED(type);
    qInfo() << "[Sync]"  << strStatus;

    Q_EMIT messageReady(strStatus);
    return 0;
}

HRESULT WizKMSyncEvents::onMessage(WizKMSyncProgressMessageType type, const QString& strTitle, const QString& strMessage)
{
    emit promptMessageRequest(type, strTitle, strMessage);
    return S_OK;
}

HRESULT WizKMSyncEvents::onBubbleNotification(const QVariant& param)
{
    emit bubbleNotificationRequest(param);
    return S_OK;
}

void WizKMSyncEvents::setDatabaseCount(int count)
{
    onStatus(QObject::tr("Set database count: %1").arg(count));
}

void WizKMSyncEvents::setCurrentDatabase(int index)
{
    onStatus(QObject::tr("Set current database index: %1").arg(index));
}

void WizKMSyncEvents::clearLastSyncError(IWizSyncableDatabase* pDatabase)
{
    // FIXME
    Q_UNUSED(pDatabase);
}

void WizKMSyncEvents::onTrafficLimit(IWizSyncableDatabase* pDatabase)
{
    // FIXME
    Q_UNUSED(pDatabase);
}

void WizKMSyncEvents::onStorageLimit(IWizSyncableDatabase* pDatabase)
{
    // FIXME
    Q_UNUSED(pDatabase);
}

void WizKMSyncEvents::onBizServiceExpr(IWizSyncableDatabase *pDatabase)
{
    // FIXME
    Q_UNUSED(pDatabase);
}

void WizKMSyncEvents::onBizNoteCountLimit(IWizSyncableDatabase* pDatabase)
{
    // FIXME
    Q_UNUSED(pDatabase);
}

void WizKMSyncEvents::onUploadDocument(const QString& strDocumentGUID, bool bDone)
{
    if (bDone)
    {
        onStatus(QObject::tr("Upload document: %1 finished").arg(strDocumentGUID));
    }
    else
    {
        onStatus(QObject::tr("Upload document: %1 start").arg(strDocumentGUID));
    }
}

void WizKMSyncEvents::onBeginKb(const QString& strKbGUID)
{
    onStatus(QObject::tr("OnBeginKb kb_guid: %1").arg(strKbGUID));
}

void WizKMSyncEvents::onEndKb(const QString& strKbGUID)
{
    onStatus(QObject::tr("OnEndKb kb_guid: %1").arg(strKbGUID));
}

/* ---------------------------- CWizKMSyncThead ---------------------------- */

#define DEFAULT_FULL_SYNC_SECONDS_INTERVAL 15 * 60
#define DEFAULT_QUICK_SYNC_MILLISECONDS_INTERVAL 1000

static WizKMSyncThread* g_pSyncThread = NULL;
WizKMSyncThread::WizKMSyncThread(WizDatabase& db, QObject* parent)
    : QThread(parent)
    , m_db(db)
    , m_bNeedSyncAll(false)
    , m_bNeedDownloadMessages(false)
    , m_pEvents(NULL)
    , m_bBackground(true)
    , m_nFullSyncSecondsInterval(DEFAULT_FULL_SYNC_SECONDS_INTERVAL)
    , m_bBusy(false)
    , m_bPause(false)
{
    m_tLastSyncAll = QDateTime::currentDateTime();
    //
    m_pEvents = new WizKMSyncEvents();
    //
    connect(m_pEvents, SIGNAL(messageReady(const QString&)), SIGNAL(processLog(const QString&)));
    connect(m_pEvents, SIGNAL(promptMessageRequest(int, QString, QString)), SIGNAL(promptMessageRequest(int, QString, QString)));
    connect(m_pEvents, SIGNAL(bubbleNotificationRequest(const QVariant&)), SIGNAL(bubbleNotificationRequest(const QVariant&)));

    m_timer.setSingleShot(true);
    connect(this, SIGNAL(startTimer(int)), &m_timer, SLOT(start(int)));
    connect(this, SIGNAL(stopTimer()), &m_timer, SLOT(stop()));
    connect(&m_timer, SIGNAL(timeout()), SLOT(on_timerOut()));
    //
    g_pSyncThread = this;
}
WizKMSyncThread::~WizKMSyncThread()
{
    g_pSyncThread = NULL;
}

void WizKMSyncThread::run()
{
    while (!m_pEvents->isStop())
    {
        m_mutex.lock();
        m_wait.wait(&m_mutex, 1000 * 3);
        m_mutex.unlock();

        if (m_pEvents->isStop())
        {
            return;
        }
        //
        if (m_bPause)
            continue;
        //
        m_bBusy = true;
        doSync();
        m_bBusy = false;
    }
}

void WizKMSyncThread::syncAfterStart()
{
#ifndef QT_DEBUG
    if (m_tLastSyncAll.secsTo(QDateTime::currentDateTime()) < 5)
        return;

    startSyncAll(false);
#endif
}

void WizKMSyncThread::on_timerOut()
{
    m_mutex.lock();
    m_wait.wakeAll();
    m_mutex.unlock();
}

void WizKMSyncThread::startSyncAll(bool bBackground)
{
    m_mutex.lock();
    m_bNeedSyncAll = true;
    m_bBackground = bBackground;

    m_wait.wakeAll();
    m_mutex.unlock();
}

bool WizKMSyncThread::isBackground() const
{
    return m_bBackground;
}


bool WizKMSyncThread::prepareToken()
{
    QString token = WizToken::token();
    if (token.isEmpty())
    {
        Q_EMIT syncFinished(WizToken::lastErrorCode(), WizToken::lastErrorMessage(), isBackground());
        return false;
    }
    //
    m_info = WizToken::info();
    //
    return true;
}

bool WizKMSyncThread::doSync()
{
    if (needSyncAll())
    {
        qDebug() << "[Sync] syncing all started, thread:" << QThread::currentThreadId();

        syncAll();
        m_bNeedSyncAll = false;
        m_tLastSyncAll = QDateTime::currentDateTime();
        emit startTimer(m_nFullSyncSecondsInterval * 1000 + 1);
        return true;
    }
    else if (needQuickSync())
    {
        qDebug() << "[Sync] quick syncing started, thread:" << QThread::currentThreadId();
        //
        m_bBackground = true;
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

bool WizKMSyncThread::clearCurrentToken()
{
    WizToken::clearToken();
    WizToken::clearLastError();
    return true;
}

void WizKMSyncThread::waitForDone()
{
    stopSync();
    //
    ::WizWaitForThread(this);
}

bool WizKMSyncThread::needSyncAll()
{
    if (m_bNeedSyncAll)
        return true;

#ifdef QT_DEBUG
    return false;
#endif

    QDateTime tNow = QDateTime::currentDateTime();
    int seconds = m_tLastSyncAll.secsTo(tNow);
    if (m_nFullSyncSecondsInterval > 0 && seconds > m_nFullSyncSecondsInterval)
    {
        m_bNeedSyncAll = true;
        m_bBackground = true;
    }

    return m_bNeedSyncAll;
}


class CWizKMSyncThreadHelper
{
    WizKMSyncThread* m_pThread;
public:
    CWizKMSyncThreadHelper(WizKMSyncThread* pThread, bool syncAll)
        :m_pThread(pThread)
    {
        Q_EMIT m_pThread->syncStarted(syncAll);
    }
    ~CWizKMSyncThreadHelper()
    {
        Q_EMIT m_pThread->syncFinished(m_pThread->m_pEvents->getLastErrorCode()
                                       , m_pThread->m_pEvents->getLastErrorMessage()
                                       , m_pThread->isBackground());
        m_pThread->m_pEvents->clearLastErrorMessage();
    }
};

bool WizKMSyncThread::syncAll()
{
    m_bNeedSyncAll = false;
    //
    CWizKMSyncThreadHelper helper(this, true);
    Q_UNUSED(helper);
    //
    m_pEvents->setLastErrorCode(0);
    if (!prepareToken())
        return false;

    if (m_db.kbGUID().isEmpty()) {
        m_db.setKbGUID(WizToken::info().strKbGUID);
    }

    syncUserCert();

    ::WizSyncDatabase(m_info, m_pEvents, &m_db, m_bBackground);

    return true;
}


bool WizKMSyncThread::quickSync()
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
            WizKMSync syncPrivate(&m_db, m_info, m_pEvents, FALSE, TRUE, NULL);
            //
            if (syncPrivate.sync())
            {
                m_db.saveLastSyncTime();
            }
        }
        else
        {
            WIZGROUPDATA group;
            if (m_db.getGroupData(kbGuid, group))
            {
                IWizSyncableDatabase* pGroupDatabase = m_db.getGroupDatabase(group);
                //
                WIZUSERINFO userInfo = m_info;
                userInfo.strKbGUID = group.strGroupGUID;
                userInfo.strDatabaseServer = group.strDatabaseServer;
                if (userInfo.strDatabaseServer.isEmpty())
                {
                    userInfo.strDatabaseServer = WizCommonApiEntry::kUrlFromGuid(userInfo.strToken, userInfo.strKbGUID);
                }
                //
                WizKMSync syncGroup(pGroupDatabase, userInfo, m_pEvents, TRUE, TRUE, NULL);
                //
                if (syncGroup.sync())
                {
                    pGroupDatabase->saveLastSyncTime();
                }
                //
                m_db.closeGroupDatabase(pGroupDatabase);
            }
        }
    }
    //
    //
    return true;
}

bool WizKMSyncThread::downloadMesages()
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
void WizKMSyncThread::syncUserCert()
{
    QString strN, stre, strd, strHint;

    WizKMAccountsServer serser(WizCommonApiEntry::syncUrl());
    if (serser.getCert(m_db.getUserId(), m_db.getPassword(), strN, stre, strd, strHint)) {
        m_db.setUserCert(strN, stre, strd, strHint);
    }
}

bool WizKMSyncThread::needQuickSync()
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    //
    if (m_setQuickSyncKb.empty())
        return false;
    //
    return true;
}

bool WizKMSyncThread::needDownloadMessage()
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


void WizKMSyncThread::stopSync()
{
    if (isRunning() && m_pEvents)
    {
        m_pEvents->setStop(true);
        m_mutex.lock();
        m_wait.wakeAll();
        m_mutex.unlock();
    }
}

void WizKMSyncThread::setFullSyncInterval(int nMinutes)
{
    m_nFullSyncSecondsInterval = nMinutes * 60;
}

void WizKMSyncThread::addQuickSyncKb(const QString& kbGuid)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    //
    m_setQuickSyncKb.insert(kbGuid);
    //
    m_tLastKbModified = QDateTime::currentDateTime();

    QTimer::singleShot(DEFAULT_QUICK_SYNC_MILLISECONDS_INTERVAL + 1, this, SLOT(on_timerOut()));
}

void WizKMSyncThread::quickDownloadMesages()
{
    // 一分钟中内不重复查询，防止过于频繁的请求
    static QTime time = QTime::currentTime().addSecs(-61);
    if (time.secsTo(QTime::currentTime()) < 60)
        return;
    time = QTime::currentTime();

    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    m_bNeedDownloadMessages = true;
    m_wait.wakeAll();
}

bool WizKMSyncThread::peekQuickSyncKb(QString& kbGuid)
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
void WizKMSyncThread::quickSyncKb(const QString& kbGuid)
{
    if (!g_pSyncThread)
        return;
    //
    g_pSyncThread->addQuickSyncKb(kbGuid);
}
bool WizKMSyncThread::isBusy()
{
    if (!g_pSyncThread)
        return false;
    //
    return g_pSyncThread->m_bBusy;
}

void WizKMSyncThread::waitUntilIdleAndPause()
{
    while(isBusy())
    {
        QThread::sleep(1);
    }
    //
    setPause(true);
}

void WizKMSyncThread::setPause(bool pause)
{
    if (!g_pSyncThread)
        return;
    //
    g_pSyncThread->m_bPause = pause;
}
