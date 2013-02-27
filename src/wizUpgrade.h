#ifndef WIZUPGRADE_H
#define WIZUPGRADE_H

#include <QtNetwork>



class CWizUpgrade : public QObject
{
    Q_OBJECT

public:
    explicit CWizUpgrade(QObject *parent = 0);
    void beginCheck();

    QString getWhatsNewUrl();

private:
    QPointer<QNetworkAccessManager> m_net;
    QTimer m_timerCheck;
    QUrl m_redirectedUrl;

    void _check(const QString& strUrl);
    QUrl redirectUrl(QUrl const &possible_redirect_url,
                     QUrl const &old_redirect_url) const;

public Q_SLOTS:
    void check();
    void on_checkUpgrade_finished();
    void on_timerCheck_timeout();

Q_SIGNALS:
    void checkFinished(bool bUpgradeAvaliable);
};

#endif // WIZUPGRADE_H
