#ifndef WIZAVATARUPLOADER_H
#define WIZAVATARUPLOADER_H

#include <QThread>
#include <QPointer>

class QNetworkReply;
class QNetworkAccessManager;
class CWizExplorerApp;
class CWizDatabase;

class CWizAvatarUploader : public QObject
{
    Q_OBJECT

public:
    explicit CWizAvatarUploader(CWizExplorerApp& app, QObject *parent = 0);
    void upload(const QString& strFileName);
    QString GetErrorString();

private:
    CWizDatabase& m_db;
    QPointer<QNetworkAccessManager> m_net;
    QString m_strFileName;
    QString m_error;

    QString convert2Avatar(const QString& strFileName);
    void upload_impl(const QString& strUrl, const QString& strFileName);

private Q_SLOTS:
    void on_acquireEntry_finished(const QString& strUrl);
    void on_upload_finished(QNetworkReply* reply);

Q_SIGNALS:
    void uploaded(bool ok);
};


#endif // WIZAVATARUPLOADER_H
