#ifndef WIZOBJECTDATADOWNLOADER_H
#define WIZOBJECTDATADOWNLOADER_H

#include <QThread>
#include <QMap>
#include <QRunnable>

#include "wizobject.h"

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

private Q_SLOTS:
    void on_downloadDone(QString data, bool bSucceed);
    void on_downloadProgress(QString data, int totalSize, int loadedSize);

Q_SIGNALS:
    void downloadDone(const WIZOBJECTDATA& data, bool bSucceed);
    void downloadProgress(QString objectGUID, int totalSize, int loadedSize);
};


class CWizDownloadObjectRunnable
        : public QObject
        , public QRunnable
{
    Q_OBJECT
public:
    CWizDownloadObjectRunnable(CWizDatabaseManager& dbMgr, const WIZOBJECTDATA& data);
    virtual void run();
private:
    CWizDatabaseManager& m_dbMgr;
    WIZOBJECTDATA m_data;
    //
private:
    bool download();
private Q_SLOTS:
    void on_downloadProgress(int totalSize, int loadedSize);
Q_SIGNALS:
    void downloadDone(QString objectGuid, bool bSucceed);
    void downloadProgress(QString objectGuid, int totalSize, int loadedSize);
};


#endif // WIZOBJECTDATADOWNLOADER_H
