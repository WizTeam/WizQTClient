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

class IWizSyncableDatabase
{
protected:
    // general
    virtual QString GetUserId() const = 0;
    virtual QString GetPasssword() const = 0;

    virtual WIZDATABASEINFO GetInfo() const = 0;
    virtual bool SetInfo(const WIZDATABASEINFO& dbInfo) = 0;

    virtual qint64 GetObjectVersion(const QString& strObjectName) = 0;
    virtual bool SetObjectVersion(const QString& strObjectName,
                                  qint64 nVersion) = 0;

    virtual bool ModifyObjectVersion(const QString& strGUID,
                                     const QString& strType,
                                     qint64 nVersion) = 0;

    // group related
    virtual bool GetBizGroupInfo(QMap<QString, QString>& bizInfo) = 0;
    virtual bool UpdateBizUsers(const CWizBizUserDataArray& arrayUser) = 0;

    virtual bool GetUserGroupInfo(CWizGroupDataArray& arrayGroup) = 0;
    virtual bool SetUserGroupInfo(const CWizGroupDataArray& arrayGroup) = 0;

    // messages
    virtual bool GetModifiedMessages(CWizMessageDataArray& arrayMsg) = 0;
    virtual bool UpdateMessages(const CWizMessageDataArray& arrayMsg) = 0;

    // deleteds
    virtual bool GetModifiedDeletedGUIDs(CWizDeletedGUIDDataArray& arrayData) = 0;
    virtual bool UpdateDeletedGUIDs(const CWizDeletedGUIDDataArray& arrayDeletedGUID) = 0;
    virtual bool DeleteDeletedGUID(const QString& strGUID) = 0;

    // tags
    virtual bool GetModifiedTags(CWizTagDataArray& arrayData) = 0;
    virtual bool UpdateTags(const CWizTagDataArray& arrayTag) = 0;

    // styles
    virtual bool GetModifiedStyles(CWizStyleDataArray& arrayData) = 0;
    virtual bool UpdateStyles(const CWizStyleDataArray& arrayStyle) = 0;

    // document info
    virtual bool GetModifiedDocuments(CWizDocumentDataArray& arrayData) = 0;
    virtual bool UpdateDocument(const WIZDOCUMENTDATAEX& data) = 0;

    // attachment info
    virtual bool GetModifiedAttachments(CWizDocumentAttachmentDataArray& arrayData) = 0;
    virtual bool UpdateAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data) = 0;
    virtual bool UpdateAttachments(const CWizDocumentAttachmentDataArray& arrayAttachment) = 0;

    // object data
    virtual bool GetAllObjectsNeedToBeDownloaded(CWizObjectDataArray& arrayData) = 0;
    virtual bool UpdateSyncObjectLocalData(const WIZOBJECTDATA& data) = 0;
    virtual bool SetObjectDataDownloaded(const QString& strGUID,
                                         const QString& strType,
                                         bool bDownloaded) = 0;

    virtual bool LoadDocumentData(const QString& strDocumentGUID,
                                  QByteArray& arrayData) = 0;

    virtual bool LoadCompressedAttachmentData(const QString& strDocumentGUID,
                                              QByteArray& arrayData) = 0;
};

#endif // WIZSYNCABLEDATABASE_H
