#ifndef CWIZUPDATER_H
#define CWIZUPDATER_H

#include <QtNetwork>
#include <QtCore>

class CWizUpgrade;

enum UpdateError {
    NetworkError,
    UnzipError,
    ParseError
};

class CWizUpgradeThread : public QThread
{
    Q_OBJECT

private:
    QPointer<CWizUpgrade> m_upgradePtr;
    bool m_bIsStarted;
    QTimer m_timer;

    QPointer<QThread> m_currentThread;

public:
    CWizUpgradeThread(QObject* parent = 0);

    void abort();
    QThread* thread() const { return m_currentThread; }


protected:
    virtual void run();

public Q_SLOTS:
    void checkUpgrade();
    void checkFinished();

private Q_SLOTS:
    void on_prepareDone(bool bNeedUpgrade);
    void on_upgradeError(UpdateError error);
};


class CWizUpgrade : public QObject
{
    Q_OBJECT

public:
    CWizUpgrade(QObject* parent = 0);

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
    QUrl m_redirectedUrl;

    QList<QStringList> m_files;

    QString m_strWhatsNewUrl;

    // download queue info
    QString m_strDownloadFileUrl;
    QList<QStringList> m_downloadQueue;
    int m_nNeedProcess;
    int m_nProcessTimes;

Q_SIGNALS:
    void upgradeError(UpdateError error);
    void prepareDone(bool bNeedUpdate);

public Q_SLOTS:
    void on_requestUpgrade_finished();
    void on_downloadFile_finished();
    void on_request_error(QNetworkReply::NetworkError error);
};

#endif // CWIZUPDATER_H
