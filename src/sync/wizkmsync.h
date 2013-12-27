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
    void startSync();
    void stopSync();

protected:
    virtual void run();

private:
    CWizDatabase& m_db;
    WIZUSERINFO m_info;
    QPointer<CWizKMSyncEvents> m_pEvents;
    bool m_bNeedSyncAll;
    bool m_bNeedAccquireToken;
    QDateTime m_tLastSyncAll;
    bool m_bBusy;

    //
    CWizKMAccountsServer m_serverAccounts;
    bool checkTokenCore();
    bool checkToken();
    //
    bool needSyncAll();
    bool needQuickSync();
    bool needAccquireToken();
    bool onIdle();
    //
    bool syncAll();
    bool quickSync();
    bool accquireToken();

    void syncUserCert();

    //
    void onLogin();

private:
    std::vector<QObject*> m_arrTokenListener;
public:
    void acquireToken(QObject* pObject);

private Q_SLOTS:
    void onTokenAcquired(const QString& strToken);
    void on_syncFinished();

Q_SIGNALS:
    void syncFinished(int nErrorCode, const QString& strErrorMesssage);
    void processLog(const QString& strStatus);
};

#endif // WIZKMSYNC_H
