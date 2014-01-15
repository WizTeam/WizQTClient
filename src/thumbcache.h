#ifndef CORE_THUMBCACHE_H
#define CORE_THUMBCACHE_H

#include <QObject>

struct WIZABSTRACT;

namespace Core {
namespace Internal{
class ThumbCachePrivate;
}

class ThumbCache : public QObject
{
    Q_OBJECT

public:
    ThumbCache();
    ~ThumbCache();

    static ThumbCache* instance();
    static bool find(const QString& strKbGUID, const QString& strGUID, WIZABSTRACT& abs);

Q_SIGNALS:
    void loaded(const QString& strKbGUID, const QString& strGUID);

    friend class Internal::ThumbCachePrivate;
};

} // namespace Core

#endif // CORE_THUMBCACHE_H
