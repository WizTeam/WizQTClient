#include "apientry.h"
#include "apientry_p.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QHostInfo>
#include <QLocale>
#include <QTime>
#include <QString>
#include <QDebug>
#include <QUrl>

#include "apientry.h"
#include "token.h"
#include "wizkmxmlrpc.h"
#include "wizdef.h"
#include "share/wizEventLoop.h"

/*
 * %1: product, use wiz
 * %2: locale, zh-cn|zh-tw|en-us
 * %3: version number
 * %4: command, see command list below
 * %5: ramdom number
 * %6: computer name, optional
 * %7: platform, ios|android|web|wp7|x86|x64|linux|macosx
 * %8: debug, true|false, optional
 */

#define WIZNOTE_API_SERVER      "http://api.wiz.cn/"
#define WIZNOTE_API_ENTRY       "%1?p=%2&l=%3&v=%4&c=%5&random=%6&cn=%7&plat=%8&debug=%9"

#define WIZNOTE_API_ARG_PRODUCT "wiz"

#if defined(Q_OS_MAC)
#define WIZNOTE_API_ARG_PLATFORM "macosx"
#elif defined(Q_OS_WIN)
#define WIZNOTE_API_ARG_PLATFORM "windows"
#else
#define WIZNOTE_API_ARG_PLATFORM "linux"
#endif

// command list
#define WIZNOTE_API_COMMAND_AS_SERVER       "as"
#define WIZNOTE_API_COMMAND_SYNC_HTTP       "sync_http"
#define WIZNOTE_API_COMMAND_SYNC_HTTPS      "sync_https"
#define WIZNOTE_API_COMMAND_MESSAGE_SERVER       "message_server"
#define WIZNOTE_API_COMMAND_MESSAGE_VERSION "message_version"
#define WIZNOTE_API_COMMAND_AVATAR          "avatar"
#define WIZNOTE_API_COMMAND_UPLOAD_AVATAR   "upload_avatar"
#define WIZNOTE_API_COMMAND_USER_INFO       "user_info"
#define WIZNOTE_API_COMMAND_VIEW_GROUP      "view_group"
#define WIZNOTE_API_COMMAND_FEEDBACK        "feedback"
#define WIZNOTE_API_COMMAND_SUPPORT        "support"
#define WIZNOTE_API_COMMAND_COMMENT         "comment"
#define WIZNOTE_API_COMMAND_COMMENT_COUNT   "comment_count2"
#define WIZNOTE_API_COMMAND_CHANGELOG        "changelog"
#define WIZNOTE_API_COMMAND_UPGRADE        "updatev2"
#define WIZNOTE_API_COMMAND_MAIL_SHARE        "mail_share2"

//#ifdef _M_X64
//    QString strPlatform = "x64";
//#else
//    QString strPlatform = "x86";
//#endif

using namespace WizService;
using namespace WizService::Internal;




QString _requestUrl(const QString& strUrl)
{
    QNetworkAccessManager* net = new QNetworkAccessManager();
    QNetworkReply* reply = net->get(QNetworkRequest(strUrl));

    CWizAutoTimeOutEventLoop loop(reply);
//    loop.setTimeoutWaitSeconds(5);
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    if (loop.error() != QNetworkReply::NoError)
        return NULL;

    //NOTE: reply has been delete in event loop, should not be deleted here
//    reply->deleteLater();

    net->deleteLater();

    return loop.result();
}



CommonApiEntryPrivate::CommonApiEntryPrivate()
    : m_strEnterpriseAPIUrl(WIZNOTE_API_SERVER)
    , m_strLocal(QLocale::system().name())
{
}

CommonApiEntryPrivate::~CommonApiEntryPrivate()
{
}

void CommonApiEntryPrivate::setEnterpriseServerIP(const QString& strIP)
{
    if (strIP == m_strEnterpriseAPIUrl)
        return;

    if (!strIP.isEmpty())
    {
        m_strEnterpriseAPIUrl = QString("http://%1/").arg(strIP);
    }
    else
    {
        m_strEnterpriseAPIUrl = WIZNOTE_API_SERVER;
    }

    qDebug() << "set server ip : " << m_strEnterpriseAPIUrl;

    m_strSyncUrl.clear();
    m_strMessageServerUrl.clear();
    m_strMessageVersionUrl.clear();
    m_strAvatarDownloadUrl.clear();
    m_strAvatarUploadUrl.clear();
    m_strCommentUrl.clear();
    m_strCommentCountUrl.clear();
}

void CommonApiEntryPrivate::setLanguage(const QString& strLocal)
{
    m_strLocal = strLocal;
}

QString CommonApiEntryPrivate::asServerUrl()
{
    QString strAsUrl;
    return requestUrl(WIZNOTE_API_COMMAND_AS_SERVER, strAsUrl, false);
}

QString CommonApiEntryPrivate::messageServerUrl()
{
    return requestUrl(WIZNOTE_API_COMMAND_MESSAGE_SERVER, m_strMessageServerUrl, false);
}

QString CommonApiEntryPrivate::urlFromCommand(const QString& strCommand, bool bUseWizServer)
{
    // random seed
    qsrand((uint)QTime::currentTime().msec());

    QString strUrl = QString(WIZNOTE_API_ENTRY)
            .arg(bUseWizServer ? WIZNOTE_API_SERVER : m_strEnterpriseAPIUrl)\
            .arg(WIZNOTE_API_ARG_PRODUCT)\
            .arg(m_strLocal)\
            .arg(WIZ_CLIENT_VERSION)\
            .arg(strCommand)\
            .arg(qrand())\
            .arg(QHostInfo::localHostName())\
            .arg(WIZNOTE_API_ARG_PLATFORM)\
            .arg("false");

    qDebug() << "url from command : " << strCommand << " use wiz server : " << bUseWizServer
             << "  enterprise url : " << m_strEnterpriseAPIUrl << "  result : " << strUrl;
    return strUrl;
}

QString CommonApiEntryPrivate::addExtendedInfo(const QString& strUrl, const QString& strExt)
{
    QString strExtInfo = QString("&a=%1").arg(QUrl::toPercentEncoding(strExt).constData());
    return strUrl + strExtInfo;
}

QString CommonApiEntryPrivate::requestUrl(const QString& strUrl)
{
    QNetworkAccessManager* net = new QNetworkAccessManager();
    QNetworkReply* reply = net->get(QNetworkRequest(strUrl));

    CWizAutoTimeOutEventLoop loop(reply);
//    loop.setTimeoutWaitSeconds(5);
    loop.exec(QEventLoop::ExcludeUserInputEvents);        

    if (loop.error() != QNetworkReply::NoError)
        return NULL;

    //NOTE: reply has been delete in event loop, should not be deleted here
//    reply->deleteLater();

    net->deleteLater();

    return loop.result();
}

QString CommonApiEntryPrivate::requestUrl(const QString& strCommand, QString& strUrl, bool bUseWizServer)
{    
    QString strRequestUrl= urlFromCommand(strCommand, bUseWizServer);

    strUrl = requestUrl(strRequestUrl);

    qDebug() << "request url by command : " << strCommand << "  request result : "  << strUrl;
    return strUrl;
}

QString CommonApiEntryPrivate::syncUrl()
{
    QString strSyncUrl = requestUrl(WIZNOTE_API_COMMAND_SYNC_HTTPS, m_strSyncUrl, false);
    qDebug() << "get sync url , cache url :  " << m_strSyncUrl  << "   result :  " << strSyncUrl;
    return strSyncUrl;
}

QString CommonApiEntryPrivate::messageVersionUrl()
{
    return requestUrl(WIZNOTE_API_COMMAND_MESSAGE_VERSION, m_strMessageVersionUrl, false);
}

QString CommonApiEntryPrivate::avatarDownloadUrl(const QString& strUserGUID)
{
    QString strRawUrl(requestUrl(WIZNOTE_API_COMMAND_AVATAR, m_strAvatarDownloadUrl, false));

    strRawUrl.replace(QRegExp("\\{.*\\}"), strUserGUID);
    strRawUrl += "?default=false"; // Do not download server default avatar
    return strRawUrl;
}

QString CommonApiEntryPrivate::avatarUploadUrl()
{
    return requestUrl(WIZNOTE_API_COMMAND_UPLOAD_AVATAR, m_strAvatarUploadUrl, false);
}

QString CommonApiEntryPrivate::commentUrl(const QString& strToken, const QString& strKbGUID,const QString& strGUID)
{
    if (m_strCommentUrl.isEmpty()) {
        requestUrl(WIZNOTE_API_COMMAND_COMMENT, m_strCommentUrl, false);
    }

    QString strUrl(m_strCommentUrl);
    strUrl.replace("{token}", strToken);
    strUrl.replace("{kbGuid}", strKbGUID);
    strUrl.replace("{documentGuid}", strGUID);

    return strUrl;
}

QString CommonApiEntryPrivate::commentCountUrl(const QString& strKUrl, const QString& strToken,
                                         const QString& strKbGUID, const QString& strGUID)
{
    if (m_strCommentCountUrl.isEmpty()) {
        requestUrl(WIZNOTE_API_COMMAND_COMMENT_COUNT, m_strCommentCountUrl, false);
    }

    QString strServer = strKUrl;
    //NOTE: 新版服务器修改了评论数目获取方法，需要自行将KUrl中的/xmlrpc移除
    strServer.remove("/xmlrpc");
    QString strUrl(m_strCommentCountUrl);
    strUrl.replace("{server_url}", strServer);
    strUrl.replace("{token}", strToken);
    strUrl.replace("{kbGuid}", strKbGUID);
    strUrl.replace("{documentGuid}", strGUID);

    //WARNING: 不知道问什么强制使用Https请求，暂时注掉
//    // use https
//    QUrl url(strUrl);
//    url.setScheme("https");
//    return url.toString();

    return strUrl;
}

QString CommonApiEntryPrivate::analyzerUploadUrl()
{
    QString analyzerUrl;
    requestUrl("analyzer", analyzerUrl, true);
    return analyzerUrl;
}

QString CommonApiEntryPrivate::mailShareUrl(const QString& strKUrl, const QString& strMailInfo)
{
//    QString strKsHost = syncUrl();

    QString strMailShare;
    requestUrl(WIZNOTE_API_COMMAND_MAIL_SHARE, strMailShare, false);
    QString strServer = strKUrl;
    //NOTE: 新版服务器修改了评论数目获取方法，需要自行将KUrl中的/xmlrpc移除
    strServer.remove("/xmlrpc");
    strMailShare.replace("{server_url}", strServer);
    strMailShare.append(strMailInfo);
    return strMailShare;
}

QString CommonApiEntryPrivate::accountInfoUrl(const QString& strToken)
{
    QString strExt = QString("token=%1").arg(strToken);
    QString strUrl = urlFromCommand(WIZNOTE_API_COMMAND_USER_INFO, false);
    return addExtendedInfo(strUrl, strExt);
}

QString CommonApiEntryPrivate::createGroupUrl(const QString& strToken)
{
    QString strExt = QString("token=%1").arg(strToken);
    QString strUrl = urlFromCommand("create_group", false);
    return addExtendedInfo(strUrl, strExt);
}

QString CommonApiEntryPrivate::standardCommandUrl(const QString& strCommand, bool bUseWizServer)
{
    QString strUrl = urlFromCommand(strCommand, bUseWizServer);
    return strUrl;
}

QString CommonApiEntryPrivate::standardCommandUrl(const QString& strCommand, const QString& strToken, bool bUseWizServer)
{
    QString strExt = QString("token=%1").arg(strToken);
    QString strUrl = urlFromCommand(strCommand, bUseWizServer);
    return addExtendedInfo(strUrl, strExt);
}

QString CommonApiEntryPrivate::standardCommandUrl(const QString& strCommand, const QString& strToken, const QString& strExtInfo, bool bUseWizServer)
{
    QString strExt = QString("token=%1").arg(strToken) + "&" + strExtInfo;
    QString strUrl = urlFromCommand(strCommand, bUseWizServer);
    return addExtendedInfo(strUrl, strExt);
}

QString CommonApiEntryPrivate::newStandardCommandUrl(const QString& strCommand, const QString& strToken, const QString& strExt, bool bUseWizServer)
{
    QString strUrl = urlFromCommand(strCommand, bUseWizServer);
    QString strExtInfo = QString("&token=%1").arg(strToken);
    strExtInfo.append(strExt.isEmpty() ? "" : "&" + strExt);
    return strUrl + strExtInfo;
}

QString CommonApiEntryPrivate::groupAttributeUrl(const QString& strToken, const QString& strKbGUID)
{
    QString strExt = QString("token=%1&kb_guid=%2").arg(strToken).arg(strKbGUID);
    QString strUrl = urlFromCommand(WIZNOTE_API_COMMAND_VIEW_GROUP, false);
    return addExtendedInfo(strUrl, strExt);
}

QString CommonApiEntryPrivate::groupUsersUrl(const QString& strToken, const QString& strBizGUID, const QString& strkbGUID)
{
    QUrl url(syncUrl());
    QString strExt = QString("?token=%1&biz_guid=%2&kb_guid=%3&t=%4")
            .arg(strToken)
            .arg(strBizGUID)
            .arg(strkbGUID)
            .arg(qrand());

    return url.scheme() + "://" + url.host() + "/wizas/a/biz/user_aliases" + strExt;
}

QString CommonApiEntryPrivate::kUrlFromGuid(const QString& strToken, const QString& strKbGUID)
{
    Q_ASSERT(!strToken.isEmpty());

    if (m_mapkUrl.contains(strKbGUID))
        return m_mapkUrl.value(strKbGUID);

    WIZUSERINFO info = Token::info();
    m_mapkUrl.insert(info.strKbGUID, info.strDatabaseServer);
    qDebug() << "user: " << info.strKbGUID << " kbUrl: " << info.strDatabaseServer;

    CWizKMAccountsServer asServer(syncUrl());
    asServer.SetUserInfo(info);

    CWizGroupDataArray arrayGroup;
    if (asServer.GetGroupList(arrayGroup)) {
        CWizGroupDataArray::const_iterator it = arrayGroup.begin();
        for (; it != arrayGroup.end(); it++) {
            const WIZGROUPDATA& group = *it;
            m_mapkUrl.insert(group.strGroupGUID, group.strDatabaseServer);
            qDebug() << "group:" << group.strGroupGUID << " kburl: " <<  group.strDatabaseServer;
        }
    } else {
        qDebug() << asServer.GetLastErrorMessage();
    }

    return m_mapkUrl.value(strKbGUID, 0);
}




class CGarbo
{
public:
    CGarbo()
    {
        apiHelper = new CommonApiEntryPrivate();
    }

    ~CGarbo()
    {
        delete apiHelper;
    }

    CommonApiEntryPrivate* apiHelper;

};

static CGarbo Garbo;

void CommonApiEntry::setEnterpriseServerIP(const QString& strIP)
{
    if (strIP == m_server)
        return;

    if (!strIP.isEmpty())
    {
        m_server = QString("http://%1/").arg(strIP);
    }
    else
    {
        m_server = WIZNOTE_API_SERVER;
    }

    qDebug() << "set server ip : " << m_server;
}

void CommonApiEntry::setLanguage(const QString& strLocal)
{
    m_strLocal = strLocal;
}

QString CommonApiEntry::syncUrl()
{
    return Garbo.apiHelper->syncUrl();
}

QString CommonApiEntry::asServerUrl()
{
    return Garbo.apiHelper->asServerUrl();
}

QString CommonApiEntry::messageServerUrl()
{
    return Garbo.apiHelper->messageServerUrl();
}

QString CommonApiEntry::messageVersionUrl()
{
    return Garbo.apiHelper->messageVersionUrl();
}

QString CommonApiEntry::avatarDownloadUrl(const QString& strUserGUID)
{
    return Garbo.apiHelper->avatarDownloadUrl(strUserGUID);
}

QString CommonApiEntry::avatarUploadUrl()
{
    return Garbo.apiHelper->avatarUploadUrl();
}

QString CommonApiEntry::mailShareUrl(const QString& strKUrl, const QString& strMailInfo)
{
    return Garbo.apiHelper->mailShareUrl(strKUrl, strMailInfo);
}

QString CommonApiEntry::commentUrl(const QString& strToken, const QString& strKbGUID,const QString& strGUID)
{
    return Garbo.apiHelper->commentUrl(strToken, strKbGUID, strGUID);
}

QString CommonApiEntry::commentCountUrl(const QString& strKUrl, const QString& strToken,
                                  const QString& strKbGUID, const QString& strGUID)
{
    return Garbo.apiHelper->commentCountUrl(strKUrl, strToken, strKbGUID, strGUID);
}

QString CommonApiEntry::analyzerUploadUrl()
{
    return Garbo.apiHelper->analyzerUploadUrl();
}

QString CommonApiEntry::accountInfoUrl(const QString& strToken)
{
    return Garbo.apiHelper->accountInfoUrl(strToken);
}

QString CommonApiEntry::createGroupUrl(const QString& strToken)
{
    return Garbo.apiHelper->createGroupUrl(strToken);
}

QString CommonApiEntry::captchaUrl(const QString& strCaptchaID, int nWidth, int nHeight)
{
    QString strUrl = Garbo.apiHelper->asServerUrl();
    strUrl += QString("/a/captcha/%1?width=%2&height=%3").arg(strCaptchaID).arg(nWidth).arg(nHeight);
    return strUrl;
}

QString CommonApiEntry::standardCommandUrl(const QString& strCommand, bool bUseWizServer)
{
    return Garbo.apiHelper->standardCommandUrl(strCommand, bUseWizServer);
}

QString CommonApiEntry::standardCommandUrl(const QString& strCommand, const QString& strToken, bool bUseWizServer)
{
    return Garbo.apiHelper->standardCommandUrl(strCommand, strToken, bUseWizServer);
}
QString CommonApiEntry::standardCommandUrl(const QString& strCommand, const QString& strToken, const QString& strExtInfo, bool bUseWizServer)
{
    return Garbo.apiHelper->standardCommandUrl(strCommand, strToken, strExtInfo, bUseWizServer);
}

QString CommonApiEntry::newStandardCommandUrl(const QString& strCommand, const QString& strToken, const QString& strExt, bool bUseWizServer)
{
    return Garbo.apiHelper->newStandardCommandUrl(strCommand, strToken, strExt, bUseWizServer);
}


QString CommonApiEntry::groupAttributeUrl(const QString& strToken, const QString& strKbGUID)
{
    return Garbo.apiHelper->groupAttributeUrl(strToken, strKbGUID);
}

QString CommonApiEntry::groupUsersUrl(const QString& strToken, const QString& strBizGUID, const QString& strkbGUID)
{
    return Garbo.apiHelper->groupUsersUrl(strToken, strBizGUID, strkbGUID);
}

QString CommonApiEntry::kUrlFromGuid(const QString& strToken, const QString& strKbGUID)
{
    return Garbo.apiHelper->kUrlFromGuid(strToken, strKbGUID);
}

QString CommonApiEntry::appstoreParam(bool useAndSymbol)
{
    QString strParam = "";
#ifdef BUILD4APPSTORE
    if (useAndSymbol) {
        strParam = "&appstore=1";
    } else {
        strParam = "appstore=1";
    }
#endif

    return strParam;
}

QString CommonApiEntry::requestUrl(const QString& strCommand)
{
    QString strRequestUrl= urlFromCommand(strCommand);
    QString strUrl = _requestUrl(strRequestUrl);
    if (!strUrl.startsWith("http"))
    {
        qCritical() << "request url by command error. command : " << strCommand << " return : " << strUrl;
        strUrl.clear();
    }
    return strUrl;
}

QString CommonApiEntry::urlFromCommand(const QString& strCommand)
{
    // random seed
    qsrand((uint)QTime::currentTime().msec());
    QString strUrl = QString(WIZNOTE_API_ENTRY)
            .arg(m_server)
            .arg(WIZNOTE_API_ARG_PRODUCT)\
            .arg(m_strLocal)\
            .arg(WIZ_CLIENT_VERSION)\
            .arg(strCommand)\
            .arg(qrand())\
            .arg(QHostInfo::localHostName())\
            .arg(WIZNOTE_API_ARG_PLATFORM)\
            .arg("false");

    qDebug() << "url from command : " << strCommand << "  server url : " << m_server << "  result : " << strUrl;
    return strUrl;
}

QString WizApiEntryPrivate::standardCommandUrl(const QString& strCommand)
{
    return urlFromCommand(strCommand);
}

QString WizApiEntryPrivate::urlFromCommand(const QString& strCommand)
{
    // random seed
    qsrand((uint)QTime::currentTime().msec());

    QString strUrl = QString(WIZNOTE_API_ENTRY)
            .arg(WIZNOTE_API_SERVER)\
            .arg(WIZNOTE_API_ARG_PRODUCT)\
            .arg(m_strLocal)\
            .arg(WIZ_CLIENT_VERSION)\
            .arg(strCommand)\
            .arg(qrand())\
            .arg(QHostInfo::localHostName())\
            .arg(WIZNOTE_API_ARG_PLATFORM)\
            .arg("false");

    return strUrl;
}
