#ifndef WIZSYNC_H
#define WIZSYNC_H

#ifndef WIZAPI_H
#include "wizapi.h"
#endif


class CWizSync : public CWizApi
{
    Q_OBJECT

public:
    CWizSync(CWizDatabase& db, const CString& strAccountsApiURL);

    void startSync();
    void setDownloadAllNotesData(bool b) { m_bDownloadAllNotesData = b; }
    void resetProxy() { CWizApi::resetProxy(); }

    virtual void abort();

private:
    bool m_bSyncStarted;
    bool m_error;
    bool m_bDownloadAllNotesData;

    // This is trick used for conflict backup.
    // cause onDocumentGetData and onDownloadObjectDataCompleted are rewrite by the class for chain call
    // But these two methods also needed when downloading conflict documents
    bool m_bChained;

    WIZDOCUMENTDATABASE m_conflictedDocument;
    WIZDOCUMENTDATAEX m_conflictDownloadedInfo;

    std::deque<WIZDOCUMENTDATABASE> m_arrayAllDocumentsNeedToBeDownloaded;
    __int64 m_nDocumentMaxVersion;
    bool m_bDocumentInfoError;

    CWizDeletedGUIDDataArray m_arrayAllDeletedsNeedToBeUploaded;
    CWizTagDataArray m_arrayAllTagsNeedToBeUploaded;
    CWizStyleDataArray m_arrayAllStylesNeedToBeUploaded;

    CWizDocumentDataArray m_arrayAllDocumentsNeedToBeUploaded;
    WIZDOCUMENTDATAEX m_currentUploadDocument;

    CWizDocumentAttachmentDataArray m_arrayAllAttachmentsNeedToBeUploaded;
    WIZDOCUMENTATTACHMENTDATAEX m_currentUploadAttachment;

    CWizObjectDataArray m_arrayAllObjectsNeedToBeDownloaded;


protected:
    virtual void onXmlRpcError(const QString& strMethodName, \
                               WizXmlRpcError err, int errorCode, const QString& errorMessage);

    // step 1: login
    virtual void onClientLogin(const WIZUSERINFO& userInfo);

    // step 2: download deleted guids
    virtual void startDownloadDeleteds();
    virtual void downloadNextDeleteds(__int64 nVersion);
    virtual void onDeletedGetList(const std::deque<WIZDELETEDGUIDDATA>& arrayRet);
    virtual void onDownloadDeletedsCompleted();

    // step 3: upload deleted guilds
    virtual void startUploadDeleteds();
    virtual void uploadNextDeleteds();
    virtual void onDeletedPostList(const std::deque<WIZDELETEDGUIDDATA>& arrayData);
    virtual void onUploadDeletedsCompleted();

    // step 4: download tags
    virtual void startDownloadTags();
    virtual void downloadNextTags(__int64 nVersion);
    virtual void onTagGetList(const std::deque<WIZTAGDATA>& arrayRet);
    virtual void onDownloadTagsCompleted();

    // step 5: upload tags
    virtual void startUploadTags();
    virtual void uploadNextTags();
    virtual void onTagPostList(const std::deque<WIZTAGDATA>& arrayData);
    virtual void onUploadTagsCompleted();

    // step 6: download styles
    virtual void startDownloadStyles();
    virtual void downloadNextStyles(__int64 nVersion);
    virtual void onStyleGetList(const std::deque<WIZSTYLEDATA>& arrayRet);
    virtual void onDownloadStylesCompleted();

    // step 7: upload styles
    virtual void startUploadStyles();
    virtual void uploadNextStyles();
    virtual void onStylePostList(const std::deque<WIZSTYLEDATA>& arrayData);
    virtual void onUploadStylesCompleted();

    // step 8: download documents info
    virtual void startDownloadDocumentsSimpleInfo();
    virtual void downloadNextDocumentsSimpleInfo(__int64 nVersion);
    virtual void onDocumentGetList(const std::deque<WIZDOCUMENTDATABASE>& arrayRet);
    virtual void onDownloadDocumentsSimpleInfoCompleted();

    // step 9: upload documents and do confilict backup
    virtual void startUploadDocuments();
    virtual void uploadNextDocument();
    virtual void queryDocumentInfo(const CString& strGUID, const CString& strTitle);
    virtual void onDocumentsGetInfo(const std::deque<WIZDOCUMENTDATABASE>& arrayRet);
    virtual void onQueryDocumentInfo(const WIZDOCUMENTDATABASE& data);
    virtual void onUploadDocument(const WIZDOCUMENTDATAEX& data);
    virtual void onUploadDocumentsCompleted();

    // step 10: download attachments info
    virtual void startDownloadAttachmentsInfo();
    virtual void downloadNextAttachmentsInfo(__int64 nVersion);
    virtual void onAttachmentGetList(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet);
    virtual void onDownloadAttachmentsInfoCompleted();

    // step 11: upload attachments and do confilict backup
    virtual void startUploadAttachments();
    virtual void uploadNextAttachment();
    virtual void queryAttachmentInfo(const CString& strGUID, const CString& strName);
    virtual void onAttachmentsGetInfo(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet);
    virtual void onQueryAttachmentInfo(const WIZDOCUMENTATTACHMENTDATA& data);
    virtual void onUploadAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data);
    virtual void onUploadAttachmentsCompleted();

    // step 12: download documents full info
    virtual void startDownloadDocumentsFullInfo();
    virtual void downloadNextDocumentFullInfo();
    virtual void onDocumentGetData(const WIZDOCUMENTDATAEX& data);
    virtual void onDownloadDocumentsFullInfoCompleted();

    // step 13: download objects data
    virtual void startDownloadObjectsData();
    virtual void downloadNextObjectData();
    virtual void onDownloadObjectDataCompleted(const WIZOBJECTDATA& data);
    virtual void onDownloadObjectsDataCompleted();

    // step 14: logout
    virtual void stopSync();
    virtual void onClientLogout();

    // conflict backup
    virtual void processDocumentData(const WIZDOCUMENTDATAEX& data);
    virtual void processObjectData(const WIZOBJECTDATA& data);

private:
    bool downloadDocument(const WIZDOCUMENTDATABASE& data);

    void filterDocuments();
    int calDocumentParts(const WIZDOCUMENTDATABASE& sourceData, \
                       const WIZDOCUMENTDATABASE& destData);
    int calAttachmentParts(const WIZDOCUMENTATTACHMENTDATA& sourceData, \
                           const WIZDOCUMENTATTACHMENTDATA& destData);

Q_SIGNALS:
    void syncStarted();
    void syncLogined();
    void syncDone(bool error);
};


#endif // WIZSYNC_H
