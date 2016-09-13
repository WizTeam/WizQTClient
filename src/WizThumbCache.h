#ifndef CORE_THUMBCACHE_H
#define CORE_THUMBCACHE_H

#include <QObject>

struct WIZABSTRACT;

class WizThumbCachePrivate;

class WizThumbCache : public QObject
{
    Q_OBJECT

public:
    WizThumbCache();
    ~WizThumbCache();

    static WizThumbCache* instance();
    static bool find(const QString& strKbGUID, const QString& strGUID, WIZABSTRACT& abs);

Q_SIGNALS:
    void loaded(const QString& strKbGUID, const QString& strGUID);

    friend class WizThumbCachePrivate;
};


#endif // CORE_THUMBCACHE_H
