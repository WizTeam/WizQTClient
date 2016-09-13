#ifndef WIZLOCALPEER_H
#define WIZLOCALPEER_H

#include "WizLockedFile.h"
#include <QLocalServer>
#include <QLocalSocket>
#include <QDir>
#include <QObject>
#include <QString>

class WizLockedFile;

class WizLocalPeer : public QObject
{
    Q_OBJECT

public:
    explicit WizLocalPeer(QObject *parent = 0, const QString &appId = QString());
    bool isClient();
    bool sendMessage(const QString &message, int timeout, bool block);
    QString applicationId() const
        { return id; }
    static QString appSessionId(const QString &appId);

Q_SIGNALS:
    void messageReceived(const QString &message, QObject *socket);

protected Q_SLOTS:
    void receiveConnection();

protected:
    QString id;
    QString socketName;
    QLocalServer* server;
    WizLockedFile lockFile;

};

#endif // WIZLOCALPEER_H
