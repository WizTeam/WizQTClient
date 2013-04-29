#include "wizThumbIndexCache.h"

#include "wizDatabaseManager.h"

#define WIZNOTE_THUMB_CACHE_MAX 1000
#define WIZNOTE_THUMB_CACHE_RESET 300

CWizThumbIndexCache::CWizThumbIndexCache(CWizExplorerApp& app)
    : m_dbMgr(app.databaseManager())
{
    connect(&m_dbMgr, SIGNAL(documentAbstractModified(const WIZDOCUMENTDATA&)),
            SLOT(on_abstract_modified(const WIZDOCUMENTDATA&)));
}

bool CWizThumbIndexCache::isFull()
{
    return m_data.size() > WIZNOTE_THUMB_CACHE_MAX;
}

void CWizThumbIndexCache::get(const QString& strKbGUID,
                              const QString& strDocumentGUID,
                              bool bReload)
{
    // search deque first
    CWizAbstractArray::iterator it;
    for (it = m_data.begin(); it != m_data.end(); it++) {
        const WIZABSTRACT& abs = *it;

        if (abs.strKbGUID == strKbGUID && abs.guid == strDocumentGUID) {
            if (bReload) {
                m_data.erase(it);
                break;
            } else {
                Q_EMIT loaded(abs);
            }
            return;
        }
    }

    WIZABSTRACT abs;
    CWizDatabase& db = m_dbMgr.db(strKbGUID);

    if (!db.PadAbstractFromGUID(strDocumentGUID, abs)) {
        if (!db.UpdateDocumentAbstract(strDocumentGUID)) {
            return;
        } else {
            if (!db.PadAbstractFromGUID(strDocumentGUID, abs)) {
                return;
            }
        }
    }

    abs.strKbGUID = strKbGUID;

    if (abs.text.isEmpty()) {
        abs.text = " ";
    }
    abs.text.replace('\n', ' ');
    abs.text.replace("\r", "");

    if (!isFull()) {
        m_data.push_back(abs);
    } else {
        m_data.erase(m_data.begin(), m_data.begin() + WIZNOTE_THUMB_CACHE_RESET);
    }

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
