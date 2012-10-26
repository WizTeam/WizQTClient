#ifndef CWIZUPDATER_H
#define CWIZUPDATER_H

#include <QtNetwork>
#include <QtCore>

class CWizUpdater : public QThread
{
    Q_OBJECT

public:
    CWizUpdater(QObject* parent = 0);

    void checkAndDownloadUpgrade();

protected:
    virtual void run();

private:
    void prepareLocalDisk();
    void requestUpgrade();

private:
    QNetworkAccessManager* m_net;

    QString m_strRequestFileName;
    QNetworkReply* m_curReply;

Q_SIGNALS:
    void upgradeError();
    void upgradeAvaliable();
    void upgradePreparedDone();

public Q_SLOTS:
    void on_request_readyRead();
    void on_finished_requestUpgrade(QNetworkReply* reply);
};

#endif // CWIZUPDATER_H
