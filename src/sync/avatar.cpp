#include "avatar.h"
#include "avatar_p.h"

#include <QThread>
#include <QImage>
#include <QTimer>
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QFileInfo>
#include <QPixmap>
#include <QPixmapCache>
#include <QDateTime>

#include "apientry.h"
#include "../utils/pathresolve.h"
#include "../utils/stylehelper.h"

using namespace WizService;
using namespace WizService::Internal;

/* ----------------------- AvatarDownloader ----------------------- */
AvatarDownloader::AvatarDownloader(QObject* parent)
    : QObject(parent)
    , m_net(new QNetworkAccessManager(this))
{
}

void AvatarDownloader::download(const QString& strUserGUID)
{
    m_strCurrentUser = strUserGUID;
    QString strUrl = ApiEntry::avatarDownloadUrl(strUserGUID);
    if (strUrl.isEmpty()) {
        return;
    }

    QNetworkReply* reply = m_net->get(QNetworkRequest(strUrl));
    connect(reply, SIGNAL(finished()), SLOT(on_queryUserAvatar_finished()));
}

void AvatarDownloader::on_queryUserAvatar_finished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error()) {
        qDebug() << "[AvatarHost]Error occured: " << reply->errorString();
        fetchUserAvatarEnd(false);
        return;
    }

    // cause we use "default", redirection may occur
    QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    m_urlRedirectedTo = redirectUrl(possibleRedirectUrl.toUrl(), m_urlRedirectedTo);

    if(!m_urlRedirectedTo.isEmpty()) {
        qDebug() << "[AvatarHost]fetching redirected, url: "
                 << m_urlRedirectedTo.toString();

        QNetworkReply* replyNext = m_net->get(QNetworkRequest(m_urlRedirectedTo));
        connect(replyNext, SIGNAL(finished()), SLOT(on_queryUserAvatar_finished()));
    } else {
        // finally arrive destination...

        // read and save avatar
        QByteArray bReply = reply->readAll();

        if (!save(m_strCurrentUser, bReply)) {
            qDebug() << "[AvatarHost]failed: unable to save user avatar, guid: " << m_strCurrentUser;
            fetchUserAvatarEnd(false);
            return;
        }

        qDebug() << "[AvatarHost]fetching finished, guid: " << m_strCurrentUser;
        fetchUserAvatarEnd(true);
    }
}

QUrl AvatarDownloader::redirectUrl(const QUrl& possibleRedirectUrl,
                                         const QUrl& oldRedirectUrl) const
{
    QUrl redirectUrl;

    if(!possibleRedirectUrl.isEmpty() && possibleRedirectUrl != oldRedirectUrl)
        redirectUrl = possibleRedirectUrl;
    return redirectUrl;
}

void AvatarDownloader::fetchUserAvatarEnd(bool bSucceed)
{
    Q_EMIT downloaded(m_strCurrentUser, bSucceed);
}

bool AvatarDownloader::save(const QString& strUserGUID, const QByteArray& bytes)
{
    QString strFileName = Utils::PathResolve::avatarPath() + strUserGUID + ".png";
    QImage img = QImage::fromData(bytes);

    return img.save(strFileName);
}


/* --------------------- AvatarHostPrivate --------------------- */

AvatarHostPrivate::AvatarHostPrivate(AvatarHost* avatarHost)
    : q(avatarHost)
{
    m_downloader = new AvatarDownloader();
    connect(m_downloader, SIGNAL(downloaded(QString, bool)),
            SLOT(on_downloaded(QString, bool)));

    m_thread = new QThread(this);
    m_thread->setPriority(QThread::IdlePriority);
    connect(m_thread, SIGNAL(started()), SLOT(on_thread_started()));

    m_downloader->moveToThread(m_thread);

    loadCacheDefault();
}

bool AvatarHostPrivate::isLoaded(const QString& strUserId)
{
    QPixmap pm;
    bool ret = QPixmapCache::find(keyFromGuid(strUserId), pm);
    qDebug() << "[AvatarHost]search: " << keyFromGuid(strUserId) << "result:" << ret;
    return ret;
}

bool AvatarHostPrivate::isNeedUpdate(const QString& strUserGUID)
{
    QString strFilePath = Utils::PathResolve::avatarPath() + strUserGUID + ".png";
    if (!QFile::exists(strFilePath)) {
        return true;
    }

    QPixmap pm(strFilePath);
    QFileInfo info(strFilePath);
    QDateTime tCreated = info.created();
    QDateTime tNow = QDateTime::currentDateTime();
    if (tCreated.daysTo(tNow) >= 1 || pm.isNull()) { // download avatar before yesterday or pixmap is not valid
        return true;
    }

    return false;
}

void AvatarHostPrivate::loadCache(const QString& strUserGUID)
{
    QString strFilePath = Utils::PathResolve::avatarPath() + strUserGUID + ".png";
    //qDebug() << "[AvatarHost]load avatar: " << strFilePath;
    loadCacheFromFile(keyFromGuid(strUserGUID), strFilePath);
}


QPixmap AvatarHostPrivate::loadOrg(const QString& strUserGUID)
{
    QString strFilePath = Utils::PathResolve::avatarPath() + strUserGUID + ".png";
    //
    QPixmap ret(strFilePath);
    if (!ret.isNull())
        return ret;
    //
    QString defaultFilePath = Utils::PathResolve::themePath("default") + "avatar_default.png";
    return QPixmap(defaultFilePath);
}

void AvatarHostPrivate::loadCacheDefault()
{
    loadCacheFromFile(defaultKey(), Utils::PathResolve::themePath("default") + "avatar_default.png");
}

void AvatarHostPrivate::loadCacheFromFile(const QString& key, const QString& strFilePath)
{
    QPixmap pixmap(strFilePath);

    if(pixmap.isNull()) {
        qDebug() << "[AvatarHost]failed to load cache: " << strFilePath;
        return;
    }

    pixmap = pixmap.scaled(Utils::StyleHelper::avatarSize(),
                           Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    Q_ASSERT(!pixmap.isNull());

    if (!QPixmapCache::insert(key, pixmap)) {
        qDebug() << "[AvatarHost]failed to insert cache: " << strFilePath;
        return;
    }

    //qDebug() << "[AvatarHost]loaded: " << key;
}

QString AvatarHostPrivate::keyFromGuid(const QString& strUserGUID) const
{
    return "WizService::Avatar::" + strUserGUID;
}

QString AvatarHostPrivate::defaultKey() const
{
    return "WizService::Avatar::Default";
}

bool AvatarHostPrivate::avatar(const QString& strUserId, QPixmap* pixmap)
{
    if (QPixmapCache::find(keyFromGuid(strUserId), pixmap)) {
        return true;
    }

    load(strUserId, false);

    if (QPixmapCache::find(defaultKey(), pixmap)) {
        return true;
    } else {
        loadCacheDefault();
    }

    if (QPixmapCache::find(defaultKey(), pixmap)) {
        return true;
    }

    Q_ASSERT(0);
    return false;
}

QPixmap AvatarHostPrivate::orgAvatar(const QString& strUserId)
{
    return loadOrg(strUserId, false);
}


QPixmap AvatarHostPrivate::loadOrg(const QString& strUserGUID, bool bForce)
{
    if (isNeedUpdate(strUserGUID) || bForce) {
        if (!m_listUser.contains(strUserGUID) && strUserGUID != m_strUserCurrent) {
            m_listUser.append(strUserGUID);
            m_thread->start();
        }

        return QPixmap();
    }
    return loadOrg(strUserGUID);
}

void AvatarHostPrivate::load(const QString& strUserGUID, bool bForce)
{
    if (isNeedUpdate(strUserGUID) || bForce) {
        if (!m_listUser.contains(strUserGUID) && strUserGUID != m_strUserCurrent) {
            m_listUser.append(strUserGUID);
            m_thread->start();
        }

        return;
    }

    QPixmap pm;
    if (!QPixmapCache::find(keyFromGuid(strUserGUID), pm)) {
        loadCache(strUserGUID);
        Q_EMIT q->loaded(strUserGUID);
    }
}

void AvatarHostPrivate::download_impl()
{
    if (m_listUser.isEmpty()) {
        qDebug() << "[AvatarHost]download pool is clean, thread: "
                 << QThread::currentThreadId();

        m_thread->quit();
        return;
    }

    m_strUserCurrent = m_listUser.takeFirst();

    if (!QMetaObject::invokeMethod(m_downloader, "download",
                                   Q_ARG(QString, m_strUserCurrent))) {
        qDebug() << "[AvatarHost]failed: unable to invoke download!";
    }
}

void AvatarHostPrivate::on_thread_started()
{
    download_impl();
}

void AvatarHostPrivate::on_downloaded(QString strUserGUID, bool bSucceed)
{
    if (bSucceed) {
        m_strUserCurrent.clear(); // Clear current otherwise download twice will be failed
        loadCache(strUserGUID);
        Q_EMIT q->loaded(strUserGUID);
    }

    download_impl();
}

/* --------------------- AvatarHost --------------------- */

static AvatarHostPrivate* d = 0;
static AvatarHost* m_instance = 0;

AvatarHost::AvatarHost()
{
    Q_ASSERT(!m_instance);

    m_instance = this;
    d = new AvatarHostPrivate(this);
}

AvatarHost::~AvatarHost()
{
    delete d;
    d = 0;
}

AvatarHost* AvatarHost::instance()
{
    return m_instance;
}

// if bForce == true, download it from server and update cache, default is false
void AvatarHost::load(const QString& strUserGUID, bool bForce)
{
    d->load(strUserGUID, bForce);
}

// retrieve pixmap from cache, return default avatar if not exist
bool AvatarHost::avatar(const QString& strUserId, QPixmap* pixmap)
{
    return d->avatar(strUserId, pixmap);
}
QPixmap AvatarHost::orgAvatar(const QString& strUserId)
{
    return d->orgAvatar(strUserId);
}
bool AvatarHost::isLoaded(const QString& strUserId)
{
    return d->isLoaded(strUserId);
}

// For user want to retrive avatar from global pixmap cache
QString AvatarHost::keyFromGuid(const QString& strUserGUID)
{
    return d->keyFromGuid(strUserGUID);
}

// the default avatar's key for fallback drawing
QString AvatarHost::defaultKey()
{
    return d->defaultKey();
}
