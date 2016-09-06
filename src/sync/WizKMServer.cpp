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

BOOL WizKMXmlRpcServerBase::getValueVersion(const QString& strMethodPrefix, const QString& strToken, const QString& strKbGUID, const QString& strKey, __int64& nVersion)
{
    CWizKMTokenOnlyParam param(strToken, strKbGUID);
    //
    param.addString(_T("key"), strKey);
    //
    QString strVersion;
    //
    if (!call(QString(strMethodPrefix) + _T(".getValueVersion"), _T("version"), strVersion, &param))
    {
        TOLOG1(_T("Failed to get value version: key=%1"), strKey);
        return FALSE;
    }
    //
    nVersion = _ttoi64(strVersion);
    //
    return TRUE;
}
BOOL WizKMXmlRpcServerBase::getValue(const QString& strMethodPrefix, const QString& strToken, const QString& strKbGUID, const QString& strKey, QString& strValue, __int64& nVersion)
{
    CWizKMTokenOnlyParam param(strToken, strKbGUID);
    //
    param.addString(_T("key"), strKey);
    //
    QString strVersion;
    //
    if (!call(QString(strMethodPrefix) + _T(".getValue"),  _T("value_of_key"), strValue, _T("version"), strVersion, &param))
    {
        TOLOG1(_T("Failed to get value: key=%1"), strKey);
        return FALSE;
    }
    //
    nVersion = _ttoi64(strVersion);
    //
    return TRUE;
}
BOOL WizKMXmlRpcServerBase::setValue(const QString& strMethodPrefix, const QString& strToken, const QString& strKbGUID, const QString& strKey, const QString& strValue, __int64& nRetVersion)
{
    CWizKMTokenOnlyParam param(strToken, strKbGUID);
    //
    param.addString(_T("key"), strKey);
    param.addString(_T("value_of_key"), strValue);
    //
    QString strRetVersion;
    //
    if (!call(QString(strMethodPrefix) + _T(".setValue"), _T("version"), strRetVersion, &param))
    {
        TOLOG1(_T("Failed to set value: key=%1"), strKey);
        return FALSE;
    }
    //
    nRetVersion = _ttoi64(strRetVersion);
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

BOOL WizKMAccountsServer::login(const QString& strUserName, const QString& strPassword, const QString& strType)
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
BOOL WizKMAccountsServer::logout()
{
    if (!m_bLogin)
        return FALSE;
    if (m_userInfo.strToken.isEmpty())
        return FALSE;
    //
    m_bLogin = FALSE;
    BOOL bRet = accounts_clientLogout(m_userInfo.strToken);
    m_userInfo.strToken.clear();
    return bRet;
}


BOOL WizKMAccountsServer::changePassword(const QString& strUserName, const QString& strOldPassword, const QString& strNewPassword)
{
    return accounts_changePassword(strUserName, strOldPassword, strNewPassword);
}

BOOL WizKMAccountsServer::changeUserId(const QString& strUserName, const QString& strPassword, const QString& strNewUserId)
{
    return accounts_changeUserId(strUserName, strPassword, strNewUserId);
}

BOOL WizKMAccountsServer::createAccount(const QString& strUserName, const QString& strPassword, const QString& strInviteCode, const QString& strCaptchaID, const QString& strCaptcha)
{
    return accounts_createAccount(strUserName, strPassword, strInviteCode, strCaptchaID, strCaptcha);
}

BOOL WizKMAccountsServer::getToken(const QString& strUserName, const QString& strPassword, QString& strToken)
{
    return accounts_getToken(strUserName, strPassword, strToken);
}
BOOL WizKMAccountsServer::getCert(const QString& strUserName, const QString& strPassword, QString& strN, QString& stre, QString& strd, QString& strHint)
{
    return accounts_getCert(strUserName, strPassword, strN, stre, strd, strHint);
}
BOOL WizKMAccountsServer::setCert(const QString& strUserName, const QString& strPassword, const QString& strN, const QString& stre, const QString& strd, const QString& strHint)
{
    return accounts_setCert(strUserName, strPassword, strN, stre, strd, strHint);
}

bool WizKMAccountsServer::getAdminBizCert(const QString& strToken, const QString& strBizGuid, QString& strN, QString& stre, QString& strd, QString& strHint)
{
    CWizKMBaseParam param;
    param.addString(_T("token"), strToken);
    param.addString(_T("biz_guid"), strBizGuid);
    //
    if (!call(_T("accounts.getBizCert"), _T("n"), strN, _T("e"), stre, _T("d"), strd, _T("hint"), strHint, &param))
    {
        TOLOG(_T("Failed to get biz cert!"));
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
    QString key = WizFormatString1(_T("BizCert/%1"), strBizGuid);
    //
    __int64 version = -1;
    return setValue(key, json, version);
}

bool WizKMAccountsServer::getUserBizCert(const QString& strBizGuid, QString& strN, QString& stre, QString& strd, QString& strHint)
{
    QString key = WizFormatString1(_T("BizCert/%1"),strBizGuid);
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
BOOL CWizKMAccountsServer::GetWizKMDatabaseServer(QString& strServer, int& nPort, QString& strXmlRpcFile)
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

BOOL WizKMAccountsServer::shareSNS(const QString& strToken, const QString& strSNS, const QString& strComment, const QString& strURL, const QString& strDocumentGUID)
{
    return document_shareSNS(strToken, strSNS, strComment, strURL, strDocumentGUID);
}

BOOL WizKMAccountsServer::getGroupList(CWizGroupDataArray& arrayGroup)
{
    return accounts_getGroupList(arrayGroup);
}

BOOL WizKMAccountsServer::getBizList(CWizBizDataArray& arrayBiz)
{
    return accounts_getBizList(arrayBiz);
}

BOOL WizKMAccountsServer::createTempGroup(const QString& strEmails, const QString& strAccessControl, const QString& strSubject, const QString& strEmailText, WIZGROUPDATA& group)
{
    return accounts_createTempGroupKb(strEmails, strAccessControl, strSubject, strEmailText, group);
}

BOOL WizKMAccountsServer::keepAlive(const QString& strToken)
{
    return accounts_keepAlive(strToken);
}
BOOL WizKMAccountsServer::getMessages(__int64 nVersion, CWizUserMessageDataArray& arrayRet)
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
            TOLOG2(_T("Failed to get message list: CountPerPage=%1, Version=%2"), WizIntToStr(nCountPerPage), WizInt64ToStr(nVersion));
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

BOOL WizKMAccountsServer::setMessageReadStatus(const QString& strMessageIDs, int nStatus)
{
    CWizKMBaseParam param;

    param.addString(_T("token"), getToken());
    param.addString(_T("ids"), strMessageIDs);
    param.addInt(_T("status"), nStatus);
    //
    if (!call(_T("accounts.setReadStatus"), &param))
    {
        TOLOG(_T("accounts.setReadStatus failure!"));
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

BOOL WizKMAccountsServer::getValueVersion(const QString& strKey, __int64& nVersion)
{
    return WizKMXmlRpcServerBase::getValueVersion(_T("accounts"), getToken(), getKbGuid(), strKey, nVersion);
}
BOOL WizKMAccountsServer::getValue(const QString& strKey, QString& strValue, __int64& nVersion)
{
    return WizKMXmlRpcServerBase::getValue(_T("accounts"), getToken(), getKbGuid(), strKey, strValue, nVersion);
}
BOOL WizKMAccountsServer::setValue(const QString& strKey, const QString& strValue, __int64& nRetVersion)
{
    return WizKMXmlRpcServerBase::setValue(_T("accounts"), getToken(), getKbGuid(), strKey, strValue, nRetVersion);
}



QString WizKMAccountsServer::makeXmlRpcPassword(const QString& strPassword)
{
    if (strPassword.startsWith(_T("md5.")))
        return QString(strPassword);
    //
    return strPassword;
}




BOOL WizKMAccountsServer::accounts_clientLogin(const QString& strUserName, const QString& strPassword, const QString& strType, WIZUSERINFO& ret)
{
    DEBUG_TOLOG(_T("Start Login"));
    //
    CWizKMBaseParam param;

    param.addString(_T("user_id"), strUserName);
    param.addString(_T("password"), makeXmlRpcPassword(strPassword));
    param.addString(_T("program_type"), strType);
//    param.AddString(_T("protocol"), "https");
    //
    if (!call(_T("accounts.clientLogin"), ret, &param))
    {
        TOLOG(_T("Failed to login!"));
        return FALSE;
    }
    //
    DEBUG_TOLOG1(_T("Login: token=%1"), ret.strToken);
    //
    return TRUE;
}

BOOL WizKMAccountsServer::accounts_createAccount(const QString& strUserName, const QString& strPassword,
                                                  const QString& strInviteCode, const QString& strCaptchaID, const QString& strCaptcha)
{
    CWizKMBaseParam param;

    param.addString(_T("user_id"), strUserName);
    param.addString(_T("password"), makeXmlRpcPassword(strPassword));
    param.addString(_T("invite_code"), strInviteCode);
    param.addString(_T("product_name"), "WizNoteQT");
    if (!strCaptchaID.isEmpty())
    {
        param.addString(_T("captcha_id"), strCaptchaID);
        param.addString(_T("captcha"), strCaptcha);
    }
    //
    WizXmlRpcResult ret;
    if (!call(_T("accounts.createAccount"), ret, &param))
    {
        TOLOG(_T("Failed to create account!"));
        return FALSE;
    }
    //
    return TRUE;
}

BOOL WizKMAccountsServer::accounts_clientLogout(const QString& strToken)
{
    CWizKMTokenOnlyParam param(strToken, getKbGuid());
    //
    WizXmlRpcResult callRet;
    if (!call(_T("accounts.clientLogout"), callRet, &param))
    {
        TOLOG(_T("Logout failure!"));
        return FALSE;
    }
    //
    return TRUE;
}
BOOL WizKMAccountsServer::accounts_keepAlive(const QString& strToken)
{
    qDebug() << "keepAlive: " << strToken << "kb: " << getKbGuid();
    CWizKMTokenOnlyParam param(strToken, getKbGuid());

    WizXmlRpcResult callRet;
    if (!call(_T("accounts.keepAlive"), callRet, &param))
    {
        TOLOG(_T("Keep alive failure!"));
        return FALSE;
    }
    //
    return TRUE;
}



BOOL WizKMAccountsServer::accounts_changePassword(const QString& strUserName, const QString& strOldPassword, const QString& strNewPassword)
{
    CWizKMBaseParam param;

    param.addString(_T("user_id"), strUserName);
    param.addString(_T("old_password"), makeXmlRpcPassword(strOldPassword));
    param.addString(_T("new_password"), makeXmlRpcPassword(strNewPassword));
    //
    WizXmlRpcResult callRet;
    if (!call(_T("accounts.changePassword"), callRet, &param))
    {
        TOLOG(_T("Change password failure!"));
        return FALSE;
    }
    //
    return TRUE;
}

BOOL WizKMAccountsServer::accounts_changeUserId(const QString& strUserName, const QString& strPassword, const QString& strNewUserId)
{
    CWizKMBaseParam param;

    param.addString(_T("user_id"), strUserName);
    param.addString(_T("password"), makeXmlRpcPassword(strPassword));
    param.addString(_T("new_user_id"), strNewUserId);
    //
    WizXmlRpcResult callRet;
    if (!call(_T("accounts.changeAccount"), callRet, &param))
    {
        TOLOG(_T("Change password failure!"));
        return FALSE;
    }
    //
    return TRUE;
}
BOOL WizKMAccountsServer::accounts_getToken(const QString& strUserName, const QString& strPassword, QString& strToken)
{
    CWizKMBaseParam param;

    param.addString(_T("user_id"), strUserName);
    param.addString(_T("password"), makeXmlRpcPassword(strPassword));

    //
    if (!call(_T("accounts.getToken"), _T("token"), strToken, &param))
    {
        TOLOG(_T("Failed to get token!"));
        return FALSE;
    }
    DEBUG_TOLOG1(_T("Get token: %1"), strToken);

    return TRUE;
}
BOOL WizKMAccountsServer::accounts_getCert(const QString& strUserName, const QString& strPassword, QString& strN, QString& stre, QString& strd, QString& strHint)
{
    CWizKMBaseParam param;

    param.addString(_T("user_id"), strUserName);
    param.addString(_T("password"), makeXmlRpcPassword(strPassword));
    //
    if (!call(_T("accounts.getCert"), _T("n"), strN, _T("e"), stre, _T("d"), strd, _T("hint"), strHint, &param))
    {
        TOLOG(_T("Failed to get cert!"));
        return FALSE;
    }
    //
    return TRUE;
}

BOOL WizKMAccountsServer::accounts_setCert(const QString& strUserName, const QString& strPassword, const QString& strN, const QString& stre, const QString& strd, const QString& strHint)
{
    CWizKMBaseParam param;

    param.addString(_T("user_id"), strUserName);
    param.addString(_T("password"), makeXmlRpcPassword(strPassword));
    param.addString(_T("n"), strN);
    param.addString(_T("e"), stre);
    param.addString(_T("d"), strd);
    param.addString(_T("hint"), strHint);
    //
    if (!call(_T("accounts.setCert"), &param))
    {
        TOLOG(_T("Failed to set cert!"));
        return FALSE;
    }
    //
    return TRUE;
}

BOOL WizKMAccountsServer::document_shareSNS(const QString& strToken, const QString& strSNS, const QString& strComment, const QString& strURL, const QString& strDocumentGUID)
{
    CWizKMTokenOnlyParam param(strToken, getKbGuid());
    param.addString(_T("sns_list"), strSNS);
    param.addString(_T("content"), strComment);
    param.addString(_T("url"), strURL);
    param.addString(_T("document_guid"), strDocumentGUID);
    //
    WizXmlRpcResult callRet;
    if (!call(_T("document.shareSNS"), callRet, &param))
    {
        TOLOG(_T("share note by sns failed!"));
        return FALSE;
    }
    //
    return TRUE;
}


BOOL WizKMAccountsServer::accounts_getGroupList(CWizGroupDataArray& arrayGroup)
{
    CWizKMTokenOnlyParam param(getToken(), getKbGuid());    
    //
    param.addString(_T("kb_type"), _T("group"));
//    param.AddString(_T("protocol"), "https");
    //
    std::deque<WIZGROUPDATA> arrayWrap;
    if (!call(_T("accounts.getGroupKbList"), arrayWrap, &param))
    {
        TOLOG(_T("accounts.getGroupKbList failure!"));
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
    if (!call(_T("accounts.getUserBizs"), arrayWrap, &param))
    {
        TOLOG(_T("accounts.getUserBizs failure!"));
        return FALSE;
    }
    //
    arrayBiz.assign(arrayWrap.begin(), arrayWrap.end());
    //
    return TRUE;
}

BOOL WizKMAccountsServer::accounts_createTempGroupKb(const QString& strEmails, const QString& strAccessControl, const QString& strSubject, const QString& strEmailText, WIZGROUPDATA& group)
{
    CWizKMTokenOnlyParam param(getToken(), getKbGuid());
    param.addString(_T("kb_name"), strSubject);
    param.addString(_T("user_ids"), strEmails);
    param.addString(_T("group_role"), strAccessControl);
    param.addString(_T("email_ext_text"), strEmailText);
    //
    WIZGROUPDATA wrap;
    if (!call(_T("accounts.createTempGroupKb"), wrap, &param))
    {
        TOLOG(_T("document.getGroupKbList failure!"));
        return FALSE;
    }
    //
    group.nUserGroup = 0;
    //
    group = wrap;
    //
    return TRUE;
}





BOOL WizKMAccountsServer::accounts_getMessagesByXmlrpc(int nCountPerPage, __int64 nVersion, CWizUserMessageDataArray& arrayMessage)
{
    CWizKMBaseParam param;

    param.addString(_T("token"), getToken());
    param.addString(_T("version"), WizInt64ToStr(nVersion));
    param.addInt(_T("count"), nCountPerPage);
    //
    std::deque<WIZUSERMESSAGEDATA> arrayWrap;
    if (!call(_T("accounts.getMessages"), arrayWrap, &param))
    {
        TOLOG(_T("document.getMessage failure!"));
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
    static int nCountPerPage = WizKMGetPrivateInt(_T("Sync"), _T("CountPerPage"), 200);
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
BOOL WizKMDatabaseServer::wiz_getInfo()
{
    CWizKMTokenOnlyParam param(m_userInfo.strToken, m_userInfo.strKbGUID);
    //
    if (!call(_T("wiz.getInfo"), m_kbInfo, &param))
    {
        TOLOG(_T("getInfo failure!"));
        return FALSE;
    }
    //
    return TRUE;
}

struct WIZOBJECTVERSION_XMLRPC : public WIZOBJECTVERSION
{
    BOOL loadFromXmlRpc(WizXmlRpcStructValue& data)
    {
        QString strType;
        //
        BOOL bRet = data.getInt64(_T("document_version"), nDocumentVersion)
            && data.getInt64(_T("tag_version"), nTagVersion)
            && data.getInt64(_T("style_version"), nStyleVersion)
            && data.getInt64(_T("attachment_version"), nAttachmentVersion)
            && data.getInt64(_T("deleted_version"), nDeletedGUIDVersion);
        //
        return bRet;
    }
};

BOOL WizKMDatabaseServer::wiz_getVersion(WIZOBJECTVERSION& version, BOOL bAuto)
{
    CWizKMTokenOnlyParam param(m_userInfo.strToken, m_userInfo.strKbGUID);
    //
    param.addBool(_T("auto"), bAuto);
    //
    WIZOBJECTVERSION_XMLRPC wrap;
    if (!call(_T("wiz.getVersion"), wrap, &param))
    {
        TOLOG(_T("GetVersion failure!"));
        return FALSE;
    }
    //
    version = wrap;
    //
    return TRUE;
}
//

BOOL WizKMDatabaseServer::getValueVersion(const QString& strKey, __int64& nVersion)
{
    return WizKMXmlRpcServerBase::getValueVersion(_T("kb"), getToken(), getKbGuid(), strKey, nVersion);
}
BOOL WizKMDatabaseServer::getValue(const QString& strKey, QString& strValue, __int64& nVersion)
{
    return WizKMXmlRpcServerBase::getValue(_T("kb"), getToken(), getKbGuid(), strKey, strValue, nVersion);
}
BOOL WizKMDatabaseServer::setValue(const QString& strKey, const QString& strValue, __int64& nRetVersion)
{
    return WizKMXmlRpcServerBase::setValue(_T("kb"), getToken(), getKbGuid(), strKey, strValue, nRetVersion);
}

BOOL WizKMDatabaseServer::document_downloadData(const QString& strDocumentGUID, WIZDOCUMENTDATAEX& ret)
{
    if (!data_download(ret.strGUID, _T("document"), ret.arrayData, ret.strTitle))
    {
        TOLOG1(_T("Failed to download attachment data: %1"), ret.strTitle);
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

        addString(_T("document_guid"), strDocumentGUID);
        addString(_T("document_title"), infodata.strTitle);
        addString(_T("document_category"), infodata.strLocation);
        addString(_T("document_filename"), infodata.strName);
        addString(_T("document_seo"), infodata.strSEO);
        addString(_T("document_url"), infodata.strURL);
        addString(_T("document_author"), infodata.strAuthor);
        addString(_T("document_keywords"), infodata.strKeywords);
        addString(_T("document_type"), infodata.strType);
        addString(_T("document_owner"), infodata.strOwner);
        addString(_T("document_filetype"), infodata.strFileType);
        addString(_T("document_styleguid"), infodata.strStyleGUID);
        addTime(_T("dt_created"), infodata.tCreated);
        addTime(_T("dt_modified"), infodata.tModified);
        addTime(_T("dt_accessed"), infodata.tAccessed);
        addInt(_T("document_protected"), infodata.nProtected);
        addInt(_T("document_readcount"), infodata.nReadCount);
        addInt(_T("document_attachment_count"), infodata.nAttachmentCount);
        addTime(_T("dt_data_modified"), infodata.tDataModified);
        addString(_T("data_md5"), infodata.strDataMD5);
        addString("document_zip_md5", strObjMd5);
        //
        CString strTagGuids;
        ::WizStringArrayToText(tags, strTagGuids, "*");
        //
        addString(_T("document_tag_guids"), strTagGuids);
    }
};


BOOL WizKMDatabaseServer::attachment_downloadData(const QString& strAttachmentGUID, WIZDOCUMENTATTACHMENTDATAEX& ret)
{
    ATLASSERT(!ret.arrayData.isEmpty());
    if (!ret.arrayData.isEmpty())
    {
        TOLOG(_T("fault error: ret.arrayData is not null!"));
        return FALSE;
    }
    //
    if (!data_download(strAttachmentGUID, _T("attachment"), ret.arrayData, ret.strName))
    {
        TOLOG1(_T("Failed to download attachment data: %1"), ret.strName);
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
        addString(_T("attachment_guid"), strAttachmentGUID);
        addString(_T("attachment_document_guid"), infodata.strDocumentGUID);
        addString(_T("attachment_name"), infodata.strName);
        addString(_T("attachment_url"), infodata.strURL);
        addString(_T("attachment_description"), infodata.strDescription);
        addTime(_T("dt_info_modified"), infodata.tInfoModified);
        addString(_T("info_md5"), infodata.strInfoMD5);
        addTime(_T("dt_data_modified"), infodata.tDataModified);
        addString(_T("data_md5"), infodata.strDataMD5);
        //
        addTime(_T("dt_data_modified"), infodata.tDataModified);
        addString(_T("data_md5"), infodata.strDataMD5);
        addString(_T("attachment_zip_md5"), strObjMd5);
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
        addString(_T("obj_guid"), strObjectGUID);
        addString(_T("obj_type"), strObjectType);
        //
        addInt64(_T("start_pos"), pos);
        addInt64(_T("part_size"), size);
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
    BOOL loadFromXmlRpc(WizXmlRpcStructValue& data)
    {
        data.getInt64(_T("obj_size"), nObjectSize);
        data.getInt(_T("eof"), bEOF);
        data.getInt64(_T("part_size"), nPartSize);
        data.getString(_T("part_md5"), strPartMD5);
        return data.getStream(_T("data"), stream);
    }
};

BOOL WizKMDatabaseServer::data_download(const QString& strObjectGUID, const QString& strObjectType, int pos, int size, QByteArray& stream, int& nAllSize, BOOL& bEOF)
{
    CWizKMDataDownloadParam param(m_userInfo.strToken, m_userInfo.strKbGUID, strObjectGUID, strObjectType, pos, size);
    //
    WIZKMDATAPART part;
    if (!call(_T("data.download"), part, &param))
    {
        TOLOG(_T("data.download failure!"));
        return FALSE;
    }
    //
    __int64 nStreamSize = part.stream.size();
    if (part.nPartSize != nStreamSize)
    {
        TOLOG2(_T("part size does not match: stream_size=%1, part_size=%2"), WizInt64ToStr(nStreamSize), WizInt64ToStr(part.nPartSize));
        return FALSE;
    }
    //
    QString strStreamMD5 = WizMd5StringNoSpaceJava(part.stream);
    if (0 != strStreamMD5.compare(part.strPartMD5, Qt::CaseInsensitive))
    {
        TOLOG2(_T("part md5 does not match, stream_md5=%1, part_md5=%2"), strStreamMD5, part.strPartMD5);
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
        addString(_T("obj_guid"), strObjectGUID);
        addString(_T("obj_type"), strObjectType);
        addString(_T("obj_md5"), strObjectMD5);
        addInt(_T("obj_size"), allSize);
        addInt(_T("part_count"), partCount);
        addInt(_T("part_sn"), partIndex);
        addInt64(_T("part_size"), stream.size());
        addString(_T("part_md5"), ::WizMd5StringNoSpaceJava(stream));
        addBase64(_T("data"), stream);
    }
};



BOOL WizKMDatabaseServer::data_upload(const QString& strObjectGUID, const QString& strObjectType, const QString& strObjectMD5, int allSize, int partCount, int partIndex, int partSize, const QByteArray& stream)
{
    __int64 nStreamSize = stream.size();
    if (partSize != (int)nStreamSize)
    {
        TOLOG2(_T("Fault error: stream_size=%1, part_size=%2"), WizIntToStr(int(nStreamSize)), WizIntToStr(partSize));
        return FALSE;
    }
    //
    CWizKMDataUploadParam param(m_userInfo.strToken, m_userInfo.strKbGUID, strObjectGUID, strObjectType, strObjectMD5, allSize, partCount, partIndex, stream);
    //
    if (!call(_T("data.upload"), &param))
    {
        TOLOG(_T("Can not upload object part data!"));
        return FALSE;
    }
    //
    return TRUE;
}


BOOL WizKMDatabaseServer::data_download(const QString& strObjectGUID, const QString& strObjectType, QByteArray& stream, const QString& strDisplayName)
{
    stream.clear();
    //
    int nAllSize = 0;
    int startPos = 0;
    while (1)
    {
        int partSize = 500 * 1000;
        //
        BOOL bEOF = FALSE;
        if (!data_download(strObjectGUID, strObjectType, startPos, partSize, stream, nAllSize, bEOF))
        {
            TOLOG(WizFormatString1(_T("Failed to download object part data: %1"), strDisplayName));
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
        TOLOG3(_T("Failed to download object data: %1, stream_size=%2, object_size=%3"), strDisplayName, WizInt64ToStr(nStreamSize), WizInt64ToStr(nAllSize));
        return FALSE;
    }
    //
    return TRUE;
}
BOOL WizKMDatabaseServer::data_upload(const QString& strObjectGUID, const QString& strObjectType, const QByteArray& stream, const QString& strObjMD5, const QString& strDisplayName)
{
    __int64 nStreamSize = stream.size();
    if (0 == nStreamSize)
    {
        TOLOG(_T("fault error: stream is zero"));
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
            TOLOG1(_T("Failed to upload part data: %1"), strDisplayName);
            return FALSE;
        }
    }
    //
    //
    return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////////
//
BOOL WizKMDatabaseServer::document_postData(const WIZDOCUMENTDATAEX& data, bool bWithDocumentData, __int64& nServerVersion)
{
    if (!data.arrayData.isEmpty() && data.arrayData.size() > m_kbInfo.getMaxFileSize())
    {
        TOLOG1(_T("%1 is too large, skip it"), data.strTitle);
        return FALSE;
    }
    //
    QString strObjMd5;
    //
    if (!data.arrayData.isEmpty() && bWithDocumentData)
    {
        strObjMd5 = WizMd5StringNoSpaceJava(data.arrayData);
        if (!data_upload(data.strGUID, _T("document"), data.arrayData, strObjMd5, data.strTitle))
        {
            TOLOG1(_T("Failed to upload note data: %1"), data.strTitle);
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
    if (!call(_T("document.postSimpleData"), ret, &param))
    {
        TOLOG(_T("document.postSimpleData failure!"));
        return FALSE;
    }
    //
    if (WizXmlRpcStructValue* pRet = ret.getResultValue<WizXmlRpcStructValue>())
    {
        pRet->getInt64(_T("version"), nServerVersion);
    }
    //
    return TRUE;
}


BOOL WizKMDatabaseServer::attachment_postData(WIZDOCUMENTATTACHMENTDATAEX& data, __int64& nServerVersion)
{
    if (data.arrayData.size() > m_kbInfo.getMaxFileSize())
    {
        TOLOG1(_T("%1 is too large, skip it"), data.strName);
        return TRUE;
    }
    //
    QString strObjMd5 = ::WizMd5StringNoSpaceJava(data.arrayData);
    //
    if (!data_upload(data.strGUID, _T("attachment"), data.arrayData, strObjMd5, data.strName))
    {
        TOLOG1(_T("Failed to upload attachment data: %1"), data.strName);
        return FALSE;
    }
    //
    CWizKMAttachmentPostDataParam param(WIZKM_WEBAPI_VERSION, m_userInfo.strToken, m_userInfo.strKbGUID, data.strGUID, data, strObjMd5);
    //
    WizXmlRpcResult ret;
    if (!call(_T("attachment.postSimpleData"), ret, &param))
    {
        TOLOG(_T("attachment.postSimpleData failure!"));
        return FALSE;
    }
    //
    if (WizXmlRpcStructValue* pRet = ret.getResultValue<WizXmlRpcStructValue>())
    {
        pRet->getInt64(_T("version"), nServerVersion);
    }
    //
    return TRUE;
}


BOOL WizKMDatabaseServer::document_getListByGuids(const CWizStdStringArray& arrayDocumentGUID, std::deque<WIZDOCUMENTDATAEX>& arrayRet)
{
   return downloadList<WIZDOCUMENTDATAEX, WIZDOCUMENTDATAEX>(_T("document.downloadInfoList"), _T("document_guids"), arrayDocumentGUID, arrayRet);
}

BOOL WizKMDatabaseServer::document_getInfo(const QString& strDocumentGuid, WIZDOCUMENTDATAEX& doc)
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


BOOL WizKMDatabaseServer::document_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDOCUMENTDATAEX>& arrayRet)
{
    return getList<WIZDOCUMENTDATAEX, WIZDOCUMENTDATAEX>(_T("document.getSimpleList"), nCountPerPage, nVersion, arrayRet);
}

BOOL WizKMDatabaseServer::attachment_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
{
    return getList<WIZDOCUMENTATTACHMENTDATAEX, WIZDOCUMENTATTACHMENTDATAEX>(_T("attachment.getList"), nCountPerPage, nVersion, arrayRet);
}

BOOL WizKMDatabaseServer::tag_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZTAGDATA>& arrayRet)
{
    return getList<WIZTAGDATA, WIZTAGDATA>(_T("tag.getList"), nCountPerPage, nVersion, arrayRet);
}

BOOL WizKMDatabaseServer::style_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZSTYLEDATA>& arrayRet)
{
    return getList<WIZSTYLEDATA, WIZSTYLEDATA>(_T("style.getList"), nCountPerPage, nVersion, arrayRet);
}


BOOL WizKMDatabaseServer::deleted_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDELETEDGUIDDATA>& arrayRet)
{
    return getList<WIZDELETEDGUIDDATA, WIZDELETEDGUIDDATA>(_T("deleted.getList"), nCountPerPage, nVersion, arrayRet);
}
BOOL WizKMDatabaseServer::tag_postList(std::deque<WIZTAGDATA>& arrayTag)
{
    return postList<WIZTAGDATA, WIZTAGDATA>(_T("tag.postList"), _T("tags"), arrayTag);
}
BOOL WizKMDatabaseServer::style_postList(std::deque<WIZSTYLEDATA>& arrayStyle)
{
    return postList<WIZSTYLEDATA, WIZSTYLEDATA>(_T("style.postList"), _T("styles"), arrayStyle);
}
BOOL WizKMDatabaseServer::deleted_postList(std::deque<WIZDELETEDGUIDDATA>& arrayDeletedGUID)
{
    return postList<WIZDELETEDGUIDDATA, WIZDELETEDGUIDDATA>(_T("deleted.postList"), _T("deleteds"), arrayDeletedGUID);
}

BOOL WizKMDatabaseServer::category_getAll(QString& str)
{
    CWizKMTokenOnlyParam param(m_userInfo.strToken, m_userInfo.strKbGUID);
    //
    if (!call(_T("category.getAll"), _T("categories"), str, &param))
    {
        TOLOG(_T("category.getList failure!"));
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
        TOLOG1(_T("Failed to download note data: %1"), strDocumentGUID);
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
        TOLOG1(_T("Failed to download attachment data: %1"), strAttachmentGUID);
        return QByteArray();
    }
    //
    if (ret.arrayData.isEmpty())
        return QByteArray();
    //
    return ret.arrayData;
}
