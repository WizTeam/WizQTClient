#ifndef WIZSERVICE_AVATAR_P_H
#define WIZSERVICE_AVATAR_P_H

#include <QObject>
#include <QStringList>
#include <QUrl>

class QNetworkAccessManager;

namespace WizService {
class AvatarHost;

namespace Internal {


class AvatarDownloader : public QObject
{
    Q_OBJECT

public:
    AvatarDownloader(QObject* parent = 0);
    Q_INVOKABLE void download(const QString& strUserGUID);

private:
    QNetworkAccessManager* m_net;
    QString m_strCurrentUser;

    void fetchUserAvatarEnd(bool bSucceed);

    QUrl m_urlRedirectedTo;
    QUrl redirectUrl(const QUrl& possibleRedirectUrl,
                     const QUrl& oldRedirectUrl) const;

    bool saveDefaultUserAvatar();
    bool save(const QString& strUserGUID, const QByteArray& bytes);

private Q_SLOTS:
    void on_queryUserAvatar_finished();

Q_SIGNALS:
    void downloaded(QString strUserGUID, bool bSucceed);
};


class AvatarHostPrivate: public QObject
{
    Q_OBJECT

public:
    explicit AvatarHostPrivate(AvatarHost* avatarHost);
    void load(const QString& strUserGUID, bool bForce);
    bool avatar(const QString& strUserId, QPixmap* pixmap);
    QPixmap orgAvatar(const QString& strUserId);

    bool isLoaded(const QString& strUserId);
    QString keyFromGuid(const QString& strUserGUID) const;
    QString defaultKey() const;

private:
    QThread* m_thread;
    AvatarDownloader* m_downloader;

    QStringList m_listUser; // download pool
    QString m_strUserCurrent;

    bool isNeedUpdate(const QString& strUserGUID);
    void loadCache(const QString& strUserGUID);
    void loadCacheDefault();
    void loadCacheFromFile(const QString &key, const QString& strFilePath);
    //
    QPixmap loadOrg(const QString& strUserGUID, bool bForce);
    QPixmap loadOrg(const QString& strUserGUID);


    void download_impl();

    AvatarHost* q;

private Q_SLOTS:
    void on_thread_started();
    void on_downloaded(QString strUserGUID, bool bSucceed);
};


} // namespace Internal
} // namespace WizService


#endif // WIZSERVICE_AVATAR_P_H
