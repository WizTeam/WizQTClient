#ifndef WIZKMXMLRPC_H
#define WIZKMXMLRPC_H

#include "WizXmlRpcServer.h"
#include "WizJSONServerBase.h"

#define WIZKM_XMLRPC_ERROR_TRAFFIC_LIMIT		304
#define WIZKM_XMLRPC_ERROR_STORAGE_LIMIT		305
#define WIZKM_XMLRPC_ERROR_NOTE_COUNT_LIMIT		3032
#define WIZKM_XMLRPC_ERROR_BIZ_SERVICE_EXPR		380

class WizKMXmlRpcServerBase : public WizXmlRpcServerBase
{
public:
    WizKMXmlRpcServerBase(const QString& strUrl, QObject* parent);
    bool getValueVersion(const QString& strMethodPrefix, const QString& strToken, const QString& strKbGUID, const QString& strKey, __int64& nVersion);
    bool getValue(const QString& strMethodPrefix, const QString& strToken, const QString& strKbGUID, const QString& strKey, QString& strValue, __int64& nVersion);
    bool setValue(const QString& strMethodPrefix, const QString& strToken, const QString& strKbGUID, const QString& strKey, const QString& strValue, __int64& nRetVersion);
};


class WizKMAccountsServer : public WizKMXmlRpcServerBase, public WizJSONServerBase
{
public:
    WizKMAccountsServer(const QString& strUrl, QObject* parent = 0);
    virtual ~WizKMAccountsServer(void);

    virtual void onXmlRpcError();

protected:
    bool m_bLogin;
    bool m_bAutoLogout;

public:
    WIZUSERINFO m_userInfo;

public:
    bool login(const QString& strUserName, const QString& strPassword, const QString& strType = "normal");
    bool logout();
    bool changePassword(const QString& strUserName, const QString& strOldPassword, const QString& strNewPassword);
    bool changeUserId(const QString& strUserName, const QString& strPassword, const QString& strNewUserId);
    bool getToken(const QString& strUserName, const QString& strPassword, QString& strToken);
    bool getCert(const QString& strUserName, const QString& strPassword, QString& strN, QString& stre, QString& strd, QString& strHint);
    bool setCert(const QString& strUserName, const QString& strPassword, const QString& strN, const QString& stre, const QString& strd, const QString& strHint);
    bool createAccount(const QString& strUserName, const QString& strPassword, const QString& InviteCode, const QString& strCaptchaID, const QString& strCaptcha);
    void setAutoLogout(bool b) { m_bAutoLogout = b; }
    bool shareSNS(const QString& strToken, const QString& strSNS, const QString& strComment, const QString& strURL, const QString& strDocumentGUID);
    bool getGroupList(CWizGroupDataArray& arrayGroup);
    bool getBizList(CWizBizDataArray& arrayBiz);
    bool createTempGroup(const QString& strEmails, const QString& strAccessControl, const QString& strSubject, const QString& strEmailText, WIZGROUPDATA& group);
    bool keepAlive(const QString& strToken);
    bool getMessages(__int64 nVersion, CWizUserMessageDataArray& arrayMessage);
    bool setMessageReadStatus(const QString& strMessageIDs, int nStatus);

    bool getAdminBizCert(const QString& strToken, const QString& strBizGUID, QString& strN, QString& stre, QString& strd, QString& strHint);
    bool setUserBizCert(const QString& strBizGUID, const QString& strN, const QString& stre, const QString& strd, const QString& strHint);
    bool getUserBizCert(const QString& strBizGUID, QString& strN, QString& stre, QString& strd, QString& strHint);

    //
    bool setMessageDeleteStatus(const QString &strMessageIDs, int nStatus);

    bool getValueVersion(const QString& strKey, __int64& nVersion);
    bool getValue(const QString& strKey, QString& strValue, __int64& nVersion);
    bool setValue(const QString& strKey, const QString& strValue, __int64& nRetVersion);
    //


public:
    bool getWizKMDatabaseServer(QString& strServer, int& nPort, QString& strXmlRpcFile);
    QString getToken();
    QString getKbGuid();
    //void setKbGUID(const QString& strkbGUID) { m_retLogin.strKbGUID = strkbGUID; }
    const WIZUSERINFO& getUserInfo() const { return m_userInfo; }
    WIZUSERINFO& getUserInfo() { return m_userInfo; }
    void setUserInfo(const WIZUSERINFO& userInfo);

private:
    QString makeXmlRpcPassword(const QString& strPassword);

    bool accounts_clientLogin(const QString& strUserName, const QString& strPassword, const QString& strType, WIZUSERINFO& ret);
    bool accounts_clientLogout(const QString& strToken);
    bool accounts_keepAlive(const QString& strToken);
    bool accounts_createAccount(const QString& strUserName, const QString& strPassword, const QString& strInviteCode, const QString& strCaptchaID, const QString& strCaptcha);
    bool accounts_changePassword(const QString& strUserName, const QString& strOldPassword, const QString& strNewPassword);
    bool accounts_changeUserId(const QString& strUserName, const QString& strPassword, const QString& strNewUserId);
    bool accounts_getToken(const QString& strUserName, const QString& strPassword, QString& strToken);
    bool accounts_getCert(const QString& strUserName, const QString& strPassword, QString& strN, QString& stre, QString& strd, QString& strHint);
    bool accounts_setCert(const QString& strUserName, const QString& strPassword, const QString& strN, const QString& stre, const QString& strd, const QString& strHint);
    bool document_shareSNS(const QString& strToken, const QString& strSNS, const QString& strComment, const QString& strURL, const QString& strDocumentGUID);

    bool accounts_getGroupList(CWizGroupDataArray& arrayGroup);
    bool accounts_getBizList(CWizBizDataArray& arrayBiz);
    bool accounts_createTempGroupKb(const QString& strEmails, const QString& strAccessControl, const QString& strSubject, const QString& strEmailText, WIZGROUPDATA& group);
    bool accounts_getMessagesByXmlrpc(int nCountPerPage, __int64 nVersion, CWizUserMessageDataArray& arrayMessage);

    //
    bool accounts_getMessagesByJson(int nCountPerPage, __int64 nVersion, CWizUserMessageDataArray& arrayMessage);
};



#define WIZKM_WEBAPI_VERSION		9

struct CWizKMBaseParam: public WizXmlRpcStructValue
{
    CWizKMBaseParam(int apiVersion = WIZKM_WEBAPI_VERSION)
    {
        changeApiVersion(apiVersion);
        //
#ifdef Q_OS_MAC
        addString(_T("client_type"), _T("mac"));
#else
        addString(_T("client_type"), _T("linux"));
#endif
        addString(_T("client_version"), _T("2.0"));
    }
    //
    void changeApiVersion(int nApiVersion)
    {
        addString(_T("api_version"), WizIntToStr(nApiVersion));
    }
    int getApiVersion()
    {
        QString str;
        getString(_T("api_version"), str);
        return _ttoi(str);
    }
};

struct CWizKMTokenOnlyParam : public CWizKMBaseParam
{
    CWizKMTokenOnlyParam(const QString& strToken, const QString& strKbGUID)
    {
        addString(_T("token"), strToken);
        addString(_T("kb_guid"), strKbGUID);
    }
};


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

class WizKMDatabaseServer: public WizKMXmlRpcServerBase
{
    Q_OBJECT
public:
    WizKMDatabaseServer(const WIZUSERINFOBASE& kbInfo, QObject* parent = 0);
    virtual ~WizKMDatabaseServer();
    virtual void onXmlRpcError();

    const WIZKBINFO& kbInfo();
    void setKBInfo(const WIZKBINFO& info);

protected:
    WIZUSERINFOBASE m_userInfo;
    WIZKBINFO m_kbInfo;

public:
    QString getToken() const { return m_userInfo.strToken; }
    QString getKbGuid() const { return m_userInfo.strKbGUID; }
    int getMaxFileSize() const { return m_kbInfo.getMaxFileSize(); }

    BOOL wiz_getInfo();
    BOOL wiz_getVersion(WIZOBJECTVERSION& version, BOOL bAuto = FALSE);

    BOOL document_downloadData(const QString& strDocumentGUID, WIZDOCUMENTDATAEX& ret);
    BOOL attachment_downloadData(const QString& strAttachmentGUID, WIZDOCUMENTATTACHMENTDATAEX& ret);
    //
    BOOL document_postData(const WIZDOCUMENTDATAEX& data, bool bWithDocumentData, __int64& nServerVersion);
    BOOL attachment_postData(WIZDOCUMENTATTACHMENTDATAEX& data, __int64& nServerVersion);
    //
    BOOL document_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDOCUMENTDATAEX>& arrayRet);
    BOOL attachment_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet);
    BOOL tag_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZTAGDATA>& arrayRet);
    BOOL style_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZSTYLEDATA>& arrayRet);
    BOOL deleted_getList(int nCountPerPage, __int64 nVersion, std::deque<WIZDELETEDGUIDDATA>& arrayRet);

    BOOL tag_postList(std::deque<WIZTAGDATA>& arrayTag);
    BOOL style_postList(std::deque<WIZSTYLEDATA>& arrayStyle);
    BOOL deleted_postList(std::deque<WIZDELETEDGUIDDATA>& arrayDeletedGUID);
    QByteArray downloadDocumentData(const QString& strDocumentGUID);
    QByteArray downloadAttachmentData(const QString& strAttachmentGUID);
    //
    BOOL document_getListByGuids(const CWizStdStringArray& arrayDocumentGUID, std::deque<WIZDOCUMENTDATAEX>& arrayRet);
    BOOL document_getInfo(const QString& strDocumentGuid, WIZDOCUMENTDATAEX& doc);

    BOOL category_getAll(QString& str);

    BOOL data_download(const QString& strObjectGUID, const QString& strObjectType, QByteArray& stream, const QString& strDisplayName);
    BOOL data_upload(const QString& strObjectGUID, const QString& strObjectType, const QByteArray& stream, const QString& strObjMD5, const QString& strDisplayName);
    //
    BOOL getValueVersion(const QString& strKey, __int64& nVersion);
    BOOL getValue(const QString& strKey, QString& strValue, __int64& nVersion);
    BOOL setValue(const QString& strKey, const QString& strValue, __int64& nRetVersion);

public:
    virtual int getCountPerPage();

signals:
    void downloadProgress(int totalSize, int loadedSize);

protected:
    BOOL data_download(const QString& strObjectGUID, const QString& strObjectType, int pos, int size, QByteArray& stream, int& nAllSize, BOOL& bEOF);
    BOOL data_upload(const QString& strObjectGUID, const QString& strObjectType, const QString& strObjectMD5, int allSize, int partCount, int partIndex, int partSize, const QByteArray& stream);
    //
    ////////////////////////////////////////////////////////////
    //downloadList
    ////通过GUID列表，下载完整的对象信息列表，和getList不同，getList对于文档，仅下载有限的信息，例如md5信息等等////
    //
    //
    template <class TData, class TWrapData>
    BOOL downloadListCore(const QString& strMethodName, const QString& strGUIDArrayValueName, const CWizStdStringArray& arrayGUID, std::deque<TData>& arrayRet)
    {
        if (arrayGUID.empty())
            return TRUE;
        //
        CWizKMTokenOnlyParam param(m_userInfo.strToken, m_userInfo.strKbGUID);
        param.addStringArray(strGUIDArrayValueName, arrayGUID);
        //
        std::deque<TWrapData> arrayWrap;
        if (!call(strMethodName, arrayWrap, &param))
        {
            TOLOG1(_T("%1 failure!"), strMethodName);
            return FALSE;
        }
        //
        arrayRet.assign(arrayWrap.begin(), arrayWrap.end());
        //
        return TRUE;
    }


    template <class TData, class TWrapData>
    BOOL downloadList(const QString& strMethodName, const QString& strGUIDArrayValueName, const CWizStdStringArray& arrayGUID, std::deque<TData>& arrayRet)
    {
        int nCountPerPage = 30;
        //
        CWizStdStringArray::const_iterator it = arrayGUID.begin();
        //
        while (1)
        {
            CWizStdStringArray subArray;
            //
            for (;
                 it != arrayGUID.end(); )
            {
                subArray.push_back(*it);
                it++;
                //
                if (subArray.size() == nCountPerPage)
                    break;
            }
            //
            std::deque<TData> subRet;
            if (!downloadListCore<TData, TWrapData>(strMethodName, strGUIDArrayValueName, subArray, subRet))
                return FALSE;
            //
            arrayRet.insert(arrayRet.end(), subRet.begin(), subRet.end());
            //
            if (it == arrayGUID.end())
                break;
        }
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
        int nCountPerPage = getCountPerPage();
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

            CWizKMTokenOnlyParam param(m_userInfo.strToken, m_userInfo.strKbGUID);
            //
            param.addArray<TWrapData>(strArrayName, subArray);
            //
            QString strCount;
            if (!call(strMethosName, _T("success_count"), strCount, &param))
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
    //
    /////////////////////////////////////////////////////////
    ////下载对象数据/////////////////

    template <class TData>
    BOOL downloadObjectData(TData& data)
    {
        return TRUE;
    }
    template <class TData>
    BOOL downloadObjectData(WIZDOCUMENTDATAEX& data)
    {
        return document_downloadData(data.strGUID, data);
    }
    template <class TData>
    BOOL downloadObjectData(WIZDOCUMENTATTACHMENTDATAEX& data)
    {
        return attachment_downloadData(data.strGUID, data);
    }


    /////////////////////////////////////////////
    //getList
    ////通过版本号获得对象列表////
    //

    template <class TData, class TWrapData>
    BOOL getList(const QString& strMethodName, int nCountPerPage, __int64 nVersion, std::deque<TData>& arrayRet)
    {
        CWizKMTokenOnlyParam param(m_userInfo.strToken, m_userInfo.strKbGUID);
        param.addInt(_T("count"), nCountPerPage);
        param.addString(_T("version"), WizInt64ToStr(nVersion));
        //
        std::deque<TWrapData> arrayWrap;
        if (!call(strMethodName, arrayWrap, &param))
        {
            TOLOG(_T("object.getList failure!"));
            return FALSE;
        }
        //
        arrayRet.assign(arrayWrap.begin(), arrayWrap.end());
        //
        return TRUE;
    }

public:
    //

    //
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
    ///////////////////////////////////////////////////////
    ////下载列表//////////////
    //
    template <class TData>
    BOOL downloadSimpleList(const CWizStdStringArray& arrayGUID, std::deque<TData>& arrayData)
    {
        return TRUE;
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
    BOOL postData(TData& data, bool bWithData, __int64& nServerVersion)
    {
        ATLASSERT(FALSE);
        return FALSE;
    }
    //
    template <class TData>
    BOOL postData(WIZDOCUMENTDATAEX& data, bool bWithData, __int64& nServerVersion)
    {
        return document_postData(data, bWithData, nServerVersion);
    }
    template <class TData>
    BOOL postData(WIZDOCUMENTATTACHMENTDATAEX& data, bool bWithData, __int64& nServerVersion)
    {
        return attachment_postData(data, nServerVersion);
    }
public:
    //
    BOOL getDocumentInfoOnServer(const QString& strDocumentGUID, WIZDOCUMENTDATAEX& dataServer)
    {
        CWizStdStringArray arrayDocument;
        arrayDocument.push_back(strDocumentGUID);
        //
        CWizDocumentDataArray arrayData;
        if (!downloadSimpleList<WIZDOCUMENTDATAEX>(arrayDocument, arrayData))
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
