#ifndef WIZSERVICE_AVATARUPLOADER_H
#define WIZSERVICE_AVATARUPLOADER_H

#include <QObject>

class QString;
class QNetworkAccessManager;
class QNetworkReply;


class WizAvatarUploader : public QObject
{
    Q_OBJECT

public:
    explicit WizAvatarUploader(QObject *parent);
    void upload(const QString& strFileName);
    QString lastErrorMessage() { return m_strError; }

private:
    QNetworkAccessManager* m_net;
    QString m_strFileName;
    QString m_strUrl;
    QString m_strError;

    QString convert2Avatar(const QString& strFileName);
    void upload_impl(const QString& strUrl, const QString& strToken, const QString& strFileName);

Q_SIGNALS:
    void uploaded(bool ok);
};


#endif // WIZSERVICE_AVATARUPLOADER_H
