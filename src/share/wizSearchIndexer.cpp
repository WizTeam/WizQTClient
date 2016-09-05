#include "wizSearchIndexer.h"

#include <QFile>
#include <QMetaType>
#include <QDebug>
#include <QCoreApplication>

#include <unistd.h>

#include "wizdef.h"
#include "wizmisc.h"
#include "wizsettings.h"
#include "html/wizhtmlcollector.h"
#include "wizDatabase.h"
#include "utils/logger.h"
#include "utils/pathresolve.h"

#define TIMEINTERVAL  60 * 1000

WizSearchIndexer::WizSearchIndexer(WizDatabaseManager& dbMgr, QObject *parent)
    : QThread(parent)
    , m_dbMgr(dbMgr)
    , m_stop(false)
{
    qRegisterMetaType<WIZDOCUMENTDATAEX>("WIZDOCUMENTDATAEX");

    m_strIndexPath = m_dbMgr.db().getAccountPath() + "fts_index";

    // signals for deletion, database responsible for reset FTS flag when update document or attachment.
    connect(&m_dbMgr, SIGNAL(documentDeleted(const WIZDOCUMENTDATA&)), \
            SLOT(on_document_deleted(const WIZDOCUMENTDATA&)));
    connect(&m_dbMgr, SIGNAL(attachmentDeleted(const WIZDOCUMENTATTACHMENTDATA&)), \
            SLOT(on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA&)));

    m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), SLOT(on_timerOut()));
    connect(this, SIGNAL(startTimer(int)), &m_timer, SLOT(start(int)));
    connect(this, SIGNAL(stopTimer()), &m_timer, SLOT(stop()));
}

void WizSearchIndexer::rebuild() {
    if (!QMetaObject::invokeMethod(this, "rebuildFTSIndex")) {
        qDebug() << "\nInvoke rebuildFTSIndex failed\n";
    }
}

void WizSearchIndexer::on_timerOut()
{
    m_mutex.lock();
    m_wait.wakeAll();
    m_mutex.unlock();
}

void WizSearchIndexer::start(QThread::Priority priority)
{
    QThread::start(priority);

    emit startTimer(TIMEINTERVAL);
}

void WizSearchIndexer::run()
{
    while (!m_stop)
    {
        m_mutex.lock();
        m_wait.wait(&m_mutex);
        m_mutex.unlock();

        if (m_stop)
            return;
        //
        buildFTSIndex();

        emit startTimer(TIMEINTERVAL);
    }
}

void WizSearchIndexer::waitForDone()
{
    stop();

    WizWaitForThread(this);
}

bool WizSearchIndexer::buildFTSIndex()
{
    int nErrors = 0;

    // build private first
    if (!buildFTSIndexByDatabase(m_dbMgr.db())) {
        nErrors++;
    }

    if (m_stop)
        return true;

    // build group db
    int total = m_dbMgr.count();
    for (int i = 0; i < total; i++) {
        if (m_stop)
            return true;

        if (!buildFTSIndexByDatabase(m_dbMgr.at(i))) {
            nErrors++;
        }
    }

    if (nErrors) {
        TOLOG(tr("Build FTS index meet error, we'll rebuild it when restart"));
        return false;
    }

    return true;
}

void clearDatabaseCipher(WizDatabase& db)
{
    if (!db.isGroup())
    {
        WizUserSettings settings(db);
        if (!settings.isRememberNotePasswordForSession())
        {
            //TODO:wsj
        }
    }
}

bool WizSearchIndexer::buildFTSIndexByDatabase(WizDatabase& db)
{
    // if FTS version is lower than release, rebuild all
    int strVersion = db.getDocumentFTSVersion().toInt();
    if (strVersion < QString(WIZNOTE_FTS_VERSION).toInt()) {
        qDebug() << "FTS index update triggered...";
        clearFlags(db);
    }

    db.setDocumentFTSVersion(WIZNOTE_FTS_VERSION);

    CWizDocumentDataArray arrayDocuments;
    if (!db.getAllDocumentsNeedToBeSearchIndexed(arrayDocuments))
        return false;

    // filter document data have not downloadeded or encrypted
    bool searchEncryptedDoc = false;
    if (!db.isGroup()) {
        WizUserSettings settings(db);
        QString strPassword = settings.encryptedNotePassword();
        if (settings.searchEncryptedNote() && !strPassword.isEmpty()) {
            db.loadUserCert();
            //
            //TODO:wsj
            //
            searchEncryptedDoc = true;
        }
    }
    filterDocuments(db, arrayDocuments, searchEncryptedDoc);

    if (arrayDocuments.empty()) {
        if (searchEncryptedDoc) {
            clearDatabaseCipher(db);
        }
        return true;
    }

    int nErrors = 0;
    int nTotal = arrayDocuments.size();
    for (int i = 0; i < nTotal; i++) {
        if (m_stop) {
            return true;
        }

        const WIZDOCUMENTDATAEX& doc = arrayDocuments.at(i);

        TOLOG(tr("Update search index (%1/%2): %3").arg(i + 1).arg(nTotal).arg(doc.strTitle));
        if (!updateDocument(doc)) {
            TOLOG(tr("[WARNING] failed to update: %1").arg(doc.strTitle));
            nErrors++;
        }

        // release CPU
        msleep(100);
    }

    // clear usercipher after build fts
    if (searchEncryptedDoc) {
        clearDatabaseCipher(db);
    }

    if (nErrors >= 3) {
        TOLOG(tr("[WARNING] total %1 notes failed to build").arg(nErrors));
        return false;
    }

    return true;
}

void WizSearchIndexer::filterDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument, bool searchEncryptedDoc)
{
    int nCount = arrayDocument.size();
    for (intptr_t i = nCount - 1; i >= 0; i--) {
        //
        bool bFilter = false;
        WIZDOCUMENTDATAEX& doc = arrayDocument.at(i);

        if (!searchEncryptedDoc && doc.nProtected)
            bFilter = true;

        if (!bFilter)
        {
            QString strFileName = db.getDocumentFileName(doc.strGUID);
            //
            if (!QFile::exists(strFileName))
            {
                db.setDocumentDataDownloaded(doc.strGUID, false);
                bFilter = true;
            }
            else if (WizZiwReader::isEncryptedFile(strFileName))
            {
                bFilter = true;
            }
        }

        if (bFilter) {
            arrayDocument.erase(arrayDocument.begin() + i);
        }
    }
}

bool WizSearchIndexer::updateDocument(const WIZDOCUMENTDATAEX& doc)
{
    Q_ASSERT(!doc.strGUID.isEmpty());

    void* pHandle = NULL;

    if (!beginUpdateDocument(m_strIndexPath.toStdWString().c_str(), &pHandle)) {
        TOLOG("begin update failed while update FTS index");
        return false;
    }

    bool ret = _updateDocumentImpl(pHandle, doc);

    if (!endUpdateDocument(pHandle)) {
        TOLOG("end update failed while update FTS index");
        return false;
    }

    return ret;
}

bool WizSearchIndexer::_updateDocumentImpl(void *pHandle,
                                            const WIZDOCUMENTDATAEX& doc)
{
    WizDatabase& db = m_dbMgr.db(doc.strKbGUID);

    // decompress
    QString strTempFolder = Utils::WizPathResolve::tempPath() + doc.strGUID + "-update/";
    if (!db.documentToHtmlFile(doc, strTempFolder)) {
        TOLOG("Can't decompress document while update FTS index: " + doc.strTitle);
        //Q_ASSERT(0);
        return false;
    }
    QString strDataFile = strTempFolder + "index.html";

    // get plain text content
    QString strHtmlData;
    if (!::WizLoadUnicodeTextFromFile(strDataFile, strHtmlData)) {
        TOLOG("Can't load document data while update FTS index:" + doc.strTitle);
        return false;
    }

    QString strPlainText;
    WizHtmlToPlainText htmlConverter;
    htmlConverter.toText(strHtmlData, strPlainText);

    bool ret = false;
    if (!strPlainText.isEmpty()) {
        ret = WizCluceneSearch::updateDocument(pHandle,
                                                doc.strKbGUID.toStdWString().c_str(),
                                                doc.strGUID.toStdWString().c_str(),
                                                doc.strTitle.toLower().toStdWString().c_str(),
                                                strPlainText.toLower().toStdWString().c_str());
    } else {
        ret = true;
    }

    if (ret) {
        db.setDocumentSearchIndexed(doc.strGUID, true);
    }
    //
    ::WizDeleteAllFilesInFolder(strTempFolder);
    ::WizDeleteFolder(strTempFolder);

    return ret;
}

bool WizSearchIndexer::deleteDocument(const WIZDOCUMENTDATAEX& doc)
{
    Q_ASSERT(!doc.strKbGUID.isEmpty() && !doc.strGUID.isEmpty());

    qDebug() << "\nDocument FTS deleted: " << doc.strTitle << "\n";

    return WizCluceneSearch::deleteDocument(m_strIndexPath.toStdWString().c_str(),
                                             doc.strGUID.toStdWString().c_str());
}

bool WizSearchIndexer::rebuildFTSIndex()
{
    if (clearAllFTSData()) {
        m_mutex.lock();
        m_wait.wakeAll();
        m_mutex.unlock();
        return true;
    }

    return false;
}

void WizSearchIndexer::clearFlags(WizDatabase& db)
{
    if (!db.setDocumentFTSVersion("0")) {
        TOLOG1("FATAL: Can't reset db index flag: %1", db.name());
    }

    if (!db.setAllDocumentsSearchIndexed(false)) {
        TOLOG1("FATAL: Can't reset document index flag: %1", db.name());
    }
}

void WizSearchIndexer::stop()
{
    emit stopTimer();

    m_mutex.lock();
    m_stop = true;
    m_wait.wakeAll();
    m_mutex.unlock();
}

bool WizSearchIndexer::clearAllFTSData()
{
    if (!::WizDeleteAllFilesInFolder(m_strIndexPath)) {
        TOLOG("Can't delete old index files while rebuild FTS index");
        return false;
    }

    clearFlags(m_dbMgr.db());

    int total = m_dbMgr.count();
    for (int i = 0; i < total; i++) {
        clearFlags(m_dbMgr.at(i));
    }

    return true;
}

void WizSearchIndexer::on_document_deleted(const WIZDOCUMENTDATA& doc)
{
    deleteDocument(doc);
}

void WizSearchIndexer::on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA& attach)
{
    Q_UNUSED(attach);
}


#define SEARCH_PAGE_MAX 100


/* ----------------------------- CWizSearcher ----------------------------- */
WizSearcher::WizSearcher(WizDatabaseManager& dbMgr, QObject *parent)
    : QThread(parent)
    , m_dbMgr(dbMgr)
    , m_mutexWait(QMutex::NonRecursive)
    , m_stop(false)
{
    m_strIndexPath = m_dbMgr.db().getAccountPath() + "fts_index";
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

void WizSearcher::searchKeyword(const QString& strKeywords)
{
    Q_ASSERT(!strKeywords.isEmpty());

    QTime counter;
    counter.start();

    qDebug() << "\n[Search]search: " << strKeywords;

    searchDatabaseByKeyword(strKeywords);

    if (m_nResults < m_nMaxResult)
    {
        // NOTE: make sure convert keyword to lower case
        searchDocument(m_strIndexPath.toStdWString().c_str(),
                       strKeywords.toLower().toStdWString().c_str());
    }

    int nMilliseconds = counter.elapsed();
    qDebug() << "[Search]search times: " << nMilliseconds;

    emitSearchProcess(strKeywords);
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

    if (m_nResults < m_nMaxResult)
    {
        // NOTE: make sure convert keyword to lower case
        searchDocument(m_strIndexPath.toStdWString().c_str(),
                       strKeywords.toLower().toStdWString().c_str());
    }

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
