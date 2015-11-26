#include "wizObjectDataDownloader.h"

#include <QDebug>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include "utils/pathresolve.h"

#include "wizDatabaseManager.h"
#include "wizDatabase.h"
#include "sync/wizKMServer.h"

#include "../sync/token.h"
#include "../sync/apientry.h"
#include "wizmisc.h"

// to avoid to much load for remote serser
#define WIZ_OBJECTDATA_DOWNLOADER_MAX 1


/* --------------------- CWizObjectDataDownloaderHost --------------------- */
CWizObjectDataDownloaderHost::CWizObjectDataDownloaderHost(CWizDatabaseManager& dbMgr,
                                                           QObject* parent /* = 0 */)
    : QObject(parent)
    , m_dbMgr(dbMgr)
{
}

void CWizObjectDataDownloaderHost::downloadData(const WIZOBJECTDATA& data)
{
    download(data, TypeNomalData);
}

void CWizObjectDataDownloaderHost::downloadDocument(const WIZOBJECTDATA& data)
{
    download(data, TypeDocument);
}

void CWizObjectDataDownloaderHost::download(const WIZOBJECTDATA& data, DownloadType type)
{
    Q_ASSERT(!data.strObjectGUID.isEmpty());
    //
    if (m_mapObject.contains(data.strObjectGUID))
    {
        qDebug() << "\n[downloader host] object already in the pool: "
                 << data.strDisplayName;

        return;
    }
    //
    m_mapObject[data.strObjectGUID] = data;
    //
    CWizDownloadObjectRunnable* downloader = new CWizDownloadObjectRunnable(m_dbMgr, data, type);
    //
    connect(downloader, SIGNAL(downloadDone(QString,bool)), this, SLOT(on_downloadDone(QString,bool)));
    connect(downloader, SIGNAL(downloadProgress(QString,int,int)), this, SLOT(on_downloadProgress(QString,int,int)));

    QThreadPool::globalInstance()->start(downloader);
}
void CWizObjectDataDownloaderHost::on_downloadDone(QString objectGUID, bool bSucceed)
{
    WIZOBJECTDATA data = m_mapObject[objectGUID];
    //
    m_mapObject.remove(objectGUID);
    //
    Q_EMIT downloadDone(data, bSucceed);
    Q_EMIT finished();
}

void CWizObjectDataDownloaderHost::on_downloadProgress(QString objectGUID, int totalSize, int loadedSize)
{
    Q_EMIT downloadProgress(objectGUID, totalSize, loadedSize);
}

CWizDownloadObjectRunnable::CWizDownloadObjectRunnable(CWizDatabaseManager& dbMgr, const WIZOBJECTDATA& data,
                                                       DownloadType type)
    : m_dbMgr(dbMgr)
    , m_data(data)
    , m_type(type)
{
}

void CWizDownloadObjectRunnable::run()
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

bool CWizDownloadObjectRunnable::downloadNormalData()
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



    return m_dbMgr.db(m_data.strKbGUID).UpdateObjectData(m_data.strObjectGUID,
                                                  WIZOBJECTDATA::ObjectTypeToTypeString(m_data.eObjectType),
                                                  m_data.arrayData);    
}

bool CWizDownloadObjectRunnable::downloadDocument()
{
    WIZUSERINFO info;
    if (!getUserInfo(info))
        return false;

    CWizKMDatabaseServer ksServer(info);
    connect(&ksServer, SIGNAL(downloadProgress(int, int)), SLOT(on_downloadProgress(int,int)));

    CWizDatabase& db = m_dbMgr.db(m_data.strKbGUID);
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

bool CWizDownloadObjectRunnable::getUserInfo(WIZUSERINFOBASE& info)
{
    QString token = WizService::Token::token();
    if (token.isEmpty()) {
        return false;
    }

    info.strToken = token;
    info.strKbGUID = m_data.strKbGUID;
    info.strDatabaseServer = WizService::CommonApiEntry::kUrlFromGuid(token, m_data.strKbGUID);

    return true;
}
void CWizDownloadObjectRunnable::on_downloadProgress(int totalSize, int loadedSize)
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
    QThreadPool::globalInstance()->start(this);
}

bool CWizFileDownloader::download()
{
     return WizURLDownloadToFile(m_strUrl, m_strFileName, m_isImage);
}
