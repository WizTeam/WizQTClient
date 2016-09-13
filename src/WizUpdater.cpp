#include "WizUpdater.h"

#ifdef WIZ_OBOSOLETE

#include "WizDef.h"
#include "share/WizMisc.h"
#include "share/WizSettings.h"
#include "zip/WizZip.h"

const QString WIZ_UPGRADE_URL = "http://api.wiz.cn/";

WizUpgradeThread::WizUpgradeThread(QObject* parent /* = 0 */)
    : QThread(parent)
    , m_bIsStarted(false)
    , m_currentThread(0)
{
    m_timer.setInterval(3 * 60 * 60 * 1000); // 3 hours
    connect(&m_timer, SIGNAL(timeout()), SLOT(checkUpgradeBegin()));

    // clean up event when thread finished
    connect(this, SIGNAL(finished()), SLOT(checkUpgradeFinished()));

    // remove stub file
    QFile file(::WizGetUpgradePath() + "WIZNOTE_SKIP_THIS_SESSION");
    if (file.exists())
        file.remove();

#ifndef Q_OS_MAC
    // FIXME: disable upgrade check, only start timer on non-mac system
    m_timer.start();
#endif // Q_OS_MAC
}

void WizUpgradeThread::abort()
{
    if (m_currentThread)
        m_upgradePtr->abort();
}

void WizUpgradeThread::checkUpgradeBegin()
{
    if (m_bIsStarted)
        return;

    m_timer.stop();
    m_bIsStarted = true;

    start();
}

void WizUpgradeThread::checkUpgradeFinished()
{
    m_upgradePtr->deleteLater();
    m_currentThread = 0;

    // if user choose not check upgrade for this session
    QFile file(::WizGetUpgradePath() + "WIZNOTE_SKIP_THIS_SESSION");
    if (!file.exists()) {
        m_timer.start();
    }

    m_bIsStarted = false;
}

void WizUpgradeThread::run()
{
    m_currentThread = QThread::currentThread();

    TOLOG(tr("Check update online"));

    m_upgradePtr = new WizUpgrade();

    qRegisterMetaType<UpdateError>("UpdateError");

    connect(m_upgradePtr, SIGNAL(prepareDone(bool)), SLOT(on_prepareDone(bool)));
    connect(m_upgradePtr, SIGNAL(upgradeError(UpdateError)), SLOT(on_upgradeError(UpdateError)));

    m_upgradePtr->requestUpgrade();

    exec();
}

void WizUpgradeThread::on_prepareDone(bool bNeedUpgrade)
{
    TOLOG1(tr("Check upgrade done, need upgrade: %1"), QString::number(bNeedUpgrade));

    // save url before thread exit
    if (bNeedUpgrade) {
        m_changelogUrl = m_upgradePtr->whatsNewUrl();
    } else {
        QFile fileConfig(::WizGetUpgradePath() + "config.txt");
        fileConfig.remove();
        QFile fileZip(::WizGetUpgradePath() + "update.zip");
        fileZip.remove();
    }

    m_currentThread->exit();
}

void WizUpgradeThread::on_upgradeError(UpdateError error)
{
    Q_UNUSED(error);

    TOLOG1(tr("Check upgrade meet error, error code: %1"), QString::number(error));
    m_currentThread->exit();
}




WizUpgrade::WizUpgrade(QObject* parent /* = 0 */)
    : QObject(parent)
    , m_nProcessTimes(0)
    , m_bNewVersion(false)
{
}

void WizUpgrade::abort()
{
    TOLOG(tr("cancle upgrade check"));

    m_net.disconnect();
    Q_EMIT prepareDone(false);
}

QString WizUpgrade::getUpgradeUrl()
{
    QString strProduct = "wiznote_qt";
    QString strCommand = "updatev2";

#ifdef _M_X64
    QString strPlatform = "x64";
#else
    QString strPlatform = "x86";
#endif

    QString strUrl = WIZ_UPGRADE_URL \
            + "?p=%1&l=%2&v=%3&c=%4&random=%5&cn=%6&plat=%7";
    strUrl = strUrl.arg(strProduct)\
            .arg(QLocale::system().name())\
            .arg(WIZ_CLIENT_VERSION)\
            .arg(strCommand)\
            .arg(QString::number(::GetTickCount()))\
            .arg(::WizGetComputerName())\
            .arg(strPlatform);

    //TOLOG("URL:" + strUrl);

    return strUrl;
}

QUrl WizUpgrade::redirectUrl(QUrl const &possible_redirect_url, \
                              QUrl const &old_redirect_url) const
{
    QUrl redirect_url;

    if(!possible_redirect_url.isEmpty() \
            && possible_redirect_url != old_redirect_url)
    {
            redirect_url = possible_redirect_url;
    }

    return redirect_url;
}

void WizUpgrade::requestUpgrade()
{
    // send request to download upgrade tarball
    requestUpgrade_impl(getUpgradeUrl());
}

void WizUpgrade::requestUpgrade_impl(QString const& url)
{
    QNetworkReply* reply = m_net.get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), SLOT(on_requestUpgrade_finished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), \
            SLOT(on_request_error(QNetworkReply::NetworkError)));
}

void WizUpgrade::on_requestUpgrade_finished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());

    if (reply->error()) {
        Q_EMIT upgradeError(NetworkError);
        reply->deleteLater();
        return;
    }

    QUrl possibleRedirectedUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    m_redirectedUrl = redirectUrl(possibleRedirectedUrl, m_redirectedUrl);

    if (!m_redirectedUrl.isEmpty()) {
        // redirect to download server.
        //TOLOG1("redirected: %1", m_redirectedUrl.toString());
        requestUpgrade_impl(m_redirectedUrl.toString());
    } else {
        // download upgrade tarball
        QFile fileReply(::WizGetUpgradePath() + "update.zip");
        fileReply.remove();

        fileReply.open(QIODevice::Truncate | QIODevice::WriteOnly);
        fileReply.write(reply->readAll());
        fileReply.close();

        TOLOG(tr("Download update tarball finished"));
        processTarball();
    }

    reply->deleteLater();
}

void WizUpgrade::processTarball()
{
    // unzip
    QString strZip = ::WizGetUpgradePath() + "update.zip";
    if (!WizUnzipFile::extractZip(strZip, ::WizGetUpgradePath())) {
        TOLOG("tarball data is not correct! please contect wiz support team");
        Q_EMIT upgradeError(UnzipError);
        return;
    }

    readMetadata();

    generateDownloadQueue();
}

void WizUpgrade::readMetadata()
{
    // read metadata to memory
    WizSettings* config = new WizSettings(::WizGetUpgradePath() + "config.txt");
    m_strDownloadFileUrl = config->getString("Common", "DownloadFileURL");
    m_strWhatsNewUrl = config->getString("Common", "WhatsNewURL");

    int i = 0;
    QString strFileEntry = config->getString("Files", QString::number(i));
    while (!strFileEntry.isEmpty()) {
        QStringList strFileMeta = strFileEntry.split("*");

        if (strFileMeta.count() != 2) {
            TOLOG("config file format is wrong, please contect wiz support team");
            Q_EMIT upgradeError(ParseError);
            return;
        }

        // debug: remove C# file
        if (strFileMeta.at(1) != "d0098473d075f0fa1973210a5ac6fe50") {
            QStringList strFile;
            strFile << strFileMeta.at(0) << strFileMeta.at(1);
            m_files.append(strFile);
        }

        i++;
        strFileEntry = config->getString("Files", QString::number(i));
    }
}

void WizUpgrade::generateDownloadQueue()
{
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
            m_bNewVersion = true;
            if (!fileUpdate.exists()) {
                m_downloadQueue.append(*i);
                continue;
            }
        } else {
            QString md5Local = ::WizMd5FileString(strLocalFullPath);
            if (md5Remote == md5Local) {
                continue;
            } else {
                m_bNewVersion = true;
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

    //if (!m_nNeedProcess) {
    //    Q_EMIT prepareDone(false);
    //    return;
    //}

    processDownload();
}

void WizUpgrade::processDownload()
{
    if (m_nProcessTimes >= m_nNeedProcess) {
        if (!m_downloadQueue.isEmpty()) {
            TOLOG(tr("Current download still meet error, scheduled 3 hours later"));
            Q_EMIT prepareDone(false);
        } else {
            if (m_bNewVersion) {
                Q_EMIT prepareDone(true);
            } else {
                Q_EMIT prepareDone(false);
            }
        }

        // exist thread from here.
        return;
    }

    m_nProcessTimes++;

    QString strCurrent = m_downloadQueue.at(0).at(0);
    QUrl fileUrl = m_strDownloadFileUrl.arg(strCurrent).arg(WizGetTimeStamp());
    QNetworkRequest request(fileUrl);
    QNetworkReply* reply = m_net.get(request);
    connect(reply, SIGNAL(finished()), SLOT(on_downloadFile_finished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), \
            SLOT(on_request_error(QNetworkReply::NetworkError)));
}

void WizUpgrade::on_downloadFile_finished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());

    // while meet error, we should try again.
    if (reply->error()) {
        moveToEnd();
        processDownload();
        return;
    }

    QString strFile = ::WizGetUpgradePath() + m_downloadQueue.at(0).at(0);
    QString strDownloadFilePath = WizExtractFilePath(strFile);
    QString strDownloadFileName = WizExtractFileName(strFile);

    // save download
    QString strWorkingPath = ::WizGetUpgradePath();
    QFile fileReply(strWorkingPath + strDownloadFileName + ".zip");
    fileReply.open(QIODevice::Truncate | QIODevice::WriteOnly);
    fileReply.write(reply->readAll());
    fileReply.close();

    reply->deleteLater();

    // unzip
    WizEnsurePathExists(strDownloadFilePath);
    QFile fileZip(strWorkingPath + strDownloadFileName + ".zip");
    if (!WizUnzipFile::extractZip(strWorkingPath + strDownloadFileName + ".zip", \
                                   strDownloadFilePath)) {
        fileZip.remove();
        TOLOG1("upzip failed: %1", strDownloadFileName);
        moveToEnd();
        processDownload();
        return;
    }
    fileZip.remove();

    // rename and move
    QFile file(strDownloadFilePath + "data");
    if (!file.exists()) {
        TOLOG1("process failed: %1", strDownloadFileName);
        moveToEnd();
        processDownload();
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

void WizUpgrade::on_request_error(QNetworkReply::NetworkError error)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());

    // error meet when request tarball
    if (m_downloadQueue.isEmpty()) {
        TOLOG2("Request upgrade error [code %1]: %2", QString::number(error), reply->errorString());
        reply->deleteLater();
        return;
    }

    reply->deleteLater();

    // else, occured when download files
    QString strFile = ::WizExtractFileName(m_downloadQueue.at(0).at(0));
    TOLOG3("Download %1 error [code %2]: %3", strFile, QString::number(error), reply->errorString());
}

void WizUpgrade::moveToEnd()
{
    if (!m_downloadQueue.isEmpty()) {
        m_downloadQueue.swap(0, m_downloadQueue.count() - 1);
    }
}

#endif // WIZ_OBOSOLETE
