#include "avatar.h"
#include "avatar_p.h"

#include <QThread>
#include <QImage>
#include <QTimer>
#include <QMutexLocker>
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QFileInfo>
#include <QPixmap>
#include <QPixmapCache>
#include <QDateTime>
#include <QPainter>

#include "apientry.h"
#include "../utils/pathresolve.h"
#include "../utils/stylehelper.h"

#include "../share/wizmisc.h"
#include "share/wizthreads.h"
#include "share/wizEventLoop.h"

using namespace WizService;
using namespace WizService::Internal;

/* ----------------------- AvatarDownloader ----------------------- */
AvatarDownloader::AvatarDownloader(QObject* parent)
    : QObject(parent)
    , m_net(nullptr)
{
}

void AvatarDownloader::download(const QString& strUserGUID, bool isSystemAvatar)
{
    if (m_net.get() == nullptr)
    {
        m_net = std::make_shared<QNetworkAccessManager>();
    }

    m_strCurrentUser = strUserGUID;
#ifdef Q_OS_LINUX
    QString strUrl = CommonApiEntry::avatarDownloadUrl(strUserGUID);
#else
    QString standGID = QUrl::toPercentEncoding(strUserGUID);
    QString strUrl = isSystemAvatar ? CommonApiEntry::systemAvatarUrl(standGID)
                                    : CommonApiEntry::avatarDownloadUrl(standGID);
#endif
    if (strUrl.isEmpty()) {
        return;
    }

    //
    queryUserAvatar(strUrl);
}

void AvatarDownloader::queryUserAvatar(const QString& strUrl)
{
    qDebug() << "downloader start to download : " << m_strCurrentUser;
    QNetworkReply* reply = m_net->get(QNetworkRequest(strUrl));
    CWizAutoTimeOutEventLoop loop(reply);
    loop.exec();

    if (loop.error() != QNetworkReply::NoError)
    {
        fetchUserAvatarEnd(false);
        return;
    }

    // cause we use "default", redirection may occur
    QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    m_urlRedirectedTo = redirectUrl(possibleRedirectUrl.toUrl(), m_urlRedirectedTo);


    if(!m_urlRedirectedTo.isEmpty()) {
        qDebug() << "[AvatarHost]fetching redirected, url: "
                 << m_urlRedirectedTo.toString();

        queryUserAvatar(m_urlRedirectedTo.toString());
    } else {
        // read and save avatar
        QByteArray bReply = loop.result();

        if (!save(m_strCurrentUser, bReply))
        {
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
    if (QFile::exists(strFileName)) {
        ::WizDeleteFile(strFileName);
        qDebug() << "[AvatarHost]avatar file exists , remove it :" << !QFile::exists(strFileName);
    }
    QImage img = QImage::fromData(bytes);

    return img.save(strFileName);
}


/* --------------------- AvatarHostPrivate --------------------- */

AvatarHostPrivate::AvatarHostPrivate(AvatarHost* avatarHost)
    : q(avatarHost)
    , m_downloader(new AvatarDownloader(this))
{
    connect(m_downloader, SIGNAL(downloaded(QString, bool)),
            SLOT(on_downloaded(QString, bool)));
    loadCacheDefault();
}

bool AvatarHostPrivate::isLoaded(const QString& strUserID)
{
    QPixmap pm;
    bool ret = QPixmapCache::find(keyFromUserID(strUserID), pm);
    qDebug() << "[AvatarHost]search: " << keyFromUserID(strUserID) << "result:" << ret;
    return ret;
}

bool AvatarHostPrivate::isFileExists(const QString& strUserID)
{
    QString strFile = Utils::PathResolve::avatarPath() + strUserID + ".png";
    return QFile::exists(strFile);
}

bool AvatarHostPrivate::loadCache(const QString& strUserID)
{
    QString strFilePath = Utils::PathResolve::avatarPath() + strUserID + ".png";
    return loadCacheFromFile(keyFromUserID(strUserID), strFilePath);
}


QPixmap AvatarHostPrivate::loadOrg(const QString& strUserID)
{
    QString strFilePath = Utils::PathResolve::avatarPath() + strUserID + ".png";
    //
    QPixmap ret(strFilePath);
    if (!ret.isNull())
        return ret;
    //
    QString defaultFilePath = Utils::PathResolve::skinResourcesPath("default") + "avatar_default.png";
    return QPixmap(defaultFilePath);
}

void AvatarHostPrivate::addToDownloadList(const QString& strUserID, bool isSystem)
{
    appendUserID(strUserID, isSystem);

    download_impl();
}

bool AvatarHostPrivate::customSizeAvatar(const QString& strUserID, int width, int height, QString& strFilePath)
{
    strFilePath = Utils::PathResolve::tempPath() + strUserID + QString::number(width) + "x" + QString::number(height) + ".png";
    if (QFile::exists(strFilePath))
        return true;

    QPixmap orgPix = loadOrg(strUserID);
    if (orgPix.isNull())
        return false;

    QPixmap customPix = orgPix.scaled(width, height);
    return customPix.save(strFilePath);
}

void AvatarHostPrivate::loadCacheDefault()
{
    loadCacheFromFile(defaultKey(), Utils::PathResolve::skinResourcesPath("default") + "avatar_default.png");
}

bool AvatarHostPrivate::loadCacheFromFile(const QString& key, const QString& strFilePath)
{
    QPixmap pixmap(strFilePath);

    if(pixmap.isNull()) {
        qDebug() << "[AvatarHost]failed to load cache: " << strFilePath;
        return false;
    }

    //
    QSize sz = Utils::StyleHelper::avatarSize();
    pixmap = AvatarHost::circleImage(pixmap, sz.width(), sz.height());
    //
    if (pixmap.isNull())
        return false;

    Q_ASSERT(!pixmap.isNull());

    if (!QPixmapCache::insert(key, pixmap)) {
        qDebug() << "[AvatarHost]failed to insert cache: " << strFilePath;
        return false;
    }

    return true;
}

QString AvatarHostPrivate::keyFromUserID(const QString& strUserID) const
{
    if (strUserID.isEmpty())
        return defaultKey();

    return "WizService::Avatar::" + strUserID;
}

QString AvatarHostPrivate::defaultKey() const
{
    return "WizService::Avatar::Default";
}

bool AvatarHostPrivate::deleteAvatar(const QString& strUserID)
{
    qDebug() << "[AvatarHost]remove user avatar: " << strUserID;
    QPixmapCache::remove(keyFromUserID(strUserID));
    QString strAvatarPath = Utils::PathResolve::avatarPath();
    return DeleteFile(strAvatarPath + strUserID + _T(".png"));
}

bool AvatarHostPrivate::avatar(const QString& strUserID, QPixmap* pixmap)
{
    if (QPixmapCache::find(keyFromUserID(strUserID), pixmap)) {
        return true;
    }

    if (!strUserID.isEmpty()) {
        load(strUserID, false);
    }

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

bool AvatarHostPrivate::systemAvatar(const QString& avatarName, QPixmap* pixmap)
{
    if (QPixmapCache::find(keyFromUserID(avatarName), pixmap)) {
        return true;
    }

    if (!avatarName.isEmpty()) {
        load(avatarName, true);
    }

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

QPixmap AvatarHostPrivate::orgAvatar(const QString& strUserID)
{
//    return loadOrg(strUserID, false);
    return loadOrg(strUserID);
}


//QPixmap AvatarHostPrivate::loadOrg(const QString& strUserID, bool bForce)
//{
//    if (bForce) {
//        if (!m_listUser.contains(strUserID) && strUserID != m_strCurrentDownloadingUser) {
//            m_listUser.append(strUserID);
//            m_thread->start(QThread::IdlePriority);
//        }
//    }
//    return loadOrg(strUserID);
//}

void AvatarHostPrivate::load(const QString& strUserID, bool isSystem)
{
    //
    QPixmap pm;
    if (!QPixmapCache::find(keyFromUserID(strUserID), pm))
    {
        if (loadCache(strUserID))
        {
            Q_EMIT q->loaded(strUserID);
        }
        else
        {
            QString defaultFilePath = Utils::PathResolve::skinResourcesPath("default") + "avatar_default.png";
            loadCacheFromFile(keyFromUserID(strUserID), defaultFilePath);
            Q_EMIT q->loaded(strUserID);            

            // can find item, download from server
            addToDownloadList(strUserID, isSystem);
        }
    }
}

void AvatarHostPrivate::reload(const QString& strUserID)
{    
    addToDownloadList(strUserID, false);
}

void AvatarHostPrivate::download_impl()
{
    if (!m_currentDownloadingUser.userID.isEmpty())
        return;

    peekUserID(m_currentDownloadingUser);

    if (m_currentDownloadingUser.userID.isEmpty())
    {
        qDebug() << "[AvatarHost]download pool is clean, thread: "
                 << QThread::currentThreadId();

        return;
    }

    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
        m_downloader->download(m_currentDownloadingUser.userID, m_currentDownloadingUser.isSystemAvatar);
    });
}

void AvatarHostPrivate::appendUserID(const QString& strUserID, bool isSystem)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);

    foreach (DownloadingUser user, m_listUser)
    {
        if (user.userID == strUserID)
            return;
    }


    DownloadingUser user;
    user.userID = strUserID;
    user.isSystemAvatar = isSystem;
    m_listUser.append(user);
}

void AvatarHostPrivate::peekUserID(DownloadingUser& user)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);

    if (!m_listUser.isEmpty())
    {
        DownloadingUser topUser = m_listUser.takeFirst();
        user.userID = topUser.userID;
        user.isSystemAvatar = topUser.isSystemAvatar;
    }
}

void AvatarHostPrivate::on_downloaded(QString strUserID, bool bSucceed)
{
    if (bSucceed)
    {
        loadCache(strUserID);
        Q_EMIT q->loaded(strUserID);        
    }

    //  下载列表中的下一个头像
    m_currentDownloadingUser.userID.clear();
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

void AvatarHost::load(const QString& strUserID, bool isSystem)
{
    d->load(strUserID, isSystem);
}

void AvatarHost::reload(const QString& strUserID)
{
    d->reload(strUserID);
}

// retrieve pixmap from cache, return default avatar if not exist
bool AvatarHost::avatar(const QString& strUserID, QPixmap* pixmap)
{
    return d->avatar(strUserID, pixmap);
}

bool AvatarHost::systemAvatar(const QString& avatarName, QPixmap* pixmap)
{
    return d->systemAvatar(avatarName, pixmap);
}

bool AvatarHost::deleteAvatar(const QString& strUserID)
{
    return d->deleteAvatar(strUserID);
}
QPixmap AvatarHost::orgAvatar(const QString& strUserID)
{
    return d->orgAvatar(strUserID);
}
bool AvatarHost::isLoaded(const QString& strUserID)
{
    return d->isLoaded(strUserID);
}

bool AvatarHost::isFileExists(const QString& strUserID)
{
    return d->isFileExists(strUserID);
}

// For user want to retrive avatar from global pixmap cache
QString AvatarHost::keyFromUserID(const QString& strUserID)
{
    return d->keyFromUserID(strUserID);
}

// the default avatar's key for fallback drawing
QString AvatarHost::defaultKey()
{
    return d->defaultKey();
}

bool AvatarHost::customSizeAvatar(const QString& strUserID, int width, int height, QString& strFileName)
{
    return d->customSizeAvatar(strUserID, width, height, strFileName);
}

QPixmap AvatarHost::corpImage(const QPixmap& org)
{
    if (org.isNull())
        return org;
    //
    QSize sz = org.size();
    //
    int width = sz.width();
    int height = sz.height();
    if (width == height)
        return org;
    //
    if (width > height)
    {
        int xOffset = (width - height) / 2;
        return org.copy(xOffset, 0, height, height);
    }
    else
    {
        int yOffset = (height - width) / 2;
        return org.copy(0, yOffset, width, width);
    }
}

QPixmap AvatarHost::circleImage(const QPixmap& src, int width, int height)
{
    QPixmap org = corpImage(src);
    //
    int largeWidth = width * 8;
    int largeHeight = height * 8;
    //
    QPixmap orgResized = org.scaled(QSize(largeWidth, largeHeight), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    //
    QPixmap largePixmap(QSize(largeWidth, largeHeight));
    largePixmap.fill(QColor(Qt::transparent));
    //
    QPainter painter(&largePixmap);
    //
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
    QPainterPath path;
    path.addEllipse(0, 0, largeWidth, largeHeight);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, orgResized);
    //
    return largePixmap.scaled(QSize(width, height), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}
