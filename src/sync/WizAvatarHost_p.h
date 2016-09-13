#ifndef WIZSERVICE_AVATAR_P_H
#define WIZSERVICE_AVATAR_P_H

#include <QObject>
#include <QStringList>
#include <QUrl>
#include <QMutex>
#include <memory>

class QNetworkReply;
class QNetworkAccessManager;
class WizAvatarHost;


class WizAvatarDownloader : public QObject
{
    Q_OBJECT

public:
    WizAvatarDownloader(QObject* parent = 0);
    Q_INVOKABLE void download(const QString& strUserGUID, bool isSystemAvatar);

private:
    std::shared_ptr<QNetworkAccessManager> m_net;
    QString m_strCurrentUser;

    void fetchUserAvatarEnd(bool bSucceed);

    QUrl m_urlRedirectedTo;
    QUrl redirectUrl(const QUrl& possibleRedirectUrl,
                     const QUrl& oldRedirectUrl) const;

    bool saveDefaultUserAvatar();
    bool save(const QString& strUserGUID, const QByteArray& bytes);
    void queryUserAvatar(const QString& strUrl);

Q_SIGNALS:
    void downloaded(QString strUserGUID, bool bSucceed);
};


class WizAvatarHostPrivate: public QObject
{
    Q_OBJECT

public:
    explicit WizAvatarHostPrivate(WizAvatarHost* avatarHost);
    void load(const QString& strUserID, bool isSystem);
    void reload(const QString& strUserID);
    bool avatar(const QString& strUserID, QPixmap* pixmap);
    bool systemAvatar(const QString& avatarName, QPixmap* pixmap);
    QPixmap orgAvatar(const QString& strUserID);
    bool customSizeAvatar(const QString& strUserID, int width, int height, QString& strFilePath);

    bool isLoaded(const QString& strUserID);
    bool isFileExists(const QString& strUserID);
    QString keyFromUserID(const QString& strUserID) const;
    QString defaultKey() const;
    bool deleteAvatar(const QString& strUserID);
    //

private:
    QThread* m_thread;
    QMutex m_mutex;
    WizAvatarDownloader* m_downloader;


    struct DownloadingUser
    {
        QString userID;
        bool isSystemAvatar;
    };

    QList<DownloadingUser> m_listUser; // download pool
    DownloadingUser m_currentDownloadingUser;    // current user's id

    bool loadCache(const QString& strUserID);
    void loadCacheDefault();
    bool loadCacheFromFile(const QString &key, const QString& strFilePath);
    //
//    QPixmap loadOrg(const QString& strUserID, bool bForce);
    QPixmap loadOrg(const QString& strUserID);

    void addToDownloadList(const QString& strUserID, bool isSystem);
    void download_impl();

    void appendUserID(const QString& strUserID, bool isSystem);
    void peekUserID(DownloadingUser& user);

    WizAvatarHost* q;

private Q_SLOTS:
    void on_downloaded(QString strUserID, bool bSucceed);
};



#endif // WIZSERVICE_AVATAR_P_H
