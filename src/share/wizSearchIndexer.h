#ifndef WIZSEARCHINDEXER_H
#define WIZSEARCHINDEXER_H

#include <QTimer>
#include <QMap>
#include <QThread>
#include  <deque>
#include <QWaitCondition>

#include "wizClucene.h"
#include "wizDatabaseManager.h"
#include "share/wizqthelper.h"

struct WIZDOCUMENTDATAEX;
typedef std::deque<WIZDOCUMENTDATAEX> CWizDocumentDataArray;


enum SearchDateInterval {
    today = 0,
    yesterday,
    dayBeforeYesterday,
    oneWeek,
    oneMonth
};

enum SearchScope {
    Scope_AllNotes = 0,
    Scope_PersonalNotes,
    Scope_GroupNotes
};


/* --------------------------- CWizSearchIndexer --------------------------- */
class CWizSearchIndexer
        : public QThread
        , public IWizCluceneSearch
{
    Q_OBJECT

public:
    explicit CWizSearchIndexer(CWizDatabaseManager& dbMgr, QObject *parent = 0);
    void waitForDone();
    void rebuild();

protected:
    virtual void run();

private:
    bool buildFTSIndex();
    bool buildFTSIndexByDatabase(CWizDatabase& db);
    void filterDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument,
                         bool searchEncryptedDoc);
    bool updateDocument(const WIZDOCUMENTDATAEX& doc);
    bool deleteDocument(const WIZDOCUMENTDATAEX& doc);

    bool _updateDocumentImpl(void *pHandle, const WIZDOCUMENTDATAEX& doc);

    Q_INVOKABLE bool rebuildFTSIndex();
    bool clearAllFTSData();
    void clearFlags(CWizDatabase& db);

    void stop();

private:
    CWizDatabaseManager& m_dbMgr;
    QString m_strIndexPath; // working path
    bool m_stop;
    bool m_buldNow;

private Q_SLOTS:
    void on_document_deleted(const WIZDOCUMENTDATA& doc);
    void on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA& attach);
};


/* ----------------------------- CWizSearcher ----------------------------- */
class CWizSearcher
        : public QThread
        , public IWizCluceneSearch
{
    Q_OBJECT

public:
    explicit CWizSearcher(CWizDatabaseManager& dbMgr, QObject *parent = 0);
    void waitForDone();

    void search(const QString& strKeywords, int nMaxSize = -1, SearchScope scope = Scope_AllNotes);
    void searchByDateCreate(SearchDateInterval dateInterval, int nMaxSize = -1, SearchScope scope = Scope_AllNotes);
    void searchByDateModified(SearchDateInterval dateInterval, int nMaxSize = -1, SearchScope scope = Scope_AllNotes);
    void searchByDateAccessed(SearchDateInterval dateInterval, int nMaxSize = -1, SearchScope scope = Scope_AllNotes);
    void searchBySQLWhere(const QString& strWhere, int nMaxSize = -1, SearchScope scope = Scope_AllNotes);
    void searchByKeywordAndWhere(const QString& strKeywords, const QString& strWhere, int nMaxSize = -1
            , SearchScope scope = Scope_AllNotes);

protected:
    virtual bool onSearchProcess(const std::string& lpszKbGUID, const std::string& lpszDocumentID, const std::string& lpszURL);
    virtual bool onSearchEnd();

    virtual void run();

private:
    CWizDatabaseManager& m_dbMgr;
    QString m_strIndexPath; // working path
    QString m_strkeywords;
    int m_nMaxResult;
    SearchScope m_scope;

    bool m_stop;
    QMutex m_mutexWait;
    QWaitCondition m_wait;


    // guid-document map, search faster
    QMap<QString, WIZDOCUMENTDATAEX> m_mapDocumentSearched;
    int m_nResults; // results returned

    void doSearch();

    Q_INVOKABLE void searchKeyword(const QString& strKeywords);
    void searchDatabaseByKeyword(const QString& strKeywords);
    COleDateTime getDateByInterval(SearchDateInterval dateInterval);

    void emitSearchProcess(const QString& strKeywords);

    void stop();

Q_SIGNALS:
    void searchProcess(const QString& strKeywords, const CWizDocumentDataArray& arrayDocument, bool bStart,  bool bEnd);
};

#endif // WIZSEARCHINDEXER_H
