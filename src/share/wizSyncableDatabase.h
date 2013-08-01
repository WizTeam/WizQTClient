#ifndef WIZSYNCABLEDATABASE_H
#define WIZSYNCABLEDATABASE_H

#include "wizobject.h"

struct WIZDATABASEINFO
{
    // optional
    QString bizName;
    QString bizGUID;

    // required, private db set to "PRIVATE"
    QString name;

    // required
    QString kbGUID;

    // required, used for syncing object data, aka kapi_url
    QString serverUrl;

    // required, private db set to 0
    int nPermission;
};


struct IWizSyncableDatabase
{
    virtual QString GetUserId() = 0;
    virtual QString GetUserGUID() = 0;
    virtual QString GetPassword() = 0;
    virtual __int64 GetObjectVersion(const QString& strObjectType) = 0;
    virtual BOOL SetObjectVersion(const QString& strObjectType, __int64 nVersion) = 0;
    virtual BOOL OnDownloadDeletedList(const CWizDeletedGUIDDataArray& arrayData) = 0;
    virtual BOOL OnDownloadTagList(const CWizTagDataArray& arrayData) = 0;
    virtual BOOL OnDownloadStyleList(const CWizStyleDataArray& arrayData) = 0;
    virtual BOOL OnDownloadAttachmentList(const CWizDocumentAttachmentDataArray& arrayData) = 0;
    virtual __int64 GetObjectLocalVersion(const QString& strObjectGUID, const QString& strObjectType) = 0;
    virtual __int64 GetObjectLocalServerVersion(const QString& strObjectGUID, const QString& strObjectType) = 0;
    virtual BOOL SetObjectLocalServerVersion(const QString& strObjectGUID, const QString& strObjectType, __int64 nVersion) = 0;
    //
    virtual BOOL DocumentFromGUID(const QString& strGUID, WIZDOCUMENTDATA& dataExists) = 0;
    virtual BOOL SetObjectDataDownloaded(const QString& strGUID, const QString& strType, BOOL downloaded) = 0;
    virtual BOOL SetObjectServerDataInfo(const QString& strGUID, const QString& strType, COleDateTime& tServerDataModified, const QString& strServerMD5) = 0;
    //
    virtual BOOL OnDownloadDocument(int part, const WIZDOCUMENTDATAEX& data) = 0;
    //
    virtual BOOL GetObjectsNeedToBeDownloaded(CWizObjectDataArray& arrayObject) = 0;
    virtual BOOL UpdateObjectData(const QString& strObjectGUID, const QString& strObjectType, const QByteArray& stream) = 0;
    virtual BOOL IsObjectDataDownloaded(const QString& strGUID, const QString& strType) = 0;
    //
    virtual BOOL GetModifiedDeletedList(CWizDeletedGUIDDataArray& arrayData) = 0;
    virtual BOOL GetModifiedTagList(CWizTagDataArray& arrayData) = 0;
    virtual BOOL GetModifiedStyleList(CWizStyleDataArray& arrayData) = 0;
    virtual BOOL GetModifiedDocumentList(CWizDocumentDataArray& arrayData) = 0;
    virtual BOOL GetModifiedAttachmentList(CWizDocumentAttachmentDataArray& arrayData) = 0;
    //
    virtual BOOL InitDocumentData(const QString& strGUID, WIZDOCUMENTDATAEX& data, UINT part) = 0;
    virtual BOOL InitAttachmentData(const QString& strGUID, WIZDOCUMENTATTACHMENTDATAEX& data, UINT part) = 0;

    //
    virtual BOOL OnUploadObject(const QString& strGUID, const QString& strObjectType) = 0;
    //
    virtual BOOL OnDownloadGroups(const CWizGroupDataArray& arrayGroup) = 0;
    virtual IWizSyncableDatabase* GetGroupDatabase(const WIZGROUPDATA& group) = 0;
    virtual void CloseGroupDatabase(IWizSyncableDatabase* pDatabase) = 0;
    //
    virtual void SetKbInfo(const QString& strKBGUID, const WIZKBINFO& info) = 0;
    virtual void SetUserInfo(const WIZUSERINFO& info) = 0;
    //
    virtual BOOL IsGroup() = 0;
    virtual BOOL IsGroupAdmin() = 0;
    virtual BOOL IsGroupSuper() = 0;
    virtual BOOL IsGroupEditor() = 0;
    virtual BOOL IsGroupAuthor() = 0;
    virtual BOOL IsGroupReader() = 0;
    //
    virtual BOOL CanEditDocument(const WIZDOCUMENTDATA& data) = 0;
    virtual BOOL CanEditAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data) = 0;
    //
    virtual BOOL CreateConflictedCopy(const QString& strObjectGUID, const QString& strObjectType) = 0;
    //
    virtual BOOL SaveLastSyncTime() = 0;
    virtual COleDateTime GetLastSyncTime() = 0;
    //
    virtual long GetLocalFlags(const QString& strObjectGUID, const QString& strObjectType) = 0;
    virtual BOOL SetLocalFlags(const QString& strObjectGUID, const QString& strObjectType, long flags) = 0;
    //
    //
    virtual void GetAccountKeys(CWizStdStringArray& arrayKey) = 0;
    virtual __int64 GetAccountLocalValueVersion(const QString& strKey) = 0;
    virtual void SetAccountLocalValue(const QString& strKey, const QString& strValue, __int64 nServerVersion, BOOL bSaveVersion) = 0;

    virtual void GetKBKeys(CWizStdStringArray& arrayKey) = 0;
    virtual BOOL ProcessValue(const QString& strKey) = 0;
    virtual __int64 GetLocalValueVersion(const QString& strKey) = 0;
    virtual QString GetLocalValue(const QString& strKey) = 0;
    virtual void SetLocalValueVersion(const QString& strKey, __int64 nServerVersion) = 0;
    virtual void SetLocalValue(const QString& strKey, const QString& strValue, __int64 nServerVersion, BOOL bSaveVersion) = 0;
    //
    //virtual CComPtr<IWizBizUserCollection> GetBizUsers() = 0;
    virtual void GetAllBizUserIds(CWizStdStringArray& arrayText) = 0;
    //
    //virtual CComPtr<IWizDocument> GetDocumentByGUID(const QString& strDocumentGUID) = 0;
    virtual BOOL OnDownloadMessages(const CWizUserMessageDataArray& arrayMessage) = 0;
    //
    virtual void ClearError() = 0;
    virtual void OnTrafficLimit(const QString& strErrorMessage) = 0;
    virtual void OnStorageLimit(const QString& strErrorMessage) = 0;
    virtual BOOL IsTrafficLimit() = 0;
    virtual BOOL IsStorageLimit() = 0;
};



enum WizKMSyncProgressStatusType
{
    wizhttpstatustypeNormal,
    wizhttpstatustypeWarning,
    wizhttpstatustypeError
};



struct IWizKMSyncEvents
{
    virtual void OnSyncProgress(int pos) {}
    virtual HRESULT OnText(WizKMSyncProgressStatusType type, const QString& strStatus) = 0;
    virtual void SetStop(BOOL b) { m_bStop = b; }
    virtual BOOL IsStop() const { return m_bStop; }
    virtual void SetLastErrorCode(int nErrorCode);
    virtual int GetLastErrorCode() const { return m_nLastError; }
    virtual void SetDatabaseCount(int count) {}
    virtual void SetCurrentDatabase(int index) {}
    virtual void OnTrafficLimit(IWizSyncableDatabase* pDatabase) {}
    virtual void OnStorageLimit(IWizSyncableDatabase* pDatabase) {}
    virtual void OnUploadDocument(const QString& strDocumentGUID, BOOL bDone) {}
    virtual void OnBeginKb(const QString& strKbGUID) {}
    virtual void OnEndKb(const QString& strKbGUID) {}
    //
public:
    void OnStatus(const QString& strText) { OnText(wizhttpstatustypeNormal, strText); }
    void OnWarning(const QString& strText) { OnText(wizhttpstatustypeWarning, strText); }
    void OnError(const QString& strText) { OnText(wizhttpstatustypeError, strText); }
    //
private:
    BOOL m_bStop;
    int m_nLastError;
public:
    IWizKMSyncEvents()
    {
        m_bStop = FALSE;
        m_nLastError = 0;
    }
};


#endif // WIZSYNCABLEDATABASE_H
