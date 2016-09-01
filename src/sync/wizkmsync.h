#ifndef WIZKMSYNC_H
#define WIZKMSYNC_H

#include <QThread>
#include <QMessageBox>

#include "sync.h"
#include "wizKMServer.h"

class CWizDatabase;

class CWizKMSyncEvents : public QObject , public IWizKMSyncEvents
{
    Q_OBJECT

    virtual void OnSyncProgress(int pos);
    virtual HRESULT OnText(WizKMSyncProgressMessageType type, const QString& strStatus);
    virtual HRESULT OnMessage(WizKMSyncProgressMessageType type, const QString& strTitle, const QString& strMessage);
    virtual HRESULT OnBubbleNotification(const QVariant& param);
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
    void promptMessageRequest(int nType, const QString& strTitle, const QString& strMsg);
    void bubbleNotificationRequest(const QVariant& param);
};


class CWizKMSyncThread : public QThread
{
    Q_OBJECT

public:
    CWizKMSyncThread(CWizDatabase& db, QObject* parent = 0);
    ~CWizKMSyncThread();
    void startSyncAll(bool bBackground = true);
    bool isBackground() const;
    void stopSync();
    //
    void setFullSyncInterval(int nMinutes);
    //
    void addQuickSyncKb(const QString& kbGuid);
    //
    void quickDownloadMesages();

    bool clearCurrentToken();
    //
    void waitForDone();

public:
    static void quickSyncKb(const QString& kbGuid); //thread safe
    static bool isBusy();
    static void waitUntilIdleAndPause();
    static void setPause(bool pause);

signals:
    void startTimer(int interval);
    void stopTimer();

protected:
    virtual void run();

private slots:
    void syncAfterStart();
    void on_timerOut();

private:
    bool m_bBackground;
    CWizDatabase& m_db;
    WIZUSERINFO m_info;
    CWizKMSyncEvents* m_pEvents;
    bool m_bNeedSyncAll;
    bool m_bNeedDownloadMessages;
    QDateTime m_tLastSyncAll;
    int m_nFullSyncSecondsInterval;
    bool m_bBusy;
    bool m_bPause;

    //
    QMutex m_mutex;
    QWaitCondition m_wait;
    QTimer m_timer;
    std::set<QString> m_setQuickSyncKb;
    QDateTime m_tLastKbModified;

    bool doSync();

    bool prepareToken();
    bool needSyncAll();
    bool needQuickSync();
    bool needDownloadMessage();
    bool syncAll();
    bool quickSync();
    bool downloadMesages();

    void syncUserCert();
    //
    bool peekQuickSyncKb(QString& kbGuid);
    //
    friend class CWizKMSyncThreadHelper;

Q_SIGNALS:
    void syncStarted(bool syncAll);
    void syncFinished(int nErrorCode, const QString& strErrorMesssage, bool isBackground);
    void processLog(const QString& strStatus);
    void promptMessageRequest(int nType, const QString& strTitle, const QString& strMsg);
    void bubbleNotificationRequest(const QVariant& param);
};

class CWizKMWaitAndPauseSyncHelper
{
public:
    CWizKMWaitAndPauseSyncHelper()
    {
        CWizKMSyncThread::waitUntilIdleAndPause();
    }
    ~CWizKMWaitAndPauseSyncHelper()
    {
        CWizKMSyncThread::setPause(false);
    }
};

#define WIZKM_WAIT_AND_PAUSE_SYNC() \
    CWizKMWaitAndPauseSyncHelper __waitHelper;\
    Q_UNUSED(__waitHelper)

#define WIZKM_CHECK_SYNCING(parent) \
    if (CWizKMSyncThread::isBusy()) \
    {   \
        QString title = QObject::tr("Syncing"); \
        QString message = QObject::tr("WizNote is synchronizing notes, please wait for the synchronization to complete before the operation.");  \
        QMessageBox::information(parent, title, message);\
        return;    \
    }


#endif // WIZKMSYNC_H
