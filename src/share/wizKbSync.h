#ifndef WIZKBSYNC_H
#define WIZKBSYNC_H

#include "wizapi.h"


class CWizKbSync : public CWizApi
{
    Q_OBJECT

public:
    CWizKbSync(CWizDatabase& db, const CString& strKbUrl = WIZ_API_URL);

    // if strToken is empty, login is required
    void startSync(const QString& strKbGUID = QString());

    void setDaysDownload(int n) { m_nDaysDownload = n; }

    virtual void abort();

private:
    bool m_bSyncStarted;
    bool m_error;
    int m_nDaysDownload;

    CWizDeletedGUIDDataArray m_arrayAllDeletedsDownloaded;
    CWizDeletedGUIDDataArray m_arrayAllDeletedsNeedToBeUploaded;

    CWizTagDataArray m_arrayAllTagsDownloaded;
    CWizTagDataArray m_arrayAllTagsNeedToBeUploaded;

    CWizStyleDataArray m_arrayAllStylesDownloaded;
    CWizStyleDataArray m_arrayAllStylesNeedToBeUploaded;

    std::deque<WIZDOCUMENTDATABASE> m_arrayAllDocumentsNeedToBeDownloaded;
    qint64 m_nDocumentMaxVersion;
    bool m_bDocumentInfoError;

    CWizDocumentDataArray m_arrayAllDocumentsNeedToBeUploaded;
    WIZDOCUMENTDATAEX m_currentUploadDocument;

    CWizDocumentAttachmentDataArray m_arrayAllAttachmentsDownloaded;

    CWizDocumentAttachmentDataArray m_arrayAllAttachmentsNeedToBeUploaded;
    WIZDOCUMENTATTACHMENTDATAEX m_currentUploadAttachment;

    CWizObjectDataArray m_arrayAllObjectsNeedToBeDownloaded;

protected:
    virtual void onXmlRpcError(const QString& strMethodName,
                               WizXmlRpcError err, int errorCode,
                               const QString& errorMessage);

    // step 1: upload deleted guilds
    virtual void startUploadDeleteds();
    virtual void uploadNextDeleteds();
    virtual void onDeletedPostList(const CWizDeletedGUIDDataArray& arrayData);
    virtual void onUploadDeletedsCompleted();

    // step 2: download deleted guids
    virtual void startDownloadDeleteds();
    virtual void downloadNextDeleteds(qint64 nVersion);
    virtual void onDeletedGetList(const CWizDeletedGUIDDataArray& arrayRet);
    virtual void onDownloadDeletedsCompleted();

    // step 3: upload tags
    virtual void startUploadTags();
    virtual void uploadNextTags();
    virtual void onTagPostList(const CWizTagDataArray& arrayData);
    virtual void onUploadTagsCompleted();

    // step 4: download tags
    virtual void startDownloadTags();
    virtual void downloadNextTags(qint64 nVersion);
    virtual void onTagGetList(const CWizTagDataArray& arrayRet);
    virtual void onDownloadTagsCompleted();

    // step 5: upload styles
    virtual void startUploadStyles();
    virtual void uploadNextStyles();
    virtual void onStylePostList(const CWizStyleDataArray& arrayData);
    virtual void onUploadStylesCompleted();

    // step 6: download styles
    virtual void startDownloadStyles();
    virtual void downloadNextStyles(qint64 nVersion);
    virtual void onStyleGetList(const CWizStyleDataArray& arrayRet);
    virtual void onDownloadStylesCompleted();

    // step 7: upload documents and just cover server data
    virtual void startUploadDocuments();
    virtual void uploadNextDocument();
    virtual void queryDocumentInfo(const CString& strGUID, const CString& strTitle);
    virtual void onDocumentsGetInfo(const std::deque<WIZDOCUMENTDATABASE>& arrayRet);
    virtual void onQueryDocumentInfo(const WIZDOCUMENTDATABASE& data);
    virtual void onUploadDocument(const WIZDOCUMENTDATAEX& data);
    virtual void onUploadDocumentsCompleted();

    // step 8: upload attachments and just cover server data
    virtual void startUploadAttachments();
    virtual void uploadNextAttachment();
    virtual void queryAttachmentInfo(const CString& strGUID, const CString& strName);
    virtual void onAttachmentsGetInfo(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet);
    virtual void onQueryAttachmentInfo(const WIZDOCUMENTATTACHMENTDATA& data);
    virtual void onUploadAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data);
    virtual void onUploadAttachmentsCompleted();

    // step 9: download documents info list
    virtual void startDownloadDocumentsSimpleInfo();
    virtual void downloadNextDocumentsSimpleInfo(qint64 nVersion);
    virtual void onDocumentGetList(const std::deque<WIZDOCUMENTDATABASE>& arrayRet);
    virtual void onDownloadDocumentsSimpleInfoCompleted();

    // step 10: download documents full info
    virtual void startDownloadDocumentsFullInfo();
    virtual void downloadNextDocumentFullInfo();
    virtual void onDocumentGetData(const WIZDOCUMENTDATAEX& data);
    virtual void onDownloadDocumentsFullInfoCompleted();

    // step 11: download attachments info
    virtual void startDownloadAttachmentsInfo();
    virtual void downloadNextAttachmentsInfo(qint64 nVersion);
    virtual void onAttachmentGetList(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet);
    virtual void onDownloadAttachmentsInfoCompleted();

    // step 12: download objects data
    virtual void startDownloadObjectsData();
    virtual void downloadNextObjectData();
    virtual void onDownloadObjectDataCompleted(const WIZOBJECTDATA& data);
    virtual void onDownloadObjectsDataCompleted();

    // step 14: clean up
    virtual void stopSync();

private:
    void filterDocuments();
    int calDocumentParts(const WIZDOCUMENTDATABASE& sourceData, \
                       const WIZDOCUMENTDATABASE& destData);
    int calAttachmentParts(const WIZDOCUMENTATTACHMENTDATA& sourceData, \
                           const WIZDOCUMENTATTACHMENTDATA& destData);

Q_SIGNALS:
    void kbSyncDone(bool error);
};


#endif // WIZKBSYNC_H
