#ifndef WIZSEARCH_H
#define WIZSEARCH_H

#include <QTimer>
#include <QMap>
#include <QThread>
#include  <deque>
#include <QWaitCondition>

#include "WizDatabaseManager.h"
#include "share/WizQtHelper.h"

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


/* ----------------------------- CWizSearcher ----------------------------- */
class WizSearcher
        : public QThread
{
    Q_OBJECT

public:
    explicit WizSearcher(WizDatabaseManager& dbMgr, QObject *parent = 0);
    void waitForDone();

    void search(const QString& strKeywords, int nMaxSize = -1, SearchScope scope = Scope_AllNotes);
    void searchByDateCreate(SearchDateInterval dateInterval, int nMaxSize = -1, SearchScope scope = Scope_AllNotes);
    void searchByDateModified(SearchDateInterval dateInterval, int nMaxSize = -1, SearchScope scope = Scope_AllNotes);
    void searchByDateAccessed(SearchDateInterval dateInterval, int nMaxSize = -1, SearchScope scope = Scope_AllNotes);
    void searchBySQLWhere(const QString& strWhere, int nMaxSize = -1, SearchScope scope = Scope_AllNotes);
    void searchByKeywordAndWhere(const QString& strKeywords, const QString& strWhere, int nMaxSize = -1
            , SearchScope scope = Scope_AllNotes);
    //
    bool onlineSearch(const QString& kbGuid, const QString& keywords, CWizDocumentDataArray& arrayResult);

protected:
    virtual bool onSearchProcess(const std::string& lpszKbGUID, const std::string& lpszDocumentID, const std::string& lpszURL);
    virtual bool onSearchEnd();

    virtual void run();

private:
    WizDatabaseManager& m_dbMgr;
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
    bool onlineSearchOnly(const QString& kbGuid, const QString& keywords, CWizDocumentDataArray& arrayResult);

    void searchDatabaseByKeyword(const QString& strKeywords);
    WizOleDateTime getDateByInterval(SearchDateInterval dateInterval);

    void emitSearchProcess(const QString& strKeywords);

    void stop();

Q_SIGNALS:
    void searchProcess(const QString& strKeywords, const CWizDocumentDataArray& arrayDocument, bool bStart,  bool bEnd);
};

#endif // WIZSEARCH_H
