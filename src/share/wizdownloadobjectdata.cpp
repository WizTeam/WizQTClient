#include "wizdownloadobjectdata.h"

#include "wizapi.h"


CWizDownloadObjectData::CWizDownloadObjectData(CWizDatabase& db)
    : CWizApi(db)
    , m_bInited(false)
{
    connect(this, SIGNAL(processLog(const QString&)), SLOT(processLog(const QString&)));
    connect(this, SIGNAL(processErrorLog(const QString&)), SLOT(processLog(const QString&)));
}

void CWizDownloadObjectData::setData(const WIZOBJECTDATA& data)
{
    Q_ASSERT(!data.strObjectGUID.isEmpty());

    m_data = data;
    m_bInited = true;
}

void CWizDownloadObjectData::startDownload()
{
    Q_ASSERT(m_bInited);

    callClientLogin(m_db.getUserId(), m_db.getPassword());
}

void CWizDownloadObjectData::onXmlRpcError(const QString& strMethodName, WizXmlRpcError err, int errorCode, const QString& errorMessage)
{
    CWizApi::onXmlRpcError(strMethodName, err, errorCode, errorMessage);
    Q_EMIT downloadDone(false);
}

void CWizDownloadObjectData::onClientLogin()
{
    downloadObjectData(m_data);
}

void CWizDownloadObjectData::onDownloadObjectDataCompleted(const WIZOBJECTDATA& data)
{
    CWizApi::onDownloadObjectDataCompleted(data);
    m_data.arrayData = data.arrayData;
    callClientLogout();
}

void CWizDownloadObjectData::onClientLogout()
{
    Q_EMIT downloadDone(true);
}

void CWizDownloadObjectData::processLog(const QString& strMsg)
{
    TOLOG(strMsg);
}
