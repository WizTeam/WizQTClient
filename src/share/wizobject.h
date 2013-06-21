#ifndef WIZOBJECT_H
#define WIZOBJECT_H

#include <QImage>
#include "wizqthelper.h"

class CWizXmlRpcStructValue;

struct WIZOBJECTBASE
{
    QString strKbGUID;
};

struct WIZUSERINFO
{
    WIZUSERINFO();
    bool LoadFromXmlRpc(CWizXmlRpcStructValue& val, const QString& kbGUID);
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


struct WIZUSERCERT
{
    WIZUSERCERT();
    bool LoadFromXmlRpc(CWizXmlRpcStructValue& val, const QString& kbGUID);

    QString strN;
    QString stre;
    QString strd;
    QString strHint;
};


struct WIZKBINFO
{
    WIZKBINFO();
    bool LoadFromXmlRpc(CWizXmlRpcStructValue& data, const QString& strKbGUID);

    __int64 nStorageLimit;
    __int64 nStorageUsage;
    QString strStorageLimit;
    QString strStorageUsage;

    __int64 nTrafficLimit;
    __int64 nTrafficUsage;
    QString strTrafficLimit;
    QString strTrafficUsage;
};


struct WIZOBJECTPARTDATA : public WIZOBJECTBASE
{
    WIZOBJECTPARTDATA();
    bool LoadFromXmlRpc(CWizXmlRpcStructValue& data, const QString& kbGUID);

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
    CString strDocumentGUID;
    CString strName;
    CString strValue;

    virtual BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data, const QString& kbGUID);
    virtual BOOL SaveToXmlRpc(CWizXmlRpcStructValue& data) const;
};


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
    __int64 nVersion;

    friend bool operator< (const WIZTAGDATA& data1, const WIZTAGDATA& data2) throw();

    BOOL EqualForSync(const WIZTAGDATA& data) const;
    virtual BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data, const QString& kbGUID);
    virtual BOOL SaveToXmlRpc(CWizXmlRpcStructValue& data) const;

    static CString VersionName() { return CString(_T("tag_version")); }
    static CString ObjectName() { return CString(_T("tag")); }
};

bool operator< ( const WIZTAGDATA& data1, const WIZTAGDATA& data2 ) throw();

struct WIZSTYLEDATA : public WIZOBJECTBASE
{
    WIZSTYLEDATA();

    CString strGUID;
    CString strName;
    CString strDescription;
    COLORREF crTextColor;
    COLORREF crBackColor;
    BOOL bTextBold;
    int nFlagIndex;
    COleDateTime tModified;
    __int64 nVersion;

    friend bool operator< (const WIZSTYLEDATA& data1, const WIZSTYLEDATA& data2 ) throw();

    bool EqualForSync(const WIZSTYLEDATA& data) const;
    virtual bool LoadFromXmlRpc(CWizXmlRpcStructValue& data, const QString& kbGUID);
    virtual bool SaveToXmlRpc(CWizXmlRpcStructValue& data) const;

    static CString VersionName() { return CString(_T("style_version")); }
    static CString ObjectName() { return CString(_T("style")); }
};

bool operator< (const WIZSTYLEDATA& data1, const WIZSTYLEDATA& data2 ) throw();


struct WIZDOCUMENTDATABASE : public WIZOBJECTBASE
{
    WIZDOCUMENTDATABASE();

    virtual BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data, const QString& kbGUID);
    static CString VersionName() { return CString("document_version"); }
    static CString ObjectName() { return CString("document"); }

    CString strGUID;
    CString strTitle;
    CString strLocation; // notebook
    COleDateTime tInfoModified;
    CString strInfoMD5;
    COleDateTime tDataModified;
    CString strDataMD5;
    COleDateTime tParamModified;
    CString strParamMD5;
    __int64 nVersion;

    int nObjectPart;
};


struct WIZDOCUMENTDATA : public WIZDOCUMENTDATABASE
{
    WIZDOCUMENTDATA();
    virtual ~WIZDOCUMENTDATA();

    bool EqualForSync(const WIZDOCUMENTDATA& data) const;

    CString strName;
    CString strSEO;
    CString strURL;
    CString strAuthor;
    CString strKeywords;
    CString strType;
    CString strOwner;
    CString strFileType;
    CString strStyleGUID;
    COleDateTime tCreated;
    COleDateTime tModified;
    COleDateTime tAccessed;
    long nIconIndex;
    long nSync;
    long nProtected;
    long nReadCount;
    long nAttachmentCount;
    long nIndexed;

    int nFlags;
    int nRate;
    CString strSystemTags;
    int nShareFlags;
};

struct WIZDOCUMENTDATAEX : public WIZDOCUMENTDATA
{
    WIZDOCUMENTDATAEX();
    WIZDOCUMENTDATAEX(const WIZDOCUMENTDATA& data);

    WIZDOCUMENTDATAEX& operator= (const WIZDOCUMENTDATAEX& right);
    bool ParamArrayToStringArray(CWizStdStringArray& params) const;
    virtual bool LoadFromXmlRpc(CWizXmlRpcStructValue& data, const QString& kbGUID);

    CWizStdStringArray arrayTagGUID;
    std::deque<WIZDOCUMENTPARAMDATA> arrayParam;
    QByteArray arrayData;

    bool bSkipped;
};


struct WIZDOCUMENTATTACHMENTDATA : public WIZOBJECTBASE
{
    WIZDOCUMENTATTACHMENTDATA();
    virtual ~WIZDOCUMENTATTACHMENTDATA();

    friend bool operator< (const WIZDOCUMENTATTACHMENTDATA& data1,const WIZDOCUMENTATTACHMENTDATA& data2 ) throw();
    BOOL EqualForSync(const WIZDOCUMENTATTACHMENTDATA& data) const;
    virtual BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data, const QString& kbGUID);

    static CString VersionName() { return CString(_T("attachment_version")); }
    static CString ObjectName() { return CString(_T("attachment")); }

    CString strKbGUID;
    CString strGUID;
    CString strDocumentGUID;
    CString strName;
    CString strURL;
    CString strDescription;
    COleDateTime tInfoModified;
    CString strInfoMD5;
    COleDateTime tDataModified;
    CString strDataMD5;
    __int64 nVersion;
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


struct WIZOBJECTDATA : public WIZOBJECTBASE
{
    WIZOBJECTDATA();
    WIZOBJECTDATA(const WIZOBJECTDATA& data);
    WIZOBJECTDATA(const WIZDOCUMENTDATA& data);
    WIZOBJECTDATA(const WIZDOCUMENTATTACHMENTDATA& data);

    static WizObjectType IntToObjectType(int n);
    static WizObjectType TypeStringToObjectType(const CString& strType);
    static CString ObjectTypeToTypeString(WizObjectType eType);

    COleDateTime tTime;
    CString strDisplayName;
    CString strObjectGUID;
    WizObjectType eObjectType;

    QByteArray arrayData;
};


struct WIZDELETEDGUIDDATA : public WIZOBJECTBASE
{
    WIZDELETEDGUIDDATA();

    CString strGUID;
    WizObjectType eType;
    COleDateTime tDeleted;
    __int64 nVersion;

    BOOL EqualForSync(const WIZDELETEDGUIDDATA& data) const;
    virtual BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data, const QString& kbGUID);
    virtual BOOL SaveToXmlRpc(CWizXmlRpcStructValue& data) const;

    static CString VersionName() { return CString(_T("deleted_guid_version")); }
    static CString ObjectName() { return CString(_T("deleted_guid")); }
};

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
    bool LoadFromXmlRpc(CWizXmlRpcStructValue& data, const QString& kbGUID);

    // kb_name, aka group name
    QString strGroupName;

    // kb_note, introduction text
    QString strGroupNote;

    // kb_guid
    QString strGroupGUID;

    // user_group, group permission
    int nUserGroup;

    // server_url, aka "ks"(knowledge server)
    QString strDatabaseServer;

    // not used
    QString strServerUrl;

    // dt_created
    COleDateTime tCreated;

    // dt_modified
    COleDateTime tModified;

    // user_name, not user id, but nick name, not used
    QString strUserName;

    // role_note, text description of permission, not used
    QString strRoleNote;

    // dt_role_created, not used
    COleDateTime tRoleCreated;

    // owner_name, default is null, not used
    QString strOwner;

    // kb_type, default is "group", not used
    QString strType;

    // kb_seo, the same as kb_name, not used
    QString strGroupSEO;

    // tag_names, default is null, not used
    QString strGroupTags;

    // kb_id, not used
    QString strId;
};

const UINT WIZ_USERGROUP_ADMIN = 0;
const UINT WIZ_USERGROUP_SUPER = 10;
const UINT WIZ_USERGROUP_EDITOR = 50;
const UINT WIZ_USERGROUP_AUTHOR = 100;
const UINT WIZ_USERGROUP_READER = 1000;
const UINT WIZ_USERGROUP_MAX = 10000000;

struct WIZMESSAGEDATA
{
    // xml-prc not support  long type, transmit use string
    qint64 nId;

    // char(38)
    QString bizGUID;    // wiz bussiness groups guid
    QString kbGUID;
    QString documentGUID;

    QString title;  // char(768)

    QString senderGUID; // char(38)
    QString senderId;   // char(128)
    QString senderAlias;    // char(32)

    QString receiverGUID;
    QString receiverId;
    QString receiverAlias;

    qint64 nVersion;

    // 0: @ message
    // 1: document edit meesage
    qint32 nMessageType;

    // 0: no email notify
    // 1: email notify sended
    // 2: email notify have not sended yet
    qint32 nEmailStatus;

    // 0: no sms notify
    // 1: sms notify sended
    // 2: sms notify have not sended yet
    qint32 nSMSStatus;

    // 0: not read
    // 1: read
    qint32 nReadStatus;

    COleDateTime tCreated;

    // char(1024), mssage body
    QString message;

    QString note;
};

enum WizMessageType
{

};


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



enum WIZTODOSTATE
{
    todoState0,
    todoState25,
    todoState50,
    todoState75,
    todoState100,
    todoStateMixed
};

enum WIZTODOPRIOR
{
    todoPriorNotUrgentAndNotImportant = -1,
    todoPriorNormal = 0,
    todoPriorImportant = 1,
    todoPriorUrgent = 2,
    todoPriorUrgentAndImportant = 3
};

struct WIZTODODATA
{
    CString strText;
    WIZTODOSTATE eState;
    WIZTODOPRIOR ePrior;
    CWizStdStringArray arrayLinkedDocumentGUID;
    BOOL bBlank;
    //
    COleDateTime tCreated;
    COleDateTime tModified;
    COleDateTime tCompleted;
    //
    __int64 nOrder;
    //
    WIZTODODATA();
    WIZTODODATA(const CString& str, WIZTODOSTATE state, WIZTODOPRIOR prior);
    //
    void InitTime();
    void AddLink(const CString& strDocumentGUID);
    CString GetLinkedDocumentGUIDString() const;
    void SetLinkedDocumentGUIDString(const CString& str);
};




typedef std::deque<WIZOBJECTDATA> CWizObjectDataArray;
typedef std::deque<WIZTAGDATA> CWizTagDataArray;
typedef std::deque<WIZSTYLEDATA> CWizStyleDataArray;
typedef std::deque<WIZDOCUMENTDATAEX> CWizDocumentDataArray;
typedef std::deque<WIZDOCUMENTPARAMDATA> CWizDocumentParamDataArray;
typedef std::deque<WIZDELETEDGUIDDATA> CWizDeletedGUIDDataArray;
typedef std::deque<WIZDOCUMENTATTACHMENTDATAEX> CWizDocumentAttachmentDataArray;
typedef std::deque<WIZMETADATA> CWizMetaDataArray;
typedef std::deque<WIZGROUPDATA> CWizGroupDataArray;
typedef std::deque<WIZABSTRACT> CWizAbstractArray;



struct WIZTODODATAEX : public WIZTODODATA
{
    typedef std::deque<WIZTODODATAEX> CWizTodoDataExArray;
    //
    CWizTodoDataExArray arrayChild;
    //
    WIZTODODATAEX();
    WIZTODODATAEX(const WIZTODODATA& data);
    //
public:
    int AddChild(const WIZTODODATAEX& data);
    //
    static BOOL WizTodoDataArrayFindLinkedDocument(const CWizTodoDataExArray& arrayData, const CString& strDocumentGUID);
    static int WizTodoDataArrayFindText(const CWizTodoDataExArray& arrayData, const CString& strText);
    static BOOL WizTodoDataItemCopyAndCombine(WIZTODODATAEX& itemDest, const WIZTODODATAEX& itemOther);
    static BOOL WizTodoDataArrayCombine(WIZTODODATAEX::CWizTodoDataExArray& arrayDest, const WIZTODODATAEX::CWizTodoDataExArray& arrayOther);
    static BOOL WizTodoDataArrayRemoveMultiItem(WIZTODODATAEX::CWizTodoDataExArray& arrayData);
    void AddCompletedDate(const CString& strTextExt);
};


COLORREF WizTodoGetTextColor(const WIZTODODATA& data);

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



const COLORREF WIZ_TODO_TEXT_COLOR_DEFAULT = RGB(0, 0, 0);
const COLORREF WIZ_TODO_TEXT_COLOR_DEFAULT_COMPLETED = RGB(0x66, 0x66, 0x66);

const COLORREF WIZ_TODO_TEXT_COLOR_URGENT_COMPLATED = RGB(0xFF, 0x64, 0x64);
const COLORREF WIZ_TODO_TEXT_COLOR_URGENTANDIMPORTANT_COMPLETED = RGB(0xFF, 0x64, 0x64);

const COLORREF WIZ_TODO_TEXT_COLOR_URGENT = RGB(255, 0, 0);
const COLORREF WIZ_TODO_TEXT_COLOR_URGENTANDIMPORTANTR = RGB(255, 0, 0);

const COLORREF WIZ_TODO_TEXT_COLOR_NOTURGENTANDNOTIMPORTANT = RGB(128, 128, 128);


#define WIZTODODATE_DOCUMENT_PARAM_NAME										_T("todolist_date")
#define WIZTODODATE_DOCUMENT_PARAM_NAME_COLLECTED_UNCOMPLETED_TASKS			_T("todolist_date_cut")
#define WIZTODODATE_DOCUMENT_PARAM_NAME_REMOVED_UNCOMPLETED_TASKS			_T("todolist_date_rut")


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


#endif // WIZOBJECT_H
