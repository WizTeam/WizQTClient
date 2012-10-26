#include "wizupdater.h"

#include "share/wizmisc.h"

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
    prepareLocalDisk();
    requestUpgrade();
    exec();
}

void CWizUpdater::prepareLocalDisk()
{
    ::WizDeleteAllFilesInFolder(::WizGetUpgradePath());
}

void CWizUpdater::requestUpgrade()
{
    //connect(m_net, SIGNAL(finished(QNetworkReply*)), \
    //        SLOT(on_finished_requestUpgrade(QNetworkReply*)), \
    //        Qt::QueuedConnection);

    // send request of download upgrade tarball
    QString strURL = WIZ_UPGRADE_URL + "?p=%1&c=%2&plat=%3";
    strURL = strURL.arg("wiz").arg("updatev2").arg("x86");

    QUrl url(strURL);
    QNetworkRequest request(url);
    m_strRequestFileName = "update.zip";
    m_curReply = m_net->get(request);
    int ret = connect(m_curReply, SIGNAL(readyRead()), SLOT(on_request_readyRead()), Qt::QueuedConnection);
    int r = 0;
}

void CWizUpdater::on_finished_requestUpgrade(QNetworkReply* reply)
{
    //disconnect(m_net, SIGNAL(finished(QNetworkReply*)), \
    //           this, SLOT(on_finished_requestUpgrade(QNetworkReply*)));

}

void CWizUpdater::on_request_readyRead()
{
    // download upgrade tarball
    QFile fileReply(::WizGetUpgradePath() + m_strRequestFileName);
    fileReply.open(QIODevice::Append);
    fileReply.write(m_curReply->readAll());
    fileReply.close();
}
