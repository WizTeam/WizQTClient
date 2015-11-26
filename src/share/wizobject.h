#ifndef WIZOBJECT_H
#define WIZOBJECT_H

#include <QMetaType>
#include <QImage>
#include <QMap>
#include "wizqthelper.h"

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
    WIZDATABASEINFO()
        : nPermission(WIZ_USERGROUP_MAX)
        , bOwner(false)
    {
    }
};


class CWizXmlRpcStructValue;

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
    int nMaxFileSize;

    WIZUSERINFOBASE()
        : nMaxFileSize(10 * 1024 * 1024)
    {
    }
    int GetMaxFileSize() const
    {
        return std::max<int>(20 * 1024 * 1024, nMaxFileSize);;
    }
};

struct WIZUSERINFO : public WIZUSERINFOBASE
{
    WIZUSERINFO();
    WIZUSERINFO(const WIZUSERINFO& info);
    virtual bool LoadFromXmlRpc(CWizXmlRpcStructValue& val);

    // field: api_version, default: 1

    // field: capi_url, default: /xmlrpc
    QString strChatUrl;

    // field: download_url, default: /a/download
    QString strDownloadUrl;

    // field: email_verify, default: verified

    // field: enable_group, default: 0
    int bEnableGroup;

    // field: expried_time
    COleDateTime tTokenExpried;

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
    COleDateTime tVipExpried;

    // field: sign up date
    COleDateTime tCreated;



    QString strBackupDatabaseServer;
};

Q_DECLARE_METATYPE(WIZUSERINFO)

//struct WIZKMUSERINFO : public WIZUSERINFO
//{
//};


struct WIZUSERCERT
{
    WIZUSERCERT();
    bool LoadFromXmlRpc(CWizXmlRpcStructValue& val);

    QString strN;
    QString stre;
    QString strd;
    QString strHint;
};


struct WIZKBINFO
{
    WIZKBINFO();
    bool LoadFromXmlRpc(CWizXmlRpcStructValue& data);

    qint64 nStorageLimit;
    qint64 nStorageUsage;
    QString strStorageLimit;
    QString strStorageUsage;

    qint64 nTrafficLimit;
    qint64 nTrafficUsage;
    QString strTrafficLimit;
    QString strTrafficUsage;
};


struct WIZOBJECTPARTDATA : public WIZOBJECTBASE
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



enum WizObjectType
{
    wizobjectError = -1,
    wizobjectTag = 1,
    wizobjectStyle,
    wizobjectDocumentAttachment,
    wizobjectDocument
};


struct WIZDOCUMENTPARAMDATA : public WIZOBJECTBASE
{
    virtual bool LoadFromXmlRpc(CWizXmlRpcStructValue& data);
    virtual bool SaveToXmlRpc(CWizXmlRpcStructValue& data) const;

    // helper field, indicate which document this object link to
    QString strDocumentGUID;

    // field: param_name, like: DOC_READ_COUNT
    QString strName;

    // field: param_value
    QString strValue;
};

typedef std::deque<WIZDOCUMENTPARAMDATA> CWizDocumentParamDataArray;


struct WIZTAGDATA : public WIZOBJECTBASE
{
    WIZTAGDATA();
    WIZTAGDATA(const WIZTAGDATA& data);
    virtual ~WIZTAGDATA();

    CString strGUID;
    CString strParentGUID;
    CString strName;
    CString strDescription;
    COleDateTime tModified;
    qint64 nVersion;
    qint64 nPostion;

    friend bool operator< (const WIZTAGDATA& data1, const WIZTAGDATA& data2) throw();

    BOOL EqualForSync(const WIZTAGDATA& data) const;
    virtual BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data);
    virtual BOOL SaveToXmlRpc(CWizXmlRpcStructValue& data) const;

    static CString VersionName() { return CString(_T("tag_version")); }
    static CString ObjectName() { return CString(_T("tag")); }
};

bool operator< ( const WIZTAGDATA& data1, const WIZTAGDATA& data2 ) throw();

typedef std::deque<WIZTAGDATA> CWizTagDataArray;


struct WIZSTYLEDATA : public WIZOBJECTBASE
{
    WIZSTYLEDATA();

    virtual bool LoadFromXmlRpc(CWizXmlRpcStructValue& data);
    virtual bool SaveToXmlRpc(CWizXmlRpcStructValue& data) const;

    bool EqualForSync(const WIZSTYLEDATA& data) const;
    static CString VersionName() { return CString(_T("style_version")); }
    static CString ObjectName() { return CString(_T("style")); }

    CString strGUID;
    CString strName;
    CString strDescription;
    COLORREF crTextColor;
    COLORREF crBackColor;
    bool bTextBold;
    int nFlagIndex;
    COleDateTime tModified;
    qint64 nVersion;

    friend bool operator< (const WIZSTYLEDATA& data1, const WIZSTYLEDATA& data2 ) throw();
};

bool operator< (const WIZSTYLEDATA& data1, const WIZSTYLEDATA& data2 ) throw();

typedef std::deque<WIZSTYLEDATA> CWizStyleDataArray;


struct WIZDOCUMENTDATABASE : public WIZOBJECTBASE
{
    WIZDOCUMENTDATABASE();

    virtual bool LoadFromXmlRpc(CWizXmlRpcStructValue& data);
    static QString VersionName() { return "document_version"; }
    static QString ObjectName() { return "document"; }

    // field: data_md5
    QString strDataMD5;

    // field: document_category, folder location
    QString strLocation;

    // field: document_guid
    QString strGUID;

    // field: document_title
    QString strTitle;

    // field: dt_data_modified
    COleDateTime tDataModified;

    // field: dt_info_modified
    COleDateTime tInfoModified;

    // field: dt_param_modified
    COleDateTime tParamModified;

    // field: info_md5
    QString strInfoMD5;

    // field: param_md5, default is empty
    QString strParamMD5;

    // field: version
    qint64 nVersion;  // -1: modified , 0: uploaded

    // helper filed
    // used to indicate which part of full info need download or downloaded
    int nObjectPart;
};

enum WizDocumentFlags
{
    wizDocumentAlwaysOnTop	= 0x1
};

struct WIZDOCUMENTDATA : public WIZDOCUMENTDATABASE
{
    WIZDOCUMENTDATA();
    virtual ~WIZDOCUMENTDATA();

    bool EqualForSync(const WIZDOCUMENTDATA& data) const;

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
    COleDateTime tCreated;

    // filed: dt_modfied
    COleDateTime tModified;

    // filed: dt_accessed
    COleDateTime tAccessed;

    // field: document_iconindex, default is 0, what's this?
    long nIconIndex;

    // field: document_protected, 1 protected, 0 none-protected
    long nProtected;

    // field: document_attachment_count
    long nAttachmentCount;

    // additional helper filed
    long nReadCount;
    long nIndexed;
    long nSync;
    int nFlags;
    int nRate;
    QString strSystemTags;
    int nShareFlags;
};

struct WIZDOCUMENTDATAEX : public WIZDOCUMENTDATA
{
    WIZDOCUMENTDATAEX();
    WIZDOCUMENTDATAEX(const WIZDOCUMENTDATA& data);

    WIZDOCUMENTDATAEX& operator= (const WIZDOCUMENTDATAEX& right);
    bool ParamArrayToStringArray(CWizStdStringArray& params) const;
    virtual bool LoadFromXmlRpc(CWizXmlRpcStructValue& data);

    // field: document_tags, guid list
    CWizStdStringArray arrayTagGUID;

    // field: document_params, default is empty, WIZDOCUMENTPARAMDATA object array.
    CWizDocumentParamDataArray arrayParam;

    QByteArray arrayData;

    bool bSkipped;
};

/*
////用于getList，获得简单信息////
*/
struct WIZDOCUMENTDATAEX_XMLRPC_SIMPLE : public WIZDOCUMENTDATAEX
{
    WIZDOCUMENTDATAEX_XMLRPC_SIMPLE()
    {
    }
    WIZDOCUMENTDATAEX_XMLRPC_SIMPLE(const WIZDOCUMENTDATAEX& data)
        : WIZDOCUMENTDATAEX(data)
    {
    }

    bool LoadFromXmlRpc(CWizXmlRpcStructValue& data);
};



struct WIZDOCUMENTATTACHMENTDATA : public WIZOBJECTBASE
{
    WIZDOCUMENTATTACHMENTDATA();
    virtual ~WIZDOCUMENTATTACHMENTDATA();

    friend bool operator< (const WIZDOCUMENTATTACHMENTDATA& data1,const WIZDOCUMENTATTACHMENTDATA& data2 ) throw();
    BOOL EqualForSync(const WIZDOCUMENTATTACHMENTDATA& data) const;
    virtual BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data);

    static QString VersionName() { return "attachment_version"; }
    static QString ObjectName() { return "attachment"; }

    QString strKbGUID;
    QString strGUID;
    QString strDocumentGUID;
    QString strName;
    QString strURL;
    QString strDescription;
    COleDateTime tInfoModified;
    QString strInfoMD5;
    COleDateTime tDataModified;
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

    static WizObjectType IntToObjectType(int n);
    static WizObjectType TypeStringToObjectType(const CString& strType);
    static QString ObjectTypeToTypeString(WizObjectType eType);

    COleDateTime tTime;
    CString strDisplayName;
    CString strObjectGUID;
    WizObjectType eObjectType;

    QByteArray arrayData;
};


struct WIZDELETEDGUIDDATA : public WIZOBJECTBASE
{
    WIZDELETEDGUIDDATA();

    virtual bool LoadFromXmlRpc(CWizXmlRpcStructValue& data);
    bool SaveToXmlRpc(CWizXmlRpcStructValue& data) const;
    bool EqualForSync(const WIZDELETEDGUIDDATA& data) const;
    static CString VersionName() { return CString(_T("deleted_guid_version")); }
    static CString ObjectName() { return CString(_T("deleted_guid")); }

    CString strGUID;
    WizObjectType eType;
    COleDateTime tDeleted;
    qint64 nVersion;
};

typedef std::deque<WIZDELETEDGUIDDATA> CWizDeletedGUIDDataArray;

struct WIZMETADATA : public WIZOBJECTBASE
{
    CString strName;
    CString strKey;
    CString strValue;
    COleDateTime tModified;
};


struct WIZGROUPDATA
{
    WIZGROUPDATA();
    WIZGROUPDATA(const WIZGROUPDATA& data);
    bool LoadFromXmlRpc(CWizXmlRpcStructValue& data);
    //
    bool IsBiz() const
    {
        return !bizGUID.isEmpty();
    }
    bool IsOwn() const
    {
        return bOwn;
    }

    // field: biz_guid, optional
    // Used for grouping groups
    QString bizGUID;

    // field: biz_name, optional
    QString bizName;

    // field: dt_created
    COleDateTime tCreated;

    // field: dt_modified
    COleDateTime tModified;

    // field: dt_role_created, not used
    COleDateTime tRoleCreated;

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
    bool LoadFromXmlRpc(CWizXmlRpcStructValue& data);

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
const int WIZ_USER_MSG_TYPE_SYSTEM = 100;
const int WIZ_USER_MSG_TYPE_MAX = 100;      //支持的最大消息类型，超过该类型的消息直接丢弃

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
    COleDateTime tCreated;
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

    bool LoadFromXmlRpc(CWizXmlRpcStructValue& data);
};

typedef std::deque<WIZUSERMESSAGEDATA> CWizUserMessageDataArray;



struct WIZMESSAGEDATA
{
    WIZMESSAGEDATA();
    WIZMESSAGEDATA(const WIZMESSAGEDATA& data);
    WIZMESSAGEDATA(const WIZUSERMESSAGEDATA& data);
    bool LoadFromXmlRpc(CWizXmlRpcStructValue& data);
    static QString ObjectName() { return "messages"; }

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
    COleDateTime tCreated;

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
    // not used currently
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
    bool LoadFromXmlRpc(CWizXmlRpcStructValue& data);

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


#define WIZDOCUMENT_SHARE_NONE			0
#define WIZDOCUMENT_SHARE_FRIENDS		1
#define WIZDOCUMENT_SHARE_PUBLIC		10


#define WIZKM_XMLRPC_OBJECT_PART_INFO		0x01
#define WIZKM_XMLRPC_OBJECT_PART_DATA		0x02
#define WIZKM_XMLRPC_OBJECT_PART_PARAM		0x04

#define WIZKM_XMKRPC_DOCUMENT_PART_INFO			WIZKM_XMLRPC_OBJECT_PART_INFO
#define WIZKM_XMKRPC_DOCUMENT_PART_DATA			WIZKM_XMLRPC_OBJECT_PART_DATA
#define WIZKM_XMKRPC_DOCUMENT_PART_PARAM		WIZKM_XMLRPC_OBJECT_PART_PARAM

#define WIZKM_XMKRPC_ATTACHMENT_PART_INFO		WIZKM_XMLRPC_OBJECT_PART_INFO
#define WIZKM_XMKRPC_ATTACHMENT_PART_DATA		WIZKM_XMLRPC_OBJECT_PART_DATA


const char* const TAG_NAME_PUBLIC				= "$public-documents$";
const char* const TAG_NAME_SHARE_WITH_FRIENDS	= "$share-with-friends$";


inline const CString TAG_DISPLAY_NAME_PUBLIC()
{
    return QObject::tr("$Public Notes");
}

inline const CString TAG_DISPLAY_NAME_SHARE_WITH_FRIENDS()
{
    return QObject::tr("$Share with friends");
}

#define LOCATION_DELETED_ITEMS      "/Deleted Items/"
#define LOCATION_DEFAULT            "/My Notes/"


#endif // WIZOBJECT_H
