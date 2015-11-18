#include "wizKMServer.h"
#include "apientry.h"
#include "token.h"
#include "rapidjson/document.h"

#define WIZUSERMESSAGE_AT		0
#define WIZUSERMESSAGE_EDIT		1



CWizKMXmlRpcServerBase::CWizKMXmlRpcServerBase(const QString& strUrl, QObject* parent)
    : CWizXmlRpcServerBase(strUrl, parent)
{
}

BOOL CWizKMXmlRpcServerBase::GetValueVersion(const QString& strMethodPrefix, const QString& strToken, const QString& strKbGUID, const QString& strKey, __int64& nVersion)
{
    CWizKMTokenOnlyParam param(strToken, strKbGUID);
    //
    param.AddString(_T("key"), strKey);
    //
    QString strVersion;
    //
    if (!Call(QString(strMethodPrefix) + _T(".getValueVersion"), _T("version"), strVersion, &param))
    {
        TOLOG1(_T("Failed to get value version: key=%1"), strKey);
        return FALSE;
    }
    //
    nVersion = _ttoi64(strVersion);
    //
    return TRUE;
}
BOOL CWizKMXmlRpcServerBase::GetValue(const QString& strMethodPrefix, const QString& strToken, const QString& strKbGUID, const QString& strKey, QString& strValue, __int64& nVersion)
{
    CWizKMTokenOnlyParam param(strToken, strKbGUID);
    //
    param.AddString(_T("key"), strKey);
    //
    QString strVersion;
    //
    if (!Call(QString(strMethodPrefix) + _T(".getValue"),  _T("value_of_key"), strValue, _T("version"), strVersion, &param))
    {
        TOLOG1(_T("Failed to get value: key=%1"), strKey);
        return FALSE;
    }
    //
    nVersion = _ttoi64(strVersion);
    //
    return TRUE;
}
BOOL CWizKMXmlRpcServerBase::SetValue(const QString& strMethodPrefix, const QString& strToken, const QString& strKbGUID, const QString& strKey, const QString& strValue, __int64& nRetVersion)
{
    CWizKMTokenOnlyParam param(strToken, strKbGUID);
    //
    param.AddString(_T("key"), strKey);
    param.AddString(_T("value_of_key"), strValue);
    //
    QString strRetVersion;
    //
    if (!Call(QString(strMethodPrefix) + _T(".setValue"), _T("version"), strRetVersion, &param))
    {
        TOLOG1(_T("Failed to set value: key=%1"), strKey);
        return FALSE;
    }
    //
    nRetVersion = _ttoi64(strRetVersion);
    //
    return TRUE;
}


CWizKMAccountsServer::CWizKMAccountsServer(const QString& strUrl, QObject* parent)
    : CWizKMXmlRpcServerBase(strUrl, parent)
{
    m_bLogin = FALSE;
    m_bAutoLogout = FALSE;
}

CWizKMAccountsServer::~CWizKMAccountsServer(void)
{
    if (m_bAutoLogout)
    {
        Logout();
    }
}

void CWizKMAccountsServer::SetUserInfo(const WIZUSERINFO& userInfo)
{
    m_bLogin = TRUE;
    m_retLogin = userInfo;
}

void CWizKMAccountsServer::OnXmlRpcError()
{
}

BOOL CWizKMAccountsServer::Login(const QString& strUserName, const QString& strPassword, const QString& strType)
{
    if (m_bLogin)
    {
        return TRUE;
    }
    //
    m_bLogin = accounts_clientLogin(strUserName, strPassword, strType, m_retLogin);
    //
    return m_bLogin;
}
BOOL CWizKMAccountsServer::Logout()
{
    if (!m_bLogin)
        return FALSE;
    if (m_retLogin.strToken.isEmpty())
        return FALSE;
    //
    m_bLogin = FALSE;
    BOOL bRet = accounts_clientLogout(m_retLogin.strToken);
    m_retLogin.strToken.clear();
    return bRet;
}


BOOL CWizKMAccountsServer::ChangePassword(const QString& strUserName, const QString& strOldPassword, const QString& strNewPassword)
{
    return accounts_changePassword(strUserName, strOldPassword, strNewPassword);
}

BOOL CWizKMAccountsServer::ChangeUserId(const QString& strUserName, const QString& strPassword, const QString& strNewUserId)
{
    return accounts_changeUserId(strUserName, strPassword, strNewUserId);
}

BOOL CWizKMAccountsServer::CreateAccount(const QString& strUserName, const QString& strPassword, const QString& strInviteCode, const QString& strCaptchaID, const QString& strCaptcha)
{
    return accounts_createAccount(strUserName, strPassword, strInviteCode, strCaptchaID, strCaptcha);
}

BOOL CWizKMAccountsServer::GetToken(const QString& strUserName, const QString& strPassword, QString& strToken)
{
    return accounts_getToken(strUserName, strPassword, strToken);
}
BOOL CWizKMAccountsServer::GetCert(const QString& strUserName, const QString& strPassword, QString& strN, QString& stre, QString& strd, QString& strHint)
{
    return accounts_getCert(strUserName, strPassword, strN, stre, strd, strHint);
}
BOOL CWizKMAccountsServer::SetCert(const QString& strUserName, const QString& strPassword, const QString& strN, const QString& stre, const QString& strd, const QString& strHint)
{
    return accounts_setCert(strUserName, strPassword, strN, stre, strd, strHint);
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

QString CWizKMAccountsServer::GetToken()
{
    if (!m_bLogin)
        return QString();
    //
    return m_retLogin.strToken;
}
QString CWizKMAccountsServer::GetKbGUID()
{
    if (!m_bLogin)
        return QString();
    //
    return m_retLogin.strKbGUID;
}

BOOL CWizKMAccountsServer::ShareSNS(const QString& strToken, const QString& strSNS, const QString& strComment, const QString& strURL, const QString& strDocumentGUID)
{
    return document_shareSNS(strToken, strSNS, strComment, strURL, strDocumentGUID);
}

BOOL CWizKMAccountsServer::GetGroupList(CWizGroupDataArray& arrayGroup)
{
    return accounts_getGroupList(arrayGroup);
}

BOOL CWizKMAccountsServer::GetBizList(CWizBizDataArray& arrayBiz)
{
    return accounts_getBizList(arrayBiz);
}

BOOL CWizKMAccountsServer::CreateTempGroup(const QString& strEmails, const QString& strAccessControl, const QString& strSubject, const QString& strEmailText, WIZGROUPDATA& group)
{
    return accounts_createTempGroupKb(strEmails, strAccessControl, strSubject, strEmailText, group);
}

BOOL CWizKMAccountsServer::KeepAlive(const QString& strToken)
{
    return accounts_keepAlive(strToken);
}
BOOL CWizKMAccountsServer::GetMessages(__int64 nVersion, CWizUserMessageDataArray& arrayRet)
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

BOOL CWizKMAccountsServer::SetMessageReadStatus(const QString& strMessageIDs, int nStatus)
{
    CWizKMBaseParam param;

    param.AddString(_T("token"), GetToken());
    param.AddString(_T("ids"), strMessageIDs);
    param.AddInt(_T("status"), nStatus);
    //
    if (!Call(_T("accounts.setReadStatus"), &param))
    {
        TOLOG(_T("accounts.setReadStatus failure!"));
        return FALSE;
    }
    //
    return TRUE;
}

bool CWizKMAccountsServer::SetMessageDeleteStatus(const QString& strMessageIDs, int nStatus)
{
    QString strUrl = WizService::CommonApiEntry::messageServerUrl();
    strUrl += QString("/messages?token=%1&ids=%2").arg(m_retLogin.strToken).arg(strMessageIDs);
    qDebug() << "set message delete status, strken:" << m_retLogin.strToken << "   ids : " << strMessageIDs << " url : " << strUrl;
    //
    return deleteResource(strUrl);
}

BOOL CWizKMAccountsServer::GetValueVersion(const QString& strKey, __int64& nVersion)
{
    return CWizKMXmlRpcServerBase::GetValueVersion(_T("accounts"), GetToken(), GetKbGUID(), strKey, nVersion);
}
BOOL CWizKMAccountsServer::GetValue(const QString& strKey, QString& strValue, __int64& nVersion)
{
    return CWizKMXmlRpcServerBase::GetValue(_T("accounts"), GetToken(), GetKbGUID(), strKey, strValue, nVersion);
}
BOOL CWizKMAccountsServer::SetValue(const QString& strKey, const QString& strValue, __int64& nRetVersion)
{
    return CWizKMXmlRpcServerBase::SetValue(_T("accounts"), GetToken(), GetKbGUID(), strKey, strValue, nRetVersion);
}



QString CWizKMAccountsServer::MakeXmlRpcPassword(const QString& strPassword)
{
    if (strPassword.startsWith(_T("md5.")))
        return QString(strPassword);
    //
    return QString(_T("md5.")) + ::WizMd5StringNoSpaceJava(strPassword.toUtf8());
}




BOOL CWizKMAccountsServer::accounts_clientLogin(const QString& strUserName, const QString& strPassword, const QString& strType, WIZUSERINFO& ret)
{
    DEBUG_TOLOG(_T("Start Login"));
    //
    CWizKMBaseParam param;

    param.AddString(_T("user_id"), strUserName);
    param.AddString(_T("password"), MakeXmlRpcPassword(strPassword));
    param.AddString(_T("program_type"), strType);
//    param.AddString(_T("protocol"), "https");
    //
    if (!Call(_T("accounts.clientLogin"), ret, &param))
    {
        TOLOG(_T("Failed to login!"));
        return FALSE;
    }
    //
    DEBUG_TOLOG1(_T("Login: token=%1"), ret.strToken);
    //
    return TRUE;
}

BOOL CWizKMAccountsServer::accounts_createAccount(const QString& strUserName, const QString& strPassword,
                                                  const QString& strInviteCode, const QString& strCaptchaID, const QString& strCaptcha)
{
    CWizKMBaseParam param;

    param.AddString(_T("user_id"), strUserName);
    param.AddString(_T("password"), MakeXmlRpcPassword(strPassword));
    param.AddString(_T("invite_code"), strInviteCode);
    param.AddString(_T("product_name"), "WizNoteQT");
    if (!strCaptchaID.isEmpty())
    {
        param.AddString(_T("captcha_id"), strCaptchaID);
        param.AddString(_T("captcha"), strCaptcha);
    }
    //
    CWizXmlRpcResult ret;
    if (!Call(_T("accounts.createAccount"), ret, &param))
    {
        TOLOG(_T("Failed to create account!"));
        return FALSE;
    }
    //
    return TRUE;
}

BOOL CWizKMAccountsServer::accounts_clientLogout(const QString& strToken)
{
    CWizKMTokenOnlyParam param(strToken, GetKbGUID());
    //
    CWizXmlRpcResult callRet;
    if (!Call(_T("accounts.clientLogout"), callRet, &param))
    {
        TOLOG(_T("Logout failure!"));
        return FALSE;
    }
    //
    return TRUE;
}
BOOL CWizKMAccountsServer::accounts_keepAlive(const QString& strToken)
{
    qDebug() << "keepAlive: " << strToken << "kb: " << GetKbGUID();
    CWizKMTokenOnlyParam param(strToken, GetKbGUID());

    CWizXmlRpcResult callRet;
    if (!Call(_T("accounts.keepAlive"), callRet, &param))
    {
        TOLOG(_T("Keep alive failure!"));
        return FALSE;
    }
    //
    return TRUE;
}



BOOL CWizKMAccountsServer::accounts_changePassword(const QString& strUserName, const QString& strOldPassword, const QString& strNewPassword)
{
    CWizKMBaseParam param;

    param.AddString(_T("user_id"), strUserName);
    param.AddString(_T("old_password"), MakeXmlRpcPassword(strOldPassword));
    param.AddString(_T("new_password"), MakeXmlRpcPassword(strNewPassword));
    //
    CWizXmlRpcResult callRet;
    if (!Call(_T("accounts.changePassword"), callRet, &param))
    {
        TOLOG(_T("Change password failure!"));
        return FALSE;
    }
    //
    return TRUE;
}

BOOL CWizKMAccountsServer::accounts_changeUserId(const QString& strUserName, const QString& strPassword, const QString& strNewUserId)
{
    CWizKMBaseParam param;

    param.AddString(_T("user_id"), strUserName);
    param.AddString(_T("password"), MakeXmlRpcPassword(strPassword));
    param.AddString(_T("new_user_id"), strNewUserId);
    //
    CWizXmlRpcResult callRet;
    if (!Call(_T("accounts.changeAccount"), callRet, &param))
    {
        TOLOG(_T("Change password failure!"));
        return FALSE;
    }
    //
    return TRUE;
}
BOOL CWizKMAccountsServer::accounts_getToken(const QString& strUserName, const QString& strPassword, QString& strToken)
{
    CWizKMBaseParam param;

    param.AddString(_T("user_id"), strUserName);
    param.AddString(_T("password"), MakeXmlRpcPassword(strPassword));

    //
    if (!Call(_T("accounts.getToken"), _T("token"), strToken, &param))
    {
        TOLOG(_T("Failed to get token!"));
        return FALSE;
    }
    DEBUG_TOLOG1(_T("Get token: %1"), strToken);

    return TRUE;
}
BOOL CWizKMAccountsServer::accounts_getCert(const QString& strUserName, const QString& strPassword, QString& strN, QString& stre, QString& strd, QString& strHint)
{
    CWizKMBaseParam param;

    param.AddString(_T("user_id"), strUserName);
    param.AddString(_T("password"), MakeXmlRpcPassword(strPassword));
    //
    if (!Call(_T("accounts.getCert"), _T("n"), strN, _T("e"), stre, _T("d"), strd, _T("hint"), strHint, &param))
    {
        TOLOG(_T("Failed to get cert!"));
        return FALSE;
    }
    //
    return TRUE;
}

BOOL CWizKMAccountsServer::accounts_setCert(const QString& strUserName, const QString& strPassword, const QString& strN, const QString& stre, const QString& strd, const QString& strHint)
{
    CWizKMBaseParam param;

    param.AddString(_T("user_id"), strUserName);
    param.AddString(_T("password"), MakeXmlRpcPassword(strPassword));
    param.AddString(_T("n"), strN);
    param.AddString(_T("e"), stre);
    param.AddString(_T("d"), strd);
    param.AddString(_T("hint"), strHint);
    //
    if (!Call(_T("accounts.setCert"), &param))
    {
        TOLOG(_T("Failed to set cert!"));
        return FALSE;
    }
    //
    return TRUE;
}

BOOL CWizKMAccountsServer::document_shareSNS(const QString& strToken, const QString& strSNS, const QString& strComment, const QString& strURL, const QString& strDocumentGUID)
{
    CWizKMTokenOnlyParam param(strToken, GetKbGUID());
    param.AddString(_T("sns_list"), strSNS);
    param.AddString(_T("content"), strComment);
    param.AddString(_T("url"), strURL);
    param.AddString(_T("document_guid"), strDocumentGUID);
    //
    CWizXmlRpcResult callRet;
    if (!Call(_T("document.shareSNS"), callRet, &param))
    {
        TOLOG(_T("share note by sns failed!"));
        return FALSE;
    }
    //
    return TRUE;
}


BOOL CWizKMAccountsServer::accounts_getGroupList(CWizGroupDataArray& arrayGroup)
{
    CWizKMTokenOnlyParam param(GetToken(), GetKbGUID());    
    //
    param.AddString(_T("kb_type"), _T("group"));
//    param.AddString(_T("protocol"), "https");
    //
    std::deque<WIZGROUPDATA> arrayWrap;
    if (!Call(_T("accounts.getGroupKbList"), arrayWrap, &param))
    {
        TOLOG(_T("accounts.getGroupKbList failure!"));
        return FALSE;
    }
    //
    arrayGroup.assign(arrayWrap.begin(), arrayWrap.end());
    //
    return TRUE;
}
bool CWizKMAccountsServer::accounts_getBizList(CWizBizDataArray& arrayBiz)
{
    CWizKMTokenOnlyParam param(GetToken(), GetKbGUID());    
    //
    std::deque<WIZBIZDATA> arrayWrap;
    if (!Call(_T("accounts.getUserBizs"), arrayWrap, &param))
    {
        TOLOG(_T("accounts.getUserBizs failure!"));
        return FALSE;
    }
    //
    arrayBiz.assign(arrayWrap.begin(), arrayWrap.end());
    //
    return TRUE;
}

BOOL CWizKMAccountsServer::accounts_createTempGroupKb(const QString& strEmails, const QString& strAccessControl, const QString& strSubject, const QString& strEmailText, WIZGROUPDATA& group)
{
    CWizKMTokenOnlyParam param(GetToken(), GetKbGUID());
    param.AddString(_T("kb_name"), strSubject);
    param.AddString(_T("user_ids"), strEmails);
    param.AddString(_T("group_role"), strAccessControl);
    param.AddString(_T("email_ext_text"), strEmailText);
    //
    WIZGROUPDATA wrap;
    if (!Call(_T("accounts.createTempGroupKb"), wrap, &param))
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





BOOL CWizKMAccountsServer::accounts_getMessagesByXmlrpc(int nCountPerPage, __int64 nVersion, CWizUserMessageDataArray& arrayMessage)
{
    CWizKMBaseParam param;

    param.AddString(_T("token"), GetToken());
    param.AddString(_T("version"), WizInt64ToStr(nVersion));
    param.AddInt(_T("count"), nCountPerPage);
    //
    std::deque<WIZUSERMESSAGEDATA> arrayWrap;
    if (!Call(_T("accounts.getMessages"), arrayWrap, &param))
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
    if (!u.FindMember(memberName.toUtf8().constData()))
        return QString();

    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QTextDecoder* encoder = codec->makeDecoder();
    return encoder->toUnicode(u[memberName.toUtf8().constData()].GetString(), u[memberName.toUtf8().constData()].GetStringLength());
}

bool CWizKMAccountsServer::accounts_getMessagesByJson(int nCountPerPage, __int64 nVersion, CWizUserMessageDataArray& arrayMessage)
{
    QString strUrl = WizService::CommonApiEntry::messageServerUrl();
    strUrl += QString("/messages?token=%1&page_size=%2&version=%3&api_version=6").arg(m_retLogin.strToken).arg(nCountPerPage).arg(nVersion);
    //
    QString strResult;
    if (!get(strUrl, strResult))
        return false;

    rapidjson::Document d;
    d.Parse<0>(strResult.toUtf8().constData());

    if (d.HasParseError() || !d.FindMember("result")) {
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
        data.tCreated = COleDateTime(dateCreated);

        arrayMessage.push_back(data);
    }
    return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






CWizKMDatabaseServer::CWizKMDatabaseServer(const WIZUSERINFOBASE& kbInfo, QObject* parent)
    : CWizKMXmlRpcServerBase(kbInfo.strDatabaseServer, parent)
    , m_kbInfo(kbInfo)
{
}
CWizKMDatabaseServer::~CWizKMDatabaseServer()
{
}
void CWizKMDatabaseServer::OnXmlRpcError()
{
}
int CWizKMDatabaseServer::GetCountPerPage()
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
BOOL CWizKMDatabaseServer::wiz_getInfo(WIZKBINFO& info)
{
    CWizKMTokenOnlyParam param(m_kbInfo.strToken, m_kbInfo.strKbGUID);
    //
    if (!Call(_T("wiz.getInfo"), info, &param))
    {
        TOLOG(_T("getInfo failure!"));
        return FALSE;
    }
    //
    return TRUE;
}

struct WIZOBJECTVERSION_XMLRPC : public WIZOBJECTVERSION
{
    BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data)
    {
        QString strType;
        //
        BOOL bRet = data.GetInt64(_T("document_version"), nDocumentVersion)
            && data.GetInt64(_T("tag_version"), nTagVersion)
            && data.GetInt64(_T("style_version"), nStyleVersion)
            && data.GetInt64(_T("attachment_version"), nAttachmentVersion)
            && data.GetInt64(_T("deleted_version"), nDeletedGUIDVersion);
        //
        return bRet;
    }
};

BOOL CWizKMDatabaseServer::wiz_getVersion(WIZOBJECTVERSION& version, BOOL bAuto)
{
    CWizKMTokenOnlyParam param(m_kbInfo.strToken, m_kbInfo.strKbGUID);
    //
    param.AddBool(_T("auto"), bAuto);
    //
    WIZOBJECTVERSION_XMLRPC wrap;
    if (!Call(_T("wiz.getVersion"), wrap, &param))
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

BOOL CWizKMDatabaseServer::GetValueVersion(const QString& strKey, __int64& nVersion)
{
    return CWizKMXmlRpcServerBase::GetValueVersion(_T("kb"), GetToken(), GetKbGUID(), strKey, nVersion);
}
BOOL CWizKMDatabaseServer::GetValue(const QString& strKey, QString& strValue, __int64& nVersion)
{
    return CWizKMXmlRpcServerBase::GetValue(_T("kb"), GetToken(), GetKbGUID(), strKey, strValue, nVersion);
}
BOOL CWizKMDatabaseServer::SetValue(const QString& strKey, const QString& strValue, __int64& nRetVersion)
{
    return CWizKMXmlRpcServerBase::SetValue(_T("kb"), GetToken(), GetKbGUID(), strKey, strValue, nRetVersion);
}

struct WIZDOCUMENTDATAEX_XMLRPC_FULL
    : public WIZDOCUMENTDATAEX
{
    int nApiVersion;
    //
    UINT nParts;
    //
    int nDataSize;
    //
    WIZDOCUMENTDATAEX_XMLRPC_FULL()
    {
        nApiVersion = WIZKM_WEBAPI_VERSION;
        nParts = 0;
        nDataSize = 0;
    }
    //
    WIZDOCUMENTDATAEX_XMLRPC_FULL(WIZDOCUMENTDATAEX& data)
        : WIZDOCUMENTDATAEX(data)
    {
        nApiVersion = WIZKM_WEBAPI_VERSION;
        nParts = 0;
        nDataSize = 0;
    }
    //
    BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data)
    {
        BOOL bInfo = FALSE;
        BOOL bData = FALSE;
        BOOL bParam = FALSE;
        data.GetBool(_T("document_info"), bInfo);
        data.GetBool(_T("document_data"), bData);
        data.GetBool(_T("document_param"), bParam);
        data.GetInt64(_T("version"), nVersion);
        //
        nParts = 0;
        nParts |= bInfo ? WIZKM_XMKRPC_DOCUMENT_PART_INFO : 0;
        nParts |= bData ? WIZKM_XMKRPC_DOCUMENT_PART_DATA : 0;
        nParts |= bParam ? WIZKM_XMKRPC_DOCUMENT_PART_PARAM : 0;
        //
        data.GetStr(_T("document_guid"), strGUID);
        //
        if (bInfo)
        {
            data.GetStr(_T("document_title"), strTitle);
            data.GetStr(_T("document_category"), strLocation);
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
            //data.GetInt(_T("document_readcount"), nReadCount);
            data.GetInt(_T("document_attachment_count"), nAttachmentCount);
            data.GetTime(_T("dt_info_modified"), tInfoModified);
            data.GetStr(_T("info_md5"), strInfoMD5);
            data.GetTime(_T("dt_data_modified"), tDataModified);
            data.GetStr(_T("data_md5"), strDataMD5);
            data.GetTime(_T("dt_param_modified"), tParamModified);
            data.GetStr(_T("param_md5"), strParamMD5);
            //
            data.GetStringArray(_T("document_tags"), arrayTagGUID);
        }
        //
        if (bData)
        {
            data.GetInt(_T("document_zip_size"), nDataSize);
            if (nApiVersion < 2)
            {
                if (!data.GetStream(_T("document_zip_data"), arrayData))
                {
                    TOLOG(_T("Failed to get note data!"));
                    return FALSE;
                }
            }
        }
        //
        if (bParam)
        {
            std::deque<WIZDOCUMENTPARAMDATA> params;
            if (!data.GetArray(_T("document_params"), params))
            {
                TOLOG(_T("Failed to get note param!"));
                return FALSE;
            }
            arrayParam.assign(params.begin(), params.end());
        }
        //
        return !strGUID.isEmpty();
    }
};


struct CWizKMDocumentGetDataParam : public CWizKMTokenOnlyParam
{
    CWizKMDocumentGetDataParam(int nApiVersion, const QString& strToken, const QString& strBookGUID, const QString& strDocumentGUID, UINT nParts)
        : CWizKMTokenOnlyParam(strToken, strBookGUID)
    {
        ChangeApiVersion(nApiVersion);
        //
        AddString(_T("document_guid"), strDocumentGUID);
        AddBool(_T("document_info"), (nParts & WIZKM_XMKRPC_DOCUMENT_PART_INFO) ? TRUE : FALSE);
        AddBool(_T("document_data"), (nParts & WIZKM_XMKRPC_DOCUMENT_PART_DATA) ? TRUE : FALSE);
        AddBool(_T("document_param"), (nParts & WIZKM_XMKRPC_DOCUMENT_PART_PARAM) ? TRUE : FALSE);
    }
};



struct CWizKMAttachmentGetDataParam : public CWizKMTokenOnlyParam
{
    CWizKMAttachmentGetDataParam(int nApiVersion, const QString& strToken, const QString& strBookGUID, const QString& strAttachmentGUID, UINT nParts)
        : CWizKMTokenOnlyParam(strToken, strBookGUID)
    {
        ChangeApiVersion(nApiVersion);
        //
        AddString(_T("attachment_guid"), strAttachmentGUID);
        AddBool(_T("attachment_info"), (nParts & WIZKM_XMKRPC_ATTACHMENT_PART_INFO) ? TRUE : FALSE);
        AddBool(_T("attachment_data"), (nParts & WIZKM_XMKRPC_ATTACHMENT_PART_DATA) ? TRUE : FALSE);
    }
};


BOOL CWizKMDatabaseServer::document_getData2(const QString& strDocumentGUID, UINT nParts, WIZDOCUMENTDATAEX& ret)
{
    WIZDOCUMENTDATAEX_XMLRPC_FULL wrap(ret);
    wrap.arrayData.clear();

    //
    CWizKMDocumentGetDataParam param(WIZKM_WEBAPI_VERSION, m_kbInfo.strToken, m_kbInfo.strKbGUID, strDocumentGUID, nParts);
    if (!Call(_T("document.getData"), wrap, &param))
    {
        TOLOG(_T("document.getData failure!"));
        return FALSE;
    }
    //
    ret = wrap;
    //
    BOOL bDownloadData = (nParts & WIZKM_XMKRPC_DOCUMENT_PART_DATA) ? TRUE : FALSE;
    //
    if (bDownloadData)
    {
        if (!data_download(ret.strGUID, _T("document"), ret.arrayData, ret.strTitle))
        {
            TOLOG1(_T("Failed to download attachment data: %1"), ret.strTitle);
            return FALSE;
        }
    }
    //
    return TRUE;
}
//

struct CWizKMDocumentPostDataParam
    : public CWizKMDocumentGetDataParam
{
    CWizKMDocumentPostDataParam(int nApiVersion, const QString& strToken, const QString& strBookGUID, const QString& strDocumentGUID, UINT nParts, const WIZDOCUMENTDATA& infodata, const CWizStdStringArray& tags, const std::deque<WIZDOCUMENTPARAMDATA>& params, const QString& strObjMd5)
        : CWizKMDocumentGetDataParam(nApiVersion, strToken, strBookGUID, strDocumentGUID, nParts)
    {
        CWizXmlRpcStructValue* pDocumentStruct = new CWizXmlRpcStructValue();
        AddStruct(_T("document"), pDocumentStruct);
        //
        pDocumentStruct->AddString(_T("document_guid"), strDocumentGUID);
        pDocumentStruct->AddBool(_T("document_info"), (nParts & WIZKM_XMKRPC_DOCUMENT_PART_INFO) ? TRUE : FALSE);
        pDocumentStruct->AddBool(_T("document_data"), (nParts & WIZKM_XMKRPC_DOCUMENT_PART_DATA) ? TRUE : FALSE);
        pDocumentStruct->AddBool(_T("document_param"), (nParts & WIZKM_XMKRPC_DOCUMENT_PART_PARAM) ? TRUE : FALSE);
        //
        BOOL bInfo = (nParts & WIZKM_XMKRPC_DOCUMENT_PART_INFO) ? TRUE : FALSE;
        BOOL bData = (nParts & WIZKM_XMKRPC_DOCUMENT_PART_DATA) ? TRUE : FALSE;
        BOOL bParam = (nParts & WIZKM_XMKRPC_DOCUMENT_PART_PARAM) ? TRUE : FALSE;
        //
        BOOL bParamInfoAdded = FALSE;
        BOOL bDataInfoAdded = FALSE;
        //
        if (bInfo)
        {
            pDocumentStruct->AddString(_T("document_title"), infodata.strTitle);
            pDocumentStruct->AddString(_T("document_category"), infodata.strLocation);
            pDocumentStruct->AddString(_T("document_filename"), infodata.strName);
            pDocumentStruct->AddString(_T("document_seo"), infodata.strSEO);
            pDocumentStruct->AddString(_T("document_url"), infodata.strURL);
            pDocumentStruct->AddString(_T("document_author"), infodata.strAuthor);
            pDocumentStruct->AddString(_T("document_keywords"), infodata.strKeywords);
            pDocumentStruct->AddString(_T("document_type"), infodata.strType);
            pDocumentStruct->AddString(_T("document_owner"), infodata.strOwner);
            pDocumentStruct->AddString(_T("document_filetype"), infodata.strFileType);
            pDocumentStruct->AddString(_T("document_styleguid"), infodata.strStyleGUID);
            pDocumentStruct->AddTime(_T("dt_created"), infodata.tCreated);
            pDocumentStruct->AddTime(_T("dt_modified"), infodata.tModified);
            pDocumentStruct->AddTime(_T("dt_accessed"), infodata.tAccessed);
            pDocumentStruct->AddInt(_T("document_iconindex"), infodata.nIconIndex);
            pDocumentStruct->AddInt(_T("document_protected"), infodata.nProtected);
            pDocumentStruct->AddInt(_T("document_readcount"), infodata.nReadCount);
            pDocumentStruct->AddInt(_T("document_attachment_count"), infodata.nAttachmentCount);
            pDocumentStruct->AddTime(_T("dt_info_modified"), infodata.tInfoModified);
            pDocumentStruct->AddString(_T("info_md5"), infodata.strInfoMD5);
            pDocumentStruct->AddTime(_T("dt_data_modified"), infodata.tDataModified);
            pDocumentStruct->AddString(_T("data_md5"), infodata.strDataMD5);
            pDocumentStruct->AddTime(_T("dt_param_modified"), infodata.tParamModified);
            pDocumentStruct->AddString(_T("param_md5"), infodata.strParamMD5);
            //
            bParamInfoAdded = TRUE;
            bDataInfoAdded = TRUE;
            //
            pDocumentStruct->AddStringArray(_T("document_tags"), tags);
        }
        if (bParam)
        {
            if (!bParamInfoAdded)
            {
                pDocumentStruct->AddTime(_T("dt_param_modified"), infodata.tParamModified);
                pDocumentStruct->AddString(_T("param_md5"), infodata.strParamMD5);
                bParamInfoAdded = TRUE;
            }
            //
            std::deque<WIZDOCUMENTPARAMDATA> arrParams;
            arrParams.assign(params.begin(), params.end());
            pDocumentStruct->AddArray(_T("document_params"), arrParams);
        }
        if (bData)
        {
            if (!bDataInfoAdded)
            {
                pDocumentStruct->AddTime(_T("dt_data_modified"), infodata.tDataModified);
                pDocumentStruct->AddString(_T("data_md5"), infodata.strDataMD5);
                bDataInfoAdded = TRUE;
            }
            //
            pDocumentStruct->AddString(_T("document_zip_md5"), strObjMd5);
        }
    }
};



BOOL CWizKMDatabaseServer::document_postData2(const WIZDOCUMENTDATAEX& data, UINT nParts, __int64& nServerVersion)
{
    QString strObjMd5;
    //
    if (!data.arrayData.isEmpty() && (nParts & WIZKM_XMKRPC_DOCUMENT_PART_DATA))
    {
        strObjMd5 = WizMd5StringNoSpaceJava(data.arrayData);
        if (!data_upload(data.strGUID, _T("document"), data.arrayData, strObjMd5, data.strTitle))
        {
            TOLOG1(_T("Failed to upload note data: %1"), data.strTitle);
            return FALSE;
        }
    }
    //
    CWizKMDocumentPostDataParam param(WIZKM_WEBAPI_VERSION, m_kbInfo.strToken, m_kbInfo.strKbGUID, data.strGUID, nParts, data, data.arrayTagGUID, data.arrayParam, strObjMd5);
    //
    CWizXmlRpcResult ret;
    if (!Call(_T("document.postData"), ret, &param))
    {
        TOLOG(_T("document.postData failure!"));
        return FALSE;
    }
    //
    if (CWizXmlRpcStructValue* pRet = ret.GetResultValue<CWizXmlRpcStructValue>())
    {
        pRet->GetInt64(_T("version"), nServerVersion);
    }
    //
    return TRUE;
}

//


BOOL CWizKMDatabaseServer::attachment_getData2(const QString& strAttachmentGUID, UINT nParts, WIZDOCUMENTATTACHMENTDATAEX& ret)
{
    WIZDOCUMENTATTACHMENTDATAEX wrap(ret);
    //
    CWizKMAttachmentGetDataParam param(WIZKM_WEBAPI_VERSION, m_kbInfo.strToken, m_kbInfo.strKbGUID, strAttachmentGUID, nParts);
    //
    if (!Call(_T("attachment.getData"), wrap, &param))
    {
        TOLOG(_T("attachment.getData failure!"));
        return FALSE;
    }
    //
    ret = wrap;
    //
    if (nParts & WIZKM_XMKRPC_ATTACHMENT_PART_DATA)
    {
        ATLASSERT(!ret.arrayData.isEmpty());
        if (!ret.arrayData.isEmpty())
        {
            TOLOG(_T("fault error: ret.arrayData is null!"));
            return FALSE;
        }
        //
        if (!data_download(ret.strGUID, _T("attachment"), ret.arrayData, ret.strName))
        {
            TOLOG1(_T("Failed to download attachment data: %1"), ret.strName);
            return FALSE;
        }
    }
    //
    //
    return TRUE;
}

struct CWizKMAttachmentPostDataParam
    : public CWizKMAttachmentGetDataParam
{
    CWizKMAttachmentPostDataParam(int nApiVersion, const QString& strToken, const QString& strBookGUID, const QString& strAttachmentGUID, UINT nParts, const WIZDOCUMENTATTACHMENTDATA& infodata, const QString& strObjMd5)
        : CWizKMAttachmentGetDataParam(nApiVersion, strToken, strBookGUID, strAttachmentGUID, nParts)
    {
        CWizXmlRpcStructValue* pAttachmentStruct = new CWizXmlRpcStructValue();
        AddStruct(_T("attachment"), pAttachmentStruct);
        //
        pAttachmentStruct->AddString(_T("attachment_guid"), strAttachmentGUID);
        pAttachmentStruct->AddBool(_T("attachment_info"), (nParts & WIZKM_XMKRPC_ATTACHMENT_PART_INFO) ? TRUE : FALSE);
        pAttachmentStruct->AddBool(_T("attachment_data"), (nParts & WIZKM_XMKRPC_ATTACHMENT_PART_DATA) ? TRUE : FALSE);
        //
        BOOL bInfo = (nParts & WIZKM_XMKRPC_ATTACHMENT_PART_INFO) ? TRUE : FALSE;
        BOOL bData = (nParts & WIZKM_XMKRPC_ATTACHMENT_PART_DATA) ? TRUE : FALSE;
        //
        BOOL bDataInfoAdded = FALSE;
        //
        if (bInfo)
        {
            pAttachmentStruct->AddString(_T("attachment_document_guid"), infodata.strDocumentGUID);
            pAttachmentStruct->AddString(_T("attachment_name"), infodata.strName);
            pAttachmentStruct->AddString(_T("attachment_url"), infodata.strURL);
            pAttachmentStruct->AddString(_T("attachment_description"), infodata.strDescription);
            pAttachmentStruct->AddTime(_T("dt_info_modified"), infodata.tInfoModified);
            pAttachmentStruct->AddString(_T("info_md5"), infodata.strInfoMD5);
            pAttachmentStruct->AddTime(_T("dt_data_modified"), infodata.tDataModified);
            pAttachmentStruct->AddString(_T("data_md5"), infodata.strDataMD5);
            //
            bDataInfoAdded = TRUE;
        }
        if (bData)
        {
            if (!bDataInfoAdded)
            {
                pAttachmentStruct->AddTime(_T("dt_data_modified"), infodata.tDataModified);
                pAttachmentStruct->AddString(_T("data_md5"), infodata.strDataMD5);
                bDataInfoAdded = TRUE;
            }
            pAttachmentStruct->AddString(_T("attachment_zip_md5"), strObjMd5);
        }
    }
};

BOOL CWizKMDatabaseServer::attachment_postData2(WIZDOCUMENTATTACHMENTDATAEX& data, UINT nParts, __int64& nServerVersion)
{
    QString strObjMd5;
    //
    if (!data.arrayData.isEmpty() && (nParts & WIZKM_XMKRPC_ATTACHMENT_PART_DATA))
    {
        strObjMd5 = ::WizMd5StringNoSpaceJava(data.arrayData);
        //
        if (!data_upload(data.strGUID, _T("attachment"), data.arrayData, strObjMd5, data.strName))
        {
            TOLOG1(_T("Failed to upload attachment data: %1"), data.strName);
            return FALSE;
        }
    }
    //
    CWizKMAttachmentPostDataParam param(WIZKM_WEBAPI_VERSION, m_kbInfo.strToken, m_kbInfo.strKbGUID, data.strGUID, nParts, data, strObjMd5);
    //
    CWizXmlRpcResult ret;
    if (!Call(_T("attachment.postData"), ret, &param))
    {
        TOLOG(_T("attachment.postData2 failure!"));
        return FALSE;
    }
    //
    if (CWizXmlRpcStructValue* pRet = ret.GetResultValue<CWizXmlRpcStructValue>())
    {
        pRet->GetInt64(_T("version"), nServerVersion);
    }
    //
    return TRUE;
}


struct CWizKMDataDownloadParam
    : public CWizKMTokenOnlyParam
{
    CWizKMDataDownloadParam(const QString& strToken, const QString& strBookGUID, const QString& strObjectGUID, const QString& strObjectType, int pos, int size)
        : CWizKMTokenOnlyParam(strToken, strBookGUID)
    {
        AddString(_T("obj_guid"), strObjectGUID);
        AddString(_T("obj_type"), strObjectType);
        //
        AddInt64(_T("start_pos"), pos);
        AddInt64(_T("part_size"), size);
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
    BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data)
    {
        data.GetInt64(_T("obj_size"), nObjectSize);
        data.GetInt(_T("eof"), bEOF);
        data.GetInt64(_T("part_size"), nPartSize);
        data.GetString(_T("part_md5"), strPartMD5);
        return data.GetStream(_T("data"), stream);
    }
};

BOOL CWizKMDatabaseServer::data_download(const QString& strObjectGUID, const QString& strObjectType, int pos, int size, QByteArray& stream, int& nAllSize, BOOL& bEOF)
{
    CWizKMDataDownloadParam param(m_kbInfo.strToken, m_kbInfo.strKbGUID, strObjectGUID, strObjectType, pos, size);
    //
    WIZKMDATAPART part;
    if (!Call(_T("data.download"), part, &param))
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
        AddString(_T("obj_guid"), strObjectGUID);
        AddString(_T("obj_type"), strObjectType);
        AddString(_T("obj_md5"), strObjectMD5);
        AddInt(_T("obj_size"), allSize);
        AddInt(_T("part_count"), partCount);
        AddInt(_T("part_sn"), partIndex);
        AddInt64(_T("part_size"), stream.size());
        AddString(_T("part_md5"), ::WizMd5StringNoSpaceJava(stream));
        AddBase64(_T("data"), stream);
    }
};



BOOL CWizKMDatabaseServer::data_upload(const QString& strObjectGUID, const QString& strObjectType, const QString& strObjectMD5, int allSize, int partCount, int partIndex, int partSize, const QByteArray& stream)
{
    __int64 nStreamSize = stream.size();
    if (partSize != (int)nStreamSize)
    {
        TOLOG2(_T("Fault error: stream_size=%1, part_size=%2"), WizIntToStr(int(nStreamSize)), WizIntToStr(partSize));
        return FALSE;
    }
    //
    CWizKMDataUploadParam param(m_kbInfo.strToken, m_kbInfo.strKbGUID, strObjectGUID, strObjectType, strObjectMD5, allSize, partCount, partIndex, stream);
    //
    if (!Call(_T("data.upload"), &param))
    {
        TOLOG(_T("Can not upload object part data!"));
        return FALSE;
    }
    //
    return TRUE;
}


BOOL CWizKMDatabaseServer::data_download(const QString& strObjectGUID, const QString& strObjectType, QByteArray& stream, const QString& strDisplayName)
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
BOOL CWizKMDatabaseServer::data_upload(const QString& strObjectGUID, const QString& strObjectType, const QByteArray& stream, const QString& strObjMD5, const QString& strDisplayName)
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

BOOL CWizKMDatabaseServer::document_getData(const QString& strDocumentGUID, UINT nParts, WIZDOCUMENTDATAEX& ret)
{
    if (document_getData2(strDocumentGUID, nParts, ret))
        return TRUE;
    //
    return FALSE;//document_getData1(strDocumentGUID, nParts, ret);
}
BOOL CWizKMDatabaseServer::document_postData(const WIZDOCUMENTDATAEX& data, UINT nParts, __int64& nServerVersion)
{
    if (!data.arrayData.isEmpty() && data.arrayData.size() > m_kbInfo.GetMaxFileSize())
    {
        TOLOG1(_T("%1 is too large, skip it"), data.strTitle);
        return FALSE;
    }
    //
    BOOL bRet = document_postData2(data, nParts, nServerVersion);
    //
    return bRet;
}

BOOL CWizKMDatabaseServer::attachment_getData(const QString& strAttachmentGUID, UINT nParts, WIZDOCUMENTATTACHMENTDATAEX& ret)
{
    if (attachment_getData2(strAttachmentGUID, nParts, ret))
        return TRUE;
    //
    return FALSE;//attachment_getData1(strAttachmentGUID, nParts, ret);
}
BOOL CWizKMDatabaseServer::attachment_postData(WIZDOCUMENTATTACHMENTDATAEX& data, UINT nParts, __int64& nServerVersion)
{
    if (!data.arrayData.isEmpty() && data.arrayData.size() > m_kbInfo.GetMaxFileSize())
    {
        TOLOG1(_T("%1 is too large, skip it"), data.strName);
        return TRUE;
    }
    //
    if (attachment_postData2(data, nParts, nServerVersion))
        return TRUE;
    //
    return FALSE;
}




BOOL CWizKMDatabaseServer::document_downloadSimpleList(const CWizStdStringArray& arrayDocumentGUID, std::deque<WIZDOCUMENTDATAEX>& arrayRet)
{
    return downloadList<WIZDOCUMENTDATAEX, WIZDOCUMENTDATAEX_XMLRPC_SIMPLE>(_T("document.downloadList"), _T("document_guids"), arrayDocumentGUID, arrayRet);
}

BOOL CWizKMDatabaseServer::document_downloadFullList(const CWizStdStringArray& arrayDocumentGUID, std::deque<WIZDOCUMENTDATAEX>& arrayRet)
{
   return downloadList<WIZDOCUMENTDATAEX, WIZDOCUMENTDATAEX_XMLRPC_FULL>(_T("document.downloadInfoList"), _T("document_guids"), arrayDocumentGUID, arrayRet);
}

BOOL CWizKMDatabaseServer::attachment_downloadSimpleList(const CWizStdStringArray& arrayAttachmentGUID, std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
{
    return downloadList<WIZDOCUMENTATTACHMENTDATAEX, WIZDOCUMENTATTACHMENTDATAEX>(_T("attachment.downloadList"), _T("attachment_guids"), arrayAttachmentGUID, arrayRet);
}
//
BOOL CWizKMDatabaseServer::document_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDOCUMENTDATAEX>& arrayRet)
{
    return getList<WIZDOCUMENTDATAEX, WIZDOCUMENTDATAEX_XMLRPC_SIMPLE>(_T("document.getList"), nCountPerPage, nVersion, arrayRet);
}


BOOL CWizKMDatabaseServer::attachment_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
{
    return getList<WIZDOCUMENTATTACHMENTDATAEX, WIZDOCUMENTATTACHMENTDATAEX>(_T("attachment.getList"), nCountPerPage, nVersion, arrayRet);
}

BOOL CWizKMDatabaseServer::tag_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZTAGDATA>& arrayRet)
{
    return getList<WIZTAGDATA, WIZTAGDATA>(_T("tag.getList"), nCountPerPage, nVersion, arrayRet);
}

BOOL CWizKMDatabaseServer::style_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZSTYLEDATA>& arrayRet)
{
    return getList<WIZSTYLEDATA, WIZSTYLEDATA>(_T("style.getList"), nCountPerPage, nVersion, arrayRet);
}


BOOL CWizKMDatabaseServer::deleted_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDELETEDGUIDDATA>& arrayRet)
{
    return getList<WIZDELETEDGUIDDATA, WIZDELETEDGUIDDATA>(_T("deleted.getList"), nCountPerPage, nVersion, arrayRet);
}
BOOL CWizKMDatabaseServer::tag_postList(std::deque<WIZTAGDATA>& arrayTag)
{
    return postList<WIZTAGDATA, WIZTAGDATA>(_T("tag.postList"), _T("tags"), arrayTag);
}
BOOL CWizKMDatabaseServer::style_postList(std::deque<WIZSTYLEDATA>& arrayStyle)
{
    return postList<WIZSTYLEDATA, WIZSTYLEDATA>(_T("style.postList"), _T("styles"), arrayStyle);
}
BOOL CWizKMDatabaseServer::deleted_postList(std::deque<WIZDELETEDGUIDDATA>& arrayDeletedGUID)
{
    return postList<WIZDELETEDGUIDDATA, WIZDELETEDGUIDDATA>(_T("deleted.postList"), _T("deleteds"), arrayDeletedGUID);
}

BOOL CWizKMDatabaseServer::category_getAll(QString& str)
{
    CWizKMTokenOnlyParam param(m_kbInfo.strToken, m_kbInfo.strKbGUID);
    //
    if (!Call(_T("category.getAll"), _T("categories"), str, &param))
    {
        TOLOG(_T("category.getList failure!"));
        return FALSE;
    }
    //
    return TRUE;
}

QByteArray CWizKMDatabaseServer::DownloadDocumentData(const QString& strDocumentGUID)
{
    WIZDOCUMENTDATAEX_XMLRPC_FULL ret;
    if (!document_getData(strDocumentGUID, WIZKM_XMKRPC_DOCUMENT_PART_DATA, ret))
    {
        TOLOG1(_T("Failed to download note data: %1"), strDocumentGUID);
        return NULL;
    }
    //
    if (ret.arrayData.isEmpty())
        return NULL;
    //
    return ret.arrayData;
}
//
struct WIZDOCUMENTATTACHMENTDATAEX_XMLRPC_FULL
    : public WIZDOCUMENTATTACHMENTDATAEX
{
    int nApiVersion;
    UINT nParts;
    int nDataSize;
    //
    WIZDOCUMENTATTACHMENTDATAEX_XMLRPC_FULL()
    {
        nApiVersion = WIZKM_WEBAPI_VERSION;
        nParts = 0;
        nDataSize = 0;
    }
    WIZDOCUMENTATTACHMENTDATAEX_XMLRPC_FULL(WIZDOCUMENTATTACHMENTDATAEX& data)
        :WIZDOCUMENTATTACHMENTDATAEX(data)
    {
        nApiVersion = WIZKM_WEBAPI_VERSION;
        nParts = 0;
        nDataSize = 0;
    }
    BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data)
    {
        BOOL bInfo = FALSE;
        BOOL bData = FALSE;
        data.GetBool(_T("attachment_info"), bInfo);
        data.GetBool(_T("attachment_data"), bData);
        data.GetInt64(_T("version"), nVersion);
        //
        nParts = 0;
        nParts |= bInfo ? WIZKM_XMKRPC_ATTACHMENT_PART_INFO : 0;
        nParts |= bData ? WIZKM_XMKRPC_ATTACHMENT_PART_DATA : 0;
        //
        data.GetString(_T("attachment_guid"), strGUID);
        data.GetInt64(_T("version"), nVersion);
        //
        if (bInfo)
        {
            data.GetStr(_T("attachment_document_guid"), strDocumentGUID);
            data.GetStr(_T("attachment_name"), strName);
            data.GetStr(_T("attachment_url"), strURL);
            data.GetStr(_T("attachment_description"), strDescription);
            data.GetTime(_T("dt_info_modified"), tInfoModified);
            data.GetStr(_T("info_md5"), strInfoMD5);
            data.GetTime(_T("dt_data_modified"), tDataModified);
            data.GetStr(_T("data_md5"), strDataMD5);
        }

        //
        data.GetInt(_T("attachment_zip_size"), nDataSize);
        //
        return !strGUID.isEmpty();
    }
};



QByteArray CWizKMDatabaseServer::DownloadAttachmentData(const QString& strAttachmentGUID)
{
    WIZDOCUMENTATTACHMENTDATAEX_XMLRPC_FULL ret;
    if (!attachment_getData(strAttachmentGUID, WIZKM_XMKRPC_ATTACHMENT_PART_DATA, ret))
    {
        TOLOG1(_T("Failed to download attachment data: %1"), strAttachmentGUID);
        return NULL;
    }
    //
    if (ret.arrayData.isEmpty())
        return NULL;
    //
    return ret.arrayData;
}
