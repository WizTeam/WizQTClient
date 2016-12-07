#include "WizDownloadObjectData.h"

WizDownloadObjectData::WizDownloadObjectData(WizDatabaseManager& dbMgr)
    : CWizApi(dbMgr.db())
    , m_dbMgr(dbMgr)
    , m_bInited(false)
{
    connect(this, SIGNAL(processLog(const QString&)), SLOT(processLog(const QString&)));
    connect(this, SIGNAL(processErrorLog(const QString&)), SLOT(processLog(const QString&)));
}

void WizDownloadObjectData::setData(const WIZOBJECTDATA& data)
{
    Q_ASSERT(!data.strObjectGUID.isEmpty());
    Q_ASSERT(!data.strKbGUID.isEmpty());

    m_data = data;
    setDatabase(m_dbMgr.db(data.strKbGUID));
    m_bInited = true;
}

void WizDownloadObjectData::startDownload()
{
    Q_ASSERT(m_bInited);
    QString strUserId = m_dbMgr.db().getUserId();
    QString strPasswd = m_dbMgr.db().getPassword();

    setKbUrl(WIZ_API_URL);
    Q_ASSERT(0);
    //callClientLogin(strUserId, strPasswd);
}

void WizDownloadObjectData::onClientLogin(const WIZUSERINFO& userInfo)
{
    Q_UNUSED(userInfo);

    // user private document
    if (m_data.strKbGUID == userInfo.strKbGUID) {
        setKbUrl(userInfo.strDatabaseServer);
        startDownloadObjectData();
        return;
    }

    // group document
    //QString strKbUrl = m_dbMgr.db(m_data.strKbGUID).server();
    QString strKbUrl;
    if (strKbUrl.isEmpty()) {
        callGetGroupList();
    } else {
        setKbUrl(strKbUrl);
        startDownloadObjectData();
    }
}

void WizDownloadObjectData::onGetGroupList(const CWizGroupDataArray& arrayGroup)
{
    CWizGroupDataArray::const_iterator it;
    for (it = arrayGroup.begin(); it != arrayGroup.end(); it++) {
        const WIZGROUPDATA& group = *it;

        if (group.strGroupGUID == m_data.strKbGUID) {
            setKbUrl(group.strDatabaseServer);
            startDownloadObjectData();
            return;
        }
    }
}

void WizDownloadObjectData::startDownloadObjectData()
{
    setKbGUID(m_data.strKbGUID);
    downloadObjectData(m_data);
}

void WizDownloadObjectData::onDownloadObjectDataCompleted(const WIZOBJECTDATA& data)
{
    if (!data.strObjectGUID.isEmpty())
        CWizApi::onDownloadObjectDataCompleted(data);

    m_data.arrayData = data.arrayData;
    //callClientLogout();

    m_bInited = false;

    Q_EMIT downloadDone(true);
}

void WizDownloadObjectData::processLog(const QString& strMsg)
{
    TOLOG(strMsg);
}

void WizDownloadObjectData::onXmlRpcError(const QString& strMethodName,
                                           WizXmlRpcError err,
                                           int errorCode,
                                           const QString& errorMessage)
{
    CWizApi::onXmlRpcError(strMethodName, err, errorCode, errorMessage);
    Q_EMIT downloadDone(false);

    m_bInited = false;
}
