#include "wizsync.h"
#include <algorithm>

enum WizSyncProgress
{
    progressStart = 0,
    progressOnLogin = 1,
    progressDeletedsDownloaded = 3,
    progressDeletedsUploaded = 5,
    progressTagsDownloaded = 7,
    progressTagsUploaded = 8,
    progressStyleDownloaded = 9,
    progressStyleUploaded = 10,
    progressDocumentSimpleInfoDownloaded = 11,
    progressDocumemtFullInfoDownloaded = 18,
    progressAttachmentInfoDownloaded = 20,
    progressDocumentUploaded = 50,
    progressAttachmentUploaded = 60,
    progressObjectDownloaded = 99,
    progressDone = 100,
};

const int DOWNLOAD_DOCUMENT_FULL_INFO_STEP_BEGIN      = progressDocumentSimpleInfoDownloaded;
const int DOWNLOAD_DOCUMENT_FULL_INFO_STEP_COUNT      = progressDocumemtFullInfoDownloaded - progressDocumentSimpleInfoDownloaded;

const int UPLOAD_DOCUMENT_STEP_BEGIN                  = progressAttachmentInfoDownloaded;
const int UPLOAD_DOCUMENT_STEP_COUNT                  = progressDocumentUploaded - progressAttachmentInfoDownloaded;

const int UPLOAD_ATTACHMENT_STEP_BEGIN                = progressDocumentUploaded;
const int UPLOAD_ATTACHMENT_STEP_COUNT                = progressAttachmentUploaded - progressDocumentUploaded;

const int DOWNLOAD_OBJECT_STEP_BEGIN                 = progressAttachmentUploaded;
const int DOWNLOAD_OBJECT_STEP_COUNT                 = progressObjectDownloaded - progressAttachmentUploaded;


CWizSync::CWizSync(CWizDatabase& db, const CString& strAccountsApiURL, CWizSyncEvents& events)
    : CWizApi(db, strAccountsApiURL, events)
    , m_error(false)
    , m_nAllDocumentsNeedToBeDownloadedCount(0)
    , m_bDocumentInfoError(FALSE)
    , m_nDocumentMaxVersion(-1)
    , m_nAllDocumentsNeedToBeUploadedCount(0)
    , m_nAllAttachmentsNeedToBeUploadedCount(0)
    , m_nAllObjectsNeedToBeDownloadedCount(0)
    , m_bDownloadAllNotesData(true)
{
}

void CWizSync::startSync()
{
    if (isSyncing())
        return;
    //
    m_error = false;
    //
    m_events.syncStarted();
    //
    m_nAllDocumentsNeedToBeDownloadedCount = 0;
    m_bDocumentInfoError = FALSE;
    m_nDocumentMaxVersion = 0;
    m_arrayAllDocumentsNeedToBeDownloaded.clear();;
    //
    m_arrayAllDeletedsNeedToBeUploaded.clear();;
    m_arrayAllTagsNeedToBeUploaded.clear();;
    m_arrayAllStylesNeedToBeUploaded.clear();;
    //
    m_nAllDocumentsNeedToBeUploadedCount = 0;
    m_arrayAllDocumentsNeedToBeUploaded.clear();;
    m_nAllAttachmentsNeedToBeUploadedCount = 0;
    m_arrayAllAttachmentsNeedToBeUploaded.clear();;
    //
    m_currentUploadDocument = WIZDOCUMENTDATAEX();
    m_currentUploadAttachment = WIZDOCUMENTATTACHMENTDATAEX();
    //
    m_nAllObjectsNeedToBeDownloadedCount = 0;
    m_arrayAllObjectsNeedToBeDownloaded.clear();
    //
    changeProgress(progressStart);
    //
    addLog("login");
    callClientLogin(m_db.GetUserId(), m_db.GetPassword());
}
void CWizSync::onXmlRpcError(const CString& strMethodName, WizXmlRpcError err, int errorCode, const CString& errorMessage)
{
    CWizApi::onXmlRpcError(strMethodName, err, errorCode, errorMessage);
    //
    m_error = true;
    //
    if (strMethodName != SyncMethod_ClientLogout)
    {
        stopSync();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CWizSync::onClientLogin()
{
    changeProgress(progressOnLogin);
    //
    addLog("downloading deleted objects list");
    startDownloadDeleteds();
    //
    m_events.syncLogin();
}

void CWizSync::startDownloadDeleteds()
{
    downloadNextDeleteds(m_db.GetObjectVersion(WIZDELETEDGUIDDATA::ObjectName()));
}

void CWizSync::onDownloadDeletedsCompleted()
{
    changeProgress(progressDeletedsDownloaded);
    //
    addLog("uploading deleted objects list");
    startUploadDeleteds();
}

void CWizSync::startUploadDeleteds()
{
    m_db.GetModifiedDeletedGUIDs(m_arrayAllDeletedsNeedToBeUploaded);
    uploadNextDeleteds();
}

void CWizSync::onUploadDeletedsCompleted()
{
    changeProgress(progressDeletedsUploaded);
    //
    addLog("downloading tags");
    startDownloadTags();
}


void CWizSync::startDownloadTags()
{
    downloadNextTags(m_db.GetObjectVersion(WIZTAGDATA::ObjectName()));
}


void CWizSync::onDownloadTagsCompleted()
{
    changeProgress(progressTagsDownloaded);
    //
    addLog("uploading tags");
    startUploadTags();
}


void CWizSync::startUploadTags()
{
    m_db.GetModifiedTags(m_arrayAllTagsNeedToBeUploaded);
    uploadNextTags();
}

void CWizSync::onUploadTagsCompleted()
{
    changeProgress(progressTagsUploaded);
    //
    addLog("downloading styles");
    startDownloadStyles();
}


void CWizSync::startDownloadStyles()
{
    downloadNextStyles(m_db.GetObjectVersion(WIZSTYLEDATA::ObjectName()));
}

void CWizSync::onDownloadStylesCompleted()
{
    changeProgress(progressStyleDownloaded);
    //
    addLog("uploading styles");
    startUploadStyles();
}

void CWizSync::startUploadStyles()
{
    m_db.GetModifiedStyles(m_arrayAllStylesNeedToBeUploaded);
    uploadNextStyles();
}

void CWizSync::onUploadStylesCompleted()
{
    changeProgress(progressStyleUploaded);
    //
    addLog("downloading notes list");
    startDownloadDocumentsSimpleInfo();
}

void CWizSync::startDownloadDocumentsSimpleInfo()
{
    downloadNextDocumentsSimpleInfo(m_db.GetObjectVersion(WIZDOCUMENTDATA::ObjectName()));
}

void CWizSync::onDownloadDocumentsSimpleInfoCompleted()
{
    m_events.changeProgress(progressDocumentSimpleInfoDownloaded);
    //
    //save max version of document
    //if no error occured while downloading document full information
    //then update this version
    //
    m_bDocumentInfoError = FALSE;
    m_nDocumentMaxVersion = -1;
    if (!m_arrayAllDocumentsNeedToBeDownloaded.empty())
    {
        m_nDocumentMaxVersion = ::WizObjectsGetMaxVersion<WIZDOCUMENTDATABASE>(m_arrayAllDocumentsNeedToBeDownloaded);
    }
    //
    //filter documents for getting document full information (not data)
    //
    filterDocuments();
    //
    addLog("downloading notes information");
    startDownloadDocumentsFullInfo();
}

void CWizSync::startDownloadDocumentsFullInfo()
{
    m_nAllDocumentsNeedToBeDownloadedCount = m_arrayAllDocumentsNeedToBeDownloaded.size();
    //
    downloadNextDocumentFullInfo();
}

void CWizSync::onDownloadDocumentsFullInfoCompleted()
{
    changeProgress(progressDocumemtFullInfoDownloaded);
    //
    //if no error occured while downloading document full information
    //update document version
    if (!m_bDocumentInfoError
        && -1 != m_nDocumentMaxVersion)
    {
        m_db.SetObjectVersion(WIZDOCUMENTDATA::ObjectName(), m_nDocumentMaxVersion);
    }
    //
    addLog("downloading attachments information");
    startDownloadAttachmentsInfo();
}


void CWizSync::startDownloadAttachmentsInfo()
{
    downloadNextAttachmentsInfo(m_db.GetObjectVersion(WIZDOCUMENTATTACHMENTDATA::ObjectName()));
}

void CWizSync::onDownloadAttachmentsInfoCompleted()
{
    changeProgress(progressAttachmentInfoDownloaded);
    //
    addLog("uploading documents");
    startUploadDocuments();
}
void CWizSync::startUploadDocuments()
{
    m_db.GetModifiedDocuments(m_arrayAllDocumentsNeedToBeUploaded);
    m_nAllDocumentsNeedToBeUploadedCount = int(m_arrayAllDocumentsNeedToBeUploaded.size());
    uploadNextDocument();
}

void CWizSync::onUploadDocumentsCompleted()
{
    changeProgress(progressDocumentUploaded);
    //
    addLog("uploading attachments");
    startUploadAttachments();
}


void CWizSync::startUploadAttachments()
{
    m_db.GetModifiedAttachments(m_arrayAllAttachmentsNeedToBeUploaded);
    m_nAllAttachmentsNeedToBeUploadedCount = int(m_arrayAllAttachmentsNeedToBeUploaded.size());
    uploadNextAttachment();
}


void CWizSync::onUploadAttachmentsCompleted()
{
    changeProgress(progressAttachmentUploaded);
    //
    addLog("downloading objects data");
    startDownloadObjectsData();
}

void CWizSync::startDownloadObjectsData()
{
    m_db.GetAllObjectsNeedToBeDownloaded(m_arrayAllObjectsNeedToBeDownloaded);
    //
    if (!m_bDownloadAllNotesData)
    {
        COleDateTime tNow = ::WizGetCurrentTime();
        //
        size_t count = m_arrayAllObjectsNeedToBeDownloaded.size();
        for (intptr_t i = count - 1; i >= 0; i--)
        {
            COleDateTime t = m_arrayAllObjectsNeedToBeDownloaded[i].tTime;
            t = t.addDays(7);
            if (t < tNow)
            {
                m_arrayAllObjectsNeedToBeDownloaded.erase(m_arrayAllObjectsNeedToBeDownloaded.begin() + i);
                continue;
            }
            //
            if (m_arrayAllObjectsNeedToBeDownloaded[i].eObjectType == wizobjectDocumentAttachment)
            {
                m_arrayAllObjectsNeedToBeDownloaded.erase(m_arrayAllObjectsNeedToBeDownloaded.begin() + i);
                continue;
            }
        }
    }
    //
    //
    m_nAllObjectsNeedToBeDownloadedCount = m_arrayAllObjectsNeedToBeDownloaded.size();
    //
    downloadNextObjectData();
}

void CWizSync::onDownloadObjectsDataCompleted()
{
    changeProgress(progressObjectDownloaded);
    //
    stopSync();
}

void CWizSync::onClientLogout()
{
    CWizApi::onClientLogout();
}

void CWizSync::stopSync()
{
    addLog("logout");
    //
    if (!m_user.strToken.isEmpty())
    {
        callClientLogout();
    }
    //
    changeProgress(progressDone);

    addLog("sync done");
    m_events.syncDone(m_error);
}

///////////////

////////////////////////////////////////////////////////////////////////////////////////////////

void CWizSync::downloadNextDeleteds(__int64 nVersion)
{
    callDeletedGetList(nVersion);
}
void CWizSync::downloadNextTags(__int64 nVersion)
{
    callTagGetList(nVersion);
}
void CWizSync::downloadNextStyles(__int64 nVersion)
{
    callStyleGetList(nVersion);
}
void CWizSync::downloadNextDocumentsSimpleInfo(__int64 nVersion)
{
    callDocumentGetList(nVersion);
}
void CWizSync::downloadNextAttachmentsInfo(__int64 nVersion)
{
    callAttachmentGetList(nVersion);
}


void CWizSync::uploadNextDeleteds()
{
    if (m_arrayAllDeletedsNeedToBeUploaded.empty())
    {
        onUploadDeletedsCompleted();
    }
    else
    {
        int countPerPage = getCountPerPage();
        //
        CWizDeletedGUIDDataArray arrayCurr;
        if (m_arrayAllDeletedsNeedToBeUploaded.size() > (size_t)countPerPage)
        {
            arrayCurr.assign(m_arrayAllDeletedsNeedToBeUploaded.begin(), m_arrayAllDeletedsNeedToBeUploaded.begin() + countPerPage);
            m_arrayAllDeletedsNeedToBeUploaded.erase(m_arrayAllDeletedsNeedToBeUploaded.begin(), m_arrayAllDeletedsNeedToBeUploaded.begin() + countPerPage);
        }
        else
        {
            arrayCurr.assign(m_arrayAllDeletedsNeedToBeUploaded.begin(), m_arrayAllDeletedsNeedToBeUploaded.end());
            m_arrayAllDeletedsNeedToBeUploaded.clear();
        }
        //
        callDeletedPostList(arrayCurr);
    }
}

void CWizSync::uploadNextTags()
{
    if (m_arrayAllTagsNeedToBeUploaded.empty())
    {
        onUploadTagsCompleted();
    }
    else
    {
        int countPerPage = getCountPerPage();
        //
        CWizTagDataArray arrayCurr;
        if (m_arrayAllTagsNeedToBeUploaded.size() > (size_t)countPerPage)
        {
            arrayCurr.assign(m_arrayAllTagsNeedToBeUploaded.begin(), m_arrayAllTagsNeedToBeUploaded.begin() + countPerPage);
            m_arrayAllTagsNeedToBeUploaded.erase(m_arrayAllTagsNeedToBeUploaded.begin(), m_arrayAllTagsNeedToBeUploaded.begin() + countPerPage);
        }
        else
        {
            arrayCurr.assign(m_arrayAllTagsNeedToBeUploaded.begin(), m_arrayAllTagsNeedToBeUploaded.end());
            m_arrayAllTagsNeedToBeUploaded.clear();
        }
        //
        callTagPostList(arrayCurr);
    }
}

void CWizSync::uploadNextStyles()
{
    if (m_arrayAllStylesNeedToBeUploaded.empty())
    {
        onUploadStylesCompleted();
    }
    else
    {
        int countPerPage = getCountPerPage();
        //
        CWizStyleDataArray arrayCurr;
        if (m_arrayAllStylesNeedToBeUploaded.size() > (size_t)countPerPage)
        {
            arrayCurr.assign(m_arrayAllStylesNeedToBeUploaded.begin(), m_arrayAllStylesNeedToBeUploaded.begin() + countPerPage);
            m_arrayAllStylesNeedToBeUploaded.erase(m_arrayAllStylesNeedToBeUploaded.begin(), m_arrayAllStylesNeedToBeUploaded.begin() + countPerPage);
        }
        else
        {
            arrayCurr.assign(m_arrayAllStylesNeedToBeUploaded.begin(), m_arrayAllStylesNeedToBeUploaded.end());
            m_arrayAllStylesNeedToBeUploaded.clear();
        }
        //
        callStylePostList(arrayCurr);
    }
}

void CWizSync::uploadNextDocument()
{
    if (m_arrayAllDocumentsNeedToBeUploaded.empty())
    {
        onUploadDocumentsCompleted();
    }
    else
    {
        ATLASSERT(m_nAllDocumentsNeedToBeUploadedCount > 0);
        changeProgressEx(m_arrayAllDocumentsNeedToBeUploaded.size(), m_nAllDocumentsNeedToBeUploadedCount, UPLOAD_DOCUMENT_STEP_BEGIN, UPLOAD_DOCUMENT_STEP_COUNT);
        //
        m_currentUploadDocument = m_arrayAllDocumentsNeedToBeUploaded[0];
        m_arrayAllDocumentsNeedToBeUploaded.erase(m_arrayAllDocumentsNeedToBeUploaded.begin());
        //
        queryDocumentInfo(m_currentUploadDocument.strGUID, m_currentUploadDocument.strTitle);
    }
}
void CWizSync::uploadNextAttachment()
{
    if (m_arrayAllAttachmentsNeedToBeUploaded.empty())
    {
        onUploadAttachmentsCompleted();
    }
    else
    {
        ATLASSERT(m_nAllAttachmentsNeedToBeUploadedCount > 0);
        changeProgressEx(m_arrayAllAttachmentsNeedToBeUploaded.size(), m_nAllAttachmentsNeedToBeUploadedCount, UPLOAD_ATTACHMENT_STEP_BEGIN, UPLOAD_ATTACHMENT_STEP_COUNT);
        //
        m_currentUploadAttachment = m_arrayAllAttachmentsNeedToBeUploaded[0];
        m_arrayAllAttachmentsNeedToBeUploaded.erase(m_arrayAllAttachmentsNeedToBeUploaded.begin());

        queryAttachmentInfo(m_currentUploadAttachment.strGUID, m_currentUploadAttachment.strName);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////


void CWizSync::onDeletedPostList(const std::deque<WIZDELETEDGUIDDATA>& arrayData)
{
    CWizApi::onDeletedPostList(arrayData);
    //
    uploadNextDeleteds();
}

void CWizSync::onTagPostList(const std::deque<WIZTAGDATA>& arrayData)
{
    CWizApi::onTagPostList(arrayData);
    //
    uploadNextTags();
}

void CWizSync::onStylePostList(const std::deque<WIZSTYLEDATA>& arrayData)
{
    CWizApi::onStylePostList(arrayData);
    //
    uploadNextStyles();
}

void CWizSync::onDocumentsGetInfo(const std::deque<WIZDOCUMENTDATABASE>& arrayRet)
{
    size_t count = arrayRet.size();
    //
    if (0 == count)
    {
        //new document
        WIZDOCUMENTDATABASE data;
        data.strGUID = m_currentUploadDocument.strGUID;
        data.strTitle = m_currentUploadDocument.strTitle;
        data.strLocation = m_currentUploadDocument.strLocation;
        data.tInfoModified = COleDateTime(1900, 1, 1, 0, 0, 0);
        data.tDataModified = data.tInfoModified;
        data.tParamModified = data.tInfoModified;
        data.strInfoMD5 = "-1";
        data.strDataMD5 = "-1";
        data.strParamMD5 = "-1";
        //
        onQueryDocumentInfo(data);
    }
    else if (1 == count)
    {
        onQueryDocumentInfo(arrayRet[0]);
    }
    else
    {
        ATLASSERT(FALSE);
        onXmlRpcError(SyncMethod_GetDocumentsInfo, errorXmlRpcFault, -1, "Fault error: Invalid document info");
        addErrorLog("Can not query document info");
    }
}
void CWizSync::onAttachmentsGetInfo(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
{
    size_t count = arrayRet.size();
    //
    if (0 == count)
    {
        //new document
        WIZDOCUMENTATTACHMENTDATA data;
        data.strGUID = m_currentUploadAttachment.strGUID;
        data.strDocumentGUID = m_currentUploadAttachment.strDocumentGUID;
        data.strName = m_currentUploadDocument.strName;
        data.tInfoModified = COleDateTime(1900, 1, 1, 0, 0, 0);
        data.tDataModified = data.tInfoModified;
        data.strInfoMD5 = "-1";
        data.strDataMD5 = "-1";
        //
        onQueryAttachmentInfo(data);
    }
    else if (1 == count)
    {
        onQueryAttachmentInfo(arrayRet[0]);
    }
    else
    {
        ATLASSERT(FALSE);
        onXmlRpcError(SyncMethod_GetDocumentsInfo, errorXmlRpcFault, -1, "Fault error: Invalid document info");
        addErrorLog("Can not query document info");
    }
}


void CWizSync::onDeletedGetList(const std::deque<WIZDELETEDGUIDDATA>& arrayRet)
{
    CWizApi::onDeletedGetList(arrayRet);
    //
    if (arrayRet.size() < getCountPerPage())
    {
        onDownloadDeletedsCompleted();
    }
    else
    {
        downloadNextDeleteds(WizObjectsGetMaxVersion<WIZDELETEDGUIDDATA>(arrayRet));
    }
}

void CWizSync::onTagGetList(const std::deque<WIZTAGDATA>& arrayRet)
{
    CWizApi::onTagGetList(arrayRet);
    //
    if (arrayRet.size() < getCountPerPage())
    {
        onDownloadTagsCompleted();
    }
    else
    {
        downloadNextTags(WizObjectsGetMaxVersion<WIZTAGDATA>(arrayRet));
    }
}

void CWizSync::onStyleGetList(const std::deque<WIZSTYLEDATA>& arrayRet)
{
    CWizApi::onStyleGetList(arrayRet);
    //
    if (arrayRet.size() < getCountPerPage())
    {
        onDownloadStylesCompleted();
    }
    else
    {
        downloadNextStyles(WizObjectsGetMaxVersion<WIZSTYLEDATA>(arrayRet));
    }
}

void CWizSync::onDocumentGetList(const std::deque<WIZDOCUMENTDATABASE>& arrayRet)
{
    CWizApi::onDocumentGetList(arrayRet);
    //
    m_arrayAllDocumentsNeedToBeDownloaded.insert(m_arrayAllDocumentsNeedToBeDownloaded.end(), arrayRet.begin(), arrayRet.end());
    //
    if (arrayRet.size() < getCountPerPage())
    {
        onDownloadDocumentsSimpleInfoCompleted();
    }
    else
    {
        downloadNextDocumentsSimpleInfo(WizObjectsGetMaxVersion<WIZDOCUMENTDATABASE>(arrayRet));
    }
}

void CWizSync::onAttachmentGetList(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
{
    CWizApi::onAttachmentGetList(arrayRet);
    //
    if (arrayRet.size() < getCountPerPage())
    {
        onDownloadAttachmentsInfoCompleted();
    }
    else
    {
        downloadNextAttachmentsInfo(WizObjectsGetMaxVersion<WIZDOCUMENTATTACHMENTDATAEX>(arrayRet));
    }
}
void CWizSync::downloadNextDocumentFullInfo()
{
    if (m_arrayAllDocumentsNeedToBeDownloaded.empty())
    {
        onDownloadDocumentsFullInfoCompleted();
    }
    else
    {
        changeProgressEx(m_arrayAllDocumentsNeedToBeDownloaded.size(), m_nAllDocumentsNeedToBeDownloadedCount, DOWNLOAD_DOCUMENT_FULL_INFO_STEP_BEGIN, DOWNLOAD_DOCUMENT_FULL_INFO_STEP_COUNT);
        //
        WIZDOCUMENTDATABASE data = m_arrayAllDocumentsNeedToBeDownloaded[0];
        m_arrayAllDocumentsNeedToBeDownloaded.erase(m_arrayAllDocumentsNeedToBeDownloaded.begin());
        //
        callDocumentGetData(data);
    }
}

void CWizSync::onDocumentGetData(const WIZDOCUMENTDATAEX& data)
{
    //CWizApi::onDocumentGetData(data);
    if (!m_db.UpdateDocument(data, &m_events))
    {
        m_bDocumentInfoError = TRUE;
    }
    //
    downloadNextDocumentFullInfo();
}


void CWizSync::downloadNextObjectData()
{
    if (m_arrayAllObjectsNeedToBeDownloaded.empty())
    {
        onDownloadObjectsDataCompleted();
    }
    else
    {
        ATLASSERT(m_nAllObjectsNeedToBeDownloadedCount > 0);
        changeProgressEx(m_arrayAllObjectsNeedToBeDownloaded.size(), m_nAllObjectsNeedToBeDownloadedCount, DOWNLOAD_OBJECT_STEP_BEGIN, DOWNLOAD_OBJECT_STEP_COUNT);
        //
        WIZOBJECTDATA data = *m_arrayAllObjectsNeedToBeDownloaded.begin();
        m_arrayAllObjectsNeedToBeDownloaded.erase(m_arrayAllObjectsNeedToBeDownloaded.begin());
        //
        downloadObjectData(data);
    }
}

void CWizSync::onDownloadObjectDataCompleted(const WIZOBJECTDATA& data)
{
    CWizApi::onDownloadObjectDataCompleted(data);
    //
    downloadNextObjectData();
}

//upload document
//1: upload info
//2: upload data
void CWizSync::queryDocumentInfo(const CString& strGUID, const CString& strTitle)
{
    addLog("query note info: " + strTitle);
    CWizStdStringArray arrayGUID;
    arrayGUID.push_back(strGUID);
    callDocumentsGetInfo(arrayGUID);
}

void CWizSync::onQueryDocumentInfo(const WIZDOCUMENTDATABASE& data)
{
    ATLASSERT(data.strGUID == m_currentUploadDocument.strGUID);
    if (data.strGUID != m_currentUploadDocument.strGUID)
    {
        onXmlRpcError(SyncMethod_GetDocumentsInfo, errorXmlRpcFault, -1, "Fault error: can not query document info");
        return;
    }
    //
    UINT nParts = 0;
    if (data.strInfoMD5 != m_currentUploadDocument.strInfoMD5)
    {
        nParts |= WIZKM_XMLRPC_OBJECT_PART_INFO;
    }
    //
    if (data.strDataMD5 != m_currentUploadDocument.strDataMD5)
    {
        nParts |= WIZKM_XMLRPC_OBJECT_PART_DATA;
    }
    //
    if (data.strParamMD5 != m_currentUploadDocument.strParamMD5)
    {
        nParts |= WIZKM_XMLRPC_OBJECT_PART_PARAM;
    }
    //
    if (0 == nParts)
    {
        onUploadDocument(m_currentUploadDocument);
    }
    else
    {
        m_currentUploadDocument.nObjectPart = nParts;
        uploadDocument(m_currentUploadDocument);
    }
}

void CWizSync::onUploadDocument(const WIZDOCUMENTDATAEX& data)
{
    m_db.ModifyObjectVersion(data.strGUID, WIZDOCUMENTDATAEX::ObjectName(), 0);
    uploadNextDocument();
}

//upload atachment
//1: upload info
//2: upload data

void CWizSync::queryAttachmentInfo(const CString& strGUID, const CString& strName)
{
    addLog("query attachment info: " + strName);
    CWizStdStringArray arrayGUID;
    arrayGUID.push_back(strGUID);
    callAttachmentsGetInfo(arrayGUID);
}

void CWizSync::onQueryAttachmentInfo(const WIZDOCUMENTATTACHMENTDATA& data)
{
    ATLASSERT(data.strGUID == m_currentUploadAttachment.strGUID);
    if (data.strGUID != m_currentUploadAttachment.strGUID)
    {
        onXmlRpcError(SyncMethod_GetAttachmentsInfo, errorXmlRpcFault, -1, "Fault error: can not query attachment info");
        return;
    }
    //
    UINT nParts = 0;
    if (data.strInfoMD5 != m_currentUploadAttachment.strInfoMD5)
    {
        nParts |= WIZKM_XMLRPC_OBJECT_PART_INFO;
    }
    //
    if (data.strDataMD5 != m_currentUploadAttachment.strDataMD5)
    {
        nParts |= WIZKM_XMLRPC_OBJECT_PART_DATA;
    }
    //
    if (0 == nParts)
    {
        onUploadAttachment(m_currentUploadAttachment);
    }
    else
    {
        m_currentUploadAttachment.nObjectPart = nParts;
        uploadAttachment(m_currentUploadAttachment);
    }
}


void CWizSync::onUploadAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data)
{
    m_db.ModifyObjectVersion(data.strGUID, WIZDOCUMENTATTACHMENTDATAEX::ObjectName(), 0);
    uploadNextAttachment();
}

//////////////////////////////////////////////////////
BOOL CWizSync::downloadDocument(const WIZDOCUMENTDATABASE& data)
{
    int nPart = data.nObjectPart;
    ATLASSERT(nPart != 0);
    //
    return callDocumentGetData(data);
}



BOOL CWizSync::updateDocument(const WIZDOCUMENTDATABASE& data)
{
    Q_UNUSED(data);
    return TRUE;
}

BOOL CWizSync::updateDocuments(const std::deque<WIZDOCUMENTDATABASE>& arrayDocument)
{
    Q_UNUSED(arrayDocument);
    return TRUE;
}

bool compareDocumentByTime(const WIZDOCUMENTDATABASE& data1, const WIZDOCUMENTDATABASE& data2)
{
    return data1.tDataModified > data2.tDataModified;
}

void CWizSync::filterDocuments()
{
    //
    //compare document md5 (info, param, data)
    //
    size_t nCount = m_arrayAllDocumentsNeedToBeDownloaded.size();
    for (intptr_t i = nCount - 1; i >= 0; i--)
    {
        WIZDOCUMENTDATABASE& data = m_arrayAllDocumentsNeedToBeDownloaded[i];
        data.nObjectPart = calObjectPart(data);
        //
        //update data
        //
        if (data.nObjectPart & WIZKM_XMLRPC_OBJECT_PART_DATA)
        {
            data.nObjectPart &= ~WIZKM_XMLRPC_OBJECT_PART_DATA;
            //
            //set document need to be downloaded
            //
            m_db.SetObjectDataDownloaded(data.strGUID, "document", false);
        }
        //
        //
        if (0 == data.nObjectPart)
        {
            m_arrayAllDocumentsNeedToBeDownloaded.erase(m_arrayAllDocumentsNeedToBeDownloaded.begin() + i);
            continue;
        }
    }
    //
    //sort by time
    //
    std::sort(m_arrayAllDocumentsNeedToBeDownloaded.begin(), m_arrayAllDocumentsNeedToBeDownloaded.end(), compareDocumentByTime);
}

int CWizSync::calObjectPart(const WIZDOCUMENTDATABASE& data)
{
    int nPart = 0;
    //
    WIZDOCUMENTDATA dataExists;
    if (m_db.DocumentFromGUID(data.strGUID, dataExists))
    {
        if (dataExists.strInfoMD5 != data.strInfoMD5)
        {
            nPart |= WIZKM_XMLRPC_OBJECT_PART_INFO;
        }
        if (dataExists.strDataMD5 != data.strDataMD5)
        {
            nPart |= WIZKM_XMLRPC_OBJECT_PART_DATA;
        }
        if (dataExists.strParamMD5 != data.strParamMD5)
        {
            nPart |= WIZKM_XMLRPC_OBJECT_PART_PARAM;
        }
    }
    else
    {
        nPart = WIZKM_XMLRPC_OBJECT_PART_INFO | WIZKM_XMLRPC_OBJECT_PART_DATA | WIZKM_XMLRPC_OBJECT_PART_PARAM;
    }
    //
    return nPart;
}

void CWizSync::changeProgressEx(size_t currentCount, size_t total, int stepStart, int stepCount)
{
    if (total <= 0)
        return;
    //
    int processed = total - currentCount;
    //
    int n = int(double(processed) / total * stepCount);
    //
    changeProgress(stepStart + n);
}
