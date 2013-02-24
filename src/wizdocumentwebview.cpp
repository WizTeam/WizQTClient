#include "wizdocumentwebview.h"

#include <QtGui>
#include <QtWebKit>

#ifdef BUILD_WITH_QT5
#include <QtWebKitWidgets>
#endif

#include <QMessageBox>

#include "share/wizmisc.h"
#include "wizmainwindow.h"

CWizDocumentWebView::CWizDocumentWebView(CWizExplorerApp& app, QWidget* parent /*= 0*/)
    : QWebView(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_bEditorInited(false)
{
    setAcceptDrops(false);

    settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
    settings()->setAttribute(QWebSettings::PluginsEnabled, true);

    setAttribute(Qt::WA_InputMethodEnabled);
    connect(this, SIGNAL(selectionChanged()), SLOT(onSelectionChanged()));

    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());

    m_cipherDialog = mainWindow->cipherForm();
    connect(m_cipherDialog, SIGNAL(accepted()), SLOT(onCipherDialogClosed()));

    m_downloadDialog = mainWindow->objectDownloadDialog();
    connect(m_downloadDialog, SIGNAL(finished(int)), SLOT(onDownloadDialogClosed(int)));

    m_renderer = new CWizDocumentWebViewRenderer(m_app);
    connect(m_renderer, SIGNAL(documentReady(const QString&)), SLOT(on_documentReady(const QString&)));
    connect(m_renderer, SIGNAL(documentSaved(bool)), SLOT(on_documentSaved(bool)));

    QThread* thread = new QThread();
    m_renderer->moveToThread(thread);
    thread->start();

    m_timerAutoSave.setInterval(10*1000); // 5 minutes
    connect(&m_timerAutoSave, SIGNAL(timeout()), SLOT(onTimerAutoSaveTimout()));
}

void CWizDocumentWebView::onTimerAutoSaveTimout()
{
    saveDocument(false);
}

void CWizDocumentWebView::on_documentReady(const QString& strFileName)
{
    m_strHtmlFileName = strFileName;

    if (m_bEditorInited) {
        viewDocumentInEditor(m_bEditingMode);
    } else {
        initEditorAndLoadDocument();
    }
}

void CWizDocumentWebView::on_documentSaved(bool ok)
{
    if (!ok) {
        TOLOG("Save document failed");
    }

    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    mainWindow->SetSavingDocument(false);
}


// This is bug of QWebView: can't receive input method commit event
void CWizDocumentWebView::inputMethodEvent(QInputMethodEvent* event)
{
    if (!event->commitString().isEmpty()) {
        // indirect set modified flag
        MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
        mainWindow->SetDocumentModified(true);
    }

    update();
    QWebView::inputMethodEvent(event);
}

// This also maybe a bug inside QWebView, QWebView did not update on Mac
void CWizDocumentWebView::keyPressEvent(QKeyEvent* event)
{
    update();
    QWebView::keyPressEvent(event);
}

// This is bug of UEditor, UEditor will insert a placeholder in the cursor position when focus out
// We just don't tell it focus out event.
void CWizDocumentWebView::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);

    return;
}

void CWizDocumentWebView::onSelectionChanged()
{
    update();
}

void CWizDocumentWebView::viewDocument(const WIZDOCUMENTDATA& doc, bool editing)
{
    // clear gui
    MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
    window->showClient(false);

    // save document
    if (m_bEditorInited) {
        saveDocument(false);
    }

    // set data
    m_renderer->setData(doc);
    m_bEditingMode = editing;

    // download document if not exist
    CWizDatabase& db = m_dbMgr.db(doc.strKbGUID);
    QString strDocumentFileName = db.GetDocumentFileName(doc.strGUID);
    if (!db.IsObjectDataDownloaded(doc.strGUID, "document") || \
            !PathFileExists(strDocumentFileName)) {
        MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
        window->showClient(false);

        m_downloadDialog->downloadData(doc);

        return;
    }

    // ask user cipher if needed
    if (doc.nProtected) {
        if(!db.loadUserCert()) {
            return;
        }

        if (db.userCipher().isEmpty()) {
            MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
            window->showClient(false);

            m_cipherDialog->setHint(db.userCipherHint());
            m_cipherDialog->sheetShow();

            return;
        }
    }

    // ask extract and load
    m_renderer->load();
}

void CWizDocumentWebView::onCipherDialogClosed()
{
    CWizDatabase& db = m_dbMgr.db(document().strKbGUID);

    db.setUserCipher(m_cipherDialog->userCipher());
    db.setSaveUserCipher(m_cipherDialog->isSaveForSession());

    m_renderer->load();
}

void CWizDocumentWebView::onDownloadDialogClosed(int result)
{
    if (result == QDialog::Rejected) {
        return;
    }

    m_renderer->load();
}

void CWizDocumentWebView::reloadDocument()
{
    //Q_ASSERT(!m_data.strGUID.isEmpty());
//
    //CWizDatabase& db = m_dbMgr.db(m_data.strKbGUID);
    //db.DocumentFromGUID(m_data.strGUID, m_data);
}

void CWizDocumentWebView::updateSize()
{
    // show & hide ueditor toolbar cause UI issue, force update.
    QRect rc = geometry();
    setGeometry(rc.adjusted(0, 0, 0, 10));
}

void CWizDocumentWebView::initEditorAndLoadDocument()
{
    if (m_bEditorInited)
        return;

    QString strFileName = WizGetResourcesPath() + "files/editor/index.html";
    QString strHtml;
    ::WizLoadUnicodeTextFromFile(strFileName, strHtml);
    QUrl url = QUrl::fromLocalFile(strFileName);

    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), \
            SLOT(on_editor_populateJavaScriptWindowObject()));

    connect(page()->mainFrame(), SIGNAL(loadFinished(bool)), \
            SLOT(on_editor_loadFinished(bool)));

    connect(page(), SIGNAL(linkClicked(const QUrl&)), \
            SLOT(on_editor_linkClicked(const QUrl&)));

    page()->mainFrame()->setHtml(strHtml, url);
}

void CWizDocumentWebView::on_editor_populateJavaScriptWindowObject()
{
    page()->mainFrame()->addToJavaScriptWindowObject("WizExplorerApp", m_app.object());
}

void CWizDocumentWebView::on_editor_loadFinished(bool ok)
{
    if (!ok) {
        TOLOG("Wow, loading editor failed!");
        return;
    }

    m_bEditorInited = true;
    viewDocumentInEditor(m_bEditingMode);
}

void CWizDocumentWebView::on_editor_linkClicked(const QUrl& url)
{
    QDesktopServices::openUrl(url);
}

void CWizDocumentWebView::viewDocumentInEditor(bool editing)
{
    Q_ASSERT(m_bEditorInited);
    //Q_ASSERT(!m_data.strGUID.isEmpty());
    Q_ASSERT(!m_strHtmlFileName.isEmpty());

    QString strScript = ("viewDocument('%1', '%2', %3);");
    strScript = strScript.arg(document().strGUID, m_strHtmlFileName,
                              editing ? "true" : "false");

    bool ret = page()->mainFrame()->evaluateJavaScript(strScript).toBool();

    if (ret) {
        MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
        window->showClient(true);
    }


    m_timerAutoSave.start();

    updateSize();
}

void CWizDocumentWebView::setEditingDocument(bool editing)
{
    Q_ASSERT(m_bEditorInited);

    bool bEditing = page()->mainFrame()->evaluateJavaScript("isEdting();").toBool();
    if (bEditing) {
        saveDocument(false);
    }

    QString strScript = QString("setEditing(%1);").arg(editing ? "true" : "false");
    page()->mainFrame()->evaluateJavaScript(strScript);

    updateSize();
}

void CWizDocumentWebView::saveDocument(bool force)
{
    if (!m_bEditorInited)
        return;

    if (!force && !m_bModified)
        return;

    // Must reset modified flag to avoid switching whiling saving issue!
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    mainWindow->SetDocumentModified(false);
    mainWindow->SetSavingDocument(true);

    QString strHtml = page()->mainFrame()->evaluateJavaScript("getEditorHtml();").toString();
    m_renderer->save(document(), strHtml, m_strHtmlFileName, 0);
}




CWizDocumentWebViewRenderer::CWizDocumentWebViewRenderer(CWizExplorerApp& app)
    : m_app(app)
    , m_dbMgr(app.databaseManager())
{

}

void CWizDocumentWebViewRenderer::setData(const WIZDOCUMENTDATA& doc)
{
    m_data = doc;
}

void CWizDocumentWebViewRenderer::load()
{
    if (!QMetaObject::invokeMethod(this, "viewDocumentImpl")) {
        TOLOG("Invoke viewDocumentImpl failed");
    }
}

void CWizDocumentWebViewRenderer::viewDocumentImpl()
{
    CWizDatabase& db = m_dbMgr.db(m_data.strKbGUID);

    QString strHtmlFileName;
    if (!db.DocumentToTempHtmlFile(m_data, strHtmlFileName)) {
        return;
    }

    Q_EMIT documentReady(strHtmlFileName);
}

void CWizDocumentWebViewRenderer::save(const WIZDOCUMENTDATA& data,
                                       const QString& strHtml,
                                       const QString& strHtmlFile,
                                       int nFlags)
{
    if (!QMetaObject::invokeMethod(this, "saveDocument",
                                   Q_ARG(QString, data.strKbGUID),
                                   Q_ARG(QString, data.strGUID),
                                   Q_ARG(QString, strHtml),
                                   Q_ARG(QString, strHtmlFile),
                                   Q_ARG(int, nFlags))) {
        TOLOG("Invoke saveDocument failed");
    }
}

void CWizDocumentWebViewRenderer::saveDocument(QString strKbGUID, QString strGUID,
                                               QString strHtml, QString strHtmlFile, int nFlags)
{
    WIZDOCUMENTDATA data;
    if (!m_dbMgr.db(strKbGUID).DocumentFromGUID(strGUID, data)) {
        Q_EMIT documentSaved(false);
        return;
    }

    bool ret = m_dbMgr.db(data.strKbGUID).UpdateDocumentData(data, strHtml, strHtmlFile, nFlags);

    Q_EMIT documentSaved(ret);
}
