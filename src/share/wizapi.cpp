#include "wizapi.h"
#include <QFile>
#include "zip/wizzip.h"
#include "wizsettings.h"

#define PARTSIZE 10*1024
#define MD5PART 10*1024
#define CONSTDEFAULTCOUNT 200

CWizApiBase::CWizApiBase(const CString& strAccountsApiURL, CWizSyncEvents& events)
    : m_server(strAccountsApiURL)
    , m_events(events)
{
    resetProxy();

    connect(&m_server, SIGNAL(xmlRpcReturn(const CString&, CWizXmlRpcValue&)), this, SLOT(xmlRpcReturn(const CString&, CWizXmlRpcValue&)));
    connect(&m_server, SIGNAL(xmlRpcError(const CString&, WizXmlRpcError, int, const CString&)), this, SLOT(xmlRpcError(const CString&, WizXmlRpcError, int, const CString&)));
}

void CWizApiBase::abort()
{
    m_server.disconnect(this);
    m_server.abort();
}


BOOL CWizApiBase::callXmlRpc(const CString& strMethodName, CWizXmlRpcValue* pVal)
{
    addDebugLog("call xmlrpc:[" + strMethodName + "]");
    //
    return m_server.xmlRpcCall(strMethodName, pVal);
}

void CWizApiBase::xmlRpcReturn(const CString& strMethodName, CWizXmlRpcValue& ret)
{
    addDebugLog("xmlrpc done:[" + strMethodName + "]");
    //
    onXmlRpcReturn(strMethodName, ret);
}

void CWizApiBase::xmlRpcError(const CString& strMethodName, WizXmlRpcError err, int errorCode, const CString& errorMessage)
{
    addErrorLog("Error: [" + strMethodName + "]: " + errorMessage);
    //
    onXmlRpcError(strMethodName, err, errorCode, errorMessage);
}

void CWizApiBase::onXmlRpcReturn(const CString& strMethodName, CWizXmlRpcValue& ret)
{
    if (strMethodName == SyncMethod_ClientLogin)
    {
        ret.ToData<WIZUSERINFO>(m_user);
        onClientLogin();
    }
    else if (strMethodName == SyncMethod_ClientLogout)
    {
        onClientLogout();
    }
    else if (strMethodName == SyncMethod_CreateAccount)
    {
        onCreateAccount();
    }
    else if (strMethodName == SyncMethod_GetUserInfo)
    {
        onGetUserInfo(ret);
    }
    else
    {
        ATLASSERT(FALSE);
    }
}

void CWizApiBase::onXmlRpcError(const CString& strMethodName, WizXmlRpcError err, int errorCode, const CString& errorMessage)
{
    Q_UNUSED(strMethodName);
    Q_UNUSED(err);
    Q_UNUSED(errorCode);
    Q_UNUSED(errorMessage);
}

bool CWizApiBase::isSyncing() const
{
    return m_server.currentId() != 0;
}

void CWizApiBase::resetProxy()
{
    CWizSettings settings(WizGetSettingsFileName());

    QString host = settings.GetProxyHost();
    int port = settings.GetProxyPort();
    QString userName = settings.GetProxyUserName();
    QString password = settings.GetProxyPassword();
    //
    m_server.setProxy(host, port, userName, password);
}

CString CWizApiBase::MakeXmlRpcUserId(const CString& strUserId)
{
    return strUserId;
}

CString CWizApiBase::MakeXmlRpcPassword(const CString& strPassword)
{
    return "md5." + ::WizMd5StringNoSpaceJava(strPassword.toUtf8());
    //return strPassword;
}

BOOL CWizApiBase::callClientLogin(const CString& strUserId, const CString& strPassword)
{
    m_user = WIZUSERINFO();

    CWizApiParamBase param;
    param.AddString("user_id", MakeXmlRpcUserId(strUserId));
    param.AddString("password", MakeXmlRpcPassword(strPassword));

    return callXmlRpc(SyncMethod_ClientLogin, &param);
}

void CWizApiBase::onClientLogin()
{
}
//
BOOL CWizApiBase::callClientLogout()
{
    if (m_user.strToken.isEmpty())
        return FALSE;
    //
    CWizApiTokenParam param(*this);
    //
    return callXmlRpc(SyncMethod_ClientLogout, &param);
}

void CWizApiBase::onClientLogout()
{
    m_user.strToken.Empty();
    m_user.strKbGUID.Empty();
}
//
BOOL CWizApiBase::callGetUserInfo()
{
    return TRUE;
}

void CWizApiBase::onGetUserInfo(CWizXmlRpcValue& ret)
{
    Q_UNUSED(ret);
}

BOOL CWizApiBase::callCreateAccount(const CString& strUserId, const CString& strPassword)
{
    CWizApiParamBase param;
    param.AddString("user_id", MakeXmlRpcUserId(strUserId));
    param.AddString("password", MakeXmlRpcPassword(strPassword));
#if defined Q_OS_MAC
    param.AddString("invite_code", "129ce11c");
    param.AddString("product_name", "qtMac");
#elif defined Q_OS_LINUX
    param.AddString("invite_code", "7abd8f4a");
    param.AddString("product_name", "qtLinux");
#else
    param.AddString("invite_code", "8480c6d7");
    param.AddString("product_name", "qtWindows");
#endif
    //
    return callXmlRpc(SyncMethod_CreateAccount, &param);
}

void CWizApiBase::onCreateAccount()
{

}

void CWizApiBase::changeProgress(int pos)
{
    m_events.changeProgress(pos);
}

void CWizApiBase::addLog(const CString& str)
{
    m_events.addLog(str);
}

void CWizApiBase::addDebugLog(const CString& str)
{
    m_events.addDebugLog(str);
}

void CWizApiBase::addErrorLog(const CString& str)
{
    m_events.addErrorLog(str);
}


///////////////////////////////////////////////////////////////

CWizApi::CWizApi(CWizDatabase& db, const CString& strAccountsApiURL, CWizSyncEvents& events)
    : CWizApiBase(strAccountsApiURL, events)
    , m_db(db)
    , m_nCurrentObjectAllSize(0)
    , m_bDownloadingObject(false)
{
}

void CWizApi::onXmlRpcReturn(const CString& strMethodName, CWizXmlRpcValue& ret)
{
    if (strMethodName == SyncMethod_GetDeletedList)
    {
        std::deque<WIZDELETEDGUIDDATA> arrayData;
        ret.ToArray<WIZDELETEDGUIDDATA>(arrayData);
        onDeletedGetList(arrayData);
    }
    else if (strMethodName == SyncMethod_GetTagList)
    {
        std::deque<WIZTAGDATA> arrayData;
        ret.ToArray<WIZTAGDATA>(arrayData);
        onTagGetList(arrayData);
    }
    else if (strMethodName == SyncMethod_GetStyleList)
    {
        std::deque<WIZSTYLEDATA> arrayData;
        ret.ToArray<WIZSTYLEDATA>(arrayData);
        onStyleGetList(arrayData);
    }
    else if (strMethodName == SyncMethod_GetDocumentList)
    {
        std::deque<WIZDOCUMENTDATABASE> arrayData;
        ret.ToArray<WIZDOCUMENTDATABASE>(arrayData);
        onDocumentGetList(arrayData);
    }
    else if (strMethodName == SyncMethod_GetAttachmentList)
    {
        std::deque<WIZDOCUMENTATTACHMENTDATAEX> arrayData;
        ret.ToArray<WIZDOCUMENTATTACHMENTDATAEX>(arrayData);
        onAttachmentGetList(arrayData);
    }
    else if (strMethodName == SyncMethod_GetDocumentData)
    {
        WIZDOCUMENTDATAEX data;
        ret.ToData(data);
        onDocumentGetData(data);
    }
    else if (strMethodName == SyncMethod_GetDocumentsInfo)
    {
        std::deque<WIZDOCUMENTDATABASE> arrayData;
        ret.ToArray<WIZDOCUMENTDATABASE>(arrayData);
        onDocumentsGetInfo(arrayData);
    }
    else if (strMethodName == SyncMethod_GetAttachmentsInfo)
    {
        std::deque<WIZDOCUMENTATTACHMENTDATAEX> arrayData;
        ret.ToArray<WIZDOCUMENTATTACHMENTDATAEX>(arrayData);
        onAttachmentsGetInfo(arrayData);
    }
    else if (strMethodName == SyncMethod_DownloadObjectPart)
    {
        WIZOBJECTPARTDATA data = m_currentObjectPartData;
        ret.ToData(data);
        onDownloadDataPart(data);
    }
    else if (strMethodName == SyncMethod_UploadObjectPart)
    {
        onUploadDataPart();
    }
    else if (strMethodName == SyncMethod_PostDeletedList)
    {
        onDeletedPostList(m_arrayCurrentPostDeletedGUID);
    }
    else if (strMethodName == SyncMethod_PostTagList)
    {
        onTagPostList(m_arrayCurrentPostTag);
    }
    else if (strMethodName == SyncMethod_PostStyleList)
    {
        onStylePostList(m_arrayCurrentPostStyle);
    }
    else if (strMethodName == SyncMethod_PostDocumentData)
    {
        onDocumentPostData(m_currentDocument);
    }
    else if (strMethodName == SyncMethod_PostAttachmentData)
    {
        onAttachmentPostData(m_currentAttachment);
    }
    else
    {
        CWizApiBase::onXmlRpcReturn(strMethodName, ret);
    }
}


BOOL CWizApi::callGetList(const CString& strMethodName, __int64 nVersion)
{
    CWizApiTokenParam param(*this);
    param.AddInt(_T("count"), getCountPerPage());
    param.AddString(_T("version"), WizInt64ToStr(nVersion));
    //
    return callXmlRpc(strMethodName, &param);
}


BOOL CWizApi::callDeletedGetList(__int64 nVersion)
{
    m_events.addLog(WizFormatString1("Syncing deleted items: [%1]", WizInt64ToStr(nVersion)));
    return callGetList(SyncMethod_GetDeletedList, nVersion);
}

void CWizApi::onDeletedGetList(const std::deque<WIZDELETEDGUIDDATA>& arrayRet)
{
    m_db.UpdateDeletedGUIDs(arrayRet, &m_events);
}

BOOL CWizApi::callTagGetList(__int64 nVersion)
{
    m_events.addLog(WizFormatString1("Syncing tags: [%1]", WizInt64ToStr(nVersion)));
    return callGetList(SyncMethod_GetTagList, nVersion);
}


void CWizApi::onTagGetList(const std::deque<WIZTAGDATA>& arrayRet)
{
    m_db.UpdateTags(arrayRet, &m_events);
}

BOOL CWizApi::callStyleGetList(__int64 nVersion)
{
    m_events.addLog(WizFormatString1("Syncing styles: [%1]", WizInt64ToStr(nVersion)));
    return callGetList(SyncMethod_GetStyleList, nVersion);
}

void CWizApi::onStyleGetList(const std::deque<WIZSTYLEDATA>& arrayRet)
{
    m_db.UpdateStyles(arrayRet, &m_events);
}

BOOL CWizApi::callDocumentGetList(__int64 nVersion)
{
    m_events.addLog(WizFormatString1("Syncing note list: [%1]", WizInt64ToStr(nVersion)));
    return callGetList(SyncMethod_GetDocumentList, nVersion);
}

void CWizApi::onDocumentGetList(const std::deque<WIZDOCUMENTDATABASE>& arrayRet)
{
    Q_UNUSED(arrayRet);
}

BOOL CWizApi::callAttachmentGetList(__int64 nVersion)
{
    m_events.addLog(WizFormatString1("Syncing attachment list: [%1]", WizInt64ToStr(nVersion)));
    return callGetList(SyncMethod_GetAttachmentList, nVersion);
}

void CWizApi::onAttachmentGetList(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
{
    m_db.UpdateAttachments(arrayRet, &m_events);
}

BOOL CWizApi::callDownloadDataPart(const CString& strObjectGUID, const CString& strObjectType, int pos)
{
    m_bDownloadingObject = true;

    unsigned int size = getPartSize();
    //
    m_currentObjectPartData.strObjectGUID = strObjectGUID;
    m_currentObjectPartData.strObjectType = strObjectType;
    m_currentObjectPartData.nStartPos = pos;
    m_currentObjectPartData.nQuerySize = size;
    //
    CWizApiTokenParam param(*this);
    //
    param.AddString(_T("obj_guid"), strObjectGUID);
    param.AddString(_T("obj_type"), strObjectType);
    //
    param.AddInt64(_T("start_pos"), pos);
    param.AddInt64(_T("part_size"), size);
    //
    return callXmlRpc(SyncMethod_DownloadObjectPart, &param);
}

void CWizApi::onDownloadDataPart(const WIZOBJECTPARTDATA& data)
{
    m_bDownloadingObject = false;

    m_currentObjectData.arrayData.append(data.arrayData);
    //
    double fPercent = 0;
    if (data.nObjectSize > 0)
        fPercent = 100.0 * m_currentObjectData.arrayData.size() / double(data.nObjectSize);
    //
    m_nCurrentObjectAllSize = data.nObjectSize;
    //
    //
    m_events.addLog(WizFormatString2("Downloading %1 [%2]", m_currentObjectData.strDisplayName, WizIntToStr(int(fPercent)) + "%"));
    //
    m_events.changeObjectDataProgress(int(fPercent));
    //
    if (data.bEOF)
    {
        onDownloadObjectDataCompleted(m_currentObjectData);
    }
    else
    {
        downloadNextPartData();
    }
}

BOOL CWizApi::callUploadDataPart(const CString& strObjectGUID, const CString& strObjectType, const CString& strObjectMD5, int allSize, int partCount, int partIndex, int partSize, const QByteArray& arrayData)
{
    Q_UNUSED(allSize);
    //
    ATLASSERT(partSize == arrayData.size());
    //
    CWizApiTokenParam param(*this);
    param.AddString(_T("obj_guid"), strObjectGUID);
    param.AddString(_T("obj_type"), strObjectType);
    param.AddString(_T("obj_md5"), strObjectMD5);
    param.AddInt(_T("part_count"), partCount);
    param.AddInt(_T("part_sn"), partIndex);
    param.AddInt64(_T("part_size"), partSize);
    param.AddString(_T("part_md5"), ::WizMd5StringNoSpaceJava(arrayData));
    param.AddBase64(_T("data"), arrayData);
    //
    return callXmlRpc(SyncMethod_UploadObjectPart, &param);
}

void CWizApi::onUploadDataPart()
{
    if (m_currentObjectData.arrayData.isEmpty())
    {
        onUploadObjectDataCompleted(m_currentObjectData);
    }
    else
    {
        uploadNextPartData();
    }
}

BOOL CWizApi::uploadObjectData(const WIZOBJECTDATA& data)
{
    if (data.eObjectType == wizobjectDocument)
    {
        addLog("uploading note: " + data.strDisplayName);
    }
    else if (data.eObjectType == wizobjectDocumentAttachment)
    {
        addLog("uploading attachment: " + data.strDisplayName);
    }
    else
    {
        ATLASSERT(FALSE);
    }
    //
    ATLASSERT(!data.arrayData.isEmpty());
    //
    m_currentObjectData = data;
    m_nCurrentObjectAllSize = data.arrayData.size();
    m_strCurrentObjectMD5 = ::WizMd5StringNoSpaceJava(m_currentObjectData.arrayData);
    //
    return uploadNextPartData();
}

void CWizApi::onUploadObjectDataCompleted(const WIZOBJECTDATA& data)
{
    if (data.eObjectType == wizobjectDocument)
    {
        ATLASSERT(data.strObjectGUID == m_currentDocument.strGUID);
        ATLASSERT(m_currentDocument.nObjectPart & WIZKM_XMLRPC_OBJECT_PART_DATA);
        callDocumentPostData(m_currentDocument);
    }
    else if (data.eObjectType == wizobjectDocumentAttachment)
    {
        ATLASSERT(data.strObjectGUID == m_currentAttachment.strGUID);
        ATLASSERT(m_currentAttachment.nObjectPart & WIZKM_XMLRPC_OBJECT_PART_DATA);
        callAttachmentPostData(m_currentAttachment);
    }
    else
    {
        ATLASSERT(FALSE);
    }
}

BOOL CWizApi::callDocumentGetData(const WIZDOCUMENTDATABASE& data)
{
    addLog("getting note info: " + data.strTitle);
    //
    int nPart = data.nObjectPart;
    ATLASSERT(nPart != 0);
    //
    CWizApiTokenParam param(*this);
    param.AddString(_T("document_guid"), data.strGUID);
    param.AddBool(_T("document_info"), (nPart & WIZKM_XMKRPC_DOCUMENT_PART_INFO) ? TRUE : FALSE);
    param.AddBool(_T("document_param"), (nPart & WIZKM_XMKRPC_DOCUMENT_PART_PARAM) ? TRUE : FALSE);
    //
    return callXmlRpc(SyncMethod_GetDocumentData, &param);
}

void CWizApi::onDocumentGetData(const WIZDOCUMENTDATAEX& data)
{
    m_db.UpdateDocument(data, &m_events);
}

BOOL CWizApi::uploadDocument(const WIZDOCUMENTDATAEX& data)
{
    m_currentDocument = data;
    //
    int nParts = m_currentDocument.nObjectPart;
    ATLASSERT(0 != nParts);
    BOOL bData = (nParts & WIZKM_XMKRPC_DOCUMENT_PART_DATA) ? TRUE : FALSE;
    //
    if (bData)
    {
        if (!m_db.LoadDocumentData(m_currentDocument.strGUID, m_currentDocument.arrayData))
        {
            //skip this document
            addErrorLog("Can not load document data: " + data.strTitle);
            onUploadDocument(data);   //skip
            return FALSE;
        }
        //
        WIZOBJECTDATA obj;
        obj.strObjectGUID = m_currentDocument.strGUID;
        obj.strDisplayName = m_currentDocument.strTitle;
        obj.eObjectType = wizobjectDocument;
        obj.arrayData = m_currentDocument.arrayData;
        //
        return uploadObjectData(obj);
    }
    else
    {
        return callDocumentPostData(m_currentDocument);
    }
}

void CWizApi::onUploadDocument(const WIZDOCUMENTDATAEX& data)
{
    Q_UNUSED(data);
}

BOOL CWizApi::callDocumentPostData(const WIZDOCUMENTDATAEX& data)
{
    m_currentDocument = data;
    //
    int nParts = m_currentDocument.nObjectPart;
    ATLASSERT(0 != nParts);
    BOOL bInfo = (nParts & WIZKM_XMKRPC_DOCUMENT_PART_INFO) ? TRUE : FALSE;
    BOOL bData = (nParts & WIZKM_XMKRPC_DOCUMENT_PART_DATA) ? TRUE : FALSE;
    BOOL bParam = (nParts & WIZKM_XMKRPC_DOCUMENT_PART_PARAM) ? TRUE : FALSE;
    //
    addLog("update note info:" + data.strTitle);
    //
    CWizApiTokenParam param(*this);
    //
    CWizXmlRpcStructValue* pDocumentStruct = new CWizXmlRpcStructValue();
    param.AddStruct(_T("document"), pDocumentStruct);
    //
    pDocumentStruct->AddString(_T("document_guid"), data.strGUID);
    pDocumentStruct->AddBool(_T("document_info"), bInfo ? TRUE : FALSE);
    pDocumentStruct->AddBool(_T("document_data"), bData ? TRUE : FALSE);
    pDocumentStruct->AddBool(_T("document_param"), bParam ? TRUE : FALSE);
    //
    BOOL bParamInfoAdded = FALSE;
    BOOL bDataInfoAdded = FALSE;
    //
    WIZDOCUMENTDATAEX& infodata = m_currentDocument;
    //
    if (bInfo)
    {
        pDocumentStruct->AddString(_T("document_title"), infodata.strTitle);
        pDocumentStruct->AddString(_T("document_category"), infodata.strLocation);
        pDocumentStruct->AddString(_T("document_filename"), infodata.strName);
        pDocumentStruct->AddString(_T("document_seo"), infodata.strSEO);
        pDocumentStruct->AddString(_T("document_url"), infodata.strURL);
        pDocumentStruct->AddString(_T("document_author"), infodata.strAuthor);
        pDocumentStruct->AddString(_T("document_keywords"), infodata.strKeywords);
        pDocumentStruct->AddString(_T("document_type"), infodata.strType);
        pDocumentStruct->AddString(_T("document_owner"), infodata.strOwner);
        pDocumentStruct->AddString(_T("document_filetype"), infodata.strFileType);
        pDocumentStruct->AddString(_T("document_styleguid"), infodata.strStyleGUID);
        pDocumentStruct->AddTime(_T("dt_created"), infodata.tCreated);
        pDocumentStruct->AddTime(_T("dt_modified"), infodata.tModified);
        pDocumentStruct->AddTime(_T("dt_accessed"), infodata.tAccessed);
        pDocumentStruct->AddInt(_T("document_iconindex"), infodata.nIconIndex);
        pDocumentStruct->AddInt(_T("document_protected"), infodata.nProtected);
        pDocumentStruct->AddInt(_T("document_readcount"), infodata.nReadCount);
        pDocumentStruct->AddInt(_T("document_attachment_count"), infodata.nAttachmentCount);
        pDocumentStruct->AddTime(_T("dt_info_modified"), infodata.tInfoModified);
        pDocumentStruct->AddString(_T("info_md5"), infodata.strInfoMD5);
        pDocumentStruct->AddTime(_T("dt_data_modified"), infodata.tDataModified);
        pDocumentStruct->AddString(_T("data_md5"), infodata.strDataMD5);
        pDocumentStruct->AddTime(_T("dt_param_modified"), infodata.tParamModified);
        pDocumentStruct->AddString(_T("param_md5"), infodata.strParamMD5);
        pDocumentStruct->AddString(_T("system_tags"), infodata.strSystemTags);
        pDocumentStruct->AddInt(_T("document_share"), infodata.nShareFlags);
        //
        bParamInfoAdded = TRUE;
        bDataInfoAdded = TRUE;
        //
        m_db.GetDocumentTags(infodata.strGUID, infodata.arrayTagGUID);
        //
        pDocumentStruct->AddStringArray(_T("document_tags"), data.arrayTagGUID);
    }
    if (bParam)
    {
        m_db.GetDocumentParams(infodata.strGUID, infodata.arrayParam);
        //
        if (!bParamInfoAdded)
        {
            pDocumentStruct->AddTime(_T("dt_param_modified"), infodata.tParamModified);
            pDocumentStruct->AddString(_T("param_md5"), infodata.strParamMD5);
            bParamInfoAdded = TRUE;
        }
        //
        pDocumentStruct->AddArray(_T("document_params"), infodata.arrayParam);
    }
    if (bData)
    {
        if (!m_db.LoadDocumentData(m_currentDocument.strGUID, m_currentDocument.arrayData))
        {
            //skip this document
            addErrorLog("Can not load document data: " + data.strTitle);
            onUploadObjectDataCompleted(data);
            return FALSE;
        }
        //
        if (!bDataInfoAdded)
        {
            pDocumentStruct->AddTime(_T("dt_data_modified"), infodata.tDataModified);
            pDocumentStruct->AddString(_T("data_md5"), infodata.strDataMD5);
            bDataInfoAdded = TRUE;
        }
        //
        pDocumentStruct->AddString(_T("document_zip_md5"), WizMd5StringNoSpaceJava(infodata.arrayData));
    }
    return callXmlRpc(SyncMethod_PostDocumentData, &param);
}

void CWizApi::onDocumentPostData(const WIZDOCUMENTDATAEX& data)
{
    ATLASSERT(data.strGUID == m_currentDocument.strGUID);
    onUploadDocument(m_currentDocument);
}

BOOL CWizApi::uploadAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data)
{
    m_currentAttachment = data;
    //
    int nParts = data.nObjectPart;
    ATLASSERT(0 != nParts);
    BOOL bData = (nParts & WIZKM_XMKRPC_ATTACHMENT_PART_DATA) ? TRUE : FALSE;
    //
    if (bData)
    {
        if (!m_db.LoadCompressedAttachmentData(m_currentAttachment.strGUID, m_currentAttachment.arrayData))
        {
            //skip this document
            addErrorLog("Can not load attachment data: " + data.strName);
            onUploadAttachment(data);   //skip
            m_db.ModifyObjectVersion(data.strGUID, WIZDOCUMENTATTACHMENTDATAEX::ObjectName(), -1);  //re-upload attachment at next time
            return FALSE;
        }
        //
        WIZOBJECTDATA obj;
        obj.strObjectGUID = m_currentAttachment.strGUID;
        obj.strDisplayName = m_currentAttachment.strName;
        obj.eObjectType = wizobjectDocumentAttachment;
        obj.arrayData = m_currentAttachment.arrayData;
        //
        return uploadObjectData(obj);
    }
    else
    {
        return callAttachmentPostData(data);
    }
}

void CWizApi::onUploadAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data)
{
    Q_UNUSED(data);
}


BOOL CWizApi::callAttachmentPostData(const WIZDOCUMENTATTACHMENTDATAEX& data)
{
    m_currentAttachment = data;
    //
    int nParts = data.nObjectPart;
    ATLASSERT(0 != nParts);
    //
    addLog("update attachment info:" + data.strName);
    //
    CWizApiTokenParam param(*this);
    //
    CWizXmlRpcStructValue* pAttachmentStruct = new CWizXmlRpcStructValue();
    param.AddStruct(_T("attachment"), pAttachmentStruct);
    //
    BOOL bInfo = (nParts & WIZKM_XMKRPC_ATTACHMENT_PART_INFO) ? TRUE : FALSE;
    BOOL bData = (nParts & WIZKM_XMKRPC_ATTACHMENT_PART_DATA) ? TRUE : FALSE;
    //
    pAttachmentStruct->AddString(_T("attachment_guid"), data.strGUID);
    pAttachmentStruct->AddBool(_T("attachment_info"), bInfo ? TRUE : FALSE);
    pAttachmentStruct->AddBool(_T("attachment_data"), bData ? TRUE : FALSE);
    //
    BOOL bDataInfoAdded = FALSE;
    //
    const WIZDOCUMENTATTACHMENTDATAEX& infodata = data;
    //
    if (bInfo)
    {
        pAttachmentStruct->AddString(_T("attachment_document_guid"), infodata.strDocumentGUID);
        pAttachmentStruct->AddString(_T("attachment_name"), infodata.strName);
        pAttachmentStruct->AddString(_T("attachment_url"), infodata.strURL);
        pAttachmentStruct->AddString(_T("attachment_description"), infodata.strDescription);
        pAttachmentStruct->AddTime(_T("dt_info_modified"), infodata.tInfoModified);
        pAttachmentStruct->AddString(_T("info_md5"), infodata.strInfoMD5);
        pAttachmentStruct->AddTime(_T("dt_data_modified"), infodata.tDataModified);
        pAttachmentStruct->AddString(_T("data_md5"), infodata.strDataMD5);
        //
        bDataInfoAdded = TRUE;
    }
    if (bData)
    {
        if (!bDataInfoAdded)
        {
            pAttachmentStruct->AddTime(_T("dt_data_modified"), infodata.tDataModified);
            pAttachmentStruct->AddString(_T("data_md5"), infodata.strDataMD5);
            bDataInfoAdded = TRUE;
        }
        pAttachmentStruct->AddString(_T("attachment_zip_md5"), WizMd5StringNoSpaceJava(infodata.arrayData));
    }
    //
    return callXmlRpc(SyncMethod_PostAttachmentData, &param);
}

void CWizApi::onAttachmentPostData(const WIZDOCUMENTATTACHMENTDATAEX& data)
{
    ATLASSERT(data.strGUID == m_currentAttachment.strGUID);
    //
    onUploadAttachment(m_currentAttachment);
}

//

BOOL CWizApi::callDocumentsGetInfo(const CWizStdStringArray& arrayDocumentGUID)
{
    CWizApiTokenParam param(*this);
    param.AddStringArray("document_guids", arrayDocumentGUID);
    //
    return callXmlRpc(SyncMethod_GetDocumentsInfo, &param);
}

void CWizApi::onDocumentsGetInfo(const std::deque<WIZDOCUMENTDATABASE>& arrayRet)
{
    Q_UNUSED(arrayRet);
}

BOOL CWizApi::callAttachmentsGetInfo(const CWizStdStringArray& arrayAttachmentGUID)
{
    CWizApiTokenParam param(*this);
    param.AddStringArray("attachment_guids", arrayAttachmentGUID);
    //
    return callXmlRpc(SyncMethod_GetAttachmentsInfo, &param);
}

void CWizApi::onAttachmentsGetInfo(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
{
    Q_UNUSED(arrayRet);
}

//

BOOL CWizApi::downloadObjectData(const WIZOBJECTDATA& data)
{
    if (data.eObjectType == wizobjectDocument)
    {
        addLog("downloading note: " + data.strDisplayName);
    }
    else if (data.eObjectType == wizobjectDocumentAttachment)
    {
        addLog("downloading attachment: " + data.strDisplayName);
    }
    else
    {
        ATLASSERT(FALSE);
    }
    //
    m_events.changeObjectDataProgress(0);
    //
    m_currentObjectData = data;
    m_currentObjectData.arrayData.clear();;
    //
    return downloadNextPartData();
}

void CWizApi::onDownloadObjectDataCompleted(const WIZOBJECTDATA& data)
{
    m_db.UpdateSyncObjectLocalData(data);
}


BOOL CWizApi::callDeletedPostList(const std::deque<WIZDELETEDGUIDDATA>& arrayData)
{
    m_arrayCurrentPostDeletedGUID.assign(arrayData.begin(), arrayData.end());
    return callPostList(SyncMethod_PostDeletedList, "deleteds", arrayData);
}

void CWizApi::onDeletedPostList(const std::deque<WIZDELETEDGUIDDATA>& arrayData)
{
    for (std::deque<WIZDELETEDGUIDDATA>::const_iterator it = arrayData.begin();
    it != arrayData.end();
    it++)
    {
        m_db.DeleteDeletedGUID(it->strGUID);
    }
}

BOOL CWizApi::callTagPostList(const std::deque<WIZTAGDATA>& arrayData)
{
    m_arrayCurrentPostTag.assign(arrayData.begin(), arrayData.end());
    return callPostList(SyncMethod_PostTagList, "tags", arrayData);
}

void CWizApi::onTagPostList(const std::deque<WIZTAGDATA>& arrayData)
{
    for (std::deque<WIZTAGDATA>::const_iterator it = arrayData.begin();
    it != arrayData.end();
    it++)
    {
        m_db.ModifyObjectVersion(it->strGUID, WIZTAGDATA::ObjectName(), 0);
    }
}

//
BOOL CWizApi::callStylePostList(const std::deque<WIZSTYLEDATA>& arrayData)
{
    m_arrayCurrentPostStyle.assign(arrayData.begin(), arrayData.end());
    return callPostList(SyncMethod_PostTagList, "styles", arrayData);
}

void CWizApi::onStylePostList(const std::deque<WIZSTYLEDATA>& arrayData)
{
    for (std::deque<WIZSTYLEDATA>::const_iterator it = arrayData.begin();
    it != arrayData.end();
    it++)
    {
        m_db.ModifyObjectVersion(it->strGUID, WIZSTYLEDATA::ObjectName(), 0);
    }
}
//
unsigned int CWizApi::getCountPerPage() const
{
    return 10;
}
unsigned int CWizApi::getPartSize() const
{
    return 512 * 1024;
}
BOOL CWizApi::downloadNextPartData()
{
    return callDownloadDataPart(m_currentObjectData.strObjectGUID,
                                WIZOBJECTDATA::ObjectTypeToTypeString(m_currentObjectData.eObjectType),
                                m_currentObjectData.arrayData.size());
}
BOOL CWizApi::uploadNextPartData()
{
    if (m_currentObjectData.arrayData.isEmpty())
    {
        onUploadObjectDataCompleted(m_currentObjectData);
        return TRUE;
    }
    //
    int allSize = m_nCurrentObjectAllSize;
    int partSize = getPartSize();
    //
    int partCount = allSize / partSize;
    if (allSize % partSize != 0)
    {
        partCount++;
    }
    //
    int lastSize = m_currentObjectData.arrayData.size();
    //
    int partIndex = (allSize - lastSize) / partSize;
    //
    QByteArray arrayData;
    //
    if (lastSize <= partSize)
    {
        arrayData = m_currentObjectData.arrayData;
        m_currentObjectData.arrayData.clear();
    }
    else
    {
        arrayData = QByteArray(m_currentObjectData.arrayData.constData(), partSize);
        m_currentObjectData.arrayData.remove(0, partSize);
    }
    //
    return callUploadDataPart(m_currentObjectData.strObjectGUID,
                              WIZOBJECTDATA::ObjectTypeToTypeString(m_currentObjectData.eObjectType),
                              m_strCurrentObjectMD5,
                              allSize, partCount, partIndex, arrayData.size(), arrayData);
}

//
//////////////////////////////////////////////////////////////////////////////

WIZUSERINFO::WIZUSERINFO()
    : nUserLevel(0)
    , nUserPoints(0)
    , nMaxFileSize(10 * 1024 * 1024)
    , bEnableGroup(FALSE)
{

}
int WIZUSERINFO::GetMaxFileSize()
{
    return std::max<int>(20 * 1024 * 1024, nMaxFileSize);;
}
//
BOOL WIZUSERINFO::LoadFromXmlRpc(CWizXmlRpcValue& val)
{
    CWizXmlRpcStructValue* pStruct = dynamic_cast<CWizXmlRpcStructValue*>(&val);
    if (!pStruct)
    {
        TOLOG(_T("Failed to cast CWizXmlRpcValue to CWizXmlRpcStructValue"));
        return FALSE;
    }
    //
    CWizXmlRpcStructValue& data = *pStruct;
    //
    data.GetStr(_T("token"), strToken);
    data.GetTime(_T("expried_time"), tTokenExpried);
    data.GetStr(_T("kapi_url"), strDatabaseServer);
    data.GetStr(_T("download_url"), strDownloadDataServer);
    data.GetStr(_T("upload_url"), strUploadDataServer);
    data.GetStr(_T("capi_url"), strChatServer);
    data.GetStr(_T("kb_guid"), strKbGUID);
    data.GetInt(_T("upload_size_limit"), nMaxFileSize);
    data.GetStr(_T("user_type"), strUserType);
    data.GetStr(_T("show_ad"), strShowAD);
    data.GetStr(_T("system_tags"), strSystemTags);
    data.GetStr(_T("push_tag"), strPushTag);
    data.GetStr(_T("user_level_name"), strUserLevelName);
    data.GetInt(_T("user_level"), nUserLevel);
    data.GetInt(_T("user_points"), nUserPoints);
    data.GetStr(_T("sns_list"), strSNSList);
    data.GetInt(_T("enable_group"), bEnableGroup);
    //
    data.GetStr(_T("notice"), strNotice);
    //
    if (CWizXmlRpcStructValue* pUser = data.GetStruct(_T("user")))
    {
        pUser->GetStr(_T("displayname"), strDisplayName);
        //pUser->GetStr(_T("nickname"), strNickName);
        pUser->GetStr(_T("language"), strLanguage);
        //pUser->GetStr(_T("backupserver"), strBackupDatabaseServer);
    }
    //
    return !strToken.IsEmpty()
        && !strKbGUID.IsEmpty()
        && !strDatabaseServer.IsEmpty();
}



WIZKBINFO::WIZKBINFO()
{
    nStorageLimit = 0;
    nStorageUsage = 0;
    nTrafficLimit = 0;
    nTrafficUsage = 0;
}
//

BOOL WIZKBINFO::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    data.GetInt64(_T("storage_limit"), nStorageLimit);
    data.GetInt64(_T("storage_usage"), nStorageUsage);
    data.GetStr(_T("storage_limit_string"), strStorageLimit);
    data.GetStr(_T("storage_usage_string"), strStorageUsage);
    //
    data.GetInt64(_T("traffic_limit"), nTrafficLimit);
    data.GetInt64(_T("traffic_usage"), nTrafficUsage);
    data.GetStr(_T("traffic_limit_string"), strTrafficLimit);
    data.GetStr(_T("traffic_usage_string"), strTrafficUsage);
    //
    return TRUE;
}

///////////////////////////////////////////////////////////////////

WIZOBJECTPARTDATA::WIZOBJECTPARTDATA()
    : nStartPos(0)
    , nQuerySize(0)
    , nObjectSize(0)
    , bEOF(FALSE)
    , nPartSize(0)
{
}

BOOL WIZOBJECTPARTDATA::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    data.GetInt64(_T("obj_size"), nObjectSize);
    data.GetInt(_T("eof"), bEOF);
    data.GetInt64(_T("part_size"), nPartSize);
    data.GetString(_T("part_md5"), strPartMD5);
    if (!data.GetStream("data", arrayData))
    {
        TOLOG(_T("Fault error, data is null!"));
        return FALSE;
    }
    //
    return TRUE;
}


///////////////////////////////////////////////////////////////////

#define WIZ_API_VERSION     "3"
#define WIZ_CLIENT_VERSION  "2.0.0.0"

#if defined Q_OS_MAC
#define WIZ_CLIENT_TYPE     "QTMAC"
#elif defined Q_OS_LINUX
#define WIZ_CLIENT_TYPE     "QTLINUX"
#else
#define WIZ_CLIENT_TYPE     "QTWIN"
#endif


CWizApiParamBase::CWizApiParamBase()
{
    AddString("api_version", WIZ_API_VERSION);
    AddString("client_type", WIZ_CLIENT_TYPE);
    AddString("client_version", WIZ_CLIENT_VERSION);
}

CWizApiTokenParam::CWizApiTokenParam(CWizApiBase& api)
{
    AddString("token", api.token());
    AddString("kb_guid", api.kbGUID());
}
