#ifndef CWIZUPDATER_H
#define CWIZUPDATER_H

#include <QtNetwork>
#include <QtCore>

#ifdef WIZ_OBOSOLETE

class WizUpgrade;

enum UpdateError {
    NetworkError,
    UnzipError,
    ParseError
};

class WizUpgradeThread : public QThread
{
    Q_OBJECT

public:
    WizUpgradeThread(QObject* parent = 0);

    void abort();
    const QString& whatsNewUrl() const { return m_changelogUrl; }
    QThread* thread() const { return m_currentThread; }

private:
    QPointer<WizUpgrade> m_upgradePtr;
    bool m_bIsStarted;
    QTimer m_timer;

    QPointer<QThread> m_currentThread;
    QString m_changelogUrl;

protected:
    virtual void run();

public Q_SLOTS:
    void checkUpgradeBegin();
    void checkUpgradeFinished();

private Q_SLOTS:
    void on_prepareDone(bool bNeedUpgrade);
    void on_upgradeError(UpdateError error);
};


class WizUpgrade : public QObject
{
    Q_OBJECT

public:
    WizUpgrade(QObject* parent = 0);

    void requestUpgrade();
    void abort();

    QString whatsNewUrl() const { return m_strWhatsNewUrl; }

private:
    QString getUpgradeUrl();
    void requestUpgrade_impl(QString const& url);
    QUrl redirectUrl(QUrl const &possible_redirect_url, \
                     QUrl const &old_redirect_url) const;

    void processTarball();
    void readMetadata();
    void generateDownloadQueue();
    void processDownload();

    void moveToEnd();

private:
    QNetworkAccessManager m_net;

    // download queue info
    QString m_strDownloadFileUrl;
    QList<QStringList> m_downloadQueue;
    int m_nNeedProcess;
    int m_nProcessTimes;

    // internal usage
    QUrl m_redirectedUrl;
    QList<QStringList> m_files;
    bool m_bNewVersion;

    QString m_strWhatsNewUrl;

Q_SIGNALS:
    void upgradeError(UpdateError error);
    void prepareDone(bool bNeedUpdate);

public Q_SLOTS:
    void on_requestUpgrade_finished();
    void on_downloadFile_finished();
    void on_request_error(QNetworkReply::NetworkError error);
};

#endif // WIZ_OBSOLETE

#endif // CWIZUPDATER_H
