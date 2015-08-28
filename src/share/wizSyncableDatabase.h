#ifndef WIZSYNCABLEDATABASE_H
#define WIZSYNCABLEDATABASE_H

#include "wizobject.h"


struct IWizSyncableDatabase
{
    virtual QString GetUserId() = 0;
    virtual QString GetUserGUID() = 0;
    virtual QString GetPassword() = 0;

    virtual qint64 GetObjectVersion(const QString& strObjectType) = 0;
    virtual bool SetObjectVersion(const QString& strObjectType, qint64 nVersion) = 0;
    virtual bool OnDownloadDeletedList(const CWizDeletedGUIDDataArray& arrayData) = 0;
    virtual bool OnDownloadTagList(const CWizTagDataArray& arrayData) = 0;
    virtual bool OnDownloadStyleList(const CWizStyleDataArray& arrayData) = 0;
    virtual bool OnDownloadAttachmentList(const CWizDocumentAttachmentDataArray& arrayData) = 0;

    virtual qint64 GetObjectLocalVersion(const QString& strObjectGUID,
                                         const QString& strObjectType) = 0;
    virtual qint64 GetObjectLocalServerVersion(const QString& strObjectGUID,
                                               const QString& strObjectType) = 0;
    virtual bool SetObjectLocalServerVersion(const QString& strObjectGUID,
                                             const QString& strObjectType,
                                             qint64 nVersion) = 0;
    virtual void OnObjectUploaded(const QString& strObjectGUID, const QString& strObjectType) = 0;

    virtual bool DocumentFromGUID(const QString& strGUID,
                                  WIZDOCUMENTDATA& dataExists) = 0;

    virtual bool SetObjectDataDownloaded(const QString& strGUID,
                                         const QString& strType,
                                         bool downloaded) = 0;

    virtual bool SetObjectServerDataInfo(const QString& strGUID,
                                         const QString& strType,
                                         COleDateTime& tServerDataModified,
                                         const QString& strServerMD5) = 0;

    virtual bool OnDownloadDocument(int part, const WIZDOCUMENTDATAEX& data) = 0;

    virtual bool GetObjectsNeedToBeDownloaded(CWizObjectDataArray& arrayObject) = 0;

    virtual bool UpdateObjectData(const QString& strObjectGUID,
                                  const QString& strObjectType,
                                  const QByteArray& stream) = 0;

    virtual bool IsObjectDataDownloaded(const QString& strGUID,
                                        const QString& strType) = 0;

    virtual bool GetModifiedDeletedList(CWizDeletedGUIDDataArray& arrayData) = 0;
    virtual bool GetModifiedTagList(CWizTagDataArray& arrayData) = 0;
    virtual bool GetModifiedStyleList(CWizStyleDataArray& arrayData) = 0;
    virtual bool GetModifiedDocumentList(CWizDocumentDataArray& arrayData) = 0;
    virtual bool GetModifiedAttachmentList(CWizDocumentAttachmentDataArray& arrayData) = 0;
    virtual bool GetModifiedMessageList(CWizMessageDataArray& arrayData) = 0;

    virtual bool InitDocumentData(const QString& strGUID, WIZDOCUMENTDATAEX& data, UINT part) = 0;
    virtual bool InitAttachmentData(const QString& strGUID, WIZDOCUMENTATTACHMENTDATAEX& data, UINT part) = 0;

    virtual bool OnUploadObject(const QString& strGUID, const QString& strObjectType) = 0;

    virtual bool ModifyDocumentsVersion(CWizDocumentDataArray& arrayData) = 0;

    virtual bool ModifyMessagesLocalChanged(CWizMessageDataArray &arrayData) = 0;

    virtual bool OnDownloadGroups(const CWizGroupDataArray& arrayGroup) = 0;
    virtual bool OnDownloadBizs(const CWizBizDataArray& arrayBiz) = 0;
    virtual IWizSyncableDatabase* GetGroupDatabase(const WIZGROUPDATA& group) = 0;
    virtual void CloseGroupDatabase(IWizSyncableDatabase* pDatabase) = 0;
    virtual IWizSyncableDatabase* GetPersonalDatabase() = 0;

    virtual void SetKbInfo(const QString& strKBGUID, const WIZKBINFO& info) = 0;
    virtual void SetUserInfo(const WIZUSERINFO& info) = 0;

    virtual bool IsGroup() = 0;
    virtual bool HasBiz() = 0;

    virtual bool IsGroupAdmin() = 0;
    virtual bool IsGroupSuper() = 0;
    virtual bool IsGroupEditor() = 0;
    virtual bool IsGroupAuthor() = 0;
    virtual bool IsGroupReader() = 0;

    virtual bool CanEditDocument(const WIZDOCUMENTDATA& data) = 0;
    virtual bool CanEditAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data) = 0;

    virtual bool CreateConflictedCopy(const QString& strObjectGUID,
                                      const QString& strObjectType) = 0;

    virtual bool SaveLastSyncTime() = 0;
    virtual COleDateTime GetLastSyncTime() = 0;

    virtual long GetLocalFlags(const QString& strObjectGUID,
                               const QString& strObjectType) = 0;

    virtual bool SetLocalFlags(const QString& strObjectGUID,
                               const QString& strObjectType, long flags) = 0;

    virtual void GetAccountKeys(CWizStdStringArray& arrayKey) = 0;
    virtual qint64 GetAccountLocalValueVersion(const QString& strKey) = 0;

    virtual void SetAccountLocalValue(const QString& strKey,
                                      const QString& strValue,
                                      qint64 nServerVersion,
                                      bool bSaveVersion) = 0;

    virtual void GetKBKeys(CWizStdStringArray& arrayKey) = 0;
    virtual bool ProcessValue(const QString& strKey) = 0;
    virtual qint64 GetLocalValueVersion(const QString& strKey) = 0;
    virtual QString GetLocalValue(const QString& strKey) = 0;

    virtual void SetLocalValueVersion(const QString& strKey,
                                      qint64 nServerVersion) = 0;

    virtual void SetLocalValue(const QString& strKey, const QString& strValue,
                               qint64 nServerVersion, bool bSaveVersion) = 0;

    //virtual CComPtr<IWizBizUserCollection> GetBizUsers() = 0;
    virtual void GetAllBizUserIds(CWizStdStringArray& arrayText) = 0;
    virtual bool GetAllBizUsers(CWizBizUserDataArray& arrayUser) = 0;
    virtual bool GetBizGUID(const QString& strGroupGUID, QString& strBizGUID) = 0;

    //virtual CComPtr<IWizDocument> GetDocumentByGUID(const QString& strDocumentGUID) = 0;
    virtual bool OnDownloadMessages(const CWizUserMessageDataArray& arrayMessage) = 0;

    virtual void ClearLastSyncError() = 0;
    virtual QString GetLastSyncErrorMessage() = 0;
    virtual void OnTrafficLimit(const QString& strErrorMessage) = 0;
    virtual void OnStorageLimit(const QString& strErrorMessage) = 0;
    virtual void OnNoteCountLimit(const QString& strErrorMessage) = 0;
    virtual void OnBizServiceExpr(const QString& strBizGUID, const QString& strErrorMessage) = 0;
    virtual bool IsTrafficLimit() = 0;
    virtual bool IsStorageLimit() = 0;
    virtual bool IsNoteCountLimit() = 0;
    virtual bool IsBizServiceExpr(const QString& strBizGUID) = 0;
    virtual bool GetStorageLimitMessage(QString& strErrorMessage) = 0;
    virtual bool GetTrafficLimitMessage(QString& strErrorMessage) = 0;
    virtual bool GetNoteCountLimit(QString& strErrorMessage) = 0;

    virtual bool setMeta(const QString& strSection, const QString& strKey, const QString& strValue) = 0;
    virtual QString meta(const QString& strSection, const QString& strKey) = 0;
    virtual void setBizGroupUsers(const QString& strkbGUID, const QString& strJson) = 0;

    virtual bool getAllNotesOwners(CWizStdStringArray &arrayOwners) = 0;
};



enum WizKMSyncProgressMessageType
{
    wizSyncMessageNormal,
    wizSyncMessageWarning,
    wizSyncMeesageError
};


enum WizBubbleMessageType {
    wizBubbleNoMessage,
    wizBubbleNormal,
    wizBubbleMessageCenter,
    wizBubbleUnknowMessage
};


struct IWizKMSyncEvents
{
public:
    IWizKMSyncEvents()
    {
        m_bStop = false;
        m_nLastError = 0;
    }

    virtual void OnSyncProgress(int pos) {}
    virtual HRESULT OnText(WizKMSyncProgressMessageType type, const QString& strStatus) = 0;
    virtual HRESULT OnMessage(WizKMSyncProgressMessageType type, const QString& strTitle, const QString& strMessage) = 0;
    virtual HRESULT OnBubbleNotification(const QVariant& param) = 0;

    virtual void SetStop(bool b) { m_bStop = b; }
    virtual bool IsStop() const { return m_bStop; }
    virtual void SetLastErrorCode(int nErrorCode) { m_nLastError = nErrorCode; }
    virtual int GetLastErrorCode() const { return m_nLastError; }
    virtual void SetDatabaseCount(int count) {}
    virtual void SetCurrentDatabase(int index) {}
    virtual void ClearLastSyncError(IWizSyncableDatabase* pDatabase) {}
    virtual void OnTrafficLimit(IWizSyncableDatabase* pDatabase) {}
    virtual void OnStorageLimit(IWizSyncableDatabase* pDatabase) {}
    virtual void OnBizServiceExpr(IWizSyncableDatabase* pDatabase) {}
    virtual void OnBizNoteCountLimit(IWizSyncableDatabase* pDatabase) {}
    virtual void OnUploadDocument(const QString& strDocumentGUID, bool bDone) {}
    virtual void OnBeginKb(const QString& strKbGUID) {}
    virtual void OnEndKb(const QString& strKbGUID) {}

public:
    void OnStatus(const QString& strText) { OnText(wizSyncMessageNormal, strText); }
    void OnWarning(const QString& strText) { OnText(wizSyncMessageWarning, strText); }
    void OnError(const QString& strText) { OnText(wizSyncMeesageError, strText); }

private:
    bool m_bStop;
    int m_nLastError;
};


#endif // WIZSYNCABLEDATABASE_H
