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

// command list
#define WIZNOTE_API_COMMAND_SYNC_HTTP "sync_http"
#define WIZNOTE_API_COMMAND_MESSAGE_VERSION "message_version"
#define WIZNOTE_API_COMMAND_AVATAR "avatar"


CWizApiEntry::CWizApiEntry(QObject* parent /* = 0 */)
    : QObject(parent)
{
    m_net = new QNetworkAccessManager(this);

    m_strLocale = QLocale::system().name();

#ifdef Q_OS_MAC
    m_strPlatform = "macos";
#else
    m_strPlatform = "linux";
#endif
}

void CWizApiEntry::getSyncUrl()
{
    QString requestUrl = WIZNOTE_API_ENTRY;

    // random seed
    qsrand((uint)QTime::currentTime().msec());

    requestUrl = requestUrl\
            .arg("wiz")\
            .arg(m_strLocale)\
            .arg(WIZ_CLIENT_VERSION)\
            .arg(WIZNOTE_API_COMMAND_SYNC_HTTP)\
            .arg(qrand())\
            .arg(QHostInfo::localHostName())\
            .arg(m_strPlatform)\
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
            .arg("wiz")\
            .arg(m_strLocale)\
            .arg(WIZ_CLIENT_VERSION)\
            .arg(WIZNOTE_API_COMMAND_MESSAGE_VERSION)\
            .arg(qrand())\
            .arg(QHostInfo::localHostName())\
            .arg(m_strPlatform)\
            .arg("false");

    QNetworkReply* reply = m_net->get(QNetworkRequest(requestUrl));
    connect(reply, SIGNAL(finished()), SLOT(on_acquire_finished()));
}

void CWizApiEntry::getAvatarUrl()
{
    QString requestUrl = WIZNOTE_API_ENTRY;

    requestUrl = requestUrl\
            .arg("wiz")\
            .arg(m_strLocale)\
            .arg(WIZ_CLIENT_VERSION)\
            .arg(WIZNOTE_API_COMMAND_AVATAR)\
            .arg(qrand())\
            .arg(QHostInfo::localHostName())\
            .arg(m_strPlatform)\
            .arg("false");

    QNetworkReply* reply = m_net->get(QNetworkRequest(requestUrl));
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
    Q_EMIT acquireEntryFinished(strReply);
    reply->close();
    reply->deleteLater();
}

