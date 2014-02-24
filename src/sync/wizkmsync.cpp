#include "wizkmsync.h"

#include <QDebug>

#include "apientry.h"
#include "token.h"

#include "../share/wizDatabase.h"

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

#define FULL_SYNC_INTERVAL 15*60

CWizKMSyncThread::CWizKMSyncThread(CWizDatabase& db, QObject* parent)
    : QThread(parent)
    , m_db(db)
    , m_bNeedSyncAll(true)
    , m_pEvents(NULL)
    , m_bBackground(true)
{
    m_tLastSyncAll = QDateTime::currentDateTime();
    m_pEvents = new CWizKMSyncEvents();
}

void CWizKMSyncThread::run()
{
    doSync();
}

void CWizKMSyncThread::startSync(bool bBackground)
{
    qDebug() << "[Sync]startSync, thread: " << QThread::currentThreadId();
    if (isRunning()) {
        qDebug() << "[Sync]syncing is started, request is schedued"; //FIXME: schedued request
        return;
    }

    m_bNeedSyncAll = true;
    m_bBackground = bBackground;

    trySync();
}

void CWizKMSyncThread::trySync()
{
    qDebug() << "[Sync]trySync, thread: " << QThread::currentThreadId();

    connect(Token::instance(), SIGNAL(tokenAcquired(QString)), SLOT(onTokenAcquired(QString)), Qt::QueuedConnection);
    Token::requestToken();
}

void CWizKMSyncThread::onTokenAcquired(const QString& strToken)
{
    qDebug() << "[Sync]token acquired, thread: " << QThread::currentThreadId();

    Token::instance()->disconnect(this);

    if (strToken.isEmpty()) {
        Q_EMIT syncFinished(Token::lastErrorCode(), Token::lastErrorMessage());
        return;
    }

    m_info = Token::info();

    start(QThread::IdlePriority);
}

void CWizKMSyncThread::doSync()
{
    qDebug() << "[Sync]syncing started, thread:" << QThread::currentThreadId();

    if (needSyncAll())
    {
        syncAll();
        m_tLastSyncAll = QDateTime::currentDateTime();
    }
    else if (needQuickSync())
    {
        quickSync();
    }
}

bool CWizKMSyncThread::needSyncAll()
{
    if (m_bNeedSyncAll)
        return true;

    QDateTime tNow = QDateTime::currentDateTime();
    if (m_tLastSyncAll.secsTo(QDateTime::currentDateTime()) > FULL_SYNC_INTERVAL)
    {
        m_bNeedSyncAll = true;
    }

    return m_bNeedSyncAll;
}

bool CWizKMSyncThread::syncAll()
{
    m_bNeedSyncAll = false;

    syncUserCert();

    connect(m_pEvents, SIGNAL(messageReady(const QString&)), SIGNAL(processLog(const QString&)));

    m_pEvents->SetLastErrorCode(0);
    ::WizSyncDatabase(m_info, m_pEvents, &m_db, true, m_bBackground);

    Q_EMIT syncFinished(m_pEvents->GetLastErrorCode(), "");
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
    return false;
}

bool CWizKMSyncThread::quickSync()
{
    Q_EMIT syncFinished(0, NULL);
    return true;
}

void CWizKMSyncThread::stopSync()
{
    if (isRunning() && m_pEvents) {
        m_pEvents->SetStop(true);
    }
}
