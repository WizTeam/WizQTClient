#include "wizUpgrade.h"

#include "share/wizmisc.h"
#include "utils/logger.h"

#define strWhatsNewUrl "http://blog.wiz.cn/wiznotechangelog"
#define strUpgradeHostUrl "http://download.wiz.cn"

#if defined(Q_OS_MAC)
#define strUpgradeUrl "http://download.wiz.cn/download?product=wiznote&client=macos"
#elif defined(Q_OS_LINUX)

#if defined(_M_X64) || defined(__amd64)
#define strUpgradeUrl "http://download.wiz.cn/download?product=wiznote&client=linux-x64"
#else
#define strUpgradeUrl "http://download.wiz.cn/download?product=wiznote&client=linux-x86"
#endif // __amd64

#endif // Q_OS_MAC

CWizUpgrade::CWizUpgrade(QObject *parent) :
    QObject(parent)
{
    connect(&m_timerCheck, SIGNAL(timeout()), SLOT(on_timerCheck_timeout()));
    m_timerCheck.start(60 * 1000);
}

void CWizUpgrade::on_timerCheck_timeout()
{
    beginCheck();
}

void CWizUpgrade::beginCheck()
{
    if (!QMetaObject::invokeMethod(this, "check")) {
        TOLOG("Invoke check of upgrade failed");
    }

    m_timerCheck.stop();
}

QString CWizUpgrade::getWhatsNewUrl()
{
    return strWhatsNewUrl;
}

void CWizUpgrade::check()
{
    _check(strUpgradeUrl);
}

void CWizUpgrade::_check(const QString& strUrl)
{
    if (!m_net) {
        m_net = new QNetworkAccessManager();
    }

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
            TOLOG("INFO: Upgrade is avaliable, version time:" + dateUpgrade.toString());
            Q_EMIT checkFinished(true);
        } else {
            TOLOG("INFO: Local version is up to date");
            Q_EMIT checkFinished(false);
        }
    } else {
        TOLOG("ERROR: Check upgrade failed");
        Q_EMIT checkFinished(false);
    }

    reply->deleteLater();
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
