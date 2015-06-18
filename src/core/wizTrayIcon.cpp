#include "wizTrayIcon.h"
#include <QApplication>
#include <QWidget>
#include <QDebug>
#include "share/wizSyncableDatabase.h"
#include "wizdef.h"

CWizTrayIcon::CWizTrayIcon(CWizExplorerApp& app, QObject* parent)
    : QSystemTrayIcon(parent)
    , m_app(app)
{
    connect(this, SIGNAL(messageClicked()), SLOT(onMessageClicked()));
}

CWizTrayIcon::CWizTrayIcon(CWizExplorerApp& app, const QIcon& icon, QObject* parent)
    : QSystemTrayIcon(icon, parent)
    , m_app(app)
{
    connect(this, SIGNAL(messageClicked()), SLOT(onMessageClicked()));
}

CWizTrayIcon::~CWizTrayIcon()
{
}

void CWizTrayIcon::showMessage(const QString& title, const QString& msg, QSystemTrayIcon::MessageIcon icon, int msecs)
{
    m_messageType = wizBubbleNormal;
    m_messageData.clear();
    QSystemTrayIcon::showMessage(title, msg, icon, msecs);
}

void CWizTrayIcon::showMessage(const QVariant& param)
{
    QList<QVariant> paramList = param.toList();
    if (paramList.count() < 2)
        return;

    m_messageType = wizBubbleNoMessage;
    m_messageData.clear();
    //
    int type = paramList.first().toInt();
    if (type == wizBubbleMessageCenter)
    {
        Q_ASSERT(paramList.count() == 4);
        m_messageType = wizBubbleMessageCenter;
        m_messageData = paramList.at(1);
        QSystemTrayIcon::showMessage(paramList.at(2).toString(), paramList.at(3).toString(), QSystemTrayIcon::Information);
    }
}

void CWizTrayIcon::onMessageClicked()
{
    m_app.mainWindow()->raise();
    if (m_messageType == wizBubbleMessageCenter)
    {
        qint64 id = m_messageData.toLongLong();
        emit viewMessageRequest(id);
    }
}


