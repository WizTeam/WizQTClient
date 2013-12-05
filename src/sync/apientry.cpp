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

#include "apientry.h"
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
    return requestUrl(WIZNOTE_API_COMMAND_SYNC_HTTP, m_strSyncUrl);
}

QString ApiEntryPrivate::messageVersionUrl()
{
    return requestUrl(WIZNOTE_API_COMMAND_MESSAGE_VERSION, m_strMessageVersionUrl);
}

QString ApiEntryPrivate::avatarDownloadUrl()
{
    return requestUrl(WIZNOTE_API_COMMAND_AVATAR, m_strAvatarDownloadUrl);
}

QString ApiEntryPrivate::avatarUploadUrl()
{
    return requestUrl(WIZNOTE_API_COMMAND_UPLOAD_AVATAR, m_strAvatarUploadUrl);
}

QString ApiEntryPrivate::commentUrl()
{
    return requestUrl(WIZNOTE_API_COMMAND_COMMENT, m_strCommentUrl);
}

QString ApiEntryPrivate::commentCountUrl()
{
    return requestUrl(WIZNOTE_API_COMMAND_COMMENT_COUNT, m_strCommentCountUrl);
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

QString ApiEntryPrivate::groupAttributeUrl(const QString& strToken, const QString& strKbGUID)
{
    QString strExt = QString("token=%1&kb_guid=%2").arg(strToken).arg(strKbGUID);
    QString strUrl = urlFromCommand(WIZNOTE_API_COMMAND_VIEW_GROUP);
    return addExtendedInfo(strUrl, strExt);
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

QString ApiEntry::avatarDownloadUrl()
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->avatarDownloadUrl();
}

QString ApiEntry::avatarUploadUrl()
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->avatarUploadUrl();
}

QString ApiEntry::commentUrl()
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->commentUrl();
}

QString ApiEntry::commentCountUrl()
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->commentCountUrl();
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

QString ApiEntry::groupAttributeUrl(const QString& strToken, const QString& strKbGUID)
{
    if (!d)
        d = new ApiEntryPrivate();
    return d->groupAttributeUrl(strToken, strKbGUID);
}
