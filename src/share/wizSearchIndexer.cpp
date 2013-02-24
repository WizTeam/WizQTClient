#include "wizSearchIndexer.h"

#include <QFile>

CWizSearchIndexer::CWizSearchIndexer(CWizDatabaseManager& dbMgr, QObject *parent)
    : m_dbMgr(dbMgr)
    , QObject(parent)
{
    m_timerSearch.setInterval(300);
    connect(&m_timerSearch, SIGNAL(timeout()), SLOT(on_searchTimeout()));

    m_strIndexPath = m_dbMgr.db().GetAccountPath() + "fts_index";

    connect(&m_dbMgr, SIGNAL(documentCreated(const WIZDOCUMENTDATA&)), \
            SLOT(on_document_created(const WIZDOCUMENTDATA&)));
    connect(&m_dbMgr, SIGNAL(documentModified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)), \
            SLOT(on_document_modified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)));
    connect(&m_dbMgr, SIGNAL(documentDataModified(const WIZDOCUMENTDATA&)), \
            SLOT(on_documentData_modified(const WIZDOCUMENTDATA&)));
    connect(&m_dbMgr, SIGNAL(documentDeleted(const WIZDOCUMENTDATA&)), \
            SLOT(on_document_deleted(const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(attachmentCreated(const WIZDOCUMENTATTACHMENTDATA&)), \
            SLOT(on_attachment_created(const WIZDOCUMENTATTACHMENTDATA&)));
    connect(&m_dbMgr, SIGNAL(attachmentModified(const WIZDOCUMENTATTACHMENTDATA&, const WIZDOCUMENTATTACHMENTDATA&)), \
            SLOT(on_attachment_modified(const WIZDOCUMENTATTACHMENTDATA&, const WIZDOCUMENTATTACHMENTDATA&)));
    connect(&m_dbMgr, SIGNAL(attachmentDeleted(const WIZDOCUMENTATTACHMENTDATA&)), \
            SLOT(on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA&)));
}

bool CWizSearchIndexer::buildFTSIndex()
{
    TOLOG(tr("Build FTS index begin"));

    int nErrors = 0;
    int total = m_dbMgr.count();
    for (int i = 0; i < total; i++) {
        if (!buildFTSIndexByDatabase(m_dbMgr.at(i))) {
            nErrors++;
        }
    }

    if (nErrors) {
        TOLOG(tr("Build FTS index meet error, we'll rebuild it when restart"));
        return false;
    }

    TOLOG(tr("Build FTS index end successfully"));
    return true;
}

bool CWizSearchIndexer::buildFTSIndexByDatabase(CWizDatabase& db)
{
    CWizDocumentDataArray arrayDocuments;

    if (!db.getAllDocumentsNeedToBeSearchIndexed(arrayDocuments))
        return false;

    if (arrayDocuments.empty()) {
        return true;
    }

    bool ret = updateDocuments(arrayDocuments);

    if (ret) {
        db.setDocumentFTSEnabled(true);
        return true;
    } else {
        db.setDocumentFTSEnabled(false);
        return false;
    }
}

bool CWizSearchIndexer::isFTSEnabled()
{
    int total = m_dbMgr.count();
    for (int i = 0; i < total; i++) {
        if (!m_dbMgr.at(i).isDocumentFTSEnabled()) {
            return false;
        }
    }

    return true;
}

bool CWizSearchIndexer::rebuildFTSIndex()
{
    if (!::WizDeleteAllFilesInFolder(m_strIndexPath)) {
        TOLOG("Can't delete old index files while rebuild FTS index");
        return false;
    }

    int total = m_dbMgr.count();
    for (int i = 0; i < total; i++) {
        if (!m_dbMgr.at(i).setDocumentFTSEnabled(false)) {
            TOLOG1("FATAL: Can't reset db index flag: %1", m_dbMgr.at(i).kbGUID());
            return false;
        }

        if (!m_dbMgr.at(i).setAllDocumentsSearchIndexed(false)) {
            TOLOG1("FATAL: Can't reset document index flag: %1", m_dbMgr.at(i).kbGUID());
            return false;
        }
    }

    return buildFTSIndex();
}

bool CWizSearchIndexer::updateDocument(const WIZDOCUMENTDATAEX& doc)
{
    Q_ASSERT(!doc.strGUID.isEmpty());

    CWizDocumentDataArray arrayDocuments;
    arrayDocuments.push_back(doc);
    return updateDocuments(arrayDocuments);
}

bool CWizSearchIndexer::updateDocuments(const CWizDocumentDataArray& arrayDocuments)
{
    Q_ASSERT(!arrayDocuments.empty());

    void* pHandle = NULL;
    bool ret = WizFTSBeginUpdateDocument(m_strIndexPath.toStdWString().c_str(), &pHandle);
    if (!ret) {
        TOLOG("begin update failed while update FTS index");
        return false;
    }

    int nErrors = 0;
    for (int i = 0; i < arrayDocuments.size(); i++) {
        WIZDOCUMENTDATAEX doc = arrayDocuments.at(i);
        if (!_updateDocumentImpl(pHandle, doc))
            nErrors++;
    }

    ret = WizFTSEndUpdateDocument(pHandle);
    if (!ret) {
        TOLOG("end update failed while update FTS index");
        return false;
    }

    if (nErrors >= 5) {
        return false;
    }

    return true;
}

bool CWizSearchIndexer::_updateDocumentImpl(void *pHandle, const WIZDOCUMENTDATAEX& doc)
{
    CWizDatabase& db = m_dbMgr.db(doc.strKbGUID);

    db.setDocumentSearchIndexed(doc.strGUID, false);

    QString strFileName = db.GetDocumentFileName(doc.strGUID);

    // document data have not downloaded yet
    if (!QFile::exists(strFileName)) {
        return true;
    }

    // FIXME : deal with encrypted document
    if (doc.nProtected) {
       return true;
    }

    // decompress
    QString strDataFile;
    if (!db.DocumentToTempHtmlFile(doc, strDataFile)) {
        TOLOG("Can't decompress document while update FTS index: " + doc.strTitle);
        return false;
    }

    // get plain text content
    QString strHtmlData;
    if (!::WizLoadUnicodeTextFromFile(strDataFile, strHtmlData)) {
        TOLOG("Can't load document data while update FTS index:" + doc.strTitle);
        return false;
    }

    QString strPlainText;
    ::WizHtml2Text(strHtmlData, strPlainText);
    if (strPlainText.isEmpty()) {
        TOLOG("Html to text is failed: " + doc.strTitle);
        return false;
    }

    // NOTE: convert text to lower case
    bool ret = WizFTSUpdateDocument(pHandle, \
                                    doc.strKbGUID.toStdWString().c_str(), \
                                    doc.strGUID.toStdWString().c_str(), \
                                    doc.strTitle.toStdWString().c_str(), \
                                    strPlainText.toLower().toStdWString().c_str());

    if (ret) {
        db.setDocumentSearchIndexed(doc.strGUID, true);
    }

    return ret;
}

bool CWizSearchIndexer::deleteDocument(const WIZDOCUMENTDATAEX& doc)
{
    Q_ASSERT(!doc.strGUID.isEmpty());

    CWizDocumentDataArray arrayDocuments;
    arrayDocuments.push_back(doc);
    return deleteDocuments(arrayDocuments);
}

bool CWizSearchIndexer::deleteDocuments(const CWizDocumentDataArray& arrayDocuments)
{
    Q_ASSERT(!arrayDocuments.empty());

    bool ret = true;
    for (int i = 0; i < arrayDocuments.size(); i++) {
        WIZDOCUMENTDATAEX doc = arrayDocuments.at(i);
        int ret = WizFTSDeleteDocument(m_strIndexPath.toStdWString().c_str(), doc.strGUID.toStdWString().c_str());
        if (!ret) {
            TOLOG("delete FTS index failed: " + doc.strTitle);
            ret = false;
        }
    }

    return ret;
}

bool CWizSearchIndexer::search(const QString& strKeywords, int nMaxResult)
{
    Q_ASSERT(!strKeywords.isEmpty() && nMaxResult > 0);

    m_nMaxResult = nMaxResult;
    m_arrayDocument.clear();
    m_bSearchEnd = false;
    m_timerSearch.start();

    // NOTE: make sure convert keyword to lower case
    return WizFTSSearchDocument(m_strIndexPath.toStdWString().c_str(), \
                                strKeywords.toLower().toStdWString().c_str(), \
                                this);
}

bool CWizSearchIndexer::onSearchProcess(const wchar_t* lpszKbGUID, const wchar_t* lpszDocumentID, const wchar_t* lpszURL)
{
    Q_UNUSED(lpszKbGUID);
    Q_UNUSED(lpszURL);

    QString strKbGUID = QString::fromStdWString(lpszKbGUID);
    QString strGUID = QString::fromStdWString(lpszDocumentID);

    WIZDOCUMENTDATA doc;
    m_dbMgr.db(strKbGUID).DocumentFromGUID(strGUID, doc);
    m_arrayDocument.push_back(doc);

    return true;
}

bool CWizSearchIndexer::onSearchEnd()
{
    m_bSearchEnd = true;
    return true;
}

void CWizSearchIndexer::on_searchTimeout()
{
    int total = m_arrayDocument.size();
    if (!total && m_bSearchEnd) {
        m_timerSearch.stop();
        return;
    }

    CWizDocumentDataArray docs;
    if (total >= m_nMaxResult) {
        docs.assign(m_arrayDocument.begin(), m_arrayDocument.begin() + m_nMaxResult);
        m_arrayDocument.erase(m_arrayDocument.begin(), m_arrayDocument.begin() + m_nMaxResult);
    } else {
        docs.assign(m_arrayDocument.begin(), m_arrayDocument.end());
        m_arrayDocument.clear();
    }

    Q_EMIT documentFind(docs);
}

void CWizSearchIndexer::on_document_created(const WIZDOCUMENTDATA& doc)
{
    updateDocument(doc);
}

void CWizSearchIndexer::on_document_modified(const WIZDOCUMENTDATA& docOld, \
                                            const WIZDOCUMENTDATA& docNew)
{
    Q_ASSERT(docOld.strGUID == docNew.strGUID);

    updateDocument(docNew);
}

void CWizSearchIndexer::on_documentData_modified(const WIZDOCUMENTDATA& doc)
{
    updateDocument(doc);
}

void CWizSearchIndexer::on_document_deleted(const WIZDOCUMENTDATA& doc)
{
    deleteDocument(doc);
}

void CWizSearchIndexer::on_attachment_created(const WIZDOCUMENTATTACHMENTDATA& attach)
{
    Q_UNUSED(attach);
}

void CWizSearchIndexer::on_attachment_modified(const WIZDOCUMENTATTACHMENTDATA& attachOld, \
                                               const WIZDOCUMENTATTACHMENTDATA& attachNew)
{
    Q_UNUSED(attachOld);
    Q_UNUSED(attachNew);
}

void CWizSearchIndexer::on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA& attach)
{
    Q_UNUSED(attach);
}
