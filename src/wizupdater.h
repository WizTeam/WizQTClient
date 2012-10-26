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
    void prepareLocalDisk();
    void requestUpgrade();
    void beginDownloads();

private:
    QNetworkAccessManager* m_net;
    CWizSettings* m_config;
    QString m_strDownloadFileUrl;
    QString m_strWhatsNewUrl;
    QHash<QString, QString> m_files;

Q_SIGNALS:
    void upgradeError(UpdateError error);
    void upgradeAvaliable();
    void upgradePreparedDone();

    void tarballDownloaded();

public Q_SLOTS:
    void on_requestUpgrade_finished();
    void on_requestRedirect_finished();
    void on_downloadTarball_finished();

    void on_tarballDownloaded();
};

#endif // CWIZUPDATER_H
