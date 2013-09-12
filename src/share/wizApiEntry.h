#ifndef WIZAPIENTRY_H
#define WIZAPIENTRY_H

#include <QtNetwork>


/*
 * used for obtain urls from entry url used by wiznote backend server
 */
class CWizApiEntry : public QObject
{
    Q_OBJECT

public:
    CWizApiEntry(QObject* parent = 0);

    void getSyncUrl();
    void getMessageUrl();
    void getAvatarUrl();
    void getAvatarUploadUrl();

    static QString getFeedbackUrl();
    static QString getAccountInfoUrl(const QString& strToken);
    static QString getGroupAttributeUrl(const QString& strToken, const QString& strKbGUID);

private:
    QNetworkAccessManager* m_net;
    QString m_strLocale;

private Q_SLOTS:
    void on_acquire_finished();

Q_SIGNALS:
    void acquireEntryFinished(const QString& strReply);
};

#endif // WIZAPIENTRY_H
