#ifndef WIZKMSYNC_H
#define WIZKMSYNC_H

#include <QThread>

#include "wizkmxmlrpc.h"

class CWizDatabase;

class CWizKMSyncEvents : public QObject , public IWizKMSyncEvents
{
    Q_OBJECT

    virtual void OnSyncProgress(int pos);
    virtual HRESULT OnText(WizKMSyncProgressStatusType type, const QString& strStatus);
    virtual void SetLastErrorCode(int nErrorCode);
    virtual void SetDatabaseCount(int count);
    virtual void SetCurrentDatabase(int index);
    virtual void OnTrafficLimit(IWizSyncableDatabase* pDatabase);
    virtual void OnStorageLimit(IWizSyncableDatabase* pDatabase);
    virtual void OnUploadDocument(const QString& strDocumentGUID, bool bDone);
    virtual void OnBeginKb(const QString& strKbGUID);
    virtual void OnEndKb(const QString& strKbGUID);

Q_SIGNALS:
    void messageReady(const QString& strStatus);
};


class CWizKMSyncThread : public QThread
{
    Q_OBJECT

public:
    CWizKMSyncThread(CWizDatabase& db, QObject* parent = 0);
    void startSync();

protected:
    virtual void run();

private:
    CWizDatabase& m_db;

private Q_SLOTS:
    void on_syncFinished();

Q_SIGNALS:
    void finished(bool noError = true);
    void processLog(const QString& strStatus);
};

#endif // WIZKMSYNC_H
