#include "wizobject.h"

#include "wizxmlrpc.h"
#include "wizmisc.h"
#include "utils/logger.h"
#include "rapidjson/document.h"

WIZUSERINFO::WIZUSERINFO()
    : nUserLevel(0)
    , nUserPoints(0)
    , bEnableGroup(false)
{
}

WIZUSERINFO::WIZUSERINFO(const WIZUSERINFO& info)
{
    strToken = info.strToken;
    strKbGUID = info.strKbGUID;
    strDatabaseServer = info.strDatabaseServer;
    nMaxFileSize = info.nMaxFileSize;
    strChatUrl = info.strChatUrl;
    strDownloadUrl = info.strDownloadUrl;
    bEnableGroup = info.bEnableGroup;
    //tTokenExpried = info.tTokenExpried;
    strInviteCode = info.strInviteCode;
    strMywizEmail = info.strMywizEmail;
    strNoticeLink = info.strNoticeLink;
    strNoticeText = info.strNoticeText;
    strSNSList = info.strSNSList;
    strUploadUrl = info.strUploadUrl;
    strDisplayName = info.strDisplayName;
    strUserEmail = info.strUserEmail;
    strLanguage = info.strLanguage;
    strNickName = info.strNickName;
    strUserGUID = info.strUserGUID;
    nUserLevel = info.nUserLevel;
    strUserLevelName = info.strUserLevelName;
    nUserPoints = info.nUserPoints;
    strUserType = info.strUserType;
    tVipExpried = info.tVipExpried;
    tCreated = info.tCreated;
}


bool WIZUSERINFO::LoadFromXmlRpc(CWizXmlRpcStructValue& val)
{
    CWizXmlRpcStructValue& data = val;    
    data.GetString("token", strToken);
    data.GetTime("expried_time", tTokenExpried);

    data.GetStr("download_url", strDownloadUrl);
    data.GetStr("upload_url", strUploadUrl);
    data.GetStr("capi_url", strChatUrl);
    data.GetInt("enable_group", bEnableGroup);

    data.GetString("kapi_url", strDatabaseServer);
    data.GetString("kb_guid", strKbGUID);
    data.GetString("invite_code", strInviteCode);
    data.GetString("mywiz_email", strMywizEmail);
    data.GetInt("upload_size_limit", nMaxFileSize);

    data.GetString("notice_link", strNoticeLink);
    data.GetString("notice_text", strNoticeText);
    data.GetString("sns_list", strSNSList);

    if (CWizXmlRpcStructValue* pUser = data.GetStruct("user"))
    {
        pUser->GetString("displayname", strDisplayName);
        pUser->GetString("email", strUserEmail);
        pUser->GetString("language", strLanguage);
        pUser->GetString("nickname", strNickName);
        pUser->GetString("user_guid", strUserGUID);
        pUser->GetTime("dt_created", tCreated);
    }

    data.GetInt("user_level", nUserLevel);
    data.GetString("user_level_name", strUserLevelName);
    data.GetInt("user_points", nUserPoints);
    data.GetString("user_type", strUserType);
    data.GetTime("vip_date", tVipExpried);

    return !strToken.isEmpty()
            && !strKbGUID.isEmpty()
            && !strUserGUID.isEmpty()
            && !strDatabaseServer.isEmpty();
}

WIZUSERCERT::WIZUSERCERT()
{
}

bool WIZUSERCERT::LoadFromXmlRpc(CWizXmlRpcStructValue& val)
{
    CWizXmlRpcStructValue& data = val;
    data.GetStr("n", strN);
    data.GetStr("e", stre);
    data.GetStr("d", strd);
    data.GetStr("hint", strHint);

    return true;
}


WIZKBINFO::WIZKBINFO()
{
    nStorageLimit = 0;
    nStorageUsage = 0;
    nTrafficLimit = 0;
    nTrafficUsage = 0;
}

bool WIZKBINFO::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    data.GetInt64(_T("storage_limit"), nStorageLimit);
    data.GetInt64(_T("storage_usage"), nStorageUsage);
    data.GetStr(_T("storage_limit_string"), strStorageLimit);
    data.GetStr(_T("storage_usage_string"), strStorageUsage);
    data.GetInt64(_T("traffic_limit"), nTrafficLimit);
    data.GetInt64(_T("traffic_usage"), nTrafficUsage);
    data.GetStr(_T("traffic_limit_string"), strTrafficLimit);
    data.GetStr(_T("traffic_usage_string"), strTrafficUsage);

    return true;
}

/* -------------------------- WIZOBJECTPARTDATA -------------------------- */
WIZOBJECTPARTDATA::WIZOBJECTPARTDATA()
    : nStartPos(0)
    , nQuerySize(0)
    , nObjectSize(0)
    , bEOF(false)
    , nPartSize(0)
{
}

bool WIZOBJECTPARTDATA::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    data.GetInt("eof", bEOF);
    data.GetInt64("obj_size", nObjectSize);
    data.GetString("part_md5", strPartMD5);
    data.GetInt64("part_size", nPartSize);

    if (!data.GetStream("data", arrayData)){
        TOLOG("Fault error, data is null!");
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////


WIZOBJECTDATA::WIZOBJECTDATA()
    : eObjectType(wizobjectError)
{

}

WIZOBJECTDATA::WIZOBJECTDATA(const WIZOBJECTDATA& data)
{
    strDisplayName = data.strDisplayName;
    strObjectGUID = data.strObjectGUID;
    strKbGUID = data.strKbGUID;
    tTime = data.tTime;
    eObjectType = data.eObjectType;
    arrayData = data.arrayData;
}

WIZOBJECTDATA::WIZOBJECTDATA(const WIZDOCUMENTDATA& data)
{
    strDisplayName = data.strTitle;
    strObjectGUID = data.strGUID;
    strKbGUID = data.strKbGUID;
    tTime = data.tDataModified;
    eObjectType = wizobjectDocument;
}

WIZOBJECTDATA::WIZOBJECTDATA(const WIZDOCUMENTATTACHMENTDATA& data)
{
    strDisplayName = data.strName;
    strObjectGUID = data.strGUID;
    strKbGUID = data.strKbGUID;
    tTime = data.tDataModified;
    eObjectType = wizobjectDocumentAttachment;
}


WizObjectType WIZOBJECTDATA::IntToObjectType(int n)
{
    if (n <= wizobjectError || n > wizobjectDocument)
    {
        ATLASSERT(FALSE);
        return wizobjectError;
    }

    return WizObjectType(n);
}

WizObjectType WIZOBJECTDATA::TypeStringToObjectType(const CString& strType)
{
    if (strType.IsEmpty())
    {
        TOLOG1(_T("Unknown guid type: %1"), strType);
        ATLASSERT(FALSE);
        return wizobjectError;
    }
    //
    if (0 == _tcsicmp(strType, _T("tag")))
        return wizobjectTag;
    if (0 == _tcsicmp(strType, _T("tag_group")))
        return wizobjectTag;
    else if (0 == _tcsicmp(strType, _T("style")))
        return wizobjectStyle;
    else if (0 == _tcsicmp(strType, _T("attachment")))
        return wizobjectDocumentAttachment;
    else if (0 == _tcsicmp(strType, _T("document")))
        return wizobjectDocument;
    //
    TOLOG1(_T("Unknown guid type: %1"), strType);
    ATLASSERT(FALSE);
    return wizobjectError;
}

QString WIZOBJECTDATA::ObjectTypeToTypeString(WizObjectType eType)
{
    switch (eType) {
    case wizobjectTag:
        return "tag";
    case wizobjectStyle:
        return "style";
    case wizobjectDocumentAttachment:
        return "attachment";
    case wizobjectDocument:
        return "document";
    default:
        TOLOG1("Unknown guid type value: %1", WizIntToStr(eType));
        Q_ASSERT(0);
        return QString();
    }
}

WIZTAGDATA::WIZTAGDATA()
    : nVersion(-1)
    , nPostion(0)
{
}

WIZTAGDATA::WIZTAGDATA(const WIZTAGDATA& data)
{
    strKbGUID = data.strKbGUID;
    strGUID = data.strGUID;
    strParentGUID = data.strParentGUID;
    strName = data.strName;
    strDescription = data.strDescription;
    tModified = data.tModified;
    nVersion = data.nVersion;
    nPostion = data.nPostion;
}

BOOL WIZTAGDATA::EqualForSync(const WIZTAGDATA& data) const
{
    ATLASSERT(strGUID == data.strGUID);
    return strName == data.strName
        && strDescription == data.strDescription
        && strParentGUID == data.strParentGUID;
}

BOOL WIZTAGDATA::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    return data.GetStr(_T("tag_guid"), strGUID)
            && data.GetStr(_T("tag_group_guid"), strParentGUID)
            && data.GetStr(_T("tag_name"), strName)
            && data.GetStr(_T("tag_description"), strDescription)
            && data.GetTime(_T("dt_info_modified"), tModified)
            && data.GetInt64(_T("version"), nVersion);
}

BOOL WIZTAGDATA::SaveToXmlRpc(CWizXmlRpcStructValue& data) const
{
    data.AddString(_T("tag_guid"), strGUID);
    data.AddString(_T("tag_group_guid"), strParentGUID);
    data.AddString(_T("tag_name"), strName);
    data.AddString(_T("tag_description"), strDescription);
    data.AddTime(_T("dt_info_modified"), tModified);
    data.AddInt64(_T("version"), nVersion);

    return TRUE;
}

bool operator< (const WIZTAGDATA& data1, const WIZTAGDATA& data2 ) throw()
{
    return( data1.strName.CompareNoCase( data2.strName) < 0 );
}

WIZTAGDATA::~WIZTAGDATA()
{
}

////////////////////////////////////////////////////////////////////

WIZSTYLEDATA::WIZSTYLEDATA()
{
    bTextBold = FALSE;
    nFlagIndex = 0;
    nVersion = -1;
}

BOOL WIZSTYLEDATA::EqualForSync(const WIZSTYLEDATA& data) const
{
    ATLASSERT(strGUID == data.strGUID);
    return strName == data.strName
            && strDescription == data.strDescription
            && crTextColor == data.crTextColor
            && crBackColor == data.crBackColor
            && bool(bTextBold ? true : false) == bool(data.bTextBold ? true : false)
            && nFlagIndex == data.nFlagIndex;
}

BOOL WIZSTYLEDATA::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    data.GetStr(_T("style_description"), strDescription);

    return data.GetStr(_T("style_guid"), strGUID)
        && data.GetStr(_T("style_name"), strName)
        && data.GetColor(_T("style_textcolor"), crTextColor)
        && data.GetColor(_T("style_backcolor"), crBackColor)
        && data.GetBool(_T("style_text_bold"), bTextBold)
        && data.GetInt(_T("style_flagindex"), nFlagIndex)
        && data.GetTime(_T("dt_info_modified"), tModified)
        && data.GetInt64(_T("version"), nVersion);
}

BOOL WIZSTYLEDATA::SaveToXmlRpc(CWizXmlRpcStructValue& data) const
{
    data.AddString(_T("style_description"), strDescription);;

    data.AddString(_T("style_guid"), strGUID);
    data.AddString(_T("style_name"), strName);
    data.AddColor(_T("style_textcolor"), crTextColor);
    data.AddColor(_T("style_backcolor"), crBackColor);
    data.AddBool(_T("style_text_bold"), bTextBold);
    data.AddInt(_T("style_flagindex"), nFlagIndex);
    data.AddTime(_T("dt_info_modified"), tModified);
    data.AddInt64(_T("version"), nVersion);;

    return TRUE;
}

bool operator< (const WIZSTYLEDATA& data1, const WIZSTYLEDATA& data2 ) throw()
{
    return( data1.strName.CompareNoCase( data2.strName) < 0 );
}

/* ------------------------- WIZDOCUMENTDATABASE ------------------------- */
WIZDOCUMENTDATABASE::WIZDOCUMENTDATABASE()
    : nVersion(-1)
    , nObjectPart(WIZKM_XMLRPC_OBJECT_PART_INFO | WIZKM_XMLRPC_OBJECT_PART_PARAM)
{
}

bool WIZDOCUMENTDATABASE::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    data.GetString("data_md5", strDataMD5);
    data.GetString("document_category", strLocation);
    data.GetString("document_guid", strGUID);
    data.GetString("document_title", strTitle);

    data.GetTime("dt_info_modified", tInfoModified);
    data.GetTime("dt_data_modified", tDataModified);
    data.GetTime("dt_param_modified", tParamModified);

    data.GetString("info_md5", strInfoMD5);
    data.GetString("param_md5", strParamMD5);

    data.GetInt64("version", nVersion);

    return !strGUID.isEmpty();
}


/* ---------------------------- WIZDOCUMENTDATA ---------------------------- */
WIZDOCUMENTDATA::WIZDOCUMENTDATA()
    : WIZDOCUMENTDATABASE()
    , nIconIndex(0)
    , nSync(0)
    , nProtected(0)
    , nReadCount(0)
    , nAttachmentCount(0)
    , nIndexed(0)
    , nFlags(0)
    , nRate(0)
    , nShareFlags(0)
{
}

bool WIZDOCUMENTDATA::EqualForSync(const WIZDOCUMENTDATA& data) const
{
    ATLASSERT(strGUID == data.strGUID);
    return strInfoMD5 == data.strInfoMD5
            && strDataMD5 == data.strDataMD5
            && strParamMD5 == data.strParamMD5;
}

WIZDOCUMENTDATA::~WIZDOCUMENTDATA()
{
}


/* -------------------------- WIZDOCUMENTPARAMDATA -------------------------- */
bool WIZDOCUMENTPARAMDATA::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    return data.GetString("param_name", strName)
        && data.GetString("param_value", strValue);
}

bool WIZDOCUMENTPARAMDATA::SaveToXmlRpc(CWizXmlRpcStructValue& data) const
{
    data.AddString("param_name", strName);
    data.AddString("param_value", strValue);
    return true;
}

bool WIZDOCUMENTDATAEX_XMLRPC_SIMPLE::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    data.GetStr(_T("document_guid"), strGUID);
    data.GetStr(_T("document_title"), strTitle);	/*用于同步过程，显示正在下载的文章标题////*/
    data.GetStr(_T("document_category"), strLocation);	/*用于CyberArticle判断是否需要下载该文档(是否属于当前书籍）////*/
    /*
        data.GetStr(_T("document_filename"), strName);
        data.GetStr(_T("document_seo"), strSEO);
        data.GetStr(_T("document_url"), strURL);
        data.GetStr(_T("document_author"), strAuthor);
        data.GetStr(_T("document_keywords"), strKeywords);
        data.GetStr(_T("document_type"), strType);
        data.GetStr(_T("document_owner"), strOwner);
        data.GetStr(_T("document_filetype"), strFileType);
        data.GetStr(_T("document_styleguid"), strStyleGUID);
        data.GetTime(_T("dt_created"), tCreated);
        data.GetTime(_T("dt_modified"), tModified);
        data.GetTime(_T("dt_accessed"), tAccessed);
        data.GetInt(_T("document_iconindex"), nIconIndex);
        data.GetInt(_T("document_protected"), nProtected);
        data.GetInt(_T("document_readcount"), nReadCount);
        data.GetInt(_T("document_attachment_count"), nAttachmentCount);
        */
    data.GetTime(_T("dt_info_modified"), tInfoModified);
    data.GetStr(_T("info_md5"), strInfoMD5);
    data.GetTime(_T("dt_data_modified"), tDataModified);
    data.GetStr(_T("data_md5"), strDataMD5);
    data.GetTime(_T("dt_param_modified"), tParamModified);
    data.GetStr(_T("param_md5"), strParamMD5);
    data.GetInt64(_T("version"), nVersion);
    //
    //data.GetArrayStringArray(_T("tags"), arrayTagGUID);
    //
    return !strGUID.isEmpty();
}


/* -------------------------- WIZDELETEDGUIDDATA -------------------------- */
WIZDELETEDGUIDDATA::WIZDELETEDGUIDDATA()
{
    eType = wizobjectError;
    nVersion = 0;
}

bool WIZDELETEDGUIDDATA::EqualForSync(const WIZDELETEDGUIDDATA& data) const
{
    ATLASSERT(strGUID == data.strGUID);
    return TRUE;
}

bool WIZDELETEDGUIDDATA::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    CString strType;

    // this field maybe "nil"
    data.GetTime(_T("dt_deleted"), tDeleted);

    bool bRet = data.GetStr(_T("deleted_guid"), strGUID)
        && data.GetStr(_T("guid_type"), strType)
        && data.GetInt64(_T("version"), nVersion);

    eType = WIZOBJECTDATA::TypeStringToObjectType(strType);

    return bRet;
}

bool WIZDELETEDGUIDDATA::SaveToXmlRpc(CWizXmlRpcStructValue& data) const
{
    data.AddString("deleted_guid", strGUID);
    data.AddString("guid_type", WIZOBJECTDATA::ObjectTypeToTypeString(eType));
    data.AddTime("dt_deleted", tDeleted);
    data.AddInt64("version", nVersion);

    return true;
}


////////////////////////////////////////////////////////////////////
WIZDOCUMENTATTACHMENTDATA::WIZDOCUMENTATTACHMENTDATA()
    : nVersion(-1)
{
}

BOOL WIZDOCUMENTATTACHMENTDATA::EqualForSync(const WIZDOCUMENTATTACHMENTDATA& data) const
{
    ATLASSERT(strGUID == data.strGUID);
    return strInfoMD5 == data.strInfoMD5
            && strDataMD5 == data.strDataMD5;
}

BOOL WIZDOCUMENTATTACHMENTDATA::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    data.GetStr("attachment_guid", strGUID);
    data.GetStr("attachment_document_guid", strDocumentGUID);
    data.GetStr("attachment_name", strName);
    data.GetStr("attachment_url", strURL);
    data.GetStr("attachment_description", strDescription);
    data.GetTime("dt_info_modified", tInfoModified);
    data.GetStr("info_md5", strInfoMD5);
    data.GetTime("dt_data_modified", tDataModified);
    data.GetStr("data_md5", strDataMD5);
    data.GetInt64("version", nVersion);

    return !strGUID.isEmpty() && !strDocumentGUID.isEmpty();
}

bool operator< (const WIZDOCUMENTATTACHMENTDATA& data1,const WIZDOCUMENTATTACHMENTDATA& data2 ) throw()
{
    return( data1.strName.compare(data2.strName, Qt::CaseInsensitive) < 0 );
}

WIZDOCUMENTATTACHMENTDATA::~WIZDOCUMENTATTACHMENTDATA()
{
}



////////////////////////////////////////////////////////////////////
WIZDOCUMENTDATAEX::WIZDOCUMENTDATAEX()
{
    bSkipped = false;
}

WIZDOCUMENTDATAEX::WIZDOCUMENTDATAEX(const WIZDOCUMENTDATA& data)
    : WIZDOCUMENTDATA(data)
{
    bSkipped = false;
}

WIZDOCUMENTDATAEX& WIZDOCUMENTDATAEX::operator= (const WIZDOCUMENTDATAEX& right)
{
    WIZDOCUMENTDATA::operator = (right);

    arrayTagGUID.assign(right.arrayTagGUID.begin(), right.arrayTagGUID.end());
    arrayParam.assign(right.arrayParam.begin(), right.arrayParam.end());
    arrayData = right.arrayData;

    bSkipped = right.bSkipped;

    return *this;
}

BOOL WIZDOCUMENTDATAEX::ParamArrayToStringArray(CWizStdStringArray& params) const
{
    for (std::deque<WIZDOCUMENTPARAMDATA>::const_iterator it = arrayParam.begin();
    it != arrayParam.end();
    it++)
    {
        params.push_back(it->strName + _T("=") + it->strValue);
    }

    return TRUE;
}

BOOL WIZDOCUMENTDATAEX::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    bool bInfo = false;
    bool bData = false;
    bool bParam = false;
    data.GetBool("document_info", bInfo);
    data.GetBool("document_data", bData);
    data.GetBool("document_param", bParam);
    data.GetInt64("version", nVersion);

    // document_data field default is 0, aquire data use other api now.
    Q_ASSERT(!bData);

    nObjectPart = 0;
    nObjectPart |= bInfo ? WIZKM_XMKRPC_DOCUMENT_PART_INFO : 0;
    nObjectPart |= bData ? WIZKM_XMKRPC_DOCUMENT_PART_DATA : 0;
    nObjectPart |= bParam ? WIZKM_XMKRPC_DOCUMENT_PART_PARAM : 0;

    data.GetString("document_guid", strGUID);

    if (bInfo) {
        data.GetString("document_title", strTitle);
        data.GetString("document_category", strLocation);
        data.GetString("document_filename", strName);
        data.GetString("document_seo", strSEO);
        data.GetString("document_url", strURL);
        data.GetString("document_author", strAuthor);
        data.GetString("document_keywords", strKeywords);
        data.GetString("document_type", strType);
        data.GetString("document_owner", strOwner);
        data.GetString("document_filetype", strFileType);
        data.GetString("document_styleguid", strStyleGUID);
        data.GetInt("document_iconindex", nIconIndex);
        data.GetInt("document_protected", nProtected);
        data.GetInt("document_attachment_count", nAttachmentCount);

        // md5
        data.GetString("data_md5", strDataMD5);
        data.GetString("info_md5", strInfoMD5);
        data.GetString("param_md5", strParamMD5);

        // time
        data.GetTime("dt_created", tCreated);
        data.GetTime("dt_modified", tModified);
        data.GetTime("dt_accessed", tAccessed);
        data.GetTime("dt_data_modified", tDataModified);
        data.GetTime("dt_info_modified", tInfoModified);
        data.GetTime("dt_param_modified", tParamModified);

        data.GetStringArray("document_tags", arrayTagGUID);
    }

    if (bData) {
        Q_ASSERT(0);
    }

    if (bParam) {
        CWizDocumentParamDataArray params;
        if (!data.GetArray("document_params", params)) {
            TOLOG("Failed to load document param when parse xml-rpc!");
            return false;
        }

        arrayParam.assign(params.begin(), params.end());
    }

    return !strGUID.isEmpty();
}

WIZDOCUMENTATTACHMENTDATAEX::WIZDOCUMENTATTACHMENTDATAEX()
    : nObjectPart(0)
{
    bSkipped = false;
}

WIZDOCUMENTATTACHMENTDATAEX::WIZDOCUMENTATTACHMENTDATAEX(const WIZDOCUMENTATTACHMENTDATA& data)
    : WIZDOCUMENTATTACHMENTDATA(data)
    , nObjectPart(0)
{
    bSkipped = false;
}

WIZDOCUMENTATTACHMENTDATAEX& WIZDOCUMENTATTACHMENTDATAEX::operator= (const WIZDOCUMENTATTACHMENTDATAEX& right)
{
    WIZDOCUMENTATTACHMENTDATA::operator = (right);

    arrayData = right.arrayData;
    bSkipped = right.bSkipped;
    nObjectPart = right.nObjectPart;

    return *this;
}


/* ------------------------------ WIZGROUPDATA ------------------------------ */
WIZGROUPDATA::WIZGROUPDATA()
    : nUserGroup(WIZ_USERGROUP_MAX)
{
}

WIZGROUPDATA::WIZGROUPDATA(const WIZGROUPDATA& data)
    : bizGUID(data.bizGUID)
    , bizName(data.bizName)
    , tCreated(data.tCreated)
    , tModified(data.tModified)
    , tRoleCreated(data.tRoleCreated)
    , strDatabaseServer(data.strDatabaseServer)
    , strGroupGUID(data.strGroupGUID)
    , strId(data.strId)
    , strGroupName(data.strGroupName)
    , strGroupNote(data.strGroupNote)
    , strGroupSEO(data.strGroupSEO)
    , strType(data.strType)
    , strOwner(data.strOwner)
    , strRoleNote(data.strRoleNote)
    , strServerUrl(data.strServerUrl)
    , strGroupTags(data.strGroupTags)
    , nUserGroup(data.nUserGroup)
    , strUserName(data.strUserName)
    , bOwn(data.bOwn)
{
}

bool WIZGROUPDATA::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    data.GetString("biz_guid", bizGUID);
    data.GetString("biz_name", bizName);

    data.GetTime("dt_created", tCreated);
    data.GetTime("dt_modified", tModified);
    data.GetTime("dt_role_created", tRoleCreated);

    data.GetString("kapi_url", strDatabaseServer);

    data.GetString("kb_guid", strGroupGUID);
    data.GetString("kb_id", strId);
    data.GetString("kb_name", strGroupName);
    data.GetString("kb_note", strGroupNote);
    data.GetString("kb_seo", strGroupSEO);
    data.GetString("kb_type", strType);
    data.GetString("owner_name", strOwner);
    data.GetString("role_note", strRoleNote);
    data.GetString("server_url", strServerUrl);
    data.GetString("tag_names", strGroupTags);
    data.GetInt("user_group", nUserGroup);
    data.GetString("user_name", strUserName);
    //
    QString owner;
    data.GetString("is_kb_owner", owner);
    owner = owner.toLower();
    bOwn = (owner == "1" || owner == "true");

    return !strGroupName.isEmpty()
            && !strGroupGUID.isEmpty()
            && !strDatabaseServer.isEmpty();
}

WIZBIZDATA::WIZBIZDATA()
    : bizUserRole(WIZ_BIZROLE_MAX)
{

}

WIZBIZDATA::WIZBIZDATA(const WIZBIZDATA& data)
    : bizName(data.bizName)
    , bizGUID(data.bizGUID)
    , bizUserRole(data.bizUserRole)
    , bizLevel(data.bizLevel)
    , bizIsDue(data.bizIsDue)
    , mapAvatarChanges(data.mapAvatarChanges)
{
}

bool WIZBIZDATA::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    data.GetStr(_T("biz_name"), bizName);
    data.GetStr(_T("biz_guid"), bizGUID);
    data.GetInt(_T("user_group"), bizUserRole);
    data.GetInt(_T("biz_level"), bizLevel);
    data.GetBool(_T("is_due"), bizIsDue);

    CWizXmlRpcStructValue* structData = data.GetStruct(_T("avatar_changes"));
    if (structData)
    {
        structData->ToStringMap(mapAvatarChanges);
    }

    if (bizGUID.isEmpty() || bizName.isEmpty())
    {
        qWarning() << "Biz data warning, guid : " << bizGUID << " biz name : " << bizName;
    }

    return true;
}

/* ---------------------------- WIZMESSAGEDATA ---------------------------- */
WIZMESSAGEDATA::WIZMESSAGEDATA()
    : nId(0)
    , nDeleteStatus(0)
    , nVersion(-1)
    , nLocalChanged(0)
{
}

WIZMESSAGEDATA::WIZMESSAGEDATA(const WIZMESSAGEDATA& data)
    : nId(data.nId)
    , bizGUID(data.bizGUID)
    , kbGUID(data.kbGUID)
    , documentGUID(data.documentGUID)
    , senderAlias(data.senderAlias)
    , senderGUID(data.senderGUID)
    , senderId(data.senderId)
    , receiverAlias(data.receiverAlias)
    , receiverGUID(data.receiverGUID)
    , receiverId(data.receiverId)
    , tCreated(data.tCreated)
    , nMessageType(data.nMessageType)
    , nReadStatus(data.nReadStatus)
    , nDeleteStatus(data.nDeleteStatus)
    , nEmailStatus(data.nEmailStatus)
    , nSMSStatus(data.nSMSStatus)
    , title(data.title)
    , messageBody(data.messageBody)
    , note(data.note)
    , nVersion(data.nVersion)
    , nLocalChanged(data.nLocalChanged)
{
}

WIZMESSAGEDATA::WIZMESSAGEDATA(const WIZUSERMESSAGEDATA& data)
    : nId(data.nMessageID)
    , bizGUID(data.strBizGUID)
    , kbGUID(data.strKbGUID)
    , documentGUID(data.strDocumentGUID)
    , senderAlias(data.strSender)
    , senderGUID(data.strSenderGUID)
    , senderId(data.strSenderID)
    , receiverAlias(data.strReceiver)
    , receiverGUID(data.strReceiverGUID)
    , receiverId(data.strReceiverID)
    , tCreated(data.tCreated)
    , nMessageType(data.nMessageType)
    , nReadStatus(data.nReadStatus)
    , nDeleteStatus(data.nDeletedStatus)
    , title(data.strTitle)
    , messageBody(data.strMessageText)
    , nVersion(data.nVersion)
    , nLocalChanged(data.nLocalChanged)
    , note(data.strNote)
{

}

bool WIZMESSAGEDATA::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    data.GetString("biz_guid", bizGUID);
    data.GetString("kb_guid", kbGUID);
    data.GetString("document_guid", documentGUID);

    data.GetInt64("id", nId);
    data.GetTime("dt_created", tCreated);
    data.GetInt("message_type", nMessageType);
    data.GetInt("read_status", nReadStatus);
    data.GetInt("delete_status", nDeleteStatus);
    data.GetInt("email_status", nEmailStatus);
    data.GetInt("sms_status", nSMSStatus);

    data.GetString("title", title);
    data.GetString("message_body", messageBody);
    data.GetString("note", note);

    data.GetString("receiver_alias", receiverAlias);
    data.GetString("receiver_guid", receiverGUID);
    data.GetString("receiver_id", receiverId);

    data.GetString("sender_alias", senderAlias);
    data.GetString("sender_guid", senderGUID);
    data.GetString("sender_id", senderId);

    data.GetInt64("version", nVersion);

    return true;
}

bool WIZMESSAGEDATA::isAd()
{
    if (nMessageType != WIZ_USER_MSG_TYPE_SYSTEM || note.isEmpty())
        return false;

    rapidjson::Document d;
    d.Parse<0>(note.toUtf8().constData());

    if (d.HasParseError())
    {
        qWarning() << "parse message note data error : " << d.GetParseError();
    }

    if (!d.HasMember("type"))
        return false;

    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QTextDecoder* encoder = codec->makeDecoder();
    QString type = encoder->toUnicode(d["type"].GetString(), d["type"].GetStringLength());

    return type == "ad";
}

/* ---------------------------- WIZKVRETURN ---------------------------- */
bool WIZKVRETURN::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    data.GetInt("return_code", nCode);
    data.GetString("value_of_key", value);
    data.GetInt64("version", nVersion);

    return nCode == 200;
}


BOOL WIZUSERMESSAGEDATA::LoadFromXmlRpc(CWizXmlRpcStructValue &data)
{
    data.GetInt64(_T("id"), nMessageID);
    data.GetStr(_T("biz_guid"), strBizGUID);
    data.GetStr(_T("kb_guid"), strKbGUID);
    data.GetStr(_T("document_guid"), strDocumentGUID);
    data.GetStr(_T("sender_guid"), strSenderGUID);
    data.GetStr(_T("sender_id"), strSenderID);
    data.GetStr(_T("receiver_guid"), strReceiverGUID);
    data.GetStr(_T("receiver_id"), strReceiverID);
    data.GetInt(_T("message_type"), nMessageType);
    data.GetInt(_T("read_status"), nReadStatus);
    data.GetInt(_T("delete_status"), nDeletedStatus);
    data.GetTime(_T("dt_created"), tCreated);
    data.GetStr(_T("message_body"), strMessageText);
    data.GetInt64(_T("version"), nVersion);
    data.GetStr(_T("sender_alias"), strSender);
    data.GetStr(_T("receiver_alias"), strReceiver);
    data.GetStr(_T("sender_alias"), strSender);
    data.GetStr(_T("title"), strTitle);
    data.GetStr(_T("note"), strNote);
    return 	TRUE;
}
