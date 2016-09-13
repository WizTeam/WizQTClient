#ifndef WIZUPGRADE_H
#define WIZUPGRADE_H

#include <QtNetwork>
#include <memory>


class WizUpgradeChecker : public QObject
{
    Q_OBJECT

public:
    explicit WizUpgradeChecker(QObject *parent = 0);
    ~WizUpgradeChecker();
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
