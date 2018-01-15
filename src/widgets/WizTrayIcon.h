#ifndef CWIZTRAYICON_H
#define CWIZTRAYICON_H

#include <QSystemTrayIcon>
#include <QVariant>

class WizExplorerApp;

class WizTrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    WizTrayIcon(WizExplorerApp& app, QObject* parent = 0);
    WizTrayIcon(WizExplorerApp& app, const QIcon &icon, QObject *parent = 0);
    ~WizTrayIcon();



public slots:
    void onMessageClicked();
    void showMessage(const QString &title, const QString &msg,
                     QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information, int msecs = 10000, const QVariant& param = QVariant());
    void showMessage(const QVariant& param);

signals:
    void viewMessageRequest(qint64 messageID);
    void viewMessageRequestNormal(QVariant messageData);


private:
    int m_messageType;
    QVariant m_messageData;
    WizExplorerApp& m_app;
};

#endif // CWIZTRAYICON_H
