#ifndef CWIZTRAYICON_H
#define CWIZTRAYICON_H

#include <QSystemTrayIcon>
#include <QVariant>

class CWizExplorerApp;

class CWizTrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    CWizTrayIcon(CWizExplorerApp& app, QObject* parent = 0);
    CWizTrayIcon(CWizExplorerApp& app, const QIcon &icon, QObject *parent = 0);
    ~CWizTrayIcon();



public slots:
    void onMessageClicked();
    void showMessage(const QString &title, const QString &msg,
                     QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information, int msecs = 10000);
    void showMessage(const QVariant& param);

signals:
    void viewMessageRequest(qint64 messageID);


private:
    int m_messageType;
    QVariant m_messageData;
    CWizExplorerApp& m_app;
};

#endif // CWIZTRAYICON_H
