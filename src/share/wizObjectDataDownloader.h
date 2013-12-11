#ifndef WIZOBJECTDATADOWNLOADER_H
#define WIZOBJECTDATADOWNLOADER_H

#include <QThread>
#include <QMap>

#include "wizobject.h"
//#include "wizapi.h"

class CWizApi;
class CWizDatabase;
class CWizDatabaseManager;

/* ---------------------- CWizObjectDataDownloaderHost ---------------------- */
// host running in main thread and manage downloader

class CWizObjectDataDownloaderHost : public QObject
{
    Q_OBJECT

public:
    CWizObjectDataDownloaderHost(CWizDatabaseManager& dbMgr, QObject* parent = 0);
    void download(const WIZOBJECTDATA& data);

private:
    CWizDatabaseManager& m_dbMgr;
    QMap<QString, WIZOBJECTDATA> m_mapObject;   // download pool
    QMap<QString, WIZOBJECTDATA> m_mapDownloading;  // working pool
    //int m_nDownloader; // current downloader count

    void downloadObject();

private Q_SLOTS:
    void on_downloader_finished();

Q_SIGNALS:
    void downloadDone(const WIZOBJECTDATA& data, bool bSucceed);
};

/* ------------------------ CWizObjectDataDownloader ------------------------ */
// thread wrapper for download flow

class CWizObjectDataDownloader : public QThread
{
    Q_OBJECT

public:
    explicit CWizObjectDataDownloader(CWizDatabaseManager& dbMgr,
                                      const WIZOBJECTDATA& data);
    virtual void run();

    WIZOBJECTDATA data() const { return m_data; }
    bool succeed() const { return m_bSucceed; }

private:
    CWizDatabaseManager& m_dbMgr;
    WIZOBJECTDATA m_data;
    bool m_bSucceed;

private Q_SLOTS:
    void on_downloaded(bool bSucceed);
};


/* ------------------------- CWizObjectDataDownload ------------------------- */
class CWizObjectDataDownloadWorker :  public QObject
{
    Q_OBJECT

public:
    CWizObjectDataDownloadWorker(CWizDatabaseManager& dbMgr, const WIZOBJECTDATA& data);
    void startDownload();

private:
    CWizDatabaseManager& m_dbMgr;
    WIZOBJECTDATA m_data;   // current downdowing object

public Q_SLOTS:
    void onTokenAcquired(const QString& strToken);

Q_SIGNALS:
    void downloaded(bool succeeded);
};


#endif // WIZOBJECTDATADOWNLOADER_H
