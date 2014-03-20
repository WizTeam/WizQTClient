#include "wizObjectDataDownloader.h"

#include <QDebug>

#include "wizDatabaseManager.h"
#include "wizDatabase.h"
#include "sync/wizkmxmlrpc.h"

#include "../sync/token.h"
#include "../sync/apientry.h"

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
    CWizDownloadObjectRunnable* downloader = new CWizDownloadObjectRunnable(m_dbMgr, data);
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
}

void CWizObjectDataDownloaderHost::on_downloadProgress(QString objectGUID, int totalSize, int loadedSize)
{
    Q_EMIT downloadProgress(totalSize, loadedSize);
}

CWizDownloadObjectRunnable::CWizDownloadObjectRunnable(CWizDatabaseManager& dbMgr, const WIZOBJECTDATA& data)
    : m_dbMgr(dbMgr)
    , m_data(data)
{
}

void CWizDownloadObjectRunnable::run()
{
    bool ret = download();
    //
    Q_EMIT downloadDone(m_data.strObjectGUID, ret);
}
bool CWizDownloadObjectRunnable::download()
{
    QString token = WizService::Token::token();
    if (token.isEmpty()) {
        return false;
    }

    WIZUSERINFOBASE info;
    info.strToken = token;
    info.strKbGUID = m_data.strKbGUID;
    info.strDatabaseServer = WizService::ApiEntry::kUrlFromGuid(token, m_data.strKbGUID);

    CWizKMDatabaseServer ksServer(info);
    connect(&ksServer, SIGNAL(downloadProgress(int, int)), SLOT(on_downloadProgress(int,int)));

    // FIXME: should we query object before download data?
    if (!ksServer.data_download(m_data.strObjectGUID,
                                WIZOBJECTDATA::ObjectTypeToTypeString(m_data.eObjectType),
                                m_data.arrayData, m_data.strDisplayName)) {
        return false;
    }

    m_dbMgr.db(m_data.strKbGUID).UpdateObjectData(m_data.strObjectGUID,
                                                  WIZOBJECTDATA::ObjectTypeToTypeString(m_data.eObjectType),
                                                  m_data.arrayData);

    return true;
}
void CWizDownloadObjectRunnable::on_downloadProgress(int totalSize, int loadedSize)
{
    emit downloadProgress(m_data.strObjectGUID, totalSize, loadedSize);
}
