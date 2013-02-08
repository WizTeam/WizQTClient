#include "wizKbSync.h"
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


CWizKbSync::CWizKbSync(CWizDatabase& db, const CString& strKbUrl)
    : CWizApi(db, strKbUrl)
    , m_bSyncStarted(false)
{
}

void CWizKbSync::startSync(const QString& strKbGUID)
{
    Q_ASSERT(!strKbGUID.isEmpty());

    if (m_bSyncStarted)
        return;

    m_bSyncStarted = true;
    m_error = false;

    m_bChained = false;

    m_arrayAllDeletedsDownloaded.clear();
    m_arrayAllDeletedsNeedToBeUploaded.clear();
    m_arrayAllTagsDownloaded.clear();
    m_arrayAllTagsNeedToBeUploaded.clear();
    m_arrayAllStylesDownloaded.clear();
    m_arrayAllStylesNeedToBeUploaded.clear();

    m_arrayAllDocumentsNeedToBeDownloaded.clear();
    m_nDocumentMaxVersion = -1;
    m_bDocumentInfoError = false;

    m_arrayAllDocumentsNeedToBeUploaded.clear();
    m_currentUploadDocument = WIZDOCUMENTDATAEX();

    m_arrayAllAttachmentsDownloaded.clear();

    m_arrayAllAttachmentsNeedToBeUploaded.clear();
    m_currentUploadAttachment = WIZDOCUMENTATTACHMENTDATAEX();

    m_arrayAllObjectsNeedToBeDownloaded.clear();

    Q_EMIT progressChanged(progressStart);

    setKbUrl(database()->server());

    if (!strKbGUID.isEmpty())
        setKbGUID(strKbGUID);

    startDownloadDeleteds();
}

void CWizKbSync::abort()
{
    CWizApi::abort();

    Q_EMIT processLog(tr("abort syncing, disconnct from server"));
    Q_EMIT kbSyncDone(m_error);
}

void CWizKbSync::onXmlRpcError(const QString& strMethodName, WizXmlRpcError err, int errorCode, const QString& errorMessage)
{
    CWizApi::onXmlRpcError(strMethodName, err, errorCode, errorMessage);

    m_error = true;

    if (strMethodName != SyncMethod_ClientLogout) {
        stopSync();
    }
}

//void CWizKbSync::onClientLogin(const WIZUSERINFO& userInfo)
//{
//    CWizApi::onClientLogin(userInfo);
//
//    //Q_EMIT syncLogined();
//    startDownloadDeleteds();
//}

void CWizKbSync::startDownloadDeleteds()
{
    Q_EMIT processLog(tr("downloading deleted objects list"));

    Q_EMIT progressChanged(progressOnLogin);
    downloadNextDeleteds(m_db->GetObjectVersion(WIZDELETEDGUIDDATA::ObjectName()));
}

void CWizKbSync::onDownloadDeletedsCompleted()
{
    int nTotal = m_arrayAllDeletedsDownloaded.size();
    if (nTotal) {
        Q_EMIT processLog(WizFormatString1(tr("Total %1 deleted objects be synchronized"), nTotal));
    }

    startUploadDeleteds();
}

void CWizKbSync::startUploadDeleteds()
{
    m_db->GetModifiedDeletedGUIDs(m_arrayAllDeletedsNeedToBeUploaded);

    int nTotal = m_arrayAllDeletedsNeedToBeUploaded.size();
    if (nTotal) {
        Q_EMIT processLog(WizFormatString1(tr("uploading deleted objects list, total %1 deleted objects need upload"), nTotal));
    }

    Q_EMIT progressChanged(progressDeletedsDownloaded);
    uploadNextDeleteds();
}

void CWizKbSync::onUploadDeletedsCompleted()
{
    startDownloadTags();
}

void CWizKbSync::startDownloadTags()
{
    Q_EMIT processLog(tr("downloading tags list"));

    Q_EMIT progressChanged(progressDeletedsUploaded);
    downloadNextTags(m_db->GetObjectVersion(WIZTAGDATA::ObjectName()));
}

void CWizKbSync::onDownloadTagsCompleted()
{
    int nTotal = m_arrayAllTagsDownloaded.size();
    if (nTotal) {
        Q_EMIT processLog(WizFormatString1(tr("Total %1 tags be synchronized"), nTotal));
    }

    startUploadTags();
}

void CWizKbSync::startUploadTags()
{
    m_db->GetModifiedTags(m_arrayAllTagsNeedToBeUploaded);

    int nTotal = m_arrayAllTagsNeedToBeUploaded.size();
    if (nTotal) {
        Q_EMIT processLog(WizFormatString1(tr("uploading tags list, total %1 tags need upload"), nTotal));
    }

    Q_EMIT progressChanged(progressTagsDownloaded);
    uploadNextTags();
}

void CWizKbSync::onUploadTagsCompleted()
{
    startDownloadStyles();
}

void CWizKbSync::startDownloadStyles()
{
    Q_EMIT processLog(tr("downloading styles list"));

    Q_EMIT progressChanged(progressTagsUploaded);
    downloadNextStyles(m_db->GetObjectVersion(WIZSTYLEDATA::ObjectName()));
}

void CWizKbSync::onDownloadStylesCompleted()
{
    int nTotal = m_arrayAllStylesDownloaded.size();
    if (nTotal) {
        Q_EMIT processLog(WizFormatString1(tr("Total %1 styles be sychronized"), nTotal));
    }

    startUploadStyles();
}

void CWizKbSync::startUploadStyles()
{
    m_db->GetModifiedStyles(m_arrayAllStylesNeedToBeUploaded);

    int nTotal = m_arrayAllStylesNeedToBeUploaded.size();
    if (nTotal) {
        Q_EMIT processLog(WizFormatString1(tr("uploading styles list, total %1 styles need upload"), nTotal));
    }

    Q_EMIT progressChanged(progressStyleDownloaded);
    uploadNextStyles();
}

void CWizKbSync::onUploadStylesCompleted()
{
    startDownloadDocumentsSimpleInfo();
}

void CWizKbSync::startDownloadDocumentsSimpleInfo()
{
    Q_EMIT processLog(tr("downloading documents list"));

    Q_EMIT progressChanged(progressStyleUploaded);
    downloadNextDocumentsSimpleInfo(m_db->GetObjectVersion(WIZDOCUMENTDATA::ObjectName()));
}

void CWizKbSync::onDownloadDocumentsSimpleInfoCompleted()
{
    Q_EMIT progressChanged(progressDocumentSimpleInfoDownloaded);

    //save max version of document
    //if no error occured while downloading document full information
    //then update this version
    if (!m_arrayAllDocumentsNeedToBeDownloaded.empty()) {
        m_nDocumentMaxVersion = ::WizObjectsGetMaxVersion<WIZDOCUMENTDATABASE>(m_arrayAllDocumentsNeedToBeDownloaded);
    }

    //filter documents for getting document full information (not data)
    filterDocuments();

    int nTotal = m_arrayAllDocumentsNeedToBeDownloaded.size();
    if (nTotal) {
        Q_EMIT processLog(WizFormatString1(tr("Total %1 documents need to be synchronized"), nTotal));
    }

    startUploadDocuments();
}

void CWizKbSync::startUploadDocuments()
{
    m_db->GetModifiedDocuments(m_arrayAllDocumentsNeedToBeUploaded);

    int nTotal = m_arrayAllDocumentsNeedToBeUploaded.size();
    if (nTotal) {
        Q_EMIT processLog(WizFormatString1(tr("uploading documents, total %1 documents need upload"), nTotal));
    }

    Q_EMIT progressChanged(progressAttachmentInfoDownloaded);
    uploadNextDocument();
}

void CWizKbSync::onUploadDocumentsCompleted()
{
    startDownloadAttachmentsInfo();
}

void CWizKbSync::startDownloadAttachmentsInfo()
{
    Q_EMIT processLog(tr("downloading attachments list"));

    downloadNextAttachmentsInfo(m_db->GetObjectVersion(WIZDOCUMENTATTACHMENTDATA::ObjectName()));
}

void CWizKbSync::onDownloadAttachmentsInfoCompleted()
{
    int nTotal = m_arrayAllAttachmentsDownloaded.size();
    if (nTotal) {
        Q_EMIT processLog(WizFormatString1(tr("Total %1 attachments be synchronized"), nTotal));
    }

    startUploadAttachments();
}

void CWizKbSync::startUploadAttachments()
{
    m_db->GetModifiedAttachments(m_arrayAllAttachmentsNeedToBeUploaded);

    int nTotal = m_arrayAllAttachmentsNeedToBeUploaded.size();
    if (nTotal) {
        Q_EMIT processLog(WizFormatString1(tr("uploading attachments, total %1 attachments need upload"), nTotal));
    }

    Q_EMIT progressChanged(progressDocumentUploaded);
    uploadNextAttachment();
}

void CWizKbSync::onUploadAttachmentsCompleted()
{
    startDownloadDocumentsFullInfo();
}

void CWizKbSync::startDownloadDocumentsFullInfo()
{
    if (!m_arrayAllDocumentsNeedToBeDownloaded.empty()) {
        Q_EMIT processLog(tr("downloading documents info"));
    }

    // Note: Chained download needed from here.
    m_bChained = true;

    downloadNextDocumentFullInfo();
}

void CWizKbSync::onDownloadDocumentsFullInfoCompleted()
{
    Q_EMIT progressChanged(progressDocumemtFullInfoDownloaded);

    //if no error occured while downloading document full information
    //update document version
    if (!m_bDocumentInfoError && -1 != m_nDocumentMaxVersion) {
        m_db->SetObjectVersion(WIZDOCUMENTDATA::ObjectName(), m_nDocumentMaxVersion);
    }

    startDownloadObjectsData();
}

void CWizKbSync::startDownloadObjectsData()
{
    m_db->GetAllObjectsNeedToBeDownloaded(m_arrayAllObjectsNeedToBeDownloaded);

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

    int nTotal = m_arrayAllObjectsNeedToBeDownloaded.size();
    if (nTotal) {
        Q_EMIT processLog(WizFormatString1(tr("downloading objects data, total %1 objects need download"), nTotal));
    }

    Q_EMIT progressChanged(progressAttachmentUploaded);
    downloadNextObjectData();
}

void CWizKbSync::onDownloadObjectsDataCompleted()
{
    Q_EMIT progressChanged(progressObjectDownloaded);

    stopSync();
}

void CWizKbSync::stopSync()
{
    m_bSyncStarted = false;

    Q_EMIT progressChanged(progressDone);
    Q_EMIT kbSyncDone(m_error);
}

void CWizKbSync::downloadNextDeleteds(__int64 nVersion)
{
    callDeletedGetList(nVersion);
}

void CWizKbSync::downloadNextTags(__int64 nVersion)
{
    callTagGetList(nVersion);
}

void CWizKbSync::downloadNextStyles(__int64 nVersion)
{
    callStyleGetList(nVersion);
}

void CWizKbSync::downloadNextDocumentsSimpleInfo(__int64 nVersion)
{
    callDocumentGetList(nVersion);
}

void CWizKbSync::downloadNextAttachmentsInfo(__int64 nVersion)
{
    callAttachmentGetList(nVersion);
}

void CWizKbSync::uploadNextDeleteds()
{
    if (m_arrayAllDeletedsNeedToBeUploaded.empty()) {
        onUploadDeletedsCompleted();
    } else {
        int countPerPage = WIZAPI_PAGE_MAX;

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

void CWizKbSync::uploadNextTags()
{
    if (m_arrayAllTagsNeedToBeUploaded.empty()) {
        onUploadTagsCompleted();
    } else {
        int countPerPage = WIZAPI_PAGE_MAX;

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

void CWizKbSync::uploadNextStyles()
{
    if (m_arrayAllStylesNeedToBeUploaded.empty()) {
        onUploadStylesCompleted();
    } else {
        int countPerPage = WIZAPI_PAGE_MAX;

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

void CWizKbSync::uploadNextDocument()
{
    if (m_arrayAllDocumentsNeedToBeUploaded.empty()) {
        onUploadDocumentsCompleted();
    } else {
        m_currentUploadDocument = m_arrayAllDocumentsNeedToBeUploaded[0];
        m_arrayAllDocumentsNeedToBeUploaded.erase(m_arrayAllDocumentsNeedToBeUploaded.begin());

        queryDocumentInfo(m_currentUploadDocument.strGUID, m_currentUploadDocument.strTitle);
    }
}

void CWizKbSync::uploadNextAttachment()
{
    if (m_arrayAllAttachmentsNeedToBeUploaded.empty()) {
        onUploadAttachmentsCompleted();
    } else {
        m_currentUploadAttachment = m_arrayAllAttachmentsNeedToBeUploaded[0];
        m_arrayAllAttachmentsNeedToBeUploaded.erase(m_arrayAllAttachmentsNeedToBeUploaded.begin());

        queryAttachmentInfo(m_currentUploadAttachment.strGUID, m_currentUploadAttachment.strName);
    }
}

void CWizKbSync::onDeletedPostList(const std::deque<WIZDELETEDGUIDDATA>& arrayData)
{
    CWizApi::onDeletedPostList(arrayData);

    uploadNextDeleteds();
}

void CWizKbSync::onTagPostList(const std::deque<WIZTAGDATA>& arrayData)
{
    CWizApi::onTagPostList(arrayData);

    uploadNextTags();
}

void CWizKbSync::onStylePostList(const std::deque<WIZSTYLEDATA>& arrayData)
{
    CWizApi::onStylePostList(arrayData);

    uploadNextStyles();
}

void CWizKbSync::onDocumentsGetInfo(const std::deque<WIZDOCUMENTDATABASE>& arrayRet)
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

void CWizKbSync::onAttachmentsGetInfo(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
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

void CWizKbSync::onDeletedGetList(const std::deque<WIZDELETEDGUIDDATA>& arrayRet)
{
    CWizApi::onDeletedGetList(arrayRet);

    m_arrayAllDeletedsDownloaded.insert(m_arrayAllDeletedsDownloaded.end(),
                                        arrayRet.begin(), arrayRet.end());

    if (arrayRet.size() < WIZAPI_PAGE_MAX) {
        onDownloadDeletedsCompleted();
    } else {
        downloadNextDeleteds(WizObjectsGetMaxVersion<WIZDELETEDGUIDDATA>(arrayRet));
    }
}

void CWizKbSync::onTagGetList(const std::deque<WIZTAGDATA>& arrayRet)
{
    CWizApi::onTagGetList(arrayRet);

    m_arrayAllTagsDownloaded.insert(m_arrayAllTagsDownloaded.end(),
                                    arrayRet.begin(), arrayRet.end());

    if (arrayRet.size() < WIZAPI_PAGE_MAX) {
        onDownloadTagsCompleted();
    } else {
        downloadNextTags(WizObjectsGetMaxVersion<WIZTAGDATA>(arrayRet));
    }
}

void CWizKbSync::onStyleGetList(const std::deque<WIZSTYLEDATA>& arrayRet)
{
    CWizApi::onStyleGetList(arrayRet);

    m_arrayAllStylesDownloaded.insert(m_arrayAllStylesDownloaded.end(),
                                      arrayRet.begin(), arrayRet.end());

    if (arrayRet.size() < WIZAPI_PAGE_MAX) {
        onDownloadStylesCompleted();
    } else {
        downloadNextStyles(WizObjectsGetMaxVersion<WIZSTYLEDATA>(arrayRet));
    }
}

void CWizKbSync::onDocumentGetList(const std::deque<WIZDOCUMENTDATABASE>& arrayRet)
{
    CWizApi::onDocumentGetList(arrayRet);

    m_arrayAllDocumentsNeedToBeDownloaded.insert(m_arrayAllDocumentsNeedToBeDownloaded.end(),
                                                 arrayRet.begin(), arrayRet.end());

    if (arrayRet.size() < WIZAPI_PAGE_MAX) {
        onDownloadDocumentsSimpleInfoCompleted();
    } else {
        downloadNextDocumentsSimpleInfo(WizObjectsGetMaxVersion<WIZDOCUMENTDATABASE>(arrayRet));
    }
}

void CWizKbSync::onAttachmentGetList(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
{
    CWizApi::onAttachmentGetList(arrayRet);

    m_arrayAllAttachmentsDownloaded.insert(m_arrayAllAttachmentsDownloaded.end(),
                                           arrayRet.begin(), arrayRet.end());

    if (arrayRet.size() < WIZAPI_PAGE_MAX) {
        onDownloadAttachmentsInfoCompleted();
    } else {
        downloadNextAttachmentsInfo(WizObjectsGetMaxVersion<WIZDOCUMENTATTACHMENTDATAEX>(arrayRet));
    }
}

void CWizKbSync::downloadNextDocumentFullInfo()
{
    if (m_arrayAllDocumentsNeedToBeDownloaded.empty()) {
        onDownloadDocumentsFullInfoCompleted();
    } else {
        WIZDOCUMENTDATABASE data = m_arrayAllDocumentsNeedToBeDownloaded[0];
        m_arrayAllDocumentsNeedToBeDownloaded.erase(m_arrayAllDocumentsNeedToBeDownloaded.begin());

        callDocumentGetData(data);
    }
}

void CWizKbSync::onDocumentGetData(const WIZDOCUMENTDATAEX& data)
{
    //CWizApi::onDocumentGetData(data);

    if (m_bChained) {
        if (!m_db->UpdateDocument(data)) {
            m_bDocumentInfoError = true;
        }

        downloadNextDocumentFullInfo();
    } else {
        processDocumentData(data);
    }
}

void CWizKbSync::downloadNextObjectData()
{
    if (m_arrayAllObjectsNeedToBeDownloaded.empty()) {
        onDownloadObjectsDataCompleted();
    } else {
        WIZOBJECTDATA data = *m_arrayAllObjectsNeedToBeDownloaded.begin();
        m_arrayAllObjectsNeedToBeDownloaded.erase(m_arrayAllObjectsNeedToBeDownloaded.begin());

        downloadObjectData(data);
    }
}

void CWizKbSync::onDownloadObjectDataCompleted(const WIZOBJECTDATA& data)
{
    //CWizApi::onDownloadObjectDataCompleted(data);

    if (m_bChained) {
        m_db->UpdateSyncObjectLocalData(data);
        downloadNextObjectData();
    } else {
        processObjectData(data);
    }
}

void CWizKbSync::queryDocumentInfo(const CString& strGUID, const CString& strTitle)
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

void CWizKbSync::processDocumentData(const WIZDOCUMENTDATAEX& data)
{
    // to avoid unable to download document object data issue.
    m_conflictDownloadedInfo = data;

    WIZOBJECTDATA objectData(data);
    downloadObjectData(objectData);
}

void CWizKbSync::processObjectData(const WIZOBJECTDATA& data)
{
    WIZOBJECTDATA conflictObjectData(data);
    conflictObjectData.strObjectGUID = WizGenGUIDLowerCaseLetterOnly();
    conflictObjectData.strDisplayName += tr("(conflict backup)");

    // set dirty flag, upload needed
    m_conflictDownloadedInfo.nVersion = -1;
    m_conflictDownloadedInfo.strKbGUID = kbGUID();
    m_conflictDownloadedInfo.strGUID = conflictObjectData.strObjectGUID;
    m_conflictDownloadedInfo.strTitle += tr("(conflict backup)");
    m_conflictDownloadedInfo.strInfoMD5 = m_db->CalDocumentInfoMD5(m_conflictDownloadedInfo);

    if (m_db->CreateDocumentEx(m_conflictDownloadedInfo)) {
        m_db->UpdateSyncObjectLocalData(conflictObjectData);
        Q_EMIT processLog(WizFormatString1(tr("Conflict backup created: %1"), m_conflictDownloadedInfo.strTitle));
    } else {
        Q_EMIT processLog("unable to create conflict backup while create document");
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

void CWizKbSync::onQueryDocumentInfo(const WIZDOCUMENTDATABASE& data)
{
    int nParts = calDocumentParts(data, m_currentUploadDocument);
    if (0 == nParts) {
        onUploadDocument(m_currentUploadDocument);
    } else {
        m_currentUploadDocument.nObjectPart = nParts;
        uploadDocument(m_currentUploadDocument);
    }
}

void CWizKbSync::onUploadDocument(const WIZDOCUMENTDATAEX& data)
{
    m_db->ModifyObjectVersion(data.strGUID, WIZDOCUMENTDATAEX::ObjectName(), 0);
    uploadNextDocument();
}

void CWizKbSync::queryAttachmentInfo(const CString& strGUID, const CString& strName)
{
    Q_EMIT processLog(tr("query attachment info: ") + strName);

    CWizStdStringArray arrayGUID;
    arrayGUID.push_back(strGUID);
    callAttachmentsGetInfo(arrayGUID);
}

void CWizKbSync::onQueryAttachmentInfo(const WIZDOCUMENTATTACHMENTDATA& data)
{
    int nParts = calAttachmentParts(data, m_currentUploadAttachment);
    if (0 == nParts) {
        onUploadAttachment(m_currentUploadAttachment);
    } else {
        m_currentUploadAttachment.nObjectPart = nParts;
        uploadAttachment(m_currentUploadAttachment);
    }
}

void CWizKbSync::onUploadAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data)
{
    m_db->ModifyObjectVersion(data.strGUID, WIZDOCUMENTATTACHMENTDATAEX::ObjectName(), 0);
    uploadNextAttachment();
}

bool compareDocumentByTime(const WIZDOCUMENTDATABASE& data1, const WIZDOCUMENTDATABASE& data2)
{
    return data1.tDataModified > data2.tDataModified;
}

void CWizKbSync::filterDocuments()
{
    //compare document md5 (info, param, data)
    size_t nCount = m_arrayAllDocumentsNeedToBeDownloaded.size();
    for (intptr_t i = nCount - 1; i >= 0; i--)
    {
        WIZDOCUMENTDATABASE& data = m_arrayAllDocumentsNeedToBeDownloaded[i];

        WIZDOCUMENTDATA dataExists;
        if (!m_db->DocumentFromGUID(data.strGUID, dataExists)) {
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
            m_db->SetObjectDataDownloaded(data.strGUID, "document", false);
        }

        if (0 == data.nObjectPart) {
            m_arrayAllDocumentsNeedToBeDownloaded.erase(m_arrayAllDocumentsNeedToBeDownloaded.begin() + i);
            continue;
        }
    }

    //sort by time
    std::sort(m_arrayAllDocumentsNeedToBeDownloaded.begin(), m_arrayAllDocumentsNeedToBeDownloaded.end(), compareDocumentByTime);
}

int CWizKbSync::calDocumentParts(const WIZDOCUMENTDATABASE& sourceData, \
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

int CWizKbSync::calAttachmentParts(const WIZDOCUMENTATTACHMENTDATA& sourceData, \
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
