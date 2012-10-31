#ifndef WIZAPI_H
#define WIZAPI_H

#include "wizxmlrpc.h"
#include "wizobject.h"
#include "wizdatabase.h"


#define WIZ_API_URL "http://service.wiz.cn/wizkm/xmlrpc"

const char* const SyncMethod_ClientLogin = "accounts.clientLogin";
const char* const SyncMethod_ClientLogout = "accounts.clientLogout";
const char* const SyncMethod_CreateAccount = "accounts.createAccount";
const char* const SyncMethod_GetUserInfo = "wiz.getInfo";

const char* const SyncMethod_GetDeletedList = "deleted.getList";
const char* const SyncMethod_GetTagList = "tag.getList";
const char* const SyncMethod_GetStyleList = "style.getList";
const char* const SyncMethod_GetDocumentList = "document.getList";
const char* const SyncMethod_GetAttachmentList = "attachment.getList";

const char* const SyncMethod_PostDeletedList = "deleted.postList";
const char* const SyncMethod_PostTagList = "tag.postList";
const char* const SyncMethod_PostStyleList = "style.postList";
const char* const SyncMethod_PostDocumentList = "document.postList";
const char* const SyncMethod_PostAttachmentList = "attachment.postList";

const char* const SyncMethod_DownloadObjectPart = "data.download";
const char* const SyncMethod_UploadObjectPart = "data.upload";

const char* const SyncMethod_GetDocumentData = "document.getData";
const char* const SyncMethod_PostDocumentData = "document.postData";

const char* const SyncMethod_PostAttachmentData = "attachment.postData";

const char* const SyncMethod_GetDocumentsInfo = "document.downloadList";
const char* const SyncMethod_GetAttachmentsInfo = "attachment.downloadList";


struct WIZUSERINFO
{
    WIZUSERINFO();
    bool LoadFromXmlRpc(CWizXmlRpcValue& val);
    int GetMaxFileSize();

    QString strDisplayName;
    QString strUserType;
    QString strShowAD;
    QString strNickName;
    QString strLanguage;
    QString strDatabaseServer;
    QString strUploadDataServer;
    QString strDownloadDataServer;
    QString strChatServer;
    QString strBackupDatabaseServer;
    QString strToken;
    COleDateTime tTokenExpried;
    QString strKbGUID;

    QString strUserLevelName;
    int nUserLevel;
    int nUserPoints;

    QString strSNSList;

    QString strSystemTags;
    QString strPushTag;

    int nMaxFileSize;

    int bEnableGroup;

    QString strNotice;
};


struct WIZKBINFO
{
    WIZKBINFO();
    bool LoadFromXmlRpc(CWizXmlRpcStructValue& data);

    __int64 nStorageLimit;
    __int64 nStorageUsage;
    QString strStorageLimit;
    QString strStorageUsage;

    __int64 nTrafficLimit;
    __int64 nTrafficUsage;
    QString strTrafficLimit;
    QString strTrafficUsage;
};


struct WIZOBJECTPARTDATA
{
    WIZOBJECTPARTDATA();
    bool LoadFromXmlRpc(CWizXmlRpcStructValue& data);

    CString strObjectGUID;
    CString strObjectType;
    __int64 nStartPos;
    __int64 nQuerySize;

    __int64 nObjectSize;
    int bEOF;
    __int64 nPartSize;
    CString strPartMD5;
    QByteArray arrayData;
};


class CWizApiBase : public QObject
{
    Q_OBJECT

public:
    CWizApiBase(const CString& strAccountsApiURL, CWizSyncEvents& events);

    CString token() const { return m_user.strToken; }
    CString kbGUID() const { return m_user.strKbGUID; }
    const WIZUSERINFO& userInfo() const { return m_user; }

    virtual bool callXmlRpc(const CString& strMethodName, CWizXmlRpcValue* pVal);
    bool isSyncing() const;
    void resetProxy();
    virtual void abort();

protected:
    WIZUSERINFO m_user;
    CWizXmlRpcServer m_server;
    CWizSyncEvents& m_events;

protected:
    CString MakeXmlRpcUserId(const CString& strUserId);
    CString MakeXmlRpcPassword(const CString& strPassword);

protected:
    virtual void onXmlRpcReturn(const CString& strMethodName, CWizXmlRpcValue& ret);
    virtual void onXmlRpcError(const CString& strMethodName, WizXmlRpcError err, int errorCode, const CString& errorMessage);

    virtual bool callClientLogin(const CString& strUserId, const CString& strPassword);
    virtual void onClientLogin();

    virtual bool callClientLogout();
    virtual void onClientLogout();

    virtual bool callGetUserInfo();
    virtual void onGetUserInfo(CWizXmlRpcValue& ret);

    virtual bool callCreateAccount(const CString& strUserId, const CString& strPassword);
    virtual void onCreateAccount();

protected:
    virtual void changeProgress(int pos);
    virtual void addLog(const CString& str);
    virtual void addDebugLog(const CString& str);
    virtual void addErrorLog(const CString& str);

public Q_SLOTS:
    void xmlRpcReturn(const CString& strMethodName, CWizXmlRpcValue& ret);
    void xmlRpcError(const CString& strMethodName, WizXmlRpcError err, \
                     int errorCode, const CString& errorMessage);
};

class CWizApi : public CWizApiBase
{
    Q_OBJECT

public:
    CWizApi(CWizDatabase& db, const CString& strAccountsApiURL, CWizSyncEvents& events);

protected:
    CWizDatabase& m_db;
    WIZOBJECTPARTDATA m_currentObjectPartData;
    WIZOBJECTDATA m_currentObjectData;
    std::deque<WIZDELETEDGUIDDATA> m_arrayCurrentPostDeletedGUID;
    std::deque<WIZTAGDATA> m_arrayCurrentPostTag;
    std::deque<WIZSTYLEDATA> m_arrayCurrentPostStyle;

    WIZDOCUMENTDATAEX m_currentDocument;
    WIZDOCUMENTATTACHMENTDATAEX m_currentAttachment;

private:
    int m_nCurrentObjectAllSize;
    CString m_strCurrentObjectMD5;
    bool m_bDownloadingObject;

protected:
    virtual void onXmlRpcReturn(const CString& strMethodName, CWizXmlRpcValue& ret);

    virtual bool callDeletedGetList(__int64 nVersion);
    virtual void onDeletedGetList(const std::deque<WIZDELETEDGUIDDATA>& arrayRet);

    virtual bool callTagGetList(__int64 nVersion);
    virtual void onTagGetList(const std::deque<WIZTAGDATA>& arrayRet);

    virtual bool callStyleGetList(__int64 nVersion);
    virtual void onStyleGetList(const std::deque<WIZSTYLEDATA>& arrayRet);

    virtual bool callDocumentGetList(__int64 nVersion);
    virtual void onDocumentGetList(const std::deque<WIZDOCUMENTDATABASE>& arrayRet);

    virtual bool callAttachmentGetList(__int64 nVersion);
    virtual void onAttachmentGetList(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet);

    virtual bool callDownloadDataPart(const CString& strObjectGUID, const CString& strObjectType, int pos);
    virtual void onDownloadDataPart(const WIZOBJECTPARTDATA& data);

    virtual bool callUploadDataPart(const CString& strObjectGUID, const CString& strObjectType, const CString& strObjectMD5, int allSize, int partCount, int partIndex, int partSize, const QByteArray& arrayData);
    virtual void onUploadDataPart();

    virtual bool uploadObjectData(const WIZOBJECTDATA& data);
    virtual void onUploadObjectDataCompleted(const WIZOBJECTDATA& data);

    virtual bool callDocumentGetData(const WIZDOCUMENTDATABASE& data);
    virtual void onDocumentGetData(const WIZDOCUMENTDATAEX& data);

    virtual bool uploadDocument(const WIZDOCUMENTDATAEX& data);
    virtual void onUploadDocument(const WIZDOCUMENTDATAEX& data);

    virtual bool callDocumentPostData(const WIZDOCUMENTDATAEX& data);
    virtual void onDocumentPostData(const WIZDOCUMENTDATAEX& data);

    virtual bool uploadAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data);
    virtual void onUploadAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data);

    virtual bool callAttachmentPostData(const WIZDOCUMENTATTACHMENTDATAEX& data);
    virtual void onAttachmentPostData(const WIZDOCUMENTATTACHMENTDATAEX& data);

    virtual bool downloadObjectData(const WIZOBJECTDATA& data);
    virtual void onDownloadObjectDataCompleted(const WIZOBJECTDATA& data);

    virtual bool callDeletedPostList(const std::deque<WIZDELETEDGUIDDATA>& arrayData);
    virtual void onDeletedPostList(const std::deque<WIZDELETEDGUIDDATA>& arrayData);

    virtual bool callTagPostList(const std::deque<WIZTAGDATA>& arrayData);
    virtual void onTagPostList(const std::deque<WIZTAGDATA>& arrayData);

    virtual bool callStylePostList(const std::deque<WIZSTYLEDATA>& arrayData);
    virtual void onStylePostList(const std::deque<WIZSTYLEDATA>& arrayData);

    virtual bool callDocumentsGetInfo(const CWizStdStringArray& arrayDocumentGUID);
    virtual void onDocumentsGetInfo(const std::deque<WIZDOCUMENTDATABASE>& arrayRet);

    virtual bool callAttachmentsGetInfo(const CWizStdStringArray& arrayAttachmentGUID);
    virtual void onAttachmentsGetInfo(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet);

    virtual unsigned int getCountPerPage() const;
    virtual unsigned int getPartSize() const;

    virtual bool downloadNextPartData();
    virtual bool uploadNextPartData();

protected:
    bool callGetList(const CString& strMethodName, __int64 nVersion);

    template <class TData>
    bool callPostList(const CString& strMethodName, const CString& strArrayName, const std::deque<TData>& arrayData);
};


class CWizApiParamBase : public CWizXmlRpcStructValue
{
public:
    CWizApiParamBase();
};

class CWizApiTokenParam : public CWizApiParamBase
{
public:
    CWizApiTokenParam(CWizApiBase& api);
};

template <class TData>
inline BOOL CWizApi::callPostList(const CString& strMethodName, const CString& strArrayName, const std::deque<TData>& arrayData)
{
    if (arrayData.empty())
        return TRUE;
    //
    CWizApiTokenParam param(*this);
    //
    param.AddArray<TData>(strArrayName, arrayData);
    //
    return callXmlRpc(strMethodName, &param);
}


#endif //WIZAPI_H
