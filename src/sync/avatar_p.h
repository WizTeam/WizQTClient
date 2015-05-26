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
    void load(const QString& strUserID);
    void reload(const QString& strUserID);
    bool avatar(const QString& strUserID, QPixmap* pixmap);
    QPixmap orgAvatar(const QString& strUserID);
    bool customSizeAvatar(const QString& strUserID, int width, int height, QString& strFilePath);

    bool isLoaded(const QString& strUserID);
    bool isFileExists(const QString& strUserID);
    QString keyFromUserID(const QString& strUserID) const;
    QString defaultKey() const;
    bool deleteAvatar(const QString& strUserID);
    //
    void waitForDone();

private:
    QThread* m_thread;
    AvatarDownloader* m_downloader;

    QStringList m_listUser; // download pool
    QString m_strCurrentDownloadingUser;    // current user's id

    bool loadCache(const QString& strUserID);
    void loadCacheDefault();
    bool loadCacheFromFile(const QString &key, const QString& strFilePath);
    //
//    QPixmap loadOrg(const QString& strUserID, bool bForce);
    QPixmap loadOrg(const QString& strUserID);

    void addToDownloadList(const QString& strUserID);
    void download_impl();

    AvatarHost* q;

private Q_SLOTS:
    void on_thread_started();
    void on_downloaded(QString strUserID, bool bSucceed);
};


} // namespace Internal
} // namespace WizService


#endif // WIZSERVICE_AVATAR_P_H
