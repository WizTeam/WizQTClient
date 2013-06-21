#ifndef WIZAPI_H
#define WIZAPI_H

#include "wizdef.h"

#include "wizXmlRpcServer.h"
#include "wizobject.h"
#include "wizDatabase.h"

#define WIZ_API_VERSION     "3"
#define WIZ_API_URL "http://service.wiz.cn/wizkm/xmlrpc"
#define WIZAPI_TRUNK_SIZE   512*1024
#define WIZAPI_PAGE_MAX     80

const char* const SyncMethod_CreateAccount = "accounts.createAccount";

const char* const SyncMethod_ClientLogin = "accounts.clientLogin";
const char* const SyncMethod_ClientKeepAlive = "accounts.keepAlive";
const char* const SyncMethod_ClientLogout = "accounts.clientLogout";
const char* const SyncMethod_GetUserInfo = "wiz.getInfo";

const char* const SyncMethod_GetUserCert = "accounts.getCert";

const char* const SyncMethod_GetGroupList = "accounts.getGroupKbList";

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

// messages
const char* const SyncMethod_getMessages = "accounts.getMessages";
const char* const SyncMethod_setMessageStatus = "accounts.setReadStatus";




/*
CWizApiBase is an abstract class,  which do nothing useful but just construct
invoke flow of xmlprc not related to database(eg: login, create account, logout).
subclass of CWizApiBase should reimplement "onXXX" like callback functions to
do some useful things.
*/
class CWizApiBase : public QObject
{
    Q_OBJECT

public:
    CWizApiBase(const QString& strKbUrl = WIZ_API_URL);

    // CWizXmlRpcServer passthrough method
    void resetProxy();
    virtual void abort() { m_server->abort(); }

    void setKbUrl(const QString& strKbUrl) { m_server->setKbUrl(strKbUrl); }

    //QString token() const { return m_user.strToken; }
    //void setToken(const QString& strToken) { m_user.strToken = strToken; }

    QString kbGUID() const { return m_strKbGUID; }
    void setKbGUID(const QString& strKbGUID) { m_strKbGUID = strKbGUID; }

    //const WIZUSERINFO& userInfo() const { return m_user; }

    virtual bool callXmlRpc(const QString& strMethodName, CWizXmlRpcValue* pVal);

public:
    virtual bool callClientLogin(const QString& strUserId, const QString& strPassword);

    virtual bool callGetUserInfo();
    virtual bool callGetUserCert(const QString& strUserId, const QString& strPassword);
    virtual bool callGetGroupList();
    virtual bool callCreateAccount(const CString& strUserId, const CString& strPassword);

    virtual bool callDeletedGetList(__int64 nVersion);
    virtual bool callDeletedPostList(const std::deque<WIZDELETEDGUIDDATA>& arrayData);

    virtual bool callTagGetList(__int64 nVersion);
    virtual bool callTagPostList(const std::deque<WIZTAGDATA>& arrayData);

    virtual bool callStyleGetList(__int64 nVersion);
    virtual bool callStylePostList(const std::deque<WIZSTYLEDATA>& arrayData);

    virtual bool callDocumentGetList(__int64 nVersion);
    virtual bool callDocumentsGetInfo(const CWizStdStringArray& arrayDocumentGUID);
    virtual bool callDocumentGetData(const WIZDOCUMENTDATABASE& data);
    virtual bool callDocumentPostData(const WIZDOCUMENTDATAEX& data);

    virtual bool callAttachmentGetList(__int64 nVersion);
    virtual bool callAttachmentsGetInfo(const CWizStdStringArray& arrayAttachmentGUID);
    virtual bool callAttachmentPostData(const WIZDOCUMENTATTACHMENTDATAEX& data);

    virtual bool downloadObjectData(const WIZOBJECTDATA& data);
    virtual bool uploadObjectData(const WIZOBJECTDATA& data);

protected:
    virtual void onXmlRpcReturn(const QString& strMethodName, CWizXmlRpcValue& ret);
    virtual void onXmlRpcError(const QString& strMethodName, WizXmlRpcError err, int errorCode, const QString& errorMessage);

    virtual void onClientLogin(const WIZUSERINFO& userInfo) { Q_UNUSED(userInfo); }

    virtual void onGetUserInfo(CWizXmlRpcValue& ret) { Q_UNUSED(ret); }
    virtual void onGetUserCert(const WIZUSERCERT& data) { Q_UNUSED(data); }
    virtual void onGetGroupList(const CWizGroupDataArray& arrayGroup) { Q_UNUSED(arrayGroup); }
    virtual void onCreateAccount() {}

    virtual void onDeletedGetList(const std::deque<WIZDELETEDGUIDDATA>& arrayRet) { Q_UNUSED(arrayRet); }
    virtual void onDeletedPostList(const std::deque<WIZDELETEDGUIDDATA>& arrayData) { Q_UNUSED(arrayData); }

    virtual void onTagGetList(const std::deque<WIZTAGDATA>& arrayRet) { Q_UNUSED(arrayRet); }
    virtual void onTagPostList(const std::deque<WIZTAGDATA>& arrayData) { Q_UNUSED(arrayData); }

    virtual void onStyleGetList(const std::deque<WIZSTYLEDATA>& arrayRet) { Q_UNUSED(arrayRet); }
    virtual void onStylePostList(const std::deque<WIZSTYLEDATA>& arrayData) { Q_UNUSED(arrayData); }

    virtual void onDocumentGetList(const std::deque<WIZDOCUMENTDATABASE>& arrayRet) { Q_UNUSED(arrayRet); }
    virtual void onDocumentGetData(const WIZDOCUMENTDATAEX& data) { Q_UNUSED(data); }
    virtual void onDocumentsGetInfo(const std::deque<WIZDOCUMENTDATABASE>& arrayRet) { Q_UNUSED(arrayRet); }
    virtual void onDocumentPostData(const WIZDOCUMENTDATAEX& data) { Q_UNUSED(data); }

    virtual void onAttachmentGetList(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet) { Q_UNUSED(arrayRet); }
    virtual void onAttachmentsGetInfo(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet) { Q_UNUSED(arrayRet); }
    virtual void onAttachmentPostData(const WIZDOCUMENTATTACHMENTDATAEX& data) { Q_UNUSED(data); }

    // download trunk data
    virtual bool callDownloadDataPart(const CString& strObjectGUID,
                                      const CString& strObjectType,
                                      int pos);
    virtual bool downloadNextPartData();
    virtual void onDownloadDataPart(const WIZOBJECTPARTDATA& data);
    virtual void onDownloadObjectDataCompleted(const WIZOBJECTDATA& data) { Q_UNUSED(data); }

    // upload trunk data
    virtual bool callUploadDataPart(const CString& strObjectGUID, \
                                    const CString& strObjectType, \
                                    const CString& strObjectMD5, \
                                    int allSize, int partCount, \
                                    int partIndex, int partSize, \
                                    const QByteArray& arrayData);
    virtual bool uploadNextPartData();
    virtual void onUploadDataPart();
    virtual void onUploadObjectDataCompleted(const WIZOBJECTDATA& data) { Q_UNUSED(data); }

    bool callGetList(const QString& strMethodName, __int64 nVersion);
    template <class TData> bool callPostList(const QString& strMethodName,
                                             const CString& strArrayName,
                                             const std::deque<TData>& arrayData);

//protected:
//    WIZUSERINFO m_user;

private:
    QPointer<CWizXmlRpcServer> m_server;
    QString m_strKbGUID;
    QString m_strUserId;
    QString m_strPasswd;

    CWizDeletedGUIDDataArray m_arrayCurrentPostDeletedGUID;
    CWizTagDataArray m_arrayCurrentPostTag;
    CWizStyleDataArray m_arrayCurrentPostStyle;
    WIZDOCUMENTDATAEX m_currentDocument;
    WIZDOCUMENTATTACHMENTDATAEX m_currentAttachment;
    WIZOBJECTPARTDATA m_currentObjectPartData;
    WIZOBJECTDATA m_currentObjectData;

    int m_nCurrentObjectAllSize;
    CString m_strCurrentObjectMD5;
    bool m_bDownloadingObject;

    CString MakeXmlRpcUserId(const CString& strUserId);
    CString MakeXmlRpcPassword(const CString& strPassword);

    void _onXmlRpcError(const QString& strMethodName, WizXmlRpcError err, int errorCode, const QString& errorMessage);

    // not have to invoke outside
    virtual bool callClientKeepAlive();
    virtual bool callClientLogout();
    virtual void onClientLogout();


Q_SIGNALS:
    void progressChanged(int pos);
    void processLog(const QString& str);
    void processDebugLog(const QString& str);
    void processErrorLog(const QString& str);

    void clientLoginDone();
    void clientLogoutDone();
    void getGroupListDone(const CWizGroupDataArray& arrayGroup);
    void createAccountDone();
    void getUserInfoDone();
    void getUserCertDone(const WIZUSERCERT& data);

public Q_SLOTS:
    void xmlRpcReturn(const QString& strMethodName, CWizXmlRpcValue& ret);
    void xmlRpcError(const QString& strMethodName, WizXmlRpcError err, \
                     int errorCode, const QString& errorMessage);
};

/*
all operations on this class will record to database and do real things, subclass of
CWizApi(eg: CWizKbSync) should not operate database directly, instead call CWizApi
base implementations
*/

class CWizApi : public CWizApiBase
{
    Q_OBJECT

public:
    CWizApi(CWizDatabase& db, const CString& strKbUrl = WIZ_API_URL);

    CWizDatabase* database() { return m_db; }
    void setDatabase(CWizDatabase& db) { m_db = &db; }

protected:
    CWizDatabase* m_db;
    WIZDOCUMENTDATAEX m_currentDocument;
    WIZDOCUMENTATTACHMENTDATAEX m_currentAttachment;

protected:
    virtual void onClientLogin(const WIZUSERINFO& userInfo);
    virtual void onGetGroupList(const CWizGroupDataArray& arrayGroup);

    virtual void onDeletedGetList(const std::deque<WIZDELETEDGUIDDATA>& arrayRet);
    virtual void onDeletedPostList(const std::deque<WIZDELETEDGUIDDATA>& arrayData);

    virtual void onTagGetList(const std::deque<WIZTAGDATA>& arrayRet);
    virtual void onTagPostList(const std::deque<WIZTAGDATA>& arrayData);

    virtual void onStyleGetList(const std::deque<WIZSTYLEDATA>& arrayRet);
    virtual void onStylePostList(const std::deque<WIZSTYLEDATA>& arrayData);

    virtual void onDocumentGetData(const WIZDOCUMENTDATAEX& data);
    virtual void onAttachmentGetList(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet);

    virtual void onDownloadObjectDataCompleted(const WIZOBJECTDATA& data);
    virtual void onUploadObjectDataCompleted(const WIZOBJECTDATA& data);

    // upload both document info (callDocumentPostData) and document data (callUploadDataPart)
    virtual bool uploadDocument(const WIZDOCUMENTDATAEX& data);
    virtual void onDocumentPostData(const WIZDOCUMENTDATAEX& data);
    virtual void onUploadDocument(const WIZDOCUMENTDATAEX& data) { Q_UNUSED(data); }

    // upload both attachment info (callAttachmentPostData) and document data (callUploadDataPart)
    virtual bool uploadAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data);
    virtual void onAttachmentPostData(const WIZDOCUMENTATTACHMENTDATAEX& data);
    virtual void onUploadAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data) { Q_UNUSED(data); }
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


#endif //WIZAPI_H
