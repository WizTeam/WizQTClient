#ifndef WIZSYNC_H
#define WIZSYNC_H

#ifndef WIZAPI_H
#include "wizapi.h"
#endif


class CWizSync : public CWizApi
{
public:
    CWizSync(CWizDatabase& db, const CString& strAccountsApiURL, CWizSyncEvents& events);

    void startSync();
    void setDownloadAllNotesData(bool b) { m_bDownloadAllNotesData = b; }

private:
    bool m_error;
    size_t m_nAllDocumentsNeedToBeDownloadedCount;
    BOOL m_bDocumentInfoError;
    __int64 m_nDocumentMaxVersion;
    std::deque<WIZDOCUMENTDATABASE> m_arrayAllDocumentsNeedToBeDownloaded;
    //
    CWizDeletedGUIDDataArray m_arrayAllDeletedsNeedToBeUploaded;
    CWizTagDataArray m_arrayAllTagsNeedToBeUploaded;
    CWizStyleDataArray m_arrayAllStylesNeedToBeUploaded;
    //
    size_t m_nAllDocumentsNeedToBeUploadedCount;
    CWizDocumentDataArray m_arrayAllDocumentsNeedToBeUploaded;
    size_t m_nAllAttachmentsNeedToBeUploadedCount;
    CWizDocumentAttachmentDataArray m_arrayAllAttachmentsNeedToBeUploaded;
    //
    WIZDOCUMENTDATAEX m_currentUploadDocument;
    WIZDOCUMENTATTACHMENTDATAEX m_currentUploadAttachment;
    //
    size_t m_nAllObjectsNeedToBeDownloadedCount;
    CWizObjectDataArray m_arrayAllObjectsNeedToBeDownloaded;
    //
    bool m_bDownloadAllNotesData;
private:
    BOOL updateDocument(const WIZDOCUMENTDATABASE& data);
    BOOL updateDocuments(const std::deque<WIZDOCUMENTDATABASE>& arrayDocument);
    //
    BOOL downloadDocument(const WIZDOCUMENTDATABASE& data);
    //
    void filterDocuments();
    int calObjectPart(const WIZDOCUMENTDATABASE& data);
    //
    void changeProgressEx(size_t currentCount, size_t total, int stepStart, int stepCount);
protected:
    virtual void onXmlRpcError(const CString& strMethodName, WizXmlRpcError err, int errorCode, const CString& errorMessage);
    //step 1 login
    virtual void onClientLogin();
    //step 2 download deleted guids
    virtual void startDownloadDeleteds();
    virtual void onDownloadDeletedsCompleted();
    //step 3 upload deleted guids
    virtual void startUploadDeleteds();
    virtual void onUploadDeletedsCompleted();
    //step 4 download tags
    virtual void startDownloadTags();
    virtual void onDownloadTagsCompleted();
    //step 5 upload tags;
    virtual void startUploadTags();
    virtual void onUploadTagsCompleted();
    //step 6 download styles
    virtual void startDownloadStyles();
    virtual void onDownloadStylesCompleted();
    //step 7 upload styles
    virtual void startUploadStyles();
    virtual void onUploadStylesCompleted();
    //step 8 download documents simple info
    virtual void startDownloadDocumentsSimpleInfo();
    virtual void onDownloadDocumentsSimpleInfoCompleted();
    //step 9 download documents full ingfo
    virtual void startDownloadDocumentsFullInfo();
    virtual void onDownloadDocumentsFullInfoCompleted();
    //step 10 download attachments
    virtual void startDownloadAttachmentsInfo();
    virtual void onDownloadAttachmentsInfoCompleted();
    //step 11 upload documents  20-50
    virtual void startUploadDocuments();
    virtual void onUploadDocumentsCompleted();
    //step 12 upload attachments  50-60
    virtual void startUploadAttachments();
    virtual void onUploadAttachmentsCompleted();
    //step 13 download objects data  60-99
    virtual void startDownloadObjectsData();
    virtual void onDownloadObjectsDataCompleted();
    //step 14 logout
    virtual void onClientLogout();
    //
    //page
    virtual void downloadNextDeleteds(__int64 nVersion);
    virtual void downloadNextTags(__int64 nVersion);
    virtual void downloadNextStyles(__int64 nVersion);
    virtual void downloadNextDocumentsSimpleInfo(__int64 nVersion);
    virtual void downloadNextAttachmentsInfo(__int64 nVersion);
    //
    virtual void uploadNextDeleteds();
    virtual void uploadNextTags();
    virtual void uploadNextStyles();
    virtual void uploadNextDocument();
    virtual void uploadNextAttachment();
    ////
    //
    virtual void stopSync();
protected:
    //download document full info
    virtual void downloadNextDocumentFullInfo();
    virtual void onDocumentGetData(const WIZDOCUMENTDATAEX& data);
    //download objects data
    virtual void downloadNextObjectData();
    virtual void onDownloadObjectDataCompleted(const WIZOBJECTDATA& data);
    //
    //upload document
    virtual void queryDocumentInfo(const CString& strGUID, const CString& strTitle);
    virtual void onQueryDocumentInfo(const WIZDOCUMENTDATABASE& data);
    virtual void onUploadDocument(const WIZDOCUMENTDATAEX& data);
    //
    //upload attachment
    virtual void queryAttachmentInfo(const CString& strGUID, const CString& strName);
    virtual void onQueryAttachmentInfo(const WIZDOCUMENTATTACHMENTDATA& data);
    virtual void onUploadAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data);
protected:
    //
    virtual void onDeletedPostList(const std::deque<WIZDELETEDGUIDDATA>& arrayData);
    virtual void onTagPostList(const std::deque<WIZTAGDATA>& arrayData);
    virtual void onStylePostList(const std::deque<WIZSTYLEDATA>& arrayData);
    //
    virtual void onDocumentsGetInfo(const std::deque<WIZDOCUMENTDATABASE>& arrayRet);
    virtual void onAttachmentsGetInfo(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet);
    //
    virtual void onDeletedGetList(const std::deque<WIZDELETEDGUIDDATA>& arrayRet);
    virtual void onTagGetList(const std::deque<WIZTAGDATA>& arrayRet);
    virtual void onStyleGetList(const std::deque<WIZSTYLEDATA>& arrayRet);
    virtual void onDocumentGetList(const std::deque<WIZDOCUMENTDATABASE>& arrayRet);
    virtual void onAttachmentGetList(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet);

};


#endif // WIZSYNC_H
