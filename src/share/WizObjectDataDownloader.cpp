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

    // FIXME: should we query object before download data?
    if (!ksServer.data_download(m_data.strObjectGUID,
                                WIZOBJECTDATA::objectTypeToTypeString(m_data.eObjectType),
                                m_data.arrayData, m_data.strDisplayName)) {
        return false;
    }

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

    if (!ksServer.document_downloadData(m_data.strObjectGUID, document))
    {
        return false;
    }

    // check update of attachment
    WIZOBJECTVERSION versionServer;
    ksServer.wiz_getVersion(versionServer);
    __int64 nLocalVersion = db.getObjectVersion("attachment");
    if (document.nAttachmentCount > 0 && nLocalVersion < versionServer.nAttachmentVersion)
    {
        //todo: wsj, 奇怪的逻辑，需要修复
        /*
        std::deque<WIZDOCUMENTATTACHMENTDATAEX> arrayRet;
        ksServer.attachment_getList(50, nLocalVersion, arrayRet);
        //
        for(auto attach : arrayRet)
        {
            if (attach.strDocumentGUID != document.strGUID)
                continue;

            nPart = WIZKM_XMKRPC_ATTACHMENT_PART_INFO;
            WIZDOCUMENTATTACHMENTDATAEX attachRet = attach;
            attachRet.strKbGUID = db.kbGUID();
            if (ksServer.attachment_downloadData(attach.strGUID, attachRet))
            {
//                qDebug() << "get attachment from server : " << attachRet.strName;
                //
                db.blockSignals(true);
                db.UpdateAttachment(attachRet);
                db.blockSignals(false);
            }
        }
        */
    }
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

bool WizObjectDownloader::getUserInfo(WIZUSERINFOBASE& info)
{
    QString token = WizToken::token();
    if (token.isEmpty()) {
        return false;
    }

    info.strToken = token;
    info.strKbGUID = m_data.strKbGUID;
    info.strDatabaseServer = WizCommonApiEntry::kUrlFromGuid(token, m_data.strKbGUID);

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

