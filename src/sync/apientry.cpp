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

ApiEntryPrivate::ApiEntryPrivate()
    : m_strEnterpriseAPIUrl(WIZNOTE_API_SERVER)
{
}

ApiEntryPrivate::~ApiEntryPrivate()
{
}

void ApiEntryPrivate::setEnterpriseServerIP(const QString& strIP)
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

    m_strSyncUrl.clear();
    m_strMessageVersionUrl.clear();
    m_strAvatarDownloadUrl.clear();
    m_strAvatarUploadUrl.clear();
    m_strCommentUrl.clear();
    m_strCommentCountUrl.clear();
}

QString ApiEntryPrivate::asServerUrl()
{
    QString strAsUrl;
    return requestUrl(WIZNOTE_API_COMMAND_AS_SERVER, strAsUrl, false);
}


QString ApiEntryPrivate::urlFromCommand(const QString& strCommand, bool bUseWizServer)
{
    // random seed
    qsrand((uint)QTime::currentTime().msec());

    QString strUrl = QString(WIZNOTE_API_ENTRY)
            .arg(bUseWizServer ? WIZNOTE_API_SERVER : m_strEnterpriseAPIUrl)\
            .arg(WIZNOTE_API_ARG_PRODUCT)\
            .arg(QLocale::system().name())\
            .arg(WIZ_CLIENT_VERSION)\
            .arg(strCommand)\
            .arg(qrand())\
            .arg(QHostInfo::localHostName())\
            .arg(WIZNOTE_API_ARG_PLATFORM)\
            .arg("false");

    return strUrl;
}

QString ApiEntryPrivate::addExtendedInfo(const QString& strUrl, const QString& strExt)
{
    QString strExtInfo = QString("&a=%1").arg(QUrl::toPercentEncoding(strExt).constData());
    return strUrl + strExtInfo;
}

QString ApiEntryPrivate::requestUrl(const QString& strUrl)
{
    QNetworkAccessManager* net = new QNetworkAccessManager();
    QNetworkReply* reply = net->get(QNetworkRequest(strUrl));

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (reply->error()) {
        reply->deleteLater();
        return 0;
    }

    QString strRequestedUrl = QString::fromUtf8(reply->readAll().constData());

    reply->deleteLater();
    net->deleteLater();

    return strRequestedUrl;
}

QString ApiEntryPrivate::requestUrl(const QString& strCommand, QString& strUrl, bool bUseWizServer)
{
    if (!strUrl.isEmpty())
        return strUrl;

    QString strRequestUrl= urlFromCommand(strCommand, bUseWizServer);

    strUrl = requestUrl(strRequestUrl);
//    qDebug() << "request url for command : " << strCommand << " request url : " << strRequestUrl << "  return url : " << strUrl;
    return strUrl;
}

QString ApiEntryPrivate::syncUrl()
{
    return requestUrl(WIZNOTE_API_COMMAND_SYNC_HTTPS, m_strSyncUrl, false);
}

QString ApiEntryPrivate::messageVersionUrl()
{
    return requestUrl(WIZNOTE_API_COMMAND_MESSAGE_VERSION, m_strMessageVersionUrl, false);
}

QString ApiEntryPrivate::avatarDownloadUrl(const QString& strUserGUID)
{
    QString strRawUrl(requestUrl(WIZNOTE_API_COMMAND_AVATAR, m_strAvatarDownloadUrl, false));

    strRawUrl.replace(QRegExp("\\{.*\\}"), strUserGUID);
    strRawUrl += "?default=false"; // Do not download server default avatar
    return strRawUrl;
}

QString ApiEntryPrivate::avatarUploadUrl()
{
    return requestUrl(WIZNOTE_API_COMMAND_UPLOAD_AVATAR, m_strAvatarUploadUrl, false);
}

QString ApiEntryPrivate::commentUrl(const QString& strToken, const QString& strKbGUID,const QString& strGUID)
{
    if (m_strCommentUrl.isEmpty()) {
        requestUrl(WIZNOTE_API_COMMAND_COMMENT, m_strCommentUrl, false);
    }

    QString strUrl(m_strCommentUrl);
    strUrl.replace("{token}", strToken);
    strUrl.replace("{kbGuid}", strKbGUID);
    strUrl.replace("{documentGuid}", strGUID);

    qDebug() << "comment url : " << m_strCommentUrl;
    return strUrl;
}

QString ApiEntryPrivate::commentCountUrl(const QString& strKUrl, const QString& strToken,
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

QString ApiEntryPrivate::feedbackUrl()
{
    return urlFromCommand(WIZNOTE_API_COMMAND_FEEDBACK, true);
}

QString ApiEntryPrivate::supportUrl()
{
    return urlFromCommand(WIZNOTE_API_COMMAND_SUPPORT, true);
}

QString ApiEntryPrivate::changeLogUrl()
{
    return urlFromCommand(WIZNOTE_API_COMMAND_CHANGELOG, true);
}

QString ApiEntryPrivate::upgradeUrl()
{
    return urlFromCommand(WIZNOTE_API_COMMAND_UPGRADE, true);
}

QString ApiEntryPrivate::analyzerUploadUrl()
{
    QString analyzerUrl;
    requestUrl("analyzer", analyzerUrl, true);
    return analyzerUrl;
}

QString ApiEntryPrivate::mailShareUrl(const QString& strKUrl, const QString& strMailInfo)
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

QString ApiEntryPrivate::accountInfoUrl(const QString& strToken)
{
    QString strExt = QString("token=%1").arg(strToken);
    QString strUrl = urlFromCommand(WIZNOTE_API_COMMAND_USER_INFO, false);
    return addExtendedInfo(strUrl, strExt);
}

QString ApiEntryPrivate::createGroupUrl(const QString& strToken)
{
    QString strExt = QString("token=%1").arg(strToken);
    QString strUrl = urlFromCommand("create_group", false);
    return addExtendedInfo(strUrl, strExt);
}

QString ApiEntryPrivate::standardCommandUrl(const QString& strCommand, bool bUseWizServer)
{
    QString strUrl = urlFromCommand(strCommand, bUseWizServer);
    return strUrl;
}

QString ApiEntryPrivate::standardCommandUrl(const QString& strCommand, const QString& strToken, bool bUseWizServer)
{
    QString strExt = QString("token=%1").arg(strToken);
    QString strUrl = urlFromCommand(strCommand, bUseWizServer);
    return addExtendedInfo(strUrl, strExt);
}

QString ApiEntryPrivate::standardCommandUrl(const QString& strCommand, const QString& strToken, const QString& strExtInfo, bool bUseWizServer)
{
    QString strExt = QString("token=%1").arg(strToken) + "&" + strExtInfo;
    QString strUrl = urlFromCommand(strCommand, bUseWizServer);
    return addExtendedInfo(strUrl, strExt);
}

QString ApiEntryPrivate::newStandardCommandUrl(const QString& strCommand, const QString& strToken, const QString& strExt, bool bUseWizServer)
{
    QString strUrl = urlFromCommand(strCommand, bUseWizServer);
    QString strExtInfo = QString("&token=%1").arg(strToken);
    strExtInfo.append(strExt.isEmpty() ? "" : "&" + strExt);
    return strUrl + strExtInfo;
}

QString ApiEntryPrivate::groupAttributeUrl(const QString& strToken, const QString& strKbGUID)
{
    QString strExt = QString("token=%1&kb_guid=%2").arg(strToken).arg(strKbGUID);
    QString strUrl = urlFromCommand(WIZNOTE_API_COMMAND_VIEW_GROUP, false);
    return addExtendedInfo(strUrl, strExt);
}

QString ApiEntryPrivate::groupUsersUrl(const QString& strToken, const QString& strBizGUID, const QString& strkbGUID)
{
    QUrl url(syncUrl());
    QString strExt = QString("?token=%1&biz_guid=%2&kb_guid=%3&t=%4")
            .arg(strToken)
            .arg(strBizGUID)
            .arg(strkbGUID)
            .arg(qrand());

    return url.scheme() + "://" + url.host() + "/wizas/a/biz/user_aliases" + strExt;
}

QString ApiEntryPrivate::kUrlFromGuid(const QString& strToken, const QString& strKbGUID)
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
        d = new ApiEntryPrivate();
    }

    ~CGarbo()
    {
        delete d;
    }

    ApiEntryPrivate* d;

};

static CGarbo Garbo;

void ApiEntry::setEnterpriseServerIP(const QString& strIP)
{
    return Garbo.d->setEnterpriseServerIP(strIP);
}

QString ApiEntry::syncUrl()
{
    return Garbo.d->syncUrl();
}

QString ApiEntry::asServerUrl()
{
    return Garbo.d->asServerUrl();
}

QString ApiEntry::messageVersionUrl()
{
    return Garbo.d->messageVersionUrl();
}

QString ApiEntry::avatarDownloadUrl(const QString& strUserGUID)
{
    return Garbo.d->avatarDownloadUrl(strUserGUID);
}

QString ApiEntry::avatarUploadUrl()
{
    return Garbo.d->avatarUploadUrl();
}

QString ApiEntry::mailShareUrl(const QString& strKUrl, const QString& strMailInfo)
{
    return Garbo.d->mailShareUrl(strKUrl, strMailInfo);
}

QString ApiEntry::commentUrl(const QString& strToken, const QString& strKbGUID,const QString& strGUID)
{
    return Garbo.d->commentUrl(strToken, strKbGUID, strGUID);
}

QString ApiEntry::commentCountUrl(const QString& strKUrl, const QString& strToken,
                                  const QString& strKbGUID, const QString& strGUID)
{
    return Garbo.d->commentCountUrl(strKUrl, strToken, strKbGUID, strGUID);
}

QString ApiEntry::feedbackUrl()
{
    return Garbo.d->feedbackUrl();
}

QString ApiEntry::supportUrl()
{
    return Garbo.d->supportUrl();
}

QString ApiEntry::changeLogUrl()
{
    return Garbo.d->changeLogUrl();
}

QString ApiEntry::upgradeUrl()
{
    return Garbo.d->upgradeUrl();
}

QString ApiEntry::analyzerUploadUrl()
{
    return Garbo.d->analyzerUploadUrl();
}

QString ApiEntry::accountInfoUrl(const QString& strToken)
{
    return Garbo.d->accountInfoUrl(strToken);
}

QString ApiEntry::createGroupUrl(const QString& strToken)
{
    return Garbo.d->createGroupUrl(strToken);
}

QString ApiEntry::captchaUrl(const QString& strCaptchaID, int nWidth, int nHeight)
{
    QString strUrl = Garbo.d->asServerUrl();
    strUrl += QString("/a/captcha/%1?width=%2&height=%3").arg(strCaptchaID).arg(nWidth).arg(nHeight);
    return strUrl;
}

QString ApiEntry::standardCommandUrl(const QString& strCommand, bool bUseWizServer)
{
    return Garbo.d->standardCommandUrl(strCommand, bUseWizServer);
}

QString ApiEntry::standardCommandUrl(const QString& strCommand, const QString& strToken, bool bUseWizServer)
{
    return Garbo.d->standardCommandUrl(strCommand, strToken, bUseWizServer);
}
QString ApiEntry::standardCommandUrl(const QString& strCommand, const QString& strToken, const QString& strExtInfo, bool bUseWizServer)
{
    return Garbo.d->standardCommandUrl(strCommand, strToken, strExtInfo, bUseWizServer);
}

QString ApiEntry::newStandardCommandUrl(const QString& strCommand, const QString& strToken, const QString& strExt, bool bUseWizServer)
{
    return Garbo.d->newStandardCommandUrl(strCommand, strToken, strExt, bUseWizServer);
}


QString ApiEntry::groupAttributeUrl(const QString& strToken, const QString& strKbGUID)
{
    return Garbo.d->groupAttributeUrl(strToken, strKbGUID);
}

QString ApiEntry::groupUsersUrl(const QString& strToken, const QString& strBizGUID, const QString& strkbGUID)
{
    return Garbo.d->groupUsersUrl(strToken, strBizGUID, strkbGUID);
}

QString ApiEntry::kUrlFromGuid(const QString& strToken, const QString& strKbGUID)
{
    return Garbo.d->kUrlFromGuid(strToken, strKbGUID);
}

QString ApiEntry::appstoreParam(bool useAndSymbol)
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

