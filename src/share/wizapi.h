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

#define WIZAPI_RETURN_SUCCESS "200"

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

// document

// get a list of WIZDOCUMENTDATABASE object of specified sync version
// return: data_md5, document_category, document_guid, document_title
// dt_data_modified, dt_info_modified, dt_param_modified, info_md5, param_md5
// version
const char* const SyncMethod_GetDocumentList = "document.getList";

// get document full info WIZDOCUMENTDATAEX object of specified document
// return: data_md5, document_attachment_count, document_author, document_category,
// document_data, document_filename, document_filetype, document_guid,
// document_iconindex, document_info, document_keywords, document_owner,
// document_param, document_params, document_protected, document_seo, document_share,
// document_styleguid, document_tags, document_title, document_type, document_url,
// dt_accessed, dt_created, dt_data_modified, dt_info_modified, dt_modified,
// dt_param_modified, dt_shared, info_md5, param_md5, public_tags, version
const char* const SyncMethod_GetDocumentFullInfo = "document.getData";

// get a list of WIZDOCUMENTDATABASE object of specified document guid
const char* const SyncMethod_GetDocumentsInfo = "document.downloadList";


// attachment

const char* const SyncMethod_GetAttachmentList = "attachment.getList";
const char* const SyncMethod_GetAttachmentsInfo = "attachment.downloadList";

const char* const SyncMethod_DownloadObjectPart = "data.download";

const char* const SyncMethod_PostDocumentData = "document.postData";
const char* const SyncMethod_PostAttachmentData = "attachment.postData";

const char* const SyncMethod_UploadObjectPart = "data.upload";

// messages

// args: token, biz_guid, kb_guid, document_guid, sender_guid, sender_id,
// message_type, read_status, page(default: 1), page_size(default: 100)
// version, count
// status: 301 token invalid, 322 args error, 500 server error, 200 ok
// return: message object, count, result?
const char* const SyncMethod_GetMessages = "accounts.getMessages";

// args: token, ids(id list, seperate by comma)， status(0 unread, 1 read)
// return: success_ids
const char* const SyncMethod_SetMessageStatus = "accounts.setReadStatus";

// key-value api

// This api is multi-usage api, set key-value for user private or group scope.
// used for syncing folder list, biz users and also user settings.

// This key is used for syncing biz users list, %s is biz_guid
#define WIZAPI_KV_KEY_USER_ALIAS "biz_users/%1"
#define WIZAPI_KV_KEY_FOLDERS   "folders"

enum WizApiKVType {
    KVTypeUserAlias,
    KVTypeFolders
};

// args: token, kb_guid(only used for kb.getValue), key
// status: 200 ok, 404 not exist
// return: value_of_key, version
const char* const SyncMethod_GetValue = "accounts.getValue";
const char* const SyncMethod_KbGetValue = "kb.getValue";

// args: totken, kb_guid(only used for kb.getValue), key
// status: 200 ok, 404 not exist
// return: version
const char* const SyncMethod_GetValueVersion = "accounts.getValueVersion";
const char* const SyncMethod_KbGetValueVersion = "kb.getValueVersion";

// args: token, kb_guid(only used for kb.getValue), key, value_of_key
// return: version
const char* const SyncMethod_SetValue = "accounts.setValue";
const char* const SyncMethod_KbSetValue = "kb.setValue";



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
    CWizApiBase(const QString& strKbUrl = WIZ_API_URL, QObject* parent = 0);

    // CWizXmlRpcServer passthrough method
    void resetProxy();
    virtual void abort() { m_server->abort(); }

    // kb url, used for sync document data.
    void setKbUrl(const QString& strKbUrl) { m_server->setKbUrl(strKbUrl); }

    QString kbGUID() const { return m_strKbGUID; }
    void setKbGUID(const QString& strKbGUID) { m_strKbGUID = strKbGUID; }

    virtual bool callXmlRpc(CWizXmlRpcValue* pVal, const QString& strMethodName,
                            const QString& arg1 = "", const QString& arg2 = "");

public:
    virtual bool callClientLogin(const QString& strUserId, const QString& strPassword);

    virtual bool callGetUserInfo();
    virtual bool callGetUserCert(const QString& strUserId, const QString& strPassword);
    virtual bool callGetGroupList();
    virtual bool callCreateAccount(const CString& strUserId, const CString& strPassword);

    virtual bool callGetBizUsers(const QString& bizGUID);
    virtual bool callGetMessages(qint64 nVersion);
    virtual bool callSetMessageStatus(const QList<qint64>& ids, bool bRead);

    // use key-value api to sync user folders
    virtual bool callFolderGetList();
    virtual bool callFolderGetVersion();
    virtual bool callFolderPostList(const CWizStdStringArray& arrayFolder);

    virtual bool callDeletedGetList(qint64 nVersion);
    virtual bool callDeletedPostList(const std::deque<WIZDELETEDGUIDDATA>& arrayData);

    virtual bool callTagGetList(qint64 nVersion);
    virtual bool callTagPostList(const std::deque<WIZTAGDATA>& arrayData);

    virtual bool callStyleGetList(qint64 nVersion);
    virtual bool callStylePostList(const std::deque<WIZSTYLEDATA>& arrayData);

    virtual bool callDocumentGetList(qint64 nVersion);
    virtual bool callDocumentGetInfo(const QString& documentGUID);
    virtual bool callDocumentsGetInfo(const CWizStdStringArray& arrayDocumentGUID);

    virtual bool callDocumentGetData(const QString& documentGUID);
    virtual bool callDocumentGetData(const WIZDOCUMENTDATABASE& data);
    virtual bool callDocumentPostData(const WIZDOCUMENTDATAEX& data);

    virtual bool callAttachmentGetList(qint64 nVersion);
    virtual bool callAttachmentGetInfo(const QString& attachmentGUID);
    virtual bool callAttachmentsGetInfo(const CWizStdStringArray& arrayAttachmentGUID);
    virtual bool callAttachmentPostData(const WIZDOCUMENTATTACHMENTDATAEX& data);

    virtual bool downloadObjectData(const WIZOBJECTDATA& data);
    virtual bool uploadObjectData(const WIZOBJECTDATA& data);

protected:
    virtual void onXmlRpcReturn(CWizXmlRpcValue& ret, const QString& strMethodName,
                                const QString& arg1, const QString& arg2);
    virtual void onXmlRpcError(const QString& strMethodName, WizXmlRpcError err, int errorCode, const QString& errorMessage);

    virtual void onClientLogin(const WIZUSERINFO& userInfo) { Q_UNUSED(userInfo); }

    virtual void onGetUserInfo(CWizXmlRpcValue& ret) { Q_UNUSED(ret); }
    virtual void onGetUserCert(const WIZUSERCERT& data) { Q_UNUSED(data); }
    virtual void onGetGroupList(const CWizGroupDataArray& arrayGroup) { Q_UNUSED(arrayGroup); }
    virtual void onCreateAccount() {}

    virtual void onGetMessages(const CWizMessageDataArray& messages) { Q_UNUSED(messages); }
    virtual void onSetMessageStatus() {}
    virtual void onGetBizUsers(const QString& strJsonUsers) { Q_UNUSED(strJsonUsers); }

    virtual void onGetFolders(const QString& strFolders) { Q_UNUSED(strFolders); }

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

    bool callGetList(const QString& strMethodName, qint64 nVersion);
    template <class TData> bool callPostList(const QString& strMethodName,
                                             const CString& strArrayName,
                                             const std::deque<TData>& arrayData);

private:
    CWizXmlRpcServer* m_server;
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

    // folders
    void folderGetVersionDone(qint64 nVersion);
    void folderGetListDone(const QStringList& listFolder, qint64 nVersion);
    void folderPostListDone(qint64 nVersion);

public Q_SLOTS:
    void xmlRpcReturn(CWizXmlRpcValue& ret, const QString& strMethodName,
                      const QString& arg1, const QString& arg2);
    void xmlRpcError(const QString& strMethodName, WizXmlRpcError err, \
                     int errorCode, const QString& errorMessage);

    void on_acquireApiEntry_finished(const QString& strReply);
};


/*
This class is used to do some default operations on database, subclass of
CWizApi(eg: CWizKbSync, CWizDownloadObjectData) should not operate database
directly, instead call CWizApi base implementations
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

    //virtual void onDeletedGetList(const std::deque<WIZDELETEDGUIDDATA>& arrayRet);
    //virtual void onDeletedPostList(const std::deque<WIZDELETEDGUIDDATA>& arrayData);

    //virtual void onTagGetList(const std::deque<WIZTAGDATA>& arrayRet);
    //virtual void onTagPostList(const std::deque<WIZTAGDATA>& arrayData);

    //virtual void onStyleGetList(const std::deque<WIZSTYLEDATA>& arrayRet);
    //virtual void onStylePostList(const std::deque<WIZSTYLEDATA>& arrayData);

    //virtual void onDocumentGetData(const WIZDOCUMENTDATAEX& data);
    //virtual void onAttachmentGetList(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet);

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
