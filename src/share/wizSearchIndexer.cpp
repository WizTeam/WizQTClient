#include "wizSearchIndexer.h"

#include "wizdef.h"
#include "html/wizhtmlcollector.h"

#include <QFile>
#include <QMetaType>
#include <QDebug>

#define WIZ_SEARCH_INDEXER_PAGE_MAX 100


CWizSearchIndexer::CWizSearchIndexer(CWizDatabaseManager& dbMgr, QObject *parent)
    : QObject(parent)
    , m_dbMgr(dbMgr)
    , m_bAbort(false)
{
    qRegisterMetaType<WIZDOCUMENTDATAEX>("WIZDOCUMENTDATAEX");

    m_timerFTS.setInterval(60*1000); // default 60 seconds for every build loop
    connect(&m_timerFTS, SIGNAL(timeout()), SLOT(on_timerFTS_timeout()));
    m_timerFTS.start();

    m_strIndexPath = m_dbMgr.db().GetAccountPath() + "fts_index";

    // signals for deletion, database responsible for reset FTS flag when update document or attachment.
    connect(&m_dbMgr, SIGNAL(documentDeleted(const WIZDOCUMENTDATA&)), \
            SLOT(on_document_deleted(const WIZDOCUMENTDATA&)));
    connect(&m_dbMgr, SIGNAL(attachmentDeleted(const WIZDOCUMENTATTACHMENTDATA&)), \
            SLOT(on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA&)));
}

void CWizSearchIndexer::search(const QString &strKeywords,
                               int nMaxSize /* = -1 */)
{
    m_mapDocumentSearched.clear();
    m_nMaxResult = nMaxSize;
    m_nResults = 0;

    if (!QMetaObject::invokeMethod(this, "searchKeyword",
                                   Q_ARG(const QString&, strKeywords))) {
        qDebug() << "\nInvoke searchKeyword failed\n";
    }
}

void CWizSearchIndexer::rebuild() {
    if (!QMetaObject::invokeMethod(this, "rebuildFTSIndex")) {
        qDebug() << "\nInvoke rebuildFTSIndex failed\n";
    }
}

void CWizSearchIndexer::abort()
{
    m_bAbort = true;
}

void CWizSearchIndexer::on_timerFTS_timeout()
{
    buildFTSIndex();
}

bool CWizSearchIndexer::buildFTSIndex()
{
    m_timerFTS.stop();

    m_bAbort = false;
    int nErrors = 0;

    // build private first
    if (!buildFTSIndexByDatabase(m_dbMgr.db())) {
        nErrors++;
    }

    // build group db
    int total = m_dbMgr.count();
    for (int i = 0; i < total; i++) {
        if (m_bAbort)
            break;

        if (!buildFTSIndexByDatabase(m_dbMgr.at(i))) {
            nErrors++;
        }
    }

    if (nErrors) {
        TOLOG(tr("Build FTS index meet error, we'll rebuild it when restart"));
        return false;
    }

    m_timerFTS.start();
    return true;
}

bool CWizSearchIndexer::buildFTSIndexByDatabase(CWizDatabase& db)
{
    // if FTS version is lower than release, rebuild all
    int strVersion = db.getDocumentFTSVersion().toInt();
    if (strVersion < QString(WIZNOTE_FTS_VERSION).toInt()) {
        clearFlags(db);
    }

    db.setDocumentFTSVersion(WIZNOTE_FTS_VERSION);

    CWizDocumentDataArray arrayDocuments;
    if (!db.getAllDocumentsNeedToBeSearchIndexed(arrayDocuments))
        return false;

    // filter document data have not downloadeded or encrypted
    filterDocuments(db, arrayDocuments);

    if (arrayDocuments.empty())
        return true;

    TOLOG(tr("Build FTS index begin: ") + db.name());
    TOLOG(tr("Total %1 documents needs to build search index").arg(arrayDocuments.size()));

    int nErrors = 0;
    for (int i = 0; i < arrayDocuments.size(); i++) {
        if (m_bAbort) {
            break;
        }

        const WIZDOCUMENTDATAEX& doc = arrayDocuments.at(i);

        TOLOG(tr("Update search index [%1]: %2").arg(i).arg(doc.strTitle));
        if (!updateDocument(doc)) {
            TOLOG(tr("[WARNING] failed to update: %1").arg(doc.strTitle));
            nErrors++;
        }

        // release CPU
        usleep(300);
    }

    if (nErrors >= 3) {
        TOLOG(tr("[WARNING] total %1 documents failed to build").arg(nErrors));
        return false;
    }

    TOLOG(tr("Build FTS index end succeed: ") + db.name());
    return true;
}

void CWizSearchIndexer::filterDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    int nCount = arrayDocument.size();
    for (intptr_t i = nCount - 1; i >= 0; i--) {
        bool bFilter = false;
        WIZDOCUMENTDATAEX& doc = arrayDocument.at(i);

        if (doc.nProtected)
            bFilter = true;

        QString strFileName = db.GetDocumentFileName(doc.strGUID);
        if (!QFile::exists(strFileName))
            bFilter = true;

        if (bFilter) {
            arrayDocument.erase(arrayDocument.begin() + i);
        }
    }
}

bool CWizSearchIndexer::updateDocument(const WIZDOCUMENTDATAEX& doc)
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

bool CWizSearchIndexer::_updateDocumentImpl(void *pHandle,
                                            const WIZDOCUMENTDATAEX& doc)
{
    CWizDatabase& db = m_dbMgr.db(doc.strKbGUID);

    // decompress
    QString strDataFile;
    if (!db.DocumentToTempHtmlFile(doc, strDataFile)) {
        TOLOG("Can't decompress document while update FTS index: " + doc.strTitle);
        Q_ASSERT(0);
        return false;
    }

    // get plain text content
    QString strHtmlData;
    if (!::WizLoadUnicodeTextFromFile(strDataFile, strHtmlData)) {
        TOLOG("Can't load document data while update FTS index:" + doc.strTitle);
        return false;
    }

    QString strPlainText;
    CWizHtmlToPlainText htmlConverter;
    htmlConverter.toText(strHtmlData, strPlainText);

    // NOTE: convert text to lower case
    bool ret = IWizCluceneSearch::updateDocument(pHandle,
                                                 doc.strKbGUID.toStdWString().c_str(),
                                                 doc.strGUID.toStdWString().c_str(),
                                                 doc.strTitle.toLower().toStdWString().c_str(),
                                                 strPlainText.toLower().toStdWString().c_str());

    if (ret) {
        db.setDocumentSearchIndexed(doc.strGUID, true);
    }

    return ret;
}

bool CWizSearchIndexer::deleteDocument(const WIZDOCUMENTDATAEX& doc)
{
    Q_ASSERT(!doc.strKbGUID.isEmpty() && !doc.strGUID.isEmpty());

    qDebug() << "\nDocument FTS deleted: " << doc.strTitle << "\n";

    return IWizCluceneSearch::deleteDocument(m_strIndexPath.toStdWString().c_str(),
                                             doc.strGUID.toStdWString().c_str());
}

bool CWizSearchIndexer::rebuildFTSIndex()
{
    if (clearAllFTSData()) {
        return buildFTSIndex();
    }

    return false;
}

void CWizSearchIndexer::clearFlags(CWizDatabase& db)
{
    if (!db.setDocumentFTSVersion("0")) {
        TOLOG1("FATAL: Can't reset db index flag: %1", db.name());
    }

    if (!db.setAllDocumentsSearchIndexed(false)) {
        TOLOG1("FATAL: Can't reset document index flag: %1", db.name());
    }
}

bool CWizSearchIndexer::clearAllFTSData()
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

void CWizSearchIndexer::searchKeyword(const QString& strKeywords)
{
    Q_ASSERT(!strKeywords.isEmpty());

    qDebug() << "\n[Search]search: " << strKeywords;

    searchDatabase(strKeywords);

    if (m_nMaxResult <= m_nResults)
        return;

    // NOTE: make sure convert keyword to lower case
    searchDocument(m_strIndexPath.toStdWString().c_str(),
                   strKeywords.toLower().toStdWString().c_str());
}

void CWizSearchIndexer::searchDatabase(const QString& strKeywords)
{
    CWizDocumentDataArray arrayDocument;
    m_dbMgr.db().SearchDocumentByTitle(strKeywords, NULL, true, 5000, arrayDocument);

    CWizDocumentDataArray::const_iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
        const WIZDOCUMENTDATAEX& doc = *it;
        m_mapDocumentSearched[doc.strGUID] = doc;
    }

    arrayDocument.clear();

    int nCount = m_dbMgr.count();
    for (int i = 0; i < nCount; i++) {
        m_dbMgr.at(i).SearchDocumentByTitle(strKeywords, NULL, true, 5000, arrayDocument);

        for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
            const WIZDOCUMENTDATAEX& doc = *it;
            m_mapDocumentSearched[doc.strGUID] = doc;
        }

        arrayDocument.clear();
    }

    qDebug() << QString("[Search]Find %1 results in database").arg(m_mapDocumentSearched.size());

    QMap<QString, WIZDOCUMENTDATAEX>::const_iterator i;
    for (i = m_mapDocumentSearched.begin(); i != m_mapDocumentSearched.end(); i++) {
        if (m_nMaxResult <= m_nResults) {
            return;
        }

        Q_EMIT documentFind(i.value());
        m_nResults++;
    }
}

bool CWizSearchIndexer::onSearchProcess(const wchar_t* lpszKbGUID,
                                        const wchar_t* lpszDocumentID,
                                        const wchar_t* lpszURL)
{
    Q_UNUSED(lpszURL);

    if (m_nMaxResult != -1 && m_nMaxResult <= m_nResults) {
        qDebug() << "\nSearch result is bigger than limits: " << m_nMaxResult;
        return true;
    }

    QString strKbGUID = QString::fromStdWString(lpszKbGUID);
    QString strGUID = QString::fromStdWString(lpszDocumentID);

    // not searched before
    if (m_mapDocumentSearched.contains(strGUID)) {
        return true;
    }

    // make sure document is not belong to invalid group
    if (!m_dbMgr.isOpened(strKbGUID)) {
        qDebug() << "\nsearch process meet invalid kb_guid: " << strKbGUID;
        return false;
    }

    // valid document
    WIZDOCUMENTDATA doc;
    if (!m_dbMgr.db(strKbGUID).DocumentFromGUID(strGUID, doc)) {
        qDebug() << "\nsearch process meet invalide document: " << strGUID;
        return false;
    }

    m_nResults++;
    m_mapDocumentSearched[strGUID] = doc;
    Q_EMIT documentFind(doc);

    return true;
}

bool CWizSearchIndexer::onSearchEnd()
{
    qDebug() << "[Search]Search process end, total: " << m_nResults << "\n";
    return true;
}

void CWizSearchIndexer::on_document_deleted(const WIZDOCUMENTDATA& doc)
{
    deleteDocument(doc);
}

void CWizSearchIndexer::on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA& attach)
{
    Q_UNUSED(attach);
}
