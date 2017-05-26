#include "WizObject.h"

#include "WizXmlRpc.h"
#include "WizMisc.h"
#include "utils/WizLogger.h"
#include "share/jsoncpp/json/json.h"

WIZUSERINFO::WIZUSERINFO()
    : nUserLevel(0)
    , nUserPoints(0)
    , bEnableGroup(false)
    , syncType(0)
{
}

WIZUSERINFO::WIZUSERINFO(const WIZUSERINFO& info)
{
    strToken = info.strToken;
    strKbGUID = info.strKbGUID;
    strDatabaseServer = info.strDatabaseServer;
    strKbServer = info.strKbServer;
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
    syncType = info.syncType;
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
    //
    data.getStr("kb_server_url", strKbServer);
    //
    data.getInt("sync_type", syncType);

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
    nDocumentVersion = -1;
    nTagVersion = -1;
    nStyleVersion = -1;
    nAttachmentVersion = -1;
    nDeletedGUIDVersion = -1;
    nParamVersion = -1;
    //
    nStorageLimit = 0;
    nStorageUsage = 0;
    nTrafficLimit = 0;
    nTrafficUsage = 0;
    nUploadSizeLimit = 30 * 1024 * 1024;
}

bool WIZKBINFO::fromJson(const Json::Value& value)
{
    try {
        //
        nStorageLimit = value["storageLimit"].asInt64();
        nStorageUsage = value["storageUsage"].asInt64();
        //
        nTrafficLimit = value["trafficLimit"].asInt64();
        nTrafficUsage = value["trafficUsage"].asInt64();
        //
        nUploadSizeLimit = value["uploadSizeLimit"].asInt64();
        //
        nNotesCount = value["noteCount"].asInt64();
        nNotesCountLimit = value["noteCountLimit"].asInt64();
        //
        nDocumentVersion = value["docVersion"].asInt64();
        nTagVersion = value["tagVersion"].asInt64();
        nStyleVersion = value["styleVersion"].asInt64();
        nAttachmentVersion = value["attVersion"].asInt64();
        nDeletedGUIDVersion = value["deletedVersion"].asInt64();
        nParamVersion = value["paramVersion"].asInt64();
        //
    } catch (Json::Exception& e) {
        TOLOG(e.what());
        return false;
    }
    //
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
    strDocumentGuid = data.strDocumentGuid;
    strObjectGUID = data.strObjectGUID;
    strKbGUID = data.strKbGUID;
    tTime = data.tTime;
    eObjectType = data.eObjectType;
    arrayData = data.arrayData;
}

WIZOBJECTDATA::WIZOBJECTDATA(const WIZDOCUMENTDATA& data)
{
    strDisplayName = data.strTitle;
    strDocumentGuid = data.strGUID;
    strObjectGUID = data.strGUID;
    strKbGUID = data.strKbGUID;
    tTime = data.tDataModified;
    eObjectType = wizobjectDocument;
}

WIZOBJECTDATA::WIZOBJECTDATA(const WIZDOCUMENTATTACHMENTDATA& data)
{
    strDisplayName = data.strName;
    strDocumentGuid = data.strDocumentGUID;
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
        TOLOG1("Unknown guid type: %1", strType);
        ATLASSERT(FALSE);
        return wizobjectError;
    }
    //
    if (0 == wiz_tcsicmp(strType, "tag"))
        return wizobjectTag;
    if (0 == wiz_tcsicmp(strType, "tag_group"))
        return wizobjectTag;
    else if (0 == wiz_tcsicmp(strType, "style"))
        return wizobjectStyle;
    else if (0 == wiz_tcsicmp(strType, "attachment"))
        return wizobjectDocumentAttachment;
    else if (0 == wiz_tcsicmp(strType, "document"))
        return wizobjectDocument;
    //
    TOLOG1("Unknown guid type: %1", strType);
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

bool WIZTAGDATA::fromJson(const Json::Value& value)
{
    try {
        //
        strKbGUID = QString::fromStdString(value["kbGuid"].asString());
        strGUID = QString::fromStdString(value["tagGuid"].asString());
        strParentGUID = QString::fromStdString(value["parentTagGuid"].asString());
        strName = QString::fromStdString(value["name"].asString());
        tModified = QDateTime::fromTime_t(value["modified"].asInt64() / 1000);
        nVersion = value["version"].asInt64();

    } catch (Json::Exception& e) {
        TOLOG(e.what());
        return false;
    }
    //
    return !strGUID.isEmpty()
            && !strName.isEmpty()
            && nVersion >= 0;
}

bool WIZTAGDATA::toJson(QString kbGuid, Json::Value& value) const
{
    //value["kbGuid"] = kbGuid.toStdString();
    value["tagGuid"] = strGUID.toStdString();
    value["parentTagGuid"] = strParentGUID.toStdString();
    value["name"] = strName.toStdString();
    value["modified"] = tModified.toTime_t() * 1000;
    //
    return true;
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

bool WIZSTYLEDATA::valid()
{
    return !strName.isEmpty();
}

bool WIZSTYLEDATA::equalForSync(const WIZSTYLEDATA& data) const
{
    ATLASSERT(strGUID == data.strGUID);
    return strName == data.strName
            && strDescription == data.strDescription
            && crTextColor == data.crTextColor
            && crBackColor == data.crBackColor
            && bool(bTextBold ? true : false) == bool(data.bTextBold ? true : false)
            && nFlagIndex == data.nFlagIndex;
}


bool WIZSTYLEDATA::fromJson(const Json::Value& value)
{
    try {
        //
        strKbGUID = QString::fromStdString(value["kbGuid"].asString());
        strGUID = QString::fromStdString(value["styleGuid"].asString());
        strName = QString::fromStdString(value["name"].asString());
        crTextColor = WizStringToColor2(QString::fromStdString(value["textColor"].asString()));
        crBackColor = WizStringToColor2(QString::fromStdString(value["backColor"].asString()));
        bTextBold = value["textBold"].asBool();
        nFlagIndex = value["flagIndex"].asInt();
        tModified = QDateTime::fromTime_t(value["modified"].asInt64() / 1000);
        nVersion = value["version"].asInt64();

    } catch (Json::Exception& e) {
        TOLOG(e.what());
        return false;
    }
    //
    return !strGUID.isEmpty()
            && !strName.isEmpty()
            && nVersion >= 0;
}

bool WIZSTYLEDATA::toJson(QString kbGuid, Json::Value& value) const
{
    value["kbGuid"] = kbGuid.toStdString();
    value["styleGuid"] = strGUID.toStdString();
    value["name"] = strName.toStdString();
    value["textColor"] = ::WizColorToString(crTextColor).toStdString();
    value["backColor"] = ::WizColorToString(crBackColor).toStdString();
    value["textBold"] = bTextBold ? true : false;
    value["flagIndex"] = nFlagIndex;
    value["modified"] = tModified.toTime_t() * 1000;
    //
    return true;
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

bool WIZDELETEDGUIDDATA::fromJson(const Json::Value& value)
{
    try {
        nVersion = value["version"].asInt64();
        tDeleted = QDateTime::fromTime_t(value["deleted"].asInt64() / 1000);
        strGUID = QString::fromStdString(value["deletedGuid"].asString());
        QString type = QString::fromStdString(value["guidType"].asString());
        eType = WIZOBJECTDATA::typeStringToObjectType(type);

    } catch (Json::Exception& e) {
        TOLOG(e.what());
        return false;
    }
    //
    return !strGUID.isEmpty()
            && nVersion >= 0;
}

bool WIZDELETEDGUIDDATA::toJson(QString kbGuid, Json::Value& value) const
{
    value["deletedGuid"] = strGUID.toStdString();
    value["guidType"] = WIZOBJECTDATA::objectTypeToTypeString(eType).toStdString();
    value["deleted"] = tDeleted.toTime_t() * 1000;
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

bool WIZDOCUMENTATTACHMENTDATA::fromJson(const Json::Value& value)
{
    try {
        //
        //strKbGuid = QString::fromStdString(value["kbGuid"].asString());
        nVersion = value["version"].asInt64();
        strGUID = QString::fromStdString(value["attGuid"].asString());
        strDocumentGUID = QString::fromStdString(value["docGuid"].asString());
        strName = QString::fromStdString(value["name"].asString());
        strURL = QString::fromStdString(value["url"].asString());
        strDataMD5 = QString::fromStdString(value["dataMd5"].asString());
        strInfoMD5 = QString::fromStdString(value["infoMd5"].asString());
        tInfoModified = QDateTime::fromTime_t(value["infoModified"].asInt64() / 1000);
        tDataModified = QDateTime::fromTime_t(value["dataModified"].asInt64() / 1000);

    } catch (Json::Exception& e) {
        TOLOG(e.what());
        return false;
    }
    //
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
    , nFlags(0)
    , nRate(0)
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
    //
    strHighlightTitle = right.strHighlightTitle;
    strHighlightText = right.strHighlightText;

    return *this;
}

bool WIZDOCUMENTDATAEX::fromJson(const Json::Value& value)
{
    try {
        //
        nInfoChanged = 0;
        nDataChanged = 0;
        //strKbGuid = QString::fromStdString(value["kbGuid"].asString());
        nVersion = value["version"].asInt64();
        strGUID = QString::fromStdString(value["docGuid"].asString());
        strTitle = QString::fromStdString(value["title"].asString());
        strLocation = QString::fromStdString(value["category"].asString());
        strDataMD5 = QString::fromStdString(value["dataMd5"].asString());
        strName = QString::fromStdString(value["name"].asString());
        strURL = QString::fromStdString(value["url"].asString());
        strSEO = QString::fromStdString(value["seo"].asString());
        strAuthor = QString::fromStdString(value["author"].asString());
        strKeywords = QString::fromStdString(value["keywords"].asString());
        strType = QString::fromStdString(value["type"].asString());
        strOwner = QString::fromStdString(value["owner"].asString());
        strFileType = QString::fromStdString(value["fileType"].asString());
        strStyleGUID = QString::fromStdString(value["styleGuid"].asString());
        //
        QString tags = QString::fromStdString(value["tags"].asString());
        QStringList sl = tags.split('*');
        arrayTagGUID.assign(sl.begin(), sl.end());
        //
        nProtected = value["protected"].asInt();
        nAttachmentCount = value["attachmentCount"].asInt();
        //
        tCreated = QDateTime::fromTime_t(value["created"].asInt64() / 1000);
        tModified = QDateTime::fromTime_t(value["modified"].asInt64() / 1000);
        tDataModified = QDateTime::fromTime_t(value["dataModified"].asInt64() / 1000);

    } catch (Json::Exception& e) {
        TOLOG(e.what());
        return false;
    }
    //
    return !strGUID.isEmpty()
            && nVersion >= 0;
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
    data.getStr("biz_name", bizName);
    data.getStr("biz_guid", bizGUID);
    data.getInt("user_group", bizUserRole);
    data.getInt("biz_level", bizLevel);
    data.getBool("is_due", bizIsDue);

    WizXmlRpcStructValue* structData = data.getStruct("avatar_changes");
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

    Json::Value d;
    Json::Reader reader;
    if (!reader.parse(note.toUtf8().constData(), d))
        return false;

    if (!d.isMember("type"))
        return false;

    QString type = QString::fromStdString(d["type"].asString());

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


bool WIZUSERMESSAGEDATA::loadFromXmlRpc(WizXmlRpcStructValue &data)
{
    data.getInt64("id", nMessageID);
    data.getStr("biz_guid", strBizGUID);
    data.getStr("kb_guid", strKbGUID);
    data.getStr("document_guid", strDocumentGUID);
    data.getStr("sender_guid", strSenderGUID);
    data.getStr("sender_id", strSenderID);
    data.getStr("receiver_guid", strReceiverGUID);
    data.getStr("receiver_id", strReceiverID);
    data.getInt("message_type", nMessageType);
    data.getInt("read_status", nReadStatus);
    data.getInt("delete_status", nDeletedStatus);
    data.getTime("dt_created", tCreated);
    data.getStr("message_body", strMessageText);
    data.getInt64("version", nVersion);
    data.getStr("sender_alias", strSender);
    data.getStr("receiver_alias", strReceiver);
    data.getStr("sender_alias", strSender);
    data.getStr("title", strTitle);
    data.getStr("note", strNote);
    return 	TRUE;
}

//---------------------------------------------------------

WIZDOCUMENTPARAMDATA::WIZDOCUMENTPARAMDATA()
    : nVersion(-1)
{

}

bool WIZDOCUMENTPARAMDATA::fromJson(const Json::Value& value)
{
    try {
        //
        //strKbGuid = QString::fromStdString(value["kbGuid"].asString());
        strDocumentGuid = QString::fromStdString(value["docGuid"].asString());
        strName = QString::fromStdString(value["name"].asString());
        strValue = QString::fromStdString(value["value"].asString());
        nVersion = value["version"].asInt64();

    } catch (Json::Exception& e) {
        TOLOG(e.what());
        return false;
    }
    //
    return !strDocumentGuid.isEmpty()
            && !strName.isEmpty()
            && nVersion >= 0;
}

bool WIZDOCUMENTPARAMDATA::toJson(QString kbGuid, Json::Value& value) const
{
    value["kbGuid"] = kbGuid.toStdString();
    value["docGuid"] = strDocumentGuid.toStdString();
    value["name"] = strName.toStdString();
    value["value"] = strValue.toStdString();
    //
    return true;
}
