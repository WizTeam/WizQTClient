#ifndef WIZKMXMLRPC_H
#define WIZKMXMLRPC_H

#include "share/WizMessageBox.h"
#include "share/WizRequest.h"
#include "share/WizObject.h"
#include "WizDef.h"

struct IWizKMSyncEvents;

#define WIZKM_XMLRPC_ERROR_TRAFFIC_LIMIT		304
#define WIZKM_XMLRPC_ERROR_STORAGE_LIMIT		305
#define WIZKM_XMLRPC_ERROR_NOTE_COUNT_LIMIT		3032
#define WIZKM_XMLRPC_ERROR_BIZ_SERVICE_EXPR		380

#define WIZKM_XMLRPC_ERROR_FREE_SERVICE_EXPR    30321
#define WIZKM_XMLRPC_ERROR_VIP_SERVICE_EXPR     30322


//返回的网络错误。 此处使用客户端自定义的错误代码
#define WIZKM_XMLRPC_ERROR_INVALID_TOKEN		301
#define WIZKM_XMLRPC_ERROR_INVALID_USER			31001
#define WIZKM_XMLRPC_ERROR_INVALID_PASSWORD		31002
#define WIZKM_XMLRPC_ERROR_TOO_MANY_LOGINS		31004

#define WIZKM_XMLRPC_ERROR_SYSTEM_ERROR         60000


#define WIZKM_WEBAPI_VERSION		10


class WizKMApiServerBase : public QObject
{
public:
    WizKMApiServerBase(const QString& strServer, QObject* parent);
    void setEvents(IWizKMSyncEvents* pEvents) { m_pEvents = pEvents; }
protected:
    QString m_strServer;
    WIZSTANDARDRESULT m_lastError;
    IWizKMSyncEvents* m_pEvents;

public:
    QString getServer() const { return m_strServer; }
    virtual QString getToken() const = 0;
    virtual QString getKbGuid() const = 0;
    virtual QString buildUrl(QString urlPath);
    //
    bool isNetworkError() const { return m_lastError.isNetworkError(); }
    int getLastErrorCode() const { return m_lastError.returnCode; }
    QString getLastErrorMessage() const { return m_lastError.returnMessage; }
    //
    WIZSTANDARDRESULT setLastError(const WIZSTANDARDRESULT& ret) { m_lastError = ret; onApiError(); return ret; }
    //
    void onApiError();
    //
    bool getValueVersion(const QString& strMethodPrefix, const QString& strGuid, const QString& strKey, __int64& nVersion);
    bool getValue(const QString& strMethodPrefix, const QString& strGuid, const QString& strKey, QString& strValue, __int64& nVersion);
    bool setValue(const QString& strMethodPrefix, const QString& strGuid, const QString& strKey, const QString& strValue, __int64& nRetVersion);
};


class WizKMAccountsServer : public WizKMApiServerBase
{
public:
    WizKMAccountsServer(QObject* parent = 0);
    virtual ~WizKMAccountsServer(void);

protected:
    bool m_bLogin;
    bool m_bAutoLogout;

public:
    WIZUSERINFO m_userInfo;
    std::map<QString, WIZKBINFO> m_kbInfos;
    std::map<QString, WIZKBVALUEVERSIONS> m_valueVersions;
public:
    void setAutoLogout(bool b) { m_bAutoLogout = b; }
    //
    bool login(const QString& strUserName, const QString& strPassword);
    bool logout();
    bool keepAlive();
    //
    bool getToken(const QString& strUserName, const QString& strPassword, QString& strToken);
    bool createAccount(const QString& strUserName, const QString& strPassword, const QString& InviteCode, const QString& strCaptchaID, const QString& strCaptcha);
    //
    bool getCert(QString& strN, QString& stre, QString& strd, QString& strHint);
    bool setCert(const QString& strN, const QString& stre, const QString& strd, const QString& strHint);
    //
    bool getGroupList(CWizGroupDataArray& arrayGroup);
    bool getBizList(CWizBizDataArray& arrayBiz);
    //
    bool getMessages(__int64 nStartVersion, CWizMessageDataArray& arrayMessage);
    bool setMessageReadStatus(const QString& strMessageIDs, int nStatus);
    bool setMessageDeleteStatus(const QString &strMessageIDs, int nStatus);
    //
    bool getBizUsers(const QString& bizGuid, const QString& kbGuid, CWizBizUserDataArray& arrayUser);
    bool getKbInfos(std::deque<WIZKBINFO>& arrayInfo);
    bool getValueVersions(std::deque<WIZKBVALUEVERSIONS>& arrayVersion);
    //
    bool getAdminBizCert(const QString& strBizGUID, QString& strN, QString& stre, QString& strd, QString& strHint);
    bool setUserBizCert(const QString& strBizGUID, const QString& strN, const QString& stre, const QString& strd, const QString& strHint);
    bool getUserBizCert(const QString& strBizGUID, QString& strN, QString& stre, QString& strd, QString& strHint);
    //
    bool getValueVersion(const QString& strKey, __int64& nVersion);
    bool getValue(const QString& strKey, QString& strValue, __int64& nVersion);
    bool setValue(const QString& strKey, const QString& strValue, __int64& nRetVersion);
public:
    QString getToken() const;
    QString getKbGuid() const;
    const WIZUSERINFO& getUserInfo() const { return m_userInfo; }
    WIZUSERINFO& getUserInfo() { return m_userInfo; }
    void setUserInfo(const WIZUSERINFO& userInfo);
    //
    bool initAllKbInfos();
    WIZKBINFO getKbInfo(QString kbGuid) const;
    //
    bool initAllValueVersions();
    WIZKBVALUEVERSIONS getValueVersions(QString kbGuid) const;
};



class WizKMDatabaseServer: public WizKMApiServerBase
{
    Q_OBJECT
public:
    WizKMDatabaseServer(const WIZUSERINFOBASE& userInfo, const WIZKBINFO& kbInfo = WIZKBINFO(), const WIZKBVALUEVERSIONS& versions = WIZKBVALUEVERSIONS(), QObject* parent = 0);
    virtual ~WizKMDatabaseServer();

    const WIZKBINFO& kbInfo();
    void setKBInfo(const WIZKBINFO& info);
    const WIZUSERINFOBASE& userInfo() const { return m_userInfo; }
    //
    bool isGroup() const;
    //
    void clearJsonResult() { m_lastJsonResult = WIZSTANDARDRESULT(200, QString(""), QString("")); }
    WIZSTANDARDRESULT lastJsonResult() const { return m_lastJsonResult; }
    void clearLocalError() { m_strLastLocalError.clear(); }
    QString lastLocalError() const { return m_strLastLocalError; }
protected:
    WIZUSERINFOBASE m_userInfo;
    WIZKBINFO m_kbInfo;
    WIZKBVALUEVERSIONS m_valueVersions;
    //
    WIZSTANDARDRESULT m_lastJsonResult;
    QString m_strLastLocalError;
    //
    qint64 m_objectsTotalSize;
    std::map<QString, qint64> m_objectDownloadedSize;
private:
    bool document_postDataNew(const WIZDOCUMENTDATAEX& data, bool bWithDocumentData, __int64& nServerVersion);
    bool document_downloadDataNew(const QString& strDocumentGUID, WIZDOCUMENTDATAEX& ret, const QString& fileName);
    bool attachment_postDataNew(WIZDOCUMENTATTACHMENTDATAEX& data, bool withData, __int64& nServerVersion);
    bool attachment_downloadDataNew(const QString& strDocumentGUID, const QString& strAttachmentGUID, WIZDOCUMENTATTACHMENTDATAEX& ret);

public:
    QString getToken() const { return m_userInfo.strToken; }
    QString getKbGuid() const { return m_userInfo.strKbGUID; }
    int getMaxFileSize() const { return m_kbInfo.getMaxFileSize(); }

    bool kb_getInfo();
    //
    bool document_downloadData(const QString& strDocumentGUID, WIZDOCUMENTDATAEX& ret, const QString& oldFileName);
    bool attachment_downloadData(const QString& strDocumentGUID, const QString& strAttachmentGUID, WIZDOCUMENTATTACHMENTDATAEX& ret);
    //
    bool document_postData(const WIZDOCUMENTDATAEX& data, bool bWithDocumentData, __int64& nServerVersion);
    bool attachment_postData(WIZDOCUMENTATTACHMENTDATAEX& data, bool withData, __int64& nServerVersion);
    //
    bool document_getList(int nCountPerPage, __int64 nStartVersion, std::deque<WIZDOCUMENTDATAEX>& arrayRet);
    //
    bool attachment_getList(int nCountPerPage, __int64 nStartVersion, std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet);
    //
    bool tag_getList(int nCountPerPage, __int64 nStartVersion, std::deque<WIZTAGDATA>& arrayRet);
    bool tag_postList(std::deque<WIZTAGDATA>& arrayTag);
    //
    bool style_getList(int nCountPerPage, __int64 nStartVersion, std::deque<WIZSTYLEDATA>& arrayRet);
    bool style_postList(const std::deque<WIZSTYLEDATA>& arrayStyle);
    //
    bool param_getList(int nCountPerPage, __int64 nStartVersion, std::deque<WIZDOCUMENTPARAMDATA>& arrayRet);
    bool param_postList(const std::deque<WIZDOCUMENTPARAMDATA>& arrayParam);
    //
    bool deleted_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDELETEDGUIDDATA>& arrayRet);
    bool deleted_postList(std::deque<WIZDELETEDGUIDDATA>& arrayDeletedGUID);
    //
    bool document_getListByGuids(const CWizStdStringArray& arrayDocumentGUID, std::deque<WIZDOCUMENTDATAEX>& arrayRet);
    bool document_getInfo(const QString& strDocumentGuid, WIZDOCUMENTDATAEX& doc);

    bool getValueVersion(const QString& strKey, __int64& nVersion);
    bool getValue(const QString& strKey, QString& strValue, __int64& nVersion);
    bool setValue(const QString& strKey, const QString& strValue, __int64& nRetVersion);
    //
    bool getCommentCount(const QString& strDocumentGuid, int& commentCount);

public:
    virtual int getCountPerPage();

signals:
    void downloadProgress(int totalSize, int loadedSize);

public slots:
    void onDocumentObjectDownloadProgress(QUrl url, qint64 downloadSize, qint64 totalSize);

public:
    //

    //
    /////////////////////////////////////////////////////
    ////获得所有的对象列表//
    //
    template <class TData>
    bool getList(int nCountPerPage, __int64 nVersion, std::deque<TData>& arrayRet)
    {
        ATLASSERT(FALSE);
        return FALSE;
    }
    template <class TData>
    bool getList(int nCountPerPage, __int64 nVersion, std::deque<WIZTAGDATA>& arrayRet)
    {
        return tag_getList(nCountPerPage, nVersion, arrayRet);
    }
    template <class TData>
    bool getList(int nCountPerPage, __int64 nVersion, std::deque<WIZSTYLEDATA>& arrayRet)
    {
        return style_getList(nCountPerPage, nVersion, arrayRet);
    }
    template <class TData>
    bool getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDELETEDGUIDDATA>& arrayRet)
    {
        return deleted_getList(nCountPerPage, nVersion, arrayRet);
    }
    template <class TData>
    bool getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDOCUMENTDATAEX>& arrayRet)
    {
        return document_getList(nCountPerPage, nVersion, arrayRet);
    }
    template <class TData>
    bool getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
    {
        return attachment_getList(nCountPerPage, nVersion, arrayRet);
    }
    template <class TData>
    bool getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDOCUMENTPARAMDATA>& arrayRet)
    {
        return param_getList(nCountPerPage, nVersion, arrayRet);
    }

    template <class TData>
    bool postList(std::deque<TData>& arrayData)
    {
        ATLASSERT(FALSE);
        return FALSE;
    }
    //
    template <class TData>
    bool postList(std::deque<WIZTAGDATA>& arrayData)
    {
        return tag_postList(arrayData);
    }
    template <class TData>
    bool postList(std::deque<WIZSTYLEDATA>& arrayData)
    {
        return style_postList(arrayData);
    }
    template <class TData>
    bool postList(std::deque<WIZDELETEDGUIDDATA>& arrayData)
    {
        return deleted_postList(arrayData);
    }
    template <class TData>
    bool postList(std::deque<WIZDOCUMENTPARAMDATA>& arrayData)
    {
        return param_postList(arrayData);
    }
    //
    template <class TData>
    bool postData(TData& data, bool bWithData, __int64& nServerVersion)
    {
        ATLASSERT(FALSE);
        return FALSE;
    }
    //
    template <class TData>
    bool postData(WIZDOCUMENTDATAEX& data, bool bWithData, __int64& nServerVersion)
    {
        return document_postData(data, bWithData, nServerVersion);
    }
    template <class TData>
    bool postData(WIZDOCUMENTATTACHMENTDATAEX& data, bool bWithData, __int64& nServerVersion)
    {
        return attachment_postData(data, bWithData, nServerVersion);
    }

};


enum WizKMSyncProgress
{
    syncAccountLogin = 0,
    syncDatabaseLogin,
    syncDownloadDeletedList,
    syncUploadDeletedList,
    syncUploadTagList,
    syncUploadStyleList,
    syncUploadDocumentList,
    syncUploadParamList,
    syncUploadAttachmentList,
    syncDownloadTagList,
    syncDownloadStyleList,
    syncDownloadSimpleDocumentList,
    syncDownloadFullDocumentList,
    syncDownloadParamList,
    syncDownloadAttachmentList,
    syncDownloadObjectData
};

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

void GetSyncProgressRange(WizKMSyncProgress progress, int& start, int& count);
int GetSyncStartProgress(WizKMSyncProgress progress);


#define _TR(x) x




#endif // WIZKMXMLRPC_H
