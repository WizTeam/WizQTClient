#ifndef WIZOBJECTDATADOWNLOADER_H
#define WIZOBJECTDATADOWNLOADER_H

#include <QThread>
#include <QMap>
#include <QRunnable>
#include <QMutex>
#include <memory>
#include <functional>

#include "wizobject.h"

class CWizApi;
class CWizDatabase;
class CWizDatabaseManager;
struct IWizThreadPool;

enum DownloadType
{
    TypeNomalData,
    TypeDocument
};

/* ---------------------- CWizObjectDataDownloaderHost ---------------------- */
// host running in main thread and manage downloader

class CWizObjectDownloaderHost : public QObject
{
    Q_OBJECT

public:
    static CWizObjectDownloaderHost* instance();

    void downloadData(const WIZOBJECTDATA& data);
    void downloadData(const WIZOBJECTDATA& data, std::function<void(void)> callback);
    void downloadDocument(const WIZOBJECTDATA& data);
    void downloadDocument(const WIZOBJECTDATA& data, std::function<void(void)> callback);

    void waitForDone();

    CWizObjectDownloaderHost(QObject* parent = 0);
    ~CWizObjectDownloaderHost();
Q_SIGNALS:
    void downloadDone(const WIZOBJECTDATA& data, bool bSucceed);
    void finished();
    void downloadProgress(QString objectGUID, int totalSize, int loadedSize);

private Q_SLOTS:
    void on_downloadDone(QString data, bool bSucceed);
    void on_downloadProgress(QString data, int totalSize, int loadedSize);

private:
    void download(const WIZOBJECTDATA& data, DownloadType type, std::function<void(void)> callback = nullptr);

private:
    QMap<QString, WIZOBJECTDATA> m_mapObject;   // download pool
    static std::shared_ptr<CWizObjectDownloaderHost> m_instance;
    IWizThreadPool* m_threadPool;
    QMutex m_mutex;
};


class CWizObjectDownloader : public QObject
{
    Q_OBJECT
public:
    CWizObjectDownloader(const WIZOBJECTDATA& data, DownloadType type);
    virtual void run();

private:
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

class CWizFileDownloader : public QObject
{
    Q_OBJECT
public:
    CWizFileDownloader(const QString& strUrl, const QString& strFileName,
                       const QString& strPath, bool isImage);
    void startDownload();

signals:
    void downloadDone(QString strFileName, bool bSucceed);

private:
    QString m_strUrl;
    QString m_strFileName;
    bool m_isImage;

    virtual void run();
    bool download();
};

#endif // WIZOBJECTDATADOWNLOADER_H
