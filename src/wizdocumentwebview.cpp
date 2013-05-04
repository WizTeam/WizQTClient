#include "wizdocumentwebview.h"

#include <QtGui>
#include <QtWebKit>

#ifdef BUILD_WITH_QT5
#include <QtWebKitWidgets>
#endif

#include "share/wizmisc.h"
#include "wizmainwindow.h"

#include "wizEditorInsertLinkForm.h"
#include "wizEditorInsertTableForm.h"

CWizDocumentWebView::CWizDocumentWebView(CWizExplorerApp& app, QWidget* parent /*= 0*/)
    : QWebView(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
{
    setAcceptDrops(false);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            SLOT(on_editor_customContextMenuRequested(const QPoint&)));

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

    m_bEditorInited = false;
    initEditorAndLoadDocument();

//    if (m_bEditorInited) {
//        viewDocumentInEditor(m_bEditingMode);
//    } else {
//        initEditorAndLoadDocument();
//    }
}

void CWizDocumentWebView::on_documentSaved(bool ok)
{
    if (!ok) {
        TOLOG("Save document failed");
    }

    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    mainWindow->SetSavingDocument(false);
}

//void CWizDocumentWebView::keyPressEvent(QKeyEvent* event)
//{
//    if (event->modifiers().testFlag(Qt::ControlModifier)) {
//        // UE bug: contentChange event not emited when cut
//        if(event->key() == Qt::Key_X) {
//            MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
//            mainWindow->SetDocumentModified(true);
//        } else if (event->key() == Qt::Key_V) {
//            QClipboard *clipboard = QApplication::clipboard();
//            const QMimeData *mimeData = clipboard->mimeData();

//            if (mimeData->hasImage()) {
//                QImage image = qvariant_cast<QImage>(mimeData->imageData());
//                QString strFileName = WizGlobal()->GetTempPath() + WizIntToStr(GetTickCount()) + ".png";
//                if (!image.save(strFileName)) {
//                    TOLOG("ERROR: Can't save image from clipboard");
//                    return;
//                }

//                QString strHtml = "<img border=\"0\", src=\"file:///" + strFileName + "\" />";
//                editorCommandExecuteInsertHtml(strHtml, true);

//            } else if (mimeData->hasHtml()) {
//                editorCommandExecuteInsertHtml(mimeData->html(), false);
//            } else if (mimeData->hasText()) {
//                editorCommandExecuteInsertHtml(mimeData->text(), false);
//            } else {
//                TOLOG("WARNING: Can't paste from clipboard");
//            }

//            return;
//        }
//    }

//    QWebView::keyPressEvent(event);
//}

// QWebView on mac have render issue
void CWizDocumentWebView::onSelectionChanged()
{
    update();
}

// UEditor can't receive all event like input method commit string change, enter key event etc.
void CWizDocumentWebView::on_pageContentsChanged()
{
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    mainWindow->SetDocumentModified(true);
}

void CWizDocumentWebView::viewDocument(const WIZDOCUMENTDATA& doc, bool editing)
{
    // clear gui
    MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());

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

void CWizDocumentWebView::initEditorStyle()
{
    QString strFont = m_app.userSettings().defaultFontFamily();
    int nSize = m_app.userSettings().defaultFontSize();

    QString strExec = QString("editor.options.initialStyle = 'body{font-family:%1; font-size:%2px;}';")
            .arg(strFont).arg(nSize);
    page()->mainFrame()->evaluateJavaScript(strExec).toString();
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
        m_bEditorInited = false;
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

void CWizDocumentWebView::on_editor_customContextMenuRequested(const QPoint& pos)
{
    if (!m_bEditorInited)
        return;

    MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
    window->ResetContextMenuAndPop(mapToGlobal(pos));
}

void CWizDocumentWebView::viewDocumentInEditor(bool editing)
{
    Q_ASSERT(m_bEditorInited);
    //Q_ASSERT(!m_data.strGUID.isEmpty());
    Q_ASSERT(!m_strHtmlFileName.isEmpty());

    disconnect(page(), SIGNAL(contentsChanged()), this, SLOT(on_pageContentsChanged()));

    QString strScript = QString("viewDocument('%1', '%2', %3);")
            .arg(document().strGUID)
            .arg(m_strHtmlFileName)
            .arg(editing ? "true" : "false");
    bool ret = page()->mainFrame()->evaluateJavaScript(strScript).toBool();

    connect(page(), SIGNAL(contentsChanged()), SLOT(on_pageContentsChanged()));

    MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
    if (!ret) {
        window->showClient(false);
        return;
    }

    window->showClient(true);
    m_timerAutoSave.start();

    update();
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
}

void CWizDocumentWebView::saveDocument(bool force)
{
    if (!m_bEditorInited)
        return;

    if (!force && !m_bModified)
        return;

    // check note permission
    int perm = m_dbMgr.db(document().strKbGUID).permission();
    QString strUserId = m_dbMgr.db().getUserId();
    if (perm > WIZ_USERGROUP_AUTHOR ||
            (perm == WIZ_USERGROUP_AUTHOR && document().strOwner != strUserId)) {
        return;
    }

    // Must reset modified flag to avoid switching whiling saving issue!
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    mainWindow->SetDocumentModified(false);
    mainWindow->SetSavingDocument(true);

    QString strHtml = page()->mainFrame()->evaluateJavaScript("getEditorHtml();").toString();
    m_renderer->save(document(), strHtml, m_strHtmlFileName, 0);
}

void CWizDocumentWebView::editorCommandExecuteCut()
{
    triggerPageAction(QWebPage::Cut);
}

void CWizDocumentWebView::editorCommandExecuteCopy()
{
    triggerPageAction(QWebPage::Copy);
}

void CWizDocumentWebView::editorCommandExecutePaste()
{
    triggerPageAction(QWebPage::Paste);
}

void CWizDocumentWebView::editorSetFullScreen()
{
    QString strExec = "editor.ui.setFullScreen(true);";
    page()->mainFrame()->evaluateJavaScript(strExec);
}

QString CWizDocumentWebView::editorCommandQueryCommandValue(const QString& strCommand)
{
    QString strExec = "editor.queryCommandValue('" + strCommand +"');";
    return page()->mainFrame()->evaluateJavaScript(strExec).toString();
}

int CWizDocumentWebView::editorCommandQueryCommandState(const QString& strCommand)
{
    QString strExec = "editor.queryCommandState('" + strCommand +"');";
    return page()->mainFrame()->evaluateJavaScript(strExec).toInt();
}

bool CWizDocumentWebView::editorCommandExecuteCommand(const QString& strCommand,
                                                      const QString& arg1 /* = QString() */,
                                                      const QString& arg2 /* = QString() */,
                                                      const QString& arg3 /* = QString() */)
{
    QString strExec = QString("editor.execCommand('%1'").arg(strCommand);
    if (!arg1.isEmpty()) {
        strExec += ", " + arg1;
    }

    if (!arg2.isEmpty()) {
        strExec += ", " + arg2;
    }

    if (!arg3.isEmpty()) {
        strExec += ", " + arg3;
    }

    strExec += ");";

    return page()->mainFrame()->evaluateJavaScript(strExec).toBool();
}

bool CWizDocumentWebView::editorCommandQueryLink()
{
    QString strUrl = page()->mainFrame()->evaluateJavaScript("WizGetLinkUrl();").toString();
    if (strUrl.isEmpty())
        return false;

    return true;
}

bool CWizDocumentWebView::editorCommandExecuteInsertHtml(const QString& strHtml, bool bNotSerialize)
{
    QString s = bNotSerialize ? "true" : "false";
    QString strExec = QString("editor.execCommand('insertHtml', '%1', %2);").arg(strHtml).arg(s);
    return page()->mainFrame()->evaluateJavaScript(strExec).toBool();
}

bool CWizDocumentWebView::editorCommandExecuteUndo()
{
    return editorCommandExecuteCommand("undo");
}

bool CWizDocumentWebView::editorCommandExecuteRedo()
{
    return editorCommandExecuteCommand("redo");
}

bool CWizDocumentWebView::editorCommandExecuteIndent()
{
    return editorCommandExecuteCommand("indent");
}

bool CWizDocumentWebView::editorCommandExecuteOutdent()
{
    return editorCommandExecuteCommand("outdent");
}

bool CWizDocumentWebView::editorCommandExecuteLinkInsert()
{
    if (!m_editorInsertLinkForm) {
        m_editorInsertLinkForm = new CWizEditorInsertLinkForm(this);
        connect(m_editorInsertLinkForm, SIGNAL(accepted()), SLOT(on_editorCommandExecuteLinkInsert_accepted()));
    }

    QString strUrl = page()->mainFrame()->evaluateJavaScript("WizGetLinkUrl();").toString();
    m_editorInsertLinkForm->setUrl(strUrl);
    m_editorInsertLinkForm->open();

    return true;
}

void CWizDocumentWebView::on_editorCommandExecuteLinkInsert_accepted()
{
    // append http if not exist
    QString strUrl = m_editorInsertLinkForm->getUrl();
    if (strUrl.lastIndexOf("http://", 0, Qt::CaseInsensitive) == -1)
        strUrl = "http://" + strUrl;

    QString strScript = QString("editor.execCommand('link', {href: '%1'});").arg(strUrl);
    page()->mainFrame()->evaluateJavaScript(strScript).toBool();
}

bool CWizDocumentWebView::editorCommandExecuteLinkRemove()
{
    return editorCommandExecuteCommand("unlink");
}

bool CWizDocumentWebView::editorCommandExecuteFontFamily(const QString& strFamily)
{
    QString strExec = "editor.execCommand('fontFamily', '" + strFamily + "');";
    return page()->mainFrame()->evaluateJavaScript(strExec).toBool();
}

bool CWizDocumentWebView::editorCommandExecuteFontSize(const QString& strSize)
{
    QString strExec = "editor.execCommand('fontSize', '" + strSize + "');";
    return page()->mainFrame()->evaluateJavaScript(strExec).toBool();
}

bool CWizDocumentWebView::editorCommandExecuteForeColor()
{
    if (!m_colorDialog) {
        m_colorDialog = new QColorDialog(this);
        connect(m_colorDialog, SIGNAL(currentColorChanged(const QColor &)),
                SLOT(on_editorCommandExecuteForeColor_accepted(const QColor&)));
    }

    m_colorDialog->show();

    return true;
}

void CWizDocumentWebView::on_editorCommandExecuteForeColor_accepted(const QColor& color)
{
    QString strExec = "editor.execCommand('foreColor', '" + color.name() + "');";
    page()->mainFrame()->evaluateJavaScript(strExec).toBool();
}

bool CWizDocumentWebView::editorCommandExecuteBold()
{
    QString strExec = "editor.execCommand('bold');";
    return page()->mainFrame()->evaluateJavaScript(strExec).toBool();
}

bool CWizDocumentWebView::editorCommandExecuteItalic()
{
    QString strExec = "editor.execCommand('italic');";
    return page()->mainFrame()->evaluateJavaScript(strExec).toBool();
}

bool CWizDocumentWebView::editorCommandExecuteUnderLine()
{
    QString strExec = "editor.execCommand('underline');";
    return page()->mainFrame()->evaluateJavaScript(strExec).toBool();
}

bool CWizDocumentWebView::editorCommandExecuteStrikeThrough()
{
    QString strExec = "editor.execCommand('strikethrough');";
    return page()->mainFrame()->evaluateJavaScript(strExec).toBool();
}

bool CWizDocumentWebView::editorCommandExecuteJustifyLeft()
{
    QString strExec = "editor.execCommand('justify', 'left');";
    return page()->mainFrame()->evaluateJavaScript(strExec).toBool();
}

bool CWizDocumentWebView::editorCommandExecuteJustifyRight()
{
    QString strExec = "editor.execCommand('justify', 'right');";
    return page()->mainFrame()->evaluateJavaScript(strExec).toBool();
}

bool CWizDocumentWebView::editorCommandExecuteJustifyCenter()
{
    QString strExec = "editor.execCommand('justify', 'center');";
    return page()->mainFrame()->evaluateJavaScript(strExec).toBool();
}

bool CWizDocumentWebView::editorCommandExecuteJustifyJustify()
{
    QString strExec = "editor.execCommand('justify', 'justify');";
    return page()->mainFrame()->evaluateJavaScript(strExec).toBool();
}

bool CWizDocumentWebView::editorCommandExecuteInsertOrderedList()
{
    QString strExec = "editor.execCommand('insertOrderedList');";
    return page()->mainFrame()->evaluateJavaScript(strExec).toBool();
}

bool CWizDocumentWebView::editorCommandExecuteInsertUnorderedList()
{
    QString strExec = "editor.execCommand('insertUnorderedList');";
    return page()->mainFrame()->evaluateJavaScript(strExec).toBool();
}

bool CWizDocumentWebView::editorCommandExecuteTableInsert()
{
    if (!m_editorInsertTableForm) {
        m_editorInsertTableForm = new CWizEditorInsertTableForm(this);
        connect(m_editorInsertTableForm, SIGNAL(accepted()), SLOT(on_editorCommandExecuteTableInsert_accepted()));
    }

    m_editorInsertTableForm->clear();
    m_editorInsertTableForm->open();

    return true;
}

void CWizDocumentWebView::on_editorCommandExecuteTableInsert_accepted()
{
    int nRows = m_editorInsertTableForm->getRows();
    int nCols = m_editorInsertTableForm->getCols();

    if (!nRows && !nCols)
        return;

    QString strExec = QString("editor.execCommand('insertTable', {numRows:%1, numCols:%2, border:1});")
            .arg(nRows)
            .arg(nCols);

    page()->mainFrame()->evaluateJavaScript(strExec).toBool();
}

bool CWizDocumentWebView::editorCommandExecuteInsertHorizontal()
{
    QString strExec = "editor.execCommand('horizontal');";
    return page()->mainFrame()->evaluateJavaScript(strExec).toBool();
}

bool CWizDocumentWebView::editorCommandExecuteInsertDate()
{
    return page()->mainFrame()->evaluateJavaScript("editor.execCommand('date');").toBool();
}

bool CWizDocumentWebView::editorCommandExecuteInsertTime()
{
    return page()->mainFrame()->evaluateJavaScript("editor.execCommand('time');").toBool();
}

bool CWizDocumentWebView::editorCommandExecuteRemoveFormat()
{
    return page()->mainFrame()->evaluateJavaScript("editor.execCommand('removeFormat');").toBool();
}

bool CWizDocumentWebView::editorCommandExecuteFormatMatch()
{
    return page()->mainFrame()->evaluateJavaScript("editor.execCommand('formatMatch');").toBool();
}

bool CWizDocumentWebView::editorCommandExecuteTableDelete()
{
    return editorCommandExecuteCommand("deletetable");
}

bool CWizDocumentWebView::editorCommandExecuteTableDeleteRow()
{
    return editorCommandExecuteCommand("deleterow");
}

bool CWizDocumentWebView::editorCommandExecuteTableDeleteCol()
{
    return editorCommandExecuteCommand("deletecol");
}

bool CWizDocumentWebView::editorCommandExecuteTableInsertRow()
{
    return editorCommandExecuteCommand("insertrow");
}

bool CWizDocumentWebView::editorCommandExecuteTableInsertRowNext()
{
    return editorCommandExecuteCommand("insertrownext");
}

bool CWizDocumentWebView::editorCommandExecuteTableInsertCol()
{
    return editorCommandExecuteCommand("insertcol");
}

bool CWizDocumentWebView::editorCommandExecuteTableInsertColNext()
{
    return editorCommandExecuteCommand("insertcolnext");
}

bool CWizDocumentWebView::editorCommandExecuteTableInsertCaption()
{
    return editorCommandExecuteCommand("insertcaption");
}

bool CWizDocumentWebView::editorCommandExecuteTableDeleteCaption()
{
    return editorCommandExecuteCommand("deletecaption");
}

bool CWizDocumentWebView::editorCommandExecuteTableInsertTitle()
{
    return editorCommandExecuteCommand("inserttitle");
}

bool CWizDocumentWebView::editorCommandExecuteTableDeleteTitle()
{
    return editorCommandExecuteCommand("deletetitle");
}

bool CWizDocumentWebView::editorCommandExecuteTableMergeCells()
{
    return editorCommandExecuteCommand("mergecells");
}

bool CWizDocumentWebView::editorCommandExecuteTalbeMergeRight()
{
    return editorCommandExecuteCommand("mergeright");
}

bool CWizDocumentWebView::editorCommandExecuteTableMergeDown()
{
    return editorCommandExecuteCommand("mergedown");
}

bool CWizDocumentWebView::editorCommandExecuteTableSplitCells()
{
    return editorCommandExecuteCommand("splittocells");
}

bool CWizDocumentWebView::editorCommandExecuteTableSplitRows()
{
    return editorCommandExecuteCommand("splittorows");
}

bool CWizDocumentWebView::editorCommandExecuteTableSplitCols()
{
    return editorCommandExecuteCommand("splittocols");
}

bool CWizDocumentWebView::editorCommandExecuteTableAverageRows()
{
    return editorCommandExecuteCommand("averagedistributerow");
}

bool CWizDocumentWebView::editorCommandExecuteTableAverageCols()
{
    return editorCommandExecuteCommand("averagedistributecol");
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
