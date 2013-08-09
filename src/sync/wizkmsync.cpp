#include "wizkmxmlrpc.h"
#include "wizkmsync.h"
#include <QThread>
#include <QDebug>

#include "../share/wizDatabase.h"

class CWizKMSyncEvents : public IWizKMSyncEvents
{
    virtual void OnSyncProgress(int pos)
    {
        qDebug() << "[OnSyncProgress] pos = " << pos;
    }

    virtual HRESULT OnText(WizKMSyncProgressStatusType type, const QString& strStatus)
    {
        qDebug() << "[OnText] type: " << type << " status: " << strStatus;
        return 0;
    }

    virtual void SetLastErrorCode(int nErrorCode) {}
    virtual void SetDatabaseCount(int count)
    {
        qDebug() << "[SetDatabaseCount] count = " << count;
    }

    virtual void SetCurrentDatabase(int index)
    {
        qDebug() << "[SetCurrentDatabase] index = " << index;
    }

    virtual void OnTrafficLimit(IWizSyncableDatabase* pDatabase) {}
    virtual void OnStorageLimit(IWizSyncableDatabase* pDatabase) {}
    virtual void OnUploadDocument(const QString& strDocumentGUID, bool bDone)
    {
        qDebug() << "[SetCurrentDatabase] guid: " << strDocumentGUID;
    }

    virtual void OnBeginKb(const QString& strKbGUID)
    {
        qDebug() << "[OnBeginKb] kb_guid: " << strKbGUID;
    }

    virtual void OnEndKb(const QString& strKbGUID)
    {
        qDebug() << "[OnEndKb] kb_guid: " << strKbGUID;
    }
};


/* ---------------------------- CWizKMSyncThead ---------------------------- */
CWizKMSyncThread::CWizKMSyncThread(CWizDatabase& db, QObject* parent)
    : QThread(parent)
    , m_db(db)
{
}

void CWizKMSyncThread::run()
{
    CWizKMSyncEvents events;
    ::WizSyncDatabase(&events, &m_db, true, true);
}

void CWizKMSyncThread::startSync()
{
    if (isRunning())
        return;

    qDebug() << ">>>>>>>>>>>>>> start syncing...";
    start();
}
