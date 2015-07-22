#include "wizUpgrade.h"

#include "share/wizmisc.h"
#include "utils/logger.h"
#include "sync/apientry.h"

#if defined(Q_OS_MAC)
#define strUpgradeUrlParam "/download?product=wiznote&client=macos"
#elif defined(Q_OS_LINUX)

#if defined(_M_X64) || defined(__amd64)
#define strUpgradeUrlParam "/download?product=wiznote&client=linux-x64"
#else
#define strUpgradeUrlParam "/download?product=wiznote&client=linux-x86"
#endif // __amd64

#endif // Q_OS_MAC

CWizUpgrade::CWizUpgrade(QObject *parent) :
    QThread(parent)
{
    connect(&m_timerCheck, SIGNAL(timeout()), SLOT(on_timerCheck_timeout()));
}

CWizUpgrade::~CWizUpgrade()
{
    quit();
}

void CWizUpgrade::startCheck()
{
    m_timerCheck.start(10 * 1000);
    if (isRunning())
        start();
}

void CWizUpgrade::on_timerCheck_timeout()
{
    beginCheck();
}

void CWizUpgrade::run()
{
    exec();
}

void CWizUpgrade::beginCheck()
{
    if (!QMetaObject::invokeMethod(this, "checkUpgrade")) {
        TOLOG("Invoke check of upgrade failed");
    }

    m_timerCheck.stop();
}

QString CWizUpgrade::getWhatsNewUrl()
{
    return WizService::WizApiEntry::standardCommandUrl("changelog");
}

void CWizUpgrade::checkUpgrade()
{
    QString strApiUrl = WizService::WizApiEntry::standardCommandUrl("download_server");

    if (!m_net) {
        m_net = new QNetworkAccessManager(this);
    }

    QNetworkReply* reply = m_net->get(QNetworkRequest(strApiUrl));
    connect(reply, SIGNAL(finished()), SLOT(on_getCheckUrl_finished()));
}

void CWizUpgrade::on_getCheckUrl_finished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());

    if (reply->error()) {
        Q_EMIT checkFinished(false);
        reply->deleteLater();
        return;
    }

    QString strCheckUrl(reply->readAll());
    strCheckUrl += strUpgradeUrlParam;

    _check(strCheckUrl);
}

void CWizUpgrade::_check(const QString& strUrl)
{
    QNetworkReply* reply = m_net->get(QNetworkRequest(strUrl));
    connect(reply, SIGNAL(finished()), SLOT(on_checkUpgrade_finished()));
}

void CWizUpgrade::on_checkUpgrade_finished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());

    if (reply->error()) {
        Q_EMIT checkFinished(false);
        reply->deleteLater();
        return;
    }

    QUrl possibleRedirectedUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    m_redirectedUrl = redirectUrl(possibleRedirectedUrl, m_redirectedUrl);

    if (!m_redirectedUrl.isEmpty()) {
        // redirect to download server.
        QString strVersion;
        QRegExp regexp("(\\d{4}-\\d{2}-\\d{2})");
        if (regexp.indexIn(m_redirectedUrl.toString()) == -1) {
            Q_EMIT checkFinished(false);
            reply->deleteLater();
            return;
        }

        strVersion = regexp.cap(0);

        int y = strVersion.split("-").at(0).toInt();
        int m = strVersion.split("-").at(1).toInt();
        int d = strVersion.split("-").at(2).toInt();

        QDate dateUpgrade(y, m, d);

        QFileInfo fi(::WizGetAppFileName());
        QDate dateLocal = fi.created().date();

        if (dateUpgrade > dateLocal) {
            TOLOG(QObject::tr("INFO: Upgrade is avaliable, version time: %1").arg(dateUpgrade.toString()));
            Q_EMIT checkFinished(true);
        } else {
            TOLOG(QObject::tr("INFO: Local version is up to date"));
            Q_EMIT checkFinished(false);
        }
    } else {
        TOLOG(QObject::tr("ERROR: Check upgrade failed"));
        Q_EMIT checkFinished(false);
    }

    reply->deleteLater();
    quit();
}

QUrl CWizUpgrade::redirectUrl(QUrl const &possible_redirect_url, \
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
