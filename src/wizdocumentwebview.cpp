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
    , m_db(app.database())
    , m_bEditorInited(false)
{
    setAcceptDrops(false);

    settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
    settings()->setAttribute(QWebSettings::PluginsEnabled, true);

    setAttribute(Qt::WA_InputMethodEnabled);
    connect(this, SIGNAL(selectionChanged()), SLOT(onSelectionChanged()));

    // FIXME: stub here
    m_timerAutoSave.setInterval(5*60*1000);
    connect(&m_timerAutoSave, SIGNAL(timeout()), SLOT(onTimerAutoSaveTimout()));

    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    m_cipherDialog = mainWindow->cipherForm();
    connect(m_cipherDialog, SIGNAL(accepted()), SLOT(onCipherDialogClosed()));

    m_downloadDialog = mainWindow->objectDownloadDialog();
    connect(m_downloadDialog, SIGNAL(finished(int)), SLOT(onDownloadDialogClosed(int)));
}

// This is bug of QWebView: can't receive input method commit event
void CWizDocumentWebView::inputMethodEvent(QInputMethodEvent* event)
{
    if (!event->commitString().isEmpty()) {
        QString strScript("setModified(true);");
        page()->mainFrame()->evaluateJavaScript(strScript);
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

void CWizDocumentWebView::onTimerAutoSaveTimout()
{
    saveDocument(false);
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

    connect(this, SIGNAL(linkClicked(const QUrl&)), \
            SLOT(on_editor_linkClicked(const QUrl&)));

    setHtml(strHtml, url);
}

bool CWizDocumentWebView::saveDocument(bool force)
{
    Q_ASSERT(m_bEditorInited);

    QString strScript("saveDocument(%1);");
    strScript = strScript.arg(force ? "true" : "false");
    return page()->mainFrame()->evaluateJavaScript(strScript).toBool();
}

bool CWizDocumentWebView::viewDocument(const WIZDOCUMENTDATA& doc, bool editing)
{
    m_cipherDialog->hide();

    if (m_bEditorInited) {
        if (!saveDocument(false))
            return false;
    }

    m_data = doc;
    m_bEditingMode = editing;

    QString strDocumentFileName = m_db.GetDocumentFileName(doc.strGUID);
    if (!m_db.IsObjectDataDownloaded(doc.strGUID, "document") || \
            !PathFileExists(strDocumentFileName)) {
        MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
        window->showClient(false);

        m_downloadDialog->downloadData(doc);

        return false;
    }

    if (doc.nProtected) {
        if(!m_db.loadUserCert()) {
            return false;
        }

        if (m_db.userCipher().isEmpty()) {
            MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
            window->showClient(false);

            m_cipherDialog->setHint(m_db.userCipherHint());
            m_cipherDialog->sheetShow();

            return false;
        }
    }

    return viewDocumentImpl();
}

bool CWizDocumentWebView::viewDocumentImpl()
{
    QString strHtmlFileName;
    if (!m_db.DocumentToTempHtmlFile(m_data, strHtmlFileName)) {
        return false;
    }

    m_strHtmlFileName = strHtmlFileName;

    if (m_bEditorInited) {
        return viewDocumentInEditor(m_bEditingMode);
    } else {
        initEditorAndLoadDocument();
        return true;
    }
}

void CWizDocumentWebView::onCipherDialogClosed()
{
    m_db.setUserCipher(m_cipherDialog->userCipher());
    m_db.setSaveUserCipher(m_cipherDialog->isSaveForSession());

    viewDocumentImpl();
}

void CWizDocumentWebView::onDownloadDialogClosed(int result)
{
    if (result == QDialog::Rejected) {
        return;
    }

    viewDocument(m_data, m_bEditingMode);
}

void CWizDocumentWebView::setEditingDocument(bool editing)
{
    Q_ASSERT(m_bEditorInited);

    CString strScript = CString("setEditing(%1);").arg(editing ? "true" : "false");
    page()->mainFrame()->evaluateJavaScript(strScript).toBool();

    if (editing) {
        setFocus();
    }

    updateSize();
}

void CWizDocumentWebView::reloadDocument()
{
    Q_ASSERT(!m_data.strGUID.isEmpty());

    m_db.DocumentFromGUID(m_data.strGUID, m_data);
}

bool CWizDocumentWebView::viewDocumentInEditor(bool editing)
{
    Q_ASSERT(m_bEditorInited);
    Q_ASSERT(!m_data.strGUID.isEmpty());
    Q_ASSERT(!m_strHtmlFileName.isEmpty());

    QString strScript = ("viewDocument('%1', '%2', %3);");
    strScript = strScript.arg(m_data.strGUID,
                              m_strHtmlFileName,
                              editing ? "true" : "false");

    bool ret = page()->mainFrame()->evaluateJavaScript(strScript).toBool();

    if (ret) {
        MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
        window->showClient(true);
    }

    if (editing) {
        setFocus();
    }

    updateSize();

    m_timerAutoSave.start();

    return ret;
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

void CWizDocumentWebView::updateSize()
{
    // show & hide ueditor toolbar cause UI issue, force update.
    QRect rc = geometry();
    setGeometry(rc.adjusted(0, 0, 0, 10));
}
