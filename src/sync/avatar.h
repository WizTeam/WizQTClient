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
    static void load(const QString& strUserGUID, bool bForce = false);
    static bool isLoaded(const QString& strUserId);
    static bool avatar(const QString& strUserId, QPixmap* pixmap);
    static QPixmap orgAvatar(const QString& strUserId);
    static QString keyFromGuid(const QString& strUserGUID);
    static QString defaultKey();

Q_SIGNALS:
    void loaded(const QString& strUserGUID);

    friend class Internal::AvatarHostPrivate;
};

} // namespace WizService

#endif // WIZSERVICE_AVATAR_H
