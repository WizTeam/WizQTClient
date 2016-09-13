#ifndef WIZSINGLEAPPLICATION_H
#define WIZSINGLEAPPLICATION_H

#include <QApplication>
#include <QString>

/*
class QWidget;
class CWizLocalPeer;
class QSharedMemory;

class CWizSingleApplication : public QApplication
{
    Q_OBJECT
public:
    CWizSingleApplication();

    CWizSingleApplication(const QString &id, int &argc, char **argv);
    ~CWizSingleApplication();

    bool isRunning(qint64 pid = -1);

    void setActivationWindow(QWidget* aw, bool activateOnMessage = true);
    QWidget* activationWindow() const;
    bool event(QEvent *event);

    QString applicationId() const;
    void setBlock(bool value);

public Q_SLOTS:
    bool sendMessage(const QString &message, int timeout = 5000, qint64 pid = -1);
    void activateWindow();

Q_SIGNALS:
    void messageReceived(const QString &message, QObject *socket);
    void fileOpenRequest(const QString &file);

private:
    QString instancesFileName(const QString &appId);

    qint64 firstPeer;
    QSharedMemory *instances;
    CWizLocalPeer *pidPeer;
    QWidget *actWin;
    QString appId;
    bool block;
};
*/

#include <QApplication>
#include <QSharedMemory>
#include <QLocalServer>

class WizSingleApplication : public QApplication
{
    Q_OBJECT
public:
    WizSingleApplication(int &argc, char *argv[], const QString uniqueKey);

    bool isRunning();
    bool sendMessage(const QString &message);

public slots:
    void receiveMessage();

signals:
    void messageAvailable(QString message);

private:
    bool _isRunning;
    QString _uniqueKey;
    QSharedMemory sharedMemory;
    QLocalServer *localServer;

    static const int timeout = 1000;
};

#endif // WIZSINGLEAPPLICATION_H
