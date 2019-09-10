#include "WizKMServer.h"
#include "WizApiEntry.h"
#include "WizToken.h"
//
#include "share/jsoncpp/json/json.h"
#include "share/WizZip.h"
#include "share/WizRequest.h"

#include "share/jsoncpp/json/json.h"
#include "share/WizEventLoop.h"
#include "share/WizRequest.h"
#include "share/WizThreads.h"

#include "utils/WizMisc.h"
#include "utils/WizLogger.h"
#include "share/WizDatabase.h"
#include "share/WizDatabaseManager.h"

#include "WizMainWindow.h"

#include "utils/WizPathResolve.h"

#include <QHttpPart>



QString appendNormalParams(const QString& strUrl, const QString& token)
{
    QString url = strUrl;
    if (-1 == url.indexOf("?"))
    {
        url += "?";
    }
    else
    {
        url += "&";
    }
    //
    url += QString("clientType=macos&clientVersion=") + WIZ_CLIENT_VERSION + "&apiVersion=" + WizIntToStr(WIZKM_WEBAPI_VERSION);
    //
    if (!token.isEmpty())
    {
        url += "&token=" + token;
    }
    //
    url = WizOfficialApiEntry::appendSrc(url);
    //
    return url;
}

/*
 * no result
 * */
namespace NoResult {

static WIZSTANDARDRESULT execStandardJsonRequest(WizKMApiServerBase& server, QString urlPath, const QString& method, const QByteArray &reqBody)
{
    QString url = server.buildUrl(urlPath);
    //
    Json::Value res;
    WIZSTANDARDRESULT ret = WizRequest::execStandardJsonRequest(url, method, reqBody, res);
    return ret;
}

static WIZSTANDARDRESULT execStandardJsonRequest(WizKMApiServerBase& server, QString urlPath, const QString& method, const Json::Value &reqBody)
{
    QString url = server.buildUrl(urlPath);
    //
    Json::Value res;
    WIZSTANDARDRESULT ret = WizRequest::execStandardJsonRequest(url, method, reqBody, res);
    return ret;
}

static WIZSTANDARDRESULT execStandardJsonRequest(WizKMApiServerBase& server, QString urlPath, const QString& method)
{
    WIZSTANDARDRESULT ret = execStandardJsonRequest(server, urlPath, method, QByteArray());
    return ret;
}

static WIZSTANDARDRESULT execStandardJsonRequest(WizKMApiServerBase& server, QString urlPath)
{
    WIZSTANDARDRESULT ret = execStandardJsonRequest(server, urlPath, "GET", QByteArray());
    return ret;
}

}

/*
 * with result
 * */

namespace WithResult {
template <class TData, class TPost, class TConverter>
static WIZSTANDARDRESULT execStandardJsonRequest(WizKMApiServerBase& server, QString urlPath, TData& data, const QString& method, const TPost &reqBody, TConverter convert)
{
    QString url = server.buildUrl(urlPath);
    //
    Json::Value res;
    WIZSTANDARDRESULT ret = WizRequest::execStandardJsonRequest(url, method, reqBody, res);
    if (!ret)
    {
        return server.setLastError(ret);
    }
    //
    try {
        Json::Value result = res["result"];
        bool parseRet = convert(result, data);
        if (!parseRet)
        {
            qDebug() << "invalid json format: failed to convert result to object data";
            ret = WIZSTANDARDRESULT(WIZSTANDARDRESULT::format);
            return server.setLastError(ret);
        }
        return WIZSTANDARDRESULT::noError();
    } catch (Json::Exception& err) {
        TOLOG1("josn error: %1", err.what());
        return WIZSTANDARDRESULT(WIZSTANDARDRESULT::format);
    }
}
//
template <class TData, class TPost = Json::Value>
static WIZSTANDARDRESULT execStandardJsonRequest(WizKMApiServerBase& server, QString urlPath, TData& data, const QString& method = "GET", const TPost &reqBody = TPost())
{
    return execStandardJsonRequest(server, urlPath, data, method, reqBody, [=](const Json::Value& result, TData& data) {
        return data.fromJson(result);
    });
}
//
template <class TData, class TPost = Json::Value>
static WIZSTANDARDRESULT execStandardJsonRequest(WizKMApiServerBase& server, QString urlPath, int& data, const QString& method = "GET", const TPost &reqBody = TPost())
{
    return execStandardJsonRequest(server, urlPath, data, method, reqBody, [=](const Json::Value& result, TData& data) {
        try {
            data = result.asInt();
            return true;
        } catch (Json::Exception& err) {
            return false;
        }
    });
}

template <class TData, class TPost = Json::Value>
static WIZSTANDARDRESULT execStandardJsonRequest(WizKMApiServerBase& server, QString urlPath, __int64& data, const QString& method = "GET", const TPost &reqBody = TPost())
{
    return execStandardJsonRequest(server, urlPath, data, method, reqBody, [=](const Json::Value& result, TData& data) {
        try {
            data = result.asInt64();
            return true;
        } catch (Json::Exception& err) {
            return false;
        }
    });
}

template <class TData, class TPost = Json::Value>
static WIZSTANDARDRESULT execStandardJsonRequest(WizKMApiServerBase& server, QString urlPath, QString& data, const QString& method = "GET", const TPost &reqBody = TPost())
{
    return execStandardJsonRequest(server, urlPath, data, method, reqBody, [=](const Json::Value& result, TData& data) {
        try {
            data = QString::fromStdString(result.asString());
            return true;
        } catch (Json::Exception& err) {
            return false;
        }
    });
}

//
}   //end WithResult
//
/*
 * GET/POST list
  */

template <class TData>
bool queryJsonList(WizKMApiServerBase& server, QString url, QString method, const QByteArray& bodyData, std::deque<TData>& arrayRet)
{
    Json::Value doc;
    WIZSTANDARDRESULT jsonRet = WizRequest::execStandardJsonRequest(url, method, bodyData, doc);
    if (!jsonRet)
    {
        TOLOG2("Failed to call %1, %2", url, jsonRet.toString());
        return server.setLastError(jsonRet);
    }
    //
    try {
        Json::Value result = doc["result"];
        if (!result || result.isNull())
        {
            TOLOG1("No result while calling %1", url);
            return false;
        }
        if (!result.isArray())
        {
            TOLOG1("Result is not an array while calling %1", url);
            return false;
        }
        //
        for (int i = 0; i < result.size(); i++)
        {
            Json::Value elem = result[i];
            TData data;
            if (!data.fromJson(elem))
            {
                TOLOG2("Failed to convert from json %1 while calling %2", QString::fromStdString(elem.toStyledString()), url);
                return false;
            }
            //
            arrayRet.push_back(data);
        }
    } catch (Json::Exception& err) {
        TOLOG1("josn error: %1", err.what());
        return server.setLastError(WIZSTANDARDRESULT(WIZSTANDARDRESULT::format));
    }

    //
    return true;
}


template <class TData>
bool getJsonList(WizKMApiServerBase& server, QString urlPath, int nCountPerPage, __int64 nStartVersion, std::deque<TData>& arrayRet)
{
    if (urlPath.indexOf("?") == -1)
    {
        urlPath = urlPath + "?version=" + WizInt64ToStr(nStartVersion) + "&count=" + WizIntToStr(nCountPerPage) + "&pageSize=" + WizIntToStr(nCountPerPage);
    }
    else
    {
        urlPath = urlPath + "&version=" + WizInt64ToStr(nStartVersion) + "&count=" + WizIntToStr(nCountPerPage) + "&pageSize=" + WizIntToStr(nCountPerPage);
    }
    QString url = server.buildUrl(urlPath);
    //
    return queryJsonList<TData>(server, url, "GET", QByteArray(), arrayRet);
}


template <class TData>
bool postJsonList(WizKMApiServerBase& server, QString urlPath, const std::deque<TData>& arrayData)
{
    Json::Value doc(Json::arrayValue);
    //
    int index = 0;
    for (auto elem : arrayData)
    {
        Json::Value v;
        if (!elem.toJson(server.getKbGuid(), v))
        {
            TOLOG1("Failed to convert data to json while calling %1", urlPath);
            return server.setLastError(WIZSTANDARDRESULT(WIZSTANDARDRESULT::format));
        }
        //
        doc[index] = v;
        index++;
    }
    //
    WIZSTANDARDRESULT jsonRet = NoResult::execStandardJsonRequest(server, urlPath, "POST", doc);
    if (!jsonRet)
    {
        TOLOG1("Failed to call %1", urlPath);
        return server.setLastError(jsonRet);
    }
    //
    return true;
}

////////////////
/////
//

WizKMApiServerBase::WizKMApiServerBase(const QString& strServer, QObject* parent)
    : QObject(parent)
    , m_strServer(strServer)
    , m_pEvents(NULL)
{
    while (m_strServer.endsWith("/"))
    {
        m_strServer.remove(m_strServer.length() - 1, 1);
    }
}

void WizKMApiServerBase::onApiError()
{
    if (m_pEvents)
    {
        m_pEvents->setLastErrorCode(m_lastError.returnCode);
        m_pEvents->setIsNetworkError(m_lastError.isNetworkError());
        m_pEvents->setLastErrorMessage(m_lastError.returnMessage);
    }
    //
    if (m_lastError.returnCode == WIZKM_XMLRPC_ERROR_INVALID_TOKEN) {
        WizToken::clearToken();
    }
}


QString WizKMApiServerBase::buildUrl(QString urlPath)
{
    if (urlPath.startsWith("http://") || urlPath.startsWith("https://")) {
        QString url = urlPath;
        return appendNormalParams(url, getToken());
    }
    else
    {
        //
        if (!urlPath.startsWith("/"))
        {
            urlPath = "/" + urlPath;
        }
        QString url = getServer() + urlPath;
        return appendNormalParams(url, getToken());
    }
}

bool WizKMApiServerBase::getValueVersion(const QString& strMethodPrefix, const QString& strGuid, const QString& strKey, __int64& nVersion)
{
    QString urlPath = "/" + strMethodPrefix + "/kv/version/" + strGuid + "?key=" + strKey;
    //
    WIZSTANDARDRESULT jsonRet = WithResult::execStandardJsonRequest<__int64>(*this, urlPath, nVersion);
    return jsonRet;
}
bool WizKMApiServerBase::getValue(const QString& strMethodPrefix, const QString& strGuid, const QString& strKey, QString& strValue, __int64& nVersion)
{
    struct KEYVALUEDATA
    {
        __int64 nVersion;
        QString strKey;
        QString strValue;
        //
        bool fromJson(const Json::Value& value)
        {
            try {
                nVersion = value["version"].asInt64();
                strKey = QString::fromStdString(value["key"].asString());
                strValue = QString::fromStdString(value["value"].asString());
                return true;
            } catch (Json::Exception& err) {
                TOLOG1("josn error: %1", err.what());
                return false;
            }
        }
    };

    //
    QString urlPath = "/" + strMethodPrefix + "/kv/value/" + strGuid + "?key=" + strKey;
    //
    KEYVALUEDATA data;
    WIZSTANDARDRESULT jsonRet = WithResult::execStandardJsonRequest<KEYVALUEDATA>(*this, urlPath, data);
    if (!jsonRet)
    {
        TOLOG1("Failed to call %1", urlPath);
        return false;
    }
    //
    nVersion = data.nVersion;
    strValue = data.strValue;
    return true;
}

bool WizKMApiServerBase::setValue(const QString& strMethodPrefix, const QString& strGuid, const QString& strKey, const QString& strValue, __int64& nRetVersion)
{
    QString urlPath = "/" + strMethodPrefix + "/kv/value/" + strGuid;
    //
    Json::Value body;
    body["key"] = strKey.toStdString();
    body["value"] = strValue.toStdString();
    //
    WIZSTANDARDRESULT jsonRet = NoResult::execStandardJsonRequest(*this, urlPath, "PUT", body);
    if (!jsonRet)
    {
        TOLOG1("Failed to call %1", urlPath);
        return false;
    }
    //
    return true;
}


WizKMAccountsServer::WizKMAccountsServer(QObject* parent)
    : WizKMApiServerBase(WizCommonApiEntry::newAsServerUrl(), parent)
{
#ifdef QT_DEBUG
    //m_strServer = "http://localhost:5001";
#endif
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
bool WizKMAccountsServer::initAllKbInfos()
{
    std::deque<WIZKBINFO> infos;
    if (!getKbInfos(infos))
        return false;
    //
    for (const auto& info : infos) {
        m_kbInfos[info.strKbGUID] = info;
    }
    //
    return true;
}


WIZKBINFO WizKMAccountsServer::getKbInfo(QString kbGuid) const
{
    auto it = m_kbInfos.find(kbGuid);
    if (it == m_kbInfos.end())
        return WIZKBINFO();
    return it->second;
}


bool WizKMAccountsServer::initAllValueVersions()
{
    std::deque<WIZKBVALUEVERSIONS> versions;
    if (!getValueVersions(versions))
        return false;
    //
    for (const auto& version : versions) {
        m_valueVersions[version.strKbGUID] = version;
    }
    //
    return true;
}

WIZKBVALUEVERSIONS WizKMAccountsServer::getValueVersions(QString kbGuid) const
{
    auto it = m_valueVersions.find(kbGuid);
    if (it == m_valueVersions.end())
        return WIZKBVALUEVERSIONS();
    return it->second;
}

bool WizKMAccountsServer::login(const QString& strUserName, const QString& strPassword)
{
    if (m_bLogin)
    {
        return TRUE;
    }
    //
    QString urlPath = "/as/user/login";
    Json::Value params;
    params["userId"] = strUserName.toStdString();
    params["password"] = strPassword.toStdString();
    //
    m_bLogin = WithResult::execStandardJsonRequest<WIZUSERINFO>(*this, urlPath, m_userInfo, "POST", params);
    //
    qDebug() << "new server" << m_userInfo.strKbServer;
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
    QString urlPath = "/as/user/logout";
    Json::Value params;
    params["token"] = m_userInfo.strToken.toStdString();
    //
    bool bRet = NoResult::execStandardJsonRequest(*this, urlPath, "POST", params);
    m_bLogin = false;
    m_userInfo.strToken.clear();
    return bRet;
}

bool WizKMAccountsServer::createAccount(const QString& strUserName, const QString& strPassword, const QString& strInviteCode, const QString& strCaptchaID, const QString& strCaptcha)
{
    QString urlPath = "/as/user/register";
    Json::Value params;
    params["userId"] = strUserName.toStdString();
    params["password"] = strPassword.toStdString();
    params["inviteCode"] = strInviteCode.toStdString();
    params["productName"] = "WizNoteQT";
    if (!strCaptchaID.isEmpty())
    {
        params["captchaId"] = strCaptchaID.toStdString();
        params["captcha"] = strCaptcha.toStdString();
    }
    //
    if (!WithResult::execStandardJsonRequest<WIZUSERINFO>(*this, urlPath, m_userInfo, "POST", params))
    {
        TOLOG("Failed to create account");
        return false;
    }
    //
    return true;
}

bool WizKMAccountsServer::getToken(const QString& strUserName, const QString& strPassword, QString& strToken)
{
    QString urlPath = "/as/user/token";
    Json::Value params;
    params["userId"] = strUserName.toStdString();
    params["password"] = strPassword.toStdString();
    //
    struct GETTOKENDATA
    {
        QString token;
        bool fromJson(const Json::Value& value) {
            try {
                token = QString::fromStdString(value["token"].asString());
                return true;
            } catch (Json::Exception& err) {
                TOLOG1("Failed to convert json to get token result. %1", err.what());
                return false;
            }
        }
    };

    GETTOKENDATA data;
    //
    if (!WithResult::execStandardJsonRequest<GETTOKENDATA>(*this, urlPath, data, "POST", params))
    {
        TOLOG("Failed to create account");
        return false;
    }
    //
    strToken = data.token;
    return true;
}


struct WIZCERTDATA
{
    QString n;
    QString e;
    QString d;
    QString hint;

    bool fromJson(const Json::Value& value)
    {
        try {
            n = QString::fromStdString(value["n"].asString());
            e = QString::fromStdString(value["e"].asString());
            d = QString::fromStdString(value["d"].asString());
            hint = QString::fromStdString(value["hint"].asString());
            return true;
        } catch (Json::Exception& err) {
            TOLOG1("Failed to convert josn to cert: %1", err.what());
            return false;
        }
    }
};
//

bool WizKMAccountsServer::getCert(QString& strN, QString& stre, QString& strd, QString& strHint)
{
    QString urlPath = "/as/user/cert";
    //
    WIZCERTDATA data;
    WIZSTANDARDRESULT jsonRet = WithResult::execStandardJsonRequest<WIZCERTDATA>(*this, urlPath, data);
    if (!jsonRet)
    {
        TOLOG1("Failed to call %1", urlPath);
        return false;
    }
    //
    strN = data.n;
    stre = data.e;
    strd = data.d;
    strHint = data.hint;
    return true;
}

bool WizKMAccountsServer::setCert(const QString& strN, const QString& stre, const QString& strd, const QString& strHint)
{
    QString urlPath = "/as/user/cert";
    //
    Json::Value params;
    params["n"] = strN.toStdString();
    params["e"] = stre.toStdString();
    params["d"] = strd.toStdString();
    params["hint"] = strHint.toStdString();
    //
    WIZSTANDARDRESULT jsonRet = NoResult::execStandardJsonRequest(*this, urlPath, "POST", params);
    if (!jsonRet)
    {
        TOLOG1("Failed to call %1", urlPath);
        return false;
    }
    return true;
}

bool WizKMAccountsServer::getAdminBizCert(const QString& strBizGuid, QString& strN, QString& stre, QString& strd, QString& strHint)
{
    QString urlPath = "/as/biz/cert/" + strBizGuid;
    //
    WIZCERTDATA data;
    WIZSTANDARDRESULT jsonRet = WithResult::execStandardJsonRequest<WIZCERTDATA>(*this, urlPath, data);
    if (!jsonRet)
    {
        TOLOG1("Failed to call %1", urlPath);
        return false;
    }
    //
    strN = data.n;
    stre = data.e;
    strd = data.d;
    strHint = data.hint;
    return true;
}


bool WizKMAccountsServer::setUserBizCert(const QString& strBizGuid, const QString& strN, const QString& stre, const QString& strd, const QString& strHint)
{
    Json::Value doc;
    //
    doc["n"] = strN.toStdString();
    doc["e"] = stre.toStdString();
    doc["d"] = strd.toStdString();
    doc["hint"] = strHint.toStdString();
    //
    Json::FastWriter writer;
    std::string ret = writer.write(doc);
    QString json = QString::fromStdString(ret);
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
    Json::Value d;
    Json::Reader reader;
    if (!reader.parse(json.toUtf8().constData(), d))
        return false;
    //
    try {
        strN = QString::fromStdString(d["n"].asString());
        stre = QString::fromStdString(d["e"].asString());
        strd = QString::fromStdString(d["d"].asString());
        strHint = QString::fromStdString(d["hint"].asString());
    }
    catch (...) {
        return false;
    }

    //
    return true;
}


QString WizKMAccountsServer::getToken() const
{
    if (!m_bLogin)
        return QString();
    //
    return m_userInfo.strToken;
}
QString WizKMAccountsServer::getKbGuid() const
{
    if (!m_bLogin)
        return QString();
    //
    return m_userInfo.strKbGUID;
}

bool WizKMAccountsServer::getGroupList(CWizGroupDataArray& arrayGroup)
{
    QString urlPath = "/as/user/groups?kbType=group";
    //
    if (!getJsonList<WIZGROUPDATA>(*this, urlPath, 0, 0, arrayGroup))
    {
        TOLOG("Failed to get group list");
        return false;
    }
    //
    return true;
}

bool WizKMAccountsServer::getBizList(CWizBizDataArray& arrayBiz)
{
    QString urlPath = "/as/user/bizs";
    //
    if (!getJsonList<WIZBIZDATA>(*this, urlPath, 0, 0, arrayBiz))
    {
        TOLOG("Failed to get group list");
        return false;
    }
    //
    return true;
}

bool WizKMAccountsServer::keepAlive()
{
    QString urlPath = "/as/user/keep";
    //
    WIZSTANDARDRESULT jsonRet = NoResult::execStandardJsonRequest(*this, urlPath);
    if (!jsonRet)
    {
        TOLOG1("Failed to call %1", urlPath);
        return false;
    }
    //
    return true;
}
//
//
bool WizKMAccountsServer::getMessages(__int64 nStartVersion, CWizMessageDataArray& arrayRet)
{
    int nCountPerPage = 100;
    __int64 nNextVersion = nStartVersion + 1;
    //
    QString urlPath = "/messages";
    QString strUrl = WizCommonApiEntry::messageServerUrl() + urlPath;
    if (!strUrl.startsWith("http")) {
        TOLOG("Failed to get message server");
        return false;
    }
    //
    while (1)
    {
        CWizMessageDataArray arrayPageData;
        //
        if (!getJsonList<WIZMESSAGEDATA>(*this, strUrl, nCountPerPage, nNextVersion, arrayRet))
        {
            TOLOG2("Failed to get message list: CountPerPage=%1, Version=%2", WizIntToStr(nCountPerPage), WizInt64ToStr(nNextVersion));
            return FALSE;
        }
        //
        arrayRet.insert(arrayRet.end(), arrayPageData.begin(), arrayPageData.end());
        //
        for (CWizMessageDataArray::const_iterator it = arrayPageData.begin();
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

bool WizKMAccountsServer::getBizUsers(const QString& bizGuid, const QString& kbGuid, CWizBizUserDataArray& arrayUser)
{
    int nCountPerPage = 100;
    __int64 nNextVersion = 0;
    //
    QString urlPath = "/a/biz/user_aliases?biz_guid=" + bizGuid + "&kb_guid=" + kbGuid;
    QString strUrl = WizCommonApiEntry::asServerUrl() + urlPath;
    //
    if (!getJsonList<WIZBIZUSER>(*this, strUrl, nCountPerPage, nNextVersion, arrayUser))
    {
        TOLOG2("Failed to get Biz Users: CountPerPage=%1, Version=%2", WizIntToStr(nCountPerPage), WizInt64ToStr(nNextVersion));
        return false;
    }
    //
    for (auto& user : arrayUser)
    {
        user.kbGUID = kbGuid;
    }
    //
    return true;
}

bool WizKMAccountsServer::getKbInfos(std::deque<WIZKBINFO>& arrayInfo)
{
    int nCountPerPage = 100;
    __int64 nNextVersion = 0;
    //
    QString urlPath = "/as/user/kb/info/all";
    //
    if (!getJsonList<WIZKBINFO>(*this, urlPath, nCountPerPage, nNextVersion, arrayInfo))
    {
        TOLOG2("Failed to get Kb Infos: CountPerPage=%1, Version=%2", WizIntToStr(nCountPerPage), WizInt64ToStr(nNextVersion));
        return false;
    }
    //
    return true;
}

bool WizKMAccountsServer::getValueVersions(std::deque<WIZKBVALUEVERSIONS>& arrayVersion)
{
    int nCountPerPage = 100;
    __int64 nNextVersion = 0;
    //
    QString urlPath = "/as/user/kv/versions";
    //
    if (!getJsonList<WIZKBVALUEVERSIONS>(*this, urlPath, nCountPerPage, nNextVersion, arrayVersion))
    {
        TOLOG2("Failed to get Kb value verskons: CountPerPage=%1, Version=%2", WizIntToStr(nCountPerPage), WizInt64ToStr(nNextVersion));
        return false;
    }
    //
    return true;
}



bool WizKMAccountsServer::setMessageReadStatus(const QString& strMessageIDs, int nStatus)
{
    QString strUrl = WizCommonApiEntry::messageServerUrl();
    strUrl += QString("/messages/status?ids=%1&status=%2").arg(strMessageIDs).arg(WizIntToStr(nStatus));
    qDebug() << "set message raad status, strToken:" << m_userInfo.strToken << "   ids : " << strMessageIDs << " url : " << strUrl;
    //
    return NoResult::execStandardJsonRequest(*this, strUrl);
}

bool WizKMAccountsServer::setMessageDeleteStatus(const QString& strMessageIDs, int nStatus)
{
    QString strUrl = WizCommonApiEntry::messageServerUrl();
    strUrl += QString("/messages?ids=%1").arg(strMessageIDs);
    qDebug() << "set message delete status, strToken:" << m_userInfo.strToken << "   ids : " << strMessageIDs << " url : " << strUrl;
    //
    return NoResult::execStandardJsonRequest(*this, strUrl, "DELETE");
}

bool WizKMAccountsServer::getValueVersion(const QString& strKey, __int64& nVersion)
{
    return WizKMApiServerBase::getValueVersion("as", m_userInfo.strUserGUID, strKey, nVersion);
}
bool WizKMAccountsServer::getValue(const QString& strKey, QString& strValue, __int64& nVersion)
{
    return WizKMApiServerBase::getValue("as", m_userInfo.strUserGUID, strKey, strValue, nVersion);
}
bool WizKMAccountsServer::setValue(const QString& strKey, const QString& strValue, __int64& nRetVersion)
{
    return WizKMApiServerBase::setValue("as", m_userInfo.strUserGUID, strKey, strValue, nRetVersion);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


WizKMDatabaseServer::WizKMDatabaseServer(const WIZUSERINFOBASE& userInfo, const WIZKBINFO& kbInfo, const WIZKBVALUEVERSIONS& versions, QObject* parent)
    : WizKMApiServerBase(userInfo.strKbServer, parent)
    , m_userInfo(userInfo)
    , m_kbInfo(kbInfo)
    , m_valueVersions(versions)
    , m_lastJsonResult(200, QString(""), QString(""))
    , m_objectsTotalSize(0)
{
#ifdef QT_DEBUG
    if (m_strServer.isEmpty()) {
        Q_ASSERT(false);
    }
    //m_strServer = m_userInfo.strKbServer = "http://localhost:4001";
#endif
}

WizKMDatabaseServer::~WizKMDatabaseServer()
{
}

const WIZKBINFO& WizKMDatabaseServer::kbInfo()
{
    return m_kbInfo;
}

bool WizKMDatabaseServer::isGroup() const
{
    Q_ASSERT(!WizToken::userInfo().strKbGUID.isEmpty());
    //
    return WizToken::userInfo().strKbGUID != m_userInfo.strKbGUID;
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
bool WizKMDatabaseServer::kb_getInfo()
{
    if (m_kbInfo.nDocumentVersion == -1)
    {
        QString url = m_userInfo.strKbServer + "/ks/kb/info/" + m_userInfo.strKbGUID;
        //
        WIZSTANDARDRESULT jsonRet = WithResult::execStandardJsonRequest<WIZKBINFO>(*this, url, m_kbInfo);
        if (!jsonRet)
        {
            TOLOG1("Failed to call %1", url);
            return false;
        }
        //
    }
    return true;
}

bool WizKMDatabaseServer::getValueVersion(const QString& strKey, __int64& nVersion)
{
    if (m_valueVersions.inited)
    {
        auto it = m_valueVersions.versions.find(strKey);
        if (it != m_valueVersions.versions.end())
        {
            nVersion = it->second;
            return true;
        }
        nVersion = -1;
        return true;
    }
    //
    return WizKMApiServerBase::getValueVersion("ks", getKbGuid(), strKey, nVersion);
}
bool WizKMDatabaseServer::getValue(const QString& strKey, QString& strValue, __int64& nVersion)
{
    return WizKMApiServerBase::getValue("ks", getKbGuid(), strKey, strValue, nVersion);
}
bool WizKMDatabaseServer::setValue(const QString& strKey, const QString& strValue, __int64& nRetVersion)
{
    return WizKMApiServerBase::setValue("ks", getKbGuid(), strKey, strValue, nRetVersion);
}


bool WizKMDatabaseServer::getCommentCount(const QString& strDocumentGuid, int& commentCount)
{
    QString urlPath = "/ks/note/comments/count/" + getKbGuid() + "/" + strDocumentGuid;
    return WithResult::execStandardJsonRequest<int>(*this, urlPath, commentCount);
}


void WizKMDatabaseServer::onDocumentObjectDownloadProgress(QUrl url, qint64 downloadSize, qint64 totalSize)
{
    QString urlString = url.toString();
    m_objectDownloadedSize[urlString] = downloadSize;
    //
    qint64 totalDownloadedSize = 0;
    for (auto it : m_objectDownloadedSize)
    {
        totalDownloadedSize += it.second;
    }
    //
    if (totalDownloadedSize < 0) {
        totalDownloadedSize = 0;
    } else if (totalDownloadedSize > m_objectsTotalSize) {
        totalDownloadedSize = m_objectsTotalSize;
    }
    //
    emit downloadProgress(m_objectsTotalSize, totalDownloadedSize);
}


bool WizKMDatabaseServer::document_downloadDataNew(const QString& strDocumentGUID, WIZDOCUMENTDATAEX& ret, const QString& oldFileName)
{
    QString url = buildUrl("/ks/note/download/" + m_userInfo.strKbGUID + "/" + strDocumentGUID + "?downloadData=1");
    //
    Json::Value doc;
    WIZSTANDARDRESULT jsonRet = WizRequest::execStandardJsonRequest(url, doc);
    m_lastJsonResult = jsonRet;
    if (!jsonRet)
    {
        setLastError(jsonRet);
        TOLOG1("Failed to download document data: %1", ret.strTitle);
        return false;
    }
    //
    Json::Value urlValue = doc["url"];
    if (!urlValue.isNull())
    {
        //encrypted note
        if (!::WizURLDownloadToData(urlValue.asString().c_str(), ret.arrayData))
        {
            TOLOG1("Failed to download document data: %1", ret.strTitle);
            return false;
        }
        //
        return true;
    }
    //
    QString html = QString::fromUtf8(doc["html"].asString().c_str());
    if (html.isEmpty())
        return false;
    //
    Json::Value resourcesObj = doc["resources"];
    //
    struct RESDATA : public WIZZIPENTRYDATA
    {
        QString url;
    };

    //
    std::vector<RESDATA> serverResources;
    if (resourcesObj.isArray())
    {
        int resourceCount = resourcesObj.size();
        for (int i = 0; i < resourceCount; i++)
        {
            Json::Value resObj = resourcesObj[i];
            RESDATA data;
            data.name = QString::fromUtf8(resObj["name"].asString().c_str());
            data.url = QString::fromUtf8(resObj["url"].asString().c_str());
            data.size = atoi((resObj["size"].asString().c_str()));
            data.time = QDateTime::fromTime_t(resObj["time"].asInt64() / 1000);
            serverResources.push_back(data);
        }
    }
    //
    WizTempFileGuard tempFile;
    WizZipFile newZip;
    if (!newZip.open(tempFile.fileName()))
    {
        TOLOG1("Failed to create temp file: %1", tempFile.fileName());
        return false;
    }
    //
    QByteArray htmlData;
    WizSaveUnicodeTextToData(htmlData, html, true);
    if (!newZip.compressFile(htmlData, "index.html"))
    {
        TOLOG("Failed to add index.html to zip file");
        return false;
    }
    //
    WizUnzipFile oldZip;
    bool hasOldZip = false;
    if (WizPathFileExists(oldFileName))
    {
        if (!oldZip.open(oldFileName))
        {
            TOLOG1("Failed to open old file: %1", oldFileName);
        }
        else
        {
            hasOldZip = true;
        }
    }
    //
    for (intptr_t i = serverResources.size() - 1; i >= 0; i--)
    {
        auto res = serverResources[i];
        QString resName = "index_files/" + res.name;
        if (hasOldZip)
        {
            int index = oldZip.fileNameToIndex(resName);
            if (-1 != index)
            {
                QByteArray data;
                if (oldZip.extractFile(index, data))
                {
                    if (newZip.compressFile(data, resName))
                    {
                        serverResources.erase(serverResources.begin() + i);
                        continue;
                    }
                }
            }
        }
    }
    //
    QMutex mutex;
    //
    int totalWaitForDownload = (int)serverResources.size();
    int totalDownloaded = 0;
    int totalFailed = 0;
    //
    int totalDownloadSize = 0;
    for (auto res : serverResources)
    {
        totalDownloadSize += res.size;
    }
    //
    m_objectsTotalSize = totalDownloadSize;
    //
    for (auto res : serverResources)
    {
        QString resName = "index_files/" + res.name;
        //
#ifdef QT_DEBUG
        qDebug() << res.url;
#endif
        ::WizExecuteOnThread(WIZ_THREAD_DOWNLOAD_RESOURCES, [=, &mutex, &totalFailed, &totalDownloaded, &newZip] {
            //
            QByteArray data;
            qDebug() << "downloading " << resName;
            bool ret = WizURLDownloadToData(res.url, data, this, SLOT(onDocumentObjectDownloadProgress(QUrl, qint64, qint64)));
            qDebug() << "downloaded " << resName;
            //
            QMutexLocker locker(&mutex);
            if (!ret)
            {
                TOLOG1("Failed to download res: %1", res.url);
                totalFailed++;
                return;
            }
            //
            if (!newZip.compressFile(data, resName))
            {
                TOLOG("Failed to add data to zip file");
                totalFailed++;
                return;
                //
            }
            //
            totalDownloaded++;

        });
    }
    //
    if (totalWaitForDownload > 0)
    {
        while (true)
        {
            if (totalWaitForDownload == totalDownloaded + totalFailed)
            {
                if (totalFailed == 0)
                    break;
                //
                return false;
            }
            //
            QEventLoop loop;
            loop.processEvents();
            //
            QThread::msleep(300);
        }
    }
    //
    if (!newZip.close())
    {
        TOLOG("Failed to close zip file");
        return false;
    }
    //
    if (!WizLoadDataFromFile(tempFile.fileName(), ret.arrayData))
    {
        TOLOG1("Failed to load data from file: %1", tempFile.fileName());
        return false;
    }
    //
    return true;
}

bool WizKMDatabaseServer::document_downloadData(const QString& strDocumentGUID, WIZDOCUMENTDATAEX& ret, const QString& fileName)
{
    return document_downloadDataNew(strDocumentGUID, ret, fileName);
}
//

bool WizKMDatabaseServer::attachment_downloadDataNew(const QString& strDocumentGUID, const QString& strAttachmentGUID, WIZDOCUMENTATTACHMENTDATAEX& ret)
{
    QString url = buildUrl("/ks/object/download/" + m_userInfo.strKbGUID + "/" + strDocumentGUID + "?objType=attachment&objId=" + strAttachmentGUID);
    return WizURLDownloadToData(url, ret.arrayData);
}

bool WizKMDatabaseServer::attachment_downloadData(const QString& strDocumentGUID, const QString& strAttachmentGUID, WIZDOCUMENTATTACHMENTDATAEX& ret)
{
    return attachment_downloadDataNew(strDocumentGUID, strAttachmentGUID, ret);
}

struct WIZRESOURCEDATA
{
    QString name;
    QByteArray data;
};

bool uploadResources(WizKMDatabaseServer& server, const QString& url, const QString& key, const QString& kbGuid, const QString& docGuid, const std::vector<WIZRESOURCEDATA>& files, bool isLast, Json::Value& res)
{
    QString objType = "resource";
    //
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    //
    QHttpPart keyPart;
    keyPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"key\""));
    keyPart.setBody(key.toUtf8());

    QHttpPart kbGuidPart;
    kbGuidPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"kbGuid\""));
    kbGuidPart.setBody(kbGuid.toUtf8());

    QHttpPart docGuidPart;
    docGuidPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"docGuid\""));
    docGuidPart.setBody(docGuid.toUtf8());

    QHttpPart objTypePart;
    objTypePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"objType\""));
    objTypePart.setBody(objType.toUtf8());

    QHttpPart isLastPart;
    isLastPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"isLast\""));
    isLastPart.setBody(isLast ? "1" : "0");

    multiPart->append(keyPart);
    multiPart->append(kbGuidPart);
    multiPart->append(docGuidPart);
    multiPart->append(objTypePart);
    multiPart->append(isLastPart);
    //
    for (auto file: files)
    {
        QHttpPart filePart;
        QString filePartHeader = QString("form-data; name=\"data\"; filename=\"%1\"").arg(file.name);
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(filePartHeader));
        filePart.setBody(file.data);
        multiPart->append(filePart);
    }
    //
    QNetworkRequest request(url);

    QNetworkAccessManager networkManager;
    QNetworkReply* reply = networkManager.post(request, multiPart);
    multiPart->setParent(reply); // delete the multiPart with the reply
    //
    WizAutoTimeOutEventLoop loop(reply);
    loop.setTimeoutWaitSeconds(60 * 60);
    loop.exec();
    //
    QNetworkReply::NetworkError err = loop.error();
    if (err != QNetworkReply::NoError)
    {
        TOLOG2("Failed to exec json request, network error=%1, message=%2", WizIntToStr(err), loop.errorString());
        return false;
    }
    //
    QByteArray resData = loop.result();
    //
    WIZSTANDARDRESULT ret = WizRequest::isSucceededStandardJsonRequest(resData);
    if (!ret)
    {
        server.setLastError(ret);
        //
        qDebug() << ret.returnMessage;
        return false;
    }
    //
    return true;
}


bool uploadObject(WizKMDatabaseServer& server, const QString& url, const QString& key, const QString& kbGuid, const QString& docGuid, const QString& objType, const QString& objId, const QByteArray& data, bool isLast, Json::Value& res)
{
    int partSize = 1024 * 1024; //1M
    int partCount = (data.length() + partSize - 1) / partSize;
    for (int i = 0; i < partCount; i++)
    {
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        //
        QHttpPart keyPart;
        keyPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"key\""));
        keyPart.setBody(key.toUtf8());

        QHttpPart kbGuidPart;
        kbGuidPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"kbGuid\""));
        kbGuidPart.setBody(kbGuid.toUtf8());

        QHttpPart docGuidPart;
        docGuidPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"docGuid\""));
        docGuidPart.setBody(docGuid.toUtf8());

        QHttpPart objIdPart;
        objIdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"objId\""));
        objIdPart.setBody(objId.toUtf8());

        QHttpPart objTypePart;
        objTypePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"objType\""));
        objTypePart.setBody(objType.toUtf8());

        QHttpPart indexPart;
        indexPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"partIndex\""));
        indexPart.setBody(WizIntToStr(i).toUtf8());

        QHttpPart countPart;
        countPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"partCount\""));
        countPart.setBody(WizIntToStr(partCount).toUtf8());

        QHttpPart isLastPart;
        isLastPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"isLast\""));
        isLastPart.setBody(isLast ? "1" : "0");

        QHttpPart filePart;
        QString filePartHeader = QString("form-data; name=\"data\"; filename=\"%1\"").arg(objId);
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(filePartHeader));
        //
        int extra = 0;
        int partEnd = i * partSize + partSize;
        if (partEnd > data.length()) {
            extra = partEnd - data.length();
        }
        QByteArray partData = data.mid(i * partSize, partSize - extra);
        filePart.setBody(partData);
        //
        multiPart->append(keyPart);
        multiPart->append(kbGuidPart);
        multiPart->append(docGuidPart);
        multiPart->append(objIdPart);
        multiPart->append(objTypePart);
        multiPart->append(indexPart);
        multiPart->append(countPart);
        multiPart->append(isLastPart);
        multiPart->append(filePart);
        //
        QNetworkRequest request(url);

        QNetworkAccessManager networkManager;
        QNetworkReply* reply = networkManager.post(request, multiPart);
        multiPart->setParent(reply); // delete the multiPart with the reply
        //
        WizAutoTimeOutEventLoop loop(reply);
        loop.setTimeoutWaitSeconds(60 * 60);
        loop.exec();
        //
        QNetworkReply::NetworkError err = loop.error();
        if (err != QNetworkReply::NoError)
        {
            TOLOG2("Failed to exec json request, network error=%1, message=%2", WizIntToStr(err), loop.errorString());
            return false;
        }
        //
        QByteArray resData = loop.result();
        //
        try {
            Json::Value partRes;
            WIZSTANDARDRESULT ret = WizRequest::isSucceededStandardJsonRequest(resData, partRes);
            if (!ret)
            {
                server.setLastError(ret);
                qDebug() << "Can't upload note data, ret code=" << ret.returnCode << ", message=" << ret.returnMessage;
                return false;
            }
            //
            if (i == partCount - 1)
            {
                res = partRes;
                return true;
            }
            //
            continue;
        }
        catch (std::exception& err)
        {
            qDebug() << "josn error: " << err.what();
            return false;
        }
    }
    //
    qDebug() << "Should not come here";
    return false;
}


bool WizKMDatabaseServer::attachment_postDataNew(WIZDOCUMENTATTACHMENTDATAEX& data, bool withData, __int64& nServerVersion)
{
    QString url_main = buildUrl("/ks/attachment/upload/" + m_userInfo.strKbGUID + "/" + data.strDocumentGUID + "/" + data.strGUID);
    QString url_data = buildUrl("/ks/object/upload/" + m_userInfo.strKbGUID + "/" + data.strDocumentGUID);
    //
    Json::Value att;
    att["kbGuid"] = m_userInfo.strKbGUID.toStdString();
    att["docGuid"] = data.strDocumentGUID.toStdString();
    att["attGuid"] = data.strGUID.toStdString();
    att["dataMd5"] = data.strDataMD5.toStdString();
    att["dataModified"] = (Json::UInt64)data.tDataModified.toTime_t() * (Json::UInt64)1000;
    att["name"] = data.strName.toStdString();
    att["url"] = data.strURL.toStdString();
    att["withData"] = withData;
    //
    if (withData)
    {
        att["dataSize"] = data.arrayData.size();
    }
    //
    Json::Value ret;
    WIZSTANDARDRESULT jsonRet = WizRequest::execStandardJsonRequest(url_main, "POST", att, ret);
    if (!jsonRet)
    {
        setLastError(jsonRet);
        qDebug() << "Failed to upload note";
        return false;
    }
    //
    long long newVersion = -1;
    //
    if (withData)
    {
        QString key = QString::fromUtf8(ret["key"].asString().c_str());
        Json::Value res;
        if (!uploadObject(*this, url_data, key, m_userInfo.strKbGUID, data.strDocumentGUID, "attachment", data.strGUID, data.arrayData, true, res))
        {
            qDebug() << "Failed to upload attachment data";
            return false;
        }
        newVersion = res["version"].asInt64();
    }
    //
    if (newVersion == -1)
    {
        newVersion = ret["version"].asInt64();
    }
    //
    nServerVersion = newVersion;
    return true;
}


bool WizKMDatabaseServer::document_postDataNew(const WIZDOCUMENTDATAEX& dataTemp, bool withData, __int64& nServerVersion)
{
    m_strLastLocalError.clear();
    //
    WIZDOCUMENTDATAEX data = dataTemp;
    //
    QString url_main = buildUrl("/ks/note/upload/" + m_userInfo.strKbGUID + "/" + data.strGUID);
    QString url_res = buildUrl("/ks/object/upload/" + m_userInfo.strKbGUID + "/" + data.strGUID);
    //
    if (withData && data.arrayData.length() > 2)
    {
        if (data.arrayData[0] == 'P' && data.arrayData[1] == 'K')
        {
            if (data.nProtected != 0)
            {
                TOLOG("note is not encrypted, but protected is 1");
                data.nProtected = 0;
            }
        }
        else
        {
            if (data.nProtected != 1)
            {
                TOLOG("note is encrypted, but protected is 0");
                data.nProtected = 1;
            }
        }
    }
    //
    CString tags;
    ::WizStringArrayToText(data.arrayTagGUID, tags, "*");
    //
    Json::Value doc;
    doc["kbGuid"] = m_userInfo.strKbGUID.toStdString();
    doc["docGuid"] = data.strGUID.toStdString();
    doc["title"] = data.strTitle.toStdString();
    doc["dataMd5"] = data.strDataMD5.toStdString();
    doc["dataModified"] = (Json::UInt64)data.tDataModified.toTime_t() * (Json::UInt64)1000;
    doc["category"] = data.strLocation.toStdString();
    doc["owner"] = data.strOwner.toStdString();
    doc["protected"] = (int)data.nProtected;
    doc["readCount"] = (int)data.nReadCount;
    doc["attachmentCount"] = (int)data.nAttachmentCount;
    doc["type"] = data.strType.toStdString();
    doc["fileType"] = data.strFileType.toStdString();
    doc["created"] = (Json::UInt64)data.tCreated.toTime_t() * (Json::UInt64)1000;
    doc["accessed"] = (Json::UInt64)data.tAccessed.toTime_t() * (Json::UInt64)1000;
    doc["url"] = data.strURL.toStdString();
    doc["styleGuid"] = data.strStyleGUID.toStdString();
    doc["seo"] = data.strSEO.toStdString();
    doc["author"] = data.strAuthor.toStdString();
    doc["keywords"] = data.strKeywords.toStdString();
    doc["tags"] = tags.toStdString();
    doc["withData"] = withData;
    //
    std::vector<WIZZIPENTRYDATA> allLocalResources;
    WizUnzipFile zip;
    if (withData && !data.nProtected)
    {
        if (!zip.open(data.arrayData))
        {
            m_strLastLocalError = "WizErrorInvalidZip";
            TOLOG(_T("Can't open document data!"));
            qDebug() << "Can't open document data";
            return false;
        }
        //
        QString html;
        if (!zip.readMainHtmlAndResources(html, allLocalResources))
        {
            m_strLastLocalError = "WizErrorInvalidZip";
            TOLOG(_T("Can't load html and resources!"));
            qDebug() << "Can't load html and resources";
            return false;
        }
        //
        doc["html"] = html.toStdString();
        //
        Json::Value res(Json::arrayValue);
        for (auto data : allLocalResources)
        {
            Json::Value elemObj;
            elemObj["name"] = data.name.toStdString();
            elemObj["time"] = (Json::UInt64)data.time.toTime_t() * (Json::UInt64)1000;
            elemObj["size"] = (Json::UInt64)data.size;
            res.append(elemObj);
        }
        //
        doc["resources"] = res;
    }

    //
    Json::Value ret;
    WIZSTANDARDRESULT jsonRet = WizRequest::execStandardJsonRequest(url_main, "POST", doc, ret);
    m_lastJsonResult = jsonRet;
    if (!jsonRet)
    {
        setLastError(jsonRet);
        qDebug() << "Failed to upload note";
        return false;
    }
    //
    long long newVersion = -1;
    //
    if (withData)
    {
        QString key = QString::fromUtf8(ret["key"].asString().c_str());
        if (data.nProtected)
        {
            Json::Value res;
            if (!uploadObject(*this, url_res, key, m_userInfo.strKbGUID, data.strGUID, "document", data.strGUID, data.arrayData, true, res))
            {
                qDebug() << "Failed to upload note res";
                return false;
            }
            newVersion = res["version"].asInt64();
        }
        else
        {
            Json::Value resourcesWaitForUpload = ret["resources"];
            size_t resCount = resourcesWaitForUpload.size();
            //
            if (resCount > 0)
            {
                //
                std::map<QString, WIZZIPENTRYDATA> localResources;
                for (auto entry : allLocalResources)
                {
                    localResources[entry.name] = entry;
                }
                //
                std::vector<WIZZIPENTRYDATA> resLess300K;
                std::vector<WIZZIPENTRYDATA> resLarge;
                //
                for (int i = 0; i < resCount; i++)
                {
                    QString resName = QString::fromUtf8(resourcesWaitForUpload[i].asString().c_str());
                    WIZZIPENTRYDATA entry = localResources[resName];
                    if (entry.size < 300 * 1024)
                    {
                        resLess300K.push_back(entry);
                    }
                    else
                    {
                        resLarge.push_back(entry);
                    }
                }
                //
                bool hasLarge = !resLarge.empty();
                //
                while (!resLess300K.empty())
                {
                    int size = 0;
                    std::vector<WIZRESOURCEDATA> uploads;
                    while (!resLess300K.empty())
                    {
                        int count = resLess300K.size();
                        WIZZIPENTRYDATA last = resLess300K[count - 1];
                        if (size + last.size > 1024 * 1024)
                            break;
                        //
                        resLess300K.pop_back();
                        size += last.size;
                        //
                        QByteArray resData;
                        if (!zip.extractFile("index_files/" + last.name, resData))
                        {
                            m_strLastLocalError = "WizErrorInvalidZip";
                            TOLOG(_T("Can't extract resource from zip file!"));
                            qDebug() << "Can't extract resource from zip file";
                            return false;
                        }
                        WIZRESOURCEDATA data;
                        data.name = last.name;
                        data.data = resData;
                        //
                        uploads.push_back(data);
                    }
                    //
                    bool isLast = resLess300K.empty() && !hasLarge;
                    //
                    Json::Value res;
                    if (!uploadResources(*this, url_res, key, m_userInfo.strKbGUID, data.strGUID, uploads, isLast, res))
                    {
                        qDebug() << "Failed to upload note res";
                        return false;
                    }
                    //
                    if (isLast)
                    {
                        newVersion = res["version"].asInt64();
                    }
                }
                //
                while (!resLarge.empty())
                {
                    int count = resLarge.size();
                    WIZZIPENTRYDATA last = resLarge[count - 1];
                    resLarge.pop_back();
                    //
                    QByteArray resData;
                    if (!zip.extractFile("index_files/" + last.name, resData))
                    {
                        m_strLastLocalError = "WizErrorInvalidZip";
                        qDebug() << "Can't extract resource from zip file";
                        return false;
                    }
                    //
                    bool isLast = resLarge.empty();
                    //
                    Json::Value res;
                    if (!uploadObject(*this, url_res, key, m_userInfo.strKbGUID, data.strGUID, "resource", last.name, resData, isLast, res))
                    {
                        qDebug() << "Failed to upload note res";
                        return false;
                    }
                    //
                    if (isLast)
                    {
                        newVersion = res["version"].asInt64();
                    }
                }
            }
        }
    }
    //
    if (newVersion == -1)
    {
        newVersion = ret["version"].asInt64();
    }
    //
    nServerVersion = newVersion;
    return true;
}

bool WizKMDatabaseServer::document_postData(const WIZDOCUMENTDATAEX& data, bool bWithDocumentData, __int64& nServerVersion)
{
    return document_postDataNew(data, bWithDocumentData, nServerVersion);
}



bool WizKMDatabaseServer::attachment_postData(WIZDOCUMENTATTACHMENTDATAEX& data, bool withData, __int64& nServerVersion)
{
    return attachment_postDataNew(data, withData, nServerVersion);
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



bool WizKMDatabaseServer::deleted_getList(int nCountPerPage, __int64 nStartVersion, std::deque<WIZDELETEDGUIDDATA>& arrayRet)
{
    QString urlPath = "/ks/deleted/list/version/" + getKbGuid();
    //
    return getJsonList<WIZDELETEDGUIDDATA>(*this, urlPath, nCountPerPage, nStartVersion, arrayRet);
}
bool WizKMDatabaseServer::deleted_postList(std::deque<WIZDELETEDGUIDDATA>& arrayDeletedGUID)
{
    QString urlPath = "/ks/deleted/upload/" + getKbGuid();
    //
    return postJsonList<WIZDELETEDGUIDDATA>(*this, urlPath, arrayDeletedGUID);
}


bool WizKMDatabaseServer::document_getList(int nCountPerPage, __int64 nStartVersion, std::deque<WIZDOCUMENTDATAEX>& arrayRet)
{
    QString urlPath = "/ks/note/list/version/" + getKbGuid();
    //
    return getJsonList<WIZDOCUMENTDATAEX>(*this, urlPath, nCountPerPage, nStartVersion, arrayRet);
}

bool WizKMDatabaseServer::document_getListByGuids(const CWizStdStringArray& arrayDocumentGUID, std::deque<WIZDOCUMENTDATAEX>& arrayRet)
{
    if (arrayDocumentGUID.empty())
        return true;
    //
    const WIZUSERINFOBASE info = userInfo();
    QString url = info.strKbServer + "/ks/note/list/guids/" + info.strKbGUID;
    //
    url = appendNormalParams(url, getToken());
    //
    Json::Value guids(Json::arrayValue);
    for (int i = 0; i < arrayDocumentGUID.size(); i++)
    {
        guids[i] = arrayDocumentGUID[i].toStdString();
    }
    //
    Json::FastWriter writer;
    std::string data = writer.write(guids);
    //
    return queryJsonList<WIZDOCUMENTDATAEX>(*this, url, "POST", QByteArray::fromStdString(data), arrayRet);
}

bool WizKMDatabaseServer::attachment_getList(int nCountPerPage, __int64 nStartVersion, std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayRet)
{
    QString urlPath = "/ks/attachment/list/version/" + getKbGuid();
    //
    return getJsonList<WIZDOCUMENTATTACHMENTDATAEX>(*this, urlPath, nCountPerPage, nStartVersion, arrayRet);
}

bool WizKMDatabaseServer::tag_getList(int nCountPerPage, __int64 nStartVersion, std::deque<WIZTAGDATA>& arrayRet)
{
    QString urlPath = "/ks/tag/list/version/" + getKbGuid();
    //
    return getJsonList<WIZTAGDATA>(*this, urlPath, nCountPerPage, nStartVersion, arrayRet);
}

bool WizKMDatabaseServer::tag_postList(std::deque<WIZTAGDATA>& arrayTag)
{
    QString urlPath = "/ks/tag/upload/" + getKbGuid();
    //
    return postJsonList<WIZTAGDATA>(*this, urlPath, arrayTag);
}


bool WizKMDatabaseServer::style_getList(int nCountPerPage, __int64 nStartVersion, std::deque<WIZSTYLEDATA>& arrayRet)
{
    QString urlPath = "/ks/style/list/version/" + getKbGuid();
    //
    return getJsonList<WIZSTYLEDATA>(*this, urlPath, nCountPerPage, nStartVersion, arrayRet);
}

bool WizKMDatabaseServer::style_postList(const std::deque<WIZSTYLEDATA>& arrayStyle)
{
    QString urlPath = "/ks/style/upload/" + getKbGuid();
    //
    return postJsonList<WIZSTYLEDATA>(*this, urlPath, arrayStyle);
}

bool WizKMDatabaseServer::param_getList(int nCountPerPage, __int64 nStartVersion, std::deque<WIZDOCUMENTPARAMDATA>& arrayRet)
{
    QString urlPath = "/ks/param/list/version/" + getKbGuid();
    //
    return getJsonList<WIZDOCUMENTPARAMDATA>(*this, urlPath, nCountPerPage, nStartVersion, arrayRet);
}

bool WizKMDatabaseServer::param_postList(const std::deque<WIZDOCUMENTPARAMDATA>& arrayParam)
{
    QString urlPath = "/ks/param/upload/" + getKbGuid();
    //
    return postJsonList<WIZDOCUMENTPARAMDATA>(*this, urlPath, arrayParam);
}

