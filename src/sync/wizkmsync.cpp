#include "wizkmsync.h"

#include <QDebug>

#include "../share/wizDatabase.h"


/* ---------------------------- CWizKMSyncThead ---------------------------- */
void CWizKMSyncEvents::OnSyncProgress(int pos)
{
    qDebug() << "[OnSyncProgress] pos = " << pos;
}

HRESULT CWizKMSyncEvents::OnText(WizKMSyncProgressStatusType type, const QString& strStatus)
{
    qDebug() << "[OnText] type: " << type << " status: " << strStatus;
    TOLOG(strStatus);

    Q_EMIT messageReady(strStatus);
    return 0;
}

void CWizKMSyncEvents::SetDatabaseCount(int count)
{
    qDebug() << "[SetDatabaseCount] count = " << count;
}

void CWizKMSyncEvents::SetCurrentDatabase(int index)
{
    qDebug() << "[SetCurrentDatabase] index = " << index;
}

void CWizKMSyncEvents::OnTrafficLimit(IWizSyncableDatabase* pDatabase)
{
}

void CWizKMSyncEvents::OnStorageLimit(IWizSyncableDatabase* pDatabase)
{

}

void CWizKMSyncEvents::OnUploadDocument(const QString& strDocumentGUID, bool bDone)
{
    qDebug() << "[SetCurrentDatabase] guid: " << strDocumentGUID;
}

void CWizKMSyncEvents::OnBeginKb(const QString& strKbGUID)
{
    qDebug() << "[OnBeginKb] kb_guid: " << strKbGUID;
}

void CWizKMSyncEvents::OnEndKb(const QString& strKbGUID)
{
    qDebug() << "[OnEndKb] kb_guid: " << strKbGUID;
}


/* ---------------------------- CWizKMSyncThead ---------------------------- */
CWizKMSyncThread::CWizKMSyncThread(CWizDatabase& db, QObject* parent)
    : QThread(parent)
    , m_db(db)
{
    connect(this, SIGNAL(finished()), SLOT(on_syncFinished()));
}

void CWizKMSyncThread::run()
{
    syncUserCert();

    if (!m_pEvents) {
        m_pEvents = new CWizKMSyncEvents();
        connect(m_pEvents, SIGNAL(messageReady(const QString&)), SIGNAL(processLog(const QString&)));
    }

    m_pEvents->SetLastErrorCode(0);
    ::WizSyncDatabase(m_pEvents, &m_db, true, true);
}

void CWizKMSyncThread::startSync()
{
    if (isRunning())
        return;

    start();
}

void CWizKMSyncThread::on_syncFinished()
{
    m_pEvents->deleteLater();

    Q_EMIT syncFinished(m_pEvents->GetLastErrorCode(), "");
}

void CWizKMSyncThread::syncUserCert()
{
    QString strN, stre, strd, strHint;

    CWizKMAccountsServer serser(::WizKMGetAccountsServerURL(true));
    if (serser.GetCert(m_db.GetUserId(), m_db.GetPassword(), strN, stre, strd, strHint)) {
        m_db.SetUserCert(strN, stre, strd, strHint);
    }
}
