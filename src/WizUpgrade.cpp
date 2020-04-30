#include "WizUpgrade.h"

#include "share/WizMisc.h"
#include "utils/WizLogger.h"
#include "sync/WizApiEntry.h"
#include "share/WizEventLoop.h"

#if defined(Q_OS_MAC)
    #define strUpgradeUrlParam "/download?product=wiznote&client=macos"
#elif defined(Q_OS_LINUX)

    #if defined(_M_X64) || defined(__amd64)
        #define strUpgradeUrlParam "/download?product=wiznote&client=linux-x64"
    #else
        #define strUpgradeUrlParam "/download?product=wiznote&client=linux-x86"
    #endif // __amd64
#else
    #define strUpgradeUrlParam ""
#endif // Q_OS_MAC



WizUpgradeChecker::WizUpgradeChecker(QObject *parent) :
    QObject(parent)
{
}

WizUpgradeChecker::~WizUpgradeChecker()
{
}

QString WizUpgradeChecker::getWhatsNewUrl()
{
    return WizOfficialApiEntry::standardCommandUrl("changelog");
}

void WizUpgradeChecker::checkUpgrade()
{
    QString strApiUrl = WizOfficialApiEntry::standardCommandUrl("download_server");

    if (!m_net.get()) {
        m_net = std::make_shared<QNetworkAccessManager>();
    }

    QNetworkReply* reply = m_net->get(QNetworkRequest(strApiUrl));
    WizAutoTimeOutEventLoop loop(reply);
    loop.exec();

    if (loop.error() != QNetworkReply::NoError)
    {
        Q_EMIT checkFinished(false);
        return;
    }

    QString strCheckUrl = QString::fromUtf8(loop.result().constData());
    strCheckUrl += strUpgradeUrlParam;

    _check(strCheckUrl);
}

void WizUpgradeChecker::_check(const QString& strUrl)
{
    QNetworkReply* reply = m_net->get(QNetworkRequest(strUrl));
    WizAutoTimeOutEventLoop loop(reply);
    loop.exec();

    QUrl possibleRedirectedUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    m_redirectedUrl = redirectUrl(possibleRedirectedUrl, m_redirectedUrl);

    if (!m_redirectedUrl.isEmpty()) {
        // redirect to download server.
        QString strVersion;
        QRegExp regexp("(\\d{4}-\\d{2}-\\d{2})");
        if (regexp.indexIn(m_redirectedUrl.toString()) == -1) {
            Q_EMIT checkFinished(false);
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
}

QUrl WizUpgradeChecker::redirectUrl(QUrl const &possible_redirect_url, \
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
