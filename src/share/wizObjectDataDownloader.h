#ifndef WIZOBJECTDATADOWNLOADER_H
#define WIZOBJECTDATADOWNLOADER_H

#include <QThread>
#include <QMap>
#include <QRunnable>

#include "wizobject.h"

class CWizApi;
class CWizDatabase;
class CWizDatabaseManager;

enum DownloadType
{
    TypeNomalData,
    TypeDocument
};

/* ---------------------- CWizObjectDataDownloaderHost ---------------------- */
// host running in main thread and manage downloader

class CWizObjectDataDownloaderHost : public QObject
{
    Q_OBJECT

public:
    CWizObjectDataDownloaderHost(CWizDatabaseManager& dbMgr, QObject* parent = 0);
    void downloadData(const WIZOBJECTDATA& data);
    void downloadDocument(const WIZOBJECTDATA& data);

private:
    void download(const WIZOBJECTDATA& data, DownloadType type);

private:
    CWizDatabaseManager& m_dbMgr;
    QMap<QString, WIZOBJECTDATA> m_mapObject;   // download pool

private Q_SLOTS:
    void on_downloadDone(QString data, bool bSucceed);
    void on_downloadProgress(QString data, int totalSize, int loadedSize);

Q_SIGNALS:
    void downloadDone(const WIZOBJECTDATA& data, bool bSucceed);
    void finished();
    void downloadProgress(QString objectGUID, int totalSize, int loadedSize);
};


class CWizDownloadObjectRunnable
        : public QObject
        , public QRunnable
{
    Q_OBJECT
public:
    CWizDownloadObjectRunnable(CWizDatabaseManager& dbMgr, const WIZOBJECTDATA& data, \
                               DownloadType type);
    virtual void run();

private:
    CWizDatabaseManager& m_dbMgr;
    WIZOBJECTDATA m_data;
    DownloadType m_type;
    //
private:
    bool downloadNormalData();
    bool downloadDocument();
    bool getUserInfo(WIZUSERINFOBASE& info);

private Q_SLOTS:
    void on_downloadProgress(int totalSize, int loadedSize);
Q_SIGNALS:
    void downloadDone(QString objectGuid, bool bSucceed);
    void downloadProgress(QString objectGuid, int totalSize, int loadedSize);
};

class CWizFileDownloader
        : public QObject
        , public QRunnable
{
    Q_OBJECT
public:
    CWizFileDownloader(const QString& strUrl, const QString& strFileName = "", const QString& strPath = "", bool isImage = "false");
    virtual void run();
    void startDownload();

signals:
    void downloadDone(QString strFileName, bool bSucceed);

private:
    QString m_strUrl;
    QString m_strFileName;
    bool m_isImage;

    bool download();
};

#endif // WIZOBJECTDATADOWNLOADER_H
