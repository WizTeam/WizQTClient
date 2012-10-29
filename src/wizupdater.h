#ifndef CWIZUPDATER_H
#define CWIZUPDATER_H

#include <QtNetwork>
#include <QtCore>

class CWizSettings;

enum UpdateError {
    NetworkError,
    UnzipError,
    ParseError
};

class CWizUpdater : public QThread
{
    Q_OBJECT

public:
    CWizUpdater(QObject* parent = 0);

    void checkAndDownloadUpgrade();

protected:
    virtual void run();

private:
    QUrl getUpgradeUrl();
    void requestUpgrade();

    void processTarball();
    void generateDownloadQueue();
    void processDownload();

private:
    QNetworkAccessManager* m_net;
    QString m_strDownloadFileUrl;
    QString m_strWhatsNewUrl;
    QList<QStringList> m_files;

    bool m_bIsStarted;

    QList<QStringList> m_downloadQueue;
    int m_nNeedProcess;
    int m_nProcessTimes;

Q_SIGNALS:
    void checkUpdate();
    void upgradeError(UpdateError error);
    void upgradeAvaliable();
    void upgradePreparedDone();

public Q_SLOTS:
    void on_request_checkUpdate();

    void on_requestUpgrade_finished();
    void on_requestRedirect_finished();
    void on_downloadTarball_finished();

    void on_downloadFile_finished();
    void on_downloadFile_error(QNetworkReply::NetworkError error);
};

#endif // CWIZUPDATER_H
