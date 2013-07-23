#ifndef WIZSEARCHINDEXER_H
#define WIZSEARCHINDEXER_H

#include <QTimer>
#include <QMap>

#include "wizClucene.h"
#include "wizDatabaseManager.h"


class CWizSearchIndexer
        : public QObject
        , public IWizCluceneSearch
{
    Q_OBJECT

public:
    explicit CWizSearchIndexer(CWizDatabaseManager& dbMgr, QObject *parent = 0);

    void abort();
    void rebuild();
    void search(const QString& strKeywords, int nMaxSize = -1);

private:
    bool buildFTSIndex();
    bool buildFTSIndexByDatabase(CWizDatabase& db);
    void filterDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument);
    bool updateDocument(const WIZDOCUMENTDATAEX& doc);
    bool deleteDocument(const WIZDOCUMENTDATAEX& doc);

    bool _updateDocumentImpl(void *pHandle, const WIZDOCUMENTDATAEX& doc);

    Q_INVOKABLE bool rebuildFTSIndex();
    bool clearAllFTSData();
    void clearFlags(CWizDatabase& db);

    Q_INVOKABLE void searchKeyword(const QString& strKeywords);
    void searchDatabase(const QString& strKeywords);

protected:
    virtual bool onSearchProcess(const wchar_t* lpszKbGUID, const wchar_t* lpszDocumentID, const wchar_t* lpszURL);
    virtual bool onSearchEnd();

private:
    CWizDatabaseManager& m_dbMgr;
    QString m_strIndexPath; // working path
    QTimer m_timerFTS;  // working interval
    bool m_bAbort;

    // guid-document map, search faster
    QMap<QString, WIZDOCUMENTDATAEX> m_mapDocumentSearched;
    int m_nResults; // results returned
    int m_nMaxResult;


private Q_SLOTS:
    void on_timerFTS_timeout();
    void on_document_deleted(const WIZDOCUMENTDATA& doc);
    void on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA& attach);

Q_SIGNALS:
    void documentFind(const WIZDOCUMENTDATAEX& data);
};

#endif // WIZSEARCHINDEXER_H
