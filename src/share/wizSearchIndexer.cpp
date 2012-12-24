#include "wizSearchIndexer.h"

#include <QFile>

CWizSearchIndexer::CWizSearchIndexer(CWizDatabase& db, QObject *parent)
    : m_db(db)
    , QObject(parent)
{
    m_timerSearch.setInterval(100);
    connect(&m_timerSearch, SIGNAL(timeout()), SLOT(on_searchTimeout()));

    m_strIndexPath = m_db.GetAccountDataPath() + "fts_index";

    connect(&m_db, SIGNAL(documentCreated(const WIZDOCUMENTDATA&)), \
            SLOT(on_document_created(const WIZDOCUMENTDATA&)));
    connect(&m_db, SIGNAL(documentModified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)), \
            SLOT(on_document_modified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)));
    connect(&m_db, SIGNAL(documentDataModified(const WIZDOCUMENTDATA&)), \
            SLOT(on_documentData_modified(const WIZDOCUMENTDATA&)));
    connect(&m_db, SIGNAL(documentDeleted(const WIZDOCUMENTDATA&)), \
            SLOT(on_document_deleted(const WIZDOCUMENTDATA&)));

    connect(&m_db, SIGNAL(attachmentCreated(const WIZDOCUMENTATTACHMENTDATA&)), \
            SLOT(on_attachment_created(const WIZDOCUMENTATTACHMENTDATA&)));
    connect(&m_db, SIGNAL(attachmentModified(const WIZDOCUMENTATTACHMENTDATA&, const WIZDOCUMENTATTACHMENTDATA&)), \
            SLOT(on_attachment_modified(const WIZDOCUMENTATTACHMENTDATA&, const WIZDOCUMENTATTACHMENTDATA&)));
    connect(&m_db, SIGNAL(attachmentDeleted(const WIZDOCUMENTATTACHMENTDATA&)), \
            SLOT(on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA&)));
}

bool CWizSearchIndexer::_initKbGUID()
{
    if (m_strKbGUID.isEmpty()) {
        WIZUSERINFO userInfo;
        m_db.getUserInfo(userInfo);
        if (userInfo.strKbGUID.isEmpty())
            return false;

        m_strKbGUID = userInfo.strKbGUID;
    }

    return true;
}

bool CWizSearchIndexer::buildFTSIndex()
{
    CWizDocumentDataArray arrayDocuments;
    if (!m_db.getAllDocumentsNeedToBeSearchIndexed(arrayDocuments))
        return false;

    if (arrayDocuments.empty()) {
        return true;
    }

    TOLOG(tr("Build FTS index begin"));

    bool ret = updateDocuments(arrayDocuments);

    if (ret) {
        m_db.setDocumentFTSEnabled(true);
        TOLOG(tr("Build FTS index end successfully"));
        return true;
    } else {
        m_db.setDocumentFTSEnabled(false);
        TOLOG(tr("Build FTS index meet error, we'll rebuild it when restart"));
        return false;
    }
}

bool CWizSearchIndexer::rebuildFTSIndex()
{
    if (!::WizDeleteAllFilesInFolder(m_strIndexPath)) {
        TOLOG("Can't delete old index files while rebuild FTS index");
        return false;
    }

    if (!m_db.setAllDocumentsSearchIndexed(false)) {
        return false;
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

    if (!_initKbGUID()) {
        TOLOG("init KbGUID failed while update FTS index");
        return false;
    }

    void* pHandle = NULL;
    bool ret = WizFTSBeginUpdateDocument(m_strIndexPath.toStdWString().c_str(), &pHandle);
    if (!ret) {
        TOLOG("begin update failed while update FTS index");
        return false;
    }

    bool r = true;
    for (int i = 0; i < arrayDocuments.size(); i++) {
        WIZDOCUMENTDATAEX doc = arrayDocuments.at(i);
        ret = _updateDocumentImpl(pHandle, doc);
        if (!ret) {
            TOLOG("update FTS index failed: " + doc.strTitle);
            r = false;
        }
    }

    ret = WizFTSEndUpdateDocument(pHandle);
    if (!ret) {
        TOLOG("end update failed while update FTS index");
        return false;
    }

    return r;
}

bool CWizSearchIndexer::_updateDocumentImpl(void *pHandle, const WIZDOCUMENTDATAEX& doc)
{
    m_db.setDocumentSearchIndexed(doc.strGUID, false);

    QString strFileName = m_db.GetDocumentFileName(doc.strGUID);

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
    if (!m_db.DocumentToTempHtmlFile(doc, strDataFile)) {
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

    bool ret = WizFTSUpdateDocument(pHandle, \
                                    m_strKbGUID.toStdWString().c_str(), \
                                    doc.strGUID.toStdWString().c_str(), \
                                    doc.strTitle.toStdWString().c_str(), \
                                    strPlainText.toStdWString().c_str());

    if (ret) {
        m_db.setDocumentSearchIndexed(doc.strGUID, true);
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
    m_arrayGUIDs.clear();
    m_bSearchEnd = false;
    m_timerSearch.start();

    return WizFTSSearchDocument(m_strIndexPath.toStdWString().c_str(), \
                                strKeywords.toStdWString().c_str(), \
                                this);
}

bool CWizSearchIndexer::onSearchProcess(const wchar_t* lpszKbGUID, const wchar_t* lpszDocumentID, const wchar_t* lpszURL)
{
    Q_UNUSED(lpszKbGUID);
    Q_UNUSED(lpszURL);

    m_arrayGUIDs.push_back(QString::fromStdWString(lpszDocumentID));
    return true;
}

bool CWizSearchIndexer::onSearchEnd()
{
    m_bSearchEnd = true;
    return true;
}

void CWizSearchIndexer::on_searchTimeout()
{
    int total = m_arrayGUIDs.size();
    if (!total && m_bSearchEnd) {
        m_timerSearch.stop();
        return;
    }

    CWizStdStringArray arrayGUIDs;
    if (total >= m_nMaxResult) {
        arrayGUIDs.assign(m_arrayGUIDs.begin(), m_arrayGUIDs.begin() + m_nMaxResult);
        m_arrayGUIDs.erase(m_arrayGUIDs.begin(), m_arrayGUIDs.begin() + m_nMaxResult);
    } else {
        arrayGUIDs.assign(m_arrayGUIDs.begin(), m_arrayGUIDs.end());
        m_arrayGUIDs.clear();
    }

    CWizDocumentDataArray documentArray;
    m_db.GetDocumentsByGUIDs(arrayGUIDs, documentArray);
    Q_EMIT documentFind(documentArray);
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
