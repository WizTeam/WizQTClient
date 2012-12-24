#ifndef WIZAPI_H
#define WIZAPI_H

#include "wizxmlrpc.h"
#include "wizobject.h"
#include "wizdatabase.h"


#define WIZ_API_URL "http://service.wiz.cn/wizkm/xmlrpc"

const char* const SyncMethod_ClientLogin = "accounts.clientLogin";
const char* const SyncMethod_ClientLogout = "accounts.clientLogout";
const char* const SyncMethod_GetUserCert = "accounts.getCert";
const char* const SyncMethod_CreateAccount = "accounts.createAccount";
const char* const SyncMethod_GetUserInfo = "wiz.getInfo";

const char* const SyncMethod_GetDeletedList = "deleted.getList";
const char* const SyncMethod_PostDeletedList = "deleted.postList";

const char* const SyncMethod_GetTagList = "tag.getList";
const char* const SyncMethod_PostTagList = "tag.postList";

const char* const SyncMethod_GetStyleList = "style.getList";
const char* const SyncMethod_PostStyleList = "style.postList";

const char* const SyncMethod_GetDocumentList = "document.getList";
const char* const SyncMethod_GetDocumentData = "document.getData";
const char* const SyncMethod_GetDocumentsInfo = "document.downloadList";

const char* const SyncMethod_GetAttachmentList = "attachment.getList";
const char* const SyncMethod_GetAttachmentsInfo = "attachment.downloadList";

const char* const SyncMethod_DownloadObjectPart = "data.download";

const char* const SyncMethod_PostDocumentData = "document.postData";
const char* const SyncMethod_PostAttachmentData = "attachment.postData";

const char* const SyncMethod_UploadObjectPart = "data.upload";


/*
CWizApiBase is an abstract class,  which do nothing useful but just construct
invoke flow of xmlprc not related to syncing(eg: login, create account, logout).
subclass of CWizApiBase should reimplement "onXXX" like callback functions to
do some useful things.
*/
class CWizApiBase : public QObject
{
    Q_OBJECT

public:
    CWizApiBase(const QString& strAccountsApiURL = WIZ_API_URL);

    CString token() const { return m_user.strToken; }
    CString kbGUID() const { return m_user.strKbGUID; }
    const WIZUSERINFO& userInfo() const { return m_user; }

    virtual bool callXmlRpc(const QString& strMethodName, CWizXmlRpcValue* pVal);
    void resetProxy();
    virtual void abort();

protected:
    WIZUSERINFO m_user;
    QPointer<CWizXmlRpcServer> m_server;

protected:
    CString MakeXmlRpcUserId(const CString& strUserId);
    CString MakeXmlRpcPassword(const CString& strPassword);

protected:
    virtual void onXmlRpcReturn(const QString& strMethodName, CWizXmlRpcValue& ret);
    virtual void onXmlRpcError(const QString& strMethodName, WizXmlRpcError err, int errorCode, const QString& errorMessage);

    virtual bool callClientLogin(const QString& strUserId, const QString& strPassword);
    virtual void onClientLogin(const WIZUSERINFO& userInfo);

    virtual bool callClientLogout();
    virtual void onClientLogout();

    virtual bool callGetUserInfo();
    virtual void onGetUserInfo(CWizXmlRpcValue& ret);

    virtual bool callGetUserCert(const QString& strUserId, const QString& strPassword);
    virtual void onGetUserCert(const WIZUSERCERT& data);

    virtual bool callCreateAccount(const CString& strUserId, const CString& strPassword);
    virtual void onCreateAccount();

Q_SIGNALS:
    void progressChanged(int pos);
    void processLog(const QString& str);
    void processDebugLog(const QString& str);
    void processErrorLog(const QString& str);

public Q_SLOTS:
    void xmlRpcReturn(const QString& strMethodName, CWizXmlRpcValue& ret);
    void xmlRpcError(const QString& strMethodName, WizXmlRpcError err, \
                     int errorCode, const QString& errorMessage);
};

/*
It's all about syncing, operate with database and do real things, subclass of
CWizApi(eg: CWizSync) should not operate database directly, instead call
CWizApi base implementations
*/
class CWizApi : public CWizApiBase
{
    Q_OBJECT

public:
    CWizApi(CWizDatabase& db, const CString& strAccountsApiURL = WIZ_API_URL);

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
    virtual void onXmlRpcReturn(const QString& strMethodName, CWizXmlRpcValue& ret);

    virtual void onClientLogin(const WIZUSERINFO& userInfo);

    virtual bool callDeletedGetList(__int64 nVersion);
    virtual void onDeletedGetList(const std::deque<WIZDELETEDGUIDDATA>& arrayRet);

    virtual bool callDeletedPostList(const std::deque<WIZDELETEDGUIDDATA>& arrayData);
    virtual void onDeletedPostList(const std::deque<WIZDELETEDGUIDDATA>& arrayData);

    virtual bool callTagGetList(__int64 nVersion);
    virtual void onTagGetList(const std::deque<WIZTAGDATA>& arrayRet);

    virtual bool callTagPostList(const std::deque<WIZTAGDATA>& arrayData);
    virtual void onTagPostList(const std::deque<WIZTAGDATA>& arrayData);

    virtual bool callStyleGetList(__int64 nVersion);
    virtual void onStyleGetList(const std::deque<WIZSTYLEDATA>& arrayRet);

    virtual bool callStylePostList(const std::deque<WIZSTYLEDATA>& arrayData);
    virtual void onStylePostList(const std::deque<WIZSTYLEDATA>& arrayData);

    virtual bool callDocumentGetList(__int64 nVersion);
    virtual void onDocumentGetList(const std::deque<WIZDOCUMENTDATABASE>& arrayRet);

    virtual bool callDocumentGetData(const WIZDOCUMENTDATABASE& data);
    virtual void onDocumentGetData(const WIZDOCUMENTDATAEX& data);

    virtual bool callDocumentsGetInfo(const CWizStdStringArray& arrayDocumentGUID);
    virtual void onDocumentsGetInfo(const std::deque<WIZDOCUMENTDATABASE>& arrayRet);

    virtual bool callAttachmentGetList(__int64 nVersion);
    virtual void onAttachmentGetList(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet);

    virtual bool callAttachmentsGetInfo(const CWizStdStringArray& arrayAttachmentGUID);
    virtual void onAttachmentsGetInfo(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet);

    // download several slices of data (callDownloadDataPart)
    virtual bool downloadObjectData(const WIZOBJECTDATA& data);
    virtual bool downloadNextPartData();
    virtual void onDownloadObjectDataCompleted(const WIZOBJECTDATA& data);

    virtual bool callDownloadDataPart(const CString& strObjectGUID, \
                                      const CString& strObjectType, \
                                      int pos);
    virtual void onDownloadDataPart(const WIZOBJECTPARTDATA& data);

    // upload both document info (callDocumentPostData) and document data (callUploadDataPart)
    virtual bool uploadDocument(const WIZDOCUMENTDATAEX& data);
    virtual void onUploadDocument(const WIZDOCUMENTDATAEX& data);

    virtual bool callDocumentPostData(const WIZDOCUMENTDATAEX& data);
    virtual void onDocumentPostData(const WIZDOCUMENTDATAEX& data);

    // upload both attachment info (callAttachmentPostData) and document data (callUploadDataPart)
    virtual bool uploadAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data);
    virtual void onUploadAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data);

    virtual bool callAttachmentPostData(const WIZDOCUMENTATTACHMENTDATAEX& data);
    virtual void onAttachmentPostData(const WIZDOCUMENTATTACHMENTDATAEX& data);

    // upload several slices of data (callUploadDataPart)
    virtual bool uploadObjectData(const WIZOBJECTDATA& data);
    virtual bool uploadNextPartData();
    virtual void onUploadObjectDataCompleted(const WIZOBJECTDATA& data);

    virtual bool callUploadDataPart(const CString& strObjectGUID, \
                                    const CString& strObjectType, \
                                    const CString& strObjectMD5, \
                                    int allSize, int partCount, \
                                    int partIndex, int partSize, \
                                    const QByteArray& arrayData);
    virtual void onUploadDataPart();

    virtual unsigned int getCountPerPage() const;
    virtual unsigned int getPartSize() const;


protected:
    bool callGetList(const QString& strMethodName, __int64 nVersion);

    template <class TData>
    bool callPostList(const QString& strMethodName, const CString& strArrayName, const std::deque<TData>& arrayData);
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
inline BOOL CWizApi::callPostList(const QString& strMethodName, const CString& strArrayName, const std::deque<TData>& arrayData)
{
    if (arrayData.empty())
        return TRUE;

    CWizApiTokenParam param(*this);

    param.AddArray<TData>(strArrayName, arrayData);

    return callXmlRpc(strMethodName, &param);
}


#endif //WIZAPI_H
