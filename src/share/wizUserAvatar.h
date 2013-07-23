#ifndef WIZUSERAVATAR_H
#define WIZUSERAVATAR_H

#include <QThread>
#include <QPointer>
#include <QUrl>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QStringList>

class CWizUserAvatarDownloader;

/* ---------------------- CWizUserAvatarDownloaderHost ---------------------- */
// thread wrapper, also manage downloading pool

class CWizUserAvatarDownloaderHost : public QObject
{
    Q_OBJECT

public:
    explicit CWizUserAvatarDownloaderHost(const QString& strPath,
                                          QObject* parent = 0);

    void download(const QString& strUserGUID);

private:
    QThread* m_thread;
    CWizUserAvatarDownloader* m_downloader;

    QStringList m_listUser; // download pool
    QString m_strUserCurrent;

    void download_impl();

private Q_SLOTS:
    void on_thread_started();
    void on_downloaded(QString strUserGUID, bool bSucceed);

Q_SIGNALS:
    void downloaded(QString strUserGUID);
};


/* ------------------------- CWizUserAvatarDownload ------------------------- */
// downloader not responsible for determine avatar is out of date or not
// the caller should check the avatar file's timestamp before download

class CWizUserAvatarDownloader : public QObject
{
    Q_OBJECT

public:
    CWizUserAvatarDownloader(const QString& strPath, QObject* parent = 0);
    Q_INVOKABLE void download(const QString& strUserGUID);

private:
    QNetworkAccessManager* m_net;
    QString m_strAvatarPath;
    QString m_strAvatarRequestUrl;

    QString m_strCurrentUser; // current downloading

    void acquireApiEntry();

    void fetchUserAvatar(const QString& strUserGUID);
    void fetchUserAvatarEnd(bool bSucceed);

    QUrl m_urlRedirectedTo;
    QUrl redirectUrl(const QUrl& possibleRedirectUrl,
                     const QUrl& oldRedirectUrl) const;

    bool saveUserAvatar(const QString& strUserGUID, const QByteArray& bytes);

private Q_SLOTS:
    void on_acquireUserAvatarEntry_finished(const QString& strReply);
    void on_queryUserAvatar_finished();

Q_SIGNALS:
    void downloaded(QString strUserGUID, bool bSucceed);
};

#endif // WIZUSERAVATAR_H
