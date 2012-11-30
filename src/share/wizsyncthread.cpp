#include "wizsyncthread.h"

CWizSyncThread::CWizSyncThread(CWizExplorerApp& app, QObject *parent /* = 0 */)
    : QThread(parent)
    , m_app(app)
    , m_db(app.database())
    , m_bNeedResetProxy(false)
    , m_bIsStarted(false)
    , m_currentThread(0)
{
    connect(this, SIGNAL(finished()), SLOT(on_syncFinished()));
}

void CWizSyncThread::startSyncing()
{
    if (m_bIsStarted)
        return;

    m_bIsStarted = true;

    start();
}

void CWizSyncThread::run()
{
    m_currentThread = QThread::currentThread();

    m_sync = new CWizSync(m_db, WIZ_API_URL);

    // chain up
    connect(m_sync, SIGNAL(syncStarted()), SIGNAL(syncStarted()));
    connect(m_sync, SIGNAL(syncLogined()), SIGNAL(syncLogined()));
    connect(m_sync, SIGNAL(progressChanged(int)), SIGNAL(progressChanged(int)));
    connect(m_sync, SIGNAL(processLog(const QString&)), SIGNAL(processLog(const QString&)));
    connect(m_sync, SIGNAL(processDebugLog(const QString&)), SIGNAL(processDebugLog(const QString&)));
    connect(m_sync, SIGNAL(processErrorLog(const QString&)), SIGNAL(processErrorLog(const QString&)));
    connect(m_sync, SIGNAL(syncDone(bool)), SIGNAL(syncDone(bool)));

    connect(m_sync, SIGNAL(syncDone(bool)), SLOT(on_syncDone(bool)));

    m_sync->setDownloadAllNotesData(m_app.userSettings().downloadAllNotesData());

    if (m_bNeedResetProxy)
        m_sync->resetProxy();

    m_sync->startSync();

    exec();
}

void CWizSyncThread::on_syncDone(bool error)
{
    m_currentThread->exit(error);
}

void CWizSyncThread::on_syncFinished()
{
    m_sync->deleteLater();
    m_currentThread = 0;

    m_bIsStarted = false;
}
