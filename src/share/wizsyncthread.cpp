#include "wizsyncthread.h"

CWizSyncThread::CWizSyncThread(CWizDatabase& db, const CString& strAccountsApiURL, QObject *parent /* = 0 */)
    : QThread(parent)
    , m_db(db)
    , m_strAccountsApiURL(strAccountsApiURL)
    , m_sync(0)
    , m_bDownloadAllNotesData(false)
    , m_bNeedResetProxy(false)
    , m_bIsStarted(false)
{
}

void CWizSyncThread::startSync()
{
    if (m_bIsStarted)
        return;

    m_bIsStarted = true;

    start();
}

void CWizSyncThread::run()
{
    m_sync = new CWizSync(m_db, m_strAccountsApiURL);

    // chain up
    connect(m_sync, SIGNAL(syncStarted()), SIGNAL(syncStarted()));
    connect(m_sync, SIGNAL(syncLogined()), SIGNAL(syncLogined()));
    connect(m_sync, SIGNAL(progressChanged(int)), SIGNAL(progressChanged(int)));
    connect(m_sync, SIGNAL(processLog(const QString&)), SIGNAL(processLog(const QString&)));
    connect(m_sync, SIGNAL(processDebugLog(const QString&)), SIGNAL(processDebugLog(const QString&)));
    connect(m_sync, SIGNAL(processErrorLog(const QString&)), SIGNAL(processErrorLog(const QString&)));
    connect(m_sync, SIGNAL(syncDone(bool)), SIGNAL(syncDone(bool)));

    connect(m_sync, SIGNAL(syncDone(bool)), SLOT(on_syncDone(bool)));

    m_sync->setDownloadAllNotesData(m_bDownloadAllNotesData);
    if (m_bNeedResetProxy)
        m_sync->resetProxy();

    m_sync->startSync();

    exec();
}

void CWizSyncThread::on_syncDone(bool error)
{
    Q_UNUSED(error);

    m_sync->deleteLater();
    m_bIsStarted = false;

    exit();
}

CWizSyncThread::~CWizSyncThread()
{
    m_sync->deleteLater();
}
