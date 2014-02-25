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
#define WIZNOTE_API_ENTRY "http://api.wiz.cn/?p=%1&l=%2&v=%3&c=%4&random=%5&cn=%6&plat=%7&debug=%8"

#define WIZNOTE_API_ARG_PRODUCT "wiz"

#if defined(Q_OS_MAC)
#define WIZNOTE_API_ARG_PLATFORM "macosx"
#elif defined(Q_OS_WIN)
#define WIZNOTE_API_ARG_PLATFORM "windows"
#else
#define WIZNOTE_API_ARG_PLATFORM "linux"
#endif

// command list
#define WIZNOTE_API_COMMAND_SYNC_HTTP       "sync_http"
#define WIZNOTE_API_COMMAND_SYNC_HTTPS      "sync_https"
#define WIZNOTE_API_COMMAND_MESSAGE_VERSION "message_version"
#define WIZNOTE_API_COMMAND_AVATAR          "avatar"
#define WIZNOTE_API_COMMAND_UPLOAD_AVATAR   "upload_avatar"
#define WIZNOTE_API_COMMAND_USER_INFO       "user_info"
#define WIZNOTE_API_COMMAND_VIEW_GROUP      "view_group"
#define WIZNOTE_API_COMMAND_FEEDBACK        "feedback"
#define WIZNOTE_API_COMMAND_COMMENT         "comment"
#define WIZNOTE_API_COMMAND_COMMENT_COUNT   "comment_count"

//#ifdef _M_X64
//    QString strPlatform = "x64";
//#else
//    QString strPlatform = "x86";
//#endif

using namespace WizService;
using namespace WizService::Internal;

ApiEntryPrivate::ApiEntryPrivate()
{
}

ApiEntryPrivate::~ApiEntryPrivate()
{
}


QString ApiEntryPrivate::urlFromCommand(const QString& strCommand)
{
    // random seed
    qsrand((uint)QTime::currentTime().msec());

    QString strUrl = QString(WIZNOTE_API_ENTRY)\
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
        return 0;
    }

    QString strRequestedUrl = QString::fromUtf8(reply->readAll().constData());

    net->deleteLater();

    return strRequestedUrl;
}

QString ApiEntryPrivate::requestUrl(const QString& strCommand, QString& strUrl)
{
    if (!strUrl.isEmpty())
        return strUrl;

    QString strRequestUrl= urlFromCommand(strCommand);

    strUrl = requestUrl(strRequestUrl);
    return strUrl;
}

QString ApiEntryPrivate::syncUrl()
{
    return requestUrl(WIZNOTE_API_COMMAND_SYNC_HTTPS, m_strSyncUrl);
}

QString ApiEntryPrivate::messageVersionUrl()
{
    return requestUrl(WIZNOTE_API_COMMAND_MESSAGE_VERSION, m_strMessageVersionUrl);
}

QString ApiEntryPrivate::avatarDownloadUrl(const QString& strUserGUID)
{
    QString strRawUrl(requestUrl(WIZNOTE_API_COMMAND_AVATAR, m_strAvatarDownloadUrl));

    // http://as.wiz.cn/wizas/a/users/avatar/{userGuid}
    strRawUrl.replace(QRegExp("\\{.*\\}"), strUserGUID);
    strRawUrl += "?default=false"; // Do not download server default avatar
    return strRawUrl;
}

QString ApiEntryPrivate::avatarUploadUrl()
{
    return requestUrl(WIZNOTE_API_COMMAND_UPLOAD_AVATAR, m_strAvatarUploadUrl);
}

QString ApiEntryPrivate::commentUrl(const QString& strToken, const QString& strKbGUID,const QString& strGUID)
{
    if (m_strCommentUrl.isEmpty()) {
        requestUrl(WIZNOTE_API_COMMAND_COMMENT, m_strCommentUrl);
    }

    QString strUrl(m_strCommentUrl);
    strUrl.replace("{token}", strToken);
    strUrl.replace("{kbGuid}", strKbGUID);
    strUrl.replace("{documentGuid}", strGUID);

    return strUrl;
}

QString ApiEntryPrivate::commentCountUrl(const QString& strServer, const QString& strToken,
                                         const QString& strKbGUID, const QString& strGUID)
{
    if (m_strCommentCountUrl.isEmpty()) {
        requestUrl(WIZNOTE_API_COMMAND_COMMENT_COUNT, m_strCommentCountUrl);
    }

    QString strUrl(m_strCommentCountUrl);
    strUrl.replace("{server_host}", strServer);
    strUrl.replace("{token}", strToken);
    strUrl.replace("{kbGuid}", strKbGUID);
    strUrl.replace("{documentGuid}", strGUID);

    // use https
    QUrl url(strUrl);
    url.setScheme("https");

    return url.toString();
}

QString ApiEntryPrivate::feedbackUrl()
{
    m_strFeedbackUrl = urlFromCommand(WIZNOTE_API_COMMAND_FEEDBACK);
    return m_strFeedbackUrl;
}

QString ApiEntryPrivate::accountInfoUrl(const QString& strToken)
{
    QString strExt = QString("token=%1").arg(strToken);
    QString strUrl = urlFromCommand(WIZNOTE_API_COMMAND_USER_INFO);
    return addExtendedInfo(strUrl, strExt);
}

QString ApiEntryPrivate::createGroupUrl(const QString& strToken)
{
    QString strExt = QString("token=%1").arg(strToken);
    QString strUrl = urlFromCommand("create_group");
    return addExtendedInfo(strUrl, strExt);
}

QString ApiEntryPrivate::standardCommandUrl(const QString& strCommand)
{
    QString strUrl = urlFromCommand(strCommand);
    return strUrl;
}

QString ApiEntryPrivate::standardCommandUrl(const QString& strCommand, const QString& strToken)
{
    QString strExt = QString("token=%1").arg(strToken);
    QString strUrl = urlFromCommand(strCommand);
    return addExtendedInfo(strUrl, strExt);
}

QString ApiEntryPrivate::standardCommandUrl(const QString& strCommand, const QString& strToken, const QString& strExtInfo)
{
    QString strExt = QString("token=%1").arg(strToken) + "&" + strExtInfo;
    QString strUrl = urlFromCommand(strCommand);
    return addExtendedInfo(strUrl, strExt);
}

QString ApiEntryPrivate::groupAttributeUrl(const QString& strToken, const QString& strKbGUID)
{
    QString strExt = QString("token=%1&kb_guid=%2").arg(strToken).arg(strKbGUID);
    QString strUrl = urlFromCommand(WIZNOTE_API_COMMAND_VIEW_GROUP);
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




static ApiEntryPrivate* d = 0;

QString ApiEntry::syncUrl()
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->syncUrl();
}

QString ApiEntry::messageVersionUrl()
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->messageVersionUrl();
}

QString ApiEntry::avatarDownloadUrl(const QString& strUserGUID)
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->avatarDownloadUrl(strUserGUID);
}

QString ApiEntry::avatarUploadUrl()
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->avatarUploadUrl();
}

QString ApiEntry::commentUrl(const QString& strToken, const QString& strKbGUID,const QString& strGUID)
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->commentUrl(strToken, strKbGUID, strGUID);
}

QString ApiEntry::commentCountUrl(const QString& strServer, const QString& strToken,
                                  const QString& strKbGUID, const QString& strGUID)
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->commentCountUrl(strServer, strToken, strKbGUID, strGUID);
}

QString ApiEntry::feedbackUrl()
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->feedbackUrl();
}

QString ApiEntry::accountInfoUrl(const QString& strToken)
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->accountInfoUrl(strToken);
}

QString ApiEntry::createGroupUrl(const QString& strToken)
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->createGroupUrl(strToken);
}
QString ApiEntry::standardCommandUrl(const QString& strCommand)
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->standardCommandUrl(strCommand);
}

QString ApiEntry::standardCommandUrl(const QString& strCommand, const QString& strToken)
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->standardCommandUrl(strCommand, strToken);
}
QString ApiEntry::standardCommandUrl(const QString& strCommand, const QString& strToken, const QString& strExtInfo)
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->standardCommandUrl(strCommand, strToken, strExtInfo);
}


QString ApiEntry::groupAttributeUrl(const QString& strToken, const QString& strKbGUID)
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->groupAttributeUrl(strToken, strKbGUID);
}

QString ApiEntry::groupUsersUrl(const QString& strToken, const QString& strBizGUID, const QString& strkbGUID)
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->groupUsersUrl(strToken, strBizGUID, strkbGUID);
}

QString ApiEntry::kUrlFromGuid(const QString& strToken, const QString& strKbGUID)
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->kUrlFromGuid(strToken, strKbGUID);
}
