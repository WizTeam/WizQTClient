#include "wizThumbIndexCache.h"

#include <QDebug>

#include "wizDatabaseManager.h"
#include "wizDatabase.h"

#define WIZNOTE_THUMB_CACHE_MAX 1000
#define WIZNOTE_THUMB_CACHE_RESET 300

CWizThumbIndexCache::CWizThumbIndexCache(CWizExplorerApp& app)
    : m_dbMgr(app.databaseManager())
{
    qRegisterMetaType<WIZABSTRACT>("WIZABSTRACT");

    connect(&m_dbMgr, SIGNAL(documentAbstractModified(const WIZDOCUMENTDATA&)),
            SLOT(on_abstract_modified(const WIZDOCUMENTDATA&)));
}

bool CWizThumbIndexCache::isFull()
{
    return m_mapAbs.size() > WIZNOTE_THUMB_CACHE_MAX;
}

void CWizThumbIndexCache::get(const QString& strKbGUID,
                              const QString& strDocumentGUID,
                              bool bReload)
{
    // discard invalid request
    if (!m_dbMgr.isOpened(strKbGUID)) {
        qDebug() << "[Thumb Cache Pool]discard request, invalid kb guid: " << strKbGUID;
        return;
    }

    if (isFull()) {
        m_mapAbs.clear();
        qDebug() << "[Thumb Cache Pool]pool is full, clear...";
    }

    // search first
    QMap<QString, WIZABSTRACT>::iterator it =  m_mapAbs.find(strDocumentGUID);
    if (it != m_mapAbs.end()) {
        if (bReload) {
            m_mapAbs.erase(it);
        } else {
            //qDebug() << "[Thumb Cache Pool]abtract already in the pool: " << strDocumentGUID;
            Q_EMIT loaded(*it);
            return;
        }
    }

    WIZABSTRACT abs;
    CWizDatabase& db = m_dbMgr.db(strKbGUID);

    // update if not exist
    if (!db.PadAbstractFromGUID(strDocumentGUID, abs)) {
        if (!db.UpdateDocumentAbstract(strDocumentGUID)) {
            return;
        }
    }

    // load again
    if (!db.PadAbstractFromGUID(strDocumentGUID, abs)) {
        qDebug() << "[Thumb Cache Pool]failed to load thumb cache, guid: "
                 << strDocumentGUID;
        return;
    }

    abs.strKbGUID = strKbGUID;

    if (abs.text.isEmpty()) {
        abs.text = " ";
    }

    //qDebug() << "[Thumb Cache Pool]load thumb cache, total: " << m_mapAbs.size();
    m_mapAbs.insert(strDocumentGUID,abs);
    Q_EMIT loaded(abs);
}

void CWizThumbIndexCache::load(const QString& strKbGUID,
                               const QString& strDocumentGUID)
{
    if (!QMetaObject::invokeMethod(this, "get",
                                   Q_ARG(QString, strKbGUID),
                                   Q_ARG(QString, strDocumentGUID),
                                   Q_ARG(bool, false))) {
        TOLOG("Invoke load of thumb cache failed");
    }
}

void CWizThumbIndexCache::on_abstract_modified(const WIZDOCUMENTDATA& doc)
{
    if (!QMetaObject::invokeMethod(this, "get",
                                   Q_ARG(QString, doc.strKbGUID),
                                   Q_ARG(QString, doc.strGUID),
                                   Q_ARG(bool, true))) {
        TOLOG("Invoke load of thumb cache failed");
    }
}
