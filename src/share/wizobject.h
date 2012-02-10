#ifndef WIZOBJECT_H
#define WIZOBJECT_H

#ifndef WIZMISC_H
#include "wizmisc.h"
#endif

class CWizXmlRpcStructValue;

class CWizSyncEvents
{
public:
    virtual void syncStarted() { }
    virtual void syncDone(bool error) { Q_UNUSED(error); }
    virtual void addLog(const CString& strMsg) { Q_UNUSED(strMsg); }
    virtual void addDebugLog(const CString& strMsg) { Q_UNUSED(strMsg); }
    virtual void addErrorLog(const CString& strMsg) { Q_UNUSED(strMsg); }
    virtual void changeProgress(int pos) { Q_UNUSED(pos); }
    virtual void changeObjectDataProgress(int pos) { Q_UNUSED(pos); }
};

enum WizObjectType
{
    wizobjectError = -1,
    wizobjectTag = 1,
    wizobjectStyle,
    wizobjectDocumentAttachment,
    wizobjectDocument,
};

struct WIZDOCUMENTDATA;
struct WIZDOCUMENTATTACHMENTDATA;


struct WIZOBJECTDATA
{
    COleDateTime tTime;
    CString strDisplayName;
    CString strObjectGUID;
    WizObjectType eObjectType;
    //
    QByteArray arrayData;
    //
    WIZOBJECTDATA();
    WIZOBJECTDATA(const WIZDOCUMENTDATA& data);
    WIZOBJECTDATA(const WIZDOCUMENTATTACHMENTDATA& data);
    //
    static WizObjectType IntToObjectType(int n);
    static WizObjectType TypeStringToObjectType(const CString& strType);
    static CString ObjectTypeToTypeString(WizObjectType eType);
};


struct WIZTAGDATA
{
    CString strGUID;
    CString strParentGUID;
    CString strName;
    CString strDescription;
    COleDateTime tModified;
    __int64 nVersion;
    //
    WIZTAGDATA();
    //
    friend bool operator< ( const WIZTAGDATA& data1, const WIZTAGDATA& data2 ) throw();
    //
    BOOL EqualForSync(const WIZTAGDATA& data) const;
    virtual BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data);
    virtual BOOL SaveToXmlRpc(CWizXmlRpcStructValue& data) const;
    //
    static CString VersionName() { return CString(_T("tag_version")); }
    static CString ObjectName() { return CString(_T("tag")); }
};

bool operator< ( const WIZTAGDATA& data1, const WIZTAGDATA& data2 ) throw();

struct WIZSTYLEDATA
{
    CString strGUID;
    CString strName;
    CString strDescription;
    COLORREF crTextColor;
    COLORREF crBackColor;
    BOOL bTextBold;
    int nFlagIndex;
    COleDateTime tModified;
    __int64 nVersion;
    //
    WIZSTYLEDATA();

    friend bool operator< (const WIZSTYLEDATA& data1, const WIZSTYLEDATA& data2 ) throw();
    //
    BOOL EqualForSync(const WIZSTYLEDATA& data) const;
    virtual BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data);
    virtual BOOL SaveToXmlRpc(CWizXmlRpcStructValue& data) const;
    //
    static CString VersionName() { return CString(_T("style_version")); }
    static CString ObjectName() { return CString(_T("style")); }
};

bool operator< (const WIZSTYLEDATA& data1, const WIZSTYLEDATA& data2 ) throw();



struct WIZDOCUMENTDATABASE
{
    CString strGUID;
    CString strTitle;
    CString strLocation;
    COleDateTime tInfoModified;
    CString strInfoMD5;
    COleDateTime tDataModified;
    CString strDataMD5;
    COleDateTime tParamModified;
    CString strParamMD5;
    __int64 nVersion;
    //
    WIZDOCUMENTDATABASE();
    virtual BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data);
    //
    static CString VersionName() { return CString(_T("document_version")); }
    static CString ObjectName() { return CString(_T("document")); }
    //
    int nObjectPart;
};


struct WIZDOCUMENTDATA : public WIZDOCUMENTDATABASE
{
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
    //
    int nFlags;
    int nRate;
    CString strSystemTags;
    int nShareFlags;
    //
    WIZDOCUMENTDATA();
    BOOL EqualForSync(const WIZDOCUMENTDATA& data) const;
};

struct WIZDOCUMENTPARAMDATA
{
    CString strDocumentGUID;
    CString strName;
    CString strValue;
    //
    virtual BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data);
    virtual BOOL SaveToXmlRpc(CWizXmlRpcStructValue& data) const;
};


struct WIZDELETEDGUIDDATA
{
    CString strGUID;
    WizObjectType eType;
    COleDateTime tDeleted;
    __int64 nVersion;
    //
    WIZDELETEDGUIDDATA();
    //
    BOOL EqualForSync(const WIZDELETEDGUIDDATA& data) const;
    virtual BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data);
    virtual BOOL SaveToXmlRpc(CWizXmlRpcStructValue& data) const;
    //
    static CString VersionName() { return CString(_T("deleted_guid_version")); }
    static CString ObjectName() { return CString(_T("deleted_guid")); }
};

struct WIZDOCUMENTATTACHMENTDATA
{
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
    //
    WIZDOCUMENTATTACHMENTDATA();
    //
    friend bool operator< (const WIZDOCUMENTATTACHMENTDATA& data1,const WIZDOCUMENTATTACHMENTDATA& data2 ) throw();

    BOOL EqualForSync(const WIZDOCUMENTATTACHMENTDATA& data) const;
    virtual BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data);
    //
    static CString VersionName() { return CString(_T("attachment_version")); }
    static CString ObjectName() { return CString(_T("attachment")); }
};

//
bool operator< (const WIZDOCUMENTATTACHMENTDATA& data1,const WIZDOCUMENTATTACHMENTDATA& data2 ) throw();


struct WIZMETADATA
{
    CString strName;
    CString strKey;
    CString strValue;
    COleDateTime tModified;
};


struct WIZDOCUMENTDATAEX : public WIZDOCUMENTDATA
{
    WIZDOCUMENTDATAEX();
    WIZDOCUMENTDATAEX(const WIZDOCUMENTDATA& data);
    //
    CWizStdStringArray arrayTagGUID;
    std::deque<WIZDOCUMENTPARAMDATA> arrayParam;
    QByteArray arrayData;
    //
    BOOL bSkipped;
    //
    WIZDOCUMENTDATAEX& operator = (const WIZDOCUMENTDATAEX& right);
    //
    BOOL ParamArrayToStringArray(CWizStdStringArray& params) const;
    virtual BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data);
};

struct WIZDOCUMENTATTACHMENTDATAEX : public WIZDOCUMENTATTACHMENTDATA
{
    WIZDOCUMENTATTACHMENTDATAEX();
    WIZDOCUMENTATTACHMENTDATAEX(const WIZDOCUMENTATTACHMENTDATA& data);
    //
    WIZDOCUMENTATTACHMENTDATAEX& operator = (const WIZDOCUMENTATTACHMENTDATAEX& right);
    //
    QByteArray arrayData;
    //
    BOOL bSkipped;
    //
    int nObjectPart;
};




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




struct WIZGROUPDATA
{
    CString strGroupName;
    CString strGroupGUID;
    CString strGroupTags;
    CString strGroupSEO;
    CString strGroupURL;
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
    intptr_t AddChild(const WIZTODODATAEX& data);
    //
    static BOOL WizTodoDataArrayFindLinkedDocument(const CWizTodoDataExArray& arrayData, const CString& strDocumentGUID);
    static intptr_t WizTodoDataArrayFindText(const CWizTodoDataExArray& arrayData, const CString& strText);
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

#define LOCATION_DELETED_ITEMS      "/Deleted Items/"


#endif // WIZOBJECT_H
