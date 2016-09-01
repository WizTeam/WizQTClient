#ifndef THUMBCACHE_P_H
#define THUMBCACHE_P_H

#include <QObject>
#include <QMap>


struct WIZABSTRACT;
struct WIZDOCUMENTDATA;

class ThumbCache;


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



#endif // THUMBCACHE_P_H
