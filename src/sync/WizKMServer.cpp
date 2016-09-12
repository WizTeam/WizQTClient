#include "WizKMServer.h"
#include "WizApiEntry.h"
#include "WizToken.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#define WIZUSERMESSAGE_AT		0
#define WIZUSERMESSAGE_EDIT		1



WizKMXmlRpcServerBase::WizKMXmlRpcServerBase(const QString& strUrl, QObject* parent)
    : WizXmlRpcServerBase(strUrl, parent)
{
}

bool WizKMXmlRpcServerBase::getValueVersion(const QString& strMethodPrefix, const QString& strToken, const QString& strKbGUID, const QString& strKey, __int64& nVersion)
{
    CWizKMTokenOnlyParam param(strToken, strKbGUID);
    //
    param.addString("key", strKey);
    //
    QString strVersion;
    //
    if (!call(QString(strMethodPrefix) + ".getValueVersion", "version", strVersion, &param))
    {
        TOLOG1("Failed to get value version: key=%1", strKey);
        return FALSE;
    }
    //
    nVersion = wiz_ttoi64(strVersion);
    //
    return TRUE;
}
bool WizKMXmlRpcServerBase::getValue(const QString& strMethodPrefix, const QString& strToken, const QString& strKbGUID, const QString& strKey, QString& strValue, __int64& nVersion)
{
    CWizKMTokenOnlyParam param(strToken, strKbGUID);
    //
    param.addString("key", strKey);
    //
    QString strVersion;
    //
    if (!call(QString(strMethodPrefix) + ".getValue",  "value_of_key", strValue, "version", strVersion, &param))
    {
        TOLOG1("Failed to get value: key=%1", strKey);
        return FALSE;
    }
    //
    nVersion = wiz_ttoi64(strVersion);
    //
    return TRUE;
}
bool WizKMXmlRpcServerBase::setValue(const QString& strMethodPrefix, const QString& strToken, const QString& strKbGUID, const QString& strKey, const QString& strValue, __int64& nRetVersion)
{
    CWizKMTokenOnlyParam param(strToken, strKbGUID);
    //
    param.addString("key", strKey);
    param.addString("value_of_key", strValue);
    //
    QString strRetVersion;
    //
    if (!call(QString(strMethodPrefix) + ".setValue", "version", strRetVersion, &param))
    {
        TOLOG1("Failed to set value: key=%1", strKey);
        return FALSE;
    }
    //
    nRetVersion = wiz_ttoi64(strRetVersion);
    //
    return TRUE;
}


WizKMAccountsServer::WizKMAccountsServer(const QString& strUrl, QObject* parent)
    : WizKMXmlRpcServerBase(strUrl, parent)
{
    m_bLogin = FALSE;
    m_bAutoLogout = FALSE;
}

WizKMAccountsServer::~WizKMAccountsServer(void)
{
    if (m_bAutoLogout)
    {
        logout();
    }
}

void WizKMAccountsServer::setUserInfo(const WIZUSERINFO& userInfo)
{
    m_bLogin = TRUE;
    m_userInfo = userInfo;
}

void WizKMAccountsServer::onXmlRpcError()
{
}

bool WizKMAccountsServer::login(const QString& strUserName, const QString& strPassword, const QString& strType)
{
    if (m_bLogin)
    {
        return TRUE;
    }
    //
    m_bLogin = accounts_clientLogin(strUserName, strPassword, strType, m_userInfo);
    //
    return m_bLogin;
}
bool WizKMAccountsServer::logout()
{
    if (!m_bLogin)
        return FALSE;
    if (m_userInfo.strToken.isEmpty())
        return FALSE;
    //
    m_bLogin = FALSE;
    bool bRet = accounts_clientLogout(m_userInfo.strToken);
    m_userInfo.strToken.clear();
    return bRet;
}


bool WizKMAccountsServer::changePassword(const QString& strUserName, const QString& strOldPassword, const QString& strNewPassword)
{
    return accounts_changePassword(strUserName, strOldPassword, strNewPassword);
}

bool WizKMAccountsServer::changeUserId(const QString& strUserName, const QString& strPassword, const QString& strNewUserId)
{
    return accounts_changeUserId(strUserName, strPassword, strNewUserId);
}

bool WizKMAccountsServer::createAccount(const QString& strUserName, const QString& strPassword, const QString& strInviteCode, const QString& strCaptchaID, const QString& strCaptcha)
{
    return accounts_createAccount(strUserName, strPassword, strInviteCode, strCaptchaID, strCaptcha);
}

bool WizKMAccountsServer::getToken(const QString& strUserName, const QString& strPassword, QString& strToken)
{
    return accounts_getToken(strUserName, strPassword, strToken);
}
bool WizKMAccountsServer::getCert(const QString& strUserName, const QString& strPassword, QString& strN, QString& stre, QString& strd, QString& strHint)
{
    return accounts_getCert(strUserName, strPassword, strN, stre, strd, strHint);
}
bool WizKMAccountsServer::setCert(const QString& strUserName, const QString& strPassword, const QString& strN, const QString& stre, const QString& strd, const QString& strHint)
{
    return accounts_setCert(strUserName, strPassword, strN, stre, strd, strHint);
}

bool WizKMAccountsServer::getAdminBizCert(const QString& strToken, const QString& strBizGuid, QString& strN, QString& stre, QString& strd, QString& strHint)
{
    CWizKMBaseParam param;
    param.addString("token", strToken);
    param.addString("biz_guid", strBizGuid);
    //
    if (!call("accounts.getBizCert", "n", strN, "e", stre, "d", strd, "hint", strHint, &param))
    {
        TOLOG("Failed to get biz cert!");
        return false;
    }
    //
    return true;
}

inline void AddJsonMemeber(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator, const QString& name, const QString& str)
{
    QByteArray baName = name.toUtf8();
    rapidjson::Value valueName(baName.constData(), baName.size());
    //
    QByteArray baValue = str.toUtf8();
    rapidjson::Value value(baValue.constData(), baValue.size());
    doc.AddMember(valueName, value, allocator);
}

bool WizKMAccountsServer::setUserBizCert(const QString& strBizGuid, const QString& strN, const QString& stre, const QString& strd, const QString& strHint)
{
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    //
    AddJsonMemeber(doc, allocator, "n", strN);
    AddJsonMemeber(doc, allocator, "e", stre);
    AddJsonMemeber(doc, allocator, "d", strd);
    AddJsonMemeber(doc, allocator, "hint", strHint);
    //
    rapidjson::GenericStringBuffer< rapidjson::UTF8<> > buffer;
    rapidjson::Writer<rapidjson::GenericStringBuffer< rapidjson::UTF8<> > > writer(buffer);

    doc.Accept(writer);

    QByteArray ba = buffer.GetString();
    //
    QString json = QString::fromUtf8(ba);
    //
    QString key = WizFormatString1("BizCert/%1", strBizGuid);
    //
    __int64 version = -1;
    return setValue(key, json, version);
}

bool WizKMAccountsServer::getUserBizCert(const QString& strBizGuid, QString& strN, QString& stre, QString& strd, QString& strHint)
{
    QString key = WizFormatString1("BizCert/%1",strBizGuid);
    QString json;
    __int64 version = -1;
    if (!getValue(key, json, version))
        return false;
    if (version == -1)
        return false;
    //
    rapidjson::Document d;
    d.Parse<0>(json.toUtf8().constData());
    //
    try {
        strN = QString::fromUtf8(d.FindMember("n")->value.GetString());
        stre = QString::fromUtf8(d.FindMember("e")->value.GetString());
        strd = QString::fromUtf8(d.FindMember("d")->value.GetString());
        strHint = QString::fromUtf8(d.FindMember("hint")->value.GetString());
    }
    catch (...) {
        return false;
    }

    //
    return true;
}

/*
bool CWizKMAccountsServer::GetWizKMDatabaseServer(QString& strServer, int& nPort, QString& strXmlRpcFile)
{
    if (!m_bLogin)
        return FALSE;
    //
    return WizKMServerDecode(m_retLogin.strDatabaseServer, strServer, nPort, strXmlRpcFile);
}
*/

QString WizKMAccountsServer::getToken()
{
    if (!m_bLogin)
        return QString();
    //
    return m_userInfo.strToken;
}
QString WizKMAccountsServer::getKbGuid()
{
    if (!m_bLogin)
        return QString();
    //
    return m_userInfo.strKbGUID;
}

bool WizKMAccountsServer::shareSNS(const QString& strToken, const QString& strSNS, const QString& strComment, const QString& strURL, const QString& strDocumentGUID)
{
    return document_shareSNS(strToken, strSNS, strComment, strURL, strDocumentGUID);
}

bool WizKMAccountsServer::getGroupList(CWizGroupDataArray& arrayGroup)
{
    return accounts_getGroupList(arrayGroup);
}

bool WizKMAccountsServer::getBizList(CWizBizDataArray& arrayBiz)
{
    return accounts_getBizList(arrayBiz);
}

bool WizKMAccountsServer::createTempGroup(const QString& strEmails, const QString& strAccessControl, const QString& strSubject, const QString& strEmailText, WIZGROUPDATA& group)
{
    return accounts_createTempGroupKb(strEmails, strAccessControl, strSubject, strEmailText, group);
}

bool WizKMAccountsServer::keepAlive(const QString& strToken)
{
    return accounts_keepAlive(strToken);
}
bool WizKMAccountsServer::getMessages(__int64 nVersion, CWizUserMessageDataArray& arrayRet)
{
    int nCountPerPage = 100;
    //
    __int64 nNextVersion = nVersion + 1;
    //
    while (1)
    {
        CWizUserMessageDataArray arrayPageData;
        //
        if (!accounts_getMessagesByJson(nCountPerPage, nNextVersion, arrayPageData))
        {
            TOLOG2("Failed to get message list: CountPerPage=%1, Version=%2", WizIntToStr(nCountPerPage), WizInt64ToStr(nVersion));
            return FALSE;
        }
        //
        arrayRet.insert(arrayRet.end(), arrayPageData.begin(), arrayPageData.end());
        //
        for (CWizUserMessageDataArray::const_iterator it = arrayPageData.begin();
            it != arrayPageData.end();
            it++)
        {
            nNextVersion = std::max<__int64>(nNextVersion, it->nVersion);
        }
        //
        if (int(arrayPageData.size()) < nCountPerPage)
            break;
        //
        nNextVersion++;
    }
    //
    return TRUE;
}

bool WizKMAccountsServer::setMessageReadStatus(const QString& strMessageIDs, int nStatus)
{
    CWizKMBaseParam param;

    param.addString("token", getToken());
    param.addString("ids", strMessageIDs);
    param.addInt("status", nStatus);
    //
    if (!call("accounts.setReadStatus", &param))
    {
        TOLOG("accounts.setReadStatus failure!");
        return FALSE;
    }
    //
    return TRUE;
}

bool WizKMAccountsServer::setMessageDeleteStatus(const QString& strMessageIDs, int nStatus)
{
    QString strUrl = WizCommonApiEntry::messageServerUrl();
    strUrl += QString("/messages?token=%1&ids=%2").arg(m_userInfo.strToken).arg(strMessageIDs);
    qDebug() << "set message delete status, strken:" << m_userInfo.strToken << "   ids : " << strMessageIDs << " url : " << strUrl;
    //
    return deleteResource(strUrl);
}

bool WizKMAccountsServer::getValueVersion(const QString& strKey, __int64& nVersion)
{
    return WizKMXmlRpcServerBase::getValueVersion("accounts", getToken(), getKbGuid(), strKey, nVersion);
}
bool WizKMAccountsServer::getValue(const QString& strKey, QString& strValue, __int64& nVersion)
{
    return WizKMXmlRpcServerBase::getValue("accounts", getToken(), getKbGuid(), strKey, strValue, nVersion);
}
bool WizKMAccountsServer::setValue(const QString& strKey, const QString& strValue, __int64& nRetVersion)
{
    return WizKMXmlRpcServerBase::setValue("accounts", getToken(), getKbGuid(), strKey, strValue, nRetVersion);
}



QString WizKMAccountsServer::makeXmlRpcPassword(const QString& strPassword)
{
    if (strPassword.startsWith("md5."))
        return QString(strPassword);
    //
    return strPassword;
}




bool WizKMAccountsServer::accounts_clientLogin(const QString& strUserName, const QString& strPassword, const QString& strType, WIZUSERINFO& ret)
{
    DEBUG_TOLOG("Start Login");
    //
    CWizKMBaseParam param;

    param.addString("user_id", strUserName);
    param.addString("password", makeXmlRpcPassword(strPassword));
    param.addString("program_type", strType);
//    param.AddString("protocol", "https");
    //
    if (!call("accounts.clientLogin", ret, &param))
    {
        TOLOG("Failed to login!");
        return FALSE;
    }
    //
    DEBUG_TOLOG1("Login: token=%1", ret.strToken);
    //
    return TRUE;
}

bool WizKMAccountsServer::accounts_createAccount(const QString& strUserName, const QString& strPassword,
                                                  const QString& strInviteCode, const QString& strCaptchaID, const QString& strCaptcha)
{
    CWizKMBaseParam param;

    param.addString("user_id", strUserName);
    param.addString("password", makeXmlRpcPassword(strPassword));
    param.addString("invite_code", strInviteCode);
    param.addString("product_name", "WizNoteQT");
    if (!strCaptchaID.isEmpty())
    {
        param.addString("captcha_id", strCaptchaID);
        param.addString("captcha", strCaptcha);
    }
    //
    WizXmlRpcResult ret;
    if (!call("accounts.createAccount", ret, &param))
    {
        TOLOG("Failed to create account!");
        return FALSE;
    }
    //
    return TRUE;
}

bool WizKMAccountsServer::accounts_clientLogout(const QString& strToken)
{
    CWizKMTokenOnlyParam param(strToken, getKbGuid());
    //
    WizXmlRpcResult callRet;
    if (!call("accounts.clientLogout", callRet, &param))
    {
        TOLOG("Logout failure!");
        return FALSE;
    }
    //
    return TRUE;
}
bool WizKMAccountsServer::accounts_keepAlive(const QString& strToken)
{
    qDebug() << "keepAlive: " << strToken << "kb: " << getKbGuid();
    CWizKMTokenOnlyParam param(strToken, getKbGuid());

    WizXmlRpcResult callRet;
    if (!call("accounts.keepAlive", callRet, &param))
    {
        TOLOG("Keep alive failure!");
        return FALSE;
    }
    //
    return TRUE;
}



bool WizKMAccountsServer::accounts_changePassword(const QString& strUserName, const QString& strOldPassword, const QString& strNewPassword)
{
    CWizKMBaseParam param;

    param.addString("user_id", strUserName);
    param.addString("old_password", makeXmlRpcPassword(strOldPassword));
    param.addString("new_password", makeXmlRpcPassword(strNewPassword));
    //
    WizXmlRpcResult callRet;
    if (!call("accounts.changePassword", callRet, &param))
    {
        TOLOG("Change password failure!");
        return FALSE;
    }
    //
    return TRUE;
}

bool WizKMAccountsServer::accounts_changeUserId(const QString& strUserName, const QString& strPassword, const QString& strNewUserId)
{
    CWizKMBaseParam param;

    param.addString("user_id", strUserName);
    param.addString("password", makeXmlRpcPassword(strPassword));
    param.addString("new_user_id", strNewUserId);
    //
    WizXmlRpcResult callRet;
    if (!call("accounts.changeAccount", callRet, &param))
    {
        TOLOG("Change password failure!");
        return FALSE;
    }
    //
    return TRUE;
}
bool WizKMAccountsServer::accounts_getToken(const QString& strUserName, const QString& strPassword, QString& strToken)
{
    CWizKMBaseParam param;

    param.addString("user_id", strUserName);
    param.addString("password", makeXmlRpcPassword(strPassword));

    //
    if (!call("accounts.getToken", "token", strToken, &param))
    {
        TOLOG("Failed to get token!");
        return FALSE;
    }
    DEBUG_TOLOG1("Get token: %1", strToken);

    return TRUE;
}
bool WizKMAccountsServer::accounts_getCert(const QString& strUserName, const QString& strPassword, QString& strN, QString& stre, QString& strd, QString& strHint)
{
    CWizKMBaseParam param;

    param.addString("user_id", strUserName);
    param.addString("password", makeXmlRpcPassword(strPassword));
    //
    if (!call("accounts.getCert", "n", strN, "e", stre, "d", strd, "hint", strHint, &param))
    {
        TOLOG("Failed to get cert!");
        return FALSE;
    }
    //
    return TRUE;
}

bool WizKMAccountsServer::accounts_setCert(const QString& strUserName, const QString& strPassword, const QString& strN, const QString& stre, const QString& strd, const QString& strHint)
{
    CWizKMBaseParam param;

    param.addString("user_id", strUserName);
    param.addString("password", makeXmlRpcPassword(strPassword));
    param.addString("n", strN);
    param.addString("e", stre);
    param.addString("d", strd);
    param.addString("hint", strHint);
    //
    if (!call("accounts.setCert", &param))
    {
        TOLOG("Failed to set cert!");
        return FALSE;
    }
    //
    return TRUE;
}

bool WizKMAccountsServer::document_shareSNS(const QString& strToken, const QString& strSNS, const QString& strComment, const QString& strURL, const QString& strDocumentGUID)
{
    CWizKMTokenOnlyParam param(strToken, getKbGuid());
    param.addString("sns_list", strSNS);
    param.addString("content", strComment);
    param.addString("url", strURL);
    param.addString("document_guid", strDocumentGUID);
    //
    WizXmlRpcResult callRet;
    if (!call("document.shareSNS", callRet, &param))
    {
        TOLOG("share note by sns failed!");
        return FALSE;
    }
    //
    return TRUE;
}


bool WizKMAccountsServer::accounts_getGroupList(CWizGroupDataArray& arrayGroup)
{
    CWizKMTokenOnlyParam param(getToken(), getKbGuid());    
    //
    param.addString("kb_type", "group");
//    param.AddString("protocol", "https");
    //
    std::deque<WIZGROUPDATA> arrayWrap;
    if (!call("accounts.getGroupKbList", arrayWrap, &param))
    {
        TOLOG("accounts.getGroupKbList failure!");
        return FALSE;
    }
    //
    arrayGroup.assign(arrayWrap.begin(), arrayWrap.end());
    //
    return TRUE;
}
bool WizKMAccountsServer::accounts_getBizList(CWizBizDataArray& arrayBiz)
{
    CWizKMTokenOnlyParam param(getToken(), getKbGuid());    
    //
    std::deque<WIZBIZDATA> arrayWrap;
    if (!call("accounts.getUserBizs", arrayWrap, &param))
    {
        TOLOG("accounts.getUserBizs failure!");
        return FALSE;
    }
    //
    arrayBiz.assign(arrayWrap.begin(), arrayWrap.end());
    //
    return TRUE;
}

bool WizKMAccountsServer::accounts_createTempGroupKb(const QString& strEmails, const QString& strAccessControl, const QString& strSubject, const QString& strEmailText, WIZGROUPDATA& group)
{
    CWizKMTokenOnlyParam param(getToken(), getKbGuid());
    param.addString("kb_name", strSubject);
    param.addString("user_ids", strEmails);
    param.addString("group_role", strAccessControl);
    param.addString("email_ext_text", strEmailText);
    //
    WIZGROUPDATA wrap;
    if (!call("accounts.createTempGroupKb", wrap, &param))
    {
        TOLOG("document.getGroupKbList failure!");
        return FALSE;
    }
    //
    group.nUserGroup = 0;
    //
    group = wrap;
    //
    return TRUE;
}





bool WizKMAccountsServer::accounts_getMessagesByXmlrpc(int nCountPerPage, __int64 nVersion, CWizUserMessageDataArray& arrayMessage)
{
    CWizKMBaseParam param;

    param.addString("token", getToken());
    param.addString("version", WizInt64ToStr(nVersion));
    param.addInt("count", nCountPerPage);
    //
    std::deque<WIZUSERMESSAGEDATA> arrayWrap;
    if (!call("accounts.getMessages", arrayWrap, &param))
    {
        TOLOG("document.getMessage failure!");
        return FALSE;
    }
    //
    arrayMessage.assign(arrayWrap.begin(), arrayWrap.end());
    //
    return TRUE;
}

QString getStringFromRapidValue(const rapidjson::Value& u, const QString& memberName)
{
    if (!u.HasMember(memberName.toUtf8().constData()))
        return QString();

    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QTextDecoder* encoder = codec->makeDecoder();
    return encoder->toUnicode(u[memberName.toUtf8().constData()].GetString(), u[memberName.toUtf8().constData()].GetStringLength());
}

bool WizKMAccountsServer::accounts_getMessagesByJson(int nCountPerPage, __int64 nVersion, CWizUserMessageDataArray& arrayMessage)
{
    QString strUrl = WizCommonApiEntry::messageServerUrl();
    strUrl += QString("/messages?token=%1&page_size=%2&version=%3&api_version=6").arg(m_userInfo.strToken).arg(nCountPerPage).arg(nVersion);
    //
    QString strResult;
    if (!get(strUrl, strResult))
        return false;

    rapidjson::Document d;
    d.Parse<0>(strResult.toUtf8().constData());

    if (d.HasParseError() || !d.HasMember("result")) {
        qDebug() << "Error occured when try to parse json of messages";
        qDebug() << strResult;
        return false;
    }

//    qDebug() << "url : " << strUrl << " result : " << strResult;

    const rapidjson::Value& users = d["result"];
    for (rapidjson::SizeType i = 0; i < users.Size(); i++) {
        const rapidjson::Value& u = users[i];
        if (!u.IsObject()) {
            qDebug() << "Error occured when parse json of messages";
            return false;
        }

        WIZUSERMESSAGEDATA data;
        data.nMessageID = (__int64)u["id"].GetInt64();
        data.strBizGUID = getStringFromRapidValue(u, "biz_guid");
        data.strKbGUID = getStringFromRapidValue(u, "kb_guid");
        data.strDocumentGUID = getStringFromRapidValue(u, "document_guid");
        data.strSenderGUID = getStringFromRapidValue(u, "sender_guid");
        data.strSenderID = getStringFromRapidValue(u, "sender_id");
        data.strReceiverGUID = getStringFromRapidValue(u, "receiver_guid");
        data.strReceiverID = getStringFromRapidValue(u, "receiver_id");
        data.strMessageText = getStringFromRapidValue(u, "message_body");
        data.strSender = getStringFromRapidValue(u, "sender_alias");
        data.strReceiver = getStringFromRapidValue(u, "receiver_alias");
        data.strSender = getStringFromRapidValue(u, "sender_alias");
        data.strTitle = getStringFromRapidValue(u, "title");
        data.strNote = getStringFromRapidValue(u, "note");
        //
        data.nMessageType = u["message_type"].GetInt();
        data.nReadStatus = u["read_status"].GetInt();
        data.nDeletedStatus = u["delete_status"].GetInt();
        data.nVersion = (__int64)u["version"].GetInt64();
        //
        time_t dateCreated = __int64(u["dt_created"].GetInt64()) / 1000;
        data.tCreated = WizOleDateTime(dateCreated);

        arrayMessage.push_back(data);
    }
    return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






WizKMDatabaseServer::WizKMDatabaseServer(const WIZUSERINFOBASE& kbInfo, QObject* parent)
    : WizKMXmlRpcServerBase(kbInfo.strDatabaseServer, parent)
    , m_userInfo(kbInfo)
{
}
WizKMDatabaseServer::~WizKMDatabaseServer()
{
}
void WizKMDatabaseServer::onXmlRpcError()
{
}

const WIZKBINFO& WizKMDatabaseServer::kbInfo()
{
    return m_kbInfo;
}

void WizKMDatabaseServer::setKBInfo(const WIZKBINFO& info)
{
    m_kbInfo = info;
}

int WizKMDatabaseServer::getCountPerPage()
{
    /*
    static int nCountPerPage = WizKMGetPrivateInt("Sync", "CountPerPage", 200);
    */
    static int nCountPerPage = 200;
    //
    if (nCountPerPage < 10)
        nCountPerPage = 200;
    else if (nCountPerPage > 500)
        nCountPerPage = 200;
    //
    return nCountPerPage;
}

//
bool WizKMDatabaseServer::wiz_getInfo()
{
    CWizKMTokenOnlyParam param(m_userInfo.strToken, m_userInfo.strKbGUID);
    //
    if (!call("wiz.getInfo", m_kbInfo, &param))
    {
        TOLOG("getInfo failure!");
        return FALSE;
    }
    //
    return TRUE;
}

struct WIZOBJECTVERSION_XMLRPC : public WIZOBJECTVERSION
{
    bool loadFromXmlRpc(WizXmlRpcStructValue& data)
    {
        QString strType;
        //
        bool bRet = data.getInt64("document_version", nDocumentVersion)
            && data.getInt64("tag_version", nTagVersion)
            && data.getInt64("style_version", nStyleVersion)
            && data.getInt64("attachment_version", nAttachmentVersion)
            && data.getInt64("deleted_version", nDeletedGUIDVersion);
        //
        return bRet;
    }
};

bool WizKMDatabaseServer::wiz_getVersion(WIZOBJECTVERSION& version, bool bAuto)
{
    CWizKMTokenOnlyParam param(m_userInfo.strToken, m_userInfo.strKbGUID);
    //
    param.addBool("auto", bAuto);
    //
    WIZOBJECTVERSION_XMLRPC wrap;
    if (!call("wiz.getVersion", wrap, &param))
    {
        TOLOG("GetVersion failure!");
        return FALSE;
    }
    //
    version = wrap;
    //
    return TRUE;
}
//

bool WizKMDatabaseServer::getValueVersion(const QString& strKey, __int64& nVersion)
{
    return WizKMXmlRpcServerBase::getValueVersion("kb", getToken(), getKbGuid(), strKey, nVersion);
}
bool WizKMDatabaseServer::getValue(const QString& strKey, QString& strValue, __int64& nVersion)
{
    return WizKMXmlRpcServerBase::getValue("kb", getToken(), getKbGuid(), strKey, strValue, nVersion);
}
bool WizKMDatabaseServer::setValue(const QString& strKey, const QString& strValue, __int64& nRetVersion)
{
    return WizKMXmlRpcServerBase::setValue("kb", getToken(), getKbGuid(), strKey, strValue, nRetVersion);
}

bool WizKMDatabaseServer::document_downloadData(const QString& strDocumentGUID, WIZDOCUMENTDATAEX& ret)
{
    if (!data_download(ret.strGUID, "document", ret.arrayData, ret.strTitle))
    {
        TOLOG1("Failed to download attachment data: %1", ret.strTitle);
        return FALSE;
    }
    //
    return TRUE;
}
//

struct CWizKMDocumentPostDataParam
    : public CWizKMTokenOnlyParam
{
    CWizKMDocumentPostDataParam(int nApiVersion, const QString& strToken, const QString& strBookGUID, const QString& strDocumentGUID, bool withDocumentData, const WIZDOCUMENTDATA& infodata, const CWizStdStringArray& tags, const QString& strObjMd5)
        : CWizKMTokenOnlyParam(strToken, strBookGUID)
    {
        changeApiVersion(nApiVersion);
        //
        addBool("with_document_data", withDocumentData);
        //
        Q_ASSERT(strDocumentGUID == infodata.strGUID);

        addString("document_guid", strDocumentGUID);
        addString("document_title", infodata.strTitle);
        addString("document_category", infodata.strLocation);
        addString("document_filename", infodata.strName);
        addString("document_seo", infodata.strSEO);
        addString("document_url", infodata.strURL);
        addString("document_author", infodata.strAuthor);
        addString("document_keywords", infodata.strKeywords);
        addString("document_type", infodata.strType);
        addString("document_owner", infodata.strOwner);
        addString("document_filetype", infodata.strFileType);
        addString("document_styleguid", infodata.strStyleGUID);
        addTime("dt_created", infodata.tCreated);
        addTime("dt_modified", infodata.tModified);
        addTime("dt_accessed", infodata.tAccessed);
        addInt("document_protected", infodata.nProtected);
        addInt("document_readcount", infodata.nReadCount);
        addInt("document_attachment_count", infodata.nAttachmentCount);
        addTime("dt_data_modified", infodata.tDataModified);
        addString("data_md5", infodata.strDataMD5);
        addString("document_zip_md5", strObjMd5);
        //
        CString strTagGuids;
        ::WizStringArrayToText(tags, strTagGuids, "*");
        //
        addString("document_tag_guids", strTagGuids);
    }
};


bool WizKMDatabaseServer::attachment_downloadData(const QString& strAttachmentGUID, WIZDOCUMENTATTACHMENTDATAEX& ret)
{
    ATLASSERT(!ret.arrayData.isEmpty());
    if (!ret.arrayData.isEmpty())
    {
        TOLOG("fault error: ret.arrayData is not null!");
        return FALSE;
    }
    //
    if (!data_download(strAttachmentGUID, "attachment", ret.arrayData, ret.strName))
    {
        TOLOG1("Failed to download attachment data: %1", ret.strName);
        return FALSE;
    }
    //
    return TRUE;
}

struct CWizKMAttachmentPostDataParam
    : public CWizKMTokenOnlyParam
{
    CWizKMAttachmentPostDataParam(int nApiVersion, const QString& strToken, const QString& strBookGUID, const QString& strAttachmentGUID, const WIZDOCUMENTATTACHMENTDATA& infodata, const QString& strObjMd5)
        : CWizKMTokenOnlyParam(strToken, strBookGUID)
    {
        changeApiVersion(nApiVersion);

        Q_ASSERT(strAttachmentGUID == infodata.strGUID);
        addString("attachment_guid", strAttachmentGUID);
        addString("attachment_document_guid", infodata.strDocumentGUID);
        addString("attachment_name", infodata.strName);
        addString("attachment_url", infodata.strURL);
        addString("attachment_description", infodata.strDescription);
        addTime("dt_info_modified", infodata.tInfoModified);
        addString("info_md5", infodata.strInfoMD5);
        addTime("dt_data_modified", infodata.tDataModified);
        addString("data_md5", infodata.strDataMD5);
        //
        addTime("dt_data_modified", infodata.tDataModified);
        addString("data_md5", infodata.strDataMD5);
        addString("attachment_zip_md5", strObjMd5);
        //
        addBool("attachment_info", true);
        addBool("attachment_data", true);
    }
};

struct CWizKMDataDownloadParam
    : public CWizKMTokenOnlyParam
{
    CWizKMDataDownloadParam(const QString& strToken, const QString& strBookGUID, const QString& strObjectGUID, const QString& strObjectType, int pos, int size)
        : CWizKMTokenOnlyParam(strToken, strBookGUID)
    {
        addString("obj_guid", strObjectGUID);
        addString("obj_type", strObjectType);
        //
        addInt64("start_pos", pos);
        addInt64("part_size", size);
    }
};


struct WIZKMDATAPART
{
    __int64 nObjectSize;
    int bEOF;
    __int64 nPartSize;
    QString strPartMD5;
    QByteArray stream;
    //
    WIZKMDATAPART()
        : nObjectSize(0)
        , bEOF(FALSE)
        , nPartSize(0)
    {
    }
    bool loadFromXmlRpc(WizXmlRpcStructValue& data)
    {
        data.getInt64("obj_size", nObjectSize);
        data.getInt("eof", bEOF);
        data.getInt64("part_size", nPartSize);
        data.getString("part_md5", strPartMD5);
        return data.getStream("data", stream);
    }
};

bool WizKMDatabaseServer::data_download(const QString& strObjectGUID, const QString& strObjectType, int pos, int size, QByteArray& stream, int& nAllSize, bool& bEOF)
{
    CWizKMDataDownloadParam param(m_userInfo.strToken, m_userInfo.strKbGUID, strObjectGUID, strObjectType, pos, size);
    //
    WIZKMDATAPART part;
    if (!call("data.download", part, &param))
    {
        TOLOG("data.download failure!");
        return FALSE;
    }
    //
    __int64 nStreamSize = part.stream.size();
    if (part.nPartSize != nStreamSize)
    {
        TOLOG2("part size does not match: stream_size=%1, part_size=%2", WizInt64ToStr(nStreamSize), WizInt64ToStr(part.nPartSize));
        return FALSE;
    }
    //
    QString strStreamMD5 = WizMd5StringNoSpaceJava(part.stream);
    if (0 != strStreamMD5.compare(part.strPartMD5, Qt::CaseInsensitive))
    {
        TOLOG2("part md5 does not match, stream_md5=%1, part_md5=%2", strStreamMD5, part.strPartMD5);
        return FALSE;
    }
    //
    nAllSize = (int)part.nObjectSize;
    bEOF = part.bEOF;
    //
    stream.append(part.stream);
    return TRUE;
}




struct CWizKMDataUploadParam
    : public CWizKMTokenOnlyParam
{
    CWizKMDataUploadParam(const QString& strToken, const QString& strBookGUID, const QString& strObjectGUID, const QString& strObjectType, const QString& strObjectMD5, int allSize, int partCount, int partIndex, const QByteArray& stream)
        : CWizKMTokenOnlyParam(strToken, strBookGUID)
    {
        addString("obj_guid", strObjectGUID);
        addString("obj_type", strObjectType);
        addString("obj_md5", strObjectMD5);
        addInt("obj_size", allSize);
        addInt("part_count", partCount);
        addInt("part_sn", partIndex);
        addInt64("part_size", stream.size());
        addString("part_md5", ::WizMd5StringNoSpaceJava(stream));
        addBase64("data", stream);
    }
};



bool WizKMDatabaseServer::data_upload(const QString& strObjectGUID, const QString& strObjectType, const QString& strObjectMD5, int allSize, int partCount, int partIndex, int partSize, const QByteArray& stream)
{
    __int64 nStreamSize = stream.size();
    if (partSize != (int)nStreamSize)
    {
        TOLOG2("Fault error: stream_size=%1, part_size=%2", WizIntToStr(int(nStreamSize)), WizIntToStr(partSize));
        return FALSE;
    }
    //
    CWizKMDataUploadParam param(m_userInfo.strToken, m_userInfo.strKbGUID, strObjectGUID, strObjectType, strObjectMD5, allSize, partCount, partIndex, stream);
    //
    if (!call("data.upload", &param))
    {
        TOLOG("Can not upload object part data!");
        return FALSE;
    }
    //
    return TRUE;
}


bool WizKMDatabaseServer::data_download(const QString& strObjectGUID, const QString& strObjectType, QByteArray& stream, const QString& strDisplayName)
{
    stream.clear();
    //
    int nAllSize = 0;
    int startPos = 0;
    while (1)
    {
        int partSize = 500 * 1000;
        //
        bool bEOF = FALSE;
        if (!data_download(strObjectGUID, strObjectType, startPos, partSize, stream, nAllSize, bEOF))
        {
            TOLOG(WizFormatString1("Failed to download object part data: %1", strDisplayName));
            return FALSE;
        }
        //
        int nDownloadedSize = stream.size();
        //
        if (bEOF)
            break;
        //
        startPos = nDownloadedSize;

        emit downloadProgress(nAllSize, nDownloadedSize);
    }
    //
    __int64 nStreamSize = stream.size();
    if (nStreamSize != nAllSize)
    {
        TOLOG3("Failed to download object data: %1, stream_size=%2, object_size=%3", strDisplayName, WizInt64ToStr(nStreamSize), WizInt64ToStr(nAllSize));
        return FALSE;
    }
    //
    return TRUE;
}
bool WizKMDatabaseServer::data_upload(const QString& strObjectGUID, const QString& strObjectType, const QByteArray& stream, const QString& strObjMD5, const QString& strDisplayName)
{
    __int64 nStreamSize = stream.size();
    if (0 == nStreamSize)
    {
        TOLOG("fault error: stream is zero");
        return FALSE;
    }
    //
    QString strMD5(strObjMD5);
    //
    QByteArray spPartStream;
    //
    int partSize = 500 * 1000;
    int partCount = int(nStreamSize / partSize);
    if (nStreamSize % partSize != 0)
    {
        partCount++;
    }
    //
    for (int i = 0; i < partCount; i++)
    {
        spPartStream.clear();

        int start = i * partSize;
        int end = std::min<int>(start + partSize, int(nStreamSize));
        ATLASSERT(end > start);
        //
        int curPartSize = end - start;
        ATLASSERT(curPartSize <= partSize);
        //
        const char* begin = stream.data() + start;
        spPartStream = spPartStream.fromRawData(begin, curPartSize);
        //
        int curPartStreamSize = (int)spPartStream.size();
        ATLASSERT(curPartStreamSize == curPartSize);
        //
        if (!data_upload(strObjectGUID, strObjectType, strMD5, (int)nStreamSize, partCount, i, curPartSize, spPartStream))
        {
            TOLOG1("Failed to upload part data: %1", strDisplayName);
            return FALSE;
        }
    }
    //
    //
    return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////////
//
bool WizKMDatabaseServer::document_postData(const WIZDOCUMENTDATAEX& data, bool bWithDocumentData, __int64& nServerVersion)
{
    if (!data.arrayData.isEmpty() && data.arrayData.size() > m_kbInfo.getMaxFileSize())
    {
        TOLOG1("%1 is too large, skip it", data.strTitle);
        return FALSE;
    }
    //
    QString strObjMd5;
    //
    if (!data.arrayData.isEmpty() && bWithDocumentData)
    {
        strObjMd5 = WizMd5StringNoSpaceJava(data.arrayData);
        if (!data_upload(data.strGUID, "document", data.arrayData, strObjMd5, data.strTitle))
        {
            TOLOG1("Failed to upload note data: %1", data.strTitle);
            return FALSE;
        }
    }
    else
    {
        bWithDocumentData = false;
    }
    //
    CWizKMDocumentPostDataParam param(WIZKM_WEBAPI_VERSION, m_userInfo.strToken, m_userInfo.strKbGUID, data.strGUID, bWithDocumentData, data, data.arrayTagGUID, strObjMd5);
    //
    WizXmlRpcResult ret;
    if (!call("document.postSimpleData", ret, &param))
    {
        TOLOG("document.postSimpleData failure!");
        return FALSE;
    }
    //
    if (WizXmlRpcStructValue* pRet = ret.getResultValue<WizXmlRpcStructValue>())
    {
        pRet->getInt64("version", nServerVersion);
    }
    //
    return TRUE;
}


bool WizKMDatabaseServer::attachment_postData(WIZDOCUMENTATTACHMENTDATAEX& data, __int64& nServerVersion)
{
    if (data.arrayData.size() > m_kbInfo.getMaxFileSize())
    {
        TOLOG1("%1 is too large, skip it", data.strName);
        return TRUE;
    }
    //
    QString strObjMd5 = ::WizMd5StringNoSpaceJava(data.arrayData);
    //
    if (!data_upload(data.strGUID, "attachment", data.arrayData, strObjMd5, data.strName))
    {
        TOLOG1("Failed to upload attachment data: %1", data.strName);
        return FALSE;
    }
    //
    CWizKMAttachmentPostDataParam param(WIZKM_WEBAPI_VERSION, m_userInfo.strToken, m_userInfo.strKbGUID, data.strGUID, data, strObjMd5);
    //
    WizXmlRpcResult ret;
    if (!call("attachment.postSimpleData", ret, &param))
    {
        TOLOG("attachment.postSimpleData failure!");
        return FALSE;
    }
    //
    if (WizXmlRpcStructValue* pRet = ret.getResultValue<WizXmlRpcStructValue>())
    {
        pRet->getInt64("version", nServerVersion);
    }
    //
    return TRUE;
}


bool WizKMDatabaseServer::document_getListByGuids(const CWizStdStringArray& arrayDocumentGUID, std::deque<WIZDOCUMENTDATAEX>& arrayRet)
{
   return downloadList<WIZDOCUMENTDATAEX, WIZDOCUMENTDATAEX>("document.downloadInfoList", "document_guids", arrayDocumentGUID, arrayRet);
}

bool WizKMDatabaseServer::document_getInfo(const QString& strDocumentGuid, WIZDOCUMENTDATAEX& doc)
{
    CWizStdStringArray guids;
    guids.push_back(strDocumentGuid);
    //
    std::deque<WIZDOCUMENTDATAEX> arrayRet;
    if (!document_getListByGuids(guids, arrayRet))
        return FALSE;
    //
    if (arrayRet.size() != 1)
        return FALSE;
    //
    doc = arrayRet[0];
    return TRUE;
}


bool WizKMDatabaseServer::document_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDOCUMENTDATAEX>& arrayRet)
{
    return getList<WIZDOCUMENTDATAEX, WIZDOCUMENTDATAEX>("document.getSimpleList", nCountPerPage, nVersion, arrayRet);
}

bool WizKMDatabaseServer::attachment_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
{
    return getList<WIZDOCUMENTATTACHMENTDATAEX, WIZDOCUMENTATTACHMENTDATAEX>("attachment.getList", nCountPerPage, nVersion, arrayRet);
}

bool WizKMDatabaseServer::tag_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZTAGDATA>& arrayRet)
{
    return getList<WIZTAGDATA, WIZTAGDATA>("tag.getList", nCountPerPage, nVersion, arrayRet);
}

bool WizKMDatabaseServer::style_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZSTYLEDATA>& arrayRet)
{
    return getList<WIZSTYLEDATA, WIZSTYLEDATA>("style.getList", nCountPerPage, nVersion, arrayRet);
}


bool WizKMDatabaseServer::deleted_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDELETEDGUIDDATA>& arrayRet)
{
    return getList<WIZDELETEDGUIDDATA, WIZDELETEDGUIDDATA>("deleted.getList", nCountPerPage, nVersion, arrayRet);
}
bool WizKMDatabaseServer::tag_postList(std::deque<WIZTAGDATA>& arrayTag)
{
    return postList<WIZTAGDATA, WIZTAGDATA>("tag.postList", "tags", arrayTag);
}
bool WizKMDatabaseServer::style_postList(std::deque<WIZSTYLEDATA>& arrayStyle)
{
    return postList<WIZSTYLEDATA, WIZSTYLEDATA>("style.postList", "styles", arrayStyle);
}
bool WizKMDatabaseServer::deleted_postList(std::deque<WIZDELETEDGUIDDATA>& arrayDeletedGUID)
{
    return postList<WIZDELETEDGUIDDATA, WIZDELETEDGUIDDATA>("deleted.postList", "deleteds", arrayDeletedGUID);
}

bool WizKMDatabaseServer::category_getAll(QString& str)
{
    CWizKMTokenOnlyParam param(m_userInfo.strToken, m_userInfo.strKbGUID);
    //
    if (!call("category.getAll", "categories", str, &param))
    {
        TOLOG("category.getList failure!");
        return FALSE;
    }
    //
    return TRUE;
}

QByteArray WizKMDatabaseServer::downloadDocumentData(const QString& strDocumentGUID)
{
    WIZDOCUMENTDATAEX ret;
    if (!document_downloadData(strDocumentGUID, ret))
    {
        TOLOG1("Failed to download note data: %1", strDocumentGUID);
        return QByteArray();
    }
    //
    if (ret.arrayData.isEmpty())
        return QByteArray();
    //
    return ret.arrayData;
}
//

QByteArray WizKMDatabaseServer::downloadAttachmentData(const QString& strAttachmentGUID)
{
    WIZDOCUMENTATTACHMENTDATAEX ret;
    if (!attachment_downloadData(strAttachmentGUID, ret))
    {
        TOLOG1("Failed to download attachment data: %1", strAttachmentGUID);
        return QByteArray();
    }
    //
    if (ret.arrayData.isEmpty())
        return QByteArray();
    //
    return ret.arrayData;
}
