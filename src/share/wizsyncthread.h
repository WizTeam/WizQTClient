#ifndef WIZSYNCTHREAD_H
#define WIZSYNCTHREAD_H

#include <QtCore>

#include "wizdef.h"
#include "wizSync.h"

class CWizSyncThread : public QThread
{
    Q_OBJECT

public:
    explicit CWizSyncThread(CWizExplorerApp& app, QObject *parent = 0);

    void startSyncing();

    void abort() const { m_sync->abort(); }
    void resetProxy() { m_bNeedResetProxy = true; }

protected:
    virtual void run();

private:
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;

    QPointer<CWizSync> m_sync;
    bool m_bNeedResetProxy;

    bool m_bIsStarted;

Q_SIGNALS:
    void syncStarted();
    void syncLogined();
    void progressChanged(int pos);
    void processLog(const QString& str);
    void processDebugLog(const QString& str);
    void processErrorLog(const QString& str);
    void syncDone(bool error);

public Q_SLOTS:
    void on_syncFinished();

private Q_SLOTS:
    void on_syncDone(bool error);
};

#endif // WIZSYNCTHREAD_H
