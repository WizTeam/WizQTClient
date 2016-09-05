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


bool WIZUSERINFO::loadFromXmlRpc(WizXmlRpcStructValue& val)
{
    WizXmlRpcStructValue& data = val;    
    data.getString("token", strToken);
    data.getTime("expried_time", tTokenExpried);

    data.getStr("download_url", strDownloadUrl);
    data.getStr("upload_url", strUploadUrl);
    data.getStr("capi_url", strChatUrl);
    data.getInt("enable_group", bEnableGroup);

    data.getString("kapi_url", strDatabaseServer);
    data.getString("kb_guid", strKbGUID);
    data.getString("invite_code", strInviteCode);
    data.getString("mywiz_email", strMywizEmail);
    data.getInt("upload_size_limit", nMaxFileSize);

    data.getString("notice_link", strNoticeLink);
    data.getString("notice_text", strNoticeText);
    data.getString("sns_list", strSNSList);

    if (WizXmlRpcStructValue* pUser = data.getStruct("user"))
    {
        pUser->getString("displayname", strDisplayName);
        pUser->getString("email", strUserEmail);
        pUser->getString("language", strLanguage);
        pUser->getString("nickname", strNickName);
        pUser->getString("user_guid", strUserGUID);
        pUser->getTime("dt_created", tCreated);
    }

    data.getInt("user_level", nUserLevel);
    data.getString("user_level_name", strUserLevelName);
    data.getInt("user_points", nUserPoints);
    data.getString("user_type", strUserType);
    data.getTime("vip_date", tVipExpried);

    return !strToken.isEmpty()
            && !strKbGUID.isEmpty()
            && !strUserGUID.isEmpty()
            && !strDatabaseServer.isEmpty();
}

WIZUSERCERT::WIZUSERCERT()
{
}

bool WIZUSERCERT::loadFromXmlRpc(WizXmlRpcStructValue& val)
{
    WizXmlRpcStructValue& data = val;
    data.getStr("n", strN);
    data.getStr("e", stre);
    data.getStr("d", strd);
    data.getStr("hint", strHint);

    return true;
}


WIZKBINFO::WIZKBINFO()
{
    nStorageLimit = 0;
    nStorageUsage = 0;
    nTrafficLimit = 0;
    nTrafficUsage = 0;
}

bool WIZKBINFO::loadFromXmlRpc(WizXmlRpcStructValue& data)
{
    data.getInt64(_T("storage_limit"), nStorageLimit);
    data.getInt64(_T("storage_usage"), nStorageUsage);
    data.getStr(_T("storage_limit_string"), strStorageLimit);
    data.getStr(_T("storage_usage_string"), strStorageUsage);
    data.getInt64(_T("traffic_limit"), nTrafficLimit);
    data.getInt64(_T("traffic_usage"), nTrafficUsage);
    data.getStr(_T("traffic_limit_string"), strTrafficLimit);
    data.getStr(_T("traffic_usage_string"), strTrafficUsage);
    data.getInt64("upload_size_limit", nUploadSizeLimit);
    data.getString("upload_size_limit_string", strUploadSizeLimitString);
    data.getInt64("notes_count", nNotesCount);
    data.getInt64("notes_count_limit", nNotesCountLimit);

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

bool WIZOBJECTPARTDATA::loadFromXmlRpc(WizXmlRpcStructValue& data)
{
    data.getInt("eof", bEOF);
    data.getInt64("obj_size", nObjectSize);
    data.getString("part_md5", strPartMD5);
    data.getInt64("part_size", nPartSize);

    if (!data.getStream("data", arrayData)){
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


WizObjectType WIZOBJECTDATA::intToObjectType(int n)
{
    if (n <= wizobjectError || n > wizobjectDocument)
    {
        ATLASSERT(FALSE);
        return wizobjectError;
    }

    return WizObjectType(n);
}

WizObjectType WIZOBJECTDATA::typeStringToObjectType(const CString& strType)
{
    if (strType.isEmpty())
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

QString WIZOBJECTDATA::objectTypeToTypeString(WizObjectType eType)
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

BOOL WIZTAGDATA::equalForSync(const WIZTAGDATA& data) const
{
    ATLASSERT(strGUID == data.strGUID);
    return strName == data.strName
        && strDescription == data.strDescription
        && strParentGUID == data.strParentGUID;
}

BOOL WIZTAGDATA::loadFromXmlRpc(WizXmlRpcStructValue& data)
{
    return data.getStr(_T("tag_guid"), strGUID)
            && data.getStr(_T("tag_group_guid"), strParentGUID)
            && data.getStr(_T("tag_name"), strName)
            && data.getStr(_T("tag_description"), strDescription)
            && data.getTime(_T("dt_info_modified"), tModified)
            && data.getInt64(_T("version"), nVersion);
}

BOOL WIZTAGDATA::saveToXmlRpc(WizXmlRpcStructValue& data) const
{
    data.addString(_T("tag_guid"), strGUID);
    data.addString(_T("tag_group_guid"), strParentGUID);
    data.addString(_T("tag_name"), strName);
    data.addString(_T("tag_description"), strDescription);
    data.addTime(_T("dt_info_modified"), tModified);
    data.addInt64(_T("version"), nVersion);

    return TRUE;
}

bool operator< (const WIZTAGDATA& data1, const WIZTAGDATA& data2 ) throw()
{
    return( data1.strName.compareNoCase( data2.strName) < 0 );
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

BOOL WIZSTYLEDATA::equalForSync(const WIZSTYLEDATA& data) const
{
    ATLASSERT(strGUID == data.strGUID);
    return strName == data.strName
            && strDescription == data.strDescription
            && crTextColor == data.crTextColor
            && crBackColor == data.crBackColor
            && bool(bTextBold ? true : false) == bool(data.bTextBold ? true : false)
            && nFlagIndex == data.nFlagIndex;
}

BOOL WIZSTYLEDATA::loadFromXmlRpc(WizXmlRpcStructValue& data)
{
    data.getStr(_T("style_description"), strDescription);

    return data.getStr(_T("style_guid"), strGUID)
        && data.getStr(_T("style_name"), strName)
        && data.getColor(_T("style_textcolor"), crTextColor)
        && data.getColor(_T("style_backcolor"), crBackColor)
        && data.getBool(_T("style_text_bold"), bTextBold)
        && data.getInt(_T("style_flagindex"), nFlagIndex)
        && data.getTime(_T("dt_info_modified"), tModified)
        && data.getInt64(_T("version"), nVersion);
}

BOOL WIZSTYLEDATA::saveToXmlRpc(WizXmlRpcStructValue& data) const
{
    data.addString(_T("style_description"), strDescription);;

    data.addString(_T("style_guid"), strGUID);
    data.addString(_T("style_name"), strName);
    data.addColor(_T("style_textcolor"), crTextColor);
    data.addColor(_T("style_backcolor"), crBackColor);
    data.addBool(_T("style_text_bold"), bTextBold);
    data.addInt(_T("style_flagindex"), nFlagIndex);
    data.addTime(_T("dt_info_modified"), tModified);
    data.addInt64(_T("version"), nVersion);;

    return TRUE;
}

bool operator< (const WIZSTYLEDATA& data1, const WIZSTYLEDATA& data2 ) throw()
{
    return( data1.strName.compareNoCase( data2.strName) < 0 );
}


/* -------------------------- WIZDELETEDGUIDDATA -------------------------- */
WIZDELETEDGUIDDATA::WIZDELETEDGUIDDATA()
{
    eType = wizobjectError;
    nVersion = 0;
}

bool WIZDELETEDGUIDDATA::equalForSync(const WIZDELETEDGUIDDATA& data) const
{
    ATLASSERT(strGUID == data.strGUID);
    return TRUE;
}

bool WIZDELETEDGUIDDATA::loadFromXmlRpc(WizXmlRpcStructValue& data)
{
    CString strType;

    // this field maybe "nil"
    data.getTime(_T("dt_deleted"), tDeleted);

    bool bRet = data.getStr(_T("deleted_guid"), strGUID)
        && data.getStr(_T("guid_type"), strType)
        && data.getInt64(_T("version"), nVersion);

    eType = WIZOBJECTDATA::typeStringToObjectType(strType);

    return bRet;
}

bool WIZDELETEDGUIDDATA::saveToXmlRpc(WizXmlRpcStructValue& data) const
{
    data.addString("deleted_guid", strGUID);
    data.addString("guid_type", WIZOBJECTDATA::objectTypeToTypeString(eType));
    data.addTime("dt_deleted", tDeleted);
    data.addInt64("version", nVersion);

    return true;
}


////////////////////////////////////////////////////////////////////
WIZDOCUMENTATTACHMENTDATA::WIZDOCUMENTATTACHMENTDATA()
    : nVersion(-1)
{
}

BOOL WIZDOCUMENTATTACHMENTDATA::equalForSync(const WIZDOCUMENTATTACHMENTDATA& data) const
{
    ATLASSERT(strGUID == data.strGUID);
    return strInfoMD5 == data.strInfoMD5
            && strDataMD5 == data.strDataMD5;
}

BOOL WIZDOCUMENTATTACHMENTDATA::loadFromXmlRpc(WizXmlRpcStructValue& data)
{
    data.getStr("attachment_guid", strGUID);
    data.getStr("attachment_document_guid", strDocumentGUID);
    data.getStr("attachment_name", strName);
    data.getStr("attachment_url", strURL);
    data.getStr("attachment_description", strDescription);
    data.getTime("dt_info_modified", tInfoModified);
    data.getStr("info_md5", strInfoMD5);
    data.getTime("dt_data_modified", tDataModified);
    data.getStr("data_md5", strDataMD5);
    data.getInt64("version", nVersion);

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
/// \brief WIZDOCUMENTDATAEX::WIZDOCUMENTDATAEX
///
/* ---------------------------- WIZDOCUMENTDATA ---------------------------- */
WIZDOCUMENTDATA::WIZDOCUMENTDATA()
    : WIZOBJECTBASE()
    , nVersion(-1)
    , nProtected(0)
    , nReadCount(0)
    , nAttachmentCount(0)
    , nIndexed(0)
    , nInfoChanged(1)
    , nDataChanged(1)
{
}

bool WIZDOCUMENTDATA::equalForSync(const WIZDOCUMENTDATA& data) const
{
    ATLASSERT(strGUID == data.strGUID);
    return strDataMD5 == data.strDataMD5;
}

WIZDOCUMENTDATA::~WIZDOCUMENTDATA()
{
}

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
    arrayData = right.arrayData;

    bSkipped = right.bSkipped;

    return *this;
}

BOOL WIZDOCUMENTDATAEX::loadFromXmlRpc(WizXmlRpcStructValue& data)
{
    nInfoChanged = 0;
    nDataChanged = 0;
    data.getInt64("version", nVersion);
    data.getString("document_guid", strGUID);

    data.getString("document_title", strTitle);
    data.getString("document_category", strLocation);
    data.getString("data_md5", strDataMD5);
    data.getString("document_filename", strName);
    data.getString("document_url", strURL);
    //
    data.getString("document_seo", strSEO);
    data.getString("document_author", strAuthor);
    data.getString("document_keywords", strKeywords);
    data.getString("document_type", strType);
    data.getString("document_owner", strOwner);
    data.getString("document_filetype", strFileType);
    data.getString("document_styleguid", strStyleGUID);
    data.getInt("document_protect", nProtected);
    data.getInt("document_attachment_count", nAttachmentCount);

    // time
    data.getTime("dt_created", tCreated);
    data.getTime("dt_modified", tModified);
    data.getTime("dt_data_modified", tDataModified);

    if (WizXmlRpcArrayValue* objTags = data.getArray("document_tags"))
    {
        std::deque<WizXmlRpcValue*> arr = objTags->value();
        for (WizXmlRpcValue* elem : arr)
        {
            if (WizXmlRpcStructValue* pTag = dynamic_cast<WizXmlRpcStructValue *>(elem))
            {
                QString tagGuid;
                if (pTag->getString("tag_guid", tagGuid))
                {
                    arrayTagGUID.push_back(tagGuid);
                }
            }
            else if (WizXmlRpcStringValue* pTag = dynamic_cast<WizXmlRpcStringValue *>(elem))
            {
                arrayTagGUID.push_back(pTag->toString());
            }
            else
            {
                qDebug() << "invalid tag param from server: " << elem->toString();
            }
        }
    }
    else
    {
        QString tagGuids;
        data.getString("document_tag_guids", tagGuids);
        if (!tagGuids.isEmpty())
        {
            QStringList sl = tagGuids.split('*');
            arrayTagGUID.assign(sl.begin(), sl.end());
        }
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
    , bEncryptData(false)
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
    , strMyWiz(data.strMyWiz)
    , strOwner(data.strOwner)
    , strRoleNote(data.strRoleNote)
    , strServerUrl(data.strServerUrl)
    , strGroupTags(data.strGroupTags)
    , nUserGroup(data.nUserGroup)
    , strUserName(data.strUserName)
    , bOwn(data.bOwn)
    , bEncryptData(data.bEncryptData)
{
}

bool WIZGROUPDATA::loadFromXmlRpc(WizXmlRpcStructValue& data)
{
    data.getString("biz_guid", bizGUID);
    data.getString("biz_name", bizName);

    data.getTime("dt_created", tCreated);
    data.getTime("dt_modified", tModified);
    data.getTime("dt_role_created", tRoleCreated);

    data.getString("kapi_url", strDatabaseServer);

    data.getString("kb_guid", strGroupGUID);
    data.getString("kb_id", strId);
    data.getString("kb_name", strGroupName);
    data.getString("kb_note", strGroupNote);
    data.getString("kb_seo", strGroupSEO);
    data.getString("kb_type", strType);
    data.getString("mywiz_email", strMyWiz);
    data.getString("owner_name", strOwner);
    data.getString("role_note", strRoleNote);
    data.getString("server_url", strServerUrl);
    data.getString("tag_names", strGroupTags);
    data.getInt("user_group", nUserGroup);
    data.getString("user_name", strUserName);
    //
    QString owner;
    data.getString("is_kb_owner", owner);
    owner = owner.toLower();
    bOwn = (owner == "1" || owner == "true");
    //
    QString encryptData;
    data.getString(("is_encrypt"), encryptData);
    encryptData = encryptData.toLower();
    bEncryptData = (encryptData == "1" || encryptData == "true");

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

bool WIZBIZDATA::loadFromXmlRpc(WizXmlRpcStructValue& data)
{
    data.getStr(_T("biz_name"), bizName);
    data.getStr(_T("biz_guid"), bizGUID);
    data.getInt(_T("user_group"), bizUserRole);
    data.getInt(_T("biz_level"), bizLevel);
    data.getBool(_T("is_due"), bizIsDue);

    WizXmlRpcStructValue* structData = data.getStruct(_T("avatar_changes"));
    if (structData)
    {
        structData->toStringMap(mapAvatarChanges);
    }

    if (bizGUID.isEmpty() || bizName.isEmpty())
    {
        qWarning() << "Biz data warning, guid : " << bizGUID << " biz name : " << bizName;
    }

    return true;
}

//TODO: 合并WIZUSERMESSAGEDATA 和WIZMESSAGEDATA
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

bool WIZMESSAGEDATA::loadFromXmlRpc(WizXmlRpcStructValue& data)
{
    data.getString("biz_guid", bizGUID);
    data.getString("kb_guid", kbGUID);
    data.getString("document_guid", documentGUID);

    data.getInt64("id", nId);
    data.getTime("dt_created", tCreated);
    data.getInt("message_type", nMessageType);
    data.getInt("read_status", nReadStatus);
    data.getInt("delete_status", nDeleteStatus);
    data.getInt("email_status", nEmailStatus);
    data.getInt("sms_status", nSMSStatus);

    data.getString("title", title);
    data.getString("message_body", messageBody);
    data.getString("note", note);

    data.getString("receiver_alias", receiverAlias);
    data.getString("receiver_guid", receiverGUID);
    data.getString("receiver_id", receiverId);

    data.getString("sender_alias", senderAlias);
    data.getString("sender_guid", senderGUID);
    data.getString("sender_id", senderId);

    data.getInt64("version", nVersion);

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
    QString type = encoder->toUnicode(d["type"].getString(), d["type"].GetStringLength());

    return type == "ad";
}

/* ---------------------------- WIZKVRETURN ---------------------------- */
bool WIZKVRETURN::loadFromXmlRpc(WizXmlRpcStructValue& data)
{
    data.getInt("return_code", nCode);
    data.getString("value_of_key", value);
    data.getInt64("version", nVersion);

    return nCode == 200;
}


BOOL WIZUSERMESSAGEDATA::loadFromXmlRpc(WizXmlRpcStructValue &data)
{
    data.getInt64(_T("id"), nMessageID);
    data.getStr(_T("biz_guid"), strBizGUID);
    data.getStr(_T("kb_guid"), strKbGUID);
    data.getStr(_T("document_guid"), strDocumentGUID);
    data.getStr(_T("sender_guid"), strSenderGUID);
    data.getStr(_T("sender_id"), strSenderID);
    data.getStr(_T("receiver_guid"), strReceiverGUID);
    data.getStr(_T("receiver_id"), strReceiverID);
    data.getInt(_T("message_type"), nMessageType);
    data.getInt(_T("read_status"), nReadStatus);
    data.getInt(_T("delete_status"), nDeletedStatus);
    data.getTime(_T("dt_created"), tCreated);
    data.getStr(_T("message_body"), strMessageText);
    data.getInt64(_T("version"), nVersion);
    data.getStr(_T("sender_alias"), strSender);
    data.getStr(_T("receiver_alias"), strReceiver);
    data.getStr(_T("sender_alias"), strSender);
    data.getStr(_T("title"), strTitle);
    data.getStr(_T("note"), strNote);
    return 	TRUE;
}
