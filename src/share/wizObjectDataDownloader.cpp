#include "wizObjectDataDownloader.h"

#include <QDebug>

#include "wizDatabaseManager.h"

// to avoid to much load for remote serser
#define WIZ_OBJECTDATA_DOWNLOADER_MAX 1


/* --------------------- CWizObjectDataDownloaderHost --------------------- */
CWizObjectDataDownloaderHost::CWizObjectDataDownloaderHost(CWizDatabaseManager& dbMgr,
                                                           QObject* parent /* = 0 */)
    : QObject(parent)
    , m_dbMgr(dbMgr)
{
}

void CWizObjectDataDownloaderHost::download(const WIZOBJECTDATA& data)
{
    Q_ASSERT(!data.strObjectGUID.isEmpty());

    if (!m_dbMgr.isOpened(data.strKbGUID)) {
        qDebug() << "\n[downloader host] object from unknown kb, "
                 << "maybe you have been kicked out, or your kb info is out of date, please sync and try again"
                 << "object: " << data.strDisplayName;

        return;
    }

    if (m_mapObject.contains(data.strObjectGUID)
            || m_mapDownloading.contains(data.strObjectGUID)) {
        qDebug() << "\n[downloader host] object already in the pool: "
                 << data.strDisplayName;

        return;
    }

    m_mapObject[data.strObjectGUID] = data;

    qDebug() << "\n[downloader host] scheduled download object: "
             << data.strDisplayName << " total: "
             << m_mapObject.size() + m_mapDownloading.size()
             << " thread: " << QThread::currentThreadId();

    downloadObject();
}

void CWizObjectDataDownloaderHost::downloadObject()
{
    if (m_mapDownloading.size() >= WIZ_OBJECTDATA_DOWNLOADER_MAX) {
        qDebug() << "\n[downloader host] maxinum downloader arrived, wait...";
        return;
    }

    if (!m_mapObject.size()) {
        qDebug() << "\n[downloader host] download pool is clean...";
        return;
    }

    QMap<QString, WIZOBJECTDATA>::iterator it = m_mapObject.begin();

    CWizObjectDataDownloader* downloader = new CWizObjectDataDownloader(m_dbMgr, it.value());
    connect(downloader, SIGNAL(terminated()), SLOT(on_downloader_finished()));
    downloader->start();

    m_mapDownloading[it.key()] = it.value();
    m_mapObject.erase(it);
}

void CWizObjectDataDownloaderHost::on_downloader_finished()
{
    CWizObjectDataDownloader* downloader = qobject_cast<CWizObjectDataDownloader*>(sender());
    WIZOBJECTDATA data = downloader->data();
    bool bSucceed = downloader->succeed();

    m_mapDownloading.remove(data.strObjectGUID);

    qDebug() << "[downloader host] download object finished"
             << data.strDisplayName << " left: "
             << m_mapObject.size() + m_mapDownloading.size();

    Q_EMIT downloadDone(data, bSucceed);

    downloadObject();

    downloader->deleteLater();
}


/* ------------------------ CWizObjectDataDownloader ------------------------ */
CWizObjectDataDownloader::CWizObjectDataDownloader(CWizDatabaseManager& dbMgr,
                                                   const WIZOBJECTDATA& data)
    : m_dbMgr(dbMgr)
    , m_data(data)
    , m_bSucceed(false)
{
}

void CWizObjectDataDownloader::run()
{
    qDebug() << "[downloader host] start download object: "
             << m_data.strDisplayName << " thread: " << QThread::currentThreadId();

    CWizObjectDataDownload* download = new CWizObjectDataDownload(m_dbMgr, m_data);
    connect(download, SIGNAL(downloaded(bool)), SLOT(on_downloaded(bool)));
    download->startDownload();

    exec();
}

void CWizObjectDataDownloader::on_downloaded(bool bSucceed)
{
    m_bSucceed = bSucceed;

    // use terminate instead of quit
    terminate();
}


/* ------------------------- CWizObjectDataDownload ------------------------- */
CWizObjectDataDownload::CWizObjectDataDownload(CWizDatabaseManager& dbMgr,
                                               const WIZOBJECTDATA& data)
    : CWizApi(dbMgr.db(data.strKbGUID))
    , m_dbMgr(dbMgr)
    , m_data(data)
{
}

void CWizObjectDataDownload::startDownload()
{
    QString strUserId = m_dbMgr.db().getUserId();
    QString strPasswd = m_dbMgr.db().getPassword();

    callClientLogin(strUserId, strPasswd);
}

void CWizObjectDataDownload::onClientLogin(const WIZUSERINFO& userInfo)
{
    QString kbUrl;

    // personal document
    if (m_data.strKbGUID == userInfo.strKbGUID) {
        kbUrl = userInfo.strDatabaseServer;
    } else {
        // if synced before, server field should exist.
        kbUrl = m_dbMgr.db(m_data.strKbGUID).server();

        if (kbUrl.isEmpty()) {
            callGetGroupList();
            return;
        }
    }

    setKbUrl(kbUrl);
    setKbGUID(m_data.strKbGUID);

    if (!isLocalObject()) {
        startDownloadObjectDataInfo();
        return;
    }

    startDownloadObjectData();
}

void CWizObjectDataDownload::onGetGroupList(const CWizGroupDataArray& arrayGroup)
{
    CWizGroupDataArray::const_iterator it;
    for (it = arrayGroup.begin(); it != arrayGroup.end(); it++) {
        const WIZGROUPDATA& group = *it;

        if (group.strGroupGUID == m_data.strKbGUID) {

            setKbUrl(group.strDatabaseServer);
            setKbGUID(m_data.strKbGUID);

            if (!isLocalObject()) {
                startDownloadObjectDataInfo();
                return;
            }

            startDownloadObjectData();
            return;
        }
    }
}

void CWizObjectDataDownload::startDownloadObjectDataInfo()
{
    qDebug() << "[downloader] download object info: " << m_data.strDisplayName;

    if (m_data.eObjectType == wizobjectDocument) {
        callDocumentGetData(m_data.strObjectGUID);
    } else if (m_data.eObjectType == wizobjectDocumentAttachment) {
        callAttachmentGetInfo(m_data.strObjectGUID);
    } else {
        Q_ASSERT(0);
    }
}

void CWizObjectDataDownload::onDocumentGetData(const WIZDOCUMENTDATAEX& data)
{
    if (!m_dbMgr.db(m_data.strKbGUID).UpdateDocument(data)) {
        qDebug() << "[downloader] failed: unable to update document info: "
                 << m_data.strDisplayName;
        Q_EMIT downloaded(false);
        return;
    }

    startDownloadObjectData();
}

void CWizObjectDataDownload::onAttachmentsGetInfo(const CWizDocumentAttachmentDataArray& arrayRet)
{
    Q_ASSERT(arrayRet.size() == 1);

    const WIZDOCUMENTATTACHMENTDATAEX& data = arrayRet[0];

    if (!m_dbMgr.db(m_data.strKbGUID).UpdateAttachment(data)) {
        qDebug() << "[downloader] failed: unable to update attachment info "
                 << m_data.strDisplayName;
        Q_EMIT downloaded(false);
        return;
    }

    startDownloadObjectData();
}

void CWizObjectDataDownload::startDownloadObjectData()
{
    qDebug() << "[downloader] start downloading: " << m_data.strDisplayName;

    downloadObjectData(m_data);
}

void CWizObjectDataDownload::onDownloadObjectDataCompleted(const WIZOBJECTDATA& data)
{
    qDebug() << "[downloader] download finished: " << data.strDisplayName;

    if (!data.strObjectGUID.isEmpty())
        m_dbMgr.db(m_data.strKbGUID).UpdateSyncObjectLocalData(data);

    Q_EMIT downloaded(true);
}

void CWizObjectDataDownload::onXmlRpcError(const QString& strMethodName,
                                           WizXmlRpcError err,
                                           int errorCode,
                                           const QString& errorMessage)
{
    CWizApi::onXmlRpcError(strMethodName, err, errorCode, errorMessage);
    Q_EMIT downloaded(false);
}

bool CWizObjectDataDownload::isLocalObject()
{
    if (m_data.eObjectType == wizobjectDocument) {
        WIZDOCUMENTDATA doc;
        if (m_dbMgr.db(m_data.strKbGUID).DocumentFromGUID(m_data.strObjectGUID, doc)) {
            return true;
        }
    } else if (m_data.eObjectType == wizobjectDocumentAttachment) {
        WIZDOCUMENTATTACHMENTDATA attach;
        if (m_dbMgr.db(m_data.strKbGUID).AttachmentFromGUID(m_data.strObjectGUID, attach)) {
            return true;
        }
    } else {
        Q_ASSERT(0);
    }

    return false;
}
