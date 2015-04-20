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
    virtual HRESULT OnPromptMessage(const QString& strMessage);
    virtual void SetDatabaseCount(int count);
    virtual void SetCurrentDatabase(int index);
    virtual void ClearLastSyncError(IWizSyncableDatabase* pDatabase);
    virtual void OnTrafficLimit(IWizSyncableDatabase* pDatabase);
    virtual void OnStorageLimit(IWizSyncableDatabase* pDatabase);
    virtual void OnBizServiceExpr(IWizSyncableDatabase* pDatabase);
    virtual void OnBizNoteCountLimit(IWizSyncableDatabase* pDatabase);
    virtual void OnUploadDocument(const QString& strDocumentGUID, bool bDone);
    virtual void OnBeginKb(const QString& strKbGUID);
    virtual void OnEndKb(const QString& strKbGUID);

Q_SIGNALS:
    void messageReady(const QString& strStatus);
    void promptMessageRequest(const QString& strMsg);
};


class CWizKMSyncThread : public QThread
{
    Q_OBJECT

public:
    CWizKMSyncThread(CWizDatabase& db, QObject* parent = 0);
    ~CWizKMSyncThread();
    void startSyncAll(bool bBackground = true);
    void stopSync();
    //
    void setFullSyncInterval(int nMinutes);
    //
    void addQuickSyncKb(const QString& kbGuid);
    bool clearCurrentToken();
    //
    void waitForDone();    

public:
    static void quickSyncKb(const QString& kbGuid); //thread safe

protected:
    virtual void run();

private slots:
    void syncAfterStart();

private:
    bool m_bBackground;
    QThread* m_worker;
    CWizDatabase& m_db;
    WIZUSERINFO m_info;
    CWizKMSyncEvents* m_pEvents;
    bool m_bNeedSyncAll;
    QDateTime m_tLastSyncAll;
    int m_nfullSyncInterval;

    //
    QMutex m_mutex;
    std::set<QString> m_setQuickSyncKb;
    QDateTime m_tLastKbModified;

    bool doSync();

    bool prepareToken();
    bool needSyncAll();
    bool needQuickSync();
    bool syncAll();
    bool quickSync();

    void syncUserCert();
    //
    bool peekQuickSyncKb(QString& kbGuid);
    //
    friend class CWizKMSyncThreadHelper;

Q_SIGNALS:
    void syncStarted(bool syncAll);
    void syncFinished(int nErrorCode, const QString& strErrorMesssage);
    void processLog(const QString& strStatus);
    void promptMessageRequest(const QString& strMsg);
};

#endif // WIZKMSYNC_H
