#ifndef WIZKMXMLRPC_H
#define WIZKMXMLRPC_H

#include "../share/wizXmlRpcServer.h"
#include "../share/wizSyncableDatabase.h"

struct IWizKMSyncEvents;
struct  IWizSyncableDatabase;

bool WizSyncDatabase(IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, bool bUseWizServer, bool bBackground);
CString WizKMGetAccountsServerURL(BOOL bUseWizServer);

class CWizKMXmlRpcServerBase : public CWizXmlRpcServerBase
{
public:
    CWizKMXmlRpcServerBase(const QString& strUrl, QObject* parent);
    bool GetValueVersion(const QString& strMethodPrefix, const QString& strToken, const QString& strKbGUID, const QString& strKey, __int64& nVersion);
    bool GetValue(const QString& strMethodPrefix, const QString& strToken, const QString& strKbGUID, const QString& strKey, QString& strValue, __int64& nVersion);
    bool SetValue(const QString& strMethodPrefix, const QString& strToken, const QString& strKbGUID, const QString& strKey, const QString& strValue, __int64& nRetVersion);
};


class CWizKMAccountsServer : public CWizKMXmlRpcServerBase
{
public:
    CWizKMAccountsServer(const QString& strUrl, QObject* parent = 0);
    virtual ~CWizKMAccountsServer(void);

    virtual void OnXmlRpcError();

protected:
    bool m_bLogin;
    bool m_bAutoLogout;

public:
    WIZKMUSERINFO m_retLogin;

public:
    bool Login(const QString& strUserName, const QString& strPassword, const QString& strType);
    bool Logout();
    bool ChangePassword(const QString& strUserName, const QString& strOldPassword, const QString& strNewPassword);
    bool ChangeUserId(const QString& strUserName, const QString& strPassword, const QString& strNewUserId);
    bool GetToken(const QString& strUserName, const QString& strPassword, QString& strToken);
    bool GetCert(const QString& strUserName, const QString& strPassword, QString& strN, QString& stre, QString& strd, QString& strHint);
    bool SetCert(const QString& strUserName, const QString& strPassword, const QString& strN, const QString& stre, const QString& strd, const QString& strHint);
    bool CreateAccount(const QString& strUserName, const QString& strPassword, const QString& InviteCode);
    void SetAutoLogout(bool b) { m_bAutoLogout = b; }
    bool ShareSNS(const QString& strToken, const QString& strSNS, const QString& strComment, const QString& strURL, const QString& strDocumentGUID);
    bool ShareGroup(const QString& strToken, const QString& strDocumentGUIDs, const QString& strGroups);
    bool GetGroupList(CWizGroupDataArray& arrayGroup);
    bool CreateTempGroup(const QString& strEmails, const QString& strAccessControl, const QString& strSubject, const QString& strEmailText, WIZGROUPDATA& group);
    bool KeepAlive(const QString& strToken);
    bool GetMessages(__int64 nVersion, CWizUserMessageDataArray& arrayMessage);
    bool SetMessageReadStatus(const QString& strMessageIDs, int nStatus);

    bool GetValueVersion(const QString& strKey, __int64& nVersion);
    bool GetValue(const QString& strKey, QString& strValue, __int64& nVersion);
    bool SetValue(const QString& strKey, const QString& strValue, __int64& nRetVersion);

public:
    bool GetWizKMDatabaseServer(QString& strServer, int& nPort, QString& strXmlRpcFile);
    QString GetToken();
    QString GetKbGUID();
    int GetMaxFileSize() { return m_retLogin.GetMaxFileSize(); }
    const WIZKMUSERINFO& GetUserInfo() const { return m_retLogin; }
    WIZKMUSERINFO& GetUserInfo() { return m_retLogin; }
    void SetUserInfo(const WIZKMUSERINFO& userInfo) { m_bLogin = TRUE; m_retLogin = userInfo; }

private:
    QString MakeXmlRpcPassword(const QString& strPassword);

    bool accounts_clientLogin(const QString& strUserName, const QString& strPassword, const QString& strType, WIZKMUSERINFO& ret);
    bool accounts_clientLogout(const QString& strToken);
    bool accounts_keepAlive(const QString& strToken);
    bool accounts_createAccount(const QString& strUserName, const QString& strPassword, const QString& strInviteCode);
    bool accounts_changePassword(const QString& strUserName, const QString& strOldPassword, const QString& strNewPassword);
    bool accounts_changeUserId(const QString& strUserName, const QString& strPassword, const QString& strNewUserId);
    bool accounts_getToken(const QString& strUserName, const QString& strPassword, QString& strToken);
    bool accounts_getCert(const QString& strUserName, const QString& strPassword, QString& strN, QString& stre, QString& strd, QString& strHint);
    bool accounts_setCert(const QString& strUserName, const QString& strPassword, const QString& strN, const QString& stre, const QString& strd, const QString& strHint);
    bool document_shareSNS(const QString& strToken, const QString& strSNS, const QString& strComment, const QString& strURL, const QString& strDocumentGUID);
    bool document_shareGroup(const QString& strToken, const QString& strDocumentGUIDs, const QString& strGroups);

    bool accounts_getGroupList(CWizGroupDataArray& arrayGroup);
    bool accounts_createTempGroupKb(const QString& strEmails, const QString& strAccessControl, const QString& strSubject, const QString& strEmailText, WIZGROUPDATA& group);
    bool accounts_getMessages(int nCountPerPage, __int64 nVersion, CWizUserMessageDataArray& arrayMessage);
};


#endif // WIZKMXMLRPC_H
