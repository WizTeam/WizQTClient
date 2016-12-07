#ifndef WIZSERVICE_AVATAR_H
#define WIZSERVICE_AVATAR_H

#include <QObject>

class QString;
class QPixmap;

class WizAvatarHostPrivate;

#define SYSTEM_AVATAR_APPLY_GROUP       "message_icons_apply_group"
#define SYSTEM_AVATAR_ADMIN_PERMIT      "message_icons_admin_permit"
#define SYSTEM_AVATAR_SYSTEM            "message_icons_system"

class WizAvatarHost: public QObject
{
    Q_OBJECT

public:
    explicit WizAvatarHost();
    ~WizAvatarHost();

    static WizAvatarHost* instance();
    static void load(const QString& strUserID, bool isSystem);
    static void reload(const QString& strUserID);
    static bool isLoaded(const QString& strUserID);
    static bool isFileExists(const QString& strUserID);
    static bool avatar(const QString& strUserID, QPixmap* pixmap);
    static bool systemAvatar(const QString& avatarName, QPixmap* pixmap);
    static bool deleteAvatar(const QString& strUserID);
    static QPixmap orgAvatar(const QString& strUserID);
    static QString keyFromUserID(const QString& strUserID);
    static QString defaultKey();
    static bool customSizeAvatar(const QString& strUserID, int width, int height, QString& strFileName);

Q_SIGNALS:
    void loaded(const QString& strUserID);

    friend class WizAvatarHostPrivate;

public:
    static QPixmap corpImage(const QPixmap& org);
    static QPixmap circleImage(const QPixmap& org, int width, int height);
};


#endif // WIZSERVICE_AVATAR_H
