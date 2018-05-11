#include "WizSync.h"
#include "WizKMSync_p.h"
#include "WizKMSync.h"

#include <QString>

#include "utils/WizPathResolve.h"
#include "WizApiEntry.h"
#include "WizAvatarHost.h"

#include "share/WizSyncableDatabase.h"
#include "share/WizAnalyzer.h"
#include "share/WizEventLoop.h"

#include "utils/WizMisc.h"

#define IDS_BIZ_SERVICE_EXPR    "Your {p} business service has expired."
#define IDS_BIZ_NOTE_COUNT_LIMIT     QObject::tr("Group notes count limit exceeded!")

void GetSyncProgressRange(WizKMSyncProgress progress, int& start, int& count)
{
    int data[syncDownloadObjectData - syncAccountLogin + 1] = {
        1, //syncAccountLogin,
        1, //syncDatabaseLogin,
        2, //syncDownloadDeletedList,
        2, //syncUploadDeletedList,
        2, //syncUploadTagList,
        2, //syncUploadStyleList,
        30, //syncUploadDocumentList,
        1, //syncUploadParamList,
        10, //syncUploadAttachmentList,
        2, //syncDownloadTagList,
        2, //syncDownloadStyleList,
        5, //syncDownloadSimpleDocumentList,
        10, //syncDownloadFullDocumentList
        1, //syncDownloadParamList
        5, //syncDownloadAttachmentList,
        24, //syncDownloadObjectData
    };
    //
    //
    start = 0;
    count = data[progress];
    //
    for (int i = 0; i < progress; i++)
    {
        start += data[i];
    }
    //
    ATLASSERT(count > 0);
}



int GetSyncProgressSize(WizKMSyncProgress progress)
{
    int start = 0;
    int count = 0;
    GetSyncProgressRange(progress, start, count);
    return count;
}

int GetSyncStartProgress(WizKMSyncProgress progress)
{
    int start = 0;
    int count = 0;
    GetSyncProgressRange(progress, start, count);
    return start;
}


WizKMSync::WizKMSync(IWizSyncableDatabase* pDatabase, const WIZUSERINFOBASE& userInfo, const WIZKBINFO& kbInfo, const WIZKBVALUEVERSIONS& versions, IWizKMSyncEvents* pEvents, bool bGroup, bool bUploadOnly, QObject* parent)
    : m_pDatabase(pDatabase)
    , m_userInfo(userInfo)
    , m_pEvents(pEvents)
    , m_bGroup(bGroup)
    , m_server(userInfo, kbInfo, versions, parent)
    , m_bUploadOnly(bUploadOnly)
{
    m_server.setEvents(m_pEvents);
}

bool WizKMSync::sync()
{
    QString strKbGUID = m_bGroup ? m_userInfo.strKbGUID  : QString("");
    m_pEvents->onBeginKb(strKbGUID);

    bool bRet = syncCore();

    m_pEvents->onEndKb(strKbGUID);
    return bRet;
}

bool WizKMSync::syncCore()
{
    m_mapOldKeyValues.clear();
    m_pEvents->onSyncProgress(::GetSyncStartProgress(syncDatabaseLogin));
    m_pEvents->onStatus(QObject::tr("Connect to server"));

    if (m_pEvents->isStop())
        return FALSE;

    m_pEvents->onStatus(QObject::tr("Query server infomation"));
    //
    //
    if (!m_server.kb_getInfo())
        return false;
    //
    QString kbInfoKey = m_bGroup ? m_userInfo.strKbGUID : "";
    m_pDatabase->setKbInfo(kbInfoKey, m_server.kbInfo());
    //
    if (m_pEvents->isStop())
        return FALSE;
    //
    WIZKBINFO info = m_server.kbInfo();
    //
    m_pEvents->onStatus(QObject::tr("Query deleted objects list"));
    if (!downloadDeletedList(info.nDeletedGUIDVersion))
    {
        m_pEvents->onError(QObject::tr("Cannot download deleted objects list!"));
        return FALSE;
    }
    //
    if (m_pEvents->isStop())
        return FALSE;
    //
    m_pEvents->onStatus(QObject::tr("Upload deleted objects list"));
    if (!uploadDeletedList())
    {
        m_pEvents->onError(QObject::tr("Cannot upload deleted objects list!"));
        //return FALSE;
    }
    //
    if (m_pEvents->isStop())
        return FALSE;
    //
    m_pEvents->onStatus(QObject::tr("Upload tags"));
    if (!uploadTagList())
    {
        m_pEvents->onError(QObject::tr("Cannot upload tags!"));
        return FALSE;
    }
    //
    //
    if (m_pEvents->isStop())
        return FALSE;
    //
    m_pEvents->onStatus(QObject::tr("Upload styles"));
    if (!uploadStyleList())
    {
        m_pEvents->onError(QObject::tr("Cannot upload styles!"));
        return FALSE;
    }
    //
    //
    if (m_pEvents->isStop())
        return FALSE;
    //
    m_pEvents->onStatus(QObject::tr("Upload notes"));
    if (!uploadDocumentList())
    {
        m_pEvents->onError(QObject::tr("Cannot upload notes!"));
        return FALSE;
    }
    //
    if (m_pEvents->isStop())
        return FALSE;
    //
    if (!uploadParamList()) {
        m_pEvents->onError(QObject::tr("Cannot upload params!"));
    }
    //
    if (m_pEvents->isStop())
        return FALSE;
    //
    //上传完笔记之后再上传文件夹等设置
    m_pEvents->onStatus(QObject::tr("Sync settings"));
    uploadKeys();
    //
    if (m_pEvents->isStop())
        return FALSE;
    //
    m_pEvents->onStatus(QObject::tr("Upload attachments"));
    if (!uploadAttachmentList())
    {
        m_pEvents->onError(QObject::tr("Cannot upload attachments!"));
        return FALSE;
    }
    //
    if (m_pEvents->isStop())
        return FALSE;
    //
    if (m_bUploadOnly)
        return TRUE;
    //    

    if (m_pEvents->isStop())
        return FALSE;
    //
    m_pEvents->onStatus(QObject::tr("Download tags"));
    if (!downloadTagList(info.nTagVersion))
    {
        m_pEvents->onError(QObject::tr("Cannot download tags!"));
        return FALSE;
    }
    //
    if (m_pEvents->isStop())
        return FALSE;
    //
    m_pEvents->onStatus(QObject::tr("Download styles"));
    if (!downloadStyleList(info.nStyleVersion))
    {
        m_pEvents->onError(QObject::tr("Cannot download styles!"));
        return FALSE;
    }
    //
    if (m_pEvents->isStop())
        return FALSE;
    //
    m_pEvents->onStatus(QObject::tr("Download notes list"));
    if (!downloadDocumentList(info.nDocumentVersion))
    {
        m_pEvents->onError(QObject::tr("Cannot download notes list!"));
        return FALSE;
    }
    //
    if (m_pEvents->isStop())
        return FALSE;
    //
    //should after download tags for grouo tag positions
    m_pEvents->onStatus(QObject::tr("Sync settings"));
    downloadKeys();
    //
    /*
    // 重新更新服务器的数据，因为如果pc客户端文件夹被移动后，
    // 服务器上面已经没有这个文件夹了，
    // 但是手机同步的时候，因为原有的文件夹里面还有笔记，
    // 因此不会被删除，导致手机上还有空的文件夹
    // 因此在这里需要重新更新一下
    */
    m_pEvents->onStatus(QObject::tr("Sync settings"));
    processOldKeyValues();
    //
    if (m_pEvents->isStop())
        return FALSE;
    //
    m_pEvents->onStatus(QObject::tr("Download param list"));
    if (!downloadParamList(info.nParamVersion))
    {
        m_pEvents->onError(QObject::tr("Cannot download param list!"));
    }
    //
    m_pEvents->onStatus(QObject::tr("Download attachments list"));
    if (!downloadAttachmentList(info.nAttachmentVersion))
    {
        m_pEvents->onError(QObject::tr("Cannot download attachments list!"));
        return FALSE;
    }
    //
    if (m_pEvents->isStop())
        return FALSE;
    //
    //
    if (!m_bGroup)
    {
        if (m_server.kb_getInfo())
        {
            m_pDatabase->setKbInfo("", m_server.kbInfo());
        }
    }
    //
    m_pEvents->onSyncProgress(100);
    //
    return TRUE;
}



bool WizKMSync::uploadKeys()
{
    CWizStdStringArray arrValue;
    m_pDatabase->getKBKeys(arrValue);
    //
    for (CWizStdStringArray::const_iterator it = arrValue.begin();
        it != arrValue.end();
        it++)
    {
        QString strKey = *it;
        //
        if (!m_pDatabase->processValue(strKey))
            continue;
        //
        if (!uploadValue(strKey))
        {
            m_pEvents->onError(WizFormatString1("Can't upload settings: %1", strKey));
        }
    }
    //
    return TRUE;
}


bool WizKMSync::downloadKeys()
{
    CWizStdStringArray arrValue;
    m_pDatabase->getKBKeys(arrValue);
    //
    for (CWizStdStringArray::const_iterator it = arrValue.begin();
        it != arrValue.end();
        it++)
    {
        QString strKey = *it;
        //
        if (!m_pDatabase->processValue(strKey))
            continue;
        //
        if (!downloadValue(strKey))
        {
            m_pEvents->onError(WizFormatString1("Can't download settings: %1", strKey));
        }
    }
    //
    return TRUE;
}

/*
 * ////重新设置服务器的key value数据 防止被移动的文件夹没有删除
 */
bool WizKMSync::processOldKeyValues()
{
    if (m_mapOldKeyValues.empty())
        return TRUE;
    //
    //for (typename std::map<QString, WIZKEYVALUEDATA>::const_iterator it = m_mapOldKeyValues.begin();
    for (std::map<QString, WIZKEYVALUEDATA>::const_iterator it = m_mapOldKeyValues.begin();
        it != m_mapOldKeyValues.end();
        it++)
    {
        QString strKey = it->first;
        const WIZKEYVALUEDATA& data = it->second;
        //
        // 最后一次才记住版本号
        m_pDatabase->setLocalValue(strKey, data.strValue, data.nVersion, TRUE);
    }
    //
    return TRUE;
}

bool WizKMSync::uploadValue(const QString& strKey)
{
    if (!m_pDatabase)
        return FALSE;
    //
    __int64 nLocalVersion = m_pDatabase->getLocalValueVersion(strKey);
    if (-1 != nLocalVersion)
        return TRUE;
    //
    QString strValue = m_pDatabase->getLocalValue(strKey);
    //
    DEBUG_TOLOG(WizFormatString1("Upload key: %1", strKey));
    DEBUG_TOLOG(strValue);
    //
    __int64 nServerVersion = 0;
    if (m_server.setValue(strKey, strValue, nServerVersion))
    {
        m_pDatabase->setLocalValueVersion(strKey, nServerVersion);
    }
    else
    {
        m_pEvents->onError(WizFormatString1("Can't upload settings: %1", strKey));
    }
    //
    return TRUE;
}
bool WizKMSync::downloadValue(const QString& strKey)
{
    if (!m_pDatabase)
        return FALSE;
    //
    __int64 nServerVersion = 0;
    if (!m_server.getValueVersion(strKey, nServerVersion))
    {
        TOLOG1("Can't get value version: %1", strKey);
        return FALSE;
    }
    //
    if (-1 == nServerVersion)	//not found
        return TRUE;
    //
    __int64 nLocalVersion = m_pDatabase->getLocalValueVersion(strKey);
    if (nServerVersion <= nLocalVersion)
        return TRUE;
    //
    //
    QString strServerValue;
    if (!m_server.getValue(strKey, strServerValue, nServerVersion))
    {
        return FALSE;
    }
    //
    DEBUG_TOLOG(WizFormatString1("Download key: %1", strKey));
    DEBUG_TOLOG(strServerValue);
    //
    m_pDatabase->setLocalValue(strKey, strServerValue, nServerVersion, FALSE);
    //
    m_mapOldKeyValues[strKey] = WIZKEYVALUEDATA(strServerValue, nServerVersion);
    //
    return TRUE;
}



template <class TData>
bool GetModifiedObjectList(IWizSyncableDatabase* pDatabase, std::deque<TData>& arrayData)
{
    ATLASSERT(FALSE);
    return FALSE;
}


template <class TData>
bool GetModifiedObjectList(IWizSyncableDatabase* pDatabase, std::deque<WIZDELETEDGUIDDATA>& arrayData)
{
    return pDatabase->getModifiedDeletedList(arrayData);
}

template <class TData>
bool GetModifiedObjectList(IWizSyncableDatabase* pDatabase, std::deque<WIZTAGDATA>& arrayData)
{
    return pDatabase->getModifiedTagList(arrayData);
}

template <class TData>
bool GetModifiedObjectList(IWizSyncableDatabase* pDatabase, std::deque<WIZSTYLEDATA>& arrayData)
{
    return pDatabase->getModifiedStyleList(arrayData);
}

template <class TData>
bool GetModifiedObjectList(IWizSyncableDatabase* pDatabase, std::deque<WIZDOCUMENTDATAEX>& arrayData)
{
    return pDatabase->getModifiedDocumentList(arrayData);
}

template <class TData>
bool GetModifiedObjectList(IWizSyncableDatabase* pDatabase, std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayData)
{
    return pDatabase->getModifiedAttachmentList(arrayData);
}

template <class TData>
bool GetModifiedObjectList(IWizSyncableDatabase* pDatabase, std::deque<WIZDOCUMENTPARAMDATA>& arrayData)
{
    return pDatabase->getModifiedParamList(arrayData);
}


template <class TData>
bool onUploadObject(IWizSyncableDatabase* pDatabase, const TData& data, const QString& strObjectType)
{
    return pDatabase->onUploadObject(data.strGUID, strObjectType);
}

template <class TData>
bool onUploadObject(IWizSyncableDatabase* pDatabase, const WIZDOCUMENTPARAMDATA& data, const QString& strObjectType)
{
    return pDatabase->onUploadParam(data.strDocumentGuid, data.strName);
}


template <class TData>
bool UploadSimpleList(const QString& strObjectType, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, WizKMDatabaseServer& server, WizKMSyncProgress progress)
{
    pEvents->onSyncProgress(::GetSyncStartProgress(progress));
    //
    std::deque<TData> arrayData;
    GetModifiedObjectList<TData>(pDatabase, arrayData);
    if (arrayData.empty())
    {
        pEvents->onStatus(QObject::tr("No change, skip"));
        return TRUE;
    }
    //
    //
    if (!server.postList<TData>(arrayData))
    {
        TOLOG(QObject::tr("Can't upload list!"));
        return FALSE;
    }
    //
    for (typename std::deque<TData>::const_iterator it = arrayData.begin();
        it != arrayData.end();
        it++)
    {
        onUploadObject<TData>(pDatabase, *it, strObjectType);
    }
    //
    return TRUE;
}

bool WizKMSync::uploadDeletedList()
{
    if (m_bGroup)
    {
        if (!m_pDatabase->isGroupAuthor())	//need author
            return TRUE;
    }
    //
    return UploadSimpleList<WIZDELETEDGUIDDATA>("deleted_guid", m_pEvents, m_pDatabase, m_server, syncUploadDeletedList);
}
bool WizKMSync::uploadTagList()
{
    if (m_bGroup)
    {
        if (!m_pDatabase->isGroupSuper())	//need super
            return TRUE;
    }
    //
    return UploadSimpleList<WIZTAGDATA>("tag", m_pEvents, m_pDatabase, m_server, syncUploadTagList);
}
bool WizKMSync::uploadStyleList()
{
    if (m_bGroup)
    {
        if (!m_pDatabase->isGroupEditor())	//need editor
            return TRUE;
    }
    //
    return UploadSimpleList<WIZSTYLEDATA>("style", m_pEvents, m_pDatabase, m_server, syncUploadStyleList);
}


bool WizKMSync::uploadParamList()
{
    if (m_bGroup)
    {
        if (!m_pDatabase->isGroupAuthor())	//need author
            return TRUE;
    }
    //
    return UploadSimpleList<WIZDOCUMENTPARAMDATA>("param", m_pEvents, m_pDatabase, m_server, syncUploadParamList);
}

bool InitDocumentData(IWizSyncableDatabase* pDatabase, const QString& strObjectGUID, WIZDOCUMENTDATAEX& data, bool forceUploadData)
{
    return pDatabase->initDocumentData(strObjectGUID, data, forceUploadData);
}

bool InitAttachmentData(IWizSyncableDatabase* pDatabase, const QString& strObjectGUID, WIZDOCUMENTATTACHMENTDATAEX& data)
{
    return pDatabase->initAttachmentData(strObjectGUID, data);
}


QByteArray WizCompressAttachmentFile(const QByteArray& stream, QString& strTempFileName, const WIZDOCUMENTATTACHMENTDATAEX& data);

template <class TData>
bool CanEditData(IWizSyncableDatabase* pDatabase, const TData& data)
{
    ATLASSERT(FALSE);
    return FALSE;
}
//
template <class TData>
bool CanEditData(IWizSyncableDatabase* pDatabase, const WIZDOCUMENTDATAEX& data)
{
    ATLASSERT(pDatabase->isGroup());
    return pDatabase->canEditDocument(data);
}
//
template <class TData>
bool CanEditData(IWizSyncableDatabase* pDatabase, const WIZDOCUMENTATTACHMENTDATAEX& data)
{
    ATLASSERT(pDatabase->isGroup());
    return pDatabase->canEditAttachment(data);
}


void SaveServerError(const WIZKBINFO& kbInfo, const WizKMDatabaseServer& server, const QString& localKbGUID, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase)
{
    int nServerErrorCode = server.getLastErrorCode();
    switch (nServerErrorCode) {
    case WIZKM_XMLRPC_ERROR_TRAFFIC_LIMIT:
    {
        QString strMessage = WizFormatString2("Monthly traffic limit reached! \n\nTraffic Limit %1\nTraffic Using:%2",
                                              ::WizInt64ToStr(kbInfo.nTrafficLimit),
                                              ::WizInt64ToStr(kbInfo.nTrafficUsage)
                                              );
        //
        pDatabase->onTrafficLimit(strMessage + "\n\n" + server.getLastErrorMessage());
        //
        pEvents->onTrafficLimit(pDatabase);
        break;
    }
    case WIZKM_XMLRPC_ERROR_STORAGE_LIMIT:
    {
        QString strMessage = WizFormatString3("Storage limit reached.\n\n%1\nStorage Limit: %2, Storage Using: %3", "",
                                              ::WizInt64ToStr(kbInfo.nStorageLimit),
                                              ::WizInt64ToStr(kbInfo.nStorageUsage)
                                              );
        //
        pDatabase->onStorageLimit(strMessage + "\n\n" + server.getLastErrorMessage());
        //
        pEvents->onStorageLimit(pDatabase);
        break;
    }
    case WIZKM_XMLRPC_ERROR_BIZ_SERVICE_EXPR:
    {
        CString strMessage = WizFormatString0(IDS_BIZ_SERVICE_EXPR);
        //
        QString strBizGUID;
        pDatabase->getBizGuid(localKbGUID, strBizGUID);
        pDatabase->onBizServiceExpr(strBizGUID, strMessage);
        //
        pEvents->onBizServiceExpr(pDatabase);
        break;
    }
    case WIZKM_XMLRPC_ERROR_NOTE_COUNT_LIMIT:
    {
        CString strMessage = WizFormatString0(IDS_BIZ_NOTE_COUNT_LIMIT);
        //
        QString strBizGUID;
        pDatabase->getBizGuid(localKbGUID, strBizGUID);
        pDatabase->onNoteCountLimit(strMessage);
        //
        pEvents->onBizNoteCountLimit(pDatabase);
        break;
    }
    case WIZKM_XMLRPC_ERROR_FREE_SERVICE_EXPR:
    {
        pEvents->onFreeServiceExpr(pDatabase->getGroupInfo());
        break;
    }
    case WIZKM_XMLRPC_ERROR_VIP_SERVICE_EXPR:
    {
        pEvents->onVipServiceExpr(pDatabase->getGroupInfo());
        break;
    }
    default:
        break;
    }
}




bool UploadDocumentCore(const WIZKBINFO& kbInfo, int size, int start, int total, int index, WIZDOCUMENTDATAEX& local, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, WizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress, bool forceUploadData)
{
    QString strDisplayName;

    strDisplayName = local.strTitle;
    //
    if (pDatabase->isGroup())
    {
        if (!CanEditData<WIZDOCUMENTDATAEX>(pDatabase, local))
        {
            //skip
            return TRUE;
        }
    }
    //
    if (!InitDocumentData(pDatabase, local.strGUID, local, forceUploadData))
    {
        pEvents->onError(_TR("Cannot init object data!"));
        TOLOG(_T("Cannot init object data!"));
        return FALSE;
    }
    //
    WizOleDateTime tLocalModified;
    tLocalModified = local.tModified;
    //
    int maxFileSize = server.getMaxFileSize();
    if (maxFileSize == 1)
    {
        pEvents->setLastErrorCode(WIZKM_XMLRPC_ERROR_FREE_SERVICE_EXPR);
        QString error = QObject::tr("User service of has expired, please upgrade to VIP.");
        pEvents->setLastErrorMessage(error);
        pEvents->onFreeServiceExpr(pDatabase->getGroupInfo());
        return FALSE;
    }
    else if (maxFileSize == 2)
    {
        pEvents->setLastErrorCode(WIZKM_XMLRPC_ERROR_VIP_SERVICE_EXPR);
        QString error = QObject::tr("VIP service of has expired, please renew to VIP.");
        pEvents->setLastErrorMessage(error);
        pEvents->onVipServiceExpr(pDatabase->getGroupInfo());
        return FALSE;
    }
    //
    //check data size
    if (!local.arrayData.isEmpty())
    {
        __int64 nDataSize = local.arrayData.size();
        if (nDataSize > maxFileSize)
        {
            QString str;
            str = local.strTitle;

            pEvents->onWarning(WizFormatString2(_TR("[%1] is too large (%2), skip it"), str, QString::number(nDataSize)));
            return FALSE;
        }
    }
    //
    //
    //upload
    bool withData = !local.arrayData.isEmpty();
    __int64 nServerVersion = -1;
    QString strParts = withData ? "info" : "data";
    QString strInfo = WizFormatString2(QObject::tr("Upload note [%2] %1"), local.strTitle, strParts);
    bool succeeded = server.document_postData(local, withData, nServerVersion);
    //
    if (!succeeded)
    {
        switch (server.getLastErrorCode())
        {
        case WIZKM_XMLRPC_ERROR_TRAFFIC_LIMIT:
        case WIZKM_XMLRPC_ERROR_STORAGE_LIMIT:
        case WIZKM_XMLRPC_ERROR_BIZ_SERVICE_EXPR:
        case WIZKM_XMLRPC_ERROR_NOTE_COUNT_LIMIT:
            return FALSE;
        }
    }
    //
    //
    bool updateVersion = false;
    //
    if (succeeded)
    {
        if (-1 != nServerVersion)
        {
            WIZDOCUMENTDATAEX local2 = local;
            local2.nDataChanged = 0;
            bool forceUploadData = false;
            InitDocumentData(pDatabase, local.strGUID, local2, forceUploadData);
            //
            WizOleDateTime tLocalModified2;
            tLocalModified2 = local2.tModified;
            //
            if (tLocalModified2 == tLocalModified)
            {
                updateVersion = true;
                pDatabase->setObjectLocalServerVersion(local.strGUID, strObjectType, nServerVersion);
            }
        }
    }
    //
    //
    double fPos = index / double(total) * size;
    pEvents->onSyncProgress(start + int(fPos));
    //
    if (!succeeded)
    {
        pEvents->onWarning(QObject::tr("Cannot upload note data: %1").arg(local.strTitle));
        return FALSE;
    }
    //
    if (!updateVersion)
    {
        pEvents->onError(WizFormatString1("Cannot update local version of note: %1!", local.strTitle));
        //
        return FALSE;
    }
    //
    return TRUE;
}

bool UploadDocument(const WIZKBINFO& kbInfo, int size, int start, int total, int index, WIZDOCUMENTDATAEX& local, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, WizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress)
{
    bool forceUploadData = false;
    server.clearJsonResult();
    server.clearLocalError();
    bool ret = UploadDocumentCore(kbInfo, size, start, total, index, local, pEvents, pDatabase, server, strObjectType, progress, forceUploadData);
    if (ret)
        return true;
    //
    if (server.lastJsonResult().externCode == "WizErrorUploadNoteData")
    {
        QString fileName = pDatabase->getDocumentFileName(local.strGUID);
        if (WizPathFileExists(fileName) && Utils::WizMisc::getFileSize(fileName) > 0)
        {
            local.nDataChanged = 1;
            forceUploadData = true;
            ret = UploadDocumentCore(kbInfo, size, start, total, index, local, pEvents, pDatabase, server, strObjectType, progress, forceUploadData);
            if (ret)
                return true;
            //
            if (server.lastLocalError() == "WizErrorInvalidZip")
            {
                ////无效的zip文件////
                pDatabase->deleteDocumentFromLocal(local.strGUID);
            }
        }
        else
        {
            ////本地没有数据，服务器也没有数据////
            pDatabase->deleteDocumentFromLocal(local.strGUID);
        }
    }
    //
    return false;
}


bool UploadAttachment(const WIZKBINFO& kbInfo, int size, int start, int total, int index, WIZDOCUMENTATTACHMENTDATAEX& local, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, WizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress)
{
    QString strDisplayName;

    strDisplayName = local.strName;
    //
    if (-1 == pDatabase->getObjectLocalVersion(local.strDocumentGUID, "document"))
    {
        pEvents->onWarning(WizFormatString1(_TR("Note of attachment [%1] has not been uploaded, skip this attachment!"), strDisplayName));
        return FALSE;
    }
    //
    if (pDatabase->isGroup())
    {
        if (!CanEditData<WIZDOCUMENTATTACHMENTDATAEX>(pDatabase, local))
        {
            //skip
            return TRUE;
        }
    }
    //
    if (!InitAttachmentData(pDatabase, local.strGUID, local))
    {
        pEvents->onError(_TR("Cannot init object data!"));
        return FALSE;
    }
    //
    WizOleDateTime tLocalModified;
    tLocalModified = local.tDataModified;
    //
    //check data size
    if (local.arrayData.isEmpty())
    {
        pEvents->onError(_TR("No attachment data"));
        return FALSE;
    }
    //
    int maxFileSize = server.getMaxFileSize();
    if (maxFileSize == 1)
    {
        pEvents->setLastErrorCode(WIZKM_XMLRPC_ERROR_FREE_SERVICE_EXPR);
        QString error = QObject::tr("User service of has expired, please upgrade to VIP.");
        pEvents->setLastErrorMessage(error);
        pEvents->onFreeServiceExpr(pDatabase->getGroupInfo());
        return FALSE;
    }
    else if (maxFileSize == 2)
    {
        pEvents->setLastErrorCode(WIZKM_XMLRPC_ERROR_VIP_SERVICE_EXPR);
        QString error = QObject::tr("VIP service of has expired, please renew to VIP.");
        pEvents->setLastErrorMessage(error);
        pEvents->onVipServiceExpr(pDatabase->getGroupInfo());
        return FALSE;
    }
    //
    __int64 nDataSize = local.arrayData.size();
    if (nDataSize > server.getMaxFileSize())
    {
        QString str;
        str = local.strName;
        pEvents->onWarning(WizFormatString2(_TR("[%1] is too large (%2), skip it"), str, QString::number(nDataSize)));
        return FALSE;
    }
    //
    //
    //upload
    __int64 nServerVersion = -1;
    //
    QString strInfo = WizFormatString1(_TR("Updating attachment [data] %1"), local.strName);
        //
    pEvents->onStatus(strInfo);
    bool succeeded = server.postData<WIZDOCUMENTATTACHMENTDATAEX>(local, true, nServerVersion);
    if (!succeeded)
    {
        switch (server.getLastErrorCode())
        {
        case WIZKM_XMLRPC_ERROR_TRAFFIC_LIMIT:
        case WIZKM_XMLRPC_ERROR_STORAGE_LIMIT:
        case WIZKM_XMLRPC_ERROR_BIZ_SERVICE_EXPR:
            return FALSE;
        }
    }
    //
    bool updateVersion = false;
    //
    if (succeeded)
    {
        if (-1 != nServerVersion)
        {
            WIZDOCUMENTATTACHMENTDATAEX local2 = local;
            InitAttachmentData(pDatabase, local.strGUID, local2);
            //
            WizOleDateTime tLocalModified2;
            tLocalModified2 = local2.tDataModified;
            //
            if (tLocalModified2 == tLocalModified)
            {
                updateVersion = true;
                pDatabase->setObjectLocalServerVersion(local.strGUID, strObjectType, nServerVersion);
            }
        }
    }
    //
    //
    double fPos = index / double(total) * size;
    pEvents->onSyncProgress(start + int(fPos));
    //
    if (!succeeded)
    {
        pEvents->onWarning(WizFormatString1("Cannot upload attachment data: %1", local.strName));
        return FALSE;
    }
    //
    if (updateVersion)
    {
        pEvents->onError(WizFormatString1("Local version of attachment: %1 updated!", local.strName));
    }
    //
    return TRUE;
}


template <class TData>
bool UploadObject(const WIZKBINFO& kbInfo, int size, int start, int total, int index, std::map<QString, TData>& mapDataOnServer, TData& local, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, WizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress)
{
    ATLASSERT(false);
}

template <class TData>
bool UploadObject(const WIZKBINFO& kbInfo, int size, int start, int total, int index, WIZDOCUMENTDATAEX& local, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, WizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress)
{
    return UploadDocument(kbInfo, size, start, total, index, local, pEvents, pDatabase, server, strObjectType, progress);
}

template <class TData>
bool UploadObject(const WIZKBINFO& kbInfo, int size, int start, int total, int index, WIZDOCUMENTATTACHMENTDATAEX& local, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, WizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress)
{
    return UploadAttachment(kbInfo, size, start, total, index, local, pEvents, pDatabase, server, strObjectType, progress);
}


template <class TData, bool _document>
bool UploadList(const WIZKBINFO& kbInfo, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, WizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress)
{
    //
    typedef std::deque<TData> TArray;
    TArray arrayData;
    GetModifiedObjectList<TData>(pDatabase, arrayData);
    if (arrayData.empty())
    {
        pEvents->onStatus(QObject::tr("No change, skip"));
        return TRUE;
    }
    //
    int start = 0;
    int size = 0;
    ::GetSyncProgressRange(progress, start, size);
    pEvents->onSyncProgress(start);
    //
    int total = int(arrayData.size());
    int index = 0;
    //
    for (typename TArray::const_iterator it = arrayData.begin();
        it != arrayData.end();
        it++)
    {
        index++;
        //
        if (pEvents->isStop())
            return FALSE;
        //
        TData local = *it;
        //
        if (_document)	//
        {
            pEvents->onUploadDocument(local.strGUID, FALSE);
        }
        //
        BOOL bUploaded = TRUE;
        if (!UploadObject<TData>(kbInfo, size, start, total, index, local, pEvents, pDatabase, server, strObjectType, progress))
        {
            bUploaded = FALSE;
            //
            pEvents->onStatus(QObject::tr("Can't upload object, error code : %1").arg(WizIntToStr(server.getLastErrorCode())));
            pEvents->onStatus(server.getLastErrorMessage());
            //
            switch (server.getLastErrorCode())
            {
            case WIZKM_XMLRPC_ERROR_BIZ_SERVICE_EXPR:
            {
                pEvents->setLastErrorCode(WIZKM_XMLRPC_ERROR_BIZ_SERVICE_EXPR);
                QString strBizGUID;
                pDatabase->getBizGuid(local.strKbGUID, strBizGUID);
                WIZBIZDATA bizData;
                pDatabase->getBizData(strBizGUID, bizData);
                QString error = pEvents->getLastErrorMessage() + QObject::tr("Team service of ' %1 ' has expired, temporarily unable to sync the new and edited notes, please renew on time.").arg(bizData.bizName) +"\n";
                pEvents->setLastErrorMessage(error);
            }
            case WIZKM_XMLRPC_ERROR_FREE_SERVICE_EXPR:
            {
                pEvents->setLastErrorCode(WIZKM_XMLRPC_ERROR_FREE_SERVICE_EXPR);
                QString error = QObject::tr("User service of has expired, please upgrade to VIP.");
                pEvents->setLastErrorMessage(error);
            }
            case WIZKM_XMLRPC_ERROR_VIP_SERVICE_EXPR:
            {
                pEvents->setLastErrorCode(WIZKM_XMLRPC_ERROR_VIP_SERVICE_EXPR);
                QString error = QObject::tr("VIP service of has expired, please renew to VIP.");
                pEvents->setLastErrorMessage(error);
            }
            case WIZKM_XMLRPC_ERROR_TRAFFIC_LIMIT:
            case WIZKM_XMLRPC_ERROR_STORAGE_LIMIT:
            case WIZKM_XMLRPC_ERROR_NOTE_COUNT_LIMIT:
                SaveServerError(kbInfo, server, local.strKbGUID, pEvents, pDatabase);                
                return FALSE;
            }
        }
        //
        if (_document && bUploaded)	//
        {
            pEvents->onUploadDocument(local.strGUID, TRUE);
            pDatabase->onObjectUploaded(local.strGUID, "document");
        }
    }

    return TRUE;
}
bool WizKMSync::uploadDocumentList()
{
    if (m_bGroup)
    {
        if (!m_pDatabase->isGroupAuthor())	//need author
            return TRUE;
    }
    //
    return UploadList<WIZDOCUMENTDATAEX, true>(m_server.kbInfo(), m_pEvents, m_pDatabase, m_server, "document", syncUploadDocumentList);
}
bool WizKMSync::uploadAttachmentList()
{
    if (m_bGroup)
    {
        if (!m_pDatabase->isGroupAuthor())	//need author
            return TRUE;
    }
    //
    return UploadList<WIZDOCUMENTATTACHMENTDATAEX, false>(m_server.kbInfo(), m_pEvents, m_pDatabase, m_server, "attachment", syncUploadAttachmentList);
}


///////////////////////////////////////////////////////////////////////////////////////////////

bool WizKMSync::downloadDeletedList(__int64 nServerVersion)
{
    return downloadList<WIZDELETEDGUIDDATA>(nServerVersion, "deleted_guid", syncDownloadDeletedList);
}

bool WizKMSync::downloadTagList(__int64 nServerVersion)
{
    return downloadList<WIZTAGDATA>(nServerVersion, "tag", syncDownloadTagList);
}

bool WizKMSync::downloadStyleList(__int64 nServerVersion)
{
    return downloadList<WIZSTYLEDATA>(nServerVersion, "style", syncDownloadStyleList);
}

bool WizKMSync::downloadDocumentList(__int64 nServerVersion)
{
    return downloadList<WIZDOCUMENTDATAEX>(nServerVersion, "document", syncDownloadSimpleDocumentList);
}



bool WizKMSync::downloadAttachmentList(__int64 nServerVersion)
{
    return downloadList<WIZDOCUMENTATTACHMENTDATAEX>(nServerVersion, "attachment", syncDownloadAttachmentList);
}


bool WizKMSync::downloadParamList(__int64 nServerVersion)
{
    return downloadList<WIZDOCUMENTPARAMDATA>(nServerVersion, "param", syncDownloadParamList);
}


bool WizCompareObjectByTypeAndTime(const WIZOBJECTDATA& data1, const WIZOBJECTDATA& data2)
{
    if (data1.eObjectType != data2.eObjectType)
    {
        return data1.eObjectType > data2.eObjectType;
    }
    //
    return data1.tTime > data2.tTime;
}

bool WizKMSync::downloadObjectData()
{
    CWizObjectDataArray arrayObject;
    if (!m_pDatabase->getObjectsNeedToBeDownloaded(arrayObject))
    {
        m_pEvents->onError("Cannot get objects need to be downloaded form server!");
        return FALSE;
    }
    //
    if (arrayObject.empty())
        return TRUE;
    //
    std::sort(arrayObject.begin(), arrayObject.end(), WizCompareObjectByTypeAndTime);
    //
    //
    int start = 0;
    int size = 0;
    ::GetSyncProgressRange(::syncDownloadObjectData, start, size);
    m_pEvents->onSyncProgress(start);
    //
    int total = int(arrayObject.size());
    //
    size_t succeeded = 0;
    //
    size_t nCount = arrayObject.size();
    for (size_t i = 0; i < nCount; i++)
    {
        if (m_pEvents->isStop())
            return FALSE;
        //
        WIZOBJECTDATA data = arrayObject[i];
        //
        QString strMsgFormat = data.eObjectType == wizobjectDocument ? _TR("Downloading note: %1"): _TR("Downloading attachment: %1");
        QString strStatus = WizFormatString1(strMsgFormat, data.strDisplayName);
        m_pEvents->onStatus(strStatus);
        //
        QByteArray stream;
        if (data.eObjectType == wizobjectDocument)
        {
            WIZDOCUMENTDATAEX ret;
            ret.strGUID = data.strObjectGUID;
            ret.strKbGUID = data.strKbGUID;
            ret.strTitle = data.strDisplayName;
            QString fileName = m_pDatabase->getDocumentFileName(ret.strGUID);
            m_server.clearJsonResult();
            if (!m_server.document_downloadData(data.strObjectGUID, ret, fileName))
            {
                if (m_server.lastJsonResult().externCode == "WizErrorNotExistsInDb")
                {
                    m_pDatabase->deleteDocumentFromLocal(ret.strGUID);
                }
                else
                {
                    m_pEvents->onError(WizFormatString1("Cannot download note data from server: %1", data.strDisplayName));
                }
                return false;
            }
            stream = ret.arrayData;
        }
        else
        {
            WIZDOCUMENTATTACHMENTDATAEX ret;
            ret.strGUID = data.strObjectGUID;
            ret.strKbGUID = data.strKbGUID;
            ret.strName = data.strDisplayName;
            m_server.clearJsonResult();
            if (!m_server.attachment_downloadData(data.strDocumentGuid, data.strObjectGUID, ret))
            {
                if (m_server.lastJsonResult().externCode == "WizErrorNotExistsInDb")
                {
                    m_pDatabase->deleteAttachmentFromLocal(data.strObjectGUID);
                }
                else
                {
                    m_pEvents->onError(WizFormatString1("Cannot download attachment data from server: %1", data.strDisplayName));
                }
                return false;
            }
            stream = ret.arrayData;
        }
        //
        if (m_pDatabase->updateObjectData(data.strDisplayName, data.strObjectGUID, WIZOBJECTDATA::objectTypeToTypeString(data.eObjectType), stream))
        {
            succeeded++;
        }
        else
        {
            m_pEvents->onError(WizFormatString1("Cannot save object data to local: %1!", data.strDisplayName));
        }
        //
        //
        int index = (int)i;
        //
        double fPos = index / double(total) * size;
        m_pEvents->onSyncProgress(start + int(fPos));
    }
    //
    return succeeded == nCount;
}


void DownloadAccountKeys(WizKMAccountsServer& server, IWizSyncableDatabase* pDatabase)
{
    CWizStdStringArray arrayKey;
    pDatabase->getAccountKeys(arrayKey);
    //
    for (CWizStdStringArray::const_iterator it = arrayKey.begin();
        it != arrayKey.end();
        it++)
    {
        QString strKey = *it;
        //
        __int64 nServerVersion = 0;
        if (!server.getValueVersion(strKey, nServerVersion))
        {
            TOLOG1("Can't get account value version: %1", strKey);
            continue;
        }
        //
        if (-1 == nServerVersion)	//not found
            continue;
        //
        __int64 nLocalVersion = pDatabase->getAccountLocalValueVersion(strKey);
        if (nServerVersion <= nLocalVersion)
            continue;
        //
        QString strServerValue;
        if (!server.getValue(strKey, strServerValue, nServerVersion))
        {
            continue;
        }
        //
        pDatabase->setAccountLocalValue(strKey, strServerValue, nServerVersion, FALSE);
    }
}


bool WizDownloadDocumentsByGuids(IWizKMSyncEvents* pEvents, WizKMAccountsServer& server, CWizGroupDataArray& arrayGroup, IWizSyncableDatabase* pDatabase, const QString& kbGuid, const CWizStdStringArray& guids, CWizDocumentDataArray& arrayDocument)
{
    /*
    ////////
    ////准备群组信息////
    */
    std::map<QString, WIZGROUPDATA> mapGroup;
    for (CWizGroupDataArray::const_iterator it = arrayGroup.begin();
        it != arrayGroup.end();
        it++)
    {
        mapGroup[it->strGroupGUID] = *it;
    }
    //
    /*
    ////下载笔记////
    */
    QString strKbGUID = kbGuid;
    const CWizStdStringArray& arrayDocumentGUID = guids;
    //
    WIZUSERINFO userInfo = server.m_userInfo;
    //
    if (kbGuid.isEmpty())
    {
        pDatabase = pDatabase->getPersonalDatabase();
    }
    else
    {
        WIZGROUPDATA group = mapGroup[strKbGUID];
        if (group.strGroupGUID.isEmpty())
            return false;
        //
        pDatabase = pDatabase->getGroupDatabase(group);
        if (!pDatabase)
        {
            pEvents->onError(WizFormatString1("Cannot open group: %1", group.strGroupName));
            return false;
        }
        //
        userInfo = WIZUSERINFO(server.m_userInfo, group);
    }
    //
    WizKMDatabaseServer serverDB(userInfo);
    //
    pEvents->onStatus(_TR("Query notes information"));
    //
    std::deque<WIZDOCUMENTDATAEX> arrayDocumentServer;
    if (!serverDB.document_getListByGuids(arrayDocumentGUID, arrayDocumentServer))
    {
        pEvents->onError("Can download notes of messages");
        return false;
    }
    //
    pDatabase->onDownloadDocumentList(arrayDocumentServer);
    arrayDocument = arrayDocumentServer;
    return true;
}



bool WizDownloadMessages(IWizKMSyncEvents* pEvents, WizKMAccountsServer& server, IWizSyncableDatabase* pDatabase, const CWizGroupDataArray& arrayGroup)
{
    __int64 nOldVersion = pDatabase->getObjectVersion("Messages");
    //
    std::deque<WIZMESSAGEDATA> arrayMessage;
    if (!server.getMessages(nOldVersion, arrayMessage))
        return false;
    //
    if (arrayMessage.empty())
    {
        return true;
    }
    //
    /*
    ////////
    ////准备群组信息////
    */
    std::map<QString, WIZGROUPDATA> mapGroup;
    for (CWizGroupDataArray::const_iterator it = arrayGroup.begin();
        it != arrayGroup.end();
        it++)
    {
        mapGroup[it->strGroupGUID] = *it;
    }
    //
    /*
    ////按照群组分组笔记////
    */
    std::map<QString, CWizStdStringArray> mapKbGUIDDocuments;
    for (WIZMESSAGEDATA it : arrayMessage)
    {
        if (!it.kbGUID.isEmpty()
            && !it.documentGUID.isEmpty())
        {
            CWizStdStringArray& documents = mapKbGUIDDocuments[it.kbGUID];
            documents.push_back(it.documentGUID);
        }
    }
    //
    /*
    ////按照kb，下载消息里面的笔记////
    */
    for (std::map<QString, CWizStdStringArray>::const_iterator it = mapKbGUIDDocuments.begin();
        it != mapKbGUIDDocuments.end();
        it++)
    {
        QString strKbGUID = it->first;
        const CWizStdStringArray& arrayDocumentGUID = it->second;
        //
        WIZGROUPDATA group = mapGroup[strKbGUID];
        if (group.strGroupGUID.isEmpty())
            continue;
        //
        IWizSyncableDatabase* pGroupDatabase = pDatabase->getGroupDatabase(group);
        if (!pGroupDatabase)
        {
            pEvents->onError(WizFormatString1("Cannot open group: %1", group.strGroupName));
            continue;
        }
        //
        WIZUSERINFO userInfo = WIZUSERINFO(server.m_userInfo, group);
        WizKMDatabaseServer serverDB(userInfo);
        //
        pEvents->onStatus(_TR("Query notes information"));
        //
        std::deque<WIZDOCUMENTDATAEX> arrayDocumentServer;
        if (!serverDB.document_getListByGuids(arrayDocumentGUID, arrayDocumentServer))
        {
            pEvents->onError("Can download notes of messages");
        }
        //
        pGroupDatabase->onDownloadDocumentList(arrayDocumentServer);
        //
        pDatabase->closeGroupDatabase(pGroupDatabase);
    }
    //
    __int64 nNewVersion = WizKMSync::getObjectsVersion<WIZMESSAGEDATA>(nOldVersion, arrayMessage);
    //
    pDatabase->onDownloadMessageList(arrayMessage);
    pDatabase->setObjectVersion("Messages", nNewVersion);

    for (WIZMESSAGEDATA it : arrayMessage)
    {
        if (it.nReadStatus == 0 && it.nDeleteStatus == 0)
        {
            QList<QVariant> paramList;
            paramList.append(wizBubbleMessageCenter);
            paramList.append(it.nId);
            paramList.append(QObject::tr("New Message"));
            paramList.append(it.messageBody);
            pEvents->onBubbleNotification(paramList);
        }
    }

    //
    return TRUE;
}

bool WizUploadMessages(IWizKMSyncEvents* pEvents, WizKMAccountsServer& server, IWizSyncableDatabase* pDatabase)
{
    CWizMessageDataArray arrayMessage;
    pDatabase->getModifiedMessageList(arrayMessage);

    qDebug() << "get modified messages , size ; " << arrayMessage.size();
    if (arrayMessage.size() == 0)
        return true;

    QString strReadIds;
    QString strDeleteIds;
    for (WIZMESSAGEDATA msg : arrayMessage)
    {
        if (msg.nReadStatus == 1 && (msg.nLocalChanged & WIZMESSAGEDATA::localChanged_Read))
        {
            strReadIds.append(QString::number(msg.nId) + ",");
        }
        if (msg.nDeleteStatus == 1 && (msg.nLocalChanged & WIZMESSAGEDATA::localChanged_Delete))
        {
            strDeleteIds.append(QString::number(msg.nId) + ",");
        }
    }

    CWizMessageDataArray::iterator it;
    if (!strReadIds.isEmpty())
    {
        strReadIds.remove(strReadIds.length() - 1, 1);      // remove the last ','
        qDebug() << "upload read message : " << strReadIds;
        if (server.setMessageReadStatus(strReadIds, 1))
        {
            QStringList readIds = strReadIds.split(',', QString::SkipEmptyParts);
            CWizMessageDataArray readMsgArray;
            for (it = arrayMessage.begin(); it != arrayMessage.end(); it++)
            {
                if (readIds.contains(QString::number(it->nId)))
                {
                    qDebug() << "upload message read status ok, update: " << it->nId;
                    it->nLocalChanged = it->nLocalChanged & ~WIZMESSAGEDATA::localChanged_Read;
                    readMsgArray.push_back(it.operator *());
                }
            }
            pDatabase->modifyMessagesLocalChanged(readMsgArray);
        }
    }

    if (!strDeleteIds.isEmpty())
    {
        strDeleteIds.remove(strDeleteIds.length() - 1, 1);      // remove the last ','
        qDebug() << "upload delete message : " << strReadIds;
        if (server.setMessageDeleteStatus(strDeleteIds, 1))
        {
            QStringList deleteIds = strDeleteIds.split(',', QString::SkipEmptyParts);
            CWizMessageDataArray deleteMsgArray;
            for (it = arrayMessage.begin(); it != arrayMessage.end(); it++)
            {
                qDebug() << "current item ; " << QString::number(it->nId);
                if (deleteIds.contains(QString::number(it->nId)))
                {
                    it->nLocalChanged = it->nLocalChanged & ~WIZMESSAGEDATA::localChanged_Delete;
                    qDebug() << "upload message read status ok, update: " << it->nId << " local changed ; " << it->nLocalChanged;
                    deleteMsgArray.push_back(it.operator *());
                }
            }
            pDatabase->modifyMessagesLocalChanged(deleteMsgArray);
        }
    }
    return true;
}

bool WizIsDayFirstSync(IWizSyncableDatabase* pDatabase)
{
    WizOleDateTime lastSyncTime = pDatabase->getLastSyncTime();
    return lastSyncTime.daysTo(WizOleDateTime::currentDateTime()) > 0;
}

//
bool WizSyncPersonalGroupAvatar(IWizSyncableDatabase* pPersonalGroupDatabase)
{
    QString strt = pPersonalGroupDatabase->meta("SYNC_INFO", "SyncPersonalGroupAvatar");
    if (!strt.isEmpty())
    {
        if (QDateTime::fromString(strt).daysTo(QDateTime::currentDateTime()) > 7)
        {
            qInfo() << "Remove all user avatar.";
            CWizStdStringArray arrayUsers;
            pPersonalGroupDatabase->getAllNotesOwners(arrayUsers);
            for (CWizStdStringArray::const_iterator it = arrayUsers.begin();
                 it != arrayUsers.end(); it ++)
            {
                WizAvatarHost::reload(*it);
            }
        }
    }

    return pPersonalGroupDatabase->setMeta("SYNC_INFO", "SyncPersonalGroupAvatar", QDateTime::currentDateTime().toString());
}

class CWizAvatarStatusChecker
{
public:
    CWizAvatarStatusChecker(const CWizBizUserDataArray& arrayUser, const QString& currentUserGUID)
        : m_arrayUser(arrayUser)
        , m_currentUserGUID(currentUserGUID)
    {
    }
private:
    CWizBizUserDataArray m_arrayUser;
    QString m_currentUserGUID;
public:
    void run()
    {
        QMap<QString, QString> mapAllUser;
        for (WIZBIZUSER bizUser : m_arrayUser)
        {
            mapAllUser.insert(bizUser.userGUID, bizUser.userId);
        }
        //
        for (QMap<QString, QString>::Iterator it = mapAllUser.begin(); it != mapAllUser.end(); it++)
        {
            QString strFileName = Utils::WizPathResolve::avatarPath() + it.value() + ".png";
            if (it.key() != m_currentUserGUID)
            {
                QFileInfo info(strFileName);
                if (info.created().daysTo(QDateTime::currentDateTime()) < 7)
                {
                    continue;
                }
            }

            WizAvatarHost::reload(it.value());
        }
    }
};

//FIXME:更新企业群组成员头像实际应该在获取biz列表的时候处理。但是现在服务器端返回的数据
//存在问题，需要在本地强制更新数据
bool WizSyncBizGroupAvatar(IWizSyncableDatabase* pPersonalDatabase)
{
    CWizBizUserDataArray arrayUser;
    pPersonalDatabase->getAllBizUsers(arrayUser);
    WIZBIZUSER userSelf;
    userSelf.userGUID = pPersonalDatabase->getUserGuid();
    userSelf.userId = pPersonalDatabase->getUserId();
    arrayUser.push_back(userSelf);

    CWizAvatarStatusChecker checker(arrayUser, userSelf.userGUID);
    checker.run();

    return true;
}



QString downloadFromUrl(const QString& strUrl)
{
    QNetworkAccessManager net;
    QNetworkReply* reply = net.get(QNetworkRequest(strUrl));


    WizAutoTimeOutEventLoop loop(reply);
    loop.exec();    

    if (loop.error() != QNetworkReply::NoError)
        return QString();

    return QString::fromUtf8(loop.result().constData());
}

void syncGroupUsers(WizKMAccountsServer& server, const CWizGroupDataArray& arrayGroup,
                    IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, bool background)
{
    pEvents->onStatus(QObject::tr("Sync group users"));

    for (CWizGroupDataArray::const_iterator it = arrayGroup.begin();
         it != arrayGroup.end();
         it++)
    {
        const WIZGROUPDATA& g = *it;
        if (!g.isBiz())
            continue;
        //
        WIZKBINFO info = server.getKbInfo(g.strGroupGUID);
        if (IWizSyncableDatabase* pGroupDatabase = pDatabase->getGroupDatabase(g))
        {
            __int64 localVersion = pGroupDatabase->getObjectVersion("user");
            if (localVersion < info.nUserVersion)
            {
                CWizBizUserDataArray arrayUser;
                if (server.getBizUsers(g.bizGUID, g.strGroupGUID, arrayUser))
                {
                    pDatabase->onDownloadBizUsers(g.strGroupGUID, arrayUser);
                    pGroupDatabase->setObjectVersion("user", info.nUserVersion);
                }
            }
        }
        //
        if (pEvents->isStop())
            return;
    }
}

bool WizSyncDatabase(const WIZUSERINFO& info, IWizKMSyncEvents* pEvents,
                     IWizSyncableDatabase* pDatabase, bool bBackground)
{
    pEvents->onStatus(QObject::tr("----------Sync start----------"));
    pEvents->onSyncProgress(0);
    pEvents->onStatus(QObject::tr("Connecting to server"));

    WizKMAccountsServer server;
    server.setUserInfo(info);
    server.setEvents(pEvents);

    pEvents->onSyncProgress(::GetSyncStartProgress(syncAccountLogin));
    pEvents->onStatus(QObject::tr("Signing in"));    

    pDatabase->setUserInfo(server.getUserInfo());
    pEvents->onSyncProgress(1);

    /*
    ////获得群组信息////
    */
    //
    //only check biz list at first sync of day, or sync by manual
    if (!bBackground || WizIsDayFirstSync(pDatabase))
    {
        pDatabase->clearLastSyncError();
        pEvents->clearLastSyncError(pDatabase);
        pEvents->onStatus(QObject::tr("Get Biz info"));
        CWizBizDataArray arrayBiz;
        if (server.getBizList(arrayBiz))
        {
            pDatabase->onDownloadBizs(arrayBiz);
            //FIXME: 因为目前服务器返回的biz列表中无头像更新数据，需要强制更新群组用户的头像。
            WizSyncBizGroupAvatar(pDatabase);
        }
        else
        {
            pEvents->setLastErrorCode(server.getLastErrorCode());
            return false;
        }
    }

    pEvents->onStatus(QObject::tr("Get groups info"));
    CWizGroupDataArray arrayGroup;
    if (server.getGroupList(arrayGroup))
    {
        pDatabase->onDownloadGroups(arrayGroup);
    }
    else
    {
        pEvents->setLastErrorCode(server.getLastErrorCode());
        pEvents->onError(QObject::tr("Can not get groups"));
        return false;
    }
    // sync analyzer info one time a day
#ifndef QT_DEBUG
    if (WizIsDayFirstSync(pDatabase))
    {
#endif
        WizGetAnalyzer().post(pDatabase);
#ifndef QT_DEBUG
    }
#endif

    //
    int groupCount = int(arrayGroup.size());
    pEvents->setDatabaseCount(groupCount + 1);
    //
    /*
    ////下载设置////
    */
    pEvents->onStatus(QObject::tr("Downloading settings"));
    DownloadAccountKeys(server, pDatabase);

    /*
    ////下载消息////
    */
    pEvents->onStatus(QObject::tr("Downloading messages"));
    WizDownloadMessages(pEvents, server, pDatabase, arrayGroup);
    //
    pEvents->onStatus(QObject::tr("Upload modified messages"));
    WizUploadMessages(pEvents, server, pDatabase);
    //
    server.initAllKbInfos();
    server.initAllValueVersions();
    //
    syncGroupUsers(server, arrayGroup, pEvents, pDatabase, bBackground);
    //
    pEvents->onStatus(QObject::tr("----------sync private notes----------"));
    //
    {
        pEvents->setCurrentDatabase(0);
        WizKMSync syncPrivate(pDatabase, server.m_userInfo,
                              server.getKbInfo(server.m_userInfo.strKbGUID),
                              server.getValueVersions(server.m_userInfo.strKbGUID),
                              pEvents, FALSE, FALSE, NULL);
        //
        if (!syncPrivate.sync())
        {
            pEvents->onText(wizSyncMeesageError, QObject::tr("Cannot sync!"));
            QString strLastError = pDatabase->getLastSyncErrorMessage();
            if (!strLastError.isEmpty() && !bBackground)
            {
                pEvents->onText(wizSyncMeesageError, QString("Sync database error, for reason : %1").arg(strLastError));
                pEvents->onMessage(wizSyncMeesageError, "", strLastError);
            }
            //quit on sync error
            return false;
        }
        else
        {
            pDatabase->saveLastSyncTime();
            pEvents->onSyncProgress(100);            
        }
    }
    //
    if (pEvents->isStop())
        return FALSE;

    pEvents->onStatus(QObject::tr("----------sync groups----------"));
    //
    for (CWizGroupDataArray::const_iterator itGroup = arrayGroup.begin();
        itGroup != arrayGroup.end();
        itGroup++)
    {
        pEvents->setCurrentDatabase(1 + int(itGroup - arrayGroup.begin()));
        //
        if (pEvents->isStop())
            return FALSE;
        //
        WIZGROUPDATA group = *itGroup;
        //
        pEvents->onSyncProgress(0);
        pEvents->onStatus(WizFormatString1(QObject::tr("----------Sync group: %1----------"), group.strGroupName));
        //
        IWizSyncableDatabase* pGroupDatabase = pDatabase->getGroupDatabase(group);
        if (!pGroupDatabase)
        {
            pEvents->onError(WizFormatString1(QObject::tr("Cannot open group: %1"), group.strGroupName));
            continue;
        }
        //
        WIZUSERINFO userInfo(server.m_userInfo, group);
        //
        WizKMSync syncGroup(pGroupDatabase, userInfo,
                            server.getKbInfo(userInfo.strKbGUID),
                            server.getValueVersions(server.m_userInfo.strKbGUID),
                            pEvents, TRUE, FALSE, NULL);
        //
        if (syncGroup.sync())
        {
            pGroupDatabase->clearLastSyncError();
            pGroupDatabase->saveLastSyncTime();
            pEvents->onStatus(WizFormatString1(QObject::tr("Sync group %1 done"), group.strGroupName));

            // sync personal group avatar.  biz group avatar has been processed when get biz info
            if (!group.isBiz())
            {
                WizSyncPersonalGroupAvatar(pGroupDatabase);
            }            
        }
        else
        {
            pEvents->onError(WizFormatString1(QObject::tr("Cannot sync group %1"), group.strGroupName));
            pEvents->onSyncProgress(100);
        }
        //
        pDatabase->closeGroupDatabase(pGroupDatabase);
    }
    //
    //
    pEvents->onStatus(QObject::tr("----------Downloading notes----------"));
    //
    {
        //pEvents->SetCurrentDatabase(0);
        WizKMSync syncPrivate(pDatabase, server.m_userInfo, WIZKBINFO(), WIZKBVALUEVERSIONS(), pEvents, FALSE, FALSE, NULL);
        //
        if (!syncPrivate.downloadObjectData())
        {
            pEvents->onText(wizSyncMeesageError, QObject::tr("Cannot sync!"));
        }
        else
        {
            pDatabase->saveLastSyncTime();
            //pEvents->OnSyncProgress(100);
        }
    }
    //
    if (pEvents->isStop())
        return FALSE;
    //
    for (CWizGroupDataArray::const_iterator itGroup = arrayGroup.begin();
        itGroup != arrayGroup.end();
        itGroup++)
    {
        pEvents->setCurrentDatabase(1 + int(itGroup - arrayGroup.begin()));
        //
        if (pEvents->isStop())
            return FALSE;
        //
        WIZGROUPDATA group = *itGroup;
        //
        //
        IWizSyncableDatabase* pGroupDatabase = pDatabase->getGroupDatabase(group);
        if (!pGroupDatabase)
        {
            pEvents->onError(WizFormatString1(QObject::tr("Cannot open group: %1"), group.strGroupName));
            continue;
        }
        //
        WIZUSERINFO userInfo = WIZUSERINFO(server.m_userInfo, group);
        //
        WizKMSync syncGroup(pGroupDatabase, userInfo, WIZKBINFO(), WIZKBVALUEVERSIONS(), pEvents, TRUE, FALSE, NULL);
        //
        if (syncGroup.downloadObjectData())
        {
            pGroupDatabase->saveLastSyncTime();
            //pEvents->OnStatus(WizFormatString1(_TR("Sync group %1 done"), group.strGroupName));
        }
        else
        {
            pEvents->onError(WizFormatString1(QObject::tr("Cannot sync group %1"), group.strGroupName));
            //pEvents->OnSyncProgress(100);
        }
        //
        pDatabase->closeGroupDatabase(pGroupDatabase);
    }
    //
    pEvents->onStatus(QObject::tr("----------Sync done----------"));
    //
    return TRUE;
}



bool WizUploadDatabase(IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, const WIZUSERINFOBASE& info, bool bGroup)
{
    WizKMSync sync(pDatabase, info, WIZKBINFO(), WIZKBVALUEVERSIONS(), pEvents, bGroup, TRUE, NULL);
    bool bRet = sync.sync();
    //
    return bRet;
}
bool WizSyncDatabaseOnly(IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, const WIZUSERINFOBASE& info, bool bGroup)
{
    WizKMSync sync(pDatabase, info, WIZKBINFO(), WIZKBVALUEVERSIONS(), pEvents, bGroup, FALSE, NULL);
    bool bRet = sync.sync();
    if (bRet)
    {
        sync.downloadObjectData();
    }
    //
    return bRet;
}


bool WizQuickDownloadMessage(const WIZUSERINFO& info, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase)
{
    pEvents->onStatus(_TR("Quick download messages"));
    WizKMAccountsServer server;
    server.setUserInfo(info);
    server.setEvents(pEvents);
    /*
    ////获得群组信息////
    */
    //
    CWizGroupDataArray arrayGroup;
    server.getGroupList(arrayGroup);
    /*
    ////下载消息////
    */
    return WizDownloadMessages(pEvents, server, pDatabase, arrayGroup);
}

bool WizDownloadDocumentsByGuids(const WIZUSERINFO& info,
                                 IWizSyncableDatabase* pDatabase,
                                 const QString& kbGuid,
                                 const CWizStdStringArray& guids,
                                 CWizDocumentDataArray& arrayDocument)
{
    WizKMAccountsServer server;
    server.setUserInfo(info);
    /*
    ////获得群组信息////
    */
    //
    CWizGroupDataArray arrayGroup;
    server.getGroupList(arrayGroup);
    /*
    ////下载消息////
    */
    //
    WizKMSyncEvents events;

    return WizDownloadDocumentsByGuids(&events, server, arrayGroup, pDatabase, kbGuid, guids, arrayDocument);
}
