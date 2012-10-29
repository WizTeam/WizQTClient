#include "wizupdater.h"

#include "share/wizmisc.h"
#include "share/wizsettings.h"
#include "zip/wizzip.h"

#define WIZ_CHECK_UPDATE_TIMEOUT 1000*60*60*3
const QString WIZ_UPGRADE_URL = "http://api.wiz.cn/";

CWizUpdater::CWizUpdater(QObject* parent)
    : QThread(parent)
    , m_net(new QNetworkAccessManager(this))
    , m_bIsStarted(false)
    , m_nProcessTimes(0)
{
}

void CWizUpdater::checkAndDownloadUpgrade()
{
    emit checkUpdate();
}

void CWizUpdater::run()
{
    QTimer timer(this);
    timer.start(WIZ_CHECK_UPDATE_TIMEOUT);
    connect(&timer, SIGNAL(timeout()), SLOT(on_request_checkUpdate()));
    connect(this, SIGNAL(checkUpdate()), SLOT(on_request_checkUpdate()));

    exec();
}

void CWizUpdater::on_request_checkUpdate()
{
    if (!m_bIsStarted) {
        TOLOG("Check update online");
        m_bIsStarted = true;
        m_nProcessTimes = 0;

        requestUpgrade();
    }
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
        TOLOG1("Network error, error number: %1", QString::number(reply->error()));
        emit upgradeError(NetworkError);
        m_bIsStarted = false;
        return;
    }


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
        m_bIsStarted = false;
        return;
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
        m_bIsStarted = false;
        return;
    }

    // download upgrade tarball
    QFile fileReply(::WizGetUpgradePath() + "update.zip");
    fileReply.remove();

    fileReply.open(QIODevice::Append);
    fileReply.write(reply->readAll());
    fileReply.close();

    reply->deleteLater();

    TOLOG("Download update tarball finished");

    processTarball();
}

void CWizUpdater::processTarball()
{
    QString strZip = ::WizGetUpgradePath() + "update.zip";
    if (!CWizUnzipFile::extractZip(strZip, ::WizGetUpgradePath())) {
        TOLOG("tarball data is not correct!");
        emit upgradeError(UnzipError);
        m_bIsStarted = false;
        return;
    }

    // read metadata to memory
    CWizSettings* config = new CWizSettings(::WizGetUpgradePath() + "config.txt");
    m_strDownloadFileUrl = config->GetString("Common", "DownloadFileURL");
    m_strWhatsNewUrl = config->GetString("Common", "WhatsNewURL");

    int i = 0;
    QString strFileEntry = config->GetString("Files", QString::number(i));
    while (!strFileEntry.isEmpty()) {
        QStringList strFileMeta = strFileEntry.split("*");

        if (strFileMeta.count() != 2) {
            emit upgradeError(ParseError);
            m_bIsStarted = false;
            return;
        }

        QStringList strFile;
        strFile << strFileMeta.at(0) << strFileMeta.at(1);
        m_files.append(strFile);

        i++;
        strFileEntry = config->GetString("Files", QString::number(i));
    }

    generateDownloadQueue();
}

void CWizUpdater::generateDownloadQueue()
{
    m_downloadQueue.clear();

    QList<QStringList>::const_iterator i;
    for (i = m_files.constBegin(); i != m_files.constEnd(); i++) {
        QString strDownloadFullPath = ::WizGetUpgradePath() + (*i).at(0);
        QString strLocalFullPath = ::WizGetAppPath() + (*i).at(0);
        QString strDownloadFileName = ::WizExtractFileName(strDownloadFullPath);

        // compare md5 to determine need download or not
        QFile fileLocal(strLocalFullPath);
        QFile fileUpdate(strDownloadFullPath);

        QString md5Remote = (*i).at(1);

        // compare remote and local
        if (!fileLocal.exists()) {
            if (!fileUpdate.exists()) {
                m_downloadQueue.append(*i);
                continue;
            }
        } else {
            QString md5Local = ::WizMd5FileString(strLocalFullPath);
            if (md5Remote == md5Local) {
                continue;
            } else {
                m_downloadQueue.append(*i);
                continue;
            }
        }

        // compare remote and update
        if (fileUpdate.exists()) {
            QString md5Download = ::WizMd5FileString(strDownloadFullPath);
            if (md5Download == md5Remote) {
                continue;
            } else {
                TOLOG1("Need download again: %1", strDownloadFileName);
                fileUpdate.remove();
                m_downloadQueue.append(*i);
                continue;
            }
        }
    }

    m_nNeedProcess = m_downloadQueue.count();
    processDownload();
}

void CWizUpdater::processDownload()
{
    if (m_nProcessTimes >= m_nNeedProcess) {
        if (!m_downloadQueue.isEmpty()) {
            TOLOG("Current download still meet error, scheduled 3 hours later");
        }

        m_bIsStarted = false;
        return;
    }

    m_nProcessTimes++;

    QString strCurrent = m_downloadQueue.at(0).at(0);
    QUrl fileUrl = m_strDownloadFileUrl.arg(strCurrent).arg(WizGetTimeStamp());
    QNetworkRequest request(fileUrl);
    QNetworkReply* reply = m_net->get(request);
    connect(reply, SIGNAL(finished()), SLOT(on_downloadFile_finished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), \
            SLOT(on_downloadFile_error(QNetworkReply::NetworkError)));
}

void CWizUpdater::on_downloadFile_error(QNetworkReply::NetworkError error)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());

    QString strFile = ::WizExtractFileName(m_downloadQueue.at(0).at(0));
    TOLOG3("Download %1 error [code %2]: %3", strFile, QString::number(error), reply->errorString());

    reply->deleteLater();

    // move current file to the end of queue, waiting for next chance.
    if (!m_downloadQueue.isEmpty()) {
        m_downloadQueue.swap(0, m_downloadQueue.count() - 1);
    }

    processDownload();
}

void CWizUpdater::on_downloadFile_finished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());

    // while meet error, we should try again.
    if (reply->error()) {
        return;
    }

    QString strFile = ::WizGetUpgradePath() + m_downloadQueue.at(0).at(0);
    QString strDownloadFilePath = WizExtractFilePath(strFile);
    QString strDownloadFileName = WizExtractFileName(strFile);

    // save download
    QString strWorkingPath = ::WizGetUpgradePath();
    QFile fileReply(strWorkingPath + strDownloadFileName + ".zip");
    fileReply.open(QIODevice::Append);
    fileReply.write(reply->readAll());
    fileReply.close();

    reply->deleteLater();

    TOLOG1("Downloaded file: %1", strDownloadFileName);

    // unzip
    WizEnsurePathExists(strDownloadFilePath);
    QFile fileZip(strWorkingPath + strDownloadFileName + ".zip");
    if (!CWizUnzipFile::extractZip(strWorkingPath + strDownloadFileName + ".zip", \
                                   strDownloadFilePath)) {
        fileZip.remove();
        TOLOG1("process failed: %1", strDownloadFileName);
        return;
    }
    fileZip.remove();

    // rename and move
    QFile file(strDownloadFilePath + "data");
    if (!file.exists()) {
        TOLOG1("process failed: %1", strDownloadFileName);
        return;
    }
    file.rename(strDownloadFilePath + strDownloadFileName);

    // compare md5
    QString md5Remote = m_downloadQueue.at(0).at(1);
    QString md5Download = ::WizMd5FileString(strFile);
    if (md5Download == md5Remote) {
        // download succeed, remove from queue
        m_downloadQueue.removeFirst();
    }

    processDownload();
}
