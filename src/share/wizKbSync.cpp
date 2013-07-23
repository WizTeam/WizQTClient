#include "wizKbSync.h"

#include <algorithm>


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

    setKbUrl(database()->server());

    if (!strKbGUID.isEmpty())
        setKbGUID(strKbGUID);

    qDebug() << "\n\n[Syncing]Begin Syncing, database: " << database()->name();

    startUploadDeleteds();
}

void CWizKbSync::abort()
{
    CWizApi::abort();

    Q_EMIT processLog(tr("abort syncing, disconnct from server"));
    Q_EMIT kbSyncDone(m_error);
}

void CWizKbSync::onXmlRpcError(const QString& strMethodName, WizXmlRpcError errorType, int errorCode, const QString& errorMessage)
{
    CWizApi::onXmlRpcError(strMethodName, errorType, errorCode, errorMessage);

    m_error = true;

    if (strMethodName == SyncMethod_ClientLogin
            || strMethodName == SyncMethod_ClientKeepAlive) {
        stopSync();
        return;
    }


    if (errorType == errorNetwork && strMethodName != SyncMethod_ClientLogout) {
        stopSync();
    }
}

void CWizKbSync::startUploadDeleteds()
{
    m_db->GetModifiedDeletedGUIDs(m_arrayAllDeletedsNeedToBeUploaded);
    int nSize = m_arrayAllDeletedsNeedToBeUploaded.size();

    qDebug() << "[Syncing]upload deleteds, total: " << nSize;

    if (nSize) {
        Q_EMIT processLog(WizFormatString1(tr("uploading deleted objects list, total %1 deleted objects need upload"), nSize));
    }

    uploadNextDeleteds();
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

void CWizKbSync::onDeletedPostList(const CWizDeletedGUIDDataArray &arrayData)
{
    CWizDeletedGUIDDataArray::const_iterator it;
    for (it = arrayData.begin(); it != arrayData.end(); it++) {
        m_db->DeleteDeletedGUID(it->strGUID);
    }

    uploadNextDeleteds();
}

void CWizKbSync::onUploadDeletedsCompleted()
{
    startDownloadDeleteds();
}

void CWizKbSync::startDownloadDeleteds()
{
    qint64 nVersion = m_db->GetObjectVersion(WIZDELETEDGUIDDATA::ObjectName());
    qDebug() << "[Syncing]download deleted, local version: " << nVersion;

    downloadNextDeleteds(nVersion);
}

void CWizKbSync::downloadNextDeleteds(qint64 nVersion)
{
    callDeletedGetList(nVersion);
}

void CWizKbSync::onDeletedGetList(const CWizDeletedGUIDDataArray &arrayRet)
{
    m_db->UpdateDeletedGUIDs(arrayRet);

    m_arrayAllDeletedsDownloaded.insert(m_arrayAllDeletedsDownloaded.end(),
                                        arrayRet.begin(), arrayRet.end());

    qDebug() << "[Syncing]download size: " << m_arrayAllDeletedsDownloaded.size();

    if (arrayRet.size() < WIZAPI_PAGE_MAX) {
        onDownloadDeletedsCompleted();
    } else {
        downloadNextDeleteds(WizObjectsGetMaxVersion<WIZDELETEDGUIDDATA>(arrayRet));
    }
}

void CWizKbSync::onDownloadDeletedsCompleted()
{
    int nTotal = m_arrayAllDeletedsDownloaded.size();
    if (nTotal) {
        Q_EMIT processLog(WizFormatString1(tr("Total %1 deleted objects be synchronized"), nTotal));
    }

    startUploadTags();
}

void CWizKbSync::startUploadTags()
{
    m_db->GetModifiedTags(m_arrayAllTagsNeedToBeUploaded);
    int nSize = m_arrayAllTagsNeedToBeUploaded.size();

    qDebug() << "[Syncing]upload tags, total: " << nSize;

    if (nSize) {
        Q_EMIT processLog(WizFormatString1(tr("uploading tags list, total %1 tags need upload"), nSize));
    }

    uploadNextTags();
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

void CWizKbSync::onTagPostList(const CWizTagDataArray& arrayData)
{
    CWizTagDataArray::const_iterator it;
    for (it = arrayData.begin(); it != arrayData.end(); it++) {
        m_db->ModifyObjectVersion(it->strGUID, WIZTAGDATA::ObjectName(), 0);
    }

    uploadNextTags();
}

void CWizKbSync::onUploadTagsCompleted()
{
    startDownloadTags();
}

void CWizKbSync::startDownloadTags()
{
    qint64 nVersion = m_db->GetObjectVersion(WIZTAGDATA::ObjectName());
    qDebug() << "[Syncing]download tags, local version: " << nVersion;

    downloadNextTags(nVersion);
}

void CWizKbSync::downloadNextTags(qint64 nVersion)
{
    callTagGetList(nVersion);
}

void CWizKbSync::onTagGetList(const CWizTagDataArray& arrayRet)
{
    m_db->UpdateTags(arrayRet);

    m_arrayAllTagsDownloaded.insert(m_arrayAllTagsDownloaded.end(),
                                    arrayRet.begin(), arrayRet.end());

    qDebug() << "[Syncing]download size: " << m_arrayAllTagsDownloaded.size();

    if (arrayRet.size() < WIZAPI_PAGE_MAX) {
        onDownloadTagsCompleted();
    } else {
        downloadNextTags(WizObjectsGetMaxVersion<WIZTAGDATA>(arrayRet));
    }
}

void CWizKbSync::onDownloadTagsCompleted()
{
    int nTotal = m_arrayAllTagsDownloaded.size();
    if (nTotal) {
        Q_EMIT processLog(WizFormatString1(tr("Total %1 tags be synchronized"), nTotal));
    }

    startUploadStyles();
}

void CWizKbSync::startUploadStyles()
{
    m_db->GetModifiedStyles(m_arrayAllStylesNeedToBeUploaded);
    int nSize = m_arrayAllStylesNeedToBeUploaded.size();

    qDebug() << "[Syncing]upload styles, total: " << nSize;

    if (nSize) {
        Q_EMIT processLog(WizFormatString1(tr("uploading styles list, total %1 styles need upload"), nSize));
    }

    uploadNextStyles();
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

void CWizKbSync::onStylePostList(const CWizStyleDataArray& arrayData)
{
    CWizStyleDataArray::const_iterator it;
    for (it = arrayData.begin(); it != arrayData.end(); it++) {
        m_db->ModifyObjectVersion(it->strGUID, WIZSTYLEDATA::ObjectName(), 0);
    }

    uploadNextStyles();
}

void CWizKbSync::onUploadStylesCompleted()
{
    startDownloadStyles();
}

void CWizKbSync::startDownloadStyles()
{
    qint64 nVersion = m_db->GetObjectVersion(WIZSTYLEDATA::ObjectName());
    qDebug() << "[Syncing]download styles, local version: " << nVersion;

    downloadNextStyles(nVersion);
}

void CWizKbSync::downloadNextStyles(qint64 nVersion)
{
    callStyleGetList(nVersion);
}

void CWizKbSync::onStyleGetList(const CWizStyleDataArray& arrayRet)
{
    m_db->UpdateStyles(arrayRet);

    m_arrayAllStylesDownloaded.insert(m_arrayAllStylesDownloaded.end(),
                                      arrayRet.begin(), arrayRet.end());

    qDebug() << "[Syncing]download size: " << m_arrayAllStylesDownloaded.size();

    if (arrayRet.size() < WIZAPI_PAGE_MAX) {
        onDownloadStylesCompleted();
    } else {
        downloadNextStyles(WizObjectsGetMaxVersion<WIZSTYLEDATA>(arrayRet));
    }
}

void CWizKbSync::onDownloadStylesCompleted()
{
    int nTotal = m_arrayAllStylesDownloaded.size();
    if (nTotal) {
        Q_EMIT processLog(WizFormatString1(tr("Total %1 styles be sychronized"), nTotal));
    }

    startUploadDocuments();
}

void CWizKbSync::startUploadDocuments()
{
    m_db->GetModifiedDocuments(m_arrayAllDocumentsNeedToBeUploaded);
    int nSize = m_arrayAllDocumentsNeedToBeUploaded.size();

    qDebug() << "[Syncing]upload documents, total: " << nSize;

    if (nSize) {
        Q_EMIT processLog(WizFormatString1(tr("uploading documents, total %1 documents need upload"), nSize));
    }

    uploadNextDocument();
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

void CWizKbSync::queryDocumentInfo(const CString& strGUID, const CString& strTitle)
{
    Q_EMIT processLog(tr("query note info: ") + strTitle);

    CWizStdStringArray arrayGUID;
    arrayGUID.push_back(strGUID);
    callDocumentsGetInfo(arrayGUID);
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
    if (!data.strGUID.isEmpty()) {
        m_db->ModifyObjectVersion(data.strGUID, WIZDOCUMENTDATAEX::ObjectName(), 0);
    }

    uploadNextDocument();
}

void CWizKbSync::onUploadDocumentsCompleted()
{
    startUploadAttachments();
}

void CWizKbSync::startUploadAttachments()
{
    m_db->GetModifiedAttachments(m_arrayAllAttachmentsNeedToBeUploaded);
    int nSize = m_arrayAllAttachmentsNeedToBeUploaded.size();

    qDebug() << "[Syncing]upload attachments, total: " << nSize;
    Q_ASSERT(nSize == 0);

    if (nSize) {
        Q_EMIT processLog(WizFormatString1(tr("uploading attachments, total %1 attachments need upload"), nSize));
    }

    uploadNextAttachment();
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

void CWizKbSync::queryAttachmentInfo(const CString& strGUID, const CString& strName)
{
    Q_EMIT processLog(tr("query attachment info: ") + strName);

    CWizStdStringArray arrayGUID;
    arrayGUID.push_back(strGUID);
    callAttachmentsGetInfo(arrayGUID);
}

void CWizKbSync::onAttachmentsGetInfo(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
{
    size_t count = arrayRet.size();

    // new attachment
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

    // server return exist
    } else if (count == 1) {
        onQueryAttachmentInfo(arrayRet[0]);

    // fatal error
    } else {
        Q_EMIT processErrorLog("Can not query document info");
        onXmlRpcError(SyncMethod_GetDocumentsInfo, errorXmlRpcFault, -1, "Fault error: Invalid document info");
    }
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
    if (!data.strGUID.isEmpty()) {
        m_db->ModifyObjectVersion(data.strGUID, WIZDOCUMENTATTACHMENTDATAEX::ObjectName(), 0);
    }

    uploadNextAttachment();
}

void CWizKbSync::onUploadAttachmentsCompleted()
{
    startDownloadDocumentsSimpleInfo();
}

void CWizKbSync::startDownloadDocumentsSimpleInfo()
{ 
    qint64 nVersion = m_db->GetObjectVersion(WIZDOCUMENTDATA::ObjectName());
    qDebug() << "[Syncing]download document list, local version: " << nVersion;

    downloadNextDocumentsSimpleInfo(nVersion);
}

void CWizKbSync::downloadNextDocumentsSimpleInfo(qint64 nVersion)
{
    callDocumentGetList(nVersion);
}

void CWizKbSync::onDocumentGetList(const std::deque<WIZDOCUMENTDATABASE>& arrayRet)
{
    m_arrayAllDocumentsNeedToBeDownloaded.insert(m_arrayAllDocumentsNeedToBeDownloaded.end(),
                                                 arrayRet.begin(), arrayRet.end());

    qDebug() << "[Syncing]download size: " << m_arrayAllDocumentsNeedToBeDownloaded.size();

    if (arrayRet.size() < WIZAPI_PAGE_MAX) {
        onDownloadDocumentsSimpleInfoCompleted();
    } else {
        downloadNextDocumentsSimpleInfo(WizObjectsGetMaxVersion<WIZDOCUMENTDATABASE>(arrayRet));
    }
}

void CWizKbSync::onDownloadDocumentsSimpleInfoCompleted()
{
    // save max version of document
    // if no error occured while downloading document full information
    // then update this version
    if (!m_arrayAllDocumentsNeedToBeDownloaded.empty()) {
        m_nDocumentMaxVersion = ::WizObjectsGetMaxVersion<WIZDOCUMENTDATABASE>(m_arrayAllDocumentsNeedToBeDownloaded);
    }

    // filter documents for getting document full information (not data)
    // if it's the first time user syncing triggered, filter is not needed.
    int nSize;
    m_db->GetAllDocumentsSize(nSize, true);

    if (nSize) {
        filterDocuments();
    }

    int nTotal = m_arrayAllDocumentsNeedToBeDownloaded.size();
    if (nTotal) {
        Q_EMIT processLog(WizFormatString1(tr("Total %1 documents need to be synchronized"), nTotal));
    }

    startDownloadDocumentsFullInfo();
}

void CWizKbSync::startDownloadDocumentsFullInfo()
{
    downloadNextDocumentFullInfo();
}

void CWizKbSync::downloadNextDocumentFullInfo()
{
    if (m_arrayAllDocumentsNeedToBeDownloaded.empty()) {
        onDownloadDocumentsFullInfoCompleted();
    } else {
        WIZDOCUMENTDATABASE data = m_arrayAllDocumentsNeedToBeDownloaded[0];
        m_arrayAllDocumentsNeedToBeDownloaded.erase(m_arrayAllDocumentsNeedToBeDownloaded.begin());

        Q_EMIT processLog(tr("download document info: ") + data.strTitle);
        callDocumentGetData(data);
    }
}

void CWizKbSync::onDocumentGetData(const WIZDOCUMENTDATAEX& data)
{
    Q_ASSERT(!data.strGUID.isEmpty());

    if (!m_db->UpdateDocument(data)) {
        TOLOG1("Update Document info failed: %1", data.strTitle);
        m_bDocumentInfoError = true;
    }

    downloadNextDocumentFullInfo();
}

void CWizKbSync::onDownloadDocumentsFullInfoCompleted()
{
    // if no error occured while downloading document full information
    // update document version
    if (!m_bDocumentInfoError && -1 != m_nDocumentMaxVersion) {
        m_db->SetObjectVersion(WIZDOCUMENTDATA::ObjectName(), m_nDocumentMaxVersion);
    }

    startDownloadAttachmentsInfo();
}

void CWizKbSync::startDownloadAttachmentsInfo()
{
    qint64 nVersion = m_db->GetObjectVersion(WIZDOCUMENTATTACHMENTDATA::ObjectName());
    qDebug() << "[Syncing]download attachment list, local version: " << nVersion;

    downloadNextAttachmentsInfo(nVersion);
}

void CWizKbSync::downloadNextAttachmentsInfo(qint64 nVersion)
{
    callAttachmentGetList(nVersion);
}

void CWizKbSync::onAttachmentGetList(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
{
    m_db->UpdateAttachments(arrayRet);

    m_arrayAllAttachmentsDownloaded.insert(m_arrayAllAttachmentsDownloaded.end(),
                                           arrayRet.begin(), arrayRet.end());

    qDebug() << "[Syncing]download size: " << m_arrayAllAttachmentsDownloaded.size();

    if (arrayRet.size() < WIZAPI_PAGE_MAX) {
        onDownloadAttachmentsInfoCompleted();
    } else {
        downloadNextAttachmentsInfo(WizObjectsGetMaxVersion<WIZDOCUMENTATTACHMENTDATAEX>(arrayRet));
    }
}

void CWizKbSync::onDownloadAttachmentsInfoCompleted()
{
    int nTotal = m_arrayAllAttachmentsDownloaded.size();
    if (nTotal) {
        Q_EMIT processLog(WizFormatString1(tr("Total %1 attachments be synchronized"), nTotal));
    }

    startDownloadObjectsData();
}

void CWizKbSync::startDownloadObjectsData()
{
    m_db->GetAllObjectsNeedToBeDownloaded(m_arrayAllObjectsNeedToBeDownloaded);

    int nTotal = m_arrayAllObjectsNeedToBeDownloaded.size();

    if (m_nDaysDownload == -1) {
        m_arrayAllObjectsNeedToBeDownloaded.clear();
    } else {
        COleDateTime tNow = ::WizGetCurrentTime();
        size_t count = m_arrayAllObjectsNeedToBeDownloaded.size();
        for (intptr_t i = count - 1; i >= 0; i--)
        {
            COleDateTime t = m_arrayAllObjectsNeedToBeDownloaded[i].tTime;
            t = t.addDays(m_nDaysDownload);
            if (t < tNow) {
                m_arrayAllObjectsNeedToBeDownloaded.erase(m_arrayAllObjectsNeedToBeDownloaded.begin() + i);
                continue;
            }

            if (m_arrayAllObjectsNeedToBeDownloaded[i].eObjectType == wizobjectDocumentAttachment) {
                m_arrayAllObjectsNeedToBeDownloaded.erase(m_arrayAllObjectsNeedToBeDownloaded.begin() + i);
                continue;
            }
        }
    }

    int nTotalDownload = m_arrayAllObjectsNeedToBeDownloaded.size();
    if (nTotal) {
        Q_EMIT processLog(WizFormatString2(tr("downloading objects data, total %1 objects need download, actually download: %2"),
                                           QString::number(nTotal),
                                           QString::number(nTotalDownload)));
    }

    downloadNextObjectData();
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
    if (!data.strObjectGUID.isEmpty()) {
        m_db->UpdateSyncObjectLocalData(data);
    }

    downloadNextObjectData();
}

void CWizKbSync::onDownloadObjectsDataCompleted()
{
    stopSync();
}

void CWizKbSync::stopSync()
{
    m_bSyncStarted = false;

    Q_EMIT kbSyncDone(m_error);
}

bool compareDocumentByTime(const WIZDOCUMENTDATABASE& data1, const WIZDOCUMENTDATABASE& data2)
{
    return data1.tDataModified > data2.tDataModified;
}

void CWizKbSync::filterDocuments()
{
    // compare document md5 (info, param, data)
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
    std::sort(m_arrayAllDocumentsNeedToBeDownloaded.begin(),
              m_arrayAllDocumentsNeedToBeDownloaded.end(),
              compareDocumentByTime);
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
