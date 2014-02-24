#include "sync.h"
#include "sync_p.h"

#include <QString>

#include "apientry.h"
#include "rapidjson/document.h"

#include  "../share/wizSyncableDatabase.h"

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
        10, //syncUploadAttachmentList,
        2, //syncDownloadTagList,
        2, //syncDownloadStyleList,
        5, //syncDownloadSimpleDocumentList,
        10, //syncDownloadFullDocumentList
        5, //syncDownloadAttachmentList,
        26, //syncDownloadObjectData
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


CWizKMSync::CWizKMSync(IWizSyncableDatabase* pDatabase, const WIZUSERINFOBASE& info, IWizKMSyncEvents* pEvents, bool bGroup, bool bUploadOnly, QObject* parent)
    : m_pDatabase(pDatabase)
    , m_info(info)
    , m_pEvents(pEvents)
    , m_bGroup(bGroup)
    , m_server(m_info, parent)
    , m_bUploadOnly(bUploadOnly)
{
#ifdef _DEBUG
    pEvents->OnError(WizFormatString1(_T("XmlRpcUrl: %1"), info.strDatabaseServer));
#endif
}

bool CWizKMSync::Sync()
{
    QString strKbGUID = m_bGroup ? m_info.strKbGUID  : QString(_T(""));
    m_pEvents->OnBeginKb(strKbGUID);

    bool bRet = SyncCore();

    m_pEvents->OnEndKb(strKbGUID);
    return bRet;
}

bool CWizKMSync::SyncCore()
{
    m_pDatabase->ClearError();
    m_mapOldKeyValues.clear();
    m_pEvents->OnSyncProgress(::GetSyncStartProgress(syncDatabaseLogin));
    m_pEvents->OnStatus(_TR("Connect to server"));

    if (m_pEvents->IsStop())
        return FALSE;

    m_pEvents->OnStatus(_TR("Query server infomation"));
    if (!m_bGroup)
    {
        WIZKBINFO info;
        if (m_server.wiz_getInfo(info))
        {
            m_pDatabase->SetKbInfo(_T(""), info);
            m_kbInfo = info;
        }
    }
    else
    {
        WIZKBINFO info;
        if (m_server.wiz_getInfo(info))
        {
            m_pDatabase->SetKbInfo(m_info.strKbGUID, info);
            m_kbInfo = info;
        }
    }
    //
    WIZOBJECTVERSION versionServer;
    if (!m_server.wiz_getVersion(versionServer))
    {
        m_pEvents->OnError(_T("Cannot get version information!"));
        return FALSE;
    }
    //
    m_pEvents->OnStatus(_TR("Query deleted objects list"));
    if (!DownloadDeletedList(versionServer.nDeletedGUIDVersion))
    {
        m_pEvents->OnError(_T("Cannot download deleted objects list!"));
        return FALSE;
    }
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    m_pEvents->OnStatus(_TR("Upload deleted objects list"));
    if (!UploadDeletedList())
    {
        m_pEvents->OnError(_T("Cannot upload deleted objects list!"));
        //return FALSE;
    }
    //
    m_pEvents->OnStatus(_TR("Sync settings"));
    UploadKeys();
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    m_pEvents->OnStatus(_TR("Upload tags"));
    if (!UploadTagList())
    {
        m_pEvents->OnError(_T("Cannot upload tags!"));
        return FALSE;
    }
    //
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    m_pEvents->OnStatus(_TR("Upload styles"));
    if (!UploadStyleList())
    {
        m_pEvents->OnError(_T("Cannot upload styles!"));
        return FALSE;
    }
    //
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    m_pEvents->OnStatus(_TR("Upload notes"));
    if (!UploadDocumentList())
    {
        m_pEvents->OnError(_T("Cannot upload notes!"));
        return FALSE;
    }
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    m_pEvents->OnStatus(_TR("Upload attachments"));
    if (!UploadAttachmentList())
    {
        m_pEvents->OnError(_T("Cannot upload attachments!"));
        return FALSE;
    }
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    if (m_bUploadOnly)
        return TRUE;
    //
    m_pEvents->OnStatus(_TR("Sync settings"));
    DownloadKeys();
    //
    m_pEvents->OnStatus(_TR("Download tags"));
    if (!DownloadTagList(versionServer.nTagVersion))
    {
        m_pEvents->OnError(_T("Cannot download tags!"));
        return FALSE;
    }
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    m_pEvents->OnStatus(_TR("Download styles"));
    if (!DownloadStyleList(versionServer.nStyleVersion))
    {
        m_pEvents->OnError(_T("Cannot download styles!"));
        return FALSE;
    }
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    m_pEvents->OnStatus(_TR("Download notes list"));
    if (!DownloadSimpleDocumentList(versionServer.nDocumentVersion))
    {
        m_pEvents->OnError(_T("Cannot download notes list!"));
        return FALSE;
    }
    //
    //
    /*
    // 重新更新服务器的数据，因为如果pc客户端文件夹被移动后，
    // 服务器上面已经没有这个文件夹了，
    // 但是手机同步的时候，因为原有的文件夹里面还有笔记，
    // 因此不会被删除，导致手机上还有空的文件夹
    // 因此在这里需要重新更新一下
    */
    m_pEvents->OnStatus(_TR("Sync settings"));
    ProcessOldKeyValues();
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    m_pEvents->OnStatus(_TR("Download attachments list"));
    if (!DownloadAttachmentList(versionServer.nAttachmentVersion))
    {
        m_pEvents->OnError(_T("Cannot download attachments list!"));
        return FALSE;
    }
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    //
    if (!m_bGroup)
    {
        WIZKBINFO info;
        if (m_server.wiz_getInfo(info))
        {
            m_pDatabase->SetKbInfo(_T(""), info);
        }
    }
    //
    m_pEvents->OnSyncProgress(100);
    //
    return TRUE;
}



bool CWizKMSync::UploadKeys()
{
    CWizStdStringArray arrValue;
    m_pDatabase->GetKBKeys(arrValue);
    //
    for (CWizStdStringArray::const_iterator it = arrValue.begin();
        it != arrValue.end();
        it++)
    {
        QString strKey = *it;
        //
        if (!m_pDatabase->ProcessValue(strKey))
            continue;
        //
        if (!UploadValue(strKey))
        {
            m_pEvents->OnError(WizFormatString1(_T("Can't upload settings: %1"), strKey));
        }
    }
    //
    return TRUE;
}


bool CWizKMSync::DownloadKeys()
{
    CWizStdStringArray arrValue;
    m_pDatabase->GetKBKeys(arrValue);
    //
    for (CWizStdStringArray::const_iterator it = arrValue.begin();
        it != arrValue.end();
        it++)
    {
        QString strKey = *it;
        //
        if (!m_pDatabase->ProcessValue(strKey))
            continue;
        //
        if (!DownloadValue(strKey))
        {
            m_pEvents->OnError(WizFormatString1(_T("Can't download settings: %1"), strKey));
        }
    }
    //
    return TRUE;
}

/*
 * ////重新设置服务器的key value数据 防止被移动的文件夹没有删除
 */
bool CWizKMSync::ProcessOldKeyValues()
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
        m_pDatabase->SetLocalValue(strKey, data.strValue, data.nVersion, TRUE);
    }
    //
    return TRUE;
}

bool CWizKMSync::UploadValue(const QString& strKey)
{
    if (!m_pDatabase)
        return FALSE;
    //
    __int64 nLocalVersion = m_pDatabase->GetLocalValueVersion(strKey);
    if (-1 != nLocalVersion)
        return TRUE;
    //
    QString strValue = m_pDatabase->GetLocalValue(strKey);
    //
    DEBUG_TOLOG(WizFormatString1(_T("Upload key: %1"), strKey));
    DEBUG_TOLOG(strValue);
    //
    __int64 nServerVersion = 0;
    if (m_server.SetValue(strKey, strValue, nServerVersion))
    {
        m_pDatabase->SetLocalValueVersion(strKey, nServerVersion);
    }
    else
    {
        m_pEvents->OnError(WizFormatString1(_T("Can't upload settings: %1"), strKey));
    }
    //
    return TRUE;
}
bool CWizKMSync::DownloadValue(const QString& strKey)
{
    if (!m_pDatabase)
        return FALSE;
    //
    __int64 nServerVersion = 0;
    if (!m_server.GetValueVersion(strKey, nServerVersion))
    {
        TOLOG1(_T("Can't get value version: %1"), strKey);
        return FALSE;
    }
    //
    if (-1 == nServerVersion)	//not found
        return TRUE;
    //
    __int64 nLocalVersion = m_pDatabase->GetLocalValueVersion(strKey);
    if (nServerVersion <= nLocalVersion)
        return TRUE;
    //
    //
    QString strServerValue;
    if (!m_server.GetValue(strKey, strServerValue, nServerVersion))
    {
        return FALSE;
    }
    //
    m_pDatabase->SetLocalValue(strKey, strServerValue, nServerVersion, FALSE);
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
    return pDatabase->GetModifiedDeletedList(arrayData);
}




template <class TData>
bool GetModifiedObjectList(IWizSyncableDatabase* pDatabase, std::deque<WIZTAGDATA>& arrayData)
{
    return pDatabase->GetModifiedTagList(arrayData);
}



template <class TData>
bool GetModifiedObjectList(IWizSyncableDatabase* pDatabase, std::deque<WIZSTYLEDATA>& arrayData)
{
    return pDatabase->GetModifiedStyleList(arrayData);
}



template <class TData>
bool GetModifiedObjectList(IWizSyncableDatabase* pDatabase, std::deque<WIZDOCUMENTDATAEX>& arrayData)
{
    return pDatabase->GetModifiedDocumentList(arrayData);
}



template <class TData>
bool GetModifiedObjectList(IWizSyncableDatabase* pDatabase, std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayData)
{
    return pDatabase->GetModifiedAttachmentList(arrayData);
}


template <class TData>
bool UploadSimpleList(const QString& strObjectType, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, CWizKMDatabaseServer& server, WizKMSyncProgress progress)
{
    pEvents->OnSyncProgress(::GetSyncStartProgress(progress));
    //
    std::deque<TData> arrayData;
    GetModifiedObjectList<TData>(pDatabase, arrayData);
    if (arrayData.empty())
    {
        pEvents->OnStatus(_TR("No change, skip"));
        return TRUE;
    }
    //
    //
    if (!server.postList<TData>(arrayData))
    {
        TOLOG(_T("Can't upload list!"));
        return FALSE;
    }
    //
    for (typename std::deque<TData>::const_iterator it = arrayData.begin();
        it != arrayData.end();
        it++)
    {
        pDatabase->OnUploadObject(it->strGUID, strObjectType);
    }
    //
    return TRUE;
}

bool CWizKMSync::UploadDeletedList()
{
    if (m_bGroup)
    {
        if (!m_pDatabase->IsGroupAuthor())	//need author
            return TRUE;
    }
    //
    return UploadSimpleList<WIZDELETEDGUIDDATA>(_T("deleted_guid"), m_pEvents, m_pDatabase, m_server, syncUploadDeletedList);
}
bool CWizKMSync::UploadTagList()
{
    if (m_bGroup)
    {
        if (!m_pDatabase->IsGroupEditor())	//need editor
            return TRUE;
    }
    //
    return UploadSimpleList<WIZTAGDATA>(_T("tag"), m_pEvents, m_pDatabase, m_server, syncUploadTagList);
}
bool CWizKMSync::UploadStyleList()
{
    if (m_bGroup)
    {
        if (!m_pDatabase->IsGroupEditor())	//need editor
            return TRUE;
    }
    //
    return UploadSimpleList<WIZSTYLEDATA>(_T("style"), m_pEvents, m_pDatabase, m_server, syncUploadStyleList);
}


template <class TData>
bool InitObjectData(IWizSyncableDatabase* pDatabase, const QString& strObjectGUID, TData& data, int part)
{
    ATLASSERT(FALSE);
    return FALSE;
}

template <class TData>
bool InitObjectData(IWizSyncableDatabase* pDatabase, const QString& strObjectGUID, WIZDOCUMENTDATAEX& data, int part)
{
    return pDatabase->InitDocumentData(strObjectGUID, data, part);
}

template <class TData>
bool InitObjectData(IWizSyncableDatabase* pDatabase, const QString& strObjectGUID, WIZDOCUMENTATTACHMENTDATAEX& data, int part)
{
    return pDatabase->InitAttachmentData(strObjectGUID, data, part);
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
    ATLASSERT(pDatabase->IsGroup());
    return pDatabase->CanEditDocument(data);
}
//
template <class TData>
bool CanEditData(IWizSyncableDatabase* pDatabase, const WIZDOCUMENTATTACHMENTDATAEX& data)
{
    ATLASSERT(pDatabase->IsGroup());
    return pDatabase->CanEditAttachment(data);
}



int CalDocumentDataForUploadToServer(IWizSyncableDatabase* pDatabase, const QString& strObjectType, const WIZDOCUMENTDATAEX& dataLocal, const WIZDOCUMENTDATAEX& dataServer)
{
    int nPart = 0;

    if (dataLocal.strInfoMD5 != dataServer.strInfoMD5
            && dataLocal.tInfoModified > dataServer.tInfoModified)
    {
        nPart |= WIZKM_XMLRPC_OBJECT_PART_INFO;
    }

    if (dataLocal.strDataMD5 != dataServer.strDataMD5
            && dataLocal.tDataModified > dataServer.tDataModified)
    {
        nPart |= WIZKM_XMLRPC_OBJECT_PART_DATA;
    }

    //if (dataLocal.strDataMD5 != dataServer.strDataMD5)
    //{
    //    long nFlags = pDatabase->GetLocalFlags(dataLocal.strGUID, strObjectType);
    //    if (-1 == nFlags)	////为了兼容以前的数据，以前没有记录这个字段，所以需要用时间来判断////
    //    {
    //        if (dataLocal.tDataModified > dataServer.tDataModified)
    //        {
    //            nPart |= WIZKM_XMLRPC_OBJECT_PART_DATA;
    //        }
    //    }
    //    else
    //    {
    //        if (nFlags & WIZKM_XMLRPC_OBJECT_PART_DATA)	////本地数据修改了////
    //        {
    //            nPart |= WIZKM_XMLRPC_OBJECT_PART_DATA;
    //        }
    //    }
    //}

    if (dataLocal.strParamMD5 != dataServer.strParamMD5
            && dataLocal.tParamModified > dataServer.tParamModified)
    {
        nPart |= WIZKM_XMLRPC_OBJECT_PART_PARAM;
    }

    return nPart;
}

int CalAttachmentDataForUploadToServer(IWizSyncableDatabase* pDatabase, const QString& strObjectType, const WIZDOCUMENTATTACHMENTDATAEX& dataLocal, const WIZDOCUMENTATTACHMENTDATAEX& dataServer)
{
    int nPart = 0;

    if (dataLocal.strInfoMD5 != dataServer.strInfoMD5
            && dataLocal.tInfoModified > dataServer.tInfoModified)
    {
        nPart |= WIZKM_XMLRPC_OBJECT_PART_INFO;
    }

    if (dataLocal.strDataMD5 != dataServer.strDataMD5
            && dataLocal.tDataModified > dataServer.tDataModified)
    {
        nPart |= WIZKM_XMLRPC_OBJECT_PART_DATA;
    }

    //if (dataLocal.strDataMD5 != dataServer.strDataMD5)
    //{
    //    long nFlags = pDatabase->GetLocalFlags(dataLocal.strGUID, strObjectType);
    //    if (-1 == nFlags)	////为了兼容以前的数据，以前没有记录这个字段，所以需要用时间来判断////
    //    {
    //        if (dataLocal.tDataModified > dataServer.tDataModified)
    //        {
    //            nPart |= WIZKM_XMLRPC_OBJECT_PART_DATA;
    //        }
    //    }
    //    else
    //    {
    //        if (nFlags & WIZKM_XMLRPC_OBJECT_PART_DATA)	////本地数据修改了////
    //        {
    //            nPart |= WIZKM_XMLRPC_OBJECT_PART_DATA;
    //        }
    //    }
    //}

    return nPart;
}


template <class TData, bool _document>
int CalObjectDataForUploadToServer(IWizSyncableDatabase* pDatabase, const QString& strObjectType, const TData& dataLocal, const TData& dataServer)
{
    int nPart = 0;
    //
    if (dataLocal.strInfoMD5 != dataServer.strInfoMD5
        && dataLocal.tInfoModified > dataServer.tInfoModified)
    {
        nPart |= WIZKM_XMLRPC_OBJECT_PART_INFO;
    }
    if (dataLocal.strDataMD5 != dataServer.strDataMD5)
    {
        long nFlags = pDatabase->GetLocalFlags(dataLocal.strGUID, strObjectType);
        if (-1 == nFlags)	////为了兼容以前的数据，以前没有记录这个字段，所以需要用时间来判断////
        {
            if (dataLocal.tDataModified > dataServer.tDataModified)
            {
                nPart |= WIZKM_XMLRPC_OBJECT_PART_DATA;
            }
        }
        else
        {
            if (nFlags & WIZKM_XMLRPC_OBJECT_PART_DATA)	////本地数据修改了////
            {
                nPart |= WIZKM_XMLRPC_OBJECT_PART_DATA;
            }
        }
    }
    //
    if (_document )
    {
        if (dataLocal.strParamMD5 != dataServer.strParamMD5
            && dataLocal.tParamModified > dataServer.tParamModified)
        {
            nPart |= WIZKM_XMLRPC_OBJECT_PART_PARAM;
        }
    }
    //
    return nPart;
}





bool UploadDocument(const WIZKBINFO& kbInfo, int size, int start, int total, int index, std::map<QString, WIZDOCUMENTDATAEX>& mapDataOnServer, WIZDOCUMENTDATAEX& local, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, CWizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress)
{
    QString strDisplayName;

    strDisplayName = local.strTitle;
    //
    if (pDatabase->IsGroup())
    {
        if (!CanEditData<WIZDOCUMENTDATAEX>(pDatabase, local))
        {
            //skip
            return TRUE;
        }
    }
    //
    __int64 nOldServerVersion = -1;
    //
    //typename std::map<QString, WIZDOCUMENTDATAEX>::const_iterator itMapOnServer = mapDataOnServer.find(local.strGUID);
    std::map<QString, WIZDOCUMENTDATAEX>::const_iterator itMapOnServer = mapDataOnServer.find(local.strGUID);
    //
    UINT part = 0;
    if (itMapOnServer == mapDataOnServer.end())	//new
    {
        part = WIZKM_XMLRPC_OBJECT_PART_INFO | WIZKM_XMLRPC_OBJECT_PART_DATA | WIZKM_XMLRPC_OBJECT_PART_PARAM;
    }
    else
    {
        const WIZDOCUMENTDATAEX& server = itMapOnServer->second;
        nOldServerVersion = server.nVersion;
        part = CalDocumentDataForUploadToServer(pDatabase, strObjectType, local, server);
        //
        ////服务器有了历史版本功能，不再需要解决冲突问题////
    }
    //
    if (!InitObjectData<WIZDOCUMENTDATAEX>(pDatabase, local.strGUID, local, part))
    {
        pEvents->OnError(_TR("Cannot init object data!"));
        return FALSE;
    }
    //
    COleDateTime tLocalModified;
    tLocalModified = local.tModified;
    //
    //check data size
    if (!local.arrayData.isEmpty())
    {
        __int64 nDataSize = local.arrayData.size();
        if (nDataSize > server.GetMaxFileSize())
        {
            QString str;
            str = local.strTitle;

            //pEvents->OnWarning(WizFormatString2(_TR("[%1] is too large (%2), skip it"), str, ::WizInt64ToByteSizeStr(nDataSize)));
            return FALSE;
        }
    }
    //
    //
    //upload
    bool succeeded = false;
    __int64 nServerVersion = -1;
    if (0 == part)
    {
        succeeded = true;	//update server version to newest
        //
        if (nOldServerVersion != -1)
        {
            nServerVersion = nOldServerVersion;
        }
        else
        {
            TOLOG(_T("Fault error: part == 0 && nOldServerVersion = -1"));	//on server and version = -1?
#ifdef _DEBUG
            WizErrorMessageBox(_T("Fault error: part == 0 && nOldServerVersion = -1"));
#endif
            ATLASSERT(FALSE);
        }
    }
    else
    {
        CWizStdStringArray arrayPart;
        if (part & WIZKM_XMLRPC_OBJECT_PART_INFO)
        {
            arrayPart.push_back(_TR("information"));
        }
        if (part & WIZKM_XMLRPC_OBJECT_PART_DATA)
        {
            arrayPart.push_back(_TR("data"));
        }
        if (part & WIZKM_XMLRPC_OBJECT_PART_PARAM)
        {
            arrayPart.push_back(_TR("params"));
        }
        //
        CString strParts;
        WizStringArrayToText(arrayPart, strParts, _T(", "));
        //
        QString strInfo;
        strInfo = ::WizFormatString2(_TR("Updating note [%2] %1"), local.strTitle, strParts);
        //
        for (int i = 0; i < 2; i++)	//try twice
        {
            pEvents->OnStatus(strInfo);
            if (server.postData<WIZDOCUMENTDATAEX>(local, part, nServerVersion))
            {
                succeeded = true;
                break;
            }
            else if (server.GetLastErrorCode() == WIZKM_XMLRPC_ERROR_TRAFFIC_LIMIT)
            {
                QString strMessage = WizFormatString2("Monthly traffic limit reached! \n\nTraffic Limit %1\nTraffic Using:%2",
                                                      ::WizInt64ToStr(kbInfo.nTrafficLimit),
                                                      ::WizInt64ToStr(kbInfo.nTrafficUsage)
                    );
                //
                pDatabase->OnTrafficLimit(strMessage + _T("\n\n") + server.GetLastErrorMessage());
                //
                //pEvents->SetStop(TRUE);
                pEvents->OnTrafficLimit(pDatabase);
                return FALSE;
            }
            else if (server.GetLastErrorCode() == WIZKM_XMLRPC_ERROR_STORAGE_LIMIT)
            {
                QString strMessage = WizFormatString3("Storage limit reached.\n\n%1\nStorage Limit: %2, Storage Using: %3", _T(""),
                    ::WizInt64ToStr(kbInfo.nStorageLimit),
                    ::WizInt64ToStr(kbInfo.nStorageUsage)
                    );
                //
                pDatabase->OnStorageLimit(strMessage + _T("\n\n") + server.GetLastErrorMessage());
                //
                //pEvents->SetStop(TRUE);
                pEvents->OnStorageLimit(pDatabase);
                return FALSE;
            }
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
            InitObjectData<WIZDOCUMENTDATAEX>(pDatabase, local.strGUID, local2, WIZKM_XMLRPC_OBJECT_PART_INFO);
            //
            COleDateTime tLocalModified2;
            tLocalModified2 = local2.tModified;
            //
            if (tLocalModified2 == tLocalModified)
            {
                updateVersion = true;
                pDatabase->SetObjectLocalServerVersion(local.strGUID, strObjectType, nServerVersion);
            }
        }
    }
    //
    //
    double fPos = index / double(total) * size;
    pEvents->OnSyncProgress(start + int(fPos));
    //
    if (!succeeded)
    {
        pEvents->OnWarning(WizFormatString1(_T("Cannot upload note data: %1"), local.strTitle));
        return FALSE;
    }
    //
    if (updateVersion
        && !pDatabase->OnUploadObject(local.strGUID, strObjectType))
    {
        pEvents->OnError(WizFormatString1(_T("Cannot upload local version of document: %1!"), local.strTitle));
        //
        return FALSE;
    }
    //
    return TRUE;
}

bool UploadAttachment(const WIZKBINFO& kbInfo, int size, int start, int total, int index, std::map<QString, WIZDOCUMENTATTACHMENTDATAEX>& mapDataOnServer, WIZDOCUMENTATTACHMENTDATAEX& local, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, CWizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress)
{
    QString strDisplayName;

    strDisplayName = local.strName;
    //
    if (-1 == pDatabase->GetObjectLocalVersion(local.strDocumentGUID, _T("document")))
    {
        pEvents->OnWarning(WizFormatString1(_TR("Note of attachment [%1] has not been uploaded, skip this attachment!"), strDisplayName));
        return FALSE;
    }
    //
    if (pDatabase->IsGroup())
    {
        if (!CanEditData<WIZDOCUMENTATTACHMENTDATAEX>(pDatabase, local))
        {
            //skip
            return TRUE;
        }
    }
    //
    __int64 nOldServerVersion = -1;
    //
    //typename std::map<QString, WIZDOCUMENTATTACHMENTDATAEX>::const_iterator itMapOnServer = mapDataOnServer.find(local.strGUID);
    std::map<QString, WIZDOCUMENTATTACHMENTDATAEX>::const_iterator itMapOnServer = mapDataOnServer.find(local.strGUID);
    //
    UINT part = 0;
    if (itMapOnServer == mapDataOnServer.end())	//new
    {
        part = WIZKM_XMLRPC_OBJECT_PART_INFO | WIZKM_XMLRPC_OBJECT_PART_DATA | WIZKM_XMLRPC_OBJECT_PART_PARAM;
    }
    else
    {
        const WIZDOCUMENTATTACHMENTDATAEX& server = itMapOnServer->second;
        nOldServerVersion = server.nVersion;
        part = CalAttachmentDataForUploadToServer(pDatabase, strObjectType, local, server);
        //
        ////服务器有了历史版本功能，不再需要解决冲突问题////
    }
    //
    if (!InitObjectData<WIZDOCUMENTATTACHMENTDATAEX>(pDatabase, local.strGUID, local, part))
    {
        pEvents->OnError(_TR("Cannot init object data!"));
        return FALSE;
    }
    //
    COleDateTime tLocalModified;
    tLocalModified = local.tDataModified;
    //
    //check data size
    if (!local.arrayData.isEmpty())
    {
        __int64 nDataSize = local.arrayData.size();
        if (nDataSize > server.GetMaxFileSize())
        {
            QString str;
            str = local.strName;
            //pEvents->OnWarning(WizFormatString2(_TR("[%1] is too large (%2), skip it"), str, ::WizInt64ToByteSizeStr(nDataSize)));
            return FALSE;
        }
    }
    //
    //
    //upload
    bool succeeded = false;
    __int64 nServerVersion = -1;
    if (0 == part)
    {
        succeeded = true;	//update server version to newest
        //
        if (nOldServerVersion != -1)
        {
            nServerVersion = nOldServerVersion;
        }
        else
        {
            TOLOG(_T("Fault error: part == 0 && nOldServerVersion = -1"));	//on server and version = -1?
#ifdef _DEBUG
            WizErrorMessageBox(_T("Fault error: part == 0 && nOldServerVersion = -1"));
#endif
            ATLASSERT(FALSE);
        }
    }
    else
    {
        CWizStdStringArray arrayPart;
        if (part & WIZKM_XMLRPC_OBJECT_PART_INFO)
        {
            arrayPart.push_back(_TR("information"));
        }
        if (part & WIZKM_XMLRPC_OBJECT_PART_DATA)
        {
            arrayPart.push_back(_TR("data"));
        }
        if (part & WIZKM_XMLRPC_OBJECT_PART_PARAM)
        {
            arrayPart.push_back(_TR("params"));
        }
        //
        CString strParts;
        WizStringArrayToText(arrayPart, strParts, _T(", "));
        //
        QString strInfo;
        strInfo = ::WizFormatString2(_TR("Updating attachment [%2] %1"), local.strName, strParts);
        //
        for (int i = 0; i < 2; i++)	//try twice
        {
            pEvents->OnStatus(strInfo);
            if (server.postData<WIZDOCUMENTATTACHMENTDATAEX>(local, part, nServerVersion))
            {
                succeeded = true;
                break;
            }
            else if (server.GetLastErrorCode() == WIZKM_XMLRPC_ERROR_TRAFFIC_LIMIT)
            {
                QString strMessage = WizFormatString2("Monthly traffic limit reached! \n\nTraffic Limit %1\nTraffic Using:%2",
                                                      ::WizInt64ToStr(kbInfo.nTrafficLimit),
                                                      ::WizInt64ToStr(kbInfo.nTrafficUsage)
                    );
                //
                pDatabase->OnTrafficLimit(strMessage + _T("\n\n") + server.GetLastErrorMessage());
                //
                //pEvents->SetStop(TRUE);
                pEvents->OnTrafficLimit(pDatabase);
                return FALSE;
            }
            else if (server.GetLastErrorCode() == WIZKM_XMLRPC_ERROR_STORAGE_LIMIT)
            {
                QString strMessage = WizFormatString3("Storage limit reached.\n\n%1\nStorage Limit: %2, Storage Using: %3", _T(""),
                    ::WizInt64ToStr(kbInfo.nStorageLimit),
                    ::WizInt64ToStr(kbInfo.nStorageUsage)
                    );
                //
                pDatabase->OnStorageLimit(strMessage + _T("\n\n") + server.GetLastErrorMessage());
                //
                //
                //pEvents->SetStop(TRUE);
                pEvents->OnStorageLimit(pDatabase);
                return FALSE;
            }
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
            WIZDOCUMENTATTACHMENTDATAEX local2 = local;
            InitObjectData<WIZDOCUMENTATTACHMENTDATAEX>(pDatabase, local.strGUID, local2, WIZKM_XMLRPC_OBJECT_PART_INFO);
            //
            COleDateTime tLocalModified2;
            tLocalModified2 = local2.tDataModified;
            //
            if (tLocalModified2 == tLocalModified)
            {
                updateVersion = true;
                pDatabase->SetObjectLocalServerVersion(local.strGUID, strObjectType, nServerVersion);
            }
        }
    }
    //
    //
    double fPos = index / double(total) * size;
    pEvents->OnSyncProgress(start + int(fPos));
    //
    if (!succeeded)
    {
        pEvents->OnWarning(WizFormatString1(_T("Cannot upload attachment data: %1"), local.strName));
        return FALSE;
    }
    //
    if (updateVersion
        && !pDatabase->OnUploadObject(local.strGUID, strObjectType))
    {
        pEvents->OnError(WizFormatString1(_T("Cannot upload local version of attachment: %1!"), local.strName));
        //
        return FALSE;
    }
    //
    return TRUE;
}


template <class TData>
bool UploadObject(const WIZKBINFO& kbInfo, int size, int start, int total, int index, std::map<QString, TData>& mapDataOnServer, TData& local, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, CWizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress)
{
    ATLASSERT(false);
}

template <class TData>
bool UploadObject(const WIZKBINFO& kbInfo, int size, int start, int total, int index, std::map<QString, WIZDOCUMENTDATAEX>& mapDataOnServer, WIZDOCUMENTDATAEX& local, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, CWizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress)
{
    return UploadDocument(kbInfo, size, start, total, index, mapDataOnServer, local, pEvents, pDatabase, server, strObjectType, progress);
}

template <class TData>
bool UploadObject(const WIZKBINFO& kbInfo, int size, int start, int total, int index, std::map<QString, WIZDOCUMENTATTACHMENTDATAEX>& mapDataOnServer, WIZDOCUMENTATTACHMENTDATAEX& local, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, CWizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress)
{
    return UploadAttachment(kbInfo, size, start, total, index, mapDataOnServer, local, pEvents, pDatabase, server, strObjectType, progress);
}


template <class TData, bool _document>
bool UploadList(const WIZKBINFO& kbInfo, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, CWizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress)
{
    if (pDatabase->IsTrafficLimit())
        return FALSE;
    if (pDatabase->IsStorageLimit())
        return FALSE;
    //
    typedef std::deque<TData> TArray;
    TArray arrayData;
    GetModifiedObjectList<TData>(pDatabase, arrayData);
    if (arrayData.empty())
    {
        pEvents->OnStatus(_TR("No change, skip"));
        return TRUE;
    }
    //
    int start = 0;
    int size = 0;
    ::GetSyncProgressRange(progress, start, size);
    pEvents->OnSyncProgress(start);
    //
    int total = int(arrayData.size());
    int index = 0;
    //
    TArray arraySubData;
    CWizStdStringArray arraySubGUID;
    while (!arrayData.empty())
    {
        if (pEvents->IsStop())
            return FALSE;
        //
        TData dataTemp = arrayData.back();
        arrayData.pop_back();
        //
        arraySubData.push_back(dataTemp);
        arraySubGUID.push_back(dataTemp.strGUID);
        //
        if (arraySubData.size() == 50
            || arrayData.empty()
            )
        {
            TArray arrayDataOnServer;
            if (!server.downloadList<TData>(arraySubGUID, arrayDataOnServer))
            {
                pEvents->OnError(_TR("Cannot download object list!"));
                return FALSE;
            }
            //
            std::map<QString, TData> mapDataOnServer;
            for (typename TArray::const_iterator it = arrayDataOnServer.begin();
                it != arrayDataOnServer.end();
                it++)
            {
                mapDataOnServer[it->strGUID] = *it;
            }
            //
            for (typename TArray::const_iterator it = arraySubData.begin();
                it != arraySubData.end();
                it++)
            {
                index++;
                //
                if (pEvents->IsStop())
                    return FALSE;
                //
                TData local = *it;
                //
                if (_document)	//
                {
                    pEvents->OnUploadDocument(local.strGUID, FALSE);
                }
                //
                if (!UploadObject<TData>(kbInfo, size, start, total, index, mapDataOnServer, local, pEvents, pDatabase, server, strObjectType, progress))
                {
                    switch (server.GetLastErrorCode())
                    {
                    case WIZKM_XMLRPC_ERROR_TRAFFIC_LIMIT:
                    case WIZKM_XMLRPC_ERROR_STORAGE_LIMIT:
                        return FALSE;
                    }
                }
                //
                if (_document)	//
                {
                    pEvents->OnUploadDocument(local.strGUID, TRUE);
                }
            }
        }
    }
    //
    return TRUE;
}
bool CWizKMSync::UploadDocumentList()
{
    if (m_bGroup)
    {
        if (!m_pDatabase->IsGroupAuthor())	//need author
            return TRUE;
    }
    //
    return UploadList<WIZDOCUMENTDATAEX, true>(m_kbInfo, m_pEvents, m_pDatabase, m_server, _T("document"), syncUploadDocumentList);
}
bool CWizKMSync::UploadAttachmentList()
{
    if (m_bGroup)
    {
        if (!m_pDatabase->IsGroupAuthor())	//need author
            return TRUE;
    }
    //
    return UploadList<WIZDOCUMENTATTACHMENTDATAEX, false>(m_kbInfo, m_pEvents, m_pDatabase, m_server, _T("attachment"), syncUploadAttachmentList);
}
///////////////////////////////////////////////////////////////////////////////////////////////

bool CWizKMSync::DownloadDeletedList(__int64 nServerVersion)
{
    return DownloadList<WIZDELETEDGUIDDATA>(nServerVersion, _T("deleted_guid"), syncDownloadDeletedList);
}

bool CWizKMSync::DownloadTagList(__int64 nServerVersion)
{
    return DownloadList<WIZTAGDATA>(nServerVersion, _T("tag"), syncDownloadTagList);
}

bool CWizKMSync::DownloadStyleList(__int64 nServerVersion)
{
    return DownloadList<WIZSTYLEDATA>(nServerVersion, _T("style"), syncDownloadStyleList);
}

bool CWizKMSync::DownloadSimpleDocumentList(__int64 nServerVersion)
{
    return DownloadList<WIZDOCUMENTDATAEX>(nServerVersion, _T("document"), syncDownloadSimpleDocumentList);
}


bool CWizKMSync::DownloadFullDocumentList()
{
    if (m_arrayDocumentNeedToBeDownloaded.empty())
        return TRUE;
    //
    const QString& strObjectType = _T("document");
    //
    int start = 0;
    int size = 0;
    ::GetSyncProgressRange(::syncDownloadFullDocumentList, start, size);
    m_pEvents->OnSyncProgress(start);
    //
    int total = int(m_arrayDocumentNeedToBeDownloaded.size());
    //
    CWizStdStringArray arrayDocumentGUID;
    //
    std::map<QString, int> mapDocumentPart;
    //
    for (std::deque<WIZDOCUMENTDATAEX_XMLRPC_SIMPLE>::const_iterator itNeedToBeDownloaded = m_arrayDocumentNeedToBeDownloaded.begin();
        itNeedToBeDownloaded != m_arrayDocumentNeedToBeDownloaded.end();
        itNeedToBeDownloaded++)
    {
        if (m_pEvents->IsStop())
            return FALSE;
        //
        WIZDOCUMENTDATAEX_XMLRPC_SIMPLE simple = *itNeedToBeDownloaded;
        //
        int part = CalDocumentPartForDownloadToLocal(simple);
        if (part & WIZKM_XMLRPC_OBJECT_PART_DATA)
        {
            part &= ~WIZKM_XMLRPC_OBJECT_PART_DATA;
            //
            m_pDatabase->SetObjectDataDownloaded(simple.strGUID, strObjectType, false);	//////设置为未下载//////
            m_pDatabase->SetObjectServerDataInfo(simple.strGUID, strObjectType, simple.tDataModified, simple.strDataMD5);	//////设置成服务器的修改时间和md5，等待下载//////
            m_pDatabase->SetObjectLocalServerVersion(simple.strGUID, strObjectType, simple.nVersion);		//////设置为服务器的版本号//////
        }
        //
        if (0 != part)
        {
            mapDocumentPart[itNeedToBeDownloaded->strGUID] = part;
            arrayDocumentGUID.push_back(itNeedToBeDownloaded->strGUID);
        }
        //
        //
        if (arrayDocumentGUID.size() >= 30
            || itNeedToBeDownloaded == (m_arrayDocumentNeedToBeDownloaded.end() - 1))
        {
            if (!arrayDocumentGUID.empty())
            {
                m_pEvents->OnStatus(_TR(_T("Query notes information")));
                //
                std::deque<WIZDOCUMENTDATAEX> arrayDocument;
                if (!m_server.document_downloadFullList(arrayDocumentGUID, arrayDocument))
                {
                    TOLOG(_T("Can't download note info list!"));
                    return FALSE;
                }
                //
                for (std::deque<WIZDOCUMENTDATAEX>::const_iterator itDocument = arrayDocument.begin();
                    itDocument != arrayDocument.end();
                    itDocument++)
                {
                    m_pEvents->OnStatus(WizFormatString1(_T("Update note information: %1"), itDocument->strTitle));
                    //
                    int nDocumentPart = mapDocumentPart[itDocument->strGUID];
                    if (!m_pDatabase->OnDownloadDocument(nDocumentPart, *itDocument))
                    {
                        m_pEvents->OnError(WizFormatString1(_T("Cannot update note information: %1"), itDocument->strTitle));
                        return FALSE;
                    }
                }
            }
            //
            int index = int(itNeedToBeDownloaded - m_arrayDocumentNeedToBeDownloaded.begin());
            //
            double fPos = index / double(total) * size;
            m_pEvents->OnSyncProgress(start + int(fPos));
            //
            arrayDocumentGUID.clear();
            mapDocumentPart.clear();
        }
    }
    //
    return TRUE;
}


bool CWizKMSync::DownloadAttachmentList(__int64 nServerVersion)
{
    return DownloadList<WIZDOCUMENTATTACHMENTDATAEX>(nServerVersion, _T("attachment"), syncDownloadAttachmentList);
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

bool CWizKMSync::DownloadObjectData()
{
    CWizObjectDataArray arrayObject;
    if (!m_pDatabase->GetObjectsNeedToBeDownloaded(arrayObject))
    {
        m_pEvents->OnError(_T("Cannot get objects need to be downloaded form server!"));
        return FALSE;
    }
    //
    std::sort(arrayObject.begin(), arrayObject.end(), WizCompareObjectByTypeAndTime);
    //
    //
    int start = 0;
    int size = 0;
    ::GetSyncProgressRange(::syncDownloadObjectData, start, size);
    m_pEvents->OnSyncProgress(start);
    //
    int total = int(arrayObject.size());
    //
    size_t succeeded = 0;
    //
    size_t nCount = arrayObject.size();
    for (size_t i = 0; i < nCount; i++)
    {
        if (m_pEvents->IsStop())
            return FALSE;
        //
        WIZOBJECTDATA data = arrayObject[i];
        //
        QString strMsgFormat = data.eObjectType == wizobjectDocument ? _TR("Downloading note: %1"): _TR("Downloading attachment: %1");
        QString strStatus = WizFormatString1(strMsgFormat, data.strDisplayName);
        m_pEvents->OnStatus(strStatus);
        //
        QByteArray stream;
        if (m_server.data_download(data.strObjectGUID, WIZOBJECTDATA::ObjectTypeToTypeString(data.eObjectType), stream, data.strDisplayName))
        {
            if (m_pDatabase->UpdateObjectData(data.strObjectGUID, WIZOBJECTDATA::ObjectTypeToTypeString(data.eObjectType), stream))
            {
                succeeded++;
            }
            else
            {
                m_pEvents->OnError(WizFormatString1(_T("Cannot save object data to local: %1!"), data.strDisplayName));
            }
        }
        else
        {
            m_pEvents->OnError(WizFormatString1(_T("Cannot download object data from server: %1"), data.strDisplayName));
        }
        //
        //
        int index = (int)i;
        //
        double fPos = index / double(total) * size;
        m_pEvents->OnSyncProgress(start + int(fPos));
    }
    //
    return succeeded == nCount;
}


int WizCalDocumentPartForDownloadToLocal(IWizSyncableDatabase* pDatabase, const WIZDOCUMENTDATAEX_XMLRPC_SIMPLE& dataServer)
{
    int nPart = 0;
    //
    WIZDOCUMENTDATA dataLocal;
    if (pDatabase->DocumentFromGUID(dataServer.strGUID, dataLocal))
    {
        if (dataLocal.strInfoMD5 != dataServer.strInfoMD5)
        {
            if (dataLocal.tInfoModified < dataServer.tInfoModified)
            {
                nPart |= WIZKM_XMLRPC_OBJECT_PART_INFO;
            }
            else
            {
                TOLOG2(_T("local and server changed info: local: %1, server :%2\n"), ::WizTimeToSQL(dataLocal.tInfoModified), ::WizTimeToSQL(dataServer.tInfoModified));
            }
        }
        if (dataLocal.strDataMD5 != dataServer.strDataMD5)
        {
            ////不需要解决冲突，在上传的时候，已经解决冲突了////
            nPart |= WIZKM_XMLRPC_OBJECT_PART_DATA;
        }
        if (dataLocal.strParamMD5 != dataServer.strParamMD5)
        {
            if (dataLocal.tParamModified < dataServer.tParamModified)
            {
                nPart |= WIZKM_XMLRPC_OBJECT_PART_PARAM;
            }
            else
            {
                TOLOG2(_T("local and server changed param: local: %1, server :%2\n"), ::WizTimeToSQL(dataLocal.tParamModified), ::WizTimeToSQL(dataServer.tParamModified));
            }
        }
    }
    else
    {
        nPart = WIZKM_XMLRPC_OBJECT_PART_INFO | WIZKM_XMLRPC_OBJECT_PART_DATA | WIZKM_XMLRPC_OBJECT_PART_PARAM;
    }
    //
    return nPart;
}

int CWizKMSync::CalDocumentPartForDownloadToLocal(const WIZDOCUMENTDATAEX_XMLRPC_SIMPLE& dataServer)
{
    return WizCalDocumentPartForDownloadToLocal(m_pDatabase, dataServer);
}


void DownloadAccountKeys(CWizKMAccountsServer& server, IWizSyncableDatabase* pDatabase)
{
    CWizStdStringArray arrayKey;
    pDatabase->GetAccountKeys(arrayKey);
    //
    for (CWizStdStringArray::const_iterator it = arrayKey.begin();
        it != arrayKey.end();
        it++)
    {
        QString strKey = *it;
        //
        __int64 nServerVersion = 0;
        if (!server.GetValueVersion(strKey, nServerVersion))
        {
            TOLOG1(_T("Can't get account value version: %1"), strKey);
            continue;
        }
        //
        if (-1 == nServerVersion)	//not found
            continue;
        //
        __int64 nLocalVersion = pDatabase->GetAccountLocalValueVersion(strKey);
        if (nServerVersion <= nLocalVersion)
            continue;
        //
        QString strServerValue;
        if (!server.GetValue(strKey, strServerValue, nServerVersion))
        {
            continue;
        }
        //
        pDatabase->SetAccountLocalValue(strKey, strServerValue, nServerVersion, FALSE);
    }
}

bool WizDownloadMessages(IWizKMSyncEvents* pEvents, CWizKMAccountsServer& server, IWizSyncableDatabase* pDatabase, const CWizGroupDataArray& arrayGroup)
{
    __int64 nOldVersion = pDatabase->GetObjectVersion(_T("Messages"));
    //
    std::deque<WIZUSERMESSAGEDATA> arrayMessage;
    server.GetMessages(nOldVersion, arrayMessage);
    //
    if (arrayMessage.empty())
        return FALSE;
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
    for (std::deque<WIZUSERMESSAGEDATA>::const_iterator it = arrayMessage.begin();
        it != arrayMessage.end();
        it++)
    {
        if (!it->strKbGUID.isEmpty()
            && !it->strDocumentGUID.isEmpty())
        {
            CWizStdStringArray& documents = mapKbGUIDDocuments[it->strKbGUID];
            documents.push_back(it->strDocumentGUID);
        }
    }
    //
    std::set<QString> setDownloadedDocumentGUID;
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
        IWizSyncableDatabase* pGroupDatabase = pDatabase->GetGroupDatabase(group);
        if (!pGroupDatabase)
        {
            pEvents->OnError(WizFormatString1(_T("Cannot open group: %1"), group.strGroupName));
            continue;
        }
        //
        WIZUSERINFO userInfo = server.m_retLogin;
        //
        if (!group.strDatabaseServer.isEmpty())
        {
            userInfo.strDatabaseServer = group.strDatabaseServer;
        }
        //
        userInfo.strKbGUID = group.strGroupGUID;
        CWizKMDatabaseServer serverDB(userInfo, server.parent());
        //
        pEvents->OnStatus(_TR(_T("Query notes information")));
        //
        std::deque<WIZDOCUMENTDATAEX> arrayDocumentServer;
        if (!serverDB.document_downloadFullListEx(arrayDocumentGUID, arrayDocumentServer))
        {
            pEvents->OnError(_T("Can download notes of messages"));
            return FALSE;
        }
        //
        for (std::deque<WIZDOCUMENTDATAEX>::const_iterator itDocument = arrayDocumentServer.begin();
            itDocument != arrayDocumentServer.end();
            itDocument++)
        {
            WIZDOCUMENTDATAEX documentServer = *itDocument;
            /*
            ////记住下载了哪些笔记////
            */
            setDownloadedDocumentGUID.insert(documentServer.strGUID);

            pEvents->OnStatus(WizFormatString1(_T("Update note information: %1"), itDocument->strTitle));
            //
            UINT nDocumentPart = WIZKM_XMKRPC_DOCUMENT_PART_PARAM | WIZKM_XMKRPC_DOCUMENT_PART_INFO | WIZKM_XMKRPC_DOCUMENT_PART_DATA;
            WIZDOCUMENTDATA dataLocal;
            if (pGroupDatabase->DocumentFromGUID(itDocument->strGUID, dataLocal))
            {
                nDocumentPart = WizCalDocumentPartForDownloadToLocal(pGroupDatabase, documentServer);
                if (nDocumentPart & WIZKM_XMLRPC_OBJECT_PART_DATA)
                {
                    nDocumentPart &= ~WIZKM_XMLRPC_OBJECT_PART_DATA;
                    //
                    static const QString& strObjectType = _T("document");
                    //
                    pGroupDatabase->SetObjectDataDownloaded(documentServer.strGUID, strObjectType, false);	//////设置为未下载//////
                    pGroupDatabase->SetObjectServerDataInfo(documentServer.strGUID, strObjectType, documentServer.tDataModified, documentServer.strDataMD5);	//////设置成服务器的修改时间和md5，等待下载//////
                    pGroupDatabase->SetObjectLocalServerVersion(documentServer.strGUID, strObjectType, documentServer.nVersion);		//////设置为服务器的版本号//////
                }
            }
            //
            if (!pGroupDatabase->OnDownloadDocument(nDocumentPart, *itDocument))
            {
                pEvents->OnError(WizFormatString1(_T("Cannot update note information: %1"), itDocument->strTitle));
                return FALSE;
            }
        }
        //
        pDatabase->CloseGroupDatabase(pGroupDatabase);
    }
    //
    ////
    int count = int(arrayMessage.size());
    for (int i = count - 1; i >= 0; i--)
    {
        const WIZUSERMESSAGEDATA& data = arrayMessage[i];
        if (data.strKbGUID.isEmpty())
            continue;
        if (data.strDocumentGUID.isEmpty())
            continue;
        /*
        ////判断对应的笔记是否被下载了（如果没有，说明服务器已经没有这个笔记了，无法显示这个消息）////
        */
        if (setDownloadedDocumentGUID.find(data.strDocumentGUID) != setDownloadedDocumentGUID.end())
            continue;
        //
        arrayMessage.erase(arrayMessage.begin() + i);
    }
    //
    __int64 nNewVersion = CWizKMSync::GetObjectsVersion<WIZUSERMESSAGEDATA>(nOldVersion, arrayMessage);
    //
    pDatabase->OnDownloadMessages(arrayMessage);
    pDatabase->SetObjectVersion(_T("Messages"), nNewVersion);
    //
    return TRUE;
}

void WizDownloadUserAvatars(IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, bool bBackground)
{
    //
    /*
    ////每天一次，或者用户手工同步////
    */
    //TODO: modify this code
    /*
    if (!::WizDayOnce(WIZKM_REG_KEY_ROOT, _T("DownloadUserAvatars")))
    {
        if (bBackground)
            return;
    }
    */
    //
    //TODO: getavatar url
    //QString strURLAvatar = ::WizKMWebSiteGetReturn(_T("avatar"));
    CString strURLAvatar = "";
    strURLAvatar.Trim();
    //
    if (0 != strURLAvatar.Find(_T("http"))
        || strURLAvatar.GetLength() > 1024)
    {
        return;
    }
    //
    if (pEvents->IsStop())
        return;
    //
    pEvents->OnStatus("Downloading user image");
    //
    CWizStdStringArray arrayPersonalGroupUser;
    //
    /*
    ////普通群组头像不需要主动下载，额可以在阅读的时候下载////
    //db.GetAllGroupUserIds(arrayPersonalGroupUser);		////
    */
    //
    CWizStdStringArray arrayBizGroupUser;
    pDatabase->GetAllBizUserIds(arrayBizGroupUser);
    //
    QString strCurrentUserID = pDatabase->GetUserId();
    //
    CWizStdStringArray arrayAllUserID;
    arrayAllUserID.push_back(strCurrentUserID);
    arrayAllUserID.insert(arrayAllUserID.begin(), arrayPersonalGroupUser.begin(), arrayPersonalGroupUser.end());
    arrayAllUserID.insert(arrayAllUserID.begin(), arrayBizGroupUser.begin(), arrayBizGroupUser.end());
    //
    std::set<QString> downloaded;
    //
    COleDateTime tNow = ::WizGetCurrentTime();
    //
    /*
    QString strSettingsFileName = ::WizKMGetAvatarsPath() + _T("settings.ini");
    CWizIniFileEx settings;
    settings.LoadFromFile(strSettingsFileName);

    for (CWizStdStringArray::const_iterator it = arrayAllUserID.begin();
        it != arrayAllUserID.end();
        it++)
    {
        QString strUserId = *it;
        //
        if (downloaded.find(strUserId) != downloaded.end())
            continue;
        //
        if (pEvents->IsStop())
            break;
        //
        downloaded.insert(strUserId);
        //
        QString strURL(strURLAvatar);
        strURL.Replace(_T("{userGuid}"), strUserId);
        //
        QString strFileName = ::WizKMGetAvatarsPath() + strUserId + _T(".png");
        //
        if (strUserId != strCurrentUserID)
        {
            if (PathFileExists(strFileName))
            {
                COleDateTimeSpan ts = tNow - WizGetFileModifiedTime(strFileName);
                if (ts.GetDays() <= 7)
                    continue;
                //////不需要更新////
            }
            else
            {
                QString strKey = strUserId;
                const QString& strUserImageSection = _T("UserImage");
                QString strTime = settings.GetStringDef(strUserImageSection, strKey);
                if (!strTime.isEmpty())
                {
                    COleDateTimeSpan ts = tNow - ::WizStringToDateTime(strTime);
                    if (ts.GetDays() <= 7)
                        continue;
                }
                //
                settings.SetString(strUserImageSection, strKey, ::WizDateTimeToString(tNow));
            }
        }
        //
        QString strLeft;
        QString strRight;
        ::WizStringSimpleSplit(strUserId, '@', strLeft, strRight);
        pEvents->OnStatus(strLeft);
        //
        if (SUCCEEDED(URLDownloadToFile(NULL, strURL, strFileName, 0, NULL)))
        {
            if (strUserId == strCurrentUserID)
            {
                pEvents->OnSyncStep(wizsyncstepUserAvatarDownloaded, 0);
            }
        }
    }
    //
    settings.SaveToUnicodeFile(strSettingsFileName);
    */
}
//

QString downloadFromUrl(const QString& strUrl)
{
    QNetworkAccessManager net;
    QNetworkReply* reply = net.get(QNetworkRequest(strUrl));

    QEventLoop loop;
    loop.connect(reply, SIGNAL(finished()), SLOT(quit()));
    loop.exec();

    if (reply->error()) {
        return NULL;
    }

    return QString::fromUtf8(reply->readAll().constData());
}

void syncGroupUsers(CWizKMAccountsServer& server, const CWizGroupDataArray& arrayGroup,
                    IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, bool background)
{
    QString strt = pDatabase->meta("SYNC_INFO", "DownloadGroupUsers");
    if (!strt.isEmpty()) {
        if (QDateTime::fromString(strt).addDays(1) > QDateTime::currentDateTime()) {
            if (background) {
                return;
            }
        }
    }

    pEvents->OnStatus("Sync group users");

    for (CWizGroupDataArray::const_iterator it = arrayGroup.begin();
         it != arrayGroup.end();
         it++)
    {
        const WIZGROUPDATA& g = *it;
        if (!g.bizGUID.isEmpty()) {
            QString strUrl = WizService::ApiEntry::groupUsersUrl(server.GetToken(), g.bizGUID, g.strGroupGUID);
            QString strJsonRaw = downloadFromUrl(strUrl);

            if (!strJsonRaw.isEmpty())
                pDatabase->setBizGroupUsers(g.strGroupGUID, strJsonRaw);
        }

        if (pEvents->IsStop())
            return;
    }

    pDatabase->setMeta("SYNC_INFO", "DownloadGroupUsers", QDateTime::currentDateTime().toString());
}

bool WizSyncDatabase(const WIZUSERINFO& info, IWizKMSyncEvents* pEvents,
                     IWizSyncableDatabase* pDatabase,
                     bool bUseWizServer, bool bBackground)
{
    Q_UNUSED(bUseWizServer);

    pEvents->OnStatus(_TR("-------Sync start--------------"));
    pEvents->OnSyncProgress(0);
    pEvents->OnStatus(_TR("Connecting to server"));

    CWizKMAccountsServer server(WizService::ApiEntry::syncUrl());
    server.SetUserInfo(info);

    pEvents->OnSyncProgress(::GetSyncStartProgress(syncAccountLogin));
    pEvents->OnStatus(_TR("Signing in"));

    //QString strPassword = pDatabase->GetPassword();
    //while (1)
    //{
    //    if (server.Login(pDatabase->GetUserId(), strPassword, _T("normal")))
    //        break;

    //    pEvents->SetLastErrorCode(server.GetLastErrorCode());
    //    pEvents->OnError(server.GetLastErrorMessage());

    //    return false;
    //}

    pDatabase->SetUserInfo(server.GetUserInfo());
    pEvents->OnSyncProgress(1);

    /*
    ////获得群组信息////
    */
    //
    CWizBizDataArray arrayBiz;
    if (server.GetBizList(arrayBiz))
    {
        pDatabase->OnDownloadBizs(arrayBiz);
    }

    //
    CWizGroupDataArray arrayGroup;
    if (server.GetGroupList(arrayGroup))
    {
        pDatabase->OnDownloadGroups(arrayGroup);
    }
    //
    int groupCount = int(arrayGroup.size());
    pEvents->SetDatabaseCount(groupCount + 1);
    //
    /*
    ////下载设置////
    */
    pEvents->OnStatus(_TR("Downloading settings"));
    DownloadAccountKeys(server, pDatabase);

    syncGroupUsers(server, arrayGroup, pEvents, pDatabase, bBackground);

    /*
    ////下载消息////
    */
    pEvents->OnStatus(_TR("Downloading messages"));
    WizDownloadMessages(pEvents, server, pDatabase, arrayGroup);
    //
    //
    pEvents->OnStatus(_TR("-------sync private notes--------------"));
    //
    {
        pEvents->SetCurrentDatabase(0);
        CWizKMSync syncPrivate(pDatabase, server.m_retLogin, pEvents, FALSE, FALSE, NULL);
        //
        if (!syncPrivate.Sync())
        {
            pEvents->OnText(wizhttpstatustypeError, _T("Cannot sync!"));
        }
        else
        {
            pDatabase->SaveLastSyncTime();
            pEvents->OnSyncProgress(100);
        }
    }
    //
    if (pEvents->IsStop())
        return FALSE;

    pEvents->OnStatus(_TR("-------sync groups--------------"));
    //
    for (CWizGroupDataArray::const_iterator itGroup = arrayGroup.begin();
        itGroup != arrayGroup.end();
        itGroup++)
    {
        pEvents->SetCurrentDatabase(1 + int(itGroup - arrayGroup.begin()));
        //
        if (pEvents->IsStop())
            return FALSE;
        //
        WIZGROUPDATA group = *itGroup;
        //
        pEvents->OnSyncProgress(0);
        pEvents->OnStatus(WizFormatString1(_TR("-------Sync group: %1--------------"), group.strGroupName));
        //
        IWizSyncableDatabase* pGroupDatabase = pDatabase->GetGroupDatabase(group);
        if (!pGroupDatabase)
        {
            pEvents->OnError(WizFormatString1(_T("Cannot open group: %1"), group.strGroupName));
            continue;
        }
        //
        WIZUSERINFO userInfo = server.m_retLogin;
        userInfo.strDatabaseServer = group.strDatabaseServer;
        userInfo.strKbGUID = group.strGroupGUID;
        //
        //
        CWizKMSync syncGroup(pGroupDatabase, userInfo, pEvents, TRUE, FALSE, NULL);
        //
        if (syncGroup.Sync())
        {
            pGroupDatabase->SaveLastSyncTime();
            pEvents->OnStatus(WizFormatString1(_TR("Sync group %1 done"), group.strGroupName));
        }
        else
        {
            pEvents->OnError(WizFormatString1(_TR("Cannot sync group %1"), group.strGroupName));
            pEvents->OnSyncProgress(100);
        }
        //
        pDatabase->CloseGroupDatabase(pGroupDatabase);
    }
    //
    //
    WizDownloadUserAvatars(pEvents, pDatabase, bBackground);
    //
    pEvents->OnStatus(_TR("-------Downloading notes--------------"));
    //
    {
        //pEvents->SetCurrentDatabase(0);
        CWizKMSync syncPrivate(pDatabase, server.m_retLogin, pEvents, FALSE, FALSE, NULL);
        //
        if (!syncPrivate.DownloadObjectData())
        {
            pEvents->OnText(wizhttpstatustypeError, _T("Cannot sync!"));
        }
        else
        {
            pDatabase->SaveLastSyncTime();
            //pEvents->OnSyncProgress(100);
        }
    }
    //
    if (pEvents->IsStop())
        return FALSE;
    //
    for (CWizGroupDataArray::const_iterator itGroup = arrayGroup.begin();
        itGroup != arrayGroup.end();
        itGroup++)
    {
        pEvents->SetCurrentDatabase(1 + int(itGroup - arrayGroup.begin()));
        //
        if (pEvents->IsStop())
            return FALSE;
        //
        WIZGROUPDATA group = *itGroup;
        //
        //pEvents->OnSyncProgress(0);
        //pEvents->OnStatus(WizFormatString1(_TR("-------Sync group: %1--------------"), group.strGroupName));
        //
        IWizSyncableDatabase* pGroupDatabase = pDatabase->GetGroupDatabase(group);
        if (!pGroupDatabase)
        {
            pEvents->OnError(WizFormatString1(_T("Cannot open group: %1"), group.strGroupName));
            continue;
        }
        //
        WIZUSERINFO userInfo = server.m_retLogin;
        userInfo.strDatabaseServer = group.strDatabaseServer;
        userInfo.strKbGUID = group.strGroupGUID;
        //
        //
        CWizKMSync syncGroup(pGroupDatabase, userInfo, pEvents, TRUE, FALSE, NULL);
        //
        if (syncGroup.DownloadObjectData())
        {
            pGroupDatabase->SaveLastSyncTime();
            //pEvents->OnStatus(WizFormatString1(_TR("Sync group %1 done"), group.strGroupName));
        }
        else
        {
            pEvents->OnError(WizFormatString1(_TR("Cannot sync group %1"), group.strGroupName));
            //pEvents->OnSyncProgress(100);
        }
        //
        pDatabase->CloseGroupDatabase(pGroupDatabase);
    }
    //
    pEvents->OnStatus(_TR("-------Sync done--------------"));
    //
    return TRUE;
}



bool WizUploadDatabase(IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, const WIZUSERINFOBASE& info, bool bGroup)
{
    CWizKMSync sync(pDatabase, info, pEvents, bGroup, TRUE, NULL);
    bool bRet = sync.Sync();
    //
    return bRet;
}
bool WizSyncDatabaseOnly(IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, const WIZUSERINFOBASE& info, bool bGroup)
{
    CWizKMSync sync(pDatabase, info, pEvents, bGroup, FALSE, NULL);
    bool bRet = sync.Sync();
    if (bRet)
    {
        sync.DownloadObjectData();
    }
    //
    return bRet;
}
