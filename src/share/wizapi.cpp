#include "wizapi.h"
#include <QFile>
#include <QDebug>
#include "zip/wizzip.h"
#include "wizsettings.h"

#define PARTSIZE 10*1024
#define MD5PART 10*1024
#define CONSTDEFAULTCOUNT 200

CWizApiBase::CWizApiBase(const QString& strAccountsApiURL /* = WIZ_API_URL*/)
{
    m_server = new CWizXmlRpcServer(strAccountsApiURL);

    connect(m_server, SIGNAL(xmlRpcReturn(const QString&, CWizXmlRpcValue&)), \
            SLOT(xmlRpcReturn(const QString&, CWizXmlRpcValue&)));

    connect(m_server, SIGNAL(xmlRpcError(const QString&, WizXmlRpcError, int, const QString&)), \
            SLOT(xmlRpcError(const QString&, WizXmlRpcError, int, const QString&)));

    resetProxy();
}

void CWizApiBase::abort()
{
    m_server->abort();
}

bool CWizApiBase::callXmlRpc(const QString& strMethodName, CWizXmlRpcValue* pVal)
{
    return m_server->xmlRpcCall(strMethodName, pVal);
}

void CWizApiBase::xmlRpcReturn(const QString& strMethodName, CWizXmlRpcValue& ret)
{
    onXmlRpcReturn(strMethodName, ret);
}

void CWizApiBase::xmlRpcError(const QString& strMethodName, WizXmlRpcError err, int errorCode, const QString& errorMessage)
{
    onXmlRpcError(strMethodName, err, errorCode, errorMessage);
}

void CWizApiBase::onXmlRpcReturn(const QString& strMethodName, CWizXmlRpcValue& ret)
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
    else if (strMethodName == SyncMethod_GetUserCert)
    {
        WIZUSERCERT data;
        ret.ToData(data);
        onGetUserCert(data);
    }
    else
    {
        Q_ASSERT(false);
    }
}

void CWizApiBase::onXmlRpcError(const QString& strMethodName, WizXmlRpcError err, int errorCode, const QString& errorMessage)
{
    Q_UNUSED(err);
    Q_UNUSED(errorCode);

    QString errorMsg(QString("Error: [%1]: %2").arg(strMethodName).arg(errorMessage));
    Q_EMIT processErrorLog(errorMsg);
}

void CWizApiBase::resetProxy()
{
    CWizSettings settings(WizGetSettingsFileName());

    bool bStatus = settings.GetProxyStatus();

    if (bStatus) {
        QString host = settings.GetProxyHost();
        int port = settings.GetProxyPort();
        QString userName = settings.GetProxyUserName();
        QString password = settings.GetProxyPassword();

        m_server->setProxy(host, port, userName, password);
    } else {
        m_server->setProxy(0, 0, 0, 0);
    }
}

CString CWizApiBase::MakeXmlRpcUserId(const CString& strUserId)
{
    return strUserId;
}

CString CWizApiBase::MakeXmlRpcPassword(const CString& strPassword)
{
    return "md5." + ::WizMd5StringNoSpaceJava(strPassword.toUtf8());
}

bool CWizApiBase::callClientLogin(const CString& strUserId, const CString& strPassword)
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

bool CWizApiBase::callClientLogout()
{
    Q_ASSERT(!m_user.strToken.isEmpty());

    CWizApiTokenParam param(*this);
    return callXmlRpc(SyncMethod_ClientLogout, &param);
}

void CWizApiBase::onClientLogout()
{
    m_user.strToken.clear();
    m_user.strKbGUID.clear();
}

bool CWizApiBase::callGetUserInfo()
{
    return true;
}

void CWizApiBase::onGetUserInfo(CWizXmlRpcValue& ret)
{
    Q_UNUSED(ret);
}

bool CWizApiBase::callGetUserCert(const QString& strUserId, const QString& strPassword)
{
    CWizApiParamBase param;
    param.AddString("user_id", MakeXmlRpcUserId(strUserId));
    param.AddString("password", MakeXmlRpcPassword(strPassword));

    return callXmlRpc(SyncMethod_GetUserCert, &param);
}

void CWizApiBase::onGetUserCert(const WIZUSERCERT& data)
{
    Q_UNUSED(data);
}

bool CWizApiBase::callCreateAccount(const CString& strUserId, const CString& strPassword)
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

    return callXmlRpc(SyncMethod_CreateAccount, &param);
}

void CWizApiBase::onCreateAccount()
{

}

CWizApi::CWizApi(CWizDatabase& db, const CString& strAccountsApiURL /* = WIZ_API_URL */)
    : CWizApiBase(strAccountsApiURL)
    , m_db(db)
    , m_nCurrentObjectAllSize(0)
    , m_bDownloadingObject(false)
{
}

void CWizApi::onXmlRpcReturn(const QString& strMethodName, CWizXmlRpcValue& ret)
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


bool CWizApi::callGetList(const QString& strMethodName, __int64 nVersion)
{
    CWizApiTokenParam param(*this);
    param.AddInt(_T("count"), getCountPerPage());
    param.AddString(_T("version"), WizInt64ToStr(nVersion));

    return callXmlRpc(strMethodName, &param);
}

bool CWizApi::callDeletedGetList(__int64 nVersion)
{
    Q_EMIT processLog(tr("Syncing deleted items, version: ") + QString::number(nVersion));
    return callGetList(SyncMethod_GetDeletedList, nVersion);
}

void CWizApi::onDeletedGetList(const std::deque<WIZDELETEDGUIDDATA>& arrayRet)
{
    m_db.UpdateDeletedGUIDs(arrayRet);
}

bool CWizApi::callTagGetList(__int64 nVersion)
{
    Q_EMIT processLog(tr("Syncing tags, version: ") + QString::number(nVersion));
    return callGetList(SyncMethod_GetTagList, nVersion);
}

void CWizApi::onTagGetList(const std::deque<WIZTAGDATA>& arrayRet)
{
    m_db.UpdateTags(arrayRet);
}

bool CWizApi::callStyleGetList(__int64 nVersion)
{
    Q_EMIT processLog(tr("Syncing styles, version: ") + QString::number(nVersion));
    return callGetList(SyncMethod_GetStyleList, nVersion);
}

void CWizApi::onStyleGetList(const std::deque<WIZSTYLEDATA>& arrayRet)
{
    m_db.UpdateStyles(arrayRet);
}

bool CWizApi::callDocumentGetList(__int64 nVersion)
{
    Q_EMIT processLog(tr("Syncing note list, version: ") + QString::number(nVersion));
    return callGetList(SyncMethod_GetDocumentList, nVersion);
}

void CWizApi::onDocumentGetList(const std::deque<WIZDOCUMENTDATABASE>& arrayRet)
{
    Q_UNUSED(arrayRet);
}

bool CWizApi::callAttachmentGetList(__int64 nVersion)
{
    Q_EMIT processLog(tr("Syncing attachment list: version: ") + QString::number(nVersion));
    return callGetList(SyncMethod_GetAttachmentList, nVersion);
}

void CWizApi::onAttachmentGetList(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
{
    m_db.UpdateAttachments(arrayRet);
}

bool CWizApi::callDownloadDataPart(const CString& strObjectGUID, const CString& strObjectType, int pos)
{
    m_bDownloadingObject = true;

    unsigned int size = getPartSize();

    m_currentObjectPartData.strObjectGUID = strObjectGUID;
    m_currentObjectPartData.strObjectType = strObjectType;
    m_currentObjectPartData.nStartPos = pos;
    m_currentObjectPartData.nQuerySize = size;

    CWizApiTokenParam param(*this);

    param.AddString(_T("obj_guid"), strObjectGUID);
    param.AddString(_T("obj_type"), strObjectType);

    param.AddInt64(_T("start_pos"), pos);
    param.AddInt64(_T("part_size"), size);

    return callXmlRpc(SyncMethod_DownloadObjectPart, &param);
}

void CWizApi::onDownloadDataPart(const WIZOBJECTPARTDATA& data)
{
    m_bDownloadingObject = false;

    // TODO: verify MD5 of data

    m_currentObjectData.arrayData.append(data.arrayData);

    double fPercent = 0;
    if (data.nObjectSize > 0)
        fPercent = 100.0 * m_currentObjectData.arrayData.size() / double(data.nObjectSize);

    m_nCurrentObjectAllSize = data.nObjectSize;

    //QString info = m_currentObjectData.strDisplayName;
    Q_EMIT processLog(tr("Downloaded : ") + QString::number(fPercent) + "%");
    Q_EMIT progressChanged(int(fPercent));

    if (data.bEOF)
    {
        onDownloadObjectDataCompleted(m_currentObjectData);
    }
    else
    {
        downloadNextPartData();
    }
}

bool CWizApi::callUploadDataPart(const CString& strObjectGUID, const CString& strObjectType, const CString& strObjectMD5, int allSize, int partCount, int partIndex, int partSize, const QByteArray& arrayData)
{
    Q_UNUSED(allSize);

    Q_ASSERT(partSize == arrayData.size());

    CWizApiTokenParam param(*this);
    param.AddString(_T("obj_guid"), strObjectGUID);
    param.AddString(_T("obj_type"), strObjectType);
    param.AddString(_T("obj_md5"), strObjectMD5);
    param.AddInt(_T("part_count"), partCount);
    param.AddInt(_T("part_sn"), partIndex);
    param.AddInt64(_T("part_size"), partSize);
    param.AddString(_T("part_md5"), ::WizMd5StringNoSpaceJava(arrayData));
    param.AddBase64(_T("data"), arrayData);

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

bool CWizApi::uploadObjectData(const WIZOBJECTDATA& data)
{
    if (data.eObjectType == wizobjectDocument)
    {
        QString info = data.strDisplayName;
        Q_EMIT processLog(tr("uploading note: ") + info);
    }
    else if (data.eObjectType == wizobjectDocumentAttachment)
    {
        QString info = data.strDisplayName;
        Q_EMIT processLog(tr("uploading attachment: ") + info);
    }
    else
    {
        Q_ASSERT(false);
    }

    Q_ASSERT(!data.arrayData.isEmpty());

    m_currentObjectData = data;
    m_nCurrentObjectAllSize = data.arrayData.size();
    m_strCurrentObjectMD5 = ::WizMd5StringNoSpaceJava(m_currentObjectData.arrayData);

    return uploadNextPartData();
}

void CWizApi::onUploadObjectDataCompleted(const WIZOBJECTDATA& data)
{
    if (data.eObjectType == wizobjectDocument) {
        Q_ASSERT(data.strObjectGUID == m_currentDocument.strGUID);
        Q_ASSERT(m_currentDocument.nObjectPart & WIZKM_XMLRPC_OBJECT_PART_DATA);

        m_db.SetObjectDataDownloaded(m_currentDocument.strGUID, "document", true);
        callDocumentPostData(m_currentDocument);

    } else if (data.eObjectType == wizobjectDocumentAttachment) {
        Q_ASSERT(data.strObjectGUID == m_currentAttachment.strGUID);
        Q_ASSERT(m_currentAttachment.nObjectPart & WIZKM_XMLRPC_OBJECT_PART_DATA);

        m_db.SetObjectDataDownloaded(m_currentDocument.strGUID, "attachment", true);
        callAttachmentPostData(m_currentAttachment);

    } else {
        Q_ASSERT(false);
    }
}

bool CWizApi::callDocumentGetData(const WIZDOCUMENTDATABASE& data)
{
    QString info = data.strTitle;
    Q_EMIT processLog(tr("downloading note info: ") + info);

    int nPart = data.nObjectPart;
    Q_ASSERT(nPart != 0);

    CWizApiTokenParam param(*this);
    param.AddString("document_guid", data.strGUID);
    param.AddBool("document_info", (nPart & WIZKM_XMKRPC_DOCUMENT_PART_INFO) ? true : false);
    param.AddBool("document_param", (nPart & WIZKM_XMKRPC_DOCUMENT_PART_PARAM) ? true : false);

    return callXmlRpc(SyncMethod_GetDocumentData, &param);
}

void CWizApi::onDocumentGetData(const WIZDOCUMENTDATAEX& data)
{
    m_db.UpdateDocument(data);
}

bool CWizApi::uploadDocument(const WIZDOCUMENTDATAEX& data)
{
    m_currentDocument = data;

    int nParts = m_currentDocument.nObjectPart;
    Q_ASSERT(0 != nParts);
    bool bData = (nParts & WIZKM_XMKRPC_DOCUMENT_PART_DATA) ? true : false;

    if (bData) {
        if (!m_db.LoadDocumentData(m_currentDocument.strGUID, m_currentDocument.arrayData)) {
            //skip this document
            QString info = data.strTitle;
            Q_EMIT processErrorLog(tr("could not load document data: ") + info);
            onUploadDocument(data);   //skip
            return false;
        }

        WIZOBJECTDATA obj;
        obj.strObjectGUID = m_currentDocument.strGUID;
        obj.strDisplayName = m_currentDocument.strTitle;
        obj.eObjectType = wizobjectDocument;
        obj.arrayData = m_currentDocument.arrayData;

        return uploadObjectData(obj);
    } else {
        return callDocumentPostData(m_currentDocument);
    }
}

void CWizApi::onUploadDocument(const WIZDOCUMENTDATAEX& data)
{
    Q_UNUSED(data);
}

bool CWizApi::callDocumentPostData(const WIZDOCUMENTDATAEX& data)
{
    m_currentDocument = data;

    int nParts = m_currentDocument.nObjectPart;
    Q_ASSERT(0 != nParts);
    bool bInfo = (nParts & WIZKM_XMKRPC_DOCUMENT_PART_INFO) ? true : false;
    bool bData = (nParts & WIZKM_XMKRPC_DOCUMENT_PART_DATA) ? true : false;
    bool bParam = (nParts & WIZKM_XMKRPC_DOCUMENT_PART_PARAM) ? true : false;

    QString info = data.strTitle;
    Q_EMIT processLog(tr("upload document info:") + info);

    CWizApiTokenParam param(*this);

    CWizXmlRpcStructValue* pDocumentStruct = new CWizXmlRpcStructValue();
    param.AddStruct(_T("document"), pDocumentStruct);

    pDocumentStruct->AddString(_T("document_guid"), data.strGUID);
    pDocumentStruct->AddBool(_T("document_info"), bInfo ? true : false);
    pDocumentStruct->AddBool(_T("document_data"), bData ? true : false);
    pDocumentStruct->AddBool(_T("document_param"), bParam ? true : false);

    bool bParamInfoAdded = false;
    bool bDataInfoAdded = false;

    WIZDOCUMENTDATAEX& infodata = m_currentDocument;

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

        bParamInfoAdded = true;
        bDataInfoAdded = true;

        m_db.GetDocumentTags(infodata.strGUID, infodata.arrayTagGUID);

        pDocumentStruct->AddStringArray(_T("document_tags"), data.arrayTagGUID);
    }
    if (bParam)
    {
        m_db.GetDocumentParams(infodata.strGUID, infodata.arrayParam);

        if (!bParamInfoAdded)
        {
            pDocumentStruct->AddTime(_T("dt_param_modified"), infodata.tParamModified);
            pDocumentStruct->AddString(_T("param_md5"), infodata.strParamMD5);
            bParamInfoAdded = true;
        }

        pDocumentStruct->AddArray(_T("document_params"), infodata.arrayParam);
    }
    if (bData)
    {
        if (!m_db.LoadDocumentData(m_currentDocument.strGUID, m_currentDocument.arrayData))
        {
            //skip this document
            QString info2 = data.strTitle;
            Q_EMIT processErrorLog(tr("Can not load document data: ") + info2);
            onUploadObjectDataCompleted(data);
            return false;
        }

        if (!bDataInfoAdded)
        {
            pDocumentStruct->AddTime(_T("dt_data_modified"), infodata.tDataModified);
            pDocumentStruct->AddString(_T("data_md5"), infodata.strDataMD5);
            bDataInfoAdded = true;
        }

        pDocumentStruct->AddString(_T("document_zip_md5"), WizMd5StringNoSpaceJava(infodata.arrayData));
    }

    return callXmlRpc(SyncMethod_PostDocumentData, &param);
}

void CWizApi::onDocumentPostData(const WIZDOCUMENTDATAEX& data)
{
    Q_ASSERT(data.strGUID == m_currentDocument.strGUID);
    onUploadDocument(m_currentDocument);
}

bool CWizApi::uploadAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data)
{
    m_currentAttachment = data;

    int nParts = data.nObjectPart;
    Q_ASSERT(0 != nParts);
    bool bData = (nParts & WIZKM_XMKRPC_ATTACHMENT_PART_DATA) ? true : false;

    if (bData)
    {
        if (!m_db.LoadCompressedAttachmentData(m_currentAttachment.strGUID, m_currentAttachment.arrayData))
        {
            //skip this document
            QString info = data.strName;
            Q_EMIT processErrorLog(tr("Could not load attachment data: ") + info);
            onUploadAttachment(data);   //skip
            m_db.ModifyObjectVersion(data.strGUID, WIZDOCUMENTATTACHMENTDATAEX::ObjectName(), -1);  //re-upload attachment at next time
            return false;
        }

        WIZOBJECTDATA obj;
        obj.strObjectGUID = m_currentAttachment.strGUID;
        obj.strDisplayName = m_currentAttachment.strName;
        obj.eObjectType = wizobjectDocumentAttachment;
        obj.arrayData = m_currentAttachment.arrayData;

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

bool CWizApi::callAttachmentPostData(const WIZDOCUMENTATTACHMENTDATAEX& data)
{
    m_currentAttachment = data;

    int nParts = data.nObjectPart;
    Q_ASSERT(0 != nParts);

    QString info = data.strName;
    Q_EMIT processLog(tr("upload attachment info: ") + info);

    CWizApiTokenParam param(*this);

    CWizXmlRpcStructValue* pAttachmentStruct = new CWizXmlRpcStructValue();
    param.AddStruct(_T("attachment"), pAttachmentStruct);

    bool bInfo = (nParts & WIZKM_XMKRPC_ATTACHMENT_PART_INFO) ? true : false;
    bool bData = (nParts & WIZKM_XMKRPC_ATTACHMENT_PART_DATA) ? true : false;

    pAttachmentStruct->AddString(_T("attachment_guid"), data.strGUID);
    pAttachmentStruct->AddBool(_T("attachment_info"), bInfo ? true : false);
    pAttachmentStruct->AddBool(_T("attachment_data"), bData ? true : false);

    bool bDataInfoAdded = false;

    const WIZDOCUMENTATTACHMENTDATAEX& infodata = data;

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

        bDataInfoAdded = true;
    }
    if (bData)
    {
        if (!bDataInfoAdded)
        {
            pAttachmentStruct->AddTime(_T("dt_data_modified"), infodata.tDataModified);
            pAttachmentStruct->AddString(_T("data_md5"), infodata.strDataMD5);
            bDataInfoAdded = true;
        }
        pAttachmentStruct->AddString(_T("attachment_zip_md5"), WizMd5StringNoSpaceJava(infodata.arrayData));
    }

    return callXmlRpc(SyncMethod_PostAttachmentData, &param);
}

void CWizApi::onAttachmentPostData(const WIZDOCUMENTATTACHMENTDATAEX& data)
{
    Q_ASSERT(data.strGUID == m_currentAttachment.strGUID);
    onUploadAttachment(m_currentAttachment);
}

bool CWizApi::callDocumentsGetInfo(const CWizStdStringArray& arrayDocumentGUID)
{
    CWizApiTokenParam param(*this);
    param.AddStringArray("document_guids", arrayDocumentGUID);

    return callXmlRpc(SyncMethod_GetDocumentsInfo, &param);
}

void CWizApi::onDocumentsGetInfo(const std::deque<WIZDOCUMENTDATABASE>& arrayRet)
{
    Q_UNUSED(arrayRet);
}

bool CWizApi::callAttachmentsGetInfo(const CWizStdStringArray& arrayAttachmentGUID)
{
    CWizApiTokenParam param(*this);
    param.AddStringArray("attachment_guids", arrayAttachmentGUID);

    return callXmlRpc(SyncMethod_GetAttachmentsInfo, &param);
}

void CWizApi::onAttachmentsGetInfo(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
{
    Q_UNUSED(arrayRet);
}

bool CWizApi::downloadObjectData(const WIZOBJECTDATA& data)
{
    if (data.eObjectType == wizobjectDocument)
    {
        QString info = data.strDisplayName;
        Q_EMIT processLog(tr("downloading note: ") + info);
    }
    else if (data.eObjectType == wizobjectDocumentAttachment)
    {
        QString info = data.strDisplayName;
        Q_EMIT processLog(tr("downloading attachment: ") + info);
    }
    else
    {
        Q_ASSERT(false);
    }

    Q_EMIT progressChanged(0);

    m_currentObjectData = data;
    m_currentObjectData.arrayData.clear();;

    return downloadNextPartData();
}

void CWizApi::onDownloadObjectDataCompleted(const WIZOBJECTDATA& data)
{
    m_db.UpdateSyncObjectLocalData(data);
}

bool CWizApi::callDeletedPostList(const std::deque<WIZDELETEDGUIDDATA>& arrayData)
{
    m_arrayCurrentPostDeletedGUID.assign(arrayData.begin(), arrayData.end());
    return callPostList(SyncMethod_PostDeletedList, "deleteds", arrayData);
}

void CWizApi::onDeletedPostList(const std::deque<WIZDELETEDGUIDDATA>& arrayData)
{
    std::deque<WIZDELETEDGUIDDATA>::const_iterator it;
    for (it = arrayData.begin(); it != arrayData.end(); it++) {
        m_db.DeleteDeletedGUID(it->strGUID);
    }
}

bool CWizApi::callTagPostList(const std::deque<WIZTAGDATA>& arrayData)
{
    m_arrayCurrentPostTag.assign(arrayData.begin(), arrayData.end());
    return callPostList(SyncMethod_PostTagList, "tags", arrayData);
}

void CWizApi::onTagPostList(const std::deque<WIZTAGDATA>& arrayData)
{
    std::deque<WIZTAGDATA>::const_iterator it;
    for (it = arrayData.begin(); it != arrayData.end(); it++) {
        m_db.ModifyObjectVersion(it->strGUID, WIZTAGDATA::ObjectName(), 0);
    }
}

bool CWizApi::callStylePostList(const std::deque<WIZSTYLEDATA>& arrayData)
{
    m_arrayCurrentPostStyle.assign(arrayData.begin(), arrayData.end());
    return callPostList(SyncMethod_PostStyleList, "styles", arrayData);
}

void CWizApi::onStylePostList(const std::deque<WIZSTYLEDATA>& arrayData)
{
    std::deque<WIZSTYLEDATA>::const_iterator it;
    for (it = arrayData.begin(); it != arrayData.end(); it++) {
        m_db.ModifyObjectVersion(it->strGUID, WIZSTYLEDATA::ObjectName(), 0);
    }
}

unsigned int CWizApi::getCountPerPage() const
{
    return 10;
}

unsigned int CWizApi::getPartSize() const
{
    return 512 * 1024;
}

bool CWizApi::downloadNextPartData()
{
    int size = m_currentObjectData.arrayData.size();
    return callDownloadDataPart(m_currentObjectData.strObjectGUID,
                                WIZOBJECTDATA::ObjectTypeToTypeString(m_currentObjectData.eObjectType),
                                size);
}

bool CWizApi::uploadNextPartData()
{
    if (m_currentObjectData.arrayData.isEmpty())
    {
        onUploadObjectDataCompleted(m_currentObjectData);
        return true;
    }

    int allSize = m_nCurrentObjectAllSize;
    int partSize = getPartSize();

    int partCount = allSize / partSize;
    if (allSize % partSize != 0)
    {
        partCount++;
    }

    int lastSize = m_currentObjectData.arrayData.size();

    int partIndex = (allSize - lastSize) / partSize;

    QByteArray arrayData;

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

    return callUploadDataPart(m_currentObjectData.strObjectGUID,
                              WIZOBJECTDATA::ObjectTypeToTypeString(m_currentObjectData.eObjectType),
                              m_strCurrentObjectMD5,
                              allSize, partCount, partIndex, arrayData.size(), arrayData);
}




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
