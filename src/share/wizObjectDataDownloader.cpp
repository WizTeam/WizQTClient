#include "wizObjectDataDownloader.h"

#include <QDebug>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include "utils/pathresolve.h"
#include "share/wizthreads.h"
#include "sync/wizKMServer.h"
#include "sync/token.h"
#include "sync/apientry.h"
#include "wizmisc.h"
#include "wizDatabaseManager.h"
#include "wizDatabase.h"

// to avoid to much load for remote serser
#define WIZ_OBJECTDATA_DOWNLOADER_MAX 1

std::shared_ptr<CWizObjectDownloaderHost> CWizObjectDownloaderHost::m_instance = nullptr;

/* --------------------- CWizObjectDataDownloaderHost --------------------- */
CWizObjectDownloaderHost::CWizObjectDownloaderHost(QObject* parent /* = 0 */)
    : QObject(parent)
    , m_threadPool(nullptr)
{
}

CWizObjectDownloaderHost::~CWizObjectDownloaderHost()
{
}

CWizObjectDownloaderHost* CWizObjectDownloaderHost::instance()
{
    if (nullptr == m_instance.get())
    {
        m_instance = std::make_shared<CWizObjectDownloaderHost>();
    }

    return m_instance.get();
}

void CWizObjectDownloaderHost::downloadData(const WIZOBJECTDATA& data)
{
    download(data, TypeNomalData);
}

void CWizObjectDownloaderHost::downloadData(const WIZOBJECTDATA& data, std::function<void ()> callback)
{
    download(data, TypeNomalData, callback);
}

void CWizObjectDownloaderHost::downloadDocument(const WIZOBJECTDATA& data)
{
    download(data, TypeDocument);
}

void CWizObjectDownloaderHost::downloadDocument(const WIZOBJECTDATA& data, std::function<void ()> callback)
{
    download(data, TypeDocument, callback);
}

void CWizObjectDownloaderHost::waitForDone()
{
    if (m_threadPool)
    {
        m_threadPool->Shutdown(0);
        m_threadPool = nullptr;
    }
}

void CWizObjectDownloaderHost::download(const WIZOBJECTDATA& data, DownloadType type, std::function<void(void)> callback)
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
    CWizObjectDownloader* downloader = new CWizObjectDownloader(data, type);
    //
    connect(downloader, SIGNAL(downloadDone(QString,bool)), this, SLOT(on_downloadDone(QString,bool)));
    connect(downloader, SIGNAL(downloadProgress(QString,int,int)), this, SLOT(on_downloadProgress(QString,int,int)));

    if (!m_threadPool)
    {
        m_threadPool = WizCreateThreadPool(2);
    }

    IWizRunable* action = WizCreateRunable([=](){
        downloader->run();
        downloader->deleteLater();
        if (callback)
        {
            callback();
        }
    });
    m_threadPool->AddTask(action);
}

void CWizObjectDownloaderHost::on_downloadDone(QString objectGUID, bool bSucceed)
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

void CWizObjectDownloaderHost::on_downloadProgress(QString objectGUID, int totalSize, int loadedSize)
{
    Q_EMIT downloadProgress(objectGUID, totalSize, loadedSize);
}

CWizObjectDownloader::CWizObjectDownloader(const WIZOBJECTDATA& data, DownloadType type)
    : m_data(data)
    , m_type(type)
{
}

void CWizObjectDownloader::run()
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

bool CWizObjectDownloader::downloadNormalData()
{
    WIZUSERINFO info;
    if (!getUserInfo(info))
        return false;

    CWizKMDatabaseServer ksServer(info);
    connect(&ksServer, SIGNAL(downloadProgress(int, int)), SLOT(on_downloadProgress(int,int)));

    // FIXME: should we query object before download data?
    if (!ksServer.data_download(m_data.strObjectGUID,
                                WIZOBJECTDATA::ObjectTypeToTypeString(m_data.eObjectType),
                                m_data.arrayData, m_data.strDisplayName)) {
        return false;
    }

    if (CWizDatabaseManager* dbMgr = CWizDatabaseManager::instance())
    {
        return dbMgr->db(m_data.strKbGUID).UpdateObjectData(m_data.strObjectGUID,
                                                             WIZOBJECTDATA::ObjectTypeToTypeString(m_data.eObjectType),
                                                             m_data.arrayData);
    }

    return false;
}

bool CWizObjectDownloader::downloadDocument()
{
    WIZUSERINFO info;
    if (!getUserInfo(info))
        return false;

    CWizKMDatabaseServer ksServer(info);
    connect(&ksServer, SIGNAL(downloadProgress(int, int)), SLOT(on_downloadProgress(int,int)));

    CWizDatabaseManager* dbMgr = CWizDatabaseManager::instance();
    if (!dbMgr)
        return false;

    CWizDatabase& db = dbMgr->db(m_data.strKbGUID);
    WIZDOCUMENTDATAEX document;
    if (!db.DocumentFromGUID(m_data.strObjectGUID, document))
        return false;

    int nPart = WIZKM_XMKRPC_DOCUMENT_PART_INFO | WIZKM_XMKRPC_DOCUMENT_PART_DATA;
    if (!ksServer.document_getData(m_data.strObjectGUID, nPart, document))
    {
        return false;
    }

    // check update of attachment
    WIZOBJECTVERSION versionServer;
    ksServer.wiz_getVersion(versionServer);
    __int64 nLocalVersion = db.GetObjectVersion("attachment");
    if (document.nAttachmentCount > 0 && nLocalVersion < versionServer.nAttachmentVersion)
    {
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
            if (ksServer.attachment_getData(attach.strGUID, nPart, attachRet))
            {
//                qDebug() << "get attachment from server : " << attachRet.strName;
                //
                db.blockSignals(true);
                db.UpdateAttachment(attachRet);
                db.blockSignals(false);
            }
        }
    }

    bool ret = false;
    db.blockSignals(true);
    if (db.UpdateObjectData(document.strGUID, WIZOBJECTDATA::ObjectTypeToTypeString(wizobjectDocument),
                             document.arrayData))
    {
        ret = db.UpdateDocument(document);
        db.SetObjectDataDownloaded(document.strGUID, WIZOBJECTDATA::ObjectTypeToTypeString(wizobjectDocument), true);
    }
    db.blockSignals(false);
    return ret;
}

bool CWizObjectDownloader::getUserInfo(WIZUSERINFOBASE& info)
{
    QString token = Token::token();
    if (token.isEmpty()) {
        return false;
    }

    info.strToken = token;
    info.strKbGUID = m_data.strKbGUID;
    info.strDatabaseServer = CommonApiEntry::kUrlFromGuid(token, m_data.strKbGUID);

    return true;
}
void CWizObjectDownloader::on_downloadProgress(int totalSize, int loadedSize)
{
    emit downloadProgress(m_data.strObjectGUID, totalSize, loadedSize);
}


CWizFileDownloader::CWizFileDownloader(const QString& strUrl, const QString& strFileName, const QString& strPath, bool isImage)
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
        m_strFileName = Utils::PathResolve::tempPath() + m_strFileName;
    }
    else
    {
        m_strFileName = strPath + m_strFileName;
    }
}

void CWizFileDownloader::run()
{
    bool ret = download();
    emit downloadDone(m_strFileName, ret);
}

void CWizFileDownloader::startDownload()
{
    WizExecuteOnThread(WIZ_THREAD_DOWNLOAD, [=](){
        run();
        deleteLater();
    });
}

bool CWizFileDownloader::download()
{
     return WizURLDownloadToFile(m_strUrl, m_strFileName, m_isImage);
}

