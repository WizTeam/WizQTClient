#ifndef WIZSYNCTHREAD_H
#define WIZSYNCTHREAD_H

#include <QtCore>

#include "wizsync.h"

class CWizSyncThread : public QThread
{
    Q_OBJECT

public:
    explicit CWizSyncThread(CWizDatabase& db, const CString& strAccountsApiURL, QObject *parent = 0);

    void startSync();
    void setDownloadAllNotesData(bool b) { m_bDownloadAllNotesData = b; }
    void resetProxy() { m_bNeedResetProxy = true; }
    void abort() { if (m_bIsStarted) m_sync->abort(); }

protected:
    virtual void run();

private:
    CWizSync* m_sync;

    CWizDatabase& m_db;
    QString m_strAccountsApiURL;

    bool m_bDownloadAllNotesData;
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
    void on_syncDone(bool error);
    
};

#endif // WIZSYNCTHREAD_H
