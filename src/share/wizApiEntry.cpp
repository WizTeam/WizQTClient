#include "wizApiEntry.h"

#include <QDebug>

#include "wizdef.h"

/*
 * %1: product, use wiz
 * %2: locale, zh-cn|zh-tw|en-us
 * %3: version number
 * %4: command, see command list below
 * %5: ramdom number
 * %6: computer name, optional
 * %7: platform, ios|android|web|wp7|x86|x64|linux|macos
 * %8: debug, true|false, optional
 */
#define WIZNOTE_API_ENTRY "http://api.wiz.cn/?p=%1&l=%2&v=%3&c=%4&random=%5&cn=%6&plat=%7&debug=%8"

#define WIZNOTE_API_ARG_PRODUCT "wiz"

#if defined(Q_WS_MAC)
#define WIZNOTE_API_ARG_PLATFORM "macosx"
#elif defined(Q_WS_WIN)
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

//#ifdef _M_X64
//    QString strPlatform = "x64";
//#else
//    QString strPlatform = "x86";
//#endif


CWizApiEntry::CWizApiEntry(QObject* parent /* = 0 */)
    : QObject(parent)
{
    m_net = new QNetworkAccessManager(this);

    m_strLocale = QLocale::system().name();
}

void CWizApiEntry::getSyncUrl()
{
    QString requestUrl = WIZNOTE_API_ENTRY;

    // random seed
    qsrand((uint)QTime::currentTime().msec());

    requestUrl = requestUrl\
            .arg(WIZNOTE_API_ARG_PRODUCT)\
            .arg(m_strLocale)\
            .arg(WIZ_CLIENT_VERSION)\
            .arg(WIZNOTE_API_COMMAND_SYNC_HTTP)\
            .arg(qrand())\
            .arg(QHostInfo::localHostName())\
            .arg(WIZNOTE_API_ARG_PLATFORM)\
            .arg("false");

    QNetworkReply* reply = m_net->get(QNetworkRequest(requestUrl));
    connect(reply, SIGNAL(finished()), SLOT(on_acquire_finished()));
}

void CWizApiEntry::getMessageUrl()
{
    QString requestUrl = WIZNOTE_API_ENTRY;

    // random seed
    qsrand((uint)QTime::currentTime().msec());

    requestUrl = requestUrl\
            .arg(WIZNOTE_API_ARG_PRODUCT)\
            .arg(m_strLocale)\
            .arg(WIZ_CLIENT_VERSION)\
            .arg(WIZNOTE_API_COMMAND_MESSAGE_VERSION)\
            .arg(qrand())\
            .arg(QHostInfo::localHostName())\
            .arg(WIZNOTE_API_ARG_PLATFORM)\
            .arg("false");

    QNetworkReply* reply = m_net->get(QNetworkRequest(requestUrl));
    connect(reply, SIGNAL(finished()), SLOT(on_acquire_finished()));
}

void CWizApiEntry::getAvatarUrl()
{
    QString requestUrl = WIZNOTE_API_ENTRY;

    requestUrl = requestUrl\
            .arg(WIZNOTE_API_ARG_PRODUCT)\
            .arg(m_strLocale)\
            .arg(WIZ_CLIENT_VERSION)\
            .arg(WIZNOTE_API_COMMAND_AVATAR)\
            .arg(qrand())\
            .arg(QHostInfo::localHostName())\
            .arg(WIZNOTE_API_ARG_PLATFORM)\
            .arg("false");

    QNetworkReply* reply = m_net->get(QNetworkRequest(requestUrl));
    connect(reply, SIGNAL(finished()), SLOT(on_acquire_finished()));
}

void CWizApiEntry::getAvatarUploadUrl()
{
    QString strUrl = WIZNOTE_API_ENTRY;

    strUrl = strUrl\
            .arg(WIZNOTE_API_ARG_PRODUCT)\
            .arg(QLocale::system().name())\
            .arg(WIZ_CLIENT_VERSION)\
            .arg(WIZNOTE_API_COMMAND_UPLOAD_AVATAR)\
            .arg(qrand())\
            .arg(QHostInfo::localHostName())\
            .arg(WIZNOTE_API_ARG_PLATFORM)\
            .arg("false");

    QNetworkReply* reply = m_net->get(QNetworkRequest(strUrl));
    connect(reply, SIGNAL(finished()), SLOT(on_acquire_finished()));
}

void CWizApiEntry::on_acquire_finished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());

    if (int e = reply->error()) {
        Q_EMIT acquireEntryFinished(QString());
        reply->deleteLater();
        return;
    }

    if (!reply->open(QIODevice::ReadOnly)) {
        Q_EMIT acquireEntryFinished(QString());
        reply->deleteLater();
        return;
    }

    QString strReply = reply->readAll();
    qDebug() << "[CWizApiEntry] reply: " << strReply;

    Q_EMIT acquireEntryFinished(strReply);
    reply->close();
    reply->deleteLater();
}

QString CWizApiEntry::getAccountInfoUrl(const QString& strToken)
{
    QString strExtInfo("token=%1");
    strExtInfo = strExtInfo.arg(strToken);

    QString strUrl = QString(WIZNOTE_API_ENTRY) + "&a=%9";

    strUrl = strUrl\
            .arg(WIZNOTE_API_ARG_PRODUCT)\
            .arg(QLocale::system().name())\
            .arg(WIZ_CLIENT_VERSION)\
            .arg(WIZNOTE_API_COMMAND_USER_INFO)\
            .arg(qrand())\
            .arg(QHostInfo::localHostName())\
            .arg(WIZNOTE_API_ARG_PLATFORM)\
            .arg("false")\
            .arg(QUrl::toPercentEncoding(strExtInfo).constData());

    return strUrl;
}

QString CWizApiEntry::getGroupAttributeUrl(const QString& strToken, const QString& strKbGUID)
{
    QString strExtInfo("token=%1&kb_guid=%2");
    strExtInfo = strExtInfo.arg(strToken).arg(strKbGUID);

    QString strUrl = QString(WIZNOTE_API_ENTRY) + "&a=%9";

    strUrl = strUrl\
            .arg(WIZNOTE_API_ARG_PRODUCT)\
            .arg(QLocale::system().name())\
            .arg(WIZ_CLIENT_VERSION)\
            .arg(WIZNOTE_API_COMMAND_VIEW_GROUP)\
            .arg(qrand())\
            .arg(QHostInfo::localHostName())\
            .arg(WIZNOTE_API_ARG_PLATFORM)\
            .arg("false")\
            .arg(QUrl::toPercentEncoding(strExtInfo).constData());

    return strUrl;
}

QString CWizApiEntry::getFeedbackUrl()
{
    QString strUrl = WIZNOTE_API_ENTRY;

    strUrl = strUrl\
            .arg(WIZNOTE_API_ARG_PRODUCT)\
            .arg(QLocale::system().name())\
            .arg(WIZ_CLIENT_VERSION)\
            .arg(WIZNOTE_API_COMMAND_FEEDBACK)\
            .arg(qrand())\
            .arg(QHostInfo::localHostName())\
            .arg(WIZNOTE_API_ARG_PLATFORM)\
            .arg("false");

    return strUrl;
}
