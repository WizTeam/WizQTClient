#include "WizSingleApplication.h"

/*

#include "WizLocalPeer.h"
#include "WizLockedFile.h"

#include <QDir>
#include <QFileOpenEvent>
#include <QSharedMemory>
#include <QWidget>


static const int instancesSize = 1024;

static QString instancesLockFilename(const QString &appSessionId)
{
    const QChar slash(QLatin1Char('/'));
    QString res = QDir::tempPath();
    if (!res.endsWith(slash))
        res += slash;
    return res + appSessionId + QLatin1String("-instances");
}

CWizSingleApplication::CWizSingleApplication(const QString &appId, int &argc, char **argv)
    : QApplication(argc, argv),
      firstPeer(-1),
      pidPeer(0)
{
    this->appId = appId;

    const QString appSessionId = CWizLocalPeer::appSessionId(appId);

    // This shared memory holds a zero-terminated array of active (or crashed) instances
    instances = new QSharedMemory(appSessionId, this);
    actWin = 0;
    block = false;

    // First instance creates the shared memory, later instances attach to it
    const bool created = instances->create(instancesSize);
    if (!created) {
        if (!instances->attach()) {
            qWarning() << "Failed to initialize instances shared memory: "
                       << instances->errorString();
            delete instances;
            instances = 0;
            return;
        }
    }

    // CWizLockedFile is used to workaround QTBUG-10364
    CWizLockedFile lockfile(instancesLockFilename(appSessionId));

    lockfile.open(CWizLockedFile::ReadWrite);
    lockfile.lock(CWizLockedFile::WriteLock);
    qint64 *pids = static_cast<qint64 *>(instances->data());
    if (!created) {
        // Find the first instance that it still running
        // The whole list needs to be iterated in order to append to it
        for (; *pids; ++pids) {
            if (firstPeer == -1 && isRunning(*pids))
                firstPeer = *pids;
        }
    }
    // Add current pid to list and terminate it
    *pids++ = QCoreApplication::applicationPid();
    *pids = 0;
    pidPeer = new CWizLocalPeer(this, appId + QLatin1Char('-') +
                              QString::number(QCoreApplication::applicationPid()));
    connect(pidPeer, SIGNAL(messageReceived(QString,QObject*)), SIGNAL(messageReceived(QString,QObject*)));
    pidPeer->isClient();
    lockfile.unlock();
}

CWizSingleApplication::~CWizSingleApplication()
{
    if (!instances)
        return;
    const qint64 appPid = QCoreApplication::applicationPid();
    CWizLockedFile lockfile(instancesLockFilename(CWizLocalPeer::appSessionId(appId)));
    lockfile.open(CWizLockedFile::ReadWrite);
    lockfile.lock(CWizLockedFile::WriteLock);
    // Rewrite array, removing current pid and previously crashed ones
    qint64 *pids = static_cast<qint64 *>(instances->data());
    qint64 *newpids = pids;
    for (; *pids; ++pids) {
        if (*pids != appPid && isRunning(*pids))
            *newpids++ = *pids;
    }
    *newpids = 0;
    lockfile.unlock();
}

bool CWizSingleApplication::event(QEvent *event)
{
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *foe = static_cast<QFileOpenEvent*>(event);
        emit fileOpenRequest(foe->file());
        return true;
    }
    return QApplication::event(event);
}

bool CWizSingleApplication::isRunning(qint64 pid)
{
    if (pid == -1) {
        pid = firstPeer;
        if (pid == -1)
            return false;
    }

    CWizLocalPeer peer(this, appId + QLatin1Char('-') + QString::number(pid, 10));
    return peer.isClient();
}

bool CWizSingleApplication::sendMessage(const QString &message, int timeout, qint64 pid)
{
    if (pid == -1) {
        pid = firstPeer;
        if (pid == -1)
            return false;
    }

    CWizLocalPeer peer(this, appId + QLatin1Char('-') + QString::number(pid, 10));
    return peer.sendMessage(message, timeout, block);
}

QString CWizSingleApplication::applicationId() const
{
    return appId;
}

void CWizSingleApplication::setBlock(bool value)
{
    block = value;
}

void CWizSingleApplication::setActivationWindow(QWidget *aw, bool activateOnMessage)
{
    actWin = aw;
    if (!pidPeer)
        return;
    if (activateOnMessage)
        connect(pidPeer, SIGNAL(messageReceived(QString,QObject*)), this, SLOT(activateWindow()));
    else
        disconnect(pidPeer, SIGNAL(messageReceived(QString,QObject*)), this, SLOT(activateWindow()));
}


QWidget* CWizSingleApplication::activationWindow() const
{
    return actWin;
}


void CWizSingleApplication::activateWindow()
{
    if (actWin) {
        actWin->setWindowState(actWin->windowState() & ~Qt::WindowMinimized);
        actWin->raise();
        actWin->activateWindow();
    }
}
*/


#include <QLocalSocket>
#include <QDebug>

WizSingleApplication::WizSingleApplication(int &argc, char *argv[], const QString uniqueKey) : QApplication(argc, argv), _uniqueKey(uniqueKey)
{
    sharedMemory.setKey(_uniqueKey);
    if (sharedMemory.attach())
    {
        _isRunning = true;
    }
    else
    {
        _isRunning = false;
        // create shared memory.
        if (!sharedMemory.create(1))
        {
            qDebug("Unable to create single instance.");
            return;
        }
        // create local server and listen to incomming messages from other instances.
        localServer = new QLocalServer(this);
        connect(localServer, SIGNAL(newConnection()), this, SLOT(receiveMessage()));
        localServer->listen(_uniqueKey);
    }
}

// public slots.

void WizSingleApplication::receiveMessage()
{
    QLocalSocket *localSocket = localServer->nextPendingConnection();
    if (!localSocket->waitForReadyRead(timeout))
    {
        qDebug() << localSocket->errorString();
        return;
    }
    QByteArray byteArray = localSocket->readAll();
    QString message = QString::fromUtf8(byteArray.constData());
    emit messageAvailable(message);
    localSocket->disconnectFromServer();
}

// public functions.

bool WizSingleApplication::isRunning()
{
    return _isRunning;
}

bool WizSingleApplication::sendMessage(const QString &message)
{
    if (!_isRunning)
        return false;
    QLocalSocket localSocket(this);
    localSocket.connectToServer(_uniqueKey, QIODevice::WriteOnly);
    if (!localSocket.waitForConnected(timeout))
    {
        qDebug() <<localSocket.errorString();
        return false;
    }
    localSocket.write(message.toUtf8());
    if (!localSocket.waitForBytesWritten(timeout))
    {
        qDebug() <<localSocket.errorString();
        return false;
    }
    localSocket.disconnectFromServer();
    return true;
}
