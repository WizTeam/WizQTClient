#include "WizTrayIcon.h"
#include <QApplication>
#include <QWidget>
#include <QDebug>
#include "share/WizSyncableDatabase.h"
#include "WizDef.h"

WizTrayIcon::WizTrayIcon(WizExplorerApp& app, QObject* parent)
    : QSystemTrayIcon(parent)
    , m_app(app)
{
    connect(this, SIGNAL(messageClicked()), SLOT(onMessageClicked()));
}

WizTrayIcon::WizTrayIcon(WizExplorerApp& app, const QIcon& icon, QObject* parent)
    : QSystemTrayIcon(icon, parent)
    , m_app(app)
{
    connect(this, SIGNAL(messageClicked()), SLOT(onMessageClicked()));
}

WizTrayIcon::~WizTrayIcon()
{
}

void WizTrayIcon::showMessage(const QString& title, const QString& msg, QSystemTrayIcon::MessageIcon icon, int msecs, const QVariant& param)
{
    m_messageType = wizBubbleNormal;
    m_messageData = param;
    QSystemTrayIcon::showMessage(title, msg, icon, msecs);
}

void WizTrayIcon::showMessage(const QVariant& param)
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

void WizTrayIcon::onMessageClicked()
{
    m_app.mainWindow()->raise();
    if (m_messageType == wizBubbleMessageCenter)
    {
        qint64 id = m_messageData.toLongLong();
        emit viewMessageRequest(id);
    }
    else if (m_messageType == wizBubbleNormal)
    {
        emit viewMessageRequestNormal(m_messageData);
    }
}


