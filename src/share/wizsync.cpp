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
    progressDone = 100
};

const int DOWNLOAD_DOCUMENT_FULL_INFO_STEP_BEGIN      = progressDocumentSimpleInfoDownloaded;
const int DOWNLOAD_DOCUMENT_FULL_INFO_STEP_COUNT      = progressDocumemtFullInfoDownloaded - progressDocumentSimpleInfoDownloaded;

const int UPLOAD_DOCUMENT_STEP_BEGIN                  = progressAttachmentInfoDownloaded;
const int UPLOAD_DOCUMENT_STEP_COUNT                  = progressDocumentUploaded - progressAttachmentInfoDownloaded;

const int UPLOAD_ATTACHMENT_STEP_BEGIN                = progressDocumentUploaded;
const int UPLOAD_ATTACHMENT_STEP_COUNT                = progressAttachmentUploaded - progressDocumentUploaded;

const int DOWNLOAD_OBJECT_STEP_BEGIN                 = progressAttachmentUploaded;
const int DOWNLOAD_OBJECT_STEP_COUNT                 = progressObjectDownloaded - progressAttachmentUploaded;


CWizSync::CWizSync(CWizDatabase& db, const CString& strAccountsApiURL)
    : CWizApi(db, strAccountsApiURL)
    , m_bSyncStarted(false)
{
}

void CWizSync::startSync()
{
    if (m_bSyncStarted)
        return;

    m_bSyncStarted = true;
    m_error = false;

    m_bChained = false;

    m_arrayAllDeletedsNeedToBeUploaded.clear();
    m_arrayAllTagsNeedToBeUploaded.clear();
    m_arrayAllStylesNeedToBeUploaded.clear();

    m_arrayAllDocumentsNeedToBeDownloaded.clear();
    m_nDocumentMaxVersion = -1;
    m_bDocumentInfoError = false;

    m_arrayAllDocumentsNeedToBeUploaded.clear();
    m_currentUploadDocument = WIZDOCUMENTDATAEX();

    m_arrayAllAttachmentsNeedToBeUploaded.clear();
    m_currentUploadAttachment = WIZDOCUMENTATTACHMENTDATAEX();

    m_arrayAllObjectsNeedToBeDownloaded.clear();

    Q_EMIT syncStarted();
    Q_EMIT progressChanged(progressStart);
    Q_EMIT processLog(tr("begin syning"));

    callClientLogin(m_db.getUserId(), m_db.getPassword());
}

void CWizSync::abort()
{
    CWizApi::abort();

    Q_EMIT processLog(tr("abort syncing, disconnct from server"));
    Q_EMIT syncDone(m_error);
}

void CWizSync::onXmlRpcError(const QString& strMethodName, WizXmlRpcError err, int errorCode, const QString& errorMessage)
{
    CWizApi::onXmlRpcError(strMethodName, err, errorCode, errorMessage);

    m_error = true;

    if (strMethodName != SyncMethod_ClientLogout) {
        stopSync();
    }
}

void CWizSync::onClientLogin()
{
    Q_EMIT syncLogined();

    startDownloadDeleteds();
}

void CWizSync::startDownloadDeleteds()
{
    Q_EMIT processLog(tr("downloading deleted objects list"));
    Q_EMIT progressChanged(progressOnLogin);
    downloadNextDeleteds(m_db.GetObjectVersion(WIZDELETEDGUIDDATA::ObjectName()));
}

void CWizSync::onDownloadDeletedsCompleted()
{
    startUploadDeleteds();
}

void CWizSync::startUploadDeleteds()
{
    Q_EMIT processLog(tr("uploading deleted objects list"));
    Q_EMIT progressChanged(progressDeletedsDownloaded);

    m_db.GetModifiedDeletedGUIDs(m_arrayAllDeletedsNeedToBeUploaded);
    uploadNextDeleteds();
}

void CWizSync::onUploadDeletedsCompleted()
{
    startDownloadTags();
}

void CWizSync::startDownloadTags()
{
    Q_EMIT processLog(tr("downloading tags"));
    Q_EMIT progressChanged(progressDeletedsUploaded);

    downloadNextTags(m_db.GetObjectVersion(WIZTAGDATA::ObjectName()));
}

void CWizSync::onDownloadTagsCompleted()
{
    startUploadTags();
}

void CWizSync::startUploadTags()
{
    Q_EMIT processLog(tr("uploading tags"));
    Q_EMIT progressChanged(progressTagsDownloaded);

    m_db.GetModifiedTags(m_arrayAllTagsNeedToBeUploaded);
    uploadNextTags();
}

void CWizSync::onUploadTagsCompleted()
{
    startDownloadStyles();
}

void CWizSync::startDownloadStyles()
{
    Q_EMIT processLog(tr("downloading styles"));
    Q_EMIT progressChanged(progressTagsUploaded);

    downloadNextStyles(m_db.GetObjectVersion(WIZSTYLEDATA::ObjectName()));
}

void CWizSync::onDownloadStylesCompleted()
{
    startUploadStyles();
}

void CWizSync::startUploadStyles()
{
    Q_EMIT processLog(tr("uploading styles"));
    Q_EMIT progressChanged(progressStyleDownloaded);

    m_db.GetModifiedStyles(m_arrayAllStylesNeedToBeUploaded);
    uploadNextStyles();
}

void CWizSync::onUploadStylesCompleted()
{
    startDownloadDocumentsSimpleInfo();
}

void CWizSync::startDownloadDocumentsSimpleInfo()
{
    Q_EMIT processLog(tr("downloading notes list"));
    Q_EMIT progressChanged(progressStyleUploaded);

    downloadNextDocumentsSimpleInfo(m_db.GetObjectVersion(WIZDOCUMENTDATA::ObjectName()));
}

void CWizSync::onDownloadDocumentsSimpleInfoCompleted()
{
    Q_EMIT progressChanged(progressDocumentSimpleInfoDownloaded);

    TOLOG1(tr("Total %1 documents need to be update"), QString::number(m_arrayAllDocumentsNeedToBeDownloaded.size()));

    //save max version of document
    //if no error occured while downloading document full information
    //then update this version
    if (!m_arrayAllDocumentsNeedToBeDownloaded.empty()) {
        m_nDocumentMaxVersion = ::WizObjectsGetMaxVersion<WIZDOCUMENTDATABASE>(m_arrayAllDocumentsNeedToBeDownloaded);
    }

    //filter documents for getting document full information (not data)
    filterDocuments();

    startUploadDocuments();
}

void CWizSync::startUploadDocuments()
{
    Q_EMIT processLog(tr("uploading documents"));
    Q_EMIT progressChanged(progressAttachmentInfoDownloaded);

    m_db.GetModifiedDocuments(m_arrayAllDocumentsNeedToBeUploaded);
    uploadNextDocument();
}

void CWizSync::onUploadDocumentsCompleted()
{
    startDownloadAttachmentsInfo();
}

void CWizSync::startDownloadAttachmentsInfo()
{
    Q_EMIT processLog(tr("downloading attachments information"));

    downloadNextAttachmentsInfo(m_db.GetObjectVersion(WIZDOCUMENTATTACHMENTDATA::ObjectName()));
}

void CWizSync::onDownloadAttachmentsInfoCompleted()
{
    startUploadAttachments();
}

void CWizSync::startUploadAttachments()
{
    Q_EMIT processLog(tr("uploading attachments"));
    Q_EMIT progressChanged(progressDocumentUploaded);

    m_db.GetModifiedAttachments(m_arrayAllAttachmentsNeedToBeUploaded);
    uploadNextAttachment();
}

void CWizSync::onUploadAttachmentsCompleted()
{
    startDownloadDocumentsFullInfo();
}

void CWizSync::startDownloadDocumentsFullInfo()
{
    Q_EMIT processLog(tr("downloading notes information"));

    // Note: Chained download needed from here.
    m_bChained = true;

    downloadNextDocumentFullInfo();
}

void CWizSync::onDownloadDocumentsFullInfoCompleted()
{
    Q_EMIT progressChanged(progressDocumemtFullInfoDownloaded);

    //if no error occured while downloading document full information
    //update document version
    if (!m_bDocumentInfoError && -1 != m_nDocumentMaxVersion) {
        m_db.SetObjectVersion(WIZDOCUMENTDATA::ObjectName(), m_nDocumentMaxVersion);
    }

    startDownloadObjectsData();
}

void CWizSync::startDownloadObjectsData()
{
    Q_EMIT processLog(tr("downloading objects data"));
    Q_EMIT progressChanged(progressAttachmentUploaded);

    m_db.GetAllObjectsNeedToBeDownloaded(m_arrayAllObjectsNeedToBeDownloaded);

    if (!m_bDownloadAllNotesData)
    {
        COleDateTime tNow = ::WizGetCurrentTime();

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

            if (m_arrayAllObjectsNeedToBeDownloaded[i].eObjectType == wizobjectDocumentAttachment)
            {
                m_arrayAllObjectsNeedToBeDownloaded.erase(m_arrayAllObjectsNeedToBeDownloaded.begin() + i);
                continue;
            }
        }
    }

    downloadNextObjectData();
}

void CWizSync::onDownloadObjectsDataCompleted()
{
    Q_EMIT progressChanged(progressObjectDownloaded);

    stopSync();
}

void CWizSync::stopSync()
{
    // network error occured
    if (m_user.strToken.isEmpty()) {
        Q_EMIT processLog(tr("network error occured when syncing, please verify your network connection"));
        Q_EMIT progressChanged(progressDone);
        Q_EMIT syncDone(m_error);
        m_bSyncStarted = false;
        return;
    }

    callClientLogout();
}

void CWizSync::onClientLogout()
{
    CWizApi::onClientLogout();

    Q_EMIT processLog(tr("sync done"));
    Q_EMIT progressChanged(progressDone);
    Q_EMIT syncDone(m_error);
    m_bSyncStarted = false;
}


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
    if (m_arrayAllDeletedsNeedToBeUploaded.empty()) {
        onUploadDeletedsCompleted();
    } else {
        int countPerPage = getCountPerPage();

        CWizDeletedGUIDDataArray arrayCurr;
        if (m_arrayAllDeletedsNeedToBeUploaded.size() > (size_t)countPerPage) {
            arrayCurr.assign(m_arrayAllDeletedsNeedToBeUploaded.begin(), \
                             m_arrayAllDeletedsNeedToBeUploaded.begin() + countPerPage);
            m_arrayAllDeletedsNeedToBeUploaded.erase(m_arrayAllDeletedsNeedToBeUploaded.begin(), \
                                                     m_arrayAllDeletedsNeedToBeUploaded.begin() + countPerPage);
        } else {
            arrayCurr.assign(m_arrayAllDeletedsNeedToBeUploaded.begin(), \
                             m_arrayAllDeletedsNeedToBeUploaded.end());
            m_arrayAllDeletedsNeedToBeUploaded.clear();
        }

        callDeletedPostList(arrayCurr);
    }
}

void CWizSync::uploadNextTags()
{
    if (m_arrayAllTagsNeedToBeUploaded.empty()) {
        onUploadTagsCompleted();
    } else {
        int countPerPage = getCountPerPage();

        CWizTagDataArray arrayCurr;
        if (m_arrayAllTagsNeedToBeUploaded.size() > (size_t)countPerPage) {
            arrayCurr.assign(m_arrayAllTagsNeedToBeUploaded.begin(), \
                             m_arrayAllTagsNeedToBeUploaded.begin() + countPerPage);
            m_arrayAllTagsNeedToBeUploaded.erase(m_arrayAllTagsNeedToBeUploaded.begin(), \
                                                 m_arrayAllTagsNeedToBeUploaded.begin() + countPerPage);
        } else {
            arrayCurr.assign(m_arrayAllTagsNeedToBeUploaded.begin(), \
                             m_arrayAllTagsNeedToBeUploaded.end());
            m_arrayAllTagsNeedToBeUploaded.clear();
        }

        callTagPostList(arrayCurr);
    }
}

void CWizSync::uploadNextStyles()
{
    if (m_arrayAllStylesNeedToBeUploaded.empty()) {
        onUploadStylesCompleted();
    } else {
        int countPerPage = getCountPerPage();

        CWizStyleDataArray arrayCurr;
        if (m_arrayAllStylesNeedToBeUploaded.size() > (size_t)countPerPage) {
            arrayCurr.assign(m_arrayAllStylesNeedToBeUploaded.begin(), \
                             m_arrayAllStylesNeedToBeUploaded.begin() + countPerPage);
            m_arrayAllStylesNeedToBeUploaded.erase(m_arrayAllStylesNeedToBeUploaded.begin(), \
                                                   m_arrayAllStylesNeedToBeUploaded.begin() + countPerPage);
        } else {
            arrayCurr.assign(m_arrayAllStylesNeedToBeUploaded.begin(), \
                             m_arrayAllStylesNeedToBeUploaded.end());
            m_arrayAllStylesNeedToBeUploaded.clear();
        }

        callStylePostList(arrayCurr);
    }
}

void CWizSync::uploadNextDocument()
{
    if (m_arrayAllDocumentsNeedToBeUploaded.empty()) {
        onUploadDocumentsCompleted();
    } else {
        m_currentUploadDocument = m_arrayAllDocumentsNeedToBeUploaded[0];
        m_arrayAllDocumentsNeedToBeUploaded.erase(m_arrayAllDocumentsNeedToBeUploaded.begin());

        queryDocumentInfo(m_currentUploadDocument.strGUID, m_currentUploadDocument.strTitle);
    }
}

void CWizSync::uploadNextAttachment()
{
    if (m_arrayAllAttachmentsNeedToBeUploaded.empty()) {
        onUploadAttachmentsCompleted();
    } else {
        m_currentUploadAttachment = m_arrayAllAttachmentsNeedToBeUploaded[0];
        m_arrayAllAttachmentsNeedToBeUploaded.erase(m_arrayAllAttachmentsNeedToBeUploaded.begin());

        queryAttachmentInfo(m_currentUploadAttachment.strGUID, m_currentUploadAttachment.strName);
    }
}

void CWizSync::onDeletedPostList(const std::deque<WIZDELETEDGUIDDATA>& arrayData)
{
    CWizApi::onDeletedPostList(arrayData);

    uploadNextDeleteds();
}

void CWizSync::onTagPostList(const std::deque<WIZTAGDATA>& arrayData)
{
    CWizApi::onTagPostList(arrayData);

    uploadNextTags();
}

void CWizSync::onStylePostList(const std::deque<WIZSTYLEDATA>& arrayData)
{
    CWizApi::onStylePostList(arrayData);

    uploadNextStyles();
}

void CWizSync::onDocumentsGetInfo(const std::deque<WIZDOCUMENTDATABASE>& arrayRet)
{
    size_t count = arrayRet.size();

    // new document
    if (count == 0) {
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

        onQueryDocumentInfo(data);

    // server already have this document
    } else if (count == 1) {
        onQueryDocumentInfo(arrayRet[0]);

    // absolutely count should not more than 1
    } else {
        Q_EMIT processErrorLog("Can not query document info");
        onXmlRpcError(SyncMethod_GetDocumentsInfo, errorXmlRpcFault, -1, "Fault error: Invalid document info");
    }
}

void CWizSync::onAttachmentsGetInfo(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
{
    size_t count = arrayRet.size();

    //new document
    if (count == 0) {
        WIZDOCUMENTATTACHMENTDATA data;
        data.strGUID = m_currentUploadAttachment.strGUID;
        data.strDocumentGUID = m_currentUploadAttachment.strDocumentGUID;
        data.strName = m_currentUploadAttachment.strName;
        data.tInfoModified = COleDateTime(1900, 1, 1, 0, 0, 0);
        data.tDataModified = data.tInfoModified;
        data.strInfoMD5 = "-1";
        data.strDataMD5 = "-1";

        onQueryAttachmentInfo(data);
    } else if (count == 1) {
        onQueryAttachmentInfo(arrayRet[0]);
    } else {
        Q_EMIT processErrorLog("Can not query document info");
        onXmlRpcError(SyncMethod_GetDocumentsInfo, errorXmlRpcFault, -1, "Fault error: Invalid document info");
    }
}

void CWizSync::onDeletedGetList(const std::deque<WIZDELETEDGUIDDATA>& arrayRet)
{
    CWizApi::onDeletedGetList(arrayRet);

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

    m_arrayAllDocumentsNeedToBeDownloaded.insert(m_arrayAllDocumentsNeedToBeDownloaded.end(), arrayRet.begin(), arrayRet.end());

    if (arrayRet.size() < getCountPerPage()) {
        onDownloadDocumentsSimpleInfoCompleted();
    } else {
        downloadNextDocumentsSimpleInfo(WizObjectsGetMaxVersion<WIZDOCUMENTDATABASE>(arrayRet));
    }
}

void CWizSync::onAttachmentGetList(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
{
    CWizApi::onAttachmentGetList(arrayRet);

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
    if (m_arrayAllDocumentsNeedToBeDownloaded.empty()) {
        onDownloadDocumentsFullInfoCompleted();
    } else {
        WIZDOCUMENTDATABASE data = m_arrayAllDocumentsNeedToBeDownloaded[0];
        m_arrayAllDocumentsNeedToBeDownloaded.erase(m_arrayAllDocumentsNeedToBeDownloaded.begin());

        callDocumentGetData(data);
    }
}

void CWizSync::onDocumentGetData(const WIZDOCUMENTDATAEX& data)
{
    //CWizApi::onDocumentGetData(data);

    if (m_bChained) {
        if (!m_db.UpdateDocument(data)) {
            m_bDocumentInfoError = true;
        }

        downloadNextDocumentFullInfo();
    } else {
        processDocumentData(data);
    }
}

void CWizSync::downloadNextObjectData()
{
    if (m_arrayAllObjectsNeedToBeDownloaded.empty()) {
        onDownloadObjectsDataCompleted();
    } else {
        WIZOBJECTDATA data = *m_arrayAllObjectsNeedToBeDownloaded.begin();
        m_arrayAllObjectsNeedToBeDownloaded.erase(m_arrayAllObjectsNeedToBeDownloaded.begin());

        downloadObjectData(data);
    }
}

void CWizSync::onDownloadObjectDataCompleted(const WIZOBJECTDATA& data)
{
    //CWizApi::onDownloadObjectDataCompleted(data);

    if (m_bChained) {
        m_db.UpdateSyncObjectLocalData(data);
        downloadNextObjectData();
    } else {
        processObjectData(data);
    }
}

void CWizSync::queryDocumentInfo(const CString& strGUID, const CString& strTitle)
{
    // if current modified document already inside need download list, means confilict found!
    // no need to query document info anymore.
    std::deque<WIZDOCUMENTDATABASE>::const_iterator it;
    for (it = m_arrayAllDocumentsNeedToBeDownloaded.begin(); \
         it != m_arrayAllDocumentsNeedToBeDownloaded.end();
         it++) {
        WIZDOCUMENTDATABASE data = *it;

        if (data.strGUID == strGUID) {
            Q_EMIT processLog(tr("Conflict found: ") + strTitle);
            m_conflictedDocument = data;
            callDocumentGetData(data);
            return;
        }
    }

    Q_EMIT processLog(tr("query note info: ") + strTitle);

    CWizStdStringArray arrayGUID;
    arrayGUID.push_back(strGUID);
    callDocumentsGetInfo(arrayGUID);
}

void CWizSync::processDocumentData(const WIZDOCUMENTDATAEX& data)
{
    // to avoid unable to download document object data issue.
    m_conflictDownloadedInfo = data;

    WIZOBJECTDATA objectData(data);
    downloadObjectData(objectData);
}

void CWizSync::processObjectData(const WIZOBJECTDATA& data)
{
    WIZOBJECTDATA conflictObjectData(data);
    conflictObjectData.strObjectGUID = WizGenGUIDLowerCaseLetterOnly();
    conflictObjectData.strDisplayName += tr("(conflict backup)");

    // set dirty flag, uploaded needed
    m_conflictDownloadedInfo.nVersion = -1;
    m_conflictDownloadedInfo.strGUID = conflictObjectData.strObjectGUID;
    m_conflictDownloadedInfo.strTitle += tr("(conflict backup)");
    m_conflictDownloadedInfo.strInfoMD5 = m_db.CalDocumentInfoMD5(m_conflictDownloadedInfo);

    if (m_db.CreateDocumentEx(m_conflictDownloadedInfo)) {
        m_db.UpdateSyncObjectLocalData(conflictObjectData);
        TOLOG1(tr("Conflict backup created: %1"), m_conflictDownloadedInfo.strTitle);
    } else {
        TOLOG("unable to create conflict backup while create document");
    }

    std::deque<WIZDOCUMENTDATABASE>::iterator it;
    for (it = m_arrayAllDocumentsNeedToBeDownloaded.begin(); \
         it != m_arrayAllDocumentsNeedToBeDownloaded.end();
         it++) {
        WIZDOCUMENTDATABASE data = *it;

        if (data.strGUID == m_conflictedDocument.strGUID) {
            m_arrayAllDocumentsNeedToBeDownloaded.erase(it);
            break;
        }
    }

    // chain back
    onQueryDocumentInfo(m_conflictedDocument);
}

void CWizSync::onQueryDocumentInfo(const WIZDOCUMENTDATABASE& data)
{
    int nParts = calDocumentParts(data, m_currentUploadDocument);
    if (0 == nParts) {
        onUploadDocument(m_currentUploadDocument);
    } else {
        m_currentUploadDocument.nObjectPart = nParts;
        uploadDocument(m_currentUploadDocument);
    }
}

void CWizSync::onUploadDocument(const WIZDOCUMENTDATAEX& data)
{
    m_db.ModifyObjectVersion(data.strGUID, WIZDOCUMENTDATAEX::ObjectName(), 0);
    uploadNextDocument();
}

void CWizSync::queryAttachmentInfo(const CString& strGUID, const CString& strName)
{
    Q_EMIT processLog(tr("query attachment info: ") + strName);

    CWizStdStringArray arrayGUID;
    arrayGUID.push_back(strGUID);
    callAttachmentsGetInfo(arrayGUID);
}

void CWizSync::onQueryAttachmentInfo(const WIZDOCUMENTATTACHMENTDATA& data)
{
    int nParts = calAttachmentParts(data, m_currentUploadAttachment);
    if (0 == nParts) {
        onUploadAttachment(m_currentUploadAttachment);
    } else {
        m_currentUploadAttachment.nObjectPart = nParts;
        uploadAttachment(m_currentUploadAttachment);
    }
}

void CWizSync::onUploadAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data)
{
    m_db.ModifyObjectVersion(data.strGUID, WIZDOCUMENTATTACHMENTDATAEX::ObjectName(), 0);
    uploadNextAttachment();
}

bool CWizSync::downloadDocument(const WIZDOCUMENTDATABASE& data)
{
    int nPart = data.nObjectPart;
    Q_ASSERT(nPart != 0);

    return callDocumentGetData(data);
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

        WIZDOCUMENTDATA dataExists;
        if (!m_db.DocumentFromGUID(data.strGUID, dataExists)) {
            data.nObjectPart = WIZKM_XMLRPC_OBJECT_PART_INFO | \
                    WIZKM_XMLRPC_OBJECT_PART_DATA | \
                    WIZKM_XMLRPC_OBJECT_PART_PARAM;
        } else {
            data.nObjectPart = calDocumentParts(dataExists, data);
        }

        //set document need to be downloaded
        if (data.nObjectPart & WIZKM_XMLRPC_OBJECT_PART_DATA)
        {
            data.nObjectPart &= ~WIZKM_XMLRPC_OBJECT_PART_DATA;
            m_db.SetObjectDataDownloaded(data.strGUID, "document", false);
        }

        if (0 == data.nObjectPart) {
            m_arrayAllDocumentsNeedToBeDownloaded.erase(m_arrayAllDocumentsNeedToBeDownloaded.begin() + i);
            continue;
        }
    }

    //sort by time
    std::sort(m_arrayAllDocumentsNeedToBeDownloaded.begin(), m_arrayAllDocumentsNeedToBeDownloaded.end(), compareDocumentByTime);
}

int CWizSync::calDocumentParts(const WIZDOCUMENTDATABASE& sourceData, \
                               const WIZDOCUMENTDATABASE& destData)
{
    int nParts = 0;

    if (sourceData.strInfoMD5 != destData.strInfoMD5) {
        nParts |= WIZKM_XMLRPC_OBJECT_PART_INFO;
    }

    if (sourceData.strDataMD5 != destData.strDataMD5) {
        nParts |= WIZKM_XMLRPC_OBJECT_PART_DATA;
    }

    if (sourceData.strParamMD5 != destData.strParamMD5) {
        nParts |= WIZKM_XMLRPC_OBJECT_PART_PARAM;
    }

    return nParts;
}

int CWizSync::calAttachmentParts(const WIZDOCUMENTATTACHMENTDATA& sourceData, \
                                 const WIZDOCUMENTATTACHMENTDATA& destData)
{
    int nParts = 0;

    if (sourceData.strInfoMD5 != destData.strInfoMD5) {
        nParts |= WIZKM_XMLRPC_OBJECT_PART_INFO;
    }

    if (sourceData.strDataMD5 != destData.strDataMD5) {
        nParts |= WIZKM_XMLRPC_OBJECT_PART_DATA;
    }

    return nParts;
}
