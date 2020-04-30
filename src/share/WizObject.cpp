#include "WizObject.h"

#include "WizMisc.h"
#include "utils/WizLogger.h"
#include "share/jsoncpp/json/json.h"

WIZUSERINFO::WIZUSERINFO()
    : nUserLevel(0)
    , nUserPoints(0)
{
}

WIZUSERINFO::WIZUSERINFO(const WIZUSERINFO& info)
{
    strToken = info.strToken;
    strKbGUID = info.strKbGUID;
    strKbServer = info.strKbServer;
    nMaxFileSize = info.nMaxFileSize;
    strInviteCode = info.strInviteCode;
    strMywizEmail = info.strMywizEmail;
    strNoticeLink = info.strNoticeLink;
    strNoticeText = info.strNoticeText;
    strDisplayName = info.strDisplayName;
    strUserEmail = info.strUserEmail;
    strUserGUID = info.strUserGUID;
    nUserLevel = info.nUserLevel;
    strUserLevelName = info.strUserLevelName;
    nUserPoints = info.nUserPoints;
    strUserType = info.strUserType;
    tVipExpried = info.tVipExpried;
    tCreated = info.tCreated;
}

WIZUSERINFO::WIZUSERINFO(const WIZUSERINFO& info, const WIZGROUPDATA& group)
{
    strToken = info.strToken;
    strKbGUID = group.strGroupGUID;    //group
    strKbServer = group.strKbServer;    //group
    nMaxFileSize = info.nMaxFileSize;
    strInviteCode = info.strInviteCode;
    strMywizEmail = info.strMywizEmail;
    strNoticeLink = info.strNoticeLink;
    strNoticeText = info.strNoticeText;
    strDisplayName = info.strDisplayName;
    strUserEmail = info.strUserEmail;
    strUserGUID = info.strUserGUID;
    nUserLevel = info.nUserLevel;
    strUserLevelName = info.strUserLevelName;
    nUserPoints = info.nUserPoints;
    strUserType = info.strUserType;
    tVipExpried = info.tVipExpried;
    tCreated = info.tCreated;
}

bool WIZUSERINFO::fromJson(const Json::Value& value)
{
    try {
        strToken = QString::fromStdString(value["token"].asString());
        strKbGUID = QString::fromStdString(value["kbGuid"].asString());
        strKbServer = QString::fromStdString(value["kbServer"].asString());
        strMywizEmail = QString::fromStdString(value["mywizEmail"].asString());
        nUserLevel = value["userLevel"].asInt();
        strUserLevelName = QString::fromStdString(value["userLevelName"].asString());
        nUserPoints = value["userPoints"].asInt();
        strUserType = QString::fromStdString(value["userType"].asString());
        nMaxFileSize = value["uploadSizeLimit"].asInt64();

        strInviteCode = QString::fromStdString(value["inviteCode"].asString());
        strNoticeText = QString::fromStdString(value["noticeText"].asString());
        strNoticeLink = QString::fromStdString(value["noticeLink"].asString());
        //
        tVipExpried = QDateTime::fromTime_t(value["vipDate"].asInt64() / 1000);
        //
        Json::Value user = value["user"];
        //
        strUserEmail = QString::fromStdString(user["email"].asString());
        strDisplayName = QString::fromStdString(user["displayName"].asString());
        strUserGUID = QString::fromStdString(user["userGuid"].asString());
        tCreated = QDateTime::fromTime_t(user["created"].asInt64() / 1000);
        //
        tTokenExpried = QDateTime::currentDateTime().addSecs(TOKEN_TIMEOUT_INTERVAL);
        //
        return !strToken.isEmpty()
                && !strKbGUID.isEmpty()
                && !strUserGUID.isEmpty()
                && !strKbServer.isEmpty();
        //
    } catch (Json::Exception& e) {
        TOLOG1("failed to convert json to userinfo: %1", e.what());
        return false;
    }
    //
    return true;
}


WIZUSERCERT::WIZUSERCERT()
{
}


WIZKBVALUEVERSIONS::WIZKBVALUEVERSIONS()
    : inited(false)
{

}


bool WIZKBVALUEVERSIONS::fromJson(const Json::Value& value)
{
    try {
        strKbGUID = QString::fromStdString(value["kbGuid"].asString());
        //
        Json::Value versionsVal = value["versions"];
        //
        for (int i = 0; i < versionsVal.size(); i++)
        {
            Json::Value version = versionsVal[i];
            //
            QString key = QString::fromStdString(version["key"].asString());
            __int64 versionVal = version["version"].asInt64();
            //
            versions[key] = versionVal;
        }
        //
        inited = true;
        //
    } catch (Json::Exception& e) {
        TOLOG(e.what());
        return false;
    }
    //
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
    nUserVersion = -1;
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
        strKbGUID = QString::fromStdString(value["kbGuid"].asString());
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
        nUserVersion = value["userVersion"].asInt64();
        //
    } catch (Json::Exception& e) {
        TOLOG(e.what());
        return false;
    }
    //
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
    , nPosition(0)
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
    nPosition = data.nPosition;
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
        nPosition = value["pos"].asInt64();

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
    value["modified"] = tModified.toTime_t() * (Json::UInt64)1000;
    value["pos"] = (int)nPosition;
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
    value["modified"] = tModified.toTime_t() * (Json::UInt64)1000;
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
        //
        tDeleted = QDateTime::fromTime_t(value["created"].asInt64() / 1000);
        strGUID = QString::fromStdString(value["deletedGuid"].asString());
        strKbGUID = QString::fromStdString(value["kbGuid"].asString());
        QString strType = QString::fromStdString(value["type"].asString());
        nVersion = value["version"].asInt64();
        //
        bool bRet = !strGUID.isEmpty()
                && !strType.isEmpty()
                && nVersion > 0;

        eType = WIZOBJECTDATA::typeStringToObjectType(strType);
        //
        return bRet;

    } catch (Json::Exception& e) {
        TOLOG(e.what());
        return false;
    }
}

bool WIZDELETEDGUIDDATA::toJson(QString kbGuid, Json::Value& value) const
{
    value["deletedGuid"] = strGUID.toStdString();
    value["type"] = WIZOBJECTDATA::objectTypeToTypeString(eType).toStdString();
    value["created"] = tDeleted.toTime_t() * (Json::UInt64)1000;
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
    , strKbServer(data.strKbServer)
    , bizName(data.bizName)
    , tCreated(data.tCreated)
    , tModified(data.tModified)
    , tRoleCreated(data.tRoleCreated)
    , strGroupGUID(data.strGroupGUID)
    , strId(data.strId)
    , strGroupName(data.strGroupName)
    , strGroupNote(data.strGroupNote)
    , strGroupSEO(data.strGroupSEO)
    , strType(data.strType)
    , strMyWiz(data.strMyWiz)
    , strOwner(data.strOwner)
    , strRoleNote(data.strRoleNote)
    , nUserGroup(data.nUserGroup)
    , strUserName(data.strUserName)
    , bOwn(data.bOwn)
    , bEncryptData(data.bEncryptData)
{
}

bool WIZGROUPDATA::fromJson(const Json::Value& value)
{
    try {
        //
        strKbServer = QString::fromStdString(value["kbServer"].asString());
        bizGUID = QString::fromStdString(value["bizGuid"].asString());
        bizName = QString::fromStdString(value["bizName"].asString());
        tCreated = QDateTime::fromTime_t(value["created"].asInt64() / 1000);
        tModified = QDateTime::fromTime_t(value["modified"].asInt64() / 1000);
        tRoleCreated = QDateTime::fromTime_t(value["roleCreated"].asInt64() / 1000);
        strGroupGUID = QString::fromStdString(value["kbGuid"].asString());
        strId = QString::fromStdString(value["id"].asString());
        strGroupName = QString::fromStdString(value["name"].asString());
        strGroupNote = QString::fromStdString(value["note"].asString());
        strGroupSEO = QString::fromStdString(value["seo"].asString());
        strType = QString::fromStdString(value["type"].asString());
        strMyWiz = QString::fromStdString(value["myWizEmail"].asString());
        strOwner = QString::fromStdString(value["ownerGuid"].asString());
        strRoleNote = QString::fromStdString(value["roleNote"].asString());
        nUserGroup = value["userGroup"].asInt();
        strUserName = QString::fromStdString(value["userName"].asString());
        //
        QString isKbOwner = QString::fromStdString(value["isKbOwner"].asString());
        isKbOwner = isKbOwner.toLower();
        bOwn = (isKbOwner == "1" || isKbOwner == "true");
        //
        QString encryptData = QString::fromStdString(value["isEncrypt"].asString());
        encryptData = encryptData.toLower();
        bEncryptData = (encryptData == "1" || encryptData == "true");
        //
        return !strGroupName.isEmpty()
                && !strGroupGUID.isEmpty();

    } catch (Json::Exception& err) {
        TOLOG1("Failed to convert json to group, %1", err.what());
        return false;
    }

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

bool WIZBIZDATA::fromJson(const Json::Value& value)
{
    try {
        bizName = QString::fromStdString(value["name"].asString());
        bizGUID = QString::fromStdString(value["bizGuid"].asString());
        bizUserRole = value["userRole"].asInt();
        bizLevel = value["level"].asInt();
        bizIsDue = value["isDue"].asBool();
        //
        Json::Value avatars = value["avatarChanges"];
        if (avatars.isObject())
        {
            Json::Value::Members keys = avatars.getMemberNames();
            for (auto key : keys) {
                //
                QString name = QString::fromStdString(avatars[key].asString());
                mapAvatarChanges[QString::fromStdString(key)] = name;
            }
        }
        //
        return !bizName.isEmpty()
                && !bizGUID.isEmpty();

    } catch (Json::Exception& err) {
        TOLOG1("Failed to convert json to biz, %1", err.what());
        return false;
    }
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

bool WIZMESSAGEDATA::fromJson(const Json::Value& value)
{
    try {
        nId = (__int64)value["id"].asInt64();
        bizGUID = QString::fromStdString(value["biz_guid"].asString());
        kbGUID = QString::fromStdString(value["kb_guid"].asString());
        documentGUID = QString::fromStdString(value["document_guid"].asString());
        senderGUID = QString::fromStdString(value["sender_guid"].asString());
        senderId = QString::fromStdString(value["sender_id"].asString());
        receiverGUID = QString::fromStdString(value["receiver_guid"].asString());
        receiverId = QString::fromStdString(value["receiver_id"].asString());
        messageBody = QString::fromStdString(value["message_body"].asString());
        senderAlias = QString::fromStdString(value["sender_alias"].asString());
        receiverAlias = QString::fromStdString(value["receiver_alias"].asString());
        senderAlias = QString::fromStdString(value["sender_alias"].asString());
        title = QString::fromStdString(value["title"].asString());
        note = QString::fromStdString(value["note"].asString());

        nMessageType = value["message_type"].asInt();
        nReadStatus = value["read_status"].asInt();
        nDeleteStatus = value["delete_status"].asInt();
        nVersion = (__int64)value["version"].asInt64();
        //
        time_t dateCreated = __int64(value["dt_created"].asInt64()) / 1000;
        tCreated = WizOleDateTime(dateCreated);
        //
        return nId > 0
                && nVersion > 0;
    } catch (Json::Exception err) {
        TOLOG1("invalid json: %1", err.what());
        return false;
    }
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

bool WIZBIZUSER::operator ==(const WIZBIZUSER& other) const
{
    bool ret = kbGUID == other.kbGUID
            && userGUID == other.userGUID
            && userId == other.userId
            && alias == other.alias
            && pinyin == other.pinyin;
    //
    return ret;
}


bool WIZBIZUSER::fromJson(const Json::Value& value)
{
    try {
        //
        alias = QString::fromStdString(value["alias"].asString());
        pinyin = QString::fromStdString(value["pinyin"].asString());
        userGUID = QString::fromStdString(value["user_guid"].asString());
        userId = QString::fromStdString(value["user_id"].asString());
        //user.bizGUID = strBizGUID;
        //
        return !userGUID.isEmpty();

    } catch (Json::Exception& e) {
        TOLOG(e.what());
        return false;
    }
}
