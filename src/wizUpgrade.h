#ifndef WIZUPGRADE_H
#define WIZUPGRADE_H

#include <QtNetwork>
#include <memory>


class CWizUpgradeChecker : public QObject
{
    Q_OBJECT

public:
    explicit CWizUpgradeChecker(QObject *parent = 0);
    ~CWizUpgradeChecker();
    void startCheck();

    static QString getWhatsNewUrl();

    void checkUpgrade();

Q_SIGNALS:
    void checkFinished(bool bUpgradeAvaliable);


private:
    std::shared_ptr<QNetworkAccessManager> m_net;
    QUrl m_redirectedUrl;

    void _check(const QString& strUrl);
    QUrl redirectUrl(QUrl const &possible_redirect_url,
                     QUrl const &old_redirect_url) const;
};

#endif // WIZUPGRADE_H
