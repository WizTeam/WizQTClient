#ifndef WIZUPGRADE_H
#define WIZUPGRADE_H

#include <QtNetwork>
#include <QThread>


class CWizUpgrade : public QThread
{
    Q_OBJECT

public:
    explicit CWizUpgrade(QObject *parent = 0);
    ~CWizUpgrade();
    void startCheck();

    QString getWhatsNewUrl();

public Q_SLOTS:
    void checkUpgrade();
    void on_getCheckUrl_finished();
    void on_checkUpgrade_finished();
    void on_timerCheck_timeout();

Q_SIGNALS:
    void checkFinished(bool bUpgradeAvaliable);

protected:
    void run();

private:
    QPointer<QNetworkAccessManager> m_net;
    QTimer m_timerCheck;
    QUrl m_redirectedUrl;

    void _check(const QString& strUrl);
    QUrl redirectUrl(QUrl const &possible_redirect_url,
                     QUrl const &old_redirect_url) const;
    void beginCheck();
};

#endif // WIZUPGRADE_H
