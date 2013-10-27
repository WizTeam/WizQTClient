#include "wizDocumentWebView.h"

#include <QApplication>
#include <QWebFrame>

#include "wizdef.h"
#include "share/wizmisc.h"
#include "wizmainwindow.h"
#include "wizEditorInsertLinkForm.h"
#include "wizEditorInsertTableForm.h"
#include "share/wizObjectDataDownloader.h"
#include "wizDocumentTransitionView.h"

void CWizDocumentWebViewPage::triggerAction(QWebPage::WebAction typeAction, bool checked)
{
    if (typeAction == QWebPage::Back || typeAction == QWebPage::Forward) {
        return;
    }

    if (typeAction == QWebPage::Paste) {
        on_editorCommandPaste_triggered();
    }

    QWebPage::triggerAction(typeAction, checked);
}

void CWizDocumentWebViewPage::on_editorCommandPaste_triggered()
{
    QClipboard* clip = QApplication::clipboard();
    Q_ASSERT(clip);

    //const QMimeData* mime = clip->mimeData();
    //qDebug() << mime->formats();
    //qDebug() << mime->data("text/html");
    //qDebug() << mime->hasImage();

    if (!clip->image().isNull()) {
        // save clipboard image to $TMPDIR
        QString strTempPath = WizGlobal()->GetTempPath();
        CString strFileName = strTempPath + WizIntToStr(GetTickCount()) + ".png";
        if (!clip->image().save(strFileName)) {
            TOLOG("ERROR: Can't save clipboard image to file");
            return;
        }

        QMimeData* data = new QMimeData();
        QString strHtml = QString("<img border=\"0\" src=\"file://%1\" />").arg(strFileName);
        data->setHtml(strHtml);
        clip->setMimeData(data);
    }
}

void CWizDocumentWebViewPage::javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID)
{
    Q_UNUSED(sourceID);

    qDebug() << "[Console]line: " << lineNumber << ", " << message;
}


CWizDocumentWebView::CWizDocumentWebView(CWizExplorerApp& app, QWidget* parent /*= 0*/)
    : QWebView(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_bEditorInited(false)
{
    CWizDocumentWebViewPage* page = new CWizDocumentWebViewPage(this);
    setPage(page);

    // minimum page size hint
    setMinimumSize(400, 250);

    // only accept focus by mouse click as the best way to trigger toolbar reset
    setFocusPolicy(Qt::ClickFocus);
    setAttribute(Qt::WA_AcceptTouchEvents, false);

    // FIXME: should accept drop picture, attachment, link etc.
    setAcceptDrops(false);

    // refers
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());

    m_cipherDialog = mainWindow->cipherForm();
    connect(m_cipherDialog, SIGNAL(accepted()), SLOT(onCipherDialogClosed()));

    m_downloaderHost = mainWindow->downloaderHost();
    connect(m_downloaderHost, SIGNAL(downloadDone(const WIZOBJECTDATA&, bool)),
            SLOT(on_download_finished(const WIZOBJECTDATA&, bool)));

    m_transitionView = mainWindow->transitionView();

    // loading and saving thread
    m_timerAutoSave.setInterval(5*60*1000); // 5 minutes
    connect(&m_timerAutoSave, SIGNAL(timeout()), SLOT(onTimerAutoSaveTimout()));

    m_renderer = new CWizDocumentWebViewRenderer(m_app);
    connect(m_renderer, SIGNAL(endLoading(const QString&, bool)), SLOT(on_documentReady(const QString&, bool)));
    connect(m_renderer, SIGNAL(documentSaved(bool)), SLOT(on_documentSaved(bool)));

    QThread* thread = new QThread();
    m_renderer->moveToThread(thread);
    thread->start();
}

void CWizDocumentWebView::inputMethodEvent(QInputMethodEvent* event)
{
    QWebView::inputMethodEvent(event);

    int nLength = 0;
    int nOffset = 0;
    for (int i = 0; i < event->attributes().size(); i++) {
        const QInputMethodEvent::Attribute& a = event->attributes().at(i);
        if (a.type == QInputMethodEvent::Cursor) {
            nLength = a.length;
            nOffset = a.start;
            break;
        }
    }

    // Move cursor
    // because every time input method event triggered, old preedit text will be
    // deleted and the new one inserted, this action made cursor restore to the
    // beginning of the input context, move it as far as offset indicated after
    // default implementation should correct this issue!!!
    for (int i = 0; i < nOffset; i++) {
        page()->triggerAction(QWebPage::MoveToNextChar);
    }
}

void CWizDocumentWebView::keyPressEvent(QKeyEvent* event)
{
    // special cases process
    if (event->key() == Qt::Key_Escape) {
        // FIXME: press esc will insert space at cursor if not clear focus
        clearFocus();
        return;
    }

    QWebView::keyPressEvent(event);
}

void CWizDocumentWebView::focusInEvent(QFocusEvent *event)
{
    if (m_bEditingMode) {
        Q_EMIT focusIn();
        Q_EMIT statusChanged();
    }

    QWebView::focusInEvent(event);
}

void CWizDocumentWebView::focusOutEvent(QFocusEvent *event)
{
    // because qt will clear focus when context menu popup, we need keep focus there.
    if (event->reason() == Qt::PopupFocusReason) {
        return;
    }

    Q_EMIT focusOut();
    Q_EMIT statusChanged();
    QWebView::focusOutEvent(event);
}

void CWizDocumentWebView::contextMenuEvent(QContextMenuEvent *event)
{
    if (!m_bEditorInited)
        return;

    Q_EMIT requestShowContextMenu(mapToGlobal(event->pos()));
}

void CWizDocumentWebView::onTimerAutoSaveTimout()
{
    saveDocument(false);
}

void CWizDocumentWebView::on_documentReady(const QString& strFileName, bool bOk)
{
    Q_UNUSED(bOk);

    // FIXME: deal with encrypted document

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

        m_downloaderHost->download(doc);
        window->showClient(false);
        window->transitionView()->showAsMode(CWizDocumentTransitionView::Downloading);

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

void CWizDocumentWebView::on_download_finished(const WIZOBJECTDATA& data,
                                               bool bSucceed)
{
    if (m_renderer->data().strKbGUID != data.strKbGUID
            || m_renderer->data().strGUID != data.strObjectGUID)
        return;

    if (!bSucceed)
        return;

    m_renderer->load();
}

void CWizDocumentWebView::reloadDocument()
{
    Q_ASSERT(!document().strGUID.isEmpty());

    WIZDOCUMENTDATA data;
    m_dbMgr.db(document().strKbGUID).DocumentFromGUID(document().strGUID, data);

    m_renderer->setData(data);
    m_renderer->load();
}

void CWizDocumentWebView::initEditorStyle()
{
    QString strFont = m_app.userSettings().defaultFontFamily();
    int nSize = m_app.userSettings().defaultFontSize();

    QString strExec = QString("editor.options.initialStyle = 'body{font-family:%1; font-size:%2px;} img{max-width:100%;}';")
            .arg(strFont).arg(nSize);
    page()->mainFrame()->evaluateJavaScript(strExec);
}

void CWizDocumentWebView::editorResetFont()
{
    QString strFont = m_app.userSettings().defaultFontFamily();
    int nSize = m_app.userSettings().defaultFontSize();

    QString strExec = QString("editor.document.body.style.fontFamily='%1';editor.document.body.style.fontSize='%2px';")
            .arg(strFont).arg(nSize);
    page()->mainFrame()->evaluateJavaScript(strExec);
}

void CWizDocumentWebView::editorFocus()
{
    page()->mainFrame()->evaluateJavaScript("editor.focus();");
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

    connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            SLOT(on_editor_populateJavaScriptWindowObject()));

    connect(page()->mainFrame(), SIGNAL(loadFinished(bool)),
            SLOT(on_editor_loadFinished(bool)));

    connect(page(), SIGNAL(linkClicked(const QUrl&)),
            SLOT(on_editor_linkClicked(const QUrl&)));

    connect(page(), SIGNAL(selectionChanged()),
            SLOT(on_editor_selectionChanged()));

    connect(page(), SIGNAL(contentsChanged()),
            SLOT(on_editor_contentChanged()));

    page()->mainFrame()->setHtml(strHtml, url);
}

void CWizDocumentWebView::on_editor_contentChanged()
{
    Q_EMIT statusChanged();
}

void CWizDocumentWebView::on_editor_selectionChanged()
{
    if (hasFocus()) {
        // FIXME: every time change content shuld tell webview to clean the canvas
        update();
    }

    Q_EMIT statusChanged();
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
    if (isInternalUrl(url)) {
        viewDocumentByUrl(url);
        return;
    }

    QDesktopServices::openUrl(url);
}

bool CWizDocumentWebView::isInternalUrl(const QUrl& url)
{
    if (url.scheme().toLower() == "wiz")
        return true;
    return false;
}

bool WizStringList2Map(const QStringList& list, QMap<QString, QString>& map)
{
    for (int i = 0; i < list.size(); i++) {
        int indx = list[i].indexOf("=");
        if (indx == -1) {
            return false;
        }

        qDebug() << "key: " << list[i].left(indx).toLower();
        qDebug() << "value: " << list[i].mid(indx + 1);

        map.insert(list[i].left(indx).toLower(), list[i].mid(indx + 1));
    }

    return true;
}

void CWizDocumentWebView::viewDocumentByUrl(const QUrl& url)
{
    QString strUrl = url.toString();
    if (!strUrl.startsWith("wiz:", Qt::CaseInsensitive)) {
        return;
    }

    int indx = strUrl.indexOf('?');
    if (indx == -1) {
        return;
    }

    QString strOpenType = strUrl.mid(4, indx - 4).toLower();

    QString strFragment = strUrl.mid(indx + 1);
    QMap<QString, QString> mapArgs;
    if (!WizStringList2Map(strFragment.split('&'), mapArgs)) {
        return;
    }

    QString strGUID, strKbGUID;
    if (strOpenType == "open_document") {
        QMap<QString, QString>::const_iterator it = mapArgs.find("guid");
        if (it != mapArgs.end()) {
            strGUID = it.value();
        }

        QMap<QString, QString>::const_iterator it2 = mapArgs.find("kbguid");
        if (it2 != mapArgs.end()) {
            strKbGUID = it2.value();
        }

        if (strGUID.isEmpty()) {
            return;
        }

        WIZDOCUMENTDATA doc;
        if (!m_dbMgr.db(strKbGUID).DocumentFromGUID(strGUID, doc)) {
            qDebug() << "Can't find user document, it maybe deleted!";
            return;
        }

        Q_EMIT requestView(doc);
   }
}

void CWizDocumentWebView::viewDocumentInEditor(bool editing)
{
    Q_ASSERT(m_bEditorInited);

    bool ret = false;
    if (!m_strHtmlFileName.isEmpty()) {
        QString strScript = QString("viewDocument('%1', '%2', %3);")
                .arg(document().strGUID)
                .arg(m_strHtmlFileName)
                .arg(editing ? "true" : "false");
        ret = page()->mainFrame()->evaluateJavaScript(strScript).toBool();
    }

    MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
    if (!ret) {
        window->showClient(false);
        window->transitionView()->showAsMode(CWizDocumentTransitionView::ErrorOccured);
        return;
    }

    window->showClient(true);
    window->transitionView()->hide();
    m_timerAutoSave.start();

    update();
}

void CWizDocumentWebView::setEditingDocument(bool editing)
{
    Q_ASSERT(m_bEditorInited);

    // show editor toolbar properly
    if (!editing && hasFocus()) {
        Q_EMIT focusOut();
    }

    if (editing && hasFocus()) {
        Q_EMIT focusIn();
    }

    //bool bEditing = page()->mainFrame()->evaluateJavaScript("isEditing();").toBool();
    if (m_bEditingMode) {
        saveDocument(false);
    }

    m_bEditingMode = editing;

    QString strScript = QString("setEditing(%1);").arg(editing ? "true" : "false");
    page()->mainFrame()->evaluateJavaScript(strScript);

    Q_EMIT statusChanged();
}

void CWizDocumentWebView::saveDocument(bool force)
{
    if (!m_bEditorInited)
        return;

    if (!force && !page()->isModified())
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
    mainWindow->SetSavingDocument(true);

    QString strHtml = page()->mainFrame()->evaluateJavaScript("editor.getContent();").toString();
    m_renderer->save(document(), strHtml, m_strHtmlFileName, 0);

    page()->undoStack()->clear();
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

void CWizDocumentWebView::editorCommandExecuteBackColor()
{
    if (!m_colorDialog) {
        m_colorDialog = new QColorDialog(this);
    }

    m_colorDialog->disconnect();

    connect(m_colorDialog, SIGNAL(currentColorChanged(const QColor &)),
            SLOT(on_editorCommandExecuteBackColor_accepted(const QColor&)));
    connect(m_colorDialog, SIGNAL(colorSelected(const QColor &)),
            SLOT(on_editorCommandExecuteBackColor_accepted(const QColor&)));

    m_colorDialog->show();
}

void CWizDocumentWebView::on_editorCommandExecuteBackColor_accepted(const QColor& color)
{
    QString strExec = "editor.execCommand('backColor', '" + color.name() + "');";
    page()->mainFrame()->evaluateJavaScript(strExec).toBool();
}

void CWizDocumentWebView::editorCommandExecuteForeColor()
{
    if (!m_colorDialog) {
        m_colorDialog = new QColorDialog(this);
    }

    m_colorDialog->disconnect();
    connect(m_colorDialog, SIGNAL(currentColorChanged(const QColor &)),
            SLOT(on_editorCommandExecuteForeColor_accepted(const QColor&)));
    connect(m_colorDialog, SIGNAL(colorSelected(const QColor &)),
            SLOT(on_editorCommandExecuteForeColor_accepted(const QColor&)));

    m_colorDialog->show();
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

bool CWizDocumentWebView::editorCommandExecuteViewSource()
{
    return page()->mainFrame()->evaluateJavaScript("editor.execCommand('source');").toBool();
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
    Q_EMIT startLoading();

    if (!QMetaObject::invokeMethod(this, "viewDocumentImpl")) {
        TOLOG("Invoke viewDocumentImpl failed");
    }
}

void CWizDocumentWebViewRenderer::viewDocumentImpl()
{
    CWizDatabase& db = m_dbMgr.db(m_data.strKbGUID);

    QString strHtmlFileName;
    bool bOk = db.DocumentToTempHtmlFile(m_data, strHtmlFileName);

    Q_EMIT endLoading(strHtmlFileName, bOk);
}

void CWizDocumentWebViewRenderer::save(const WIZDOCUMENTDATA& data,
                                       const QString& strHtml,
                                       const QString& strHtmlFile,
                                       int nFlags)
{
    Q_EMIT startSaving();

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
