#include "wizkmxmlrpc.h"


#include "../share/wizXmlRpcServer.h"


#define WIZUSERMESSAGE_AT		0
#define WIZUSERMESSAGE_EDIT		1

#define WIZKM_XMLRPC_ERROR_TRAFFIC_LIMIT		304
#define WIZKM_XMLRPC_ERROR_STORAGE_LIMIT		305


const int WIZ_USER_MSG_TYPE_CALLED = 0;
const int WIZ_USER_MSG_TYPE_MODIFIED = 1;

struct WIZUSERMESSAGEDATA
{
    __int64 nMessageID;
    QString strBizGUID;
    QString strKbGUID;
    QString strDocumentGUID;
    QString strSenderGUID;
    QString strSenderID;
    QString strReceiverGUID;
    QString strReceiverID;
    int nMessageType;
    int nReadStatus;	//阅读状态, 0:未读，1:已读
    COleDateTime tCreated;
    QString strMessageText;
    __int64 nVersion;
    QString strSender;
    QString strReceiver;
    QString strTitle;
    //
    WIZUSERMESSAGEDATA()
        : nMessageID(0)
        , nMessageType(WIZ_USER_MSG_TYPE_CALLED)
        , nReadStatus(0)
        , nVersion(0)
    {

    }
    //

    BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data)
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
        data.GetTime(_T("dt_created"), tCreated);
        data.GetStr(_T("message_body"), strMessageText);
        data.GetInt64(_T("version"), nVersion);
        data.GetStr(_T("sender_alias"), strSender);
        data.GetStr(_T("receiver_alias"), strReceiver);
        data.GetStr(_T("sender_alias"), strSender);
        data.GetStr(_T("title"), strTitle);
        return 	TRUE;
    }
};
typedef std::deque<WIZUSERMESSAGEDATA> CWizUserMessageDataArray;





class CWizKMXmlRpcServerBase : public CWizXmlRpcServerBase
{
public:
    CWizKMXmlRpcServerBase(const QString& strUrl, QObject* parent);
    BOOL GetValueVersion(const QString& strMethodPrefix, const QString& strToken, const QString& strKbGUID, const QString& strKey, __int64& nVersion);
    BOOL GetValue(const QString& strMethodPrefix, const QString& strToken, const QString& strKbGUID, const QString& strKey, QString& strValue, __int64& nVersion);
    BOOL SetValue(const QString& strMethodPrefix, const QString& strToken, const QString& strKbGUID, const QString& strKey, const QString& strValue, __int64& nRetVersion);
};

class WIZKMUSERINFO : public WIZUSERINFO
{

};

class CWizKMAccountsServer : public CWizKMXmlRpcServerBase
{
public:
    CWizKMAccountsServer(const QString& strUrl, QObject* parent);
    virtual ~CWizKMAccountsServer(void);
    //
    virtual void OnXmlRpcError();
protected:
    BOOL m_bLogin;
    //
    BOOL m_bAutoLogout;
public:
    WIZKMUSERINFO m_retLogin;
public:
    BOOL Login(const QString& strUserName, const QString& strPassword, const QString& strType);
    BOOL Logout();
    BOOL ChangePassword(const QString& strUserName, const QString& strOldPassword, const QString& strNewPassword);
    BOOL ChangeUserId(const QString& strUserName, const QString& strPassword, const QString& strNewUserId);
    BOOL GetToken(const QString& strUserName, const QString& strPassword, QString& strToken);
    BOOL GetCert(const QString& strUserName, const QString& strPassword, QString& strN, QString& stre, QString& strd, QString& strHint);
    BOOL SetCert(const QString& strUserName, const QString& strPassword, const QString& strN, const QString& stre, const QString& strd, const QString& strHint);
    BOOL CreateAccount(const QString& strUserName, const QString& strPassword, const QString& InviteCode);
    void SetAutoLogout(BOOL b) { m_bAutoLogout = b; }
    BOOL ShareSNS(const QString& strToken, const QString& strSNS, const QString& strComment, const QString& strURL, const QString& strDocumentGUID);
    BOOL ShareGroup(const QString& strToken, const QString& strDocumentGUIDs, const QString& strGroups);
    BOOL GetGroupList(CWizGroupDataArray& arrayGroup);
    BOOL CreateTempGroup(const QString& strEmails, const QString& strAccessControl, const QString& strSubject, const QString& strEmailText, WIZGROUPDATA& group);
    BOOL KeepAlive(const QString& strToken);
    BOOL GetMessages(__int64 nVersion, CWizUserMessageDataArray& arrayMessage);
    BOOL SetMessageReadStatus(const QString& strMessageIDs, int nStatus);
    //
    BOOL GetValueVersion(const QString& strKey, __int64& nVersion);
    BOOL GetValue(const QString& strKey, QString& strValue, __int64& nVersion);
    BOOL SetValue(const QString& strKey, const QString& strValue, __int64& nRetVersion);
public:
    BOOL GetWizKMDatabaseServer(QString& strServer, int& nPort, QString& strXmlRpcFile);
    QString GetToken();
    QString GetKbGUID();
    int GetMaxFileSize() { return m_retLogin.GetMaxFileSize(); }
    const WIZKMUSERINFO& GetUserInfo() const { return m_retLogin; }
    WIZKMUSERINFO& GetUserInfo() { return m_retLogin; }
    void SetUserInfo(const WIZKMUSERINFO& userInfo) { m_bLogin = TRUE; m_retLogin = userInfo; }
private:
    //
    QString MakeXmlRpcPassword(const QString& strPassword);
    //
    BOOL accounts_clientLogin(const QString& strUserName, const QString& strPassword, const QString& strType, WIZKMUSERINFO& ret);
    BOOL accounts_clientLogout(const QString& strToken);
    BOOL accounts_keepAlive(const QString& strToken);
    BOOL accounts_createAccount(const QString& strUserName, const QString& strPassword, const QString& strInviteCode);
    BOOL accounts_changePassword(const QString& strUserName, const QString& strOldPassword, const QString& strNewPassword);
    BOOL accounts_changeUserId(const QString& strUserName, const QString& strPassword, const QString& strNewUserId);
    BOOL accounts_getToken(const QString& strUserName, const QString& strPassword, QString& strToken);
    BOOL accounts_getCert(const QString& strUserName, const QString& strPassword, QString& strN, QString& stre, QString& strd, QString& strHint);
    BOOL accounts_setCert(const QString& strUserName, const QString& strPassword, const QString& strN, const QString& stre, const QString& strd, const QString& strHint);
    BOOL document_shareSNS(const QString& strToken, const QString& strSNS, const QString& strComment, const QString& strURL, const QString& strDocumentGUID);
    BOOL document_shareGroup(const QString& strToken, const QString& strDocumentGUIDs, const QString& strGroups);
    //
    BOOL accounts_getGroupList(CWizGroupDataArray& arrayGroup);
    //
    BOOL accounts_createTempGroupKb(const QString& strEmails, const QString& strAccessControl, const QString& strSubject, const QString& strEmailText, WIZGROUPDATA& group);
    //
    BOOL accounts_getMessages(int nCountPerPage, __int64 nVersion, CWizUserMessageDataArray& arrayMessage);

};



#define WIZKM_WEBAPI_VERSION		3


struct CWizKMBaseParam: public CWizXmlRpcStructValue
{
    CWizKMBaseParam(int apiVersion = WIZKM_WEBAPI_VERSION)
    {
        ChangeApiVersion(apiVersion);
        //
#ifdef WINCE
        AddString(_T("client_type"), _T("WM"));
#else
        AddString(_T("client_type"), _T("WIN"));
#endif
        AddString(_T("client_version"), _T("2.0"));
    }
    //
    void ChangeApiVersion(int nApiVersion)
    {
        AddString(_T("api_version"), WizIntToStr(nApiVersion));
    }
    int GetApiVersion()
    {
        QString str;
        GetString(_T("api_version"), str);
        return _ttoi(str);
    }
};


struct CWizKMTokenOnlyParam : public CWizKMBaseParam
{
    CWizKMTokenOnlyParam(const QString& strToken, const QString& strKbGUID)
    {
        AddString(_T("token"), strToken);
        AddString(_T("kb_guid"), strKbGUID);
    }
};





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
    m_bAutoLogout = TRUE;
}

CWizKMAccountsServer::~CWizKMAccountsServer(void)
{
    if (m_bAutoLogout)
    {
        Logout();
    }
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

BOOL CWizKMAccountsServer::CreateAccount(const QString& strUserName, const QString& strPassword, const QString& strInviteCode)
{
    return accounts_createAccount(strUserName, strPassword, strInviteCode);
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
BOOL CWizKMAccountsServer::ShareGroup(const QString& strToken, const QString& strDocumentGUIDs, const QString& strGroups)
{
    return document_shareGroup(strToken, strDocumentGUIDs, strGroups);
}

BOOL CWizKMAccountsServer::GetGroupList(CWizGroupDataArray& arrayGroup)
{
    return accounts_getGroupList(arrayGroup);
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
        if (!accounts_getMessages(nCountPerPage, nNextVersion, arrayPageData))
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




BOOL CWizKMAccountsServer::accounts_clientLogin(const QString& strUserName, const QString& strPassword, const QString& strType, WIZKMUSERINFO& ret)
{
    DEBUG_TOLOG(_T("Start Login"));
    //
    CWizKMBaseParam param;

    param.AddString(_T("user_id"), strUserName);
    param.AddString(_T("password"), MakeXmlRpcPassword(strPassword));
    param.AddString(_T("program_type"), strType);
    param.AddString(_T("protocol"), "https");
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

BOOL CWizKMAccountsServer::accounts_createAccount(const QString& strUserName, const QString& strPassword, const QString& strInviteCode)
{
    CWizKMBaseParam param;

    param.AddString(_T("user_id"), strUserName);
    param.AddString(_T("password"), MakeXmlRpcPassword(strPassword));
    param.AddString(_T("invite_code"), strInviteCode);
    param.AddString(_T("product_name"), "WizNoteQT");
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
    CWizKMTokenOnlyParam param(strToken, GetKbGUID());
    //
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
    //
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

BOOL CWizKMAccountsServer::document_shareGroup(const QString& strToken, const QString& strDocumentGUIDs, const QString& strGroups)
{
    CWizKMTokenOnlyParam param(strToken, GetKbGUID());
    param.AddString(_T("document_guids"), strDocumentGUIDs);
    param.AddString(_T("groups"), strGroups);
    //
    CWizXmlRpcResult callRet;
    if (!Call(_T("document.shareGroup"), callRet, &param))
    {
        TOLOG(_T("share note to groups failed!"));
        return FALSE;
    }
    //
    return TRUE;
}



BOOL CWizKMAccountsServer::accounts_getGroupList(CWizGroupDataArray& arrayGroup)
{
    CWizKMTokenOnlyParam param(GetToken(), GetKbGUID());
    if (WIZKM_WEBAPI_VERSION < 4)
    {
        param.ChangeApiVersion(4);
    }
    //
    param.AddString(_T("kb_type"), _T("group"));
    param.AddString(_T("protocol"), "https");
    //
    std::deque<WIZGROUPDATA> arrayWrap;
    if (!Call(_T("accounts.getGroupKbList"), arrayWrap, &param))
    {
        TOLOG(_T("document.getGroupKbList failure!"));
        return FALSE;
    }
    //
    arrayGroup.assign(arrayWrap.begin(), arrayWrap.end());
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





BOOL CWizKMAccountsServer::accounts_getMessages(int nCountPerPage, __int64 nVersion, CWizUserMessageDataArray& arrayMessage)
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



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



struct WIZOBJECTVERSION
{
    __int64 nDocumentVersion;
    __int64 nTagVersion;
    __int64 nStyleVersion;
    __int64 nAttachmentVersion;
    __int64 nDeletedGUIDVersion;
    //
    WIZOBJECTVERSION()
    {
        nDocumentVersion = -1;
        nTagVersion = -1;
        nStyleVersion = -1;
        nAttachmentVersion = -1;
        nDeletedGUIDVersion = -1;
    }
};



class CWizKMDatabaseServer: public CWizKMXmlRpcServerBase
{
public:
    CWizKMDatabaseServer(const WIZUSERINFOBASE& kbInfo, QObject* parent);
    virtual ~CWizKMDatabaseServer();
    //
    virtual void OnXmlRpcError();
protected:
    WIZUSERINFOBASE m_kbInfo;
public:
    QString GetToken() const { return m_kbInfo.strToken; }
    QString GetKbGUID() const { return m_kbInfo.strKbGUID; }
    int GetMaxFileSize() const { return m_kbInfo.GetMaxFileSize(); }
    //////////////////////////////////////////////////////////////////////////////////////
    BOOL wiz_getInfo(WIZKBINFO& info);
    BOOL wiz_getVersion(WIZOBJECTVERSION& version, BOOL bAuto = FALSE);
    //
    BOOL document_getData(const QString& strDocumentGUID, UINT nParts, WIZDOCUMENTDATAEX& ret);
    BOOL document_postData(const WIZDOCUMENTDATAEX& data, UINT nParts, __int64& nServerVersion);
    BOOL attachment_getData(const QString& strAttachmentGUID, UINT nParts, WIZDOCUMENTATTACHMENTDATAEX& ret);
    BOOL attachment_postData(WIZDOCUMENTATTACHMENTDATAEX& data, UINT nParts, __int64& nServerVersion);
    //
    BOOL document_downloadList(const CWizStdStringArray& arrayDocumentGUID, std::deque<WIZDOCUMENTDATAEX>& arrayRet);;
    BOOL attachment_downloadList(const CWizStdStringArray& arrayAttachmentGUID, std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet);
    //
    BOOL document_downloadFullList(const CWizStdStringArray& arrayDocumentGUID, std::deque<WIZDOCUMENTDATAEX>& arrayRet);
    BOOL document_downloadFullListEx(const CWizStdStringArray& arrayDocumentGUID, std::deque<WIZDOCUMENTDATAEX>& arrayRet);
    //
    BOOL document_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDOCUMENTDATAEX>& arrayRet);
    BOOL attachment_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet);
    BOOL tag_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZTAGDATA>& arrayRet);
    BOOL style_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZSTYLEDATA>& arrayRet);
    BOOL deleted_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDELETEDGUIDDATA>& arrayRet);
    //
    BOOL tag_postList(std::deque<WIZTAGDATA>& arrayTag);
    BOOL style_postList(std::deque<WIZSTYLEDATA>& arrayStyle);
    BOOL deleted_postList(std::deque<WIZDELETEDGUIDDATA>& arrayDeletedGUID);
    QByteArray DownloadDocumentData(const QString& strDocumentGUID);
    QByteArray DownloadAttachmentData(const QString& strAttachmentGUID);
    //
    BOOL category_getAll(QString& str);
    //
    BOOL data_download(const QString& strObjectGUID, const QString& strObjectType, QByteArray& stream, const QString& strDisplayName);
    BOOL data_upload(const QString& strObjectGUID, const QString& strObjectType, const QByteArray& stream, const QString& strObjMD5, const QString& strDisplayName);
    //
    BOOL GetValueVersion(const QString& strKey, __int64& nVersion);
    BOOL GetValue(const QString& strKey, QString& strValue, __int64& nVersion);
    BOOL SetValue(const QString& strKey, const QString& strValue, __int64& nRetVersion);
public:
    virtual int GetCountPerPage();
protected:
    BOOL document_getData2(const QString& strDocumentGUID, UINT nParts, WIZDOCUMENTDATAEX& ret);
    BOOL document_postData2(const WIZDOCUMENTDATAEX& data, UINT nParts, __int64& nServerVersion);
    BOOL attachment_getData2(const QString& strAttachmentGUID, UINT nParts, WIZDOCUMENTATTACHMENTDATAEX& ret);
    BOOL attachment_postData2(WIZDOCUMENTATTACHMENTDATAEX& data, UINT nParts, __int64& nServerVersion);
    //
    BOOL data_download(const QString& strObjectGUID, const QString& strObjectType, int pos, int size, QByteArray& stream, int& nAllSize, BOOL& bEOF);
    BOOL data_upload(const QString& strObjectGUID, const QString& strObjectType, const QString& strObjectMD5, int allSize, int partCount, int partIndex, int partSize, const QByteArray& stream);
    //
    ////////////////////////////////////////////////////////////
    //downloadList
    ////通过GUID列表，下载完整的对象信息列表，和getList不同，getList对于文档，仅下载有限的信息，例如md5信息等等////
    //
    //
    template <class TData, class TWrapData>
    BOOL downloadList(const QString& strMethodName, const QString& strGUIDArrayValueName, const CWizStdStringArray& arrayGUID, std::deque<TData>& arrayRet)
    {
        if (arrayGUID.empty())
            return TRUE;
        //
        CWizKMTokenOnlyParam param(m_kbInfo.strToken, m_kbInfo.strKbGUID);
        param.AddStringArray(strGUIDArrayValueName, arrayGUID);
        //
        std::deque<TWrapData> arrayWrap;
        if (!Call(strMethodName, arrayWrap, &param))
        {
            TOLOG1(_T("%1 failure!"), strMethodName);
            return FALSE;
        }
        //
        arrayRet.assign(arrayWrap.begin(), arrayWrap.end());
        //
        return TRUE;
    }

    //
    /////////////////////////////////////////////
    //getList
    ////通过版本号获得对象列表////
    //
    template <class TData, class TWrapData>
    BOOL getList(const QString& strMethodName, int nCountPerPage, __int64 nVersion, std::deque<TData>& arrayRet)
    {
        CWizKMTokenOnlyParam param(m_kbInfo.strToken, m_kbInfo.strKbGUID);
        param.AddInt(_T("count"), nCountPerPage);
        param.AddString(_T("version"), WizInt64ToStr(nVersion));
        //
        std::deque<TWrapData> arrayWrap;
        if (!Call(strMethodName, arrayWrap, &param))
        {
            TOLOG(_T("object.getList failure!"));
            return FALSE;
        }
        //
        arrayRet.assign(arrayWrap.begin(), arrayWrap.end());
        //
        return TRUE;
    }
    //
    //
    ////////////////////////////////////////////
    //postList
    ////上传对象列表，适用于简单对象：标签，样式，已删除对象////
    //
    template <class TData, class TWrapData>
    BOOL postList(const QString& strMethosName, const QString& strArrayName, std::deque<TData>& arrayData)
    {
        if (arrayData.empty())
            return TRUE;
        //
        int nCountPerPage = GetCountPerPage();
        //
        typename std::deque<TData>::const_iterator it = arrayData.begin();
        //
        while (1)
        {
            //
            std::deque<TWrapData> subArray;
            //
            for (;
                it != arrayData.end(); )
            {
                subArray.push_back(*it);
                it++;
                //
                if (subArray.size() == nCountPerPage)
                    break;
            }
            //

            CWizKMTokenOnlyParam param(m_kbInfo.strToken, m_kbInfo.strKbGUID);
            //
            param.AddArray<TWrapData>(strArrayName, subArray);
            //
            QString strCount;
            if (!Call(strMethosName, _T("success_count"), strCount, &param))
            {
        #ifdef _DEBUG
                WizMessageBox1(_T("Failed to upload list: %1"), strMethosName);
        #endif
                TOLOG1(_T("%1 failure!"), strMethosName);
                return FALSE;
            }
            //
            if (_ttoi(strCount) != (int)subArray.size())
            {
                QString strError = WizFormatString3(_T("Failed to upload list: %1, upload count=%2, success_count=%3"), strMethosName,
                    WizIntToStr(int(subArray.size())),
                    strCount);
        #ifdef _DEBUG
                WizMessageBox(strError);
        #endif
                TOLOG1(_T("%1 failure!"), strMethosName);
                //
                ATLASSERT(FALSE);
                return FALSE;
            }
            //
            //
            if (it == arrayData.end())
                break;
        }
        //
        return TRUE;
    }
    /////////////////////////////////////////////////////
    ////获得所有的对象列表//
    //
    template <class TData>
    BOOL getList(int nCountPerPage, __int64 nVersion, std::deque<TData>& arrayRet)
    {
        ATLASSERT(FALSE);
        return FALSE;
    }
    template <class TData>
    BOOL getList(int nCountPerPage, __int64 nVersion, std::deque<WIZTAGDATA>& arrayRet)
    {
        return tag_getList(nCountPerPage, nVersion, arrayRet);
    }
    template <class TData>
    BOOL getList(int nCountPerPage, __int64 nVersion, std::deque<WIZSTYLEDATA>& arrayRet)
    {
        return style_getList(nCountPerPage, nVersion, arrayRet);
    }
    template <class TData>
    BOOL getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDELETEDGUIDDATA>& arrayRet)
    {
        return deleted_getList(nCountPerPage, nVersion, arrayRet);
    }
    template <class TData>
    BOOL getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDOCUMENTDATAEX>& arrayRet)
    {
        return document_getList(nCountPerPage, nVersion, arrayRet);
    }
    template <class TData>
    BOOL getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
    {
        return attachment_getList(nCountPerPage, nVersion, arrayRet);
    }
    //
    /////////////////////////////////////////////////////////
    ////下载对象数据/////////////////

    template <class TData>
    BOOL getObjectData(TData& data, UINT nParts)
    {
        return TRUE;
    }
    template <class TData>
    BOOL getObjectData(WIZDOCUMENTDATAEX& data, UINT nParts)
    {
        return document_getData(data.strGUID, nParts, data);
    }
    template <class TData>
    BOOL getObjectData(WIZDOCUMENTATTACHMENTDATAEX& data, UINT nParts)
    {
        return attachment_getData(data.strGUID, nParts, data);
    }
public:
    //
    ///////////////////////////////////////////////////////
    ////下载列表//////////////
    //
    template <class TData>
    BOOL downloadList(const CWizStdStringArray& arrayGUID, std::deque<TData>& arrayData)
    {
        return TRUE;
    }
    template <class TData>
    BOOL downloadList(const CWizStdStringArray& arrayGUID, std::deque<WIZDOCUMENTDATAEX>& arrayData)
    {
        return document_downloadList(arrayGUID, arrayData);
    }
    template <class TData>
    BOOL downloadList(const CWizStdStringArray& arrayGUID, std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayData)
    {
        return attachment_downloadList(arrayGUID, arrayData);
    }
    //
    template <class TData>
    BOOL postList(std::deque<TData>& arrayData)
    {
        ATLASSERT(FALSE);
        return FALSE;
    }
    //
    template <class TData>
    BOOL postList(std::deque<WIZTAGDATA>& arrayData)
    {
        return tag_postList(arrayData);
    }
    template <class TData>
    BOOL postList(std::deque<WIZSTYLEDATA>& arrayData)
    {
        return style_postList(arrayData);
    }
    template <class TData>
    BOOL postList(std::deque<WIZDELETEDGUIDDATA>& arrayData)
    {
        return deleted_postList(arrayData);
    }
    //
    template <class TData>
    BOOL postData(TData& data, UINT nParts, __int64& nServerVersion)
    {
        ATLASSERT(FALSE);
        return FALSE;
    }
    //
    template <class TData>
    BOOL postData(WIZDOCUMENTDATAEX& data, UINT nParts, __int64& nServerVersion)
    {
        return document_postData(data, nParts, nServerVersion);
    }
    template <class TData>
    BOOL postData(WIZDOCUMENTATTACHMENTDATAEX& data, UINT nParts, __int64& nServerVersion)
    {
        return attachment_postData(data, nParts, nServerVersion);
    }
public:

    template <class TData>
    BOOL getAllList(int nCountPerPage, __int64 nVersion, std::deque<TData>& arrayRet)
    {
        __int64 nNextVersion = nVersion + 1;
        //
        while (1)
        {
            std::deque<TData> arrayPageData;
            //
            //TODO: version
            //QString strProgress = WizFormatString1(::WizTranslationsTranslateString(_T("Start Version: %1")), WizInt64ToStr(nNextVersion));
            //m_pProgress->OnText(wizhttpstatustypeNormal, strProgress);
            //
            if (!getList<TData>(nCountPerPage, nNextVersion, arrayPageData))
            {
                TOLOG2(_T("Failed to get object list: CountPerPage=%1, Version=%2"), WizIntToStr(nCountPerPage), WizInt64ToStr(nVersion));
                return FALSE;
            }
            //
            arrayRet.insert(arrayRet.end(), arrayPageData.begin(), arrayPageData.end());
            //
            for (typename std::deque<TData>::const_iterator it = arrayPageData.begin();
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

    //
    BOOL getDocumentInfoOnServer(const QString& strDocumentGUID, WIZDOCUMENTDATAEX& dataServer)
    {
        CWizStdStringArray arrayDocument;
        arrayDocument.push_back(strDocumentGUID);
        //
        CWizDocumentDataArray arrayData;
        if (!downloadList<WIZDOCUMENTDATAEX>(arrayDocument, arrayData))
        {
            TOLOG(_T("Can't download document info list from server!"));
            return FALSE;
        }
        //
        if (arrayData.empty())
            return TRUE;
        //
        if (arrayData.size() != 1)
        {
            TOLOG1(_T("Too more documents info downloaded: %1!"), WizInt64ToStr(arrayData.size()));
            return FALSE;
        }
        //
        dataServer = arrayData[0];
        //
        return TRUE;
    }
};



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
        spPartStream.fromRawData(begin, curPartSize);
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


/*
////用于getList，获得简单信息////
*/


struct WIZDOCUMENTDATAEX_XMLRPC_SIMPLE
    : public WIZDOCUMENTDATAEX
{
    WIZDOCUMENTDATAEX_XMLRPC_SIMPLE()
    {
    }
    WIZDOCUMENTDATAEX_XMLRPC_SIMPLE(const WIZDOCUMENTDATAEX& data)
        : WIZDOCUMENTDATAEX(data)
    {
    }
    BOOL LoadFromXmlRpc(CWizXmlRpcStructValue& data)
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
};


BOOL CWizKMDatabaseServer::document_downloadList(const CWizStdStringArray& arrayDocumentGUID, std::deque<WIZDOCUMENTDATAEX>& arrayRet)
{
    return downloadList<WIZDOCUMENTDATAEX, WIZDOCUMENTDATAEX_XMLRPC_SIMPLE>(_T("document.downloadList"), _T("document_guids"), arrayDocumentGUID, arrayRet);
}

BOOL CWizKMDatabaseServer::document_downloadFullList(const CWizStdStringArray& arrayDocumentGUID, std::deque<WIZDOCUMENTDATAEX>& arrayRet)
{
    return downloadList<WIZDOCUMENTDATAEX, WIZDOCUMENTDATAEX_XMLRPC_FULL>(_T("document.downloadInfoList"), _T("document_guids"), arrayDocumentGUID, arrayRet);
}

BOOL CWizKMDatabaseServer::document_downloadFullListEx(const CWizStdStringArray& arrayDocumentGUID, std::deque<WIZDOCUMENTDATAEX>& arrayRet)
{
    int nCountPerPage = 30;
    //
    CWizStdStringArray::const_iterator it = arrayDocumentGUID.begin();
    //
    while (1)
    {
        //
        CWizStdStringArray subArray;
        //
        for (;
            it != arrayDocumentGUID.end(); )
        {
            subArray.push_back(*it);
            it++;
            //
            if (subArray.size() == nCountPerPage)
                break;
        }
        //
        std::deque<WIZDOCUMENTDATAEX> subRet;
        if (!document_downloadFullList(subArray, subRet))
            return FALSE;
        //
        arrayRet.insert(arrayRet.end(), subRet.begin(), subRet.end());
        //
        if (it == arrayDocumentGUID.end())
            break;
    }
    //
    return TRUE;
}

BOOL CWizKMDatabaseServer::attachment_downloadList(const CWizStdStringArray& arrayAttachmentGUID, std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
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

//////////////////////////////////////////////////////////////////////////////////////////////



enum WizKMSyncProgress
{
    syncAccountLogin = 0,
    syncDatabaseLogin,
    syncDownloadDeletedList,
    syncUploadDeletedList,
    syncUploadTagList,
    syncUploadStyleList,
    syncUploadDocumentList,
    syncUploadAttachmentList,
    syncDownloadTagList,
    syncDownloadStyleList,
    syncDownloadSimpleDocumentList,
    syncDownloadFullDocumentList,
    syncDownloadAttachmentList,
    syncDownloadObjectData
};

void GetSyncProgressRange(WizKMSyncProgress progress, int& start, int& count)
{
    int data[syncDownloadObjectData - syncAccountLogin + 1] = {
        1, //syncAccountLogin,
        1, //syncDatabaseLogin,
        2, //syncDownloadDeletedList,
        2, //syncUploadDeletedList,
        2, //syncUploadTagList,
        2, //syncUploadStyleList,
        30, //syncUploadDocumentList,
        10, //syncUploadAttachmentList,
        2, //syncDownloadTagList,
        2, //syncDownloadStyleList,
        5, //syncDownloadSimpleDocumentList,
        10, //syncDownloadFullDocumentList
        5, //syncDownloadAttachmentList,
        26, //syncDownloadObjectData
    };
    //
    //
    start = 0;
    count = data[progress];
    //
    for (int i = 0; i < progress; i++)
    {
        start += data[i];
    }
    //
    ATLASSERT(count > 0);
}
int GetSyncStartProgress(WizKMSyncProgress progress)
{
    int start = 0;
    int count = 0;
    GetSyncProgressRange(progress, start, count);
    return start;
}
int GetSyncProgressSize(WizKMSyncProgress progress)
{
    int start = 0;
    int count = 0;
    GetSyncProgressRange(progress, start, count);
    return count;
}


struct WIZKEYVALUEDATA
{
    QString strValue;
    __int64 nVersion;

    //
    WIZKEYVALUEDATA(const QString& value, __int64 ver)
    {
        strValue = value;
        nVersion = ver;
    }
    //
    WIZKEYVALUEDATA()
    {
        nVersion = 0;
    }
};



struct IWizSyncableDatabase
{
    virtual QString GetUserId() = 0;
    virtual QString GetUserGUID() = 0;
    virtual QString GetPassword() = 0;
    virtual __int64 GetObjectVersion(const QString& strObjectType) = 0;
    virtual BOOL SetObjectVersion(const QString& strObjectType, __int64 nVersion) = 0;
    virtual BOOL OnDownloadDeletedList(const CWizDeletedGUIDDataArray& arrayData) = 0;
    virtual BOOL OnDownloadTagList(const CWizTagDataArray& arrayData) = 0;
    virtual BOOL OnDownloadStyleList(const CWizStyleDataArray& arrayData) = 0;
    virtual BOOL OnDownloadAttachmentList(const CWizDocumentAttachmentDataArray& arrayData) = 0;
    virtual __int64 GetObjectLocalVersion(const QString& strObjectGUID, const QString& strObjectType) = 0;
    virtual __int64 GetObjectLocalServerVersion(const QString& strObjectGUID, const QString& strObjectType) = 0;
    virtual BOOL SetObjectLocalServerVersion(const QString& strObjectGUID, const QString& strObjectType, __int64 nVersion) = 0;
    //
    virtual BOOL DocumentFromGUID(const QString& strGUID, WIZDOCUMENTDATA& dataExists) = 0;
    virtual BOOL SetObjectDataDownloaded(const QString& strGUID, const QString& strType, BOOL downloaded) = 0;
    virtual BOOL SetObjectServerDataInfo(const QString& strGUID, const QString& strType, COleDateTime& tServerDataModified, const QString& strServerMD5) = 0;
    //
    virtual BOOL OnDownloadDocument(int part, const WIZDOCUMENTDATAEX& data) = 0;
    //
    virtual BOOL GetObjectsNeedToBeDownloaded(CWizObjectDataArray& arrayObject) = 0;
    virtual BOOL UpdateObjectData(const QString& strObjectGUID, const QString& strObjectType, const QByteArray& stream) = 0;
    virtual BOOL IsObjectDataDownloaded(const QString& strGUID, const QString& strType) = 0;
    //
    virtual BOOL GetModifiedDeletedList(CWizDeletedGUIDDataArray& arrayData) = 0;
    virtual BOOL GetModifiedTagList(CWizTagDataArray& arrayData) = 0;
    virtual BOOL GetModifiedStyleList(CWizStyleDataArray& arrayData) = 0;
    virtual BOOL GetModifiedDocumentList(CWizDocumentDataArray& arrayData) = 0;
    virtual BOOL GetModifiedAttachmentList(CWizDocumentAttachmentDataArray& arrayData) = 0;
    //
    virtual BOOL InitDocumentData(const QString& strGUID, WIZDOCUMENTDATAEX& data, UINT part) = 0;
    virtual BOOL InitAttachmentData(const QString& strGUID, WIZDOCUMENTATTACHMENTDATAEX& data, UINT part) = 0;

    //
    virtual BOOL OnUploadObject(const QString& strGUID, const QString& strObjectType) = 0;
    //
    virtual BOOL OnDownloadGroups(const CWizGroupDataArray& arrayGroup) = 0;
    virtual IWizSyncableDatabase* GetGroupDatabase(const WIZGROUPDATA& group) = 0;
    virtual void CloseGroupDatabase(IWizSyncableDatabase* pDatabase) = 0;
    //
    virtual void SetKbInfo(const QString& strKBGUID, const WIZKBINFO& info) = 0;
    virtual void SetUserInfo(const WIZUSERINFO& info) = 0;
    //
    virtual BOOL IsGroup() = 0;
    virtual BOOL IsGroupAdmin() = 0;
    virtual BOOL IsGroupSuper() = 0;
    virtual BOOL IsGroupEditor() = 0;
    virtual BOOL IsGroupAuthor() = 0;
    virtual BOOL IsGroupReader() = 0;
    //
    virtual BOOL CanEditDocument(const WIZDOCUMENTDATA& data) = 0;
    virtual BOOL CanEditAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data) = 0;
    //
    virtual BOOL CreateConflictedCopy(const QString& strObjectGUID, const QString& strObjectType) = 0;
    //
    virtual BOOL SaveLastSyncTime() = 0;
    virtual COleDateTime GetLastSyncTime() = 0;
    //
    virtual long GetLocalFlags(const QString& strObjectGUID, const QString& strObjectType) = 0;
    virtual BOOL SetLocalFlags(const QString& strObjectGUID, const QString& strObjectType, long flags) = 0;
    //
    //
    virtual void GetAccountKeys(CWizStdStringArray& arrayKey) = 0;
    virtual __int64 GetAccountLocalValueVersion(const QString& strKey) = 0;
    virtual void SetAccountLocalValue(const QString& strKey, const QString& strValue, __int64 nServerVersion, BOOL bSaveVersion) = 0;

    virtual void GetKBKeys(CWizStdStringArray& arrayKey) = 0;
    virtual BOOL ProcessValue(const QString& strKey) = 0;
    virtual __int64 GetLocalValueVersion(const QString& strKey) = 0;
    virtual QString GetLocalValue(const QString& strKey) = 0;
    virtual void SetLocalValueVersion(const QString& strKey, __int64 nServerVersion) = 0;
    virtual void SetLocalValue(const QString& strKey, const QString& strValue, __int64 nServerVersion, BOOL bSaveVersion) = 0;
    //
    //virtual CComPtr<IWizBizUserCollection> GetBizUsers() = 0;
    virtual void GetAllBizUserIds(CWizStdStringArray& arrayText) = 0;
    //
    //virtual CComPtr<IWizDocument> GetDocumentByGUID(const QString& strDocumentGUID) = 0;
    virtual BOOL OnDownloadMessages(const CWizUserMessageDataArray& arrayMessage) = 0;
    //
    virtual void ClearError() = 0;
    virtual void OnTrafficLimit(const QString& strErrorMessage) = 0;
    virtual void OnStorageLimit(const QString& strErrorMessage) = 0;
    virtual BOOL IsTrafficLimit() = 0;
    virtual BOOL IsStorageLimit() = 0;
};




enum WizKMSyncProgressStatusType
{
    wizhttpstatustypeNormal,
    wizhttpstatustypeWarning,
    wizhttpstatustypeError
};

struct IWizKMSyncEvents
{
    virtual void OnSyncProgress(int pos) {}
    virtual HRESULT OnText(WizKMSyncProgressStatusType type, const QString& strStatus) = 0;
    virtual void SetStop(BOOL b) { m_bStop = b; }
    virtual BOOL IsStop() const { return m_bStop; }
    virtual void SetLastErrorCode(int nErrorCode);
    virtual int GetLastErrorCode() const { return m_nLastError; }
    virtual void SetDatabaseCount(int count) {}
    virtual void SetCurrentDatabase(int index) {}
    virtual void OnTrafficLimit(IWizSyncableDatabase* pDatabase) {}
    virtual void OnStorageLimit(IWizSyncableDatabase* pDatabase) {}
    virtual void OnUploadDocument(const QString& strDocumentGUID, BOOL bDone) {}
    virtual void OnBeginKb(const QString& strKbGUID) {}
    virtual void OnEndKb(const QString& strKbGUID) {}
    //
public:
    void OnStatus(const QString& strText) { OnText(wizhttpstatustypeNormal, strText); }
    void OnWarning(const QString& strText) { OnText(wizhttpstatustypeWarning, strText); }
    void OnError(const QString& strText) { OnText(wizhttpstatustypeError, strText); }
    //
private:
    BOOL m_bStop;
    int m_nLastError;
public:
    IWizKMSyncEvents()
    {
        m_bStop = FALSE;
        m_nLastError = 0;
    }
};


#define _TR(x) (x)

class CWizKMSync
{
public:
    CWizKMSync(IWizSyncableDatabase* pDatabase, const WIZUSERINFOBASE& info, IWizKMSyncEvents* pEvents, BOOL bGroup, BOOL bUploadOnly, QObject* parent);
public:
    BOOL Sync();
    BOOL DownloadObjectData();
protected:
    BOOL SyncCore();
    BOOL UploadValue(const QString& strKey);
    BOOL DownloadValue(const QString& strKey);
    //
    BOOL DownloadDeletedList(__int64 nServerVersion);
    BOOL DownloadTagList(__int64 nServerVersion);
    BOOL DownloadStyleList(__int64 nServerVersion);
    BOOL DownloadSimpleDocumentList(__int64 nServerVersion);
    BOOL DownloadFullDocumentList();
    BOOL DownloadAttachmentList(__int64 nServerVersion);
    //
    BOOL UploadDeletedList();
    BOOL UploadTagList();
    BOOL UploadStyleList();
    BOOL UploadDocumentList();
    BOOL UploadAttachmentList();
    //
    BOOL UploadKeys();
    BOOL DownloadKeys();
    BOOL ProcessOldKeyValues();
private:
    std::deque<WIZDOCUMENTDATAEX_XMLRPC_SIMPLE> m_arrayDocumentNeedToBeDownloaded;
    int CalDocumentPartForDownloadToLocal(const WIZDOCUMENTDATAEX_XMLRPC_SIMPLE& data);
private:
    IWizSyncableDatabase* m_pDatabase;
    WIZUSERINFOBASE m_info;
    WIZKBINFO m_kbInfo;
    IWizKMSyncEvents* m_pEvents;
    BOOL m_bGroup;
    BOOL m_bUploadOnly;
    //
    CWizKMDatabaseServer m_server;
    //
    std::map<QString, WIZKEYVALUEDATA> m_mapOldKeyValues;
public:
    template <class TData>
    static __int64 GetObjectsVersion(__int64 nOldVersion, const std::deque<TData>& arrayData)
    {
        if (arrayData.empty())
            return nOldVersion;
        //
        __int64 nVersion = nOldVersion;
        for (typename std::deque<TData>::const_iterator it = arrayData.begin();
            it != arrayData.end();
            it++)
        {
            nVersion = std::max<__int64>(nOldVersion, it->nVersion);
        }
        return nVersion;
    }
private:
    template <class TData>
    BOOL DownloadList(__int64 nServerVersion, const QString& strObjectType, WizKMSyncProgress progress)
    {
        m_pEvents->OnSyncProgress(::GetSyncStartProgress(progress));
        //
        __int64 nVersion = m_pDatabase->GetObjectVersion(strObjectType);
        if (nServerVersion == nVersion)
        {
            m_pEvents->OnStatus(_TR("No change, skip"));
            return TRUE;
        }
        //
        std::deque<TData> arrayData;
        if (!m_server.getAllList<TData>(200, nVersion, arrayData))
            return FALSE;
        //
        if (!OnDownloadList<TData>(arrayData))
            return FALSE;
        //
        nVersion = GetObjectsVersion<TData>(nVersion, arrayData);
        //
        nVersion = std::max<__int64>(nVersion, nServerVersion);
        //
        return m_pDatabase->SetObjectVersion(strObjectType, nVersion);
    }

    template <class TData>
    BOOL OnDownloadList(const std::deque<TData>& arrayData)
    {
        ATLASSERT(FALSE);
        return FALSE;
    }
    template <class TData>
    BOOL OnDownloadList(const std::deque<WIZDELETEDGUIDDATA>& arrayData)
    {
        return m_pDatabase->OnDownloadDeletedList(arrayData);
    }
    template <class TData>
    BOOL OnDownloadList(const std::deque<WIZTAGDATA>& arrayData)
    {
        return m_pDatabase->OnDownloadTagList(arrayData);
    }
    template <class TData>
    BOOL OnDownloadList(const std::deque<WIZSTYLEDATA>& arrayData)
    {
        return m_pDatabase->OnDownloadStyleList(arrayData);
    }
    template <class TData>
    BOOL OnDownloadList(const std::deque<WIZDOCUMENTDATAEX>& arrayData)
    {
        m_arrayDocumentNeedToBeDownloaded.clear();
        m_arrayDocumentNeedToBeDownloaded.assign(arrayData.begin(), arrayData.end());
        //
        return DownloadFullDocumentList();
    }
    template <class TData>
    BOOL OnDownloadList(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayData)
    {
        return m_pDatabase->OnDownloadAttachmentList(arrayData);
    }
};



CWizKMSync::CWizKMSync(IWizSyncableDatabase* pDatabase, const WIZUSERINFOBASE& info, IWizKMSyncEvents* pEvents, BOOL bGroup, BOOL bUploadOnly, QObject* parent)
    : m_pDatabase(pDatabase)
    , m_info(info)
    , m_pEvents(pEvents)
    , m_bGroup(bGroup)
    , m_server(m_info, parent)
    , m_bUploadOnly(bUploadOnly)
{
#ifdef _DEBUG
    pEvents->OnError(WizFormatString1(_T("XmlRpcUrl: %1"), info.strDatabaseServer));
#endif
}
BOOL CWizKMSync::Sync()
{
    QString strKbGUID = m_bGroup ? m_info.strKbGUID  : QString(_T(""));
    //
    m_pEvents->OnBeginKb(strKbGUID);
    //
    BOOL bRet = SyncCore();
    //
    m_pEvents->OnEndKb(strKbGUID);
    //
    return bRet;
}
BOOL CWizKMSync::SyncCore()
{
    m_pDatabase->ClearError();
    //
    m_mapOldKeyValues.clear();
    //
    //CWizSingletonAppEx app;
    //if (!app.CreateEx(m_info.strBookGUID))
    //{
    //    DEBUG_TOLOG(_T("Sync is running!"));
    //    return FALSE;
    //}
    //
    m_pEvents->OnSyncProgress(::GetSyncStartProgress(syncDatabaseLogin));
    m_pEvents->OnStatus(_TR("Connect to server"));
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    m_pEvents->OnStatus(_TR("Query server infomation"));
    if (!m_bGroup)
    {
        WIZKBINFO info;
        if (m_server.wiz_getInfo(info))
        {
            m_pDatabase->SetKbInfo(_T(""), info);
            m_kbInfo = info;
        }
    }
    else
    {
        WIZKBINFO info;
        if (m_server.wiz_getInfo(info))
        {
            m_pDatabase->SetKbInfo(m_info.strKbGUID, info);
            m_kbInfo = info;
        }
    }
    //
    WIZOBJECTVERSION versionServer;
    if (!m_server.wiz_getVersion(versionServer))
    {
        m_pEvents->OnError(_T("Cannot get version information!"));
        return FALSE;
    }
    //
    m_pEvents->OnStatus(_TR("Query deleted objects list"));
    if (!DownloadDeletedList(versionServer.nDeletedGUIDVersion))
    {
        m_pEvents->OnError(_T("Cannot download deleted objects list!"));
        return FALSE;
    }
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    m_pEvents->OnStatus(_TR("Upload deleted objects list"));
    if (!UploadDeletedList())
    {
        m_pEvents->OnError(_T("Cannot upload deleted objects list!"));
        //return FALSE;
    }
    //
    m_pEvents->OnStatus(_TR("Sync settings"));
    UploadKeys();
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    m_pEvents->OnStatus(_TR("Upload tags"));
    if (!UploadTagList())
    {
        m_pEvents->OnError(_T("Cannot upload tags!"));
        return FALSE;
    }
    //
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    m_pEvents->OnStatus(_TR("Upload styles"));
    if (!UploadStyleList())
    {
        m_pEvents->OnError(_T("Cannot upload styles!"));
        return FALSE;
    }
    //
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    m_pEvents->OnStatus(_TR("Upload notes"));
    if (!UploadDocumentList())
    {
        m_pEvents->OnError(_T("Cannot upload notes!"));
        return FALSE;
    }
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    m_pEvents->OnStatus(_TR("Upload attachments"));
    if (!UploadAttachmentList())
    {
        m_pEvents->OnError(_T("Cannot upload attachments!"));
        return FALSE;
    }
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    if (m_bUploadOnly)
        return TRUE;
    //
    m_pEvents->OnStatus(_TR("Sync settings"));
    DownloadKeys();
    //
    m_pEvents->OnStatus(_TR("Download tags"));
    if (!DownloadTagList(versionServer.nTagVersion))
    {
        m_pEvents->OnError(_T("Cannot download tags!"));
        return FALSE;
    }
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    m_pEvents->OnStatus(_TR("Download styles"));
    if (!DownloadStyleList(versionServer.nStyleVersion))
    {
        m_pEvents->OnError(_T("Cannot download styles!"));
        return FALSE;
    }
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    m_pEvents->OnStatus(_TR("Download notes list"));
    if (!DownloadSimpleDocumentList(versionServer.nDocumentVersion))
    {
        m_pEvents->OnError(_T("Cannot download notes list!"));
        return FALSE;
    }
    //
    //
    /*
    // 重新更新服务器的数据，因为如果pc客户端文件夹被移动后，
    // 服务器上面已经没有这个文件夹了，
    // 但是手机同步的时候，因为原有的文件夹里面还有笔记，
    // 因此不会被删除，导致手机上还有空的文件夹
    // 因此在这里需要重新更新一下
    */
    m_pEvents->OnStatus(_TR("Sync settings"));
    ProcessOldKeyValues();
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    m_pEvents->OnStatus(_TR("Download attachments list"));
    if (!DownloadAttachmentList(versionServer.nAttachmentVersion))
    {
        m_pEvents->OnError(_T("Cannot download attachments list!"));
        return FALSE;
    }
    //
    if (m_pEvents->IsStop())
        return FALSE;
    //
    //
    if (!m_bGroup)
    {
        WIZKBINFO info;
        if (m_server.wiz_getInfo(info))
        {
            m_pDatabase->SetKbInfo(_T(""), info);
        }
    }
    //
    m_pEvents->OnSyncProgress(100);
    //
    return TRUE;
}



BOOL CWizKMSync::UploadKeys()
{
    CWizStdStringArray arrValue;
    m_pDatabase->GetKBKeys(arrValue);
    //
    for (CWizStdStringArray::const_iterator it = arrValue.begin();
        it != arrValue.end();
        it++)
    {
        QString strKey = *it;
        //
        if (!m_pDatabase->ProcessValue(strKey))
            continue;
        //
        if (!UploadValue(strKey))
        {
            m_pEvents->OnError(WizFormatString1(_T("Can't upload settings: %1"), strKey));
        }
    }
    //
    return TRUE;
}


BOOL CWizKMSync::DownloadKeys()
{
    CWizStdStringArray arrValue;
    m_pDatabase->GetKBKeys(arrValue);
    //
    for (CWizStdStringArray::const_iterator it = arrValue.begin();
        it != arrValue.end();
        it++)
    {
        QString strKey = *it;
        //
        if (!m_pDatabase->ProcessValue(strKey))
            continue;
        //
        if (!DownloadValue(strKey))
        {
            m_pEvents->OnError(WizFormatString1(_T("Can't download settings: %1"), strKey));
        }
    }
    //
    return TRUE;
}

/*
 * ////重新设置服务器的key value数据 防止被移动的文件夹没有删除
 */
BOOL CWizKMSync::ProcessOldKeyValues()
{
    if (m_mapOldKeyValues.empty())
        return TRUE;
    //
    for (typename std::map<QString, WIZKEYVALUEDATA>::const_iterator it = m_mapOldKeyValues.begin();
        it != m_mapOldKeyValues.end();
        it++)
    {
        QString strKey = it->first;
        const WIZKEYVALUEDATA& data = it->second;
        //
        // 最后一次才记住版本号
        m_pDatabase->SetLocalValue(strKey, data.strValue, data.nVersion, TRUE);
    }
    //
    return TRUE;
}

BOOL CWizKMSync::UploadValue(const QString& strKey)
{
    if (!m_pDatabase)
        return FALSE;
    //
    __int64 nLocalVersion = m_pDatabase->GetLocalValueVersion(strKey);
    if (-1 != nLocalVersion)
        return TRUE;
    //
    QString strValue = m_pDatabase->GetLocalValue(strKey);
    //
    DEBUG_TOLOG(WizFormatString1(_T("Upload key: %1"), strKey));
    DEBUG_TOLOG(strValue);
    //
    __int64 nServerVersion = 0;
    if (m_server.SetValue(strKey, strValue, nServerVersion))
    {
        m_pDatabase->SetLocalValueVersion(strKey, nServerVersion);
    }
    else
    {
        m_pEvents->OnError(WizFormatString1(_T("Can't upload settings: %1"), strKey));
    }
    //
    return TRUE;
}
BOOL CWizKMSync::DownloadValue(const QString& strKey)
{
    if (!m_pDatabase)
        return FALSE;
    //
    __int64 nServerVersion = 0;
    if (!m_server.GetValueVersion(strKey, nServerVersion))
    {
        TOLOG1(_T("Can't get value version: %1"), strKey);
        return FALSE;
    }
    //
    if (-1 == nServerVersion)	//not found
        return TRUE;
    //
    __int64 nLocalVersion = m_pDatabase->GetLocalValueVersion(strKey);
    if (nServerVersion <= nLocalVersion)
        return TRUE;
    //
    //
    QString strServerValue;
    if (!m_server.GetValue(strKey, strServerValue, nServerVersion))
    {
        return FALSE;
    }
    //
    m_pDatabase->SetLocalValue(strKey, strServerValue, nServerVersion, FALSE);
    //
    m_mapOldKeyValues[strKey] = WIZKEYVALUEDATA(strServerValue, nServerVersion);
    //
    return TRUE;
}



template <class TData>
BOOL GetModifiedObjectList(IWizSyncableDatabase* pDatabase, std::deque<TData>& arrayData)
{
    ATLASSERT(FALSE);
    return FALSE;
}


template <class TData>
BOOL GetModifiedObjectList(IWizSyncableDatabase* pDatabase, std::deque<WIZDELETEDGUIDDATA>& arrayData)
{
    return pDatabase->GetModifiedDeletedList(arrayData);
}




template <class TData>
BOOL GetModifiedObjectList(IWizSyncableDatabase* pDatabase, std::deque<WIZTAGDATA>& arrayData)
{
    return pDatabase->GetModifiedTagList(arrayData);
}



template <class TData>
BOOL GetModifiedObjectList(IWizSyncableDatabase* pDatabase, std::deque<WIZSTYLEDATA>& arrayData)
{
    return pDatabase->GetModifiedStyleList(arrayData);
}



template <class TData>
BOOL GetModifiedObjectList(IWizSyncableDatabase* pDatabase, std::deque<WIZDOCUMENTDATAEX>& arrayData)
{
    return pDatabase->GetModifiedDocumentList(arrayData);
}



template <class TData>
BOOL GetModifiedObjectList(IWizSyncableDatabase* pDatabase, std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayData)
{
    return pDatabase->GetModifiedAttachmentList(arrayData);
}


template <class TData>
BOOL UploadSimpleList(const QString& strObjectType, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, CWizKMDatabaseServer& server, WizKMSyncProgress progress)
{
    pEvents->OnSyncProgress(::GetSyncStartProgress(progress));
    //
    std::deque<TData> arrayData;
    GetModifiedObjectList<TData>(pDatabase, arrayData);
    if (arrayData.empty())
    {
        pEvents->OnStatus(_TR("No change, skip"));
        return TRUE;
    }
    //
    //
    if (!server.postList<TData>(arrayData))
    {
        TOLOG(_T("Can't upload list!"));
        return FALSE;
    }
    //
    for (typename std::deque<TData>::const_iterator it = arrayData.begin();
        it != arrayData.end();
        it++)
    {
        pDatabase->OnUploadObject(it->strGUID, strObjectType);
    }
    //
    return TRUE;
}

BOOL CWizKMSync::UploadDeletedList()
{
    if (m_bGroup)
    {
        if (!m_pDatabase->IsGroupAuthor())	//need author
            return TRUE;
    }
    //
    return UploadSimpleList<WIZDELETEDGUIDDATA>(_T("deleted_guid"), m_pEvents, m_pDatabase, m_server, syncUploadDeletedList);
}
BOOL CWizKMSync::UploadTagList()
{
    if (m_bGroup)
    {
        if (!m_pDatabase->IsGroupEditor())	//need editor
            return TRUE;
    }
    //
    return UploadSimpleList<WIZTAGDATA>(_T("tag"), m_pEvents, m_pDatabase, m_server, syncUploadTagList);
}
BOOL CWizKMSync::UploadStyleList()
{
    if (m_bGroup)
    {
        if (!m_pDatabase->IsGroupEditor())	//need editor
            return TRUE;
    }
    //
    return UploadSimpleList<WIZSTYLEDATA>(_T("style"), m_pEvents, m_pDatabase, m_server, syncUploadStyleList);
}


template <class TData>
BOOL InitObjectData(IWizSyncableDatabase* pDatabase, const QString& strObjectGUID, TData& data, int part)
{
    ATLASSERT(FALSE);
    return FALSE;
}

template <class TData>
BOOL InitObjectData(IWizSyncableDatabase* pDatabase, const QString& strObjectGUID, WIZDOCUMENTDATAEX& data, int part)
{
    return pDatabase->InitDocumentData(strObjectGUID, data, part);
}

template <class TData>
BOOL InitObjectData(IWizSyncableDatabase* pDatabase, const QString& strObjectGUID, WIZDOCUMENTATTACHMENTDATAEX& data, int part)
{
    return pDatabase->InitAttachmentData(strObjectGUID, data, part);
}


QByteArray WizCompressAttachmentFile(const QByteArray& stream, QString& strTempFileName, const WIZDOCUMENTATTACHMENTDATAEX& data);

template <class TData>
BOOL CanEditData(IWizSyncableDatabase* pDatabase, const TData& data)
{
    ATLASSERT(FALSE);
    return FALSE;
}
//
template <class TData>
BOOL CanEditData(IWizSyncableDatabase* pDatabase, const WIZDOCUMENTDATAEX& data)
{
    ATLASSERT(pDatabase->IsGroup());
    return pDatabase->CanEditDocument(data);
}
//
template <class TData>
BOOL CanEditData(IWizSyncableDatabase* pDatabase, const WIZDOCUMENTATTACHMENTDATAEX& data)
{
    ATLASSERT(pDatabase->IsGroup());
    return pDatabase->CanEditAttachment(data);
}



int CalDocumentDataForUploadToServer(IWizSyncableDatabase* pDatabase, const QString& strObjectType, const WIZDOCUMENTDATAEX& dataLocal, const WIZDOCUMENTDATAEX& dataServer)
{
    int nPart = 0;
    //
    if (dataLocal.strInfoMD5 != dataServer.strInfoMD5
        && dataLocal.tInfoModified > dataServer.tInfoModified)
    {
        nPart |= WIZKM_XMLRPC_OBJECT_PART_INFO;
    }
    if (dataLocal.strDataMD5 != dataServer.strDataMD5)
    {
        long nFlags = pDatabase->GetLocalFlags(dataLocal.strGUID, strObjectType);
        if (-1 == nFlags)	////为了兼容以前的数据，以前没有记录这个字段，所以需要用时间来判断////
        {
            if (dataLocal.tDataModified > dataServer.tDataModified)
            {
                nPart |= WIZKM_XMLRPC_OBJECT_PART_DATA;
            }
        }
        else
        {
            if (nFlags & WIZKM_XMLRPC_OBJECT_PART_DATA)	////本地数据修改了////
            {
                nPart |= WIZKM_XMLRPC_OBJECT_PART_DATA;
            }
        }
    }
    //
    if (dataLocal.strParamMD5 != dataServer.strParamMD5
        && dataLocal.tParamModified > dataServer.tParamModified)
    {
        nPart |= WIZKM_XMLRPC_OBJECT_PART_PARAM;
    }
    //
    return nPart;
}


int CalAttachmentDataForUploadToServer(IWizSyncableDatabase* pDatabase, const QString& strObjectType, const WIZDOCUMENTATTACHMENTDATAEX& dataLocal, const WIZDOCUMENTATTACHMENTDATAEX& dataServer)
{
    int nPart = 0;
    //
    if (dataLocal.strInfoMD5 != dataServer.strInfoMD5
        && dataLocal.tInfoModified > dataServer.tInfoModified)
    {
        nPart |= WIZKM_XMLRPC_OBJECT_PART_INFO;
    }
    if (dataLocal.strDataMD5 != dataServer.strDataMD5)
    {
        long nFlags = pDatabase->GetLocalFlags(dataLocal.strGUID, strObjectType);
        if (-1 == nFlags)	////为了兼容以前的数据，以前没有记录这个字段，所以需要用时间来判断////
        {
            if (dataLocal.tDataModified > dataServer.tDataModified)
            {
                nPart |= WIZKM_XMLRPC_OBJECT_PART_DATA;
            }
        }
        else
        {
            if (nFlags & WIZKM_XMLRPC_OBJECT_PART_DATA)	////本地数据修改了////
            {
                nPart |= WIZKM_XMLRPC_OBJECT_PART_DATA;
            }
        }
    }
    //
    //
    return nPart;
}


template <class TData, bool _document>
int CalObjectDataForUploadToServer(IWizSyncableDatabase* pDatabase, const QString& strObjectType, const TData& dataLocal, const TData& dataServer)
{
    int nPart = 0;
    //
    if (dataLocal.strInfoMD5 != dataServer.strInfoMD5
        && dataLocal.tInfoModified > dataServer.tInfoModified)
    {
        nPart |= WIZKM_XMLRPC_OBJECT_PART_INFO;
    }
    if (dataLocal.strDataMD5 != dataServer.strDataMD5)
    {
        long nFlags = pDatabase->GetLocalFlags(dataLocal.strGUID, strObjectType);
        if (-1 == nFlags)	////为了兼容以前的数据，以前没有记录这个字段，所以需要用时间来判断////
        {
            if (dataLocal.tDataModified > dataServer.tDataModified)
            {
                nPart |= WIZKM_XMLRPC_OBJECT_PART_DATA;
            }
        }
        else
        {
            if (nFlags & WIZKM_XMLRPC_OBJECT_PART_DATA)	////本地数据修改了////
            {
                nPart |= WIZKM_XMLRPC_OBJECT_PART_DATA;
            }
        }
    }
    //
    if (_document )
    {
        if (dataLocal.strParamMD5 != dataServer.strParamMD5
            && dataLocal.tParamModified > dataServer.tParamModified)
        {
            nPart |= WIZKM_XMLRPC_OBJECT_PART_PARAM;
        }
    }
    //
    return nPart;
}





BOOL UploadDocument(const WIZKBINFO& kbInfo, int size, int start, int total, int index, std::map<QString, WIZDOCUMENTDATAEX>& mapDataOnServer, WIZDOCUMENTDATAEX& local, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, CWizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress)
{
    QString strDisplayName;

    strDisplayName = local.strTitle;
    //
    if (pDatabase->IsGroup())
    {
        if (!CanEditData<WIZDOCUMENTDATAEX>(pDatabase, local))
        {
            //skip
            return TRUE;
        }
    }
    //
    __int64 nOldServerVersion = -1;
    //
    typename std::map<QString, WIZDOCUMENTDATAEX>::const_iterator itMapOnServer = mapDataOnServer.find(local.strGUID);
    //
    UINT part = 0;
    if (itMapOnServer == mapDataOnServer.end())	//new
    {
        part = WIZKM_XMLRPC_OBJECT_PART_INFO | WIZKM_XMLRPC_OBJECT_PART_DATA | WIZKM_XMLRPC_OBJECT_PART_PARAM;
    }
    else
    {
        const WIZDOCUMENTDATAEX& server = itMapOnServer->second;
        nOldServerVersion = server.nVersion;
        part = CalDocumentDataForUploadToServer(pDatabase, strObjectType, local, server);
        //
        ////服务器有了历史版本功能，不再需要解决冲突问题////
    }
    //
    if (!InitObjectData<WIZDOCUMENTDATAEX>(pDatabase, local.strGUID, local, part))
    {
        pEvents->OnError(_TR("Cannot init object data!"));
        return FALSE;
    }
    //
    COleDateTime tLocalModified;
    tLocalModified = local.tModified;
    //
    //check data size
    if (!local.arrayData.isEmpty())
    {
        __int64 nDataSize = local.arrayData.size();
        if (nDataSize > server.GetMaxFileSize())
        {
            QString str;
            str = local.strTitle;

            //pEvents->OnWarning(WizFormatString2(_TR("[%1] is too large (%2), skip it"), str, ::WizInt64ToByteSizeStr(nDataSize)));
            return FALSE;
        }
    }
    //
    //
    //upload
    bool succeeded = false;
    __int64 nServerVersion = -1;
    if (0 == part)
    {
        succeeded = true;	//update server version to newest
        //
        if (nOldServerVersion != -1)
        {
            nServerVersion = nOldServerVersion;
        }
        else
        {
            TOLOG(_T("Fault error: part == 0 && nOldServerVersion = -1"));	//on server and version = -1?
#ifdef _DEBUG
            WizErrorMessageBox(_T("Fault error: part == 0 && nOldServerVersion = -1"));
#endif
            ATLASSERT(FALSE);
        }
    }
    else
    {
        CWizStdStringArray arrayPart;
        if (part & WIZKM_XMLRPC_OBJECT_PART_INFO)
        {
            arrayPart.push_back(_TR("information"));
        }
        if (part & WIZKM_XMLRPC_OBJECT_PART_DATA)
        {
            arrayPart.push_back(_TR("data"));
        }
        if (part & WIZKM_XMLRPC_OBJECT_PART_PARAM)
        {
            arrayPart.push_back(_TR("params"));
        }
        //
        CString strParts;
        WizStringArrayToText(arrayPart, strParts, _T(", "));
        //
        QString strInfo;
        strInfo = ::WizFormatString2(_TR("Updating note [%2] %1"), local.strTitle, strParts);
        //
        for (int i = 0; i < 2; i++)	//try twice
        {
            pEvents->OnStatus(strInfo);
            if (server.postData<WIZDOCUMENTDATAEX>(local, part, nServerVersion))
            {
                succeeded = true;
                break;
            }
            else if (server.GetLastErrorCode() == WIZKM_XMLRPC_ERROR_TRAFFIC_LIMIT)
            {
                QString strMessage = WizFormatString2("Monthly traffic limit reached! \n\nTraffic Limit %1\nTraffic Using:%2",
                                                      ::WizInt64ToStr(kbInfo.nTrafficLimit),
                                                      ::WizInt64ToStr(kbInfo.nTrafficUsage)
                    );
                //
                pDatabase->OnTrafficLimit(strMessage + _T("\n\n") + server.GetLastErrorMessage());
                //
                //pEvents->SetStop(TRUE);
                pEvents->OnTrafficLimit(pDatabase);
                return FALSE;
            }
            else if (server.GetLastErrorCode() == WIZKM_XMLRPC_ERROR_STORAGE_LIMIT)
            {
                QString strMessage = WizFormatString3("Storage limit reached.\n\n%1\nStorage Limit: %2, Storage Using: %3", _T(""),
                    ::WizInt64ToStr(kbInfo.nStorageLimit),
                    ::WizInt64ToStr(kbInfo.nStorageUsage)
                    );
                //
                pDatabase->OnStorageLimit(strMessage + _T("\n\n") + server.GetLastErrorMessage());
                //
                //pEvents->SetStop(TRUE);
                pEvents->OnStorageLimit(pDatabase);
                return FALSE;
            }
        }
    }
    //
    //
    bool updateVersion = false;
    //
    if (succeeded)
    {
        if (-1 != nServerVersion)
        {
            WIZDOCUMENTDATAEX local2 = local;
            InitObjectData<WIZDOCUMENTDATAEX>(pDatabase, local.strGUID, local2, WIZKM_XMLRPC_OBJECT_PART_INFO);
            //
            COleDateTime tLocalModified2;
            tLocalModified2 = local2.tModified;
            //
            if (tLocalModified2 == tLocalModified)
            {
                updateVersion = true;
                pDatabase->SetObjectLocalServerVersion(local.strGUID, strObjectType, nServerVersion);
            }
        }
    }
    //
    //
    double fPos = index / double(total) * size;
    pEvents->OnSyncProgress(start + int(fPos));
    //
    if (!succeeded)
    {
        pEvents->OnWarning(WizFormatString1(_T("Cannot upload note data: %1"), local.strTitle));
        return FALSE;
    }
    //
    if (updateVersion
        && !pDatabase->OnUploadObject(local.strGUID, strObjectType))
    {
        pEvents->OnError(WizFormatString1(_T("Cannot upload local version of document: %1!"), local.strTitle));
        //
        return FALSE;
    }
    //
    return TRUE;
}

BOOL UploadAttachment(const WIZKBINFO& kbInfo, int size, int start, int total, int index, std::map<QString, WIZDOCUMENTATTACHMENTDATAEX>& mapDataOnServer, WIZDOCUMENTATTACHMENTDATAEX& local, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, CWizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress)
{
    QString strDisplayName;

    strDisplayName = local.strName;
    //
    if (-1 == pDatabase->GetObjectLocalVersion(local.strDocumentGUID, _T("document")))
    {
        pEvents->OnWarning(WizFormatString1(_TR("Note of attachment [%1] has not been uploaded, skip this attachment!"), strDisplayName));
        return FALSE;
    }
    //
    if (pDatabase->IsGroup())
    {
        if (!CanEditData<WIZDOCUMENTATTACHMENTDATAEX>(pDatabase, local))
        {
            //skip
            return TRUE;
        }
    }
    //
    __int64 nOldServerVersion = -1;
    //
    typename std::map<QString, WIZDOCUMENTATTACHMENTDATAEX>::const_iterator itMapOnServer = mapDataOnServer.find(local.strGUID);
    //
    UINT part = 0;
    if (itMapOnServer == mapDataOnServer.end())	//new
    {
        part = WIZKM_XMLRPC_OBJECT_PART_INFO | WIZKM_XMLRPC_OBJECT_PART_DATA | WIZKM_XMLRPC_OBJECT_PART_PARAM;
    }
    else
    {
        const WIZDOCUMENTATTACHMENTDATAEX& server = itMapOnServer->second;
        nOldServerVersion = server.nVersion;
        part = CalAttachmentDataForUploadToServer(pDatabase, strObjectType, local, server);
        //
        ////服务器有了历史版本功能，不再需要解决冲突问题////
    }
    //
    if (!InitObjectData<WIZDOCUMENTATTACHMENTDATAEX>(pDatabase, local.strGUID, local, part))
    {
        pEvents->OnError(_TR("Cannot init object data!"));
        return FALSE;
    }
    //
    COleDateTime tLocalModified;
    tLocalModified = local.tDataModified;
    //
    //check data size
    if (!local.arrayData.isEmpty())
    {
        __int64 nDataSize = local.arrayData.size();
        if (nDataSize > server.GetMaxFileSize())
        {
            QString str;
            str = local.strName;
            //pEvents->OnWarning(WizFormatString2(_TR("[%1] is too large (%2), skip it"), str, ::WizInt64ToByteSizeStr(nDataSize)));
            return FALSE;
        }
    }
    //
    //
    //upload
    bool succeeded = false;
    __int64 nServerVersion = -1;
    if (0 == part)
    {
        succeeded = true;	//update server version to newest
        //
        if (nOldServerVersion != -1)
        {
            nServerVersion = nOldServerVersion;
        }
        else
        {
            TOLOG(_T("Fault error: part == 0 && nOldServerVersion = -1"));	//on server and version = -1?
#ifdef _DEBUG
            WizErrorMessageBox(_T("Fault error: part == 0 && nOldServerVersion = -1"));
#endif
            ATLASSERT(FALSE);
        }
    }
    else
    {
        CWizStdStringArray arrayPart;
        if (part & WIZKM_XMLRPC_OBJECT_PART_INFO)
        {
            arrayPart.push_back(_TR("information"));
        }
        if (part & WIZKM_XMLRPC_OBJECT_PART_DATA)
        {
            arrayPart.push_back(_TR("data"));
        }
        if (part & WIZKM_XMLRPC_OBJECT_PART_PARAM)
        {
            arrayPart.push_back(_TR("params"));
        }
        //
        CString strParts;
        WizStringArrayToText(arrayPart, strParts, _T(", "));
        //
        QString strInfo;
        strInfo = ::WizFormatString2(_TR("Updating attachment [%2] %1"), local.strName, strParts);
        //
        for (int i = 0; i < 2; i++)	//try twice
        {
            pEvents->OnStatus(strInfo);
            if (server.postData<WIZDOCUMENTATTACHMENTDATAEX>(local, part, nServerVersion))
            {
                succeeded = true;
                break;
            }
            else if (server.GetLastErrorCode() == WIZKM_XMLRPC_ERROR_TRAFFIC_LIMIT)
            {
                QString strMessage = WizFormatString2("Monthly traffic limit reached! \n\nTraffic Limit %1\nTraffic Using:%2",
                                                      ::WizInt64ToStr(kbInfo.nTrafficLimit),
                                                      ::WizInt64ToStr(kbInfo.nTrafficUsage)
                    );
                //
                pDatabase->OnTrafficLimit(strMessage + _T("\n\n") + server.GetLastErrorMessage());
                //
                if (!strTempFileName.isEmpty()
                    && PathFileExists(strTempFileName))
                {
                    local.arrayData.clear();
                    //
                    DeleteFile(strTempFileName);
                }
                //
                //pEvents->SetStop(TRUE);
                pEvents->OnTrafficLimit(pDatabase);
                return FALSE;
            }
            else if (server.GetLastErrorCode() == WIZKM_XMLRPC_ERROR_STORAGE_LIMIT)
            {
                QString strMessage = WizFormatString3("Storage limit reached.\n\n%1\nStorage Limit: %2, Storage Using: %3", _T(""),
                    ::WizInt64ToStr(kbInfo.nStorageLimit),
                    ::WizInt64ToStr(kbInfo.nStorageUsage)
                    );
                //
                pDatabase->OnStorageLimit(strMessage + _T("\n\n") + server.GetLastErrorMessage());
                //
                if (!strTempFileName.isEmpty()
                    && PathFileExists(strTempFileName))
                {
                    local.arrayData.clear();
                    //
                    DeleteFile(strTempFileName);
                }
                //
                //pEvents->SetStop(TRUE);
                pEvents->OnStorageLimit(pDatabase);
                return FALSE;
            }
        }
    }
    //
    //
    bool updateVersion = false;
    //
    if (succeeded)
    {
        if (-1 != nServerVersion)
        {
            WIZDOCUMENTATTACHMENTDATAEX local2 = local;
            InitObjectData<WIZDOCUMENTATTACHMENTDATAEX>(pDatabase, local.strGUID, local2, WIZKM_XMLRPC_OBJECT_PART_INFO);
            //
            COleDateTime tLocalModified2;
            tLocalModified2 = local2.tDataModified;
            //
            if (tLocalModified2 == tLocalModified)
            {
                updateVersion = true;
                pDatabase->SetObjectLocalServerVersion(local.strGUID, strObjectType, nServerVersion);
            }
        }
    }
    //
    if (!strTempFileName.isEmpty()
        && PathFileExists(strTempFileName))
    {
        local.arrayData.clear();
        //
        DeleteFile(strTempFileName);
    }
    //
    //
    double fPos = index / double(total) * size;
    pEvents->OnSyncProgress(start + int(fPos));
    //
    if (!succeeded)
    {
        pEvents->OnWarning(WizFormatString1(_T("Cannot upload attachment data: %1"), local.strName));
        return FALSE;
    }
    //
    if (updateVersion
        && !pDatabase->OnUploadObject(local.strGUID, strObjectType))
    {
        pEvents->OnError(WizFormatString1(_T("Cannot upload local version of attachment: %1!"), local.strName));
        //
        return FALSE;
    }
    //
    return TRUE;
}


template <class TData>
BOOL UploadObject(const WIZKBINFO& kbInfo, int size, int start, int total, int index, std::map<QString, TData>& mapDataOnServer, TData& local, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, CWizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress)
{
    ATLASSERT(false);
}

template <class TData>
BOOL UploadObject(const WIZKBINFO& kbInfo, int size, int start, int total, int index, std::map<QString, WIZDOCUMENTDATAEX>& mapDataOnServer, WIZDOCUMENTDATAEX& local, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, CWizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress)
{
    return UploadDocument(kbInfo, size, start, total, index, mapDataOnServer, local, pEvents, pDatabase, server, strObjectType, progress);
}

template <class TData>
BOOL UploadObject(const WIZKBINFO& kbInfo, int size, int start, int total, int index, std::map<QString, WIZDOCUMENTATTACHMENTDATAEX>& mapDataOnServer, WIZDOCUMENTATTACHMENTDATAEX& local, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, CWizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress)
{
    return UploadAttachment(kbInfo, size, start, total, index, mapDataOnServer, local, pEvents, pDatabase, server, strObjectType, progress);
}


template <class TData, bool _document>
BOOL UploadList(const WIZKBINFO& kbInfo, IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, CWizKMDatabaseServer& server, const QString& strObjectType, WizKMSyncProgress progress)
{
    if (pDatabase->IsTrafficLimit())
        return FALSE;
    if (pDatabase->IsStorageLimit())
        return FALSE;
    //
    typedef std::deque<TData> TArray;
    TArray arrayData;
    GetModifiedObjectList<TData>(pDatabase, arrayData);
    if (arrayData.empty())
    {
        pEvents->OnStatus(_TR("No change, skip"));
        return TRUE;
    }
    //
    int start = 0;
    int size = 0;
    ::GetSyncProgressRange(progress, start, size);
    pEvents->OnSyncProgress(start);
    //
    int total = int(arrayData.size());
    int index = 0;
    //
    TArray arraySubData;
    CWizStdStringArray arraySubGUID;
    while (!arrayData.empty())
    {
        if (pEvents->IsStop())
            return FALSE;
        //
        TData dataTemp = arrayData.back();
        arrayData.pop_back();
        //
        arraySubData.push_back(dataTemp);
        arraySubGUID.push_back(dataTemp.strGUID);
        //
        if (arraySubData.size() == 50
            || arrayData.empty()
            )
        {
            TArray arrayDataOnServer;
            if (!server.downloadList<TData>(arraySubGUID, arrayDataOnServer))
            {
                pEvents->OnError(_TR("Cannot download object list!"));
                return FALSE;
            }
            //
            std::map<QString, TData> mapDataOnServer;
            for (typename TArray::const_iterator it = arrayDataOnServer.begin();
                it != arrayDataOnServer.end();
                it++)
            {
                mapDataOnServer[it->strGUID] = *it;
            }
            //
            for (typename TArray::const_iterator it = arraySubData.begin();
                it != arraySubData.end();
                it++)
            {
                index++;
                //
                if (pEvents->IsStop())
                    return FALSE;
                //
                TData local = *it;
                //
                if (_document)	//
                {
                    pEvents->OnUploadDocument(local.strGUID, FALSE);
                }
                //
                if (!UploadObject<TData>(kbInfo, size, start, total, index, mapDataOnServer, local, pEvents, pDatabase, server, strObjectType, progress))
                {
                    switch (server.GetLastErrorCode())
                    {
                    case WIZKM_XMLRPC_ERROR_TRAFFIC_LIMIT:
                    case WIZKM_XMLRPC_ERROR_STORAGE_LIMIT:
                        return FALSE;
                    }
                }
                //
                if (_document)	//
                {
                    pEvents->OnUploadDocument(local.strGUID, TRUE);
                }
            }
        }
    }
    //
    return TRUE;
}
BOOL CWizKMSync::UploadDocumentList()
{
    if (m_bGroup)
    {
        if (!m_pDatabase->IsGroupAuthor())	//need author
            return TRUE;
    }
    //
    return UploadList<WIZDOCUMENTDATAEX, true>(m_kbInfo, m_pEvents, m_pDatabase, m_server, _T("document"), syncUploadDocumentList);
}
BOOL CWizKMSync::UploadAttachmentList()
{
    if (m_bGroup)
    {
        if (!m_pDatabase->IsGroupAuthor())	//need author
            return TRUE;
    }
    //
    return UploadList<WIZDOCUMENTATTACHMENTDATAEX, false>(m_kbInfo, m_pEvents, m_pDatabase, m_server, _T("attachment"), syncUploadAttachmentList);
}
///////////////////////////////////////////////////////////////////////////////////////////////

BOOL CWizKMSync::DownloadDeletedList(__int64 nServerVersion)
{
    return DownloadList<WIZDELETEDGUIDDATA>(nServerVersion, _T("deleted_guid"), syncDownloadDeletedList);
}

BOOL CWizKMSync::DownloadTagList(__int64 nServerVersion)
{
    return DownloadList<WIZTAGDATA>(nServerVersion, _T("tag"), syncDownloadTagList);
}

BOOL CWizKMSync::DownloadStyleList(__int64 nServerVersion)
{
    return DownloadList<WIZSTYLEDATA>(nServerVersion, _T("style"), syncDownloadStyleList);
}

BOOL CWizKMSync::DownloadSimpleDocumentList(__int64 nServerVersion)
{
    return DownloadList<WIZDOCUMENTDATAEX>(nServerVersion, _T("document"), syncDownloadSimpleDocumentList);
}


BOOL CWizKMSync::DownloadFullDocumentList()
{
    if (m_arrayDocumentNeedToBeDownloaded.empty())
        return TRUE;
    //
    const QString& strObjectType = _T("document");
    //
    int start = 0;
    int size = 0;
    ::GetSyncProgressRange(::syncDownloadFullDocumentList, start, size);
    m_pEvents->OnSyncProgress(start);
    //
    int total = int(m_arrayDocumentNeedToBeDownloaded.size());
    //
    CWizStdStringArray arrayDocumentGUID;
    //
    std::map<QString, int> mapDocumentPart;
    //
    for (std::deque<WIZDOCUMENTDATAEX_XMLRPC_SIMPLE>::const_iterator itNeedToBeDownloaded = m_arrayDocumentNeedToBeDownloaded.begin();
        itNeedToBeDownloaded != m_arrayDocumentNeedToBeDownloaded.end();
        itNeedToBeDownloaded++)
    {
        if (m_pEvents->IsStop())
            return FALSE;
        //
        WIZDOCUMENTDATAEX_XMLRPC_SIMPLE simple = *itNeedToBeDownloaded;
        //
        int part = CalDocumentPartForDownloadToLocal(simple);
        if (part & WIZKM_XMLRPC_OBJECT_PART_DATA)
        {
            part &= ~WIZKM_XMLRPC_OBJECT_PART_DATA;
            //
            m_pDatabase->SetObjectDataDownloaded(simple.strGUID, strObjectType, false);	//////设置为未下载//////
            m_pDatabase->SetObjectServerDataInfo(simple.strGUID, strObjectType, simple.tDataModified, simple.strDataMD5);	//////设置成服务器的修改时间和md5，等待下载//////
            m_pDatabase->SetObjectLocalServerVersion(simple.strGUID, strObjectType, simple.nVersion);		//////设置为服务器的版本号//////
        }
        //
        if (0 != part)
        {
            mapDocumentPart[itNeedToBeDownloaded->strGUID] = part;
            arrayDocumentGUID.push_back(itNeedToBeDownloaded->strGUID);
        }
        //
        //
        if (arrayDocumentGUID.size() >= 30
            || itNeedToBeDownloaded == (m_arrayDocumentNeedToBeDownloaded.end() - 1))
        {
            if (!arrayDocumentGUID.empty())
            {
                m_pEvents->OnStatus(_TR(_T("Query notes information")));
                //
                std::deque<WIZDOCUMENTDATAEX> arrayDocument;
                if (!m_server.document_downloadFullList(arrayDocumentGUID, arrayDocument))
                {
                    TOLOG(_T("Can't download note info list!"));
                    return FALSE;
                }
                //
                for (std::deque<WIZDOCUMENTDATAEX>::const_iterator itDocument = arrayDocument.begin();
                    itDocument != arrayDocument.end();
                    itDocument++)
                {
                    m_pEvents->OnStatus(WizFormatString1(_T("Update note information: %1"), itDocument->strTitle));
                    //
                    int nDocumentPart = mapDocumentPart[itDocument->strGUID];
                    if (!m_pDatabase->OnDownloadDocument(nDocumentPart, *itDocument))
                    {
                        m_pEvents->OnError(WizFormatString1(_T("Cannot update note information: %1"), itDocument->strTitle));
                        return FALSE;
                    }
                }
            }
            //
            int index = int(itNeedToBeDownloaded - m_arrayDocumentNeedToBeDownloaded.begin());
            //
            double fPos = index / double(total) * size;
            m_pEvents->OnSyncProgress(start + int(fPos));
            //
            arrayDocumentGUID.clear();
            mapDocumentPart.clear();
        }
    }
    //
    return TRUE;
}


BOOL CWizKMSync::DownloadAttachmentList(__int64 nServerVersion)
{
    return DownloadList<WIZDOCUMENTATTACHMENTDATAEX>(nServerVersion, _T("attachment"), syncDownloadAttachmentList);
}


bool WizCompareObjectByTypeAndTime(const WIZOBJECTDATA& data1, const WIZOBJECTDATA& data2)
{
    if (data1.eObjectType != data2.eObjectType)
    {
        return data1.eObjectType > data2.eObjectType;
    }
    //
    return data1.tTime > data2.tTime;
}

BOOL CWizKMSync::DownloadObjectData()
{
    CWizObjectDataArray arrayObject;
    if (!m_pDatabase->GetObjectsNeedToBeDownloaded(arrayObject))
    {
        m_pEvents->OnError(_T("Cannot get objects need to be downloaded form server!"));
        return FALSE;
    }
    //
    std::sort(arrayObject.begin(), arrayObject.end(), WizCompareObjectByTypeAndTime);
    //
    //
    int start = 0;
    int size = 0;
    ::GetSyncProgressRange(::syncDownloadObjectData, start, size);
    m_pEvents->OnSyncProgress(start);
    //
    int total = int(arrayObject.size());
    //
    size_t succeeded = 0;
    //
    size_t nCount = arrayObject.size();
    for (size_t i = 0; i < nCount; i++)
    {
        if (m_pEvents->IsStop())
            return FALSE;
        //
        WIZOBJECTDATA data = arrayObject[i];
        //
        BOOL bRet = FALSE;
        //
        QString strTempFileName = ::WizGlobal()->GetTempPath() + WizIntToStr(GetTickCount()) + _T(".tmp");
        QByteArray stream;
        //if (spStream)
        {
            QString strMsgFormat = data.eObjectType == wizobjectDocument ? _TR("Downloading note: %1"): _TR("Downloading attachment: %1");
            QString strStatus = WizFormatString1(strMsgFormat, data.strDisplayName);
            m_pEvents->OnStatus(strStatus);
            //
            if (m_server.data_download(data.strObjectGUID, WIZOBJECTDATA::ObjectTypeToTypeString(data.eObjectType), stream, data.strDisplayName))
            {
                ::WizSaveDataToFile(strTempFileName, stream);
                stream.clear();
                //
                //TODO: umcompress stream
                /*
                if (spStream = ::WizCreateReadOnlyStreamOnFile(strTempFileName))
                {
                    QString strTempUncompressedFileName;
                    //
                    if (data.eObjectType == wizobjectDocumentAttachment)
                    {
                        spStream = ::WizUncompressAttachmentStream(spStream, strTempUncompressedFileName);
                        if (!spStream)
                        {
                            m_pEvents->OnError(WizFormatString1(_T("Failed to uncomrepss attachment: %1"), data.strDisplayName));
                            continue;
                        }
                    }

                    //
                    if (m_pDatabase->UpdateObjectData(data.strObjectGUID, WIZOBJECTDATA::ObjectTypeToTypeString(data.eObjectType), spStream))
                    {
                        succeeded++;
                    }
                    else
                    {
                        m_pEvents->OnError(WizFormatString1(_T("Cannot save object data to local: %1!"), data.strDisplayName));
                    }
                    //
                    if (!strTempUncompressedFileName.isEmpty()
                        && PathFileExists(strTempUncompressedFileName)
                        )
                    {
                        DeleteFile(strTempUncompressedFileName);
                    }
                }
                else
                {
                    m_pEvents->OnError(WizFormatString1(_T("Failed to read temp file: %1"), strTempFileName));
                    continue;
                }
                */
            }
            else
            {
                m_pEvents->OnError(WizFormatString1(_T("Cannot download object data from server: %1"), data.strDisplayName));
            }
            //
            //spStream = NULL;
        }
        //
        DeleteFile(strTempFileName);
        //
        //
        int index = (int)i;
        //
        double fPos = index / double(total) * size;
        m_pEvents->OnSyncProgress(start + int(fPos));
    }
    //
    return succeeded == nCount;
}


int WizCalDocumentPartForDownloadToLocal(IWizSyncableDatabase* pDatabase, const WIZDOCUMENTDATAEX_XMLRPC_SIMPLE& dataServer)
{
    int nPart = 0;
    //
    WIZDOCUMENTDATA dataLocal;
    if (pDatabase->DocumentFromGUID(dataServer.strGUID, dataLocal))
    {
        if (dataLocal.strInfoMD5 != dataServer.strInfoMD5)
        {
            if (dataLocal.tInfoModified < dataServer.tInfoModified)
            {
                nPart |= WIZKM_XMLRPC_OBJECT_PART_INFO;
            }
            else
            {
                TOLOG2(_T("local and server changed info: local: %1, server :%2\n"), ::WizTimeToSQL(dataLocal.tInfoModified), ::WizTimeToSQL(dataServer.tInfoModified));
            }
        }
        if (dataLocal.strDataMD5 != dataServer.strDataMD5)
        {
            ////不需要解决冲突，在上传的时候，已经解决冲突了////
            nPart |= WIZKM_XMLRPC_OBJECT_PART_DATA;
        }
        if (dataLocal.strParamMD5 != dataServer.strParamMD5)
        {
            if (dataLocal.tParamModified < dataServer.tParamModified)
            {
                nPart |= WIZKM_XMLRPC_OBJECT_PART_PARAM;
            }
            else
            {
                TOLOG2(_T("local and server changed param: local: %1, server :%2\n"), ::WizTimeToSQL(dataLocal.tParamModified), ::WizTimeToSQL(dataServer.tParamModified));
            }
        }
    }
    else
    {
        nPart = WIZKM_XMLRPC_OBJECT_PART_INFO | WIZKM_XMLRPC_OBJECT_PART_DATA | WIZKM_XMLRPC_OBJECT_PART_PARAM;
    }
    //
    return nPart;
}

int CWizKMSync::CalDocumentPartForDownloadToLocal(const WIZDOCUMENTDATAEX_XMLRPC_SIMPLE& dataServer)
{
    return WizCalDocumentPartForDownloadToLocal(m_pDatabase, dataServer);
}


void DownloadAccountKeys(CWizKMAccountsServer& server, IWizSyncableDatabase* pDatabase)
{
    CWizStdStringArray arrayKey;
    pDatabase->GetAccountKeys(arrayKey);
    //
    for (CWizStdStringArray::const_iterator it = arrayKey.begin();
        it != arrayKey.end();
        it++)
    {
        QString strKey = *it;
        //
        __int64 nServerVersion = 0;
        if (!server.GetValueVersion(strKey, nServerVersion))
        {
            TOLOG1(_T("Can't get account value version: %1"), strKey);
            continue;
        }
        //
        if (-1 == nServerVersion)	//not found
            continue;
        //
        __int64 nLocalVersion = pDatabase->GetAccountLocalValueVersion(strKey);
        if (nServerVersion <= nLocalVersion)
            continue;
        //
        QString strServerValue;
        if (!server.GetValue(strKey, strServerValue, nServerVersion))
        {
            continue;
        }
        //
        pDatabase->SetAccountLocalValue(strKey, strServerValue, nServerVersion, FALSE);
    }
}

BOOL WizDownloadMessages(IWizKMSyncEvents* pEvents, CWizKMAccountsServer& server, IWizSyncableDatabase* pDatabase, const CWizGroupDataArray& arrayGroup)
{
    __int64 nOldVersion = pDatabase->GetObjectVersion(_T("Messages"));
    //
    std::deque<WIZUSERMESSAGEDATA> arrayMessage;
    server.GetMessages(nOldVersion, arrayMessage);
    //
    if (arrayMessage.empty())
        return FALSE;
    //

    /*
    ////////
    ////准备群组信息////
    */
    std::map<QString, WIZGROUPDATA> mapGroup;
    for (CWizGroupDataArray::const_iterator it = arrayGroup.begin();
        it != arrayGroup.end();
        it++)
    {
        mapGroup[it->strGroupGUID] = *it;
    }
    //
    /*
    ////按照群组分组笔记////
    */
    std::map<QString, CWizStdStringArray> mapKbGUIDDocuments;
    for (std::deque<WIZUSERMESSAGEDATA>::const_iterator it = arrayMessage.begin();
        it != arrayMessage.end();
        it++)
    {
        if (!it->strKbGUID.isEmpty()
            && !it->strDocumentGUID.isEmpty())
        {
            CWizStdStringArray& documents = mapKbGUIDDocuments[it->strKbGUID];
            documents.push_back(it->strDocumentGUID);
        }
    }
    //
    std::set<QString> setDownloadedDocumentGUID;
    //
    /*
    ////按照kb，下载消息里面的笔记////
    */
    for (std::map<QString, CWizStdStringArray>::const_iterator it = mapKbGUIDDocuments.begin();
        it != mapKbGUIDDocuments.end();
        it++)
    {
        QString strKbGUID = it->first;
        const CWizStdStringArray& arrayDocumentGUID = it->second;
        //
        WIZGROUPDATA group = mapGroup[strKbGUID];
        if (group.strGroupGUID.isEmpty())
            continue;
        //
        IWizSyncableDatabase* pGroupDatabase = pDatabase->GetGroupDatabase(group);
        if (!pGroupDatabase)
        {
            pEvents->OnError(WizFormatString1(_T("Cannot open group: %1"), group.strGroupName));
            continue;
        }
        //
        WIZKMUSERINFO userInfo = server.m_retLogin;
        //
        if (!group.strDatabaseServer.isEmpty())
        {
            userInfo.strDatabaseServer = group.strDatabaseServer;
        }
        //
        userInfo.strKbGUID = group.strGroupGUID;
        CWizKMDatabaseServer serverDB(userInfo, server.parent());
        //
        pEvents->OnStatus(_TR(_T("Query notes information")));
        //
        std::deque<WIZDOCUMENTDATAEX> arrayDocumentServer;
        if (!serverDB.document_downloadFullListEx(arrayDocumentGUID, arrayDocumentServer))
        {
            pEvents->OnError(_T("Can download notes of messages"));
            return FALSE;
        }
        //
        for (std::deque<WIZDOCUMENTDATAEX>::const_iterator itDocument = arrayDocumentServer.begin();
            itDocument != arrayDocumentServer.end();
            itDocument++)
        {
            WIZDOCUMENTDATAEX documentServer = *itDocument;
            /*
            ////记住下载了哪些笔记////
            */
            setDownloadedDocumentGUID.insert(documentServer.strGUID);

            pEvents->OnStatus(WizFormatString1(_T("Update note information: %1"), itDocument->strTitle));
            //
            UINT nDocumentPart = WIZKM_XMKRPC_DOCUMENT_PART_PARAM | WIZKM_XMKRPC_DOCUMENT_PART_INFO | WIZKM_XMKRPC_DOCUMENT_PART_DATA;
            WIZDOCUMENTDATA dataLocal;
            if (pGroupDatabase->DocumentFromGUID(itDocument->strGUID, dataLocal))
            {
                nDocumentPart = WizCalDocumentPartForDownloadToLocal(pGroupDatabase, documentServer);
                if (nDocumentPart & WIZKM_XMLRPC_OBJECT_PART_DATA)
                {
                    nDocumentPart &= ~WIZKM_XMLRPC_OBJECT_PART_DATA;
                    //
                    static const QString& strObjectType = _T("document");
                    //
                    pGroupDatabase->SetObjectDataDownloaded(documentServer.strGUID, strObjectType, false);	//////设置为未下载//////
                    pGroupDatabase->SetObjectServerDataInfo(documentServer.strGUID, strObjectType, documentServer.tDataModified, documentServer.strDataMD5);	//////设置成服务器的修改时间和md5，等待下载//////
                    pGroupDatabase->SetObjectLocalServerVersion(documentServer.strGUID, strObjectType, documentServer.nVersion);		//////设置为服务器的版本号//////
                }
            }
            //
            if (!pGroupDatabase->OnDownloadDocument(nDocumentPart, *itDocument))
            {
                pEvents->OnError(WizFormatString1(_T("Cannot update note information: %1"), itDocument->strTitle));
                return FALSE;
            }
        }
        //
        pDatabase->CloseGroupDatabase(pGroupDatabase);
    }
    //
    ////
    int count = int(arrayMessage.size());
    for (int i = count - 1; i >= 0; i--)
    {
        const WIZUSERMESSAGEDATA& data = arrayMessage[i];
        if (data.strKbGUID.isEmpty())
            continue;
        if (data.strDocumentGUID.isEmpty())
            continue;
        /*
        ////判断对应的笔记是否被下载了（如果没有，说明服务器已经没有这个笔记了，无法显示这个消息）////
        */
        if (setDownloadedDocumentGUID.find(data.strDocumentGUID) != setDownloadedDocumentGUID.end())
            continue;
        //
        arrayMessage.erase(arrayMessage.begin() + i);
    }
    //
    __int64 nNewVersion = CWizKMSync::GetObjectsVersion<WIZUSERMESSAGEDATA>(nOldVersion, arrayMessage);
    //
    pDatabase->OnDownloadMessages(arrayMessage);
    pDatabase->SetObjectVersion(_T("Messages"), nNewVersion);
    //
    return TRUE;
}

void WizDownloadUserAvatars(IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, BOOL bBackground)
{
    //
    /*
    ////每天一次，或者用户手工同步////
    */
    //TODO: modify this code
    /*
    if (!::WizDayOnce(WIZKM_REG_KEY_ROOT, _T("DownloadUserAvatars")))
    {
        if (bBackground)
            return;
    }
    */
    //
    //TODO: getavatar url
    //QString strURLAvatar = ::WizKMWebSiteGetReturn(_T("avatar"));
    CString strURLAvatar = "";
    strURLAvatar.Trim();
    //
    if (0 != strURLAvatar.Find(_T("http"))
        || strURLAvatar.GetLength() > 1024)
    {
        return;
    }
    //
    if (pEvents->IsStop())
        return;
    //
    pEvents->OnStatus("Downloading user image");
    //
    CWizStdStringArray arrayPersonalGroupUser;
    //
    /*
    ////普通群组头像不需要主动下载，额可以在阅读的时候下载////
    //db.GetAllGroupUserIds(arrayPersonalGroupUser);		////
    */
    //
    CWizStdStringArray arrayBizGroupUser;
    pDatabase->GetAllBizUserIds(arrayBizGroupUser);
    //
    QString strCurrentUserID = pDatabase->GetUserId();
    //
    CWizStdStringArray arrayAllUserID;
    arrayAllUserID.push_back(strCurrentUserID);
    arrayAllUserID.insert(arrayAllUserID.begin(), arrayPersonalGroupUser.begin(), arrayPersonalGroupUser.end());
    arrayAllUserID.insert(arrayAllUserID.begin(), arrayBizGroupUser.begin(), arrayBizGroupUser.end());
    //
    std::set<QString> downloaded;
    //
    COleDateTime tNow = ::WizGetCurrentTime();
    //
    /*
    QString strSettingsFileName = ::WizKMGetAvatarsPath() + _T("settings.ini");
    CWizIniFileEx settings;
    settings.LoadFromFile(strSettingsFileName);

    for (CWizStdStringArray::const_iterator it = arrayAllUserID.begin();
        it != arrayAllUserID.end();
        it++)
    {
        QString strUserId = *it;
        //
        if (downloaded.find(strUserId) != downloaded.end())
            continue;
        //
        if (pEvents->IsStop())
            break;
        //
        downloaded.insert(strUserId);
        //
        QString strURL(strURLAvatar);
        strURL.Replace(_T("{userGuid}"), strUserId);
        //
        QString strFileName = ::WizKMGetAvatarsPath() + strUserId + _T(".png");
        //
        if (strUserId != strCurrentUserID)
        {
            if (PathFileExists(strFileName))
            {
                COleDateTimeSpan ts = tNow - WizGetFileModifiedTime(strFileName);
                if (ts.GetDays() <= 7)
                    continue;
                //////不需要更新////
            }
            else
            {
                QString strKey = strUserId;
                const QString& strUserImageSection = _T("UserImage");
                QString strTime = settings.GetStringDef(strUserImageSection, strKey);
                if (!strTime.isEmpty())
                {
                    COleDateTimeSpan ts = tNow - ::WizStringToDateTime(strTime);
                    if (ts.GetDays() <= 7)
                        continue;
                }
                //
                settings.SetString(strUserImageSection, strKey, ::WizDateTimeToString(tNow));
            }
        }
        //
        QString strLeft;
        QString strRight;
        ::WizStringSimpleSplit(strUserId, '@', strLeft, strRight);
        pEvents->OnStatus(strLeft);
        //
        if (SUCCEEDED(URLDownloadToFile(NULL, strURL, strFileName, 0, NULL)))
        {
            if (strUserId == strCurrentUserID)
            {
                pEvents->OnSyncStep(wizsyncstepUserAvatarDownloaded, 0);
            }
        }
    }
    //
    settings.SaveToUnicodeFile(strSettingsFileName);
    */
}
//

//TODO: get url by api.wiz.cn
CString WizKMGetAccountsServerURL(BOOL bUseWizServer)
{
    return "https://as.wiz.cn/wizas/xmlrpc";
}

BOOL WizSyncDatabase(IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, BOOL bUseWizServer, BOOL bBackground)
{
    pEvents->OnStatus(_TR("-------Sync start--------------"));
    pEvents->OnSyncProgress(0);
    pEvents->OnStatus(_TR("Connecting to server"));
    //
    CWizKMAccountsServer server(WizKMGetAccountsServerURL(bUseWizServer), NULL);
    //
    //
    pEvents->OnSyncProgress(::GetSyncStartProgress(syncAccountLogin));
    //
    pEvents->OnStatus(_TR("Signing in"));
    QString strPassword = pDatabase->GetPassword();
    while (1)
    {
        if (server.Login(pDatabase->GetUserId(), strPassword, _T("normal")))
            break;
        //
        if (server.GetLastErrorCode() != 301)
        {
            return FALSE;
        }
        //
        //TODO: prompt user enter a new password
        return FALSE;
        //strPassword = pDatabase->QueryNewPassword(pEvents->GetWindow());
        //if (strPassword.isEmpty())
        //    return FALSE;
    }
    //
    pDatabase->SetUserInfo(server.GetUserInfo());
    //
    pEvents->OnSyncProgress(1);
    //
    /*
    ////获得群组信息////
    */
    CWizGroupDataArray arrayGroup;
    if (server.GetGroupList(arrayGroup)
        && !arrayGroup.empty())
    {
        pDatabase->OnDownloadGroups(arrayGroup);
    }
    //
    int groupCount = int(arrayGroup.size());
    pEvents->SetDatabaseCount(groupCount + 1);
    //
    /*
    ////下载设置////
    */
    pEvents->OnStatus(_TR("Downloading settings"));
    DownloadAccountKeys(server, pDatabase);
    //
    /*
    ////下载消息////
    */
    pEvents->OnStatus(_TR("Downloading messages"));
    WizDownloadMessages(pEvents, server, pDatabase, arrayGroup);
    //
    //
    pEvents->OnStatus(_TR("-------sync private notes--------------"));
    //
    {
        pEvents->SetCurrentDatabase(0);
        CWizKMSync syncPrivate(pDatabase, server.m_retLogin, pEvents, FALSE, FALSE, NULL);
        //
        if (!syncPrivate.Sync())
        {
            pEvents->OnText(wizhttpstatustypeError, _T("Cannot sync!"));
        }
        else
        {
            pDatabase->SaveLastSyncTime();
            pEvents->OnSyncProgress(100);
        }
    }
    //
    if (pEvents->IsStop())
        return FALSE;
    //
    pEvents->OnStatus(_TR("-------sync groups--------------"));
    //
    for (CWizGroupDataArray::const_iterator itGroup = arrayGroup.begin();
        itGroup != arrayGroup.end();
        itGroup++)
    {
        pEvents->SetCurrentDatabase(1 + int(itGroup - arrayGroup.begin()));
        //
        if (pEvents->IsStop())
            return FALSE;
        //
        WIZGROUPDATA group = *itGroup;
        //
        pEvents->OnSyncProgress(0);
        pEvents->OnStatus(WizFormatString1(_TR("-------Sync group: %1--------------"), group.strGroupName));
        //
        IWizSyncableDatabase* pGroupDatabase = pDatabase->GetGroupDatabase(group);
        if (!pGroupDatabase)
        {
            pEvents->OnError(WizFormatString1(_T("Cannot open group: %1"), group.strGroupName));
            continue;
        }
        //
        WIZKMUSERINFO userInfo = server.m_retLogin;
        userInfo.strDatabaseServer = group.strDatabaseServer;
        userInfo.strKbGUID = group.strGroupGUID;
        //
        //
        CWizKMSync syncGroup(pGroupDatabase, userInfo, pEvents, TRUE, FALSE, NULL);
        //
        if (syncGroup.Sync())
        {
            pGroupDatabase->SaveLastSyncTime();
            pEvents->OnStatus(WizFormatString1(_TR("Sync group %1 done"), group.strGroupName));
        }
        else
        {
            pEvents->OnError(WizFormatString1(_TR("Cannot sync group %1"), group.strGroupName));
            pEvents->OnSyncProgress(100);
        }
        //
        pDatabase->CloseGroupDatabase(pGroupDatabase);
    }
    //
    //
    WizDownloadUserAvatars(pEvents, pDatabase, bBackground);
    //
    pEvents->OnStatus(_TR("-------Downloading notes--------------"));
    //
    {
        //pEvents->SetCurrentDatabase(0);
        CWizKMSync syncPrivate(pDatabase, server.m_retLogin, pEvents, FALSE, FALSE, NULL);
        //
        if (!syncPrivate.DownloadObjectData())
        {
            pEvents->OnText(wizhttpstatustypeError, _T("Cannot sync!"));
        }
        else
        {
            pDatabase->SaveLastSyncTime();
            //pEvents->OnSyncProgress(100);
        }
    }
    //
    if (pEvents->IsStop())
        return FALSE;
    //
    for (CWizGroupDataArray::const_iterator itGroup = arrayGroup.begin();
        itGroup != arrayGroup.end();
        itGroup++)
    {
        pEvents->SetCurrentDatabase(1 + int(itGroup - arrayGroup.begin()));
        //
        if (pEvents->IsStop())
            return FALSE;
        //
        WIZGROUPDATA group = *itGroup;
        //
        //pEvents->OnSyncProgress(0);
        //pEvents->OnStatus(WizFormatString1(_TR("-------Sync group: %1--------------"), group.strGroupName));
        //
        IWizSyncableDatabase* pGroupDatabase = pDatabase->GetGroupDatabase(group);
        if (!pGroupDatabase)
        {
            pEvents->OnError(WizFormatString1(_T("Cannot open group: %1"), group.strGroupName));
            continue;
        }
        //
        WIZKMUSERINFO userInfo = server.m_retLogin;
        userInfo.strDatabaseServer = group.strDatabaseServer;
        userInfo.strKbGUID = group.strGroupGUID;
        //
        //
        CWizKMSync syncGroup(pGroupDatabase, userInfo, pEvents, TRUE, FALSE, NULL);
        //
        if (syncGroup.DownloadObjectData())
        {
            pGroupDatabase->SaveLastSyncTime();
            //pEvents->OnStatus(WizFormatString1(_TR("Sync group %1 done"), group.strGroupName));
        }
        else
        {
            pEvents->OnError(WizFormatString1(_TR("Cannot sync group %1"), group.strGroupName));
            //pEvents->OnSyncProgress(100);
        }
        //
        pDatabase->CloseGroupDatabase(pGroupDatabase);
    }
    //
    pEvents->OnStatus(_TR("-------Sync done--------------"));
    //
    return TRUE;
}



BOOL WizUploadDatabase(IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, const WIZUSERINFOBASE& info, BOOL bGroup)
{
    CWizKMSync sync(pDatabase, info, pEvents, bGroup, TRUE, NULL);
    BOOL bRet = sync.Sync();
    //
    return bRet;
}
BOOL WizSyncDatabaseOnly(IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, const WIZUSERINFOBASE& info, BOOL bGroup)
{
    CWizKMSync sync(pDatabase, info, pEvents, bGroup, FALSE, NULL);
    BOOL bRet = sync.Sync();
    if (bRet)
    {
        sync.DownloadObjectData();
    }
    //
    return bRet;
}

