#include "wizObjectDataDownloader.h"

#include <QDebug>

#include "wizDatabaseManager.h"
#include "sync/wizkmxmlrpc.h"

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
    connect(downloader, SIGNAL(finished()), SLOT(on_downloader_finished()));
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

    CWizObjectDataDownloadWorker* download = new CWizObjectDataDownloadWorker(m_dbMgr, m_data);
    //CWizObjectDataDownload* download = new CWizObjectDataDownload(m_dbMgr, m_data);
    connect(download, SIGNAL(downloaded(bool)), SLOT(on_downloaded(bool)));
    download->startDownload();

    exec();
}

void CWizObjectDataDownloader::on_downloaded(bool bSucceed)
{
    qDebug() << "[downloader host] object: "
             << m_data.strDisplayName << " result: " << (bSucceed ? "ok" : "failed");

    m_bSucceed = bSucceed;

    quit();
}


/* ------------------------- CWizObjectDataDownload ------------------------- */
CWizObjectDataDownloadWorker::CWizObjectDataDownloadWorker(CWizDatabaseManager& dbMgr,
                                                           const WIZOBJECTDATA& data)
    : m_dbMgr(dbMgr)
    , m_data(data)
{
}

void CWizObjectDataDownloadWorker::startDownload()
{
    // FIXME
    CWizKMAccountsServer asServer(WizKMGetAccountsServerURL(true));

    QString strUserId = m_dbMgr.db().GetUserId();
    QString strPassword = m_dbMgr.db().GetPassword();

    // FIXME: hard-coded "normal"
    if (!asServer.Login(strUserId, strPassword, "normal")) {
        Q_EMIT downloaded(false);
        return;
    }

    WIZUSERINFOBASE info = asServer.GetUserInfo();
    info.strKbGUID = m_data.strKbGUID;
    bool bOk = false;

    // reset info kb_guid and server url for downloading
    if (m_data.strKbGUID != m_dbMgr.db().kbGUID()) {
        CWizGroupDataArray arrayGroup;
        if (!asServer.GetGroupList(arrayGroup) || arrayGroup.empty()) {
            Q_EMIT downloaded(false);
        }

        CWizGroupDataArray::const_iterator it = arrayGroup.begin();
        for (; it != arrayGroup.end(); it++) {
            const WIZGROUPDATA& group = *it;
            if (group.strGroupGUID == m_data.strKbGUID) {
                info.strDatabaseServer = group.strDatabaseServer;
                bOk = true;
                break;
            }
        }
    } else {
        bOk = true;
    }

    if (!bOk) {
        Q_EMIT downloaded(false);
        return;
    }

    CWizKMDatabaseServer ksServer(info);

    // FIXME: should we query object before download data?
    if (!ksServer.data_download(m_data.strObjectGUID,
                                WIZOBJECTDATA::ObjectTypeToTypeString(m_data.eObjectType),
                                m_data.arrayData, m_data.strDisplayName)) {
        Q_EMIT downloaded(false);
        return;
    }

    m_dbMgr.db(m_data.strKbGUID).UpdateObjectData(m_data.strObjectGUID,
                                                  WIZOBJECTDATA::ObjectTypeToTypeString(m_data.eObjectType),
                                                  m_data.arrayData);

    Q_EMIT downloaded(true);
}
