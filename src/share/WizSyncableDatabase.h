#ifndef WIZSYNCABLEDATABASE_H
#define WIZSYNCABLEDATABASE_H

#include "WizObject.h"


struct IWizSyncableDatabase
{
    virtual QString getUserId() = 0;
    virtual QString getUserGuid() = 0;
    virtual QString getPassword() = 0;

    virtual qint64 getObjectVersion(const QString& strObjectType) = 0;
    virtual bool setObjectVersion(const QString& strObjectType, qint64 nVersion) = 0;
    virtual bool onDownloadDeletedList(const CWizDeletedGUIDDataArray& arrayData) = 0;
    virtual bool onDownloadTagList(const CWizTagDataArray& arrayData) = 0;
    virtual bool onDownloadStyleList(const CWizStyleDataArray& arrayData) = 0;
    virtual bool onDownloadDocumentList(const CWizDocumentDataArray& arrayData) = 0;
    virtual bool onDownloadAttachmentList(const CWizDocumentAttachmentDataArray& arrayData) = 0;
    virtual bool onDownloadMessageList(const CWizMessageDataArray& arrayMessage) = 0;
    virtual bool onDownloadParamList(const CWizDocumentParamDataArray& arrayData) = 0;

    virtual QString getDocumentFileName(const QString& strGUID) const = 0;

    virtual qint64 getObjectLocalVersion(const QString& strObjectGUID,
                                         const QString& strObjectType) = 0;
    virtual qint64 getObjectLocalServerVersion(const QString& strObjectGUID,
                                               const QString& strObjectType) = 0;
    virtual bool setObjectLocalServerVersion(const QString& strObjectGUID,
                                             const QString& strObjectType,
                                             qint64 nVersion) = 0;
    virtual void onObjectUploaded(const QString& strObjectGUID, const QString& strObjectType) = 0;

    virtual bool documentFromGuid(const QString& strGUID,
                                  WIZDOCUMENTDATA& dataExists) = 0;

    virtual bool setObjectDataDownloaded(const QString& strGUID,
                                         const QString& strType,
                                         bool downloaded) = 0;

    virtual bool setObjectServerDataInfo(const QString& strGUID,
                                         const QString& strType,
                                         WizOleDateTime& tServerDataModified,
                                         const QString& strServerMD5) = 0;

    virtual bool getObjectsNeedToBeDownloaded(CWizObjectDataArray& arrayObject) = 0;

    virtual bool updateObjectData(const QString& strDisplayName,
                                  const QString& strObjectGUID,
                                  const QString& strObjectType,
                                  const QByteArray& stream) = 0;

    virtual bool isObjectDataDownloaded(const QString& strGUID,
                                        const QString& strType) = 0;

    virtual bool getModifiedDeletedList(CWizDeletedGUIDDataArray& arrayData) = 0;
    virtual bool getModifiedTagList(CWizTagDataArray& arrayData) = 0;
    virtual bool getModifiedStyleList(CWizStyleDataArray& arrayData) = 0;
    virtual bool getModifiedDocumentList(CWizDocumentDataArray& arrayData) = 0;
    virtual bool getModifiedAttachmentList(CWizDocumentAttachmentDataArray& arrayData) = 0;
    virtual bool getModifiedMessageList(CWizMessageDataArray& arrayData) = 0;
    virtual bool getModifiedParamList(CWizDocumentParamDataArray& arrayData) = 0;

    virtual bool initDocumentData(const QString& strGUID, WIZDOCUMENTDATAEX& data, bool forceUploadData) = 0;
    virtual bool initAttachmentData(const QString& strGUID, WIZDOCUMENTATTACHMENTDATAEX& data) = 0;

    virtual bool onUploadObject(const QString& strGUID, const QString& strObjectType) = 0;
    virtual bool onUploadParam(const QString& strDocumentGuid, const QString& strName) = 0;

    virtual bool modifyMessagesLocalChanged(CWizMessageDataArray &arrayData) = 0;

    virtual bool onDownloadGroups(const CWizGroupDataArray& arrayGroup) = 0;
    virtual bool onDownloadBizs(const CWizBizDataArray& arrayBiz) = 0;
    virtual bool onDownloadBizUsers(const QString& kbGuid, const CWizBizUserDataArray& arrayUser) = 0;
    virtual IWizSyncableDatabase* getGroupDatabase(const WIZGROUPDATA& group) = 0;
    virtual void closeGroupDatabase(IWizSyncableDatabase* pDatabase) = 0;
    virtual IWizSyncableDatabase* getPersonalDatabase() = 0;

    virtual void setKbInfo(const QString& strKBGUID, const WIZKBINFO& info) = 0;
    virtual void setUserInfo(const WIZUSERINFO& info) = 0;
    //
    virtual QString getGroupName() = 0;
    virtual WIZGROUPDATA getGroupInfo() = 0;

    virtual bool isGroup() = 0;
    virtual bool hasBiz() = 0;

    virtual bool isGroupAdmin() = 0;
    virtual bool isGroupSuper() = 0;
    virtual bool isGroupEditor() = 0;
    virtual bool isGroupAuthor() = 0;
    virtual bool isGroupReader() = 0;

    virtual bool canEditDocument(const WIZDOCUMENTDATA& data) = 0;
    virtual bool canEditAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data) = 0;

    virtual bool createConflictedCopy(const QString& strObjectGUID,
                                      const QString& strObjectType) = 0;

    virtual bool saveLastSyncTime() = 0;
    virtual WizOleDateTime getLastSyncTime() = 0;

    virtual long getLocalFlags(const QString& strObjectGUID,
                               const QString& strObjectType) = 0;

    virtual bool setLocalFlags(const QString& strObjectGUID,
                               const QString& strObjectType, long flags) = 0;

    virtual void getAccountKeys(CWizStdStringArray& arrayKey) = 0;
    virtual qint64 getAccountLocalValueVersion(const QString& strKey) = 0;

    virtual void setAccountLocalValue(const QString& strKey,
                                      const QString& strValue,
                                      qint64 nServerVersion,
                                      bool bSaveVersion) = 0;

    virtual void getKBKeys(CWizStdStringArray& arrayKey) = 0;
    virtual bool processValue(const QString& strKey) = 0;
    virtual qint64 getLocalValueVersion(const QString& strKey) = 0;
    virtual QString getLocalValue(const QString& strKey) = 0;

    virtual void setLocalValueVersion(const QString& strKey,
                                      qint64 nServerVersion) = 0;

    virtual void setLocalValue(const QString& strKey, const QString& strValue,
                               qint64 nServerVersion, bool bSaveVersion) = 0;

    //virtual CComPtr<IWizBizUserCollection> GetBizUsers() = 0;
    virtual void getAllBizUserIds(CWizStdStringArray& arrayText) = 0;
    virtual bool getAllBizUsers(CWizBizUserDataArray& arrayUser) = 0;
    virtual bool getBizGuid(const QString& strGroupGUID, QString& strBizGUID) = 0;
    virtual bool getBizData(const QString& bizGUID, WIZBIZDATA& biz) = 0;

    //virtual CComPtr<IWizDocument> GetDocumentByGUID(const QString& strDocumentGUID) = 0;

    virtual void clearLastSyncError() = 0;
    virtual QString getLastSyncErrorMessage() = 0;
    virtual void onTrafficLimit(const QString& strErrorMessage) = 0;
    virtual void onStorageLimit(const QString& strErrorMessage) = 0;
    virtual void onNoteCountLimit(const QString& strErrorMessage) = 0;
    virtual void onBizServiceExpr(const QString& strBizGUID, const QString& strErrorMessage) = 0;
    virtual bool isTrafficLimit() = 0;
    virtual bool isStorageLimit() = 0;
    virtual bool isNoteCountLimit() = 0;
    virtual bool isBizServiceExpr(const QString& strBizGUID) = 0;
    virtual bool getStorageLimitMessage(QString& strErrorMessage) = 0;
    virtual bool getTrafficLimitMessage(QString& strErrorMessage) = 0;
    virtual bool getNoteCountLimit(QString& strErrorMessage) = 0;

    virtual bool setMeta(const QString& strSection, const QString& strKey, const QString& strValue) = 0;
    virtual QString meta(const QString& strSection, const QString& strKey) = 0;

    virtual bool getAllNotesOwners(CWizStdStringArray &arrayOwners) = 0;
    //
    virtual bool deleteDocumentFromLocal(const QString& strDocumentGuid) = 0;
    virtual bool deleteAttachmentFromLocal(const QString& strAttachmentGuid) = 0;
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
        m_bIsNetworkError = false;
    }

    virtual void onSyncProgress(int pos) {}
    virtual HRESULT onText(WizKMSyncProgressMessageType type, const QString& strStatus) = 0;
    virtual HRESULT onMessage(WizKMSyncProgressMessageType type, const QString& strTitle, const QString& strMessage) = 0;
    virtual HRESULT onBubbleNotification(const QVariant& param) = 0;

    virtual void setStop(bool b) { m_bStop = b; }
    virtual bool isStop() const { return m_bStop; }
    virtual void setLastErrorCode(int nErrorCode) { m_nLastError = nErrorCode; }
    virtual bool isNetworkError() const { return m_bIsNetworkError; }
    virtual void setIsNetworkError(bool networkError) { m_bIsNetworkError = networkError; }
    virtual int getLastErrorCode() const { return m_nLastError; }
    virtual void setLastErrorMessage(const QString& message) { m_strLastErrorMessage = message; }
    virtual QString getLastErrorMessage() const { return m_strLastErrorMessage; }
    virtual void clearLastErrorMessage() { m_strLastErrorMessage.clear(); }
    virtual void setDatabaseCount(int count) {}
    virtual void setCurrentDatabase(int index) {}
    virtual void clearLastSyncError(IWizSyncableDatabase* pDatabase) {}
    virtual void onTrafficLimit(IWizSyncableDatabase* pDatabase) {}
    virtual void onStorageLimit(IWizSyncableDatabase* pDatabase) {}
    virtual void onBizServiceExpr(IWizSyncableDatabase* pDatabase) {}
    virtual void onBizNoteCountLimit(IWizSyncableDatabase* pDatabase) {}
    virtual void onFreeServiceExpr(WIZGROUPDATA group) {}
    virtual void onVipServiceExpr(WIZGROUPDATA group) {}
    virtual void onUploadDocument(const QString& strDocumentGUID, bool bDone) {}
    virtual void onBeginKb(const QString& strKbGUID) {}
    virtual void onEndKb(const QString& strKbGUID) {}

public:
    void onStatus(const QString& strText) { onText(wizSyncMessageNormal, strText); }
    void onWarning(const QString& strText) { onText(wizSyncMessageWarning, strText); }
    void onError(const QString& strText) { onText(wizSyncMeesageError, strText); }

private:
    bool m_bStop;
    int m_nLastError;
    bool m_bIsNetworkError;
    QString m_strLastErrorMessage;
};


#endif // WIZSYNCABLEDATABASE_H
