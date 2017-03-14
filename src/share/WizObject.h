#ifndef WIZOBJECT_H
#define WIZOBJECT_H

#include <QMetaType>
#include <QImage>
#include <QMap>
#include "WizQtHelper.h"

const UINT WIZ_USERGROUP_ADMIN = 0;
const UINT WIZ_USERGROUP_SUPER = 10;
const UINT WIZ_USERGROUP_EDITOR = 50;
const UINT WIZ_USERGROUP_AUTHOR = 100;
const UINT WIZ_USERGROUP_READER = 1000;
const UINT WIZ_USERGROUP_MAX = 10000000;

struct WIZDATABASEINFO
{
    // optional
    QString bizName;
    QString bizGUID;

    // required, private db set to "PRIVATE"
    QString name;

    // required
    //QString kbGUID;

    // required, used for syncing object data, aka kapi_url
    //QString serverUrl;

    // required, private db set to 0
    int nPermission;
    //
    bool bOwner;
    //
    bool bEncryptData;
    //
    WIZDATABASEINFO()
        : nPermission(WIZ_USERGROUP_MAX)
        , bOwner(false)
        , bEncryptData(false)
    {
    }
};


class WizXmlRpcStructValue;

struct WIZOBJECTBASE
{
    QString strKbGUID;
};

// this struct is obtained from client login api

struct WIZUSERINFOBASE
{
    QString strToken;
    QString strKbGUID;
    QString strDatabaseServer;
    QString strKbServer;

    //NOTE: DEPRECATED
    int nMaxFileSize;

    WIZUSERINFOBASE()
        : nMaxFileSize(10 * 1024 * 1024)
    {
    }    
};

struct WIZUSERINFO : public WIZUSERINFOBASE
{
    WIZUSERINFO();
    WIZUSERINFO(const WIZUSERINFO& info);
    virtual bool loadFromXmlRpc(WizXmlRpcStructValue& val);

    // field: api_version, default: 1

    // field: capi_url, default: /xmlrpc
    QString strChatUrl;

    // field: download_url, default: /a/download
    QString strDownloadUrl;

    // field: email_verify, default: verified

    // field: enable_group, default: 0
    int bEnableGroup;

    // field: expried_time
    WizOleDateTime tTokenExpried;

    // field: invite_code, current is 8 length char
    QString strInviteCode;

    // field: mywiz_email
    QString strMywizEmail;

    // field: notice_link, currently null
    QString strNoticeLink;

    // field: notice_text, currently null
    QString strNoticeText;

    // field: public_tags, default: config.public_tags, what's this?
    // field: push_tags, default: config.push_tags, what's this?
    // field: return_code, default: 200
    // field: return_message, default: successfuly login, hello! user!
    // field: server, currently null
    // field: sns_list, default: sina=zh_CN name, 1
    QString strSNSList;

    // field: upload_url, default: /a/upload
    QString strUploadUrl;

    // ---> user struct begin
    // field: api_version, why return two times of this field?

    // field: displayname
    QString strDisplayName;

    // field: email, account id
    QString strUserEmail;

    // field: language, default: zh_CN
    QString strLanguage;

    // field: nickname
    QString strNickName;

    // field: return_code, why return two times of this field?

    // field: user_guid
    QString strUserGUID;

    // <--- user struct end

    // field: user_level
    int nUserLevel;

    // field: user_level_name
    QString strUserLevelName;

    // field: user_photo_url, currently null

    // field: user_points
    int nUserPoints;

    // field: user_type, default: vip or free
    QString strUserType;

    // field: vip_date
    WizOleDateTime tVipExpried;

    // field: sign up date
    WizOleDateTime tCreated;

    //
    int syncType;

    QString strBackupDatabaseServer;
};

Q_DECLARE_METATYPE(WIZUSERINFO)

//struct WIZKMUSERINFO : public WIZUSERINFO
//{
//};


struct WIZUSERCERT
{
    WIZUSERCERT();
    bool loadFromXmlRpc(WizXmlRpcStructValue& val);

    QString strN;
    QString stre;
    QString strd;
    QString strHint;
};


struct WIZKBINFO
{
    WIZKBINFO();
    bool loadFromXmlRpc(WizXmlRpcStructValue& data);
    //
    int getMaxFileSize() const
    {
        return nUploadSizeLimit;
    }

    qint64 nStorageLimit;
    qint64 nStorageUsage;
    QString strStorageLimit;
    QString strStorageUsage;

    qint64 nUploadSizeLimit;
    QString strUploadSizeLimitString;
    qint64 nNotesCount;
    qint64 nNotesCountLimit;

    qint64 nTrafficLimit;
    qint64 nTrafficUsage;
    QString strTrafficLimit;
    QString strTrafficUsage;
};


struct WIZOBJECTPARTDATA : public WIZOBJECTBASE
{
    WIZOBJECTPARTDATA();
    bool loadFromXmlRpc(WizXmlRpcStructValue& data);

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



enum WizObjectType
{
    wizobjectError = -1,
    wizobjectTag = 1,
    wizobjectStyle,
    wizobjectDocumentAttachment,
    wizobjectDocument
};




struct WIZTAGDATA : public WIZOBJECTBASE
{
    explicit WIZTAGDATA();
    WIZTAGDATA(const WIZTAGDATA& data);
    virtual ~WIZTAGDATA();

    CString strGUID;
    CString strParentGUID;
    CString strName;
    CString strDescription;
    WizOleDateTime tModified;
    qint64 nVersion;
    qint64 nPostion;

    friend bool operator< (const WIZTAGDATA& data1, const WIZTAGDATA& data2) throw();

    BOOL equalForSync(const WIZTAGDATA& data) const;
    virtual BOOL loadFromXmlRpc(WizXmlRpcStructValue& data);
    virtual BOOL saveToXmlRpc(WizXmlRpcStructValue& data) const;

    static CString versionName() { return CString("tag_version"); }
    static CString objectName() { return CString("tag"); }
};

bool operator< ( const WIZTAGDATA& data1, const WIZTAGDATA& data2 ) throw();

typedef std::deque<WIZTAGDATA> CWizTagDataArray;


struct WIZSTYLEDATA : public WIZOBJECTBASE
{
    WIZSTYLEDATA();

    virtual bool loadFromXmlRpc(WizXmlRpcStructValue& data);
    virtual bool saveToXmlRpc(WizXmlRpcStructValue& data) const;

    bool equalForSync(const WIZSTYLEDATA& data) const;
    static CString versionName() { return CString("style_version"); }
    static CString objectName() { return CString("style"); }

    CString strGUID;
    CString strName;
    CString strDescription;
    COLORREF crTextColor;
    COLORREF crBackColor;
    bool bTextBold;
    int nFlagIndex;
    WizOleDateTime tModified;
    qint64 nVersion;

    friend bool operator< (const WIZSTYLEDATA& data1, const WIZSTYLEDATA& data2 ) throw();
};

bool operator< (const WIZSTYLEDATA& data1, const WIZSTYLEDATA& data2 ) throw();

typedef std::deque<WIZSTYLEDATA> CWizStyleDataArray;

enum WizDocumentFlags
{
    wizDocumentAlwaysOnTop	= 0x1
};

struct WIZDOCUMENTDATA : public WIZOBJECTBASE
{
    WIZDOCUMENTDATA();
    virtual ~WIZDOCUMENTDATA();
    //
    bool equalForSync(const WIZDOCUMENTDATA& data) const;
    //
    static QString versionName() { return "document_version"; }
    static QString objectName() { return "document"; }

    // field: data_md5
    QString strDataMD5;

    // field: document_category, folder location
    QString strLocation;

    // field: document_guid
    QString strGUID;

    // field: document_title
    QString strTitle;

    // field: dt_data_modified
    WizOleDateTime tDataModified;

    // field: version
    qint64 nVersion;  // -1: modified , 0: uploaded

    // field: document_filename, default: "guid + .ziw"
    QString strName;

    // field: document_seo, default is empty
    QString strSEO;

    // filed: document_url, default is empty
    QString strURL;

    // field: document_author, default is empty
    QString strAuthor;

    // field: document_keywords, default is empty
    QString strKeywords;

    // field: document_type, default: "document"
    QString strType;

    // field: document_owner, default: creator's user id
    QString strOwner;

    // field: document_filetype, default is empty
    QString strFileType;

    // field: document_styleguid, default is empty
    QString strStyleGUID;

    // field: dt_created
    WizOleDateTime tCreated;

    // filed: dt_modfied
    WizOleDateTime tModified;

    // filed: dt_accessed
    WizOleDateTime tAccessed;

    // field: document_protected, 1 protected, 0 none-protected
    long nProtected;

    // field: document_attachment_count
    long nAttachmentCount;

    // additional helper filed
    long nReadCount;
    //
    long nIndexed;

    // field: local info modified
    long nInfoChanged;

    // field: local data modified
    long nDataChanged;
};

struct WIZDOCUMENTDATAEX : public WIZDOCUMENTDATA
{
    WIZDOCUMENTDATAEX();
    WIZDOCUMENTDATAEX(const WIZDOCUMENTDATA& data);

    WIZDOCUMENTDATAEX& operator= (const WIZDOCUMENTDATAEX& right);
    virtual bool loadFromXmlRpc(WizXmlRpcStructValue& data);

    // field: document_tags, guid list
    CWizStdStringArray arrayTagGUID;
    //
    QByteArray arrayData;
    //
    QString strHighlightTitle;
    QString strHighlightText;

    bool bSkipped;
};


struct WIZDOCUMENTATTACHMENTDATA : public WIZOBJECTBASE
{
    WIZDOCUMENTATTACHMENTDATA();
    virtual ~WIZDOCUMENTATTACHMENTDATA();

    friend bool operator< (const WIZDOCUMENTATTACHMENTDATA& data1,const WIZDOCUMENTATTACHMENTDATA& data2 ) throw();
    BOOL equalForSync(const WIZDOCUMENTATTACHMENTDATA& data) const;
    virtual BOOL loadFromXmlRpc(WizXmlRpcStructValue& data);

    static QString versionName() { return "attachment_version"; }
    static QString objectName() { return "attachment"; }

    QString strGUID;
    QString strDocumentGUID;
    QString strName;
    QString strURL;
    QString strDescription;
    WizOleDateTime tInfoModified;
    QString strInfoMD5;
    WizOleDateTime tDataModified;
    QString strDataMD5;
    qint64 nVersion;
};

bool operator< (const WIZDOCUMENTATTACHMENTDATA& data1,const WIZDOCUMENTATTACHMENTDATA& data2 ) throw();

struct WIZDOCUMENTATTACHMENTDATAEX : public WIZDOCUMENTATTACHMENTDATA
{
    WIZDOCUMENTATTACHMENTDATAEX();
    WIZDOCUMENTATTACHMENTDATAEX(const WIZDOCUMENTATTACHMENTDATA& data);

    WIZDOCUMENTATTACHMENTDATAEX& operator= (const WIZDOCUMENTATTACHMENTDATAEX& right);

    QByteArray arrayData;
    BOOL bSkipped;
    int nObjectPart;
};

typedef std::deque<WIZDOCUMENTATTACHMENTDATAEX> CWizDocumentAttachmentDataArray;


struct WIZOBJECTDATA : public WIZOBJECTBASE
{
    WIZOBJECTDATA();
    WIZOBJECTDATA(const WIZOBJECTDATA& data);
    WIZOBJECTDATA(const WIZDOCUMENTDATA& data);
    WIZOBJECTDATA(const WIZDOCUMENTATTACHMENTDATA& data);

    static WizObjectType intToObjectType(int n);
    static WizObjectType typeStringToObjectType(const CString& strType);
    static QString objectTypeToTypeString(WizObjectType eType);

    WizOleDateTime tTime;
    CString strDisplayName;
    CString strDocumentGuid;
    CString strObjectGUID;
    WizObjectType eObjectType;

    QByteArray arrayData;
};


struct WIZDELETEDGUIDDATA : public WIZOBJECTBASE
{
    WIZDELETEDGUIDDATA();

    virtual bool loadFromXmlRpc(WizXmlRpcStructValue& data);
    bool saveToXmlRpc(WizXmlRpcStructValue& data) const;
    bool equalForSync(const WIZDELETEDGUIDDATA& data) const;
    static CString versionName() { return CString("deleted_guid_version"); }
    static CString objectName() { return CString("deleted_guid"); }

    CString strGUID;
    WizObjectType eType;
    WizOleDateTime tDeleted;
    qint64 nVersion;
};

typedef std::deque<WIZDELETEDGUIDDATA> CWizDeletedGUIDDataArray;

struct WIZMETADATA : public WIZOBJECTBASE
{
    CString strName;
    CString strKey;
    CString strValue;
    WizOleDateTime tModified;
};


struct WIZGROUPDATA
{
    WIZGROUPDATA();
    WIZGROUPDATA(const WIZGROUPDATA& data);
    bool loadFromXmlRpc(WizXmlRpcStructValue& data);
    //
    bool isBiz() const
    {
        return !bizGUID.isEmpty();
    }
    bool isOwn() const
    {
        return bOwn;
    }

    // field: biz_guid, optional
    // Used for grouping groups
    QString bizGUID;

    // field: biz_name, optional
    QString bizName;

    // field: dt_created
    WizOleDateTime tCreated;

    // field: dt_modified
    WizOleDateTime tModified;

    // field: dt_role_created, not used
    WizOleDateTime tRoleCreated;

    // field: kapi_url
    // server_url, aka "ks"(knowledge server)
    QString strDatabaseServer;

    // field: kb_guid
    QString strGroupGUID;

    // field: kb_id, not used
    QString strId;

    // field: kb_name, aka group name
    QString strGroupName;

    // field: kb_note, introduction text
    QString strGroupNote;

    // field: kb_seo, the same as kb_name, not used
    QString strGroupSEO;

    // field: kb_type, default is "group", not used
    QString strType;
    // field: mywiz_email, group mywiz_email
    QString strMyWiz;

    // field: owner_name, default is null, not used
    QString strOwner;

    // field: role_note
    // text description of permission, not used
    QString strRoleNote;

    // filed: server_url, not used
    QString strServerUrl;

    // field: tag_names
    // default "tags names api change", obsolete field now
    QString strGroupTags;

    // field: user_group, group permission
    int nUserGroup;

    // field: user_name
    // not user id, but nick name, not used
    QString strUserName;
    //
    bool bOwn;
    //
    bool bEncryptData;
};



const UINT WIZ_BIZROLE_OWNER			= 0;
const UINT WIZ_BIZROLE_ADMIN			= 10;
const UINT WIZ_BIZROLE_HR				= 100;
const UINT WIZ_BIZROLE_NORMAL			= 1000;
const UINT WIZ_BIZROLE_GUEST			= 10000;
const UINT WIZ_BIZROLE_MAX				= 10000000;

struct WIZBIZDATA
{
    WIZBIZDATA();
    WIZBIZDATA(const WIZBIZDATA& data);
    bool loadFromXmlRpc(WizXmlRpcStructValue& data);

    QString bizGUID;
    QString bizName;
    int bizUserRole;
    int bizLevel;
    bool bizIsDue;
    std::map<QString, QString> mapAvatarChanges;
};


const int WIZ_USER_MSG_TYPE_CALLED_IN_TITLE = 0;
const int WIZ_USER_MSG_TYPE_MODIFIED = 1;
const int WIZ_USER_MSG_TYPE_COMMENT = 10;
const int WIZ_USER_MSG_TYPE_CALLED_IN_COMMENT = 20;
const int WIZ_USER_MSG_TYPE_COMMENT_REPLY = 30;
const int WIZ_USER_MSG_TYPE_REQUEST_JOIN_GROUP = 40;
const int WIZ_USER_MSG_TYPE_ADDED_TO_GROUP = 50;
const int WIZ_USER_MSG_TYPE_LIKE = 60;
const int WIZ_USER_MSG_TYPE_REMIND = 90;
const int WIZ_USER_MSG_TYPE_SYSTEM = 100;
const int WIZ_USER_MSG_TYPE_REMIND_CREATE = 110;
const int WIZ_USER_MSG_TYPE_MAX = 110;      //支持的最大消息类型，超过该类型的消息直接丢弃

struct WIZUSERMESSAGEDATA
{
    qint64 nMessageID;
    QString strBizGUID;
    QString strKbGUID;
    QString strDocumentGUID;
    QString strSenderGUID;
    QString strSenderID;
    QString strReceiverGUID;
    QString strReceiverID;
    int nMessageType;
    int nReadStatus;	//阅读状态, 0:未读，1:已读
    int nDeletedStatus;  // 删除状态， 0：为删除， 1：已删除
    WizOleDateTime tCreated;
    QString strMessageText;
    qint64 nVersion;
    QString strSender;
    QString strReceiver;
    QString strTitle;
    QString strNote;     //消息携带的数据，用于显示广告等内容
    int nLocalChanged;

    WIZUSERMESSAGEDATA()
        : nMessageID(0)
        , nMessageType(WIZ_USER_MSG_TYPE_CALLED_IN_TITLE)
        , nReadStatus(0)
        , nDeletedStatus(0)
        , nVersion(0)
        , nLocalChanged(0)
    {

    }

    bool loadFromXmlRpc(WizXmlRpcStructValue& data);
};

typedef std::deque<WIZUSERMESSAGEDATA> CWizUserMessageDataArray;



struct WIZMESSAGEDATA
{
    WIZMESSAGEDATA();
    WIZMESSAGEDATA(const WIZMESSAGEDATA& data);
    WIZMESSAGEDATA(const WIZUSERMESSAGEDATA& data);
    bool loadFromXmlRpc(WizXmlRpcStructValue& data);
    static QString objectName() { return "messages"; }

    bool isAd();

    enum LocalChanged{
        localChanged_None = 0x0000,
        localChanged_Read = 0x0001,
        localChanged_Delete = 0x0002
    };

    // Field: biz_guid, char(36)
    // wiz bussiness groups guid
    QString bizGUID;

    // Field: kb_guid, char(36)
    QString kbGUID;

    // Field: document_guid, char(36)
    // indicate refered document, aware maybe this document already deleted
    QString documentGUID;

    // Field: dt_created
    WizOleDateTime tCreated;

    // Field: email_status
    // 0: no email notify
    // 1: email notify sended
    // 2: email notify have not sended yet
    qint32 nEmailStatus;

    // Filed: id
    // xml-prc not support long type, transmit use string
    // used to set read status
    qint64 nId;

    // Field: message_body, char(1024)
    QString messageBody;

    // Field: message_type
    // 0: @ message in title
    // 1: document edited meesage
    // 10: comment
    // 30: @ message in comment
    qint32 nMessageType;

    // Filed: note
    QString note;

    // Field: read_status
    // 0: not read
    // 1: read
    qint32 nReadStatus;

    //Field:DELETE_STATUS
    //0: not deleted
    //1:deleted
    qint32 nDeleteStatus;

    // Field: receiver_alias
    QString receiverAlias;

    // Field: receiver_guid, char(36)
    QString receiverGUID;

    // Field: receiver_id
    // account id
    QString receiverId;

    // Field: sender_alias, char(32)
    // this field may not exist
    QString senderAlias;

    // Field: sender_guid, char(36)
    QString senderGUID;

    // Field: sender_id, char(128)
    // sender's account id
    QString senderId;

    // Field: sms_status
    // 0: no sms notify
    // 1: sms notify sended
    // 2: sms notify have not sended yet
    qint32 nSMSStatus;

    // Field: title, char(768)
    // document title
    QString title;

    // Field: version
    qint64 nVersion;

    // Field: localchanged  should not upload. use value of  WIZMESSAGEDATA::LocalChanged
    int nLocalChanged;
};

typedef std::deque<WIZMESSAGEDATA> CWizMessageDataArray;

// struct return from accounts.getValue method
struct WIZKVRETURN
{
    bool loadFromXmlRpc(WizXmlRpcStructValue& data);

    // field: return_code
    // 200 ok
    int nCode;

    // field: value_of_key
    QString value;

    // field: version
    qint64 nVersion;
};

// this struct is parsed from json document by xml-rpc kv api return field value
struct WIZBIZUSER
{
    // field: alias
    QString alias;

    // field: pinyin
    QString pinyin;

    // field: user_guid
    QString userGUID;

    // field: user_id, email account name
    QString userId;

    // no field, indicate user biz group
    QString bizGUID;
};

typedef std::deque<WIZBIZUSER> CWizBizUserDataArray;


struct WIZABSTRACT : public WIZOBJECTBASE
{
    CString guid;
    CString text;
    QImage image;
};

struct WIZSEARCHDATA
{
    CWizStdStringArray arrayTag;
    CWizStdStringArray arrayFileType;
    CString strTitle;
    CString strURL;
    int nHasAttachment;
    CString strAttachmentName;
    CString strSQL;
    CString strSyntax;
    CString strDateCreated;
    CString strDateModified;
    CString strDateAccessed;

    WIZSEARCHDATA()
        : nHasAttachment(-1)
    {
    }
};


struct WIZDOCUMENTLOCATIONDATA
{
    CString strLocation;
    int nDocumentCount;

    WIZDOCUMENTLOCATIONDATA()
    {
        nDocumentCount = 0;
    }
};


typedef std::deque<WIZDOCUMENTLOCATIONDATA> CDocumentLocationArray;



typedef std::deque<WIZOBJECTDATA> CWizObjectDataArray;
typedef std::deque<WIZDOCUMENTDATAEX> CWizDocumentDataArray;
typedef std::deque<WIZMETADATA> CWizMetaDataArray;
typedef std::deque<WIZGROUPDATA> CWizGroupDataArray;
typedef std::deque<WIZABSTRACT> CWizAbstractArray;
typedef std::deque<WIZBIZDATA> CWizBizDataArray;


template <class TData>
__int64 WizObjectsGetMaxVersion(const std::deque<TData>& arrayData)
{
    if (arrayData.empty())
        return -1;
    //
    __int64 nVersion = 0;
    for (typename std::deque<TData>::const_iterator it = arrayData.begin();
        it != arrayData.end();
        it++)
    {
        nVersion = std::max<__int64>(nVersion, it->nVersion);
    }
    //
    return nVersion;
}

#define LOCATION_DELETED_ITEMS      "/Deleted Items/"
#define LOCATION_DEFAULT            "/My Notes/"

enum WizDocumentViewMode
{
    viewmodeAlwaysEditing,
    viewmodeAlwaysReading,
    viewmodeKeep
};

enum WizEditorMode
{
    modeEditor,
    modeReader
};


#endif // WIZOBJECT_H
