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
    void abort();

    void resetProxy() const { m_kbSync->resetProxy(); }
    void setDaysDownload(int n) { m_kbSync->setDaysDownload(n); }

private:
    CWizDatabaseManager& m_dbMgr;
    CWizKbSync* m_kbSync;
    CWizGroupDataArray m_arrayGroup;
    bool m_bStarted;
    bool m_bAborted;

    void syncUserFoldersStart();
    void uploadUserFolders();
    void downloadUserFolders();
    void syncUserFoldersEnd();

public Q_SLOTS:
    void on_folderGetVersionDone(qint64 nVersion);
    void on_folderGetListDone(const QStringList& listFolder, qint64 nVersion);
    void on_folderPostListDone(qint64 nVersion);

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
