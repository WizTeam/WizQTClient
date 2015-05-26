#ifndef WIZSERVICE_AVATAR_H
#define WIZSERVICE_AVATAR_H

#include <QObject>

class QString;
class QPixmap;

namespace WizService {
namespace Internal {
class AvatarHostPrivate;
}

class AvatarHost: public QObject
{
    Q_OBJECT

public:
    explicit AvatarHost();
    ~AvatarHost();

    static AvatarHost* instance();
    static void load(const QString& strUserID);
    static void reload(const QString& strUserID);
    static bool isLoaded(const QString& strUserID);
    static bool isFileExists(const QString& strUserID);
    static bool avatar(const QString& strUserID, QPixmap* pixmap);
    static bool deleteAvatar(const QString& strUserID);
    static QPixmap orgAvatar(const QString& strUserID);
    static QString keyFromUserID(const QString& strUserID);
    static QString defaultKey();
    static bool customSizeAvatar(const QString& strUserID, int width, int height, QString& strFileName);

Q_SIGNALS:
    void loaded(const QString& strUserID);

    friend class Internal::AvatarHostPrivate;

public:

    static void waitForDone();

    static QPixmap corpImage(const QPixmap& org);
    static QPixmap circleImage(const QPixmap& org, int width, int height);
};

} // namespace WizService

#endif // WIZSERVICE_AVATAR_H
