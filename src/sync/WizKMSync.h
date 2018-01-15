#ifndef WIZKMSYNC_H
#define WIZKMSYNC_H

#include <QThread>
#include <QMessageBox>
#include <QWaitCondition>
#include <QTimer>
#include <QMutex>

#include "WizSync.h"
#include "WizKMServer.h"


class WizDatabase;

class WizKMSyncEvents : public QObject , public IWizKMSyncEvents
{
    Q_OBJECT

    virtual void onSyncProgress(int pos);
    virtual HRESULT onText(WizKMSyncProgressMessageType type, const QString& strStatus);
    virtual HRESULT onMessage(WizKMSyncProgressMessageType type, const QString& strTitle, const QString& strMessage);
    virtual HRESULT onBubbleNotification(const QVariant& param);
    virtual void setDatabaseCount(int count);
    virtual void setCurrentDatabase(int index);
    virtual void clearLastSyncError(IWizSyncableDatabase* pDatabase);
    virtual void onTrafficLimit(IWizSyncableDatabase* pDatabase);
    virtual void onStorageLimit(IWizSyncableDatabase* pDatabase);
    virtual void onBizServiceExpr(IWizSyncableDatabase* pDatabase);
    virtual void onBizNoteCountLimit(IWizSyncableDatabase* pDatabase);
    virtual void onFreeServiceExpr(WIZGROUPDATA group);
    virtual void onVipServiceExpr(WIZGROUPDATA group);
    virtual void onUploadDocument(const QString& strDocumentGUID, bool bDone);
    virtual void onBeginKb(const QString& strKbGUID);
    virtual void onEndKb(const QString& strKbGUID);

Q_SIGNALS:
    void messageReady(const QString& strStatus);
    void promptMessageRequest(int nType, const QString& strTitle, const QString& strMsg);
    void bubbleNotificationRequest(const QVariant& param);
    void promptFreeServiceExpr(WIZGROUPDATA group);
    void promptVipServiceExpr(WIZGROUPDATA group);
};


class WizKMSyncThread : public QThread
{
    Q_OBJECT

public:
    WizKMSyncThread(WizDatabase& db, bool quickOnly, QObject* parent = 0);
    ~WizKMSyncThread();
    void startSyncAll(bool bBackground = true);
    bool isBackground() const;
    void stopSync();
    void setNeedResetGroups() { m_bNeedResetGroups = true; }
    //
    void setFullSyncInterval(int nMinutes);
    //
    void addQuickSyncKb(const QString& kbGuid);
    //
    bool clearCurrentToken();
    //
    void waitForDone();

public slots:
    void quickDownloadMesages();

public:
    static void setQuickThread(WizKMSyncThread* thread);
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
    WizDatabase& m_db;
    WIZUSERINFO m_info;
    WizKMSyncEvents* m_pEvents;
    bool m_bNeedSyncAll;
    bool m_bNeedDownloadMessages;
    bool m_bNeedResetGroups;
    QDateTime m_tLastSyncAll;
    int m_nFullSyncSecondsInterval;
    bool m_bBusy;
    bool m_bPause;
    bool m_quickOnly;

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
    bool needResetGroups();
    bool needDownloadMessage();
    bool syncAll();
    bool quickSync();
    bool downloadMesages();
    bool resetGroups();

    bool peekQuickSyncKb(QString& kbGuid);
    //
    friend class CWizKMSyncThreadHelper;

Q_SIGNALS:
    void syncStarted(bool syncAll);
    void syncFinished(int nErrorCode, bool isNetworkError, const QString& strErrorMesssage, bool isBackground);
    void processLog(const QString& strStatus);
    void promptMessageRequest(int nType, const QString& strTitle, const QString& strMsg);
    void bubbleNotificationRequest(const QVariant& param);
    void promptFreeServiceExpr(WIZGROUPDATA group);
    void promptVipServiceExpr(WIZGROUPDATA group);
};

class WizKMWaitAndPauseSyncHelper
{
public:
    WizKMWaitAndPauseSyncHelper()
    {
        WizKMSyncThread::waitUntilIdleAndPause();
    }
    ~WizKMWaitAndPauseSyncHelper()
    {
        WizKMSyncThread::setPause(false);
    }
};

#define WIZKM_WAIT_AND_PAUSE_SYNC() \
    WizKMWaitAndPauseSyncHelper __waitHelper;\
    Q_UNUSED(__waitHelper)

#define WIZKM_CHECK_SYNCING(parent) \
    if (WizKMSyncThread::isBusy()) \
    {   \
        QString title = QObject::tr("Syncing"); \
        QString message = QObject::tr("WizNote is synchronizing notes, please wait for the synchronization to complete before the operation.");  \
        QMessageBox::information(parent, title, message);\
        return;    \
    }


#endif // WIZKMSYNC_H
