#ifndef WIZTHUMBINDEXCACHE_H
#define WIZTHUMBINDEXCACHE_H

#include <QObject>
#include <QMap>

#include "wizdef.h"
#include "wizobject.h"

class CWizThumbIndexCache : public QObject
{
    Q_OBJECT

public:
    CWizThumbIndexCache(CWizExplorerApp& app);

    void load(const QString& strKbGUID, const QString& strDocumentGUID);
    bool isFull();

private:
    CWizDatabaseManager& m_dbMgr;
    QMap<QString, WIZABSTRACT> m_mapAbs;

protected Q_SLOTS:
    void get(const QString& strKbGUID, const QString& strDocumentGUID, bool bReload);
    void on_abstract_modified(const WIZDOCUMENTDATA& doc);

Q_SIGNALS:
    void loaded(const WIZABSTRACT& abs);
};


#endif // WIZTHUMBINDEXCACHE_H
