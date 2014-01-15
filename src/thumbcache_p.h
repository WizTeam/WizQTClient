#ifndef THUMBCACHE_P_H
#define THUMBCACHE_P_H

#include <QObject>
#include <QMap>


struct WIZABSTRACT;
struct WIZDOCUMENTDATA;

namespace Core {
class ThumbCache;

namespace Internal{


class ThumbCachePrivate : public QObject
{
    Q_OBJECT

public:
    ThumbCachePrivate(ThumbCache* cache);
    bool find(const QString& strKbGUID, const QString& strGUID, WIZABSTRACT& abs);

private:
    QString key(const QString& strKbGUID, const QString& strGUID);
    void load(const QString& strKbGUID, const QString& strGUID);
    void load_impl(const QString& strKbGUID, const QString& strGUID);

protected Q_SLOTS:
    void onNoteThumbChanged(const WIZDOCUMENTDATA& data);

Q_SIGNALS:
    void thumbLoaded(const QString& strKbGUID, const QString& strGUID);

private:
    QMap<QString, WIZABSTRACT> m_mapThumb;
    ThumbCache* q;
};


} // namespace Internal
} // namespace Core

#endif // THUMBCACHE_P_H
