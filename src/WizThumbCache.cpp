#include "WizThumbCache.h"
#include "WizThumbCache_p.h"
#include "share/WizDatabaseManager.h"
#include "share/WizDatabase.h"
#include "share/WizThreads.h"


#define THUMB_CACHE_MAX 10000


WizThumbCachePrivate::WizThumbCachePrivate(WizThumbCache* cache)
    : q(cache)
{
    connect(WizDatabaseManager::instance(), SIGNAL(documentAbstractModified(const WIZDOCUMENTDATA&)),
            SLOT(onNoteThumbChanged(const WIZDOCUMENTDATA&)));
    connect(this, SIGNAL(thumbLoaded(const QString&, const QString&)),
            cache, SIGNAL(loaded(const QString&, const QString&)));
}

QString WizThumbCachePrivate::key(const QString& strKbGUID, const QString& strGUID)
{
    return strKbGUID + "::" + strGUID;
}

bool WizThumbCachePrivate::find(const QString& strKbGUID, const QString& strGUID, WIZABSTRACT& abs)
{
    QString strKey(key(strKbGUID, strGUID));
    if (m_mapThumb.contains(strKey)) {
        abs = m_mapThumb.value(strKey);
        return true;
    }

    if (m_mapThumb.size() >= THUMB_CACHE_MAX) {
        m_mapThumb.clear();
        qDebug() << "[ThumCache]pool is full, clear...";
    }

    load(strKbGUID, strGUID);
    return false;
}

void WizThumbCachePrivate::load(const QString& strKbGUID, const QString& strGUID)
{
    WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
        load_impl(strKbGUID, strGUID);
    });
}

void WizThumbCachePrivate::load_impl(const QString& strKbGUID, const QString& strGUID)
{
    if (!WizDatabaseManager::instance()->isOpened(strKbGUID)) {
        qDebug() << "[ThumbCache]discard for invalid kb: " << strKbGUID;
        return;
    }

    WIZABSTRACT abs;
    WizDatabase& db = WizDatabaseManager::instance()->db(strKbGUID);

    bool bUpdated = false;

    // update if not exist
    if (db.padAbstractFromGuid(strGUID, abs)) {
        bUpdated = false;
    } else {
        qDebug() << "[ThumbCache]thumb not exist, try update: " << strGUID;
        if (db.updateDocumentAbstract(strGUID)) {
            bUpdated = true;
        } else {
            bUpdated = false;
        }
    }

    // load again if updated
    if (bUpdated && !db.padAbstractFromGuid(strGUID, abs)) {
        qDebug() << "[ThumCache]failed to load thumb from db: " << strGUID;
    }

    abs.strKbGUID = strKbGUID;
    if (abs.text.isEmpty()) {
        abs.text = " ";
    }

    m_mapThumb.insert(key(strKbGUID, strGUID), abs);
    Q_EMIT thumbLoaded(strKbGUID, strGUID);
}

void WizThumbCachePrivate::onNoteThumbChanged(const WIZDOCUMENTDATA& data)
{
    load(data.strKbGUID, data.strGUID);
}



WizThumbCache* m_instance = 0;
WizThumbCachePrivate* d = 0;

WizThumbCache::WizThumbCache()
{
    Q_ASSERT(!m_instance);

    m_instance = this;
    d = new WizThumbCachePrivate(this);
}

WizThumbCache* WizThumbCache::instance()
{
    return m_instance;
}

WizThumbCache::~WizThumbCache()
{
    delete d;
    d = 0;
}

bool WizThumbCache::find(const QString& strKbGUID, const QString& strGUID, WIZABSTRACT& abs)
{
    return d->find(strKbGUID, strGUID, abs);
}
