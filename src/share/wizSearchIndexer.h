#ifndef WIZSEARCHINDEXER_H
#define WIZSEARCHINDEXER_H

#include <QThread>
#include <QTimer>
#include <QMetaType>

#include "wizClucene.h"
#include "wizDatabaseManager.h"


class CWizSearchIndexer
        : public QObject
        , public IWizSearchDocumentsEvents
{
    Q_OBJECT

public:
    explicit CWizSearchIndexer(CWizDatabaseManager& dbMgr, QObject *parent = 0);

// only call these methods use QMetaObject::invokeMethod
public Q_SLOTS:
    bool isFTSEnabled();
    bool buildFTSIndex();
    bool buildFTSIndexByDatabase(CWizDatabase& db);
    bool rebuildFTSIndex();
    bool updateDocument(const WIZDOCUMENTDATAEX& doc);
    bool updateDocuments(const CWizDocumentDataArray& arrayDocuments);
    bool deleteDocument(const WIZDOCUMENTDATAEX& doc);
    bool deleteDocuments(const CWizDocumentDataArray& arrayDocuments);
    bool search(const QString& strKeywords, int nMaxSize);

protected:
    virtual bool onSearchProcess(const wchar_t* lpszKbGUID, const wchar_t* lpszDocumentID, const wchar_t* lpszURL);
    virtual bool onSearchEnd();

private:
    CWizDatabaseManager& m_dbMgr;
    QString m_strIndexPath;
    //QString m_strKbGUID;

    // collect search result and emit documentFind signal for every m_nMaxResult times
    // this variable is used for tuning proformance of clucene search engine
    int m_nMaxResult;

    QTimer m_timerSearch;
    bool m_bSearchEnd;

    // store search result for signal emiting
    CWizDocumentDataArray m_arrayDocument;

    bool _updateDocumentImpl(void *pHandle, const WIZDOCUMENTDATAEX& doc);

private Q_SLOTS:
    void on_searchTimeout();
    void on_document_created(const WIZDOCUMENTDATA& doc);
    void on_document_modified(const WIZDOCUMENTDATA& docOld, \
                              const WIZDOCUMENTDATA& docNew);
    void on_documentData_modified(const WIZDOCUMENTDATA& doc);
    void on_document_deleted(const WIZDOCUMENTDATA& doc);

    void on_attachment_created(const WIZDOCUMENTATTACHMENTDATA& attach);
    void on_attachment_modified(const WIZDOCUMENTATTACHMENTDATA& attachOld, \
                                const WIZDOCUMENTATTACHMENTDATA& attachNew);
    void on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA& attach);

Q_SIGNALS:
    void documentFind(const CWizDocumentDataArray& arrayDocument);
};


// thread wrapper of CWizSearchIndexer
class CWizSearchIndexerThread : public QThread
{
    Q_OBJECT

public:
    CWizSearchIndexerThread(CWizDatabaseManager& dbMgr, QObject *parent = 0)
        : QThread(parent)
        , m_dbMgr(dbMgr)
        , m_indexer(NULL)
    {
    }

    ~CWizSearchIndexerThread()
    {
        delete m_indexer;
    }

    virtual void run()
    {
        m_indexer = new CWizSearchIndexer(m_dbMgr);

        qRegisterMetaType<CWizDocumentDataArray>("CWizDocumentDataArray");

        // signal pass through
        connect(m_indexer, SIGNAL(documentFind(const CWizDocumentDataArray&)), \
                SIGNAL(documentFind(const CWizDocumentDataArray&)));

        exec();
    }

    CWizSearchIndexer* worker() const
    {
        Q_ASSERT(m_indexer);
        return m_indexer;
    }

private:
    QPointer<CWizSearchIndexer> m_indexer;
    CWizDatabaseManager& m_dbMgr;

Q_SIGNALS:
    void documentFind(const CWizDocumentDataArray& arrayDocument);
};

#endif // WIZSEARCHINDEXER_H
