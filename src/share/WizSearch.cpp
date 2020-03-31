#include "WizSearch.h"

#include <QFile>
#include <QMetaType>
#include <QDebug>
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QUrlQuery>

#ifndef Q_OS_WIN
#include <unistd.h>
#endif

#include "WizDef.h"
#include "WizMisc.h"
#include "WizSettings.h"
#include "html/WizHtmlCollector.h"
#include "WizDatabase.h"
#include "utils/WizLogger.h"
#include "utils/WizPathResolve.h"
#include "sync/WizApiEntry.h"
#include "sync/WizSync.h"
#include "share/WizEventLoop.h"
#include "share/jsoncpp/json/json.h"
#include "sync/WizToken.h"


#define SEARCH_PAGE_MAX 100


/* ----------------------------- CWizSearcher ----------------------------- */
WizSearcher::WizSearcher(WizDatabaseManager& dbMgr, QObject *parent)
    : QThread(parent)
    , m_dbMgr(dbMgr)
    , m_mutexWait(QMutex::NonRecursive)
    , m_stop(false)
{
    qRegisterMetaType<CWizDocumentDataArray>("CWizDocumentDataArray");
}

void WizSearcher::search(const QString &strKeywords, int nMaxSize /* = -1 */, SearchScope scope)
{
    m_mutexWait.lock();
    m_strkeywords = strKeywords;
    m_nMaxResult = nMaxSize;
    m_scope = scope;
    m_wait.wakeAll();
    m_mutexWait.unlock();

}

void WizSearcher::searchByDateCreate(SearchDateInterval dateInterval, int nMaxSize, SearchScope scope)
{
    m_mapDocumentSearched.clear();
    m_nMaxResult = nMaxSize;
    m_scope = scope;
    WizOleDateTime dt = getDateByInterval(dateInterval);
    //
    CWizDocumentDataArray arrayDocument;
    CWizDocumentDataArray::const_iterator it;
    if (Scope_AllNotes == m_scope || Scope_PersonalNotes == m_scope)
    {
        m_dbMgr.db().getRecentDocumentsByCreatedTime(dt, arrayDocument);

        for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {

            const WIZDOCUMENTDATAEX& doc = *it;
            m_mapDocumentSearched[doc.strGUID] = doc;
            m_nResults++;
            if (m_nResults > nMaxSize)
                break;
        }

        arrayDocument.clear();
    }
    if (Scope_AllNotes == m_scope || Scope_GroupNotes == m_scope)
    {
        int nCount = m_dbMgr.count();
        for (int i = 0; i < nCount; i++) {
            m_dbMgr.at(i).getRecentDocumentsByCreatedTime(dt, arrayDocument);

            for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
                const WIZDOCUMENTDATAEX& doc = *it;
                m_mapDocumentSearched[doc.strGUID] = doc;
                m_nResults++;
                if (m_nResults > nMaxSize)
                    break;
            }

            arrayDocument.clear();
        }
    }

    qDebug() << QString("[Search]Find %1 results in database").arg(m_mapDocumentSearched.size());

    emitSearchProcess("");
}

void WizSearcher::searchByDateModified(SearchDateInterval dateInterval, int nMaxSize, SearchScope scope)
{
    m_mapDocumentSearched.clear();
    m_nMaxResult = nMaxSize;
    m_scope = scope;
    WizOleDateTime dt = getDateByInterval(dateInterval);
    //
    CWizDocumentDataArray arrayDocument;
    CWizDocumentDataArray::const_iterator it;
    if (Scope_AllNotes == m_scope || Scope_PersonalNotes == m_scope)
    {
        m_dbMgr.db().getRecentDocumentsByModifiedTime(dt, arrayDocument);

        for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {

            const WIZDOCUMENTDATAEX& doc = *it;
            m_mapDocumentSearched[doc.strGUID] = doc;
            m_nResults++;
            if (m_nResults > nMaxSize)
                break;
        }

        arrayDocument.clear();
    }
    if (Scope_AllNotes == m_scope || Scope_GroupNotes == m_scope)
    {
        int nCount = m_dbMgr.count();
        for (int i = 0; i < nCount; i++) {
            m_dbMgr.at(i).getRecentDocumentsByModifiedTime(dt, arrayDocument);

            for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
                const WIZDOCUMENTDATAEX& doc = *it;
                m_mapDocumentSearched[doc.strGUID] = doc;
                m_nResults++;
                if (m_nResults > nMaxSize)
                    break;
            }

            arrayDocument.clear();
        }
    }

    qDebug() << QString("[Search]Find %1 results in database").arg(m_mapDocumentSearched.size());

    emitSearchProcess("");
}

void WizSearcher::searchByDateAccessed(SearchDateInterval dateInterval, int nMaxSize, SearchScope scope)
{
    m_mapDocumentSearched.clear();
    m_nMaxResult = nMaxSize;
    m_scope = scope;
    WizOleDateTime dt = getDateByInterval(dateInterval);
    //
    CWizDocumentDataArray arrayDocument;
    CWizDocumentDataArray::const_iterator it;
    if (Scope_AllNotes == m_scope || Scope_PersonalNotes == m_scope)
    {
        m_dbMgr.db().getRecentDocumentsByAccessedTime(dt, arrayDocument);

        for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {

            const WIZDOCUMENTDATAEX& doc = *it;
            m_mapDocumentSearched[doc.strGUID] = doc;
            m_nResults++;
            if (m_nResults > nMaxSize)
                break;
        }

        arrayDocument.clear();
    }
    if (Scope_AllNotes == m_scope || Scope_GroupNotes == m_scope)
    {
        int nCount = m_dbMgr.count();
        for (int i = 0; i < nCount; i++) {
            m_dbMgr.at(i).getRecentDocumentsByAccessedTime(dt, arrayDocument);

            for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
                const WIZDOCUMENTDATAEX& doc = *it;
                m_mapDocumentSearched[doc.strGUID] = doc;
                m_nResults++;
                if (m_nResults > nMaxSize)
                    break;
            }

            arrayDocument.clear();
        }
    }

    qDebug() << QString("[Search]Find %1 results in database").arg(m_mapDocumentSearched.size());

    emitSearchProcess("");
}

void WizSearcher::stop()
{
    m_stop = true;
    m_wait.wakeAll();
}

void WizSearcher::waitForDone()
{
    stop();

    WizWaitForThread(this);
}

void WizSearcher::doSearch()
{
    m_mapDocumentSearched.clear();
    m_nResults = 0;

    if (!QMetaObject::invokeMethod(this, "searchKeyword",
                                   Q_ARG(const QString&, m_strkeywords))) {
        qDebug() << "\nInvoke searchKeyword failed\n";
    }
}


void WizSearcher::searchDatabaseByKeyword(const QString& strKeywords)
{
    CWizDocumentDataArray arrayDocument;
    CWizDocumentDataArray::const_iterator it;
    //
    if (Scope_AllNotes == m_scope || Scope_PersonalNotes == m_scope)
    {
        m_dbMgr.db().searchDocumentByTitle(strKeywords, NULL, true, 5000, arrayDocument);

        for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {

            const WIZDOCUMENTDATAEX& doc = *it;
            m_mapDocumentSearched[doc.strGUID] = doc;
            m_nResults++;
        }

        arrayDocument.clear();
    }
    if (Scope_AllNotes == m_scope || Scope_GroupNotes == m_scope)
    {
        int nCount = m_dbMgr.count();
        for (int i = 0; i < nCount; i++) {
            m_dbMgr.at(i).searchDocumentByTitle(strKeywords, NULL, true, 5000, arrayDocument);

            for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
                const WIZDOCUMENTDATAEX& doc = *it;
                m_mapDocumentSearched[doc.strGUID] = doc;
                m_nResults++;
            }

            arrayDocument.clear();
        }
    }

    qDebug() << QString("[Search]Find %1 results in database").arg(m_mapDocumentSearched.size());
}

void WizSearcher::searchBySQLWhere(const QString& strWhere, int nMaxSize, SearchScope scope)
{
    m_mapDocumentSearched.clear();
    m_nMaxResult = nMaxSize;
    m_scope = scope;
    //
    CWizDocumentDataArray arrayDocument;
    CWizDocumentDataArray::const_iterator it;
    if (Scope_AllNotes == m_scope || Scope_PersonalNotes == m_scope)
    {
        m_dbMgr.db().searchDocumentByWhere(strWhere, 5000, arrayDocument);

        for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {

            const WIZDOCUMENTDATAEX& doc = *it;
            m_mapDocumentSearched[doc.strGUID] = doc;
            m_nResults++;
        }

        arrayDocument.clear();
    }
    if (Scope_AllNotes == m_scope || Scope_GroupNotes == m_scope)
    {
        int nCount = m_dbMgr.count();
        for (int i = 0; i < nCount; i++) {
            m_dbMgr.at(i).searchDocumentByWhere(strWhere, 5000, arrayDocument);

            for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
                const WIZDOCUMENTDATAEX& doc = *it;
                m_mapDocumentSearched[doc.strGUID] = doc;
                m_nResults++;
            }

            arrayDocument.clear();
        }
    }
    qDebug() << QString("[Search]Find %1 results in database").arg(m_mapDocumentSearched.size());

    emitSearchProcess("");
}

void WizSearcher::searchByKeywordAndWhere(const QString& strKeywords,
                                           const QString& strWhere, int nMaxSize, SearchScope scope)
{
    // search keyword
    Q_ASSERT(!strKeywords.isEmpty());
    qDebug() << "\n[Search]search: " << strKeywords;

    m_nMaxResult = nMaxSize;
    m_scope = scope;
    m_nResults = 0;
    m_mapDocumentSearched.clear();
    searchDatabaseByKeyword(strKeywords);

    // search by where
    QSet<QString> whereSet; // = mapDocByWhere.keys().toSet();
    CWizDocumentDataArray arrayDocument;
    m_dbMgr.db().searchDocumentByWhere(strWhere, 5000, arrayDocument);

    CWizDocumentDataArray::const_iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {

        const WIZDOCUMENTDATAEX& doc = *it;
        whereSet.insert(doc.strGUID);
    }

    arrayDocument.clear();

    int nCount = m_dbMgr.count();
    for (int i = 0; i < nCount; i++) {
        m_dbMgr.at(i).searchDocumentByWhere(strWhere, 5000, arrayDocument);

        for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
            const WIZDOCUMENTDATAEX& doc = *it;
            whereSet.insert(doc.strGUID);
        }

        arrayDocument.clear();
    }

    QSet<QString> keywordSet = m_mapDocumentSearched.keys().toSet();

    keywordSet.intersect(whereSet);

    QMap<QString, WIZDOCUMENTDATAEX> keywordSearchMap = m_mapDocumentSearched;
    m_mapDocumentSearched.clear();
    QMap<QString, WIZDOCUMENTDATAEX>::iterator itM;
    for (itM = keywordSearchMap.begin(); itM != keywordSearchMap.end(); itM++)
    {
        if (keywordSet.contains(itM.key()))
            m_mapDocumentSearched.insert(itM.key(), itM.value());
    }

    qDebug() << QString("[Search]Find %1 results in database").arg(m_mapDocumentSearched.size());


    emitSearchProcess(strKeywords);
}

WizOleDateTime WizSearcher::getDateByInterval(SearchDateInterval dateInterval)
{
    WizOleDateTime dt;
    switch (dateInterval) {
    case today:
        dt = dt.addDays(-1);
        break;
    case yesterday:
        dt = dt.addDays(-2);
        break;
    case dayBeforeYesterday:
        dt = dt.addDays(-3);
        break;
    case oneWeek:
        dt = dt.addDays(-8);
        break;
    case oneMonth:
        dt = dt.addMonths(-1);
    default:
        break;
    }

    return dt;
}

void WizSearcher::emitSearchProcess(const QString& strKeywords)
{
    int nTimes = m_mapDocumentSearched.size() % SEARCH_PAGE_MAX ?
                (m_mapDocumentSearched.size() / SEARCH_PAGE_MAX) + 1: (m_mapDocumentSearched.size() / SEARCH_PAGE_MAX);
    int nPos = 0;
    for (int i = 0; i < nTimes; i++) {

        CWizDocumentDataArray arrayDocument;
        QMap<QString, WIZDOCUMENTDATAEX>::const_iterator it;
        int nCounter = 0;
        for (it = m_mapDocumentSearched.begin() + nPos; it != m_mapDocumentSearched.end() && nCounter < SEARCH_PAGE_MAX; it++, nCounter ++) {
            arrayDocument.push_back(it.value());
            nPos++;
        }

        bool bStart = i == 0;
        if (i == nTimes - 1) {
            Q_EMIT searchProcess(strKeywords, arrayDocument, bStart, true);
            return;
        } else {
            Q_EMIT searchProcess(strKeywords, arrayDocument, bStart, false);
        }

        QTime dieTime = QTime::currentTime().addMSecs(30);
        while (QTime::currentTime() < dieTime) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }
    }

    CWizDocumentDataArray arrayDocument;
    Q_EMIT searchProcess(strKeywords, arrayDocument, true, true);
}

bool WizSearcher::onSearchProcess(const std::string& lpszKbGUID,
                                   const std::string& lpszDocumentID,
                                   const std::string& lpszURL)
{
    Q_UNUSED(lpszURL);

    if (m_nMaxResult != -1 && m_nMaxResult <= m_nResults) {
        qDebug() << "\nSearch result is bigger than limits: " << m_nMaxResult;
        return true;
    }

    QString strKbGUID = QString::fromStdString(lpszKbGUID);
    QString strGUID = QString::fromStdString(lpszDocumentID);

    // not searched before
    if (m_mapDocumentSearched.contains(strGUID)) {
        return true;
    }

    // make sure document is not belong to invalid group
    if (!m_dbMgr.isOpened(strKbGUID)) {
        qDebug() << "\nsearch process meet invalid kb_guid: " << strKbGUID;
        return false;
    }

    // make sure document in the search scope
    if (Scope_PersonalNotes == m_scope && strKbGUID != m_dbMgr.db().kbGUID()) {
        return false;
    } else if (Scope_GroupNotes == m_scope && strKbGUID == m_dbMgr.db().kbGUID()) {
        return false;
    }

    // valid document
    WIZDOCUMENTDATA doc;
    if (!m_dbMgr.db(strKbGUID).documentFromGuid(strGUID, doc)) {
        qDebug() << "\nsearch process meet invalide document: " << strGUID;
        return false;
    }

    m_nResults++;
    m_mapDocumentSearched[strGUID] = doc;

    return true;
}

bool WizSearcher::onSearchEnd()
{
    qDebug() << "[Search]Search process end, total: " << m_nResults;
    return true;
}

void WizSearcher::run()
{
    QString strKeyWord;
    while (!m_stop)
    {
        //////
        {
            QMutexLocker lock(&m_mutexWait);
            m_wait.wait(&m_mutexWait);
            if (m_stop)
                return;

            strKeyWord = m_strkeywords;
        }
        //
        //
        if (!strKeyWord.isEmpty()) {
            doSearch();
        }
    }
}

QString JsonValueToText(const Json::Value& value)
{
    if (value.isArray())
    {
        CWizStdStringArray arr;
        for (Json::ArrayIndex i = 0; i < value.size(); i++)
        {
            const Json::Value& elem = value[i];
            arr.push_back(QString::fromStdString(elem.asString()));
        }
        //
        CString text;
        ::WizStringArrayToText(arr, text, " ");
        return text;
    }
    else if (value.isString())
    {
        return QString::fromStdString(value.asString());
    }
    else
    {
        return "";
    }
}

//
static void highlightKeywordsInString(QString& str, const QString& keywords)
{
    int start = 0;
    int keywordsLength = keywords.length();
    //
    while (1) {
        start = str.indexOf(keywords, start, Qt::CaseInsensitive);
        if (start == -1)
            return;
        //
        str.insert(start, "<em>");
        str.insert(start + 4 + keywordsLength, "</em>");
        start += 4 + keywordsLength + 5;
    }
}

//
bool WizSearcher::onlineSearch(const QString& kbGuid, const QString& keywords, CWizDocumentDataArray& arrayResult)
{
    if (onlineSearchOnly(kbGuid, keywords, arrayResult))
        return true;
    //
    WizDatabase& db = m_dbMgr.db(kbGuid);
    //
    if (!db.getDocumentsByTitle(keywords, arrayResult))
        return false;
    //
    for (auto& doc : arrayResult) {
        doc.strHighlightTitle = doc.strTitle;
        highlightKeywordsInString(doc.strHighlightTitle, keywords);
    }
    return true;
}


bool WizSearcher::onlineSearchOnly(const QString& kbGuid, const QString& keywords, CWizDocumentDataArray& arrayResult)
{
    QString token = WizToken::token();
    if (token.isEmpty()) {
        //
        return false;
    }
    //
    QUrlQuery postData;
    postData.addQueryItem("token", token);
    postData.addQueryItem("ss", keywords);
    if (kbGuid.isEmpty())
    {
        postData.addQueryItem("kb_guid", WizToken::userInfo().strKbGUID);
    }
    else
    {
        postData.addQueryItem("kb_guid", kbGuid);
    }
    //
    QString strKS = m_dbMgr.db().getKbServer(kbGuid);
    QString urlString = strKS + "/ks/note/search/" + kbGuid;
    if (urlString.isEmpty())
        return false;
    //
    QUrl url(urlString);
    url.setQuery(postData);
    //
    qDebug() << url.toString();
    //
    int start = WizGetTickCount();
    qDebug() << "start search";
    //
    QNetworkRequest request(url);
    QNetworkAccessManager net;
    QNetworkReply* reply = net.get(request);
    //
    WizAutoTimeOutEventLoop loop(reply);
    loop.exec();
    //
    int end = WizGetTickCount();
    qDebug() << "search time: " << end - start;

    if (loop.error() != QNetworkReply::NoError || loop.result().isEmpty())
    {
        qDebug() << "[Search] Search failed! error: " << loop.errorString();
        return false;
    }
    //
    Json::Value d;
    Json::Reader reader;
    if (!reader.parse(loop.result().constData(), d))
        return false;

    if (!d.isMember("return_code"))
    {
        qDebug() << "[Search] Can not get return code";
        return false;
    }

    int returnCode = d["return_code"].asInt();
    if (returnCode != 200)
    {
        qDebug() << "[Search] Return code was not 200, error:  " << returnCode << loop.result();
        return false;
    }
    //
    if (!d.isMember("result"))
    {
        qDebug() << "[Search] Can not get result";
        return false;
    }
    //
    const Json::Value& result = d["result"];
    if (!result.isArray())
    {
        qDebug() << "[Search] Result is not an array";
        return false;
    }
    //
    CWizStdStringArray docs;
    std::set<QString> guids;
    //
    std::map<QString, QString> highlightTitle;
    std::map<QString, QString> highlightText;
    //
    for (Json::ArrayIndex i = 0; i < result.size(); i++)
    {
        const Json::Value& elem = result[i];
        //
        QString docGuid = QString::fromStdString(elem["docGuid"].asString());
        //
        if (elem.isMember("highlight")) {
            //
            const Json::Value& highlight = elem["highlight"];
            //
            QString title;
            QString text;
            if (highlight.isMember("title"))
            {
                const Json::Value& titleVal = highlight["title"];
                title = JsonValueToText(titleVal);
            }
            if (highlight.isMember("text"))
            {
                const Json::Value& textVal = highlight["text"];
                text = JsonValueToText(textVal);
            }
            //
            //
            highlightTitle[docGuid] = title;
            highlightText[docGuid] = text;
        }
        //
        docs.push_back(docGuid);
        guids.insert(docGuid);
    }
    //
    WizDatabase& db = m_dbMgr.db(kbGuid);
    //
    CWizDocumentDataArray arrayDocument;
    if (!db.getDocumentsByGuids(docs, arrayDocument))
        return false;
    //
    CWizStdStringArray downloadDocGuids;
    //
    for (const WIZDOCUMENTDATAEX& doc : arrayDocument)
    {
        if (guids.find(doc.strGUID) == guids.end())
        {
            //not found in local
            //
            downloadDocGuids.push_back(doc.strGUID);
        }
    }
    //
    if (!downloadDocGuids.empty())
    {
        WIZUSERINFO userInfo = WizToken::userInfo();
        CWizDocumentDataArray downloadedArrayDocument;
        WizDownloadDocumentsByGuids(userInfo,
                                         &db,
                                         kbGuid,
                                         downloadDocGuids,
                                         downloadedArrayDocument);
        //
        arrayDocument.clear();
        if (!db.getDocumentsByGuids(docs, arrayDocument))
            return false;
    }
    //
    std::map<QString, WIZDOCUMENTDATAEX> docMaps;
    for (const WIZDOCUMENTDATAEX& doc : arrayDocument)
    {
        docMaps[doc.strGUID] = doc;
    }
    //
    for (const QString& guid : docs)
    {
        auto it = docMaps.find(guid);
        if (it != docMaps.end())
        {
            WIZDOCUMENTDATAEX doc = it->second;
            doc.strHighlightTitle = highlightTitle[guid];
            doc.strHighlightText = highlightText[guid];
            arrayResult.push_back(doc);
        }
    }
    //
    return true;
}

