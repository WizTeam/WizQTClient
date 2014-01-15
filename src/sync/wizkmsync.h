#ifndef WIZKMSYNC_H
#define WIZKMSYNC_H

#include <QThread>

#include "sync.h"
#include "wizkmxmlrpc.h"

class CWizDatabase;

class CWizKMSyncEvents : public QObject , public IWizKMSyncEvents
{
    Q_OBJECT

    virtual void OnSyncProgress(int pos);
    virtual HRESULT OnText(WizKMSyncProgressStatusType type, const QString& strStatus);
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
    void startSync(bool bBackground = true);
    void stopSync();

protected:
    virtual void run();

private:
    bool m_bBackground;
    QThread* m_worker;
    CWizDatabase& m_db;
    WIZUSERINFO m_info;
    CWizKMSyncEvents* m_pEvents;
    bool m_bNeedSyncAll;
    QDateTime m_tLastSyncAll;

    Q_INVOKABLE void trySync();
    void doSync();

    bool needSyncAll();
    bool needQuickSync();
    bool syncAll();
    bool quickSync();

    void syncUserCert();

private Q_SLOTS:
    void onTokenAcquired(const QString& strToken);

Q_SIGNALS:
    void syncFinished(int nErrorCode, const QString& strErrorMesssage);
    void processLog(const QString& strStatus);
};

#endif // WIZKMSYNC_H
