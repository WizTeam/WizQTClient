#include "WizObjectDataDownloader.h"

#include <QDebug>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include "utils/WizPathResolve.h"
#include "share/WizThreads.h"
#include "sync/WizKMServer.h"
#include "sync/WizToken.h"
#include "sync/WizApiEntry.h"
#include "WizMisc.h"
#include "WizDatabaseManager.h"
#include "WizDatabase.h"

// to avoid to much load for remote serser
#define WIZ_OBJECTDATA_DOWNLOADER_MAX 1

std::shared_ptr<WizObjectDownloaderHost> WizObjectDownloaderHost::m_instance = nullptr;

/* --------------------- CWizObjectDataDownloaderHost --------------------- */
WizObjectDownloaderHost::WizObjectDownloaderHost(QObject* parent /* = 0 */)
    : QObject(parent)
    , m_threadPool(nullptr)
{
}

WizObjectDownloaderHost::~WizObjectDownloaderHost()
{
}

WizObjectDownloaderHost* WizObjectDownloaderHost::instance()
{
    if (nullptr == m_instance.get())
    {
        m_instance = std::make_shared<WizObjectDownloaderHost>();
    }

    return m_instance.get();
}

void WizObjectDownloaderHost::downloadData(const WIZOBJECTDATA& data)
{
    download(data, TypeNomalData);
}

void WizObjectDownloaderHost::downloadData(const WIZOBJECTDATA& data, std::function<void ()> callback)
{
    download(data, TypeNomalData, callback);
}

void WizObjectDownloaderHost::downloadDocument(const WIZOBJECTDATA& data)
{
    download(data, TypeDocument);
}

void WizObjectDownloaderHost::downloadDocument(const WIZOBJECTDATA& data, std::function<void ()> callback)
{
    download(data, TypeDocument, callback);
}

void WizObjectDownloaderHost::waitForDone()
{
    if (m_threadPool)
    {
        m_threadPool->shutdown(0);
        m_threadPool = nullptr;
    }
}

void WizObjectDownloaderHost::download(const WIZOBJECTDATA& data, DownloadType type, std::function<void(void)> callback)
{
    Q_ASSERT(!data.strObjectGUID.isEmpty());
    //
    {
        QMutexLocker locker(&m_mutex);
        if (m_mapObject.contains(data.strObjectGUID))
        {
            qDebug() << "\n[downloader host] object already in the pool: "
                     << data.strDisplayName;
            return;
        }
        //
        m_mapObject[data.strObjectGUID] = data;
    }
    //
    WizObjectDownloader* downloader = new WizObjectDownloader(data, type);
    //
    connect(downloader, SIGNAL(downloadDone(QString,bool)), this, SLOT(on_downloadDone(QString,bool)));
    connect(downloader, SIGNAL(downloadProgress(QString,int,int)), this, SLOT(on_downloadProgress(QString,int,int)));

    if (!m_threadPool)
    {
        m_threadPool = WizCreateThreadPool(5);
    }

    IWizRunable* action = WizCreateRunable([=](){
        downloader->run();
        downloader->deleteLater();
        if (callback)
        {
            callback();
        }
    });
    m_threadPool->addTask(action);
}

void WizObjectDownloaderHost::on_downloadDone(QString objectGUID, bool bSucceed)
{
    m_mutex.lock();
    WIZOBJECTDATA data = m_mapObject[objectGUID];
    //
    m_mapObject.remove(objectGUID);
    m_mutex.unlock();
    //
    Q_EMIT downloadDone(data, bSucceed);
    Q_EMIT finished();
}

void WizObjectDownloaderHost::on_downloadProgress(QString objectGUID, int totalSize, int loadedSize)
{
    Q_EMIT downloadProgress(objectGUID, totalSize, loadedSize);
}

WizObjectDownloader::WizObjectDownloader(const WIZOBJECTDATA& data, DownloadType type)
    : m_data(data)
    , m_type(type)
{
}

void WizObjectDownloader::run()
{
    bool ret = false;
    switch (m_type) {
    case TypeNomalData:
        ret = downloadNormalData();
        break;
    case TypeDocument:
        ret = downloadDocument();
        break;
    default:
        break;
    }
    //
    Q_EMIT downloadDone(m_data.strObjectGUID, ret);
}

bool WizObjectDownloader::downloadNormalData()
{
    WIZUSERINFO info;
    if (!getUserInfo(info))
        return false;

    WizKMDatabaseServer ksServer(info);
    connect(&ksServer, SIGNAL(downloadProgress(int, int)), SLOT(on_downloadProgress(int,int)));

    if (m_data.eObjectType == wizobjectDocument)
    {
        WIZDOCUMENTDATAEX ret;
        ret.strGUID = m_data.strObjectGUID;
        ret.strKbGUID = m_data.strKbGUID;
        ret.strTitle = m_data.strDisplayName;
        QString fileName = ::WizDatabaseManager::instance()->db(ret.strKbGUID).getDocumentFileName(ret.strGUID);
        if (!ksServer.document_downloadData(m_data.strObjectGUID, ret, fileName))
        {
            qDebug() << WizFormatString1("Cannot download note data from server: %1", m_data.strDisplayName);
            return false;
        }
        m_data.arrayData = ret.arrayData;
    }
    else
    {
        WIZDOCUMENTATTACHMENTDATAEX ret;
        ret.strGUID = m_data.strObjectGUID;
        ret.strKbGUID = m_data.strKbGUID;
        ret.strName = m_data.strDisplayName;
        if (!ksServer.attachment_downloadData(m_data.strDocumentGuid, m_data.strObjectGUID, ret))
        {
            qDebug() << WizFormatString1("Cannot download attachment data from server: %1", m_data.strDisplayName);
            return false;
        }
        m_data.arrayData = ret.arrayData;
    }
    //
    if (WizDatabaseManager* dbMgr = WizDatabaseManager::instance())
    {
        return dbMgr->db(m_data.strKbGUID).updateObjectData(m_data.strDisplayName, m_data.strObjectGUID,
                                                             WIZOBJECTDATA::objectTypeToTypeString(m_data.eObjectType),
                                                             m_data.arrayData);
    }

    return false;
}

bool WizObjectDownloader::downloadDocument()
{
    WIZUSERINFO info;
    if (!getUserInfo(info))
        return false;

    WizKMDatabaseServer ksServer(info);
    connect(&ksServer, SIGNAL(downloadProgress(int, int)), SLOT(on_downloadProgress(int,int)));

    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    if (!dbMgr)
        return false;

    WizDatabase& db = dbMgr->db(m_data.strKbGUID);
    WIZDOCUMENTDATAEX document;
    if (!db.documentFromGuid(m_data.strObjectGUID, document))
        return false;
    //
    if (!ksServer.document_getInfo(m_data.strObjectGUID, document))
        return false;
    //
    QString fileName = db.getDocumentFileName(m_data.strObjectGUID);

    if (!ksServer.document_downloadData(m_data.strObjectGUID, document, fileName))
    {
        return false;
    }
    //
    document.strKbGUID = m_data.strKbGUID;

    //
    bool ret = false;
    db.blockSignals(true);
    if (db.updateObjectData(document.strTitle, document.strGUID, WIZOBJECTDATA::objectTypeToTypeString(wizobjectDocument),
                             document.arrayData))
    {
        ret = db.updateDocument(document);
        db.setObjectDataDownloaded(document.strGUID, WIZOBJECTDATA::objectTypeToTypeString(wizobjectDocument), true);
    }
    db.blockSignals(false);
    return ret;
}

bool WizObjectDownloader::getUserInfo(WIZUSERINFO& info)
{
    QString token = WizToken::token();
    if (token.isEmpty()) {
        return false;
    }

    info = WizToken::userInfo();
    info.strToken = token;
    info.strKbGUID = m_data.strKbGUID;
    //
    WIZGROUPDATA group;
    if (WizDatabaseManager::instance()->db().getGroupData(m_data.strKbGUID, group))
    {
        info = WIZUSERINFO(info, group);
    }

    return true;
}
void WizObjectDownloader::on_downloadProgress(int totalSize, int loadedSize)
{
    emit downloadProgress(m_data.strObjectGUID, totalSize, loadedSize);
}


WizFileDownloader::WizFileDownloader(const QString& strUrl, const QString& strFileName, const QString& strPath, bool isImage)
    : m_strUrl(strUrl)
    , m_strFileName(strFileName)
    , m_isImage(isImage)
{
    if (m_strFileName.isEmpty())
    {
        m_strFileName = WizGenGUIDLowerCaseLetterOnly();
    }

    if (strPath.isEmpty())
    {
        m_strFileName = Utils::WizPathResolve::tempPath() + m_strFileName;
    }
    else
    {
        m_strFileName = strPath + m_strFileName;
    }
}

void WizFileDownloader::run()
{
    bool ret = download();
    emit downloadDone(m_strFileName, ret);
}

void WizFileDownloader::startDownload()
{
    WizExecuteOnThread(WIZ_THREAD_DOWNLOAD, [=](){
        run();
        deleteLater();
    });
}

bool WizFileDownloader::download()
{
     return WizURLDownloadToFile(m_strUrl, m_strFileName, m_isImage);
}

