#ifndef CWIZNOTIFICATIONCENTER_H
#define CWIZNOTIFICATIONCENTER_H

#include <QObject>

struct WIZMESSAGEDATA;

class WizNotificationCenter : public QObject
{
    Q_OBJECT
public:
    enum NotificationType {
        Notification_None,
        Notification_System,
        Notification_Message,
        Notification_Sync,
        Notification_BizService
    };

    explicit WizNotificationCenter(QObject* parent = 0);

    void showNofification(NotificationType type, const QString& title,
                          const QString& text, const QString& userInfo);
    void showNofification(const WIZMESSAGEDATA& message);
};

#endif // CWIZNOTIFICATIONCENTER_H
