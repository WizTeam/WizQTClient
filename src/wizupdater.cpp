#include "wizupdater.h"

#include "share/wizmisc.h"
#include "share/wizsettings.h"
#include "zip/wizzip.h"

const QString WIZ_UPGRADE_URL = "http://api.wiz.cn/";

CWizUpdater::CWizUpdater(QObject* parent)
    : QThread(parent)
    , m_net(new QNetworkAccessManager(this))
{

}

void CWizUpdater::checkAndDownloadUpgrade()
{
    start();
}

void CWizUpdater::run()
{
    requestUpgrade();

    connect(this, SIGNAL(tarballDownloaded()), SLOT(on_tarballDownloaded()));

    exec();
}

void CWizUpdater::prepareLocalDisk()
{
    ::WizDeleteAllFilesInFolder(::WizGetUpgradePath());
}

QUrl CWizUpdater::getUpgradeUrl()
{
    QString strUrl = WIZ_UPGRADE_URL + "?p=%1&c=%2&plat=%3";
    strUrl = strUrl.arg("wiz").arg("updatev2").arg("x86");

    return QUrl(strUrl);
}

void CWizUpdater::requestUpgrade()
{
    // send request to download upgrade tarball
    QNetworkRequest request(getUpgradeUrl());
    QNetworkReply* reply = m_net->get(request);
    connect(reply, SIGNAL(finished()), SLOT(on_requestUpgrade_finished()));
}

void CWizUpdater::on_requestUpgrade_finished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());

    if (reply->error()) {
        emit upgradeError(NetworkError);
        exit();
    }

    prepareLocalDisk();

    // redirect to download server.
    QUrl newUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    reply->deleteLater();

    QNetworkRequest request(newUrl);
    QNetworkReply* reply2 = m_net->get(request);
    connect(reply2, SIGNAL(finished()), SLOT(on_requestRedirect_finished()));
}

void CWizUpdater::on_requestRedirect_finished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());

    if (reply->error()) {
        emit upgradeError(NetworkError);
        exit();
    }

    // redirect to real file
    QUrl newUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    reply->deleteLater();

    QNetworkRequest request(newUrl);
    QNetworkReply* reply2 = m_net->get(request);
    connect(reply2, SIGNAL(finished()), SLOT(on_downloadTarball_finished()));
}

void CWizUpdater::on_downloadTarball_finished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());

    if (reply->error()) {
        emit upgradeError(NetworkError);
        exit();
    }

    // download upgrade tarball
    QFile fileReply(::WizGetUpgradePath() + "update.zip");
    fileReply.open(QIODevice::Append);
    fileReply.write(reply->readAll());
    fileReply.close();
    emit tarballDownloaded();

    reply->deleteLater();
}

void CWizUpdater::on_tarballDownloaded()
{
    QString strZip = ::WizGetUpgradePath() + "update.zip";
    if (!CWizUnzipFile::extractZip(strZip, ::WizGetUpgradePath())) {
        emit upgradeError(UnzipError);
        exit();
    }

    // compare files which need be download
    m_config = new CWizSettings(::WizGetUpgradePath() + "config.txt");
    m_strDownloadFileUrl = m_config->GetString("Common", "DownloadFileURL");
    m_strWhatsNewUrl = m_config->GetString("Common", "WhatsNewURL");

    int i = 0;
    QString strFileEntry = m_config->GetString("Files", QString::number(i));
    while (!strFileEntry.isEmpty()) {
        QStringList strFileMeta = strFileEntry.split("*");

        if (strFileMeta.count() != 2) {
            emit upgradeError(ParseError);
            exit();
        }

        m_files.insert(strFileMeta.at(0), strFileMeta.at(1));

        i++;
        strFileEntry = m_config->GetString("Files", QString::number(i));
    }

    beginDownloads();
}

void CWizUpdater::beginDownloads()
{
    return;
}
