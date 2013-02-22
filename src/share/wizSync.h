#ifndef WIZSYNC_H
#define WIZSYNC_H

#include <QPointer>

#include "wizDatabaseManager.h"
#include "wizKbSync.h"


class CWizSync : public QObject
{
    Q_OBJECT

public:
    CWizSync(CWizDatabaseManager& db, const QString& strKbUrl = WIZ_API_URL);
    ~CWizSync();

    void startSync();

    void abort() const { m_kbSync->abort(); }
    void resetProxy() const { m_kbSync->resetProxy(); }
    void setDownloadAllNotesData(bool b) const { m_kbSync->setDownloadAllNotesData(b); }

private:
    CWizDatabaseManager& m_dbMgr;
    QPointer<CWizKbSync> m_kbSync;
    CWizGroupDataArray m_arrayGroup;
    bool m_bStarted;

public Q_SLOTS:
    void on_clientLoginDone();
    void on_getGroupListDone(const CWizGroupDataArray& arrayGroup);
    void on_kbSyncDone(bool bError);

Q_SIGNALS:
    void syncStarted();
    void syncLogined();
    void syncDone(bool bError);

    void processLog(const QString& str);
    void processDebugLog(const QString& str);
    void processErrorLog(const QString& str);
};

#endif // WIZSYNC_H
