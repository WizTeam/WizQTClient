#include "WizDocumentWebView.h"

#include <QRunnable>
#include <QList>
#include <QMimeData>
#include <QUrl>
#include <QDir>
#include <QString>
#include <QRegExp>
#include <QAction>
#include <QPrinter>
#include <QFileDialog>
#include <QTextEdit>
#include <QMultiMap>
#include <QPrintDialog>
#include <QPrinterInfo>
#include <QMessageBox>

#include <QApplication>
#include <QUndoStack>
#include <QDesktopServices>
#include <QNetworkDiskCache>
#include <QWebEngineProfile>

#ifdef Q_OS_MAC
#include <QMacPasteboardMime>
#endif

#include "share/WizGlobal.h"

#include "WizDef.h"
#include "utils/WizPathResolve.h"
#include "utils/WizLogger.h"
#include "utils/WizMisc.h"
#include "utils/WizStyleHelper.h"

#include "share/WizMisc.h"
#include "share/WizAnalyzer.h"
#include "share/WizMessageBox.h"
#include "share/WizObjectDataDownloader.h"
#include "share/WizDatabaseManager.h"
#include "share/WizThreads.h"
#include "sync/WizAvatarHost.h"
#include "sync/WizToken.h"
#include "sync/WizApiEntry.h"
#include "core/WizAccountManager.h"
#include "core/WizNoteManager.h"
#include "widgets/WizCodeEditorDialog.h"
#include "widgets/WizScreenShotWidget.h"
#include "widgets/WizEmailShareDialog.h"
#include "widgets/WizShareLinkDialog.h"
#include "widgets/WizScrollBar.h"
#include "widgets/WizExecutingActionDialog.h"
#include "mac/WizMacHelper.h"

#include "WizMainWindow.h"
#include "WizEditorInsertLinkForm.h"
#include "WizEditorInsertTableForm.h"
#include "WizDocumentTransitionView.h"
#include "WizDocumentView.h"
#include "WizSearchReplaceWidget.h"
#include "WizSvgEditorDialog.h"


#include "html/WizHtmlReader.h"
#include "WizPlugins.h"

#include "WizTitleBar.h"


enum WizLinkType {
    WizLink_Doucment,
    WizLink_Attachment
};

WizDocumentWebViewPage::WizDocumentWebViewPage(QWebEngineProfile* profile, QWidget* parent)
    : WizWebEnginePage(profile, parent)
    , m_engineView(nullptr)
{
    action(QWebEnginePage::Undo)->setShortcut(QKeySequence());
    action(QWebEnginePage::Redo)->setShortcut(QKeySequence());
    action(QWebEnginePage::Copy)->setShortcut(QKeySequence());
    action(QWebEnginePage::Cut)->setShortcut(QKeySequence());
    action(QWebEnginePage::Paste)->setShortcut(QKeySequence());
    action(QWebEnginePage::SelectAll)->setShortcut(QKeySequence());
}

bool WizDocumentWebViewPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    if (NavigationTypeBackForward == type || NavigationTypeReload == type)
        return false;
    //
    return WizWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}

void WizDocumentWebViewPage::triggerAction(QWebEnginePage::WebAction typeAction, bool checked)
{
    if (typeAction == QWebEnginePage::Back || typeAction == QWebEnginePage::Forward) {
        return;
    }

    if (typeAction == QWebEnginePage::Paste) {
        if (m_engineView->onPasteCommand())
            return;
    } else if (typeAction == QWebEnginePage::Undo || typeAction == QWebEnginePage::Redo) {
        //FIXME: 在QT5.4.2之后无法禁止webpage的快捷键，webpage的快捷键会覆盖menubar上的
        Q_EMIT actionTriggered(typeAction);
        return;
    }

    WizWebEnginePage::triggerAction(typeAction, checked);

    Q_EMIT actionTriggered(typeAction);
}

void WizDocumentWebViewPage::javaScriptConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID)
{
    Q_UNUSED(sourceID);

    qDebug() << "[Console]line: " << lineNumber << ", " << message;
}


static int nWindowIDCounter = 0;

WizDocumentWebView::WizDocumentWebView(WizExplorerApp& app, QWidget* parent)
    : WizWebEngineView(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_bNewNote(false)
    , m_bNewNoteTitleInited(false)
    , m_bContentsChanged(false)
    , m_ignoreActiveWindowEvent(false)
    , m_bInSeperateWindow(false)
    , m_nWindowID(nWindowIDCounter ++)
    , m_searchReplaceWidget(nullptr)
{
    //
    WizWebEngineViewInjectObjects objects = {{"WizExplorerApp", app.object()}, {"WizQtEditor", this}};
    initWebEngineView<WizDocumentWebView, WizDocumentWebViewPage>(this, objects);

    WizDocumentWebViewPage* p = (WizDocumentWebViewPage *)page();
    p->setDocumentWebView(this);
    if (isDarkMode()) {
        p->setBackgroundColor(QColor("#272727"));
    }

    connect(p, SIGNAL(actionTriggered(QWebEnginePage::WebAction)), SLOT(onActionTriggered(QWebEnginePage::WebAction)));
    connect(p, SIGNAL(linkClicked(QUrl,QWebEnginePage::NavigationType,bool,WizWebEnginePage*)), this, SLOT(onEditorLinkClicked(QUrl,QWebEnginePage::NavigationType,bool,WizWebEnginePage*)));
    connect(p, SIGNAL(openLinkInNewWindow(QUrl)), this, SLOT(onOpenLinkInNewWindow(QUrl)));

    // minimum page size hint
    setMinimumSize(400, 250);

    // only accept focus by mouse click as the best way to trigger toolbar reset
    setFocusPolicy(Qt::ClickFocus);
    setAttribute(Qt::WA_AcceptTouchEvents, false);

    // FIXME: should accept drop picture, attachment, link etc.
    setAcceptDrops(true);

    // refers
    qRegisterMetaType<WizEditorMode>("WizEditorMode");
    m_docLoadThread = new WizDocumentWebViewLoaderThread(m_dbMgr, this);
    connect(m_docLoadThread, SIGNAL(loaded(const QString&, const QString, const QString, WizEditorMode)),
            SLOT(onDocumentReady(const QString&, const QString, const QString, WizEditorMode)), Qt::QueuedConnection);
    //
    m_docSaverThread = new WizDocumentWebViewSaverThread(m_dbMgr, this);
    connect(m_docSaverThread, SIGNAL(saved(const QString, const QString,bool)),
            SLOT(onDocumentSaved(const QString, const QString,bool)), Qt::QueuedConnection);

    // loading and saving thread
    m_timerAutoSave.setInterval(1*60*1000); // 1 minutes
    connect(&m_timerAutoSave, SIGNAL(timeout()), SLOT(onTimerAutoSaveTimout()));
    //
    //
    connect(this, SIGNAL(loadFinishedEx(bool)), SLOT(onEditorLoadFinished(bool)));
    connect(view()->titleBar(), SIGNAL(onViewMindMap(bool)), SLOT(onViewMindMap(bool)));
    //
    //
    if (m_app.userSettings().isEnableSpellCheck()) {
        QWebEngineProfile *profile = p->profile();
        profile->setSpellCheckEnabled(true);
        profile->setSpellCheckLanguages({"en-US"});
    }
}

WizDocumentWebView::~WizDocumentWebView()
{
}
void WizDocumentWebView::waitForDone()
{
    if (m_docLoadThread) {
        m_docLoadThread->waitForDone();
        m_docLoadThread = NULL;
    }
    if (m_docSaverThread) {
        m_docSaverThread->waitForDone();
        m_docSaverThread = NULL;
    }
}


bool WizDocumentWebView::onPasteCommand()
{
    QClipboard* clip = QApplication::clipboard();
    Q_ASSERT(clip);

    if (isEditing())
    {
        if (!clip->image().isNull())
        {
            // save clipboard image to
            QString strImagePath = noteResourcesPath();
            CString strFileName = strImagePath + WizIntToStr(WizGetTickCount()) + ".png";
            if (!clip->image().save(strFileName)) {
                TOLOG("ERROR: Can't save clipboard image to file");
                return false;
            }

            QString strHtml;
            if (!WizImage2Html(strFileName, strHtml, strImagePath))
                return false;
            //
            editorCommandExecuteInsertHtml(strHtml, true);
            //
            return true;
        }
        //
    }
    //
    return false;
}

void WizDocumentWebView::inputMethodEvent(QInputMethodEvent* event)
{
    // On X windows, fcitx flick while preediting, only update while webview end process.
    // maybe it's a QT-BUG?
#ifdef Q_OS_LINUX
    setUpdatesEnabled(false);
    //QWebView::inputMethodEvent(event);
    setUpdatesEnabled(true);
#else
    QWebEngineView::inputMethodEvent(event);
#endif

#ifdef Q_OS_MAC
    /*
    ///暂时注释代码，移动输入光标会导致极高的CPU占用率，导致输入卡顿。

    //int nLength = 0;
    int nOffset = 0;
    for (int i = 0; i < event->attributes().size(); i++) {
        const QInputMethodEvent::Attribute& a = event->attributes().at(i);
        if (a.type == QInputMethodEvent::Cursor) {
            //nLength = a.length;
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
    */
#endif // Q_OS_MAC
}

void WizDocumentWebView::keyPressEvent(QKeyEvent* event)
{
    // special cases process
    if (event->key() == Qt::Key_Escape)
    {
        // FIXME: press esc will insert space at cursor if not clear focus
        clearFocus();
        return;
    }
    else if (event->key() == Qt::Key_S
             && event->modifiers() == Qt::ControlModifier)
    {
        trySaveDocument(view()->note(), false, [=](const QVariant&){});
        return;
    }
#if QT_VERSION >= 0x050402
//    else if (event->modifiers() == Qt::ControlModifier)
//    {
//        return;
//    }
    //FIXME: QT5.4.2之后无法触发全局的保存按钮
    else if (event->key() == Qt::Key_V && event->modifiers() == Qt::ControlModifier)
    {
        WizGetAnalyzer().logAction("paste");

        triggerPageAction(QWebEnginePage::Paste);
        return;
    }
#endif
#if QT_VERSION < 0x050000
    #ifdef Q_OS_MAC
    else if (event->key() == Qt::Key_Z)
    {
        //Ctrl+Shift+Z,  shortcut for redo can't catch by actions in QT4
        Qt::KeyboardModifiers keyMod = QApplication::keyboardModifiers();
        bool isSHIFT = keyMod.testFlag(Qt::ShiftModifier);
        bool isCTRL = keyMod.testFlag(Qt::ControlModifier);
        if (isCTRL && isSHIFT) {
            redo();
            return;
        } else if (isCTRL && !isSHIFT) {
            undo();
            return;
        }
    }
    #endif

#endif


#ifdef Q_OS_LINUX
    setUpdatesEnabled(false);
    //QWebView::keyPressEvent(event);
    setUpdatesEnabled(true);
#else

    QWebEngineView::keyPressEvent(event);
#endif

    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        tryResetTitle();
    }

    emit updateEditorToolBarRequest();
}

void WizDocumentWebView::mousePressEvent(QMouseEvent* event)
{
    QWebEngineView::mousePressEvent(event);
    emit updateEditorToolBarRequest();
}

void WizDocumentWebView::focusInEvent(QFocusEvent *event)
{
    if (m_currentEditorMode == modeEditor) {
        Q_EMIT focusIn();
    }

    QWebEngineView::focusInEvent(event);
}

void WizDocumentWebView::focusOutEvent(QFocusEvent *event)
{
    // because qt will clear focus when context menu popup, we need keep focus there.
    if (event->reason() == Qt::PopupFocusReason)
    {
        return;
    }
    else if (m_ignoreActiveWindowEvent && event->reason() == Qt::ActiveWindowFocusReason)
    {
        //NOTE:显示CWizTipsWidget的时候会造成编辑器失去焦点，进而导致toolbar关联的tips消失。此处通过
        //忽略tips显示时产生的ActiveWindowFocusReason来进行tips的显示
        return;
    }

    Q_EMIT focusOut();
    QWebEngineView::focusOutEvent(event);
}

void WizDocumentWebView::contextMenuEvent(QContextMenuEvent *event)
{
    Q_EMIT showContextMenuRequest(mapToGlobal(event->pos()));
}

void WizDocumentWebView::dragEnterEvent(QDragEnterEvent *event)
{
    if (!isEditing())
        return;

    int nAccepted = 0;
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> li = event->mimeData()->urls();
        QList<QUrl>::const_iterator it;
        for (it = li.begin(); it != li.end(); it++) {
            QUrl url = *it;
            if (url.toString().startsWith("file:///")) {
                nAccepted++;
            }
        }

        if (nAccepted == li.size()) {
            event->acceptProposedAction();
        }
    } else if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS)) {
        if (!event->mimeData()->data(WIZNOTE_MIMEFORMAT_DOCUMENTS).isEmpty()) {
            setFocus();
            event->acceptProposedAction();
        }
    }
}

void WizDocumentWebView::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS)) {
            if (!event->mimeData()->data(WIZNOTE_MIMEFORMAT_DOCUMENTS).isEmpty()) {
                setFocus();
                event->acceptProposedAction();
            }
    }
}

void WizDocumentWebView::onActionTriggered(QWebEnginePage::WebAction act)
{
    //在QT5.4.2之后webpage会覆盖menubar的快捷键，且无法禁止。某些操作需要由编辑器进行操作(undo, redo)，
   //需要webpage将操作反馈给webview来执行编辑器操作
    if (act == QWebEnginePage::Paste)
    {
        tryResetTitle();
    }
    else if (QWebEnginePage::Undo == act)
    {
        //WizGetAnalyzer().LogAction("Undo");
        undo();
    }
    else if (QWebEnginePage::Redo == act)
    {
        //WizGetAnalyzer().LogAction("Redo");
        redo();
    }
}

void WizDocumentWebView::tryResetTitle()
{
    if (m_bNewNoteTitleInited)
        return;

    // if note already modified, maybe title changed by use manuallly
    if (view()->note().tCreated.secsTo(view()->note().tModified) != 0)
        return;

    //
    page()->toPlainText([=](const QString& text){
        QString strTitle = WizStr2Title(text.left(128));
        if (strTitle.isEmpty())
            return;

        view()->resetTitle(strTitle);
        //
        m_bNewNoteTitleInited = true;
    });
}

void WizDocumentWebView::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();

    int nAccepted = 0;
    if (mimeData->hasUrls())
    {
        QList<QUrl> li = mimeData->urls();
        QList<QUrl>::const_iterator it;
        for (it = li.begin(); it != li.end(); it++) {
            QUrl url = *it;
            url.setScheme(0);

            //paste image file as images, add attachment for other file
            QString strFileName = url.toString();
#ifdef Q_OS_MAC
            if (wizIsYosemiteFilePath(strFileName))
            {
                strFileName = wizConvertYosemiteFilePathToNormalPath(strFileName);
            }
#endif
//            QImageReader reader(strFileName);
            QFileInfo info(strFileName);

            //FIXME: //TODO:  应该和附件列表中的添加附件合并
            if (info.size() == 0)
            {
                WizMessageBox::warning(nullptr , tr("Info"), tr("Can not add a 0 bit size file as attachment! File name : ' %1 '").arg(strFileName));
                continue;
            }
            else if (info.isBundle())
            {
                WizMessageBox::warning(nullptr, tr("Info"), tr("Can not add a bundle file as attachment! File name : ' %1 '").arg(strFileName));
                continue;
            }

            QList<QByteArray> imageFormats = QImageReader::supportedImageFormats();
            if (imageFormats.contains(info.suffix().toUtf8()))
            {
                QString strHtml;
                if (WizImage2Html(strFileName, strHtml, noteResourcesPath())) {
                    editorCommandExecuteInsertHtml(strHtml, true);
                    nAccepted++;
                }
            }
            else
            {
                WIZDOCUMENTDATA doc = view()->note();
                WizDatabase& db = m_dbMgr.db(doc.strKbGUID);
                WIZDOCUMENTATTACHMENTDATA data;
                data.strKbGUID = doc.strKbGUID; // needed by under layer
                if (!db.addAttachment(doc, strFileName, data))
                {
                    TOLOG1("[drop] add attachment failed %1", strFileName);
                    continue;
                }
                addAttachmentThumbnail(strFileName, data.strGUID);
                nAccepted ++;
            }
        }
    }
    else if (mimeData->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS))
    {
        QString strData(mimeData->data(WIZNOTE_MIMEFORMAT_DOCUMENTS));
        if (!strData.isEmpty())
        {
            QString strLinkHtml;
            QStringList doclist = strData.split(';');
            foreach (QString doc, doclist) {
                QStringList docIds = doc.split(':');
                if (docIds.count() == 2)
                {
                    WIZDOCUMENTDATA document;
                    WizDatabase& db = m_dbMgr.db(docIds.first());
                    if (db.documentFromGuid(docIds.last(), document))
                    {
                        QString strHtml, strLink;
                        WizNoteToHtmlLink(document, strHtml, strLink);
                        strLinkHtml += "<span>&nbsp;" + strHtml + "&nbsp;</span>";
                    }
                }
            }

            editorCommandExecuteInsertHtml(strLinkHtml, false);

            nAccepted ++;
        }
    }

    if (nAccepted > 0) {
        event->accept();
        trySaveDocument(view()->note(), false, [=](const QVariant&){});
    }
}

WizDocumentView* WizDocumentWebView::view() const
{
    QWidget* pParent = parentWidget();
    while(pParent) {
        if (WizDocumentView* view = dynamic_cast<WizDocumentView*>(pParent)) {
            return view;
        }

        pParent = pParent->parentWidget();
    }

    return 0;
}

void WizDocumentWebView::clear()
{
    load(QUrl("about:blank"));
}

void WizDocumentWebView::onTimerAutoSaveTimout()
{
    trySaveDocument(view()->note(), false, [=](const QVariant&){});
}

void WizDocumentWebView::onTitleEdited(QString strTitle)
{
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
    mainWindow->titleChanged();
    //
    WIZDOCUMENTDATA document = view()->note();
    document.strTitle = strTitle;
    // Only sync when contents unchanged. If contents changed would sync after document saved.
    isModified([=](bool modified){

        if (modified)
        {
            WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
            mainWindow->quickSyncKb(document.strKbGUID);
        }
    });
}

void WizDocumentWebView::onDocumentReady(const QString kbGUID, const QString strGUID, const QString strFileName, WizEditorMode editorMode)
{
    m_mapFile.insert(strGUID, strFileName);

    WIZDOCUMENTDATA doc;
    if (!m_dbMgr.db(kbGUID).documentFromGuid(strGUID, doc))
        return;

    //
    loadDocumentInWeb(editorMode);
}

void WizDocumentWebView::onDocumentSaved(const QString kbGUID, const QString strGUID, bool ok)
{
    if (!ok)
    {
        TOLOG("Save document failed");
    }
    //
    view()->sendDocumentSavedSignal(strGUID, kbGUID);
    //
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
    mainWindow->quickSyncKb(kbGUID);
}

void WizDocumentWebView::viewDocument(const WIZDOCUMENTDATA& doc, WizEditorMode editorMode)
{
    if (!m_docLoadThread)
        return;
    // set data
    int seconds = doc.tCreated.secsTo(QDateTime::currentDateTime());
    m_bNewNote = (seconds >= 0 && seconds <= 1) ? true : false;
    m_bNewNoteTitleInited = m_bNewNote ? false : true;
    //
    setModified(false);

    if (m_bNewNote)
    {
        editorMode = modeEditor;
    }

    // ask extract and load
    m_docLoadThread->load(doc, editorMode);
}

void WizDocumentWebView::reloadNoteData(const WIZDOCUMENTDATA& data)
{
    if (!m_docLoadThread)
        return;
    //
    Q_ASSERT(!data.strGUID.isEmpty());

    // reset only if user not in editing mode
    if (isEditing() && hasFocus())
        return;

    // reload may triggered when update from server or locally reflected by modify
    m_docLoadThread->load(data, m_currentEditorMode);
}


void WizDocumentWebView::setNoteTitleInited(bool inited)
{
    m_bNewNoteTitleInited = inited;
}

void WizDocumentWebView::setInSeperateWindow(bool inSeperateWindow)
{
    m_bInSeperateWindow = inSeperateWindow;
}

bool WizDocumentWebView::isInSeperateWindow() const
{
    return m_bInSeperateWindow;
}

void WizDocumentWebView::replaceDefaultCss(QString& strHtml)
{
    QString strFileName = Utils::WizPathResolve::resourcesPath() + "files/wizeditor/default.css";
    QFile f(strFileName);
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "[Editor]Failed to get default css code";
        return;
    }
    //
    QString strCss;
    if (!WizLoadUnicodeTextFromFile(strFileName, strCss))
        return;
    //
    QString strFont = m_app.userSettings().defaultFontFamily();
    int nSize = m_app.userSettings().defaultFontSize();
    QString lineHeight = m_app.userSettings().editorLineHeight();
    QString paraSpacing = m_app.userSettings().editorParaSpacing();
    QString backgroundColor = m_app.userSettings().editorBackgroundColor();
    //
    strCss.replace("/*default-font-family*/", QString("font-family:'%1';").arg(strFont));
    strCss.replace("/*default-font-size*/", QString("font-size:%1px;").arg(nSize));
    if (backgroundColor.isEmpty())
    {
        backgroundColor = m_bInSeperateWindow ? "#F5F5F5" : "#FFFFFF";
    }
    //
    if (isDarkMode()) {
        backgroundColor = "#272727";
    }
    //

    strCss.replace("/*default-background-color*/", QString("background-color:%1;").arg(backgroundColor));
    strCss.replace("/*default-line-height*/", QString("line-height:%1;").arg(lineHeight));
    strCss.replace("/*default-para-spacing*/", QString("margin-top:%1px; margin-bottom:%1px").arg(paraSpacing));

    //
    const QString customCssId("wiz_custom_css");

    WizHtmlRemoveStyle(strHtml, customCssId);
    WizHtmlInsertStyle(strHtml, customCssId, strCss);
}

void WizDocumentWebView::editorResetFont()
{
    WIZDOCUMENTDATA data = view()->note();
    if (data.strGUID.isEmpty()) {
        return;
    }
    //
    trySaveDocument(data, false, [=](const QVariant&){
        //
        QTimer::singleShot(300, [=]() {
            reloadNoteData(data);
        });
        //
    });
}

void WizDocumentWebView::editorResetSpellCheck()
{
    QWebEngineProfile *profile = page()->profile();
    if (m_app.userSettings().isEnableSpellCheck()) {
        profile->setSpellCheckEnabled(true);
        profile->setSpellCheckLanguages({"en-US"});
    } else {
        profile->setSpellCheckEnabled(false);
    }
}


void WizDocumentWebView::editorFocus()
{
    page()->runJavaScript("WizEditor.focus();");
    emit focusIn();
}

void WizDocumentWebView::enableEditor(bool enalbe)
{
    if (enalbe)
    {
        page()->runJavaScript("WizEditor.on();");
        setFocus();
    }
    else
    {
        QString code = QString("WizEditor.off({reader: {type:'%1'}});").arg(getNoteType());
        //
        page()->runJavaScript(code);
        //
        clearFocus();
    }
}

void WizDocumentWebView::setIgnoreActiveWindowEvent(bool igoreEvent)
{
    m_ignoreActiveWindowEvent = igoreEvent;
}

bool WizDocumentWebView::evaluateJavaScript(const QString& js)
{
    page()->runJavaScript(js);
    return true;
}


void WizDocumentWebView::on_insertCommentToNote_request(const QString& docGUID, const QString& comment)
{
    if (docGUID != view()->note().strGUID)
        return;

    if (!isEditing() && WizIsMarkdownNote(view()->note()))
    {
        WizMessageBox::information(this, tr("Info"), tr("Do not support insert comment into markdown note."));
        return;
    }
    //
    //应该优化，对于模版类型的笔记，插入位置不应该是最前面
    if (isEditing())
    {
        QString htmlBody = "<div>" + comment + "</div><hr>";
        htmlBody.replace("\n", "<br>");
        htmlBody.replace("'", "&#39;");
        //
        QString code = QString("var objDiv = document.createElement('div');objDiv.innerHTML= unescape('%1');"
                               "var first=document.body.firstChild;document.body.insertBefore(objDiv,first);").arg(htmlBody);
        //
        page()->runJavaScript(code);
        //
        saveEditingViewDocument(view()->note(), true, [=](const QVariant &){});

    }
    else
    {
        QString commentsText = QUrl::fromPercentEncoding(comment.toUtf8());
        QString commentsHtml = ::WizText2Html(commentsText);
        //
        QString documentGuid = docGUID;
        QString kbGuid = view()->note().strKbGUID;
        //
        ::WizExecutingActionDialog::executeAction(QObject::tr("Saving comments..."), WIZ_THREAD_DEFAULT, [=]{
            //
            WizDatabase& db = m_dbMgr.db(kbGuid);
            //
            WIZDOCUMENTDATA doc;
            if (!db.documentFromGuid(documentGuid, doc))
                return;
            //
            QString strTempPath = ::Utils::WizPathResolve::tempPath() + WizGenGUIDLowerCaseLetterOnly() + "/";
            ::WizEnsurePathExists(strTempPath);
            //
            if (!db.documentToHtmlFile(doc, strTempPath))
                return;
            //
            QString htmlFileName = strTempPath + "index.html";
            if (!QFileInfo::exists(htmlFileName))
                return;
            //
            QString html;
            if (!WizLoadUnicodeTextFromFile(htmlFileName, html))
                return;
            //
            WizHtmlInsertHtmlBeforeAllBodyChildren(html, commentsHtml);
            //
            if (!::WizSaveUnicodeTextToUtf8File(htmlFileName, html, true))
                return;
            //
            db.updateDocumentDataWithFolder(doc, strTempPath, true);
        });
    }
}

void WizDocumentWebView::setWindowVisibleOnScreenShot(bool bVisible)
{
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(m_app.mainWindow());
    if (mainWindow)
    {
        bVisible ? mainWindow->show() : mainWindow->hide();
    }
}

QString WizDocumentWebView::noteResourcesPath()
{
    Q_ASSERT(!m_strNoteHtmlFileName.isEmpty());
    if (m_strNoteHtmlFileName.isEmpty())
        return QString();
    //
    QString path = Utils::WizMisc::extractFilePath(m_strNoteHtmlFileName);
    path += "index_files/";
    Utils::WizMisc::ensurePathExists(path);
    //
    return path;
}

void WizDocumentWebView::insertImage(const QString& strFileName)
{
    QString strHtml;
    if (WizImage2Html(strFileName, strHtml, noteResourcesPath())) {
        editorCommandExecuteInsertHtml(strHtml, true);
    }
}


void WizDocumentWebView::addAttachmentThumbnail(const QString strFile, const QString& strGuid)
{
    QImage img;
    ::WizCreateThumbnailForAttachment(img, strFile, QSize(32, 32));
    QString strDestFile =Utils::WizPathResolve::tempPath() + WizGenGUIDLowerCaseLetterOnly() + ".png";
    img.save(strDestFile, "PNG");
    QString strLink = QString("wiz://open_attachment?guid=%1").arg(strGuid);
    QSize szImg = img.size();
    if (WizIsHighPixel())
    {
        szImg.scale(szImg.width() / 2, szImg.height() / 2, Qt::IgnoreAspectRatio);
    }
    QString strHtml = WizGetImageHtmlLabelWithLink(strDestFile, szImg, strLink);
    editorCommandExecuteInsertHtml(strHtml, true);
}

void WizDocumentWebView::getMailSender(std::function<void(QString)> callback)
{
    QString scriptFileName = Utils::WizPathResolve::resourcesPath() + "files/scripts/GetMailSender.js";
    QString code;
    ::WizLoadUnicodeTextFromFile(scriptFileName, code);
    //
    page()->runJavaScript(code, [=](const QVariant& vRet){
        //
        QString mailSender = vRet.toString();

        if (mailSender.isEmpty())
        {
            QRegExp rxlen("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b");
            rxlen.setCaseSensitivity(Qt::CaseInsensitive);
            rxlen.setPatternSyntax(QRegExp::RegExp);
            QString strTitle = view()->note().strTitle;
            int pos = rxlen.indexIn(strTitle);
            if (pos > -1) {
                mailSender = rxlen.cap(0);
            }
        }
        //
        callback(mailSender);
    });
}

///*
// * 是否添加用户自定义的样式，目前有字体类型、字体大小、背景颜色等信息等信息
// */
//bool CWizDocumentWebView::shouldAddUserDefaultCSS()
//{
//    if (!shouldAddCustomCSS())
//        return false;

//    bool isTemplate = page()->runJavaScript("wizIsTemplate()").toBool();

//    return !isTemplate;
//}

void WizDocumentWebView::shareNoteByEmail()
{
    getMailSender([=](QString sendTo){
        //

        WizEmailShareDialog dlg(m_app);
        //
        dlg.setNote(view()->note(), sendTo);
        //
        connect(&dlg, SIGNAL(insertCommentToNoteRequest(QString,QString)),
                SLOT(on_insertCommentToNote_request(QString,QString)));

        dlg.exec();

    });
}

void WizDocumentWebView::shareNoteByLink()
{
    const WIZDOCUMENTDATA& doc = view()->note();
    emit shareDocumentByLinkRequest(doc.strKbGUID, doc.strGUID);
}

QString WizDocumentWebView::getNoteType()
{
    const WIZDOCUMENTDATA& doc = view()->note();
    //
    if (doc.strType == "outline") {
        return "outline";
    }
    //
    QString title = doc.strTitle;
    if (title.endsWith(".md"))
        return "markdown";
    if (title.endsWith(".mj"))
        return "mathjax";
    //
    if (title.indexOf(".md@") != -1)
        return "markdown";
    if (title.indexOf(".mj@") != -1)
        return "mathjax";
    //
    if (title.indexOf(".md ") != -1)
        return "markdown";
    if (title.indexOf(".mj ") != -1)
        return "mathjax";
    //
    return "common";
}

bool WizDocumentWebView::isOutline() const
{
    const WIZDOCUMENTDATA& doc = view()->note();
    return doc.strType == "outline";
}

QString WizDocumentWebView::getHighlightKeywords()
{
    const WIZDOCUMENTDATAEX& doc = view()->note();
    CWizStdStringArray texts;
    texts.push_back(doc.strHighlightText);
    texts.push_back(doc.strHighlightTitle);
    //
    std::set<QString> lowerCaseKeywords;
    std::set<QString> keywords;
    for (auto it : texts)
    {
        QString text = it;
        int begin = 0;
        while (true) {
            int start = text.indexOf("<em>",  begin);
            if (start == -1)
                break;
            start += 4;
            int end = text.indexOf("</em>", start);
            if (end == -1)
                break;
            //
            QString keyword = text.mid(start, end - start);
            QString lower = keyword.toLower();
            if (lowerCaseKeywords.find(lower) == lowerCaseKeywords.end())
            {
                lowerCaseKeywords.insert(lower);
                keywords.insert("'" + keyword + "'");
            }
            //
            begin = end + 5;
        }
    }
    //
    CWizStdStringArray arr;
    arr.assign(keywords.begin(), keywords.end());
    //
    CString ret;
    ::WizStringArrayToText(arr, ret, ",");
    return ret;
}


void WizDocumentWebView::onEditorLoadFinished(bool ok)
{
    WizDocumentWebView* self = this;
    if (isDarkMode()) {
        //
        QTimer::singleShot(1000, [=] {
            self->setVisible(true);
        });
    }
    //
    if (!ok)
        return;
    //
    QString resPath = Utils::WizPathResolve::resourcesPath() + "files/";
    QString editorPath = resPath + "wizeditor/";
    QString lang = "zh-cn";
    QString userGUID = m_dbMgr.db().getUserGuid();
    QString userAlias = m_dbMgr.db().getUserAlias();
    //
    const WIZDOCUMENTDATAEX& doc = view()->note();
    bool ignoreTable = doc.strURL.startsWith("http");
    //
    QString noteType = getNoteType();
    //
    QString strCode = WizFormatString7("WizEditorInit(\"%1\", \"%2\", \"%3\", \"%4\", %5, \"%6\", %7);",
                                       editorPath, lang, userGUID, userAlias,
                                       ignoreTable ? "true" : "false",
                                       noteType,
                                       isDarkMode() ? "true" : "false");
    qDebug() << strCode;
    if (m_currentEditorMode == modeEditor)
    {
        strCode += "WizEditor.on();";
    }
    else
    {
        QString keywords = getHighlightKeywords();
        if (keywords.isEmpty()) {
            //
            strCode += "WizEditor.off();";
            //
        } else {
            strCode += QString("WizEditor.off(null, function(){\n\
                WizReader.highlight.on([%1]);\nconsole.log('highlight');\n\
            });").arg(keywords);
        }
    }
    //
    qDebug() << strCode;
    //
    page()->runJavaScript(strCode);
    //
    WizPlugins::plugins().notifyDocumentChanged();
}

bool WizDocumentWebView::isInternalUrl(const QUrl& url)
{
    return url.scheme().toLower() == "wiz";
}


void WizDocumentWebView::onEditorLinkClicked(QUrl url, QWebEnginePage::NavigationType navigationType, bool isMainFrame, WizWebEnginePage* page)
{
    if (!isMainFrame)
        return;
    //
    page->stopCurrentNavigation();
    //
    onOpenLinkInNewWindow(url);
}

void WizDocumentWebView::onOpenLinkInNewWindow(QUrl url)
{
    if (isInternalUrl(url))
    {
        QString strUrl = url.toString();
        switch (GetWizUrlType(strUrl)) {
        case WizUrl_Document:
            viewDocumentByUrl(strUrl);
            return;
        case WizUrl_Attachment:
            if (!isEditing())
            {
                viewAttachmentByUrl(view()->note().strKbGUID, strUrl);
            }
            return;
        default:
            qDebug() << QString("%1 is a wiz internal url , but we can not identify it");
            return;
        }
    }
    else
    {
        QString strUrl = url.toString();
        if (strUrl.left(12) == "http://file/")
        {
            strUrl.replace(0, 12, "file:/");
        }

        qDebug() << "Open url " << strUrl;
#if QT_VERSION != QT_VERSION_CHECK(5,12,3)
        QDesktopServices::openUrl(strUrl);
#endif
        return;
    }
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

void WizDocumentWebView::viewDocumentByUrl(const QString& strUrl)
{
    if (strUrl.isEmpty())
        return;

    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(m_app.mainWindow());
    mainWindow->viewDocumentByWizKMURL(strUrl);
}

void WizDocumentWebView::viewAttachmentByUrl(const QString& strKbGUID, const QString& strUrl)
{
    if (strUrl.isEmpty())
        return;

    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(m_app.mainWindow());
    mainWindow->viewAttachmentByWizKMURL(strKbGUID, strUrl);
}

void getHtmlBodyStyle(const QString& strHtml, QString& strBodyStyle)
{
    QRegExp regh("<body ([^>]*)>", Qt::CaseInsensitive);
    if (regh.indexIn(strHtml) != -1)
    {
        strBodyStyle = regh.cap(1);
    }
}

void WizDocumentWebView::saveEditingViewDocument(const WIZDOCUMENTDATA &data, bool force, std::function<void(const QVariant &)> callback)
{
    //FIXME: remove me, just for find a image losses bug.
    Q_ASSERT(!data.strGUID.isEmpty());

    // check note permission
    if (!m_dbMgr.db(data.strKbGUID).canEditDocument(data)) {
        callback(QVariant(false));
        return;
    }
    //
    WIZDOCUMENTDATA doc = data;
    //
    isModified([=](bool modified) {

        if (!force)
        {
            if (!modified)
            {
                callback(QVariant(true));
                return;
            }
        }
        //
        setModified(false);
        //
        QString strFileName = m_mapFile.value(data.strGUID);
        //
        QString strScript = "WizEditor.getContentHtml()";
        //
        TOLOG("saving note...");
        page()->runJavaScript(strScript, [=](const QVariant& ret){
            //
            bool succeeded = false;
            if (ret.type() == QVariant::String)
            {
                QString html = ret.toString();
                if (!html.isEmpty())
                {
                    succeeded = true;
                    m_currentNoteHtml = html;
                    ::WizSaveUnicodeTextToUtf8File(m_strNoteHtmlFileName, m_currentNoteHtml);
                    emit currentHtmlChanged();
                    //
                    //qDebug() << m_currentNoteHtml;
                    //
                    m_docSaverThread->save(doc, html, strFileName, 0);
                    TOLOG("save note done...");
                }
            }
            //
            if (!succeeded)
            {
                TOLOG("save note failed, html is empty");
                QString message = tr("Failed to save note.");
                WizMessageBox::warning(this, tr("Error"), message);
            }
            //
            callback(QVariant(succeeded));
        });
        //
    });

}

void WizDocumentWebView::saveReadingViewDocument(const WIZDOCUMENTDATA &data, bool force, std::function<void(const QVariant &)> callback)
{
    Q_UNUSED(force);
    //
    const WIZDOCUMENTDATA doc = data;

    QString strScript = QString("WizReader.closeDocument();");
    page()->runJavaScript(strScript, [=](const QVariant& vRet) {
        //
        QString strHtml = vRet.toString();
        //
        if (!strHtml.isEmpty())
        {
            if (!doc.strGUID.isEmpty())
            {
                QString strFileName = m_mapFile.value(doc.strGUID);
                if (!strFileName.isEmpty())
                {
                    m_docSaverThread->save(doc, strHtml, strFileName, 0);
                }
            }
        }
        //
        callback(true);
    });
}

void WizDocumentWebView::on_insertCodeHtml_requset(QString strOldHtml)
{
    QString strHtml = strOldHtml;
    if (WizGetBodyContentFromHtml(strHtml, false))
    {
        QString name("wiz_code_highlight.css");
        QString strSrcCssFileName = Utils::WizPathResolve::resourcesPath() + "files/code/" + name;
        QString strDestCssFileName = noteResourcesPath() + name;

        if (QFile::exists(strDestCssFileName))
        {
            QFile::remove(strDestCssFileName);
        }
        if (!QFile::copy(strSrcCssFileName, strDestCssFileName))
        {
            QMessageBox::critical(this, tr("Error"), tr("Can't copy style files"));
            return;
        }
        //
        QString link = "index_files/" + name;
        //
        page()->runJavaScript(QString("WizAddCssForCode('%1');").arg(link));
        //
        editorCommandExecuteInsertHtml(strHtml, true);
    }
}


#ifdef QT_DEBUG
//#define DEBUG_EDITOR
#endif

void WizDocumentWebView::getAllEditorScriptAndStypeFileName(std::map<QString, QString>& files)
{
    QString strResourcePath = Utils::WizPathResolve::resourcesPath();
    QString strHtmlEditorPath = strResourcePath + "files/wizeditor/";
    //
#ifdef DEBUG_EDITOR
    QString strEditorJS = "http://192.168.1.73:8080/libs/wizDocument/wizEditorForMac.js";
#else
    QString strEditorJS = "file:///" +  strHtmlEditorPath + "wizEditorForMac.js";
#endif
    QString strInit = "file:///" + strHtmlEditorPath + "editorHelper.js";
    //
    files.clear();
    files[strEditorJS] = "";
    files[strInit] = "";
    //
    if (isDarkMode()) {
        QString darkCss = "file:///" +  strHtmlEditorPath + "wizDarkMode.css";
        files[darkCss] = "wiz_dark_mode_style";
    }
    //
    /*
     *
     * 渐变式加载笔记，暂时不需要
    QString tempCss = "file:///" + strHtmlEditorPath + "tempeditorstyle.css";
    QString tempCssLoadOnly = "file:///" + strHtmlEditorPath + "tempeditorstyle_loadonly.css";
    //
    files[tempCss] = "wiz_unsave_style";
    files[tempCssLoadOnly] = "wiz_style_for_load";
    */

}



void WizDocumentWebView::insertScriptAndStyleCore(QString& strHtml, const std::map<QString, QString>& files)
{
    Q_ASSERT(!files.empty());
    for (std::map<QString, QString>::const_iterator it = files.begin(); it != files.end(); it++)
    {
        QString url = it->first;
        QString strFileName = QUrl(url).toLocalFile();
        QString name = it->second;
        //
#ifndef DEBUG_EDITOR
        if (!strFileName.isEmpty())
        {
            Q_ASSERT(WizPathFileExists(strFileName));
        }
#endif
        //
        QString strExt = Utils::WizMisc::extractFileExt(strFileName);
        if (strExt.isEmpty())
        {
            strExt = Utils::WizMisc::extractFileExt(url);
        }
        //
        if (0 == strExt.compare(".css", Qt::CaseInsensitive))
        {
            if (name.isEmpty()) {
                name = "wiz_inner_style";
            }
            //
            QString strTag = WizFormatString2(
                        "<link rel=\"stylesheet\" type=\"text/css\" href=\"%1\" name=\"%2\" wiz_style=\"unsave\" charset=\"utf-8\">",
                        url, name);
            //
            if (strHtml.indexOf(strTag) == -1)
            {
                WizHTMLAppendTextInHead(strTag, strHtml);
            }
        }
        else if (0 == strExt.compare(".js", Qt::CaseInsensitive))
        {
            if (name.isEmpty()) {
                name = "wiz_inner_script";
            }
            QString	strTag = WizFormatString2(
                    "<script type=\"text/javascript\" src=\"%1\" name=\"%2\" wiz_style=\"unsave\" charset=\"utf-8\"></script>",
                    url, name);
            //
            if (strHtml.indexOf(strTag) == -1)
            {
                WizHTMLAppendTextInHead(strTag, strHtml);
            }
        }
        else
        {
            Q_ASSERT(false);
        }
    }
    //
    QString strTemplateJsFileName = ::Utils::WizPathResolve::wizTemplateJsFilePath();
    if (QFileInfo(strTemplateJsFileName).exists())
    {
        QString strTag = QString("<script type=\"text/javascript\" src=\"file:///%1\" wiz_style=\"unsave\" charset=\"utf-8\"></script>").arg(strTemplateJsFileName);
        //
        WizHTMLAppendTextInHead(strTag, strHtml);
    }
}
void WizDocumentWebView::addDefaultScriptsToDocumentHtml(QString htmlFileName)
{
    QString strHtml;
    bool ret = WizLoadUnicodeTextFromFile(htmlFileName, strHtml);
    if (!ret) {
        // hide client and show error
        return;
    }
    //
    m_currentNoteHtml = strHtml;
    emit currentHtmlChanged();
    //
    std::map<QString, QString> files;
    getAllEditorScriptAndStypeFileName(files);
    insertScriptAndStyleCore(strHtml, files);
    //
    replaceDefaultCss(strHtml);
    //
    ::WizSaveUnicodeTextToUtf8File(htmlFileName, strHtml, true);

}
void WizDocumentWebView::loadDocumentInWeb(WizEditorMode editorMode)
{
    QString strGUID = view()->note().strGUID;
    QString strFileName = m_mapFile.value(strGUID);
    if (strFileName.isEmpty()) {
        return;
    }
    //
    QString strHtml;
    bool ret = WizLoadUnicodeTextFromFile(strFileName, strHtml);
    if (!ret) {
        // hide client and show error
        return;
    }
    //
    addDefaultScriptsToDocumentHtml(strFileName);

    WizEditorMode oldMode = m_currentEditorMode;
    m_currentEditorMode = editorMode;
    if (oldMode != m_currentEditorMode)
    {
        if (m_currentEditorMode == modeEditor) {
            Q_EMIT focusIn();
        } else {
            Q_EMIT focusOut();
        }
        //
        view()->titleBar()->setEditorMode(m_currentEditorMode);
    }
    //
    if (isDarkMode()) {
        page()->setBackgroundColor(QColor("#272727"));
        setVisible(false);
    }
    //
    m_strNoteHtmlFileName = strFileName;
    load(QUrl::fromLocalFile(strFileName));

    onNoteLoadFinished();
}

void WizDocumentWebView::onNoteLoadFinished()
{
    WizGlobal::instance()->emitViewNoteLoaded(view(), view()->note(), true);
}

void WizDocumentWebView::setEditorMode(WizEditorMode editorMode)
{
    if (m_currentEditorMode == editorMode)
        return;
    //
    bool editing = editorMode == modeEditor;
    // show editor toolbar properly
    if (!editing && hasFocus()) {
        Q_EMIT focusOut();
    }

    if (editing && hasFocus()) {
        Q_EMIT focusIn();
    }

    WIZDOCUMENTDATA docData;
    WizDatabase& db = m_dbMgr.db(view()->note().strKbGUID);
    if (!db.documentFromGuid(view()->note().strGUID, docData))
        return;

    trySaveDocument(docData, false, [=](const QVariant&){
        //
        enableEditor(editing);
        //
        if (editing) {
            setFocus(Qt::MouseFocusReason);
            editorFocus();
        }
        //
    });

    m_currentEditorMode = editorMode;
    //
    if (m_currentEditorMode == modeEditor) {
        m_timerAutoSave.start();
    } else {
        m_timerAutoSave.stop();
    }
}

void WizDocumentWebView::trySaveDocument(const WIZDOCUMENTDATA& data, bool force, std::function<void(const QVariant &)> callback)
{
    if (!view()->noteLoaded())  //encrypting note & has been loaded
    {
        callback(QVariant(false));
        return;
    }

    if (m_currentEditorMode == modeEditor)
    {
        saveEditingViewDocument(data, force, callback);
    }
    else
    {
        saveReadingViewDocument(data, force, callback);
    }
}

void WizDocumentWebView::editorCommandQueryCommandValue(const QString& strCommand, std::function<void(const QString& value)> callback)
{
    QString script = "document.queryCommandValue('" + strCommand +"');";
    page()->runJavaScript(script, [=](const QVariant& ret){
        //
        callback(ret.toString());
        //
    });
}

void WizDocumentWebView::editorCommandQueryCommandState(const QString& strCommand, std::function<void(int state)> callback)
{
    QString script = "document.queryCommandState('" + strCommand +"');";
    page()->runJavaScript(script, [=](const QVariant& ret){
        //
        callback(ret.toInt());
        //
    });
}

/*
 * Execute command and also save status to undostack.
 * All commands execute from client which may modify document MUST invoke this
 * instead of use frame's evaluateJavascript.
 */
void WizDocumentWebView::editorCommandExecuteCommand(const QString& strCommand,
                                                      const QString& arg1 /* = QString() */,
                                                      const QString& arg2 /* = QString() */,
                                                      const QString& arg3 /* = QString() */)
{
    QString strExec = QString("WizEditor.execCommand('%1'").arg(strCommand);
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

    qDebug() << strExec;

    page()->runJavaScript(strExec);
    //
    setModified(true);
}


bool WizDocumentWebView::editorCommandQueryMobileFileReceiverState()
{
    return m_app.userSettings().receiveMobileFile();
}

void WizDocumentWebView::editorCommandExecuteParagraph(const QString& strType)
{
    WizGetAnalyzer().logAction("editorParagraph");
    editorCommandExecuteCommand("formatBlock", "false", "'" + strType + "'");
}

void WizDocumentWebView::editorCommandExecuteInsertHtml(const QString& strHtml, bool bNotSerialize)
{
    QString s = bNotSerialize ? "true" : "false";
    //
    QString base64Html = WizStringToBase64(strHtml);
    base64Html.replace("\r", "");
    base64Html.replace("\n", "");
    QString code = QString("WizEditor.insertB64Html('%1')").arg(base64Html);
    //
    page()->runJavaScript(code);
    //editorCommandExecuteCommand("insertHtml", s, "'" + strHtml + "'");
}


void WizDocumentWebView::editorCommandExecutePastePlainText()
{
    QClipboard* clip = QApplication::clipboard();
    if (!clip)
        return;
    const QMimeData* data = clip->mimeData();
    if (!data)
        return;
    QString text = data->text();
    //
    QString base64Text = WizStringToBase64(text);
    QString js = QString("WizEditor.pasteB64('', '%1')").arg(base64Text);
    page()->runJavaScript(js);
}

void WizDocumentWebView::editorCommandExecuteIndent()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("indent");
    editorCommandExecuteCommand("indent");
}

void WizDocumentWebView::editorCommandExecuteOutdent()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("outdent");
    editorCommandExecuteCommand("outdent");
}

void WizDocumentWebView::editorCommandExecuteLinkInsert()
{
    if (!m_editorInsertLinkForm) {
        m_editorInsertLinkForm = new WizEditorInsertLinkForm(window());
        connect(m_editorInsertLinkForm, SIGNAL(accepted()), SLOT(on_editorCommandExecuteLinkInsert_accepted()));
    }
    //
    page()->runJavaScript("WizEditor.link.getCurrentLink();", [=](const QVariant& vLink){
        //
        QString strUrl = vLink.toString();
        if (strUrl.isEmpty()) {
            strUrl = "http://";
        }
        //
        m_editorInsertLinkForm->setUrl(strUrl);

        m_editorInsertLinkForm->exec();

        WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
        analyzer.logAction("linkInsert");
        //
    });
}

void WizDocumentWebView::on_editorCommandExecuteLinkInsert_accepted()
{
    // append http if not exist
    QString strUrl = m_editorInsertLinkForm->getUrl();
    if (strUrl.isEmpty() || strUrl == "http://") {
        //
        QString code = "WizEditor.link.removeSelectedLink();";
        page()->runJavaScript(code);
        //
    } else {
        QUrl url(strUrl);
        if (url.scheme().isEmpty())
        {
            strUrl = "http://" + strUrl;
        }
        else
        {
            strUrl = url.toString();
        }
        //
        QString code = QString("WizEditor.link.setCurrentLink('%1');").arg(strUrl);
        //
        page()->runJavaScript(code);
    }
}

void WizDocumentWebView::editorCommandExecuteLinkRemove()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("unlink");
    editorCommandExecuteCommand("unlink");
}

void WizDocumentWebView::editorCommandExecuteFindReplace()
{
    if (!m_searchReplaceWidget)
    {
        m_searchReplaceWidget = new WizSearchReplaceWidget(this);
        //
        connect(m_searchReplaceWidget, SIGNAL(findPre(QString,bool)), SLOT(findPre(QString,bool)));
        connect(m_searchReplaceWidget, SIGNAL(findNext(QString,bool)), SLOT(findNext(QString,bool)));
        connect(m_searchReplaceWidget, SIGNAL(replaceCurrent(QString,QString)), SLOT(replaceCurrent(QString,QString)));
        connect(m_searchReplaceWidget, SIGNAL(replaceAndFindNext(QString,QString,bool)), SLOT(replaceAndFindNext(QString,QString,bool)));
        connect(m_searchReplaceWidget, SIGNAL(replaceAll(QString,QString,bool)), SLOT(replaceAll(QString,QString,bool)));
    }

    QRect rect = geometry();
    rect.moveTo(mapToGlobal(pos()));
    m_searchReplaceWidget->showInEditor(rect);

    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("findReplace");

}

void WizDocumentWebView::innerFindText(QString text, bool next, bool matchCase)
{
    bool back = !next;
    QString code = QString("find('%1', %2, %3, true)").arg(text)
            .arg(back ? "true" : "false")
            .arg(matchCase ? "true" : "false");
    page()->runJavaScript(code);

}

static QString strOldSearchText = "";
static bool strOldCase = false;
void WizDocumentWebView::findPre(QString strTxt, bool bCasesensitive)
{
    //FIXME:  there is a problem here, HighlightAllOccurrences can not be used togethor with find one.
    if (strOldSearchText != strTxt || strOldCase != bCasesensitive)
    {
        // clear highlight
        //findText("");
        strOldSearchText = strTxt;
        strOldCase = bCasesensitive;
    }
    QWebEnginePage::FindFlags options;
    options |= QWebEnginePage::FindBackward;
    if (bCasesensitive)
    {
        options |= QWebEnginePage::FindCaseSensitively;
    }
    //
    //findText(strTxt, options);
    innerFindText(strTxt, false, bCasesensitive);
}

void WizDocumentWebView::findNext(QString strTxt, bool bCasesensitive)
{
    if (strOldSearchText != strTxt || strOldCase != bCasesensitive)
    {
        //findText("", 0);
        strOldSearchText = strTxt;
        strOldCase = bCasesensitive;
    }
    QWebEnginePage::FindFlags options;
    if (bCasesensitive)
    {
        options |= QWebEnginePage::FindCaseSensitively;
    }
    //
    //findText(strTxt, options);
    innerFindText(strTxt, true, bCasesensitive);
}

void WizDocumentWebView::replaceCurrent(QString strSource, QString strTarget)
{
    QString strExec = QString("WizEditor.replace('%1', '%2', true)").arg(strSource).arg(strTarget);
    page()->runJavaScript(strExec);
}

void WizDocumentWebView::replaceAndFindNext(QString strSource, QString strTarget, bool bCasesensitive)
{
    QString strExec = QString("WizEditor.replace('%1', '%2', %3)").arg(strSource).arg(strTarget).arg(bCasesensitive ? "true" : "false");
    page()->runJavaScript(strExec);
}

void WizDocumentWebView::replaceAll(QString strSource, QString strTarget, bool bCasesensitive)
{
    QString strExec = QString("WizEditor.replaceAll('%1', '%2', %3)").arg(strSource).arg(strTarget).arg(bCasesensitive ? "true" : "false");
    page()->runJavaScript(strExec);
    setModified(true);
}

void WizDocumentWebView::editorCommandExecuteFontFamily(const QString& strFamily)
{
    WizGetAnalyzer().logAction(QString("editorSetFontFamily : %1").arg(strFamily));
    editorCommandExecuteCommand("fontName", "false", "'" + strFamily + "'");
}

void WizDocumentWebView::editorCommandExecuteFontSize(const QString& strSize)
{
    WizGetAnalyzer().logAction(QString("editorSetFontSize : %1pt").arg(strSize));
    //
    CString strStyle = WizFormatString1("{\\\"font-size\\\" : \\\"%1pt\\\"}", strSize);
    CString strScript = WizFormatString1("WizEditor.modifySelectionDom(JSON.parse(\"%1\"))", strStyle);

    page()->runJavaScript(strScript);
    setModified(true);
}

void WizDocumentWebView::editorCommandExecuteBackColor(const QColor& color)
{
    if (color == QColor(Qt::transparent)) {
       //editorCommandExecuteCommand("backColor", "false", "'default'");
    }
    else {
        editorCommandExecuteCommand("backColor", "false", "'" + color.name() + "'");
    }
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("backColor");
}

void WizDocumentWebView::editorCommandExecuteForeColor(const QColor& color)
{
    if (color == QColor(Qt::transparent)) {
        //editorCommandExecuteCommand("foreColor", "false", "'default'");
    }
    else {
        editorCommandExecuteCommand("foreColor", "false", "'" + color.name() + "'");
    }
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("foreColor");
}

void WizDocumentWebView::editorCommandExecuteBold()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("bold");
    editorCommandExecuteCommand("bold");
}

void WizDocumentWebView::editorCommandExecuteItalic()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("italic");
    editorCommandExecuteCommand("italic");
}

void WizDocumentWebView::editorCommandExecuteUnderLine()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("underline");
    editorCommandExecuteCommand("underline");
}

void WizDocumentWebView::editorCommandExecuteStrikeThrough()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("strikethrough");
    editorCommandExecuteCommand("strikethrough");
}

void WizDocumentWebView::editorCommandExecuteJustifyLeft()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("justifyLeft");
    editorCommandExecuteCommand("JustifyLeft");
}

void WizDocumentWebView::editorCommandExecuteJustifyRight()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("justifyRight");
    editorCommandExecuteCommand("JustifyRight");
}

void WizDocumentWebView::editorCommandExecuteJustifyCenter()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("justifyCenter");
    editorCommandExecuteCommand("JustifyCenter");
}

void WizDocumentWebView::editorCommandExecuteJustifyJustify()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("justifyJustify");
    editorCommandExecuteCommand("justify", "'justify'");
}

void WizDocumentWebView::editorCommandExecuteInsertOrderedList()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("insertOrderedList");
    editorCommandExecuteCommand("insertOrderedList");
}

void WizDocumentWebView::editorCommandExecuteInsertUnorderedList()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("insertUnorderedList");
    editorCommandExecuteCommand("insertUnorderedList");
}

void WizDocumentWebView::editorCommandExecuteTableInsert(int row, int col)
{
    QString code = QString("WizEditor.table.insertTable(%1, %2);").arg(col).arg(row);
    //
    page()->runJavaScript(code);
}


void WizDocumentWebView::on_editorCommandExecuteScreenShot_imageAccepted(QPixmap pix)
{
    QObject *ssSender = qobject_cast<QObject*>(sender());
    if (ssSender)
        delete ssSender;

    setWindowVisibleOnScreenShot(true);

    if (pix.isNull())
        return;

    CString strFileName = noteResourcesPath() + WizIntToStr(WizGetTickCount()) + ".png";
    if (!pix.save(strFileName)) {
        TOLOG("ERROR: Can't save clipboard image to file");
        return;
    }

    insertImage(strFileName);
}

void WizDocumentWebView::on_editorCommandExecuteScreenShot_finished()
{
    QObject *ssSender = qobject_cast<QObject*>(sender());
    if (ssSender)
        delete ssSender;

    setWindowVisibleOnScreenShot(true);
}

void WizDocumentWebView::editorCommandExecuteInsertHorizontal()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("insertHorizontal");
    editorCommandExecuteCommand("InsertHorizontalRule");
}

void WizDocumentWebView::editorCommandExecuteInsertCheckList()
{
    QString strExec = "WizEditor.todo.setTodo();";
    page()->runJavaScript(strExec);

    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("insertCheckList");
}

void WizDocumentWebView::editorCommandExecuteInsertImage()
{
    static QString initPath = QDir::homePath();
    QStringList strImgFileList = QFileDialog::getOpenFileNames(0, tr("Image File"), initPath, tr("Images (*.png *.bmp *.gif *.jpg)"));
    if (strImgFileList.isEmpty())
        return;
    //
    QString strImagePath = noteResourcesPath();

    CWizStdStringArray files;
    foreach (QString strImgFile, strImgFileList)
    {
        QString destImageFileName = strImagePath + ::WizGenGUIDLowerCaseLetterOnly()
                + Utils::WizMisc::extractFileExt(strImgFile);
        //
        if (QFile::copy(strImgFile, destImageFileName))
        {
            files.push_back(destImageFileName);
        }
        //
        initPath = Utils::WizMisc::extractFilePath(strImgFile);
    }
    //
    CString param;
    WizStringArrayToText(files, param, "*");
    //
    QString script = QString("WizEditor.img.insertByPath('%1');").arg(param);
    page()->runJavaScript(script);

    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("insertImage");
}

void WizDocumentWebView::editorCommandExecuteStartMarkup()
{
    QString js = QString("WizEditor.marker.start();");
    page()->runJavaScript(js);
}

void WizDocumentWebView::editorCommandExecuteStopMarkup()
{
    QString js = QString("WizEditor.marker.quit();");
    page()->runJavaScript(js);
}

void WizDocumentWebView::editorExecJs(QString js)
{
    page()->runJavaScript(js);
}

void WizDocumentWebView::onViewMindMap(bool on)
{
    if (on) {
        QString title = view()->note().strTitle;
        QString js = QString("WizEditor.outline.showMinder(`%1`);").arg(title.replace("`", "\\`"));
        editorExecJs(js);
    } else {
        QString js = "WizEditor.outline.hideMinder();";
        editorExecJs(js);
    }
}

void WizDocumentWebView::editorCommandExecuteInsertPainter()
{
    view()->changeType("svgpainter");
    QString js = QString("WizEditor.createSvg()");
    page()->runJavaScript(js);
}


void WizDocumentWebView::editorCommandExecuteInsertDate()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("insertDate");
    //
    QString date = QDate::currentDate().toString(Qt::DefaultLocaleLongDate);
    editorCommandExecuteInsertHtml(date, false);
}

void WizDocumentWebView::editorCommandExecuteInsertTime()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("insertTime");
    //
    QString time = QTime::currentTime().toString(Qt::DefaultLocaleLongDate);
    editorCommandExecuteInsertHtml(time, false);
}

void WizDocumentWebView::editorCommandExecuteRemoveFormat()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("removeFormat");
    editorCommandExecuteCommand("removeFormat");
}

void WizDocumentWebView::editorCommandExecuteFormatPainterOn(bool multi)
{
    QString script = QString("WizEditor.formatPainter.on(%1);").arg(multi ? "true" : "false");
    //
    page()->runJavaScript(script, [=] (const QVariant& vRet) {
        if (vRet.type() == QVariant::Bool) {
            if (!vRet.toBool()) {
                //
                TOLOG("Can't stsart format painter");

            }
        }

    });
    //
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("formatPainter");
}

void WizDocumentWebView::editorCommandExecuteFormatPainterOff()
{
    QString script = QString("WizEditor.formatPainter.off();");
    //
    page()->runJavaScript(script, [=] (const QVariant& vRet) {
        if (vRet.type() == QVariant::Bool) {
            if (!vRet.toBool()) {
                //
                TOLOG("Can't stsart format painter");

            }
        }

    });
    //
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("formatPainter");
}

void WizDocumentWebView::editorCommandExecuteInsertCode()
{
    page()->runJavaScript("WizEditor.code.insertCode();");
    //
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("insertCode");
}

void WizDocumentWebView::editorCommandExecuteMobileImage(bool bReceiveImage)
{
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(m_app.mainWindow());
    if (bReceiveImage && m_app.userSettings().needShowMobileFileReceiverUserGuide())
    {
        mainWindow->showMobileFileReceiverUserGuide();
    }

    m_app.userSettings().setReceiveMobileFile(bReceiveImage);
    mainWindow->setMobileFileReceiverEnable(bReceiveImage);

    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("mobileImage");
}

void WizDocumentWebView::editorCommandExecuteScreenShot()
{
    WizScreenShotHelper* helper = new WizScreenShotHelper();

    connect(helper, SIGNAL(screenShotCaptured(QPixmap)),
            SLOT(on_editorCommandExecuteScreenShot_imageAccepted(QPixmap)));
    connect(helper, SIGNAL(shotScreenQuit()), SLOT(on_editorCommandExecuteScreenShot_finished()));

    setWindowVisibleOnScreenShot(false);
    QTimer::singleShot(200, helper, SLOT(startScreenShot()));

    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("screenShot");
}


void WizDocumentWebView::saveAsPDF()
{
    const QString styleId = "temp_mac_print_pdf";
    if (isDarkMode()) {
        //
        const QString styleWhite = "html, .wiz-editor-body {background-color:white !important;}";
        const QString scriptWhite = QString("WizEditor.insertCustomStyle('%1', '%2', true);WizEditor.nightMode.off();document.getElementsByName('wiz_dark_mode_style')[0].remove()").arg(styleId).arg(styleWhite);
        page()->runJavaScript(scriptWhite, [=](const QVariant&) {
            //
            saveAsPDFCore([=]{

                const QString scriptDark = QString("WizEditor.removeStyleById('%1');WizEditor.nightMode.on();").arg(styleId);
                page()->runJavaScript(scriptDark);
                //
            });
            //
        });

    } else {
        saveAsPDFCore([]{});
    }
}

void WizDocumentWebView::saveAsPDFCore(std::function<void()> callback)
{
    CString strTitle = view()->note().strTitle;
    WizMakeValidFileNameNoPath(strTitle);
    static QString strInitPath = QDir::homePath();
    QString strInitFileName = Utils::WizMisc::addBackslash2(strInitPath) + Utils::WizMisc::extractFileTitle(strTitle);
    //
    QString strFileName = QFileDialog::getSaveFileName(this, QString(),
                                                       strInitFileName,
                                                       tr("PDF Files (*.pdf)"));
    //
    if (strFileName.isEmpty())
        return;
    //
    strInitPath = Utils::WizMisc::extractFilePath(strFileName);
    //
    if (::WizPathFileExists(strFileName))
    {
        ::WizDeleteFile(strFileName);
    }
    //
    double marginTop = m_app.userSettings().printMarginValue(wizPositionTop);
    double marginBottom = m_app.userSettings().printMarginValue(wizPositionBottom);
    double marginLeft = m_app.userSettings().printMarginValue(wizPositionLeft);
    double marginRight = m_app.userSettings().printMarginValue(wizPositionRight);
    QMarginsF margins(marginLeft, marginTop, marginRight, marginBottom);
    //
    const QPageLayout layout = QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, margins);
    //
    page()->printToPdf([=](const QByteArray&data){
        ::WizSaveDataToFile(strFileName, data);
        //
        callback();
        //
    }, layout);
}

void WizDocumentWebView::saveAsHtml()
{
    CString strTitle = view()->note().strTitle;
    WizMakeValidFileNameNoPath(strTitle);
    strTitle = Utils::WizMisc::extractFileTitle(strTitle);
    //
    static QString strInitPath = QDir::homePath();
    QString strInitFileName = Utils::WizMisc::addBackslash2(strInitPath) + Utils::WizMisc::extractFileTitle(strTitle);
    //
    QString strIndexFileName = QFileDialog::getSaveFileName(this, QString(),
                                                       strInitFileName,
                                                       tr("Html Files (*.html)"));
    //
    if (strIndexFileName.isEmpty())
        return;
    //
    strInitPath = Utils::WizMisc::extractFilePath(strIndexFileName);
    //
    const WIZDOCUMENTDATA& doc = view()->note();
    WizDatabase& db = m_dbMgr.db(doc.strKbGUID);
    //
    if (!db.exportToHtmlFile(doc, strIndexFileName)) {
        return;
    }
    //
    if (WizIsMarkdownNote(doc))
    {
        QString strScript = QString("WizReader.getRenderDocument();");
        page()->runJavaScript(strScript, [=](const QVariant& vRet) {
            //
            QString strHtml = vRet.toString();
            //
            if (!strHtml.isEmpty())
            {
                QString fileTitle = Utils::WizMisc::extractFileTitle(strIndexFileName);
                QString resourcePath = fileTitle + "_files/";
                strHtml.replace("index_files/", resourcePath);
                //
                ::WizSaveUnicodeTextToUtf8File(strIndexFileName, strHtml, true);
            }
            //
        });

    }
}

void WizDocumentWebView::saveAsMarkdown()
{
    CString strTitle = view()->note().strTitle;
    WizMakeValidFileNameNoPath(strTitle);
    static QString strInitPath = QDir::homePath();
    QString strInitFileName = Utils::WizMisc::addBackslash2(strInitPath) + strTitle;
    //
    QString strIndexFileName = QFileDialog::getSaveFileName(this, QString(),
                                                       strInitFileName,
                                                       tr("Markdown Files (*.md)"));
    //
    if (strIndexFileName.isEmpty())
        return;
    //
    strInitPath = Utils::WizMisc::extractFilePath(strIndexFileName);
    //
    if (::WizPathFileExists(strIndexFileName))
    {
        ::WizDeleteFile(strIndexFileName);
    }
    //
    const WIZDOCUMENTDATA& doc = view()->note();
    WizDatabase& db = m_dbMgr.db(doc.strKbGUID);
    //
    if (!db.exportToHtmlFile(doc, strIndexFileName)) {
        return;
    }
    /*
     因为存在内置的表格，todo，图片，导致无法正常输出成标准的markdown
    */
    page()->runJavaScript(QString("WizEditor.getMarkdownSrc({unEscapeHtml: true});"), [=](const QVariant& vModified){
        //
        QString source = vModified.toString();
        //
        QString fileTitle = Utils::WizMisc::extractFileTitle(strIndexFileName);

        QString strResFolder = fileTitle.toHtmlEscaped() + "_files/";
        source.replace("index_files/", strResFolder);
        //
        ::WizSaveUnicodeTextToUtf8File(strIndexFileName, source, false);
    });
}

void WizDocumentWebView::isModified(std::function<void(bool modified)> callback)
{
    if (m_bContentsChanged)
    {
        callback(true);
        return;
    }
    page()->runJavaScript(QString("WizEditor.isModified();"), [=](const QVariant& vModified){
        //
        callback(vModified.toBool());

    });
}

void WizDocumentWebView::setModified(bool b)
{
    m_bContentsChanged = b;
    page()->runJavaScript("WizEditor.setUnModified()");
}

void WizDocumentWebView::undo()
{
    page()->runJavaScript("WizEditor.undo()");
}

void WizDocumentWebView::redo()
{
    page()->runJavaScript("WizEditor.redo()");
}

QString WizDocumentWebView::getUserGuid()
{
    return m_dbMgr.db().getUserGuid();
}


QString WizDocumentWebView::getUserAvatarFilePath()
{
    int size = 24;
    QString strFileName;
    QString strUserID = m_dbMgr.db().getUserId();
    if (WizAvatarHost::customSizeAvatar(strUserID, size, size, strFileName))
        return strFileName;


    return QString();
}

QString WizDocumentWebView::getUserAlias()
{
    QString strKbGUID = view()->note().strKbGUID;
    return m_dbMgr.db(strKbGUID).getUserAlias();
}


bool WizDocumentWebView::isPersonalDocument()
{
    QString strKbGUID = view()->note().strKbGUID;
    QString dbKbGUID = m_dbMgr.db().kbGUID();
    return strKbGUID.isEmpty() || (strKbGUID == dbKbGUID);
}

QString WizDocumentWebView::getCurrentNoteHtml()
{
    return m_currentNoteHtml;
}


void copyFileToFolder(const QString& strFileFolder, const QString& strIndexFile, \
                         const QStringList& strResourceList)
{
    //copy index file
    QString strFolderIndex = strFileFolder + "index.html";
    if (strIndexFile != strFolderIndex)
    {
        QFile::remove(strFolderIndex);
        QFile::copy(strIndexFile, strFolderIndex);
    }

    //copy resources to temp folder
    QString strResourcePath = strFileFolder + "index_files/";
    for (int i = 0; i < strResourceList.count(); i++)
    {
        if (QFile::exists(strResourceList.at(i)))
        {
            QFile::copy(strResourceList.at(i), strResourcePath + Utils::WizMisc::extractFileName(strResourceList.at(i)));
        }
    }
}

bool WizDocumentWebView::hasEditPermissionOnCurrentNote()
{
    WIZDOCUMENTDATA docData = view()->note();
    WizDatabase& db = m_dbMgr.db(docData.strKbGUID);
    return db.canEditDocument(docData) && !WizDatabase::isInDeletedItems(docData.strLocation);
}

void WizDocumentWebView::changeCurrentDocumentType(const QString &strType)
{
    WIZDOCUMENTDATA docData = view()->note();
    WizDatabase& db = m_dbMgr.db(docData.strKbGUID);
    docData.strType = strType;
    db.modifyDocumentInfoEx(docData);
}

bool WizDocumentWebView::checkListClickable()
{
    WizDocumentView* v = view();
    if (!m_dbMgr.db(v->note().strKbGUID).isGroup())
    {
        emit clickingTodoCallBack(false, false);
        return true;
    }

    if (v->checkListClickable())
    {
        emit clickingTodoCallBack(false, false);
        v->setStatusToEditingByCheckList();
        return true;
    }
    emit clickingTodoCallBack(true, true);
    return false;
}

bool WizDocumentWebView::shouldAddCustomCSS()
{
    const WIZDOCUMENTDATA& data = view()->note();
    // 通过网页剪辑的笔记不添加自定义的样式
    bool styledNote = data.strURL.startsWith("http");
    if (styledNote)
        return false;

    bool isMarkdown = WizIsMarkdownNote(data) && !view()->isEditing();

    return !isMarkdown;
}

bool WizDocumentWebView::canRenderMarkdown()
{
    const WIZDOCUMENTDATA& doc = view()->note();

    if (view()->isEditing())
        return false;

    if (doc.strTitle.indexOf(".md") == -1 && doc.strTitle.indexOf(".mj") == -1)
        return false;

    int nPointPos = doc.strTitle.length() - 3;
    if (doc.strTitle.lastIndexOf(".md") == nPointPos || doc.strTitle.lastIndexOf(".mj") == nPointPos)
        return true;

    if (doc.strTitle.indexOf(".md ") != -1 || doc.strTitle.indexOf(".md@") != -1 ||
            doc.strTitle.indexOf(".mj ") != -1|| doc.strTitle.indexOf(".mj@") != -1)
        return true;

    return false;
}

bool WizDocumentWebView::canEditNote()
{
    return view()->isEditing();
}

//编辑器初始化时使用
QString WizDocumentWebView::getLocalLanguage()
{
    QLocale locale;
    if (locale.language() == QLocale::Chinese)
    {
        if (locale.country() == QLocale::China)
            return "zh-cn";
        else if (locale.country() == QLocale::Taiwan)
            return "zh-tw";
    }

    return "en";
}

void WizDocumentWebView::onSelectionChange(const QString& currentStyle)
{
    Q_EMIT statusChanged(currentStyle);
}

void WizDocumentWebView::onClickedSvg(const QString& data)
{
    const WIZDOCUMENTDATA note = view()->note();
    //
    trySaveDocument(note, true, [=] (const QVariant) {
        //
        ::WizExecuteOnThread(WIZ_THREAD_MAIN, [=] {
            //
            editHandwritingNote(m_dbMgr, note, m_strNoteHtmlFileName, data, WizMainWindow::instance());
            addDefaultScriptsToDocumentHtml(m_strNoteHtmlFileName);
            load(QUrl::fromLocalFile(m_strNoteHtmlFileName));
        });
    });

}

void WizDocumentWebView::updateSvg(QString data)
{
    //
    data = data.replace("\\", "\\\\");
    QString js = QString("WizEditor.replaceSvg(`%1`)").arg(data);
    page()->runJavaScript(js);
    //
}


void WizDocumentWebView::saveCurrentNote()
{
    WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{
        //
        onTimerAutoSaveTimout();
        //
    });
}

void WizDocumentWebView::onReturn()
{
    WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{
        //
        tryResetTitle();
        //
    });
}

void WizDocumentWebView::doCopy()
{
    page()->triggerAction(QWebEnginePage::Copy, false);
}

void WizDocumentWebView::doPaste()
{
    WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{
        //
        onPasteCommand();
        //
    });
}

void WizDocumentWebView::afterCopied()
{
    WizWebEnginePage::processCopiedData();
}


void WizDocumentWebView::onMarkerUndoStatusChanged(QString data)
{
    emit markerUndoStatusChanged(data);

}
void WizDocumentWebView::onMarkerInitiated(QString data)
{
    emit markerInitiated(data);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////

WizDocumentWebViewLoaderThread::WizDocumentWebViewLoaderThread(WizDatabaseManager &dbMgr, QObject *parent)
    : QThread(parent)
    , m_dbMgr(dbMgr)
    , m_stop(false)
    , m_editorMode(modeReader)
{
}

void WizDocumentWebViewLoaderThread::load(const WIZDOCUMENTDATA &doc, WizEditorMode editorMode)
{
    setCurrentDoc(doc.strKbGUID, doc.strGUID, editorMode);

    if (!isRunning())
    {
        start();
    }
}
void WizDocumentWebViewLoaderThread::stop()
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    //
    m_stop = true;
    //
    m_waitEvent.wakeAll();
}
void WizDocumentWebViewLoaderThread::waitForDone()
{
    stop();
    //
    WizWaitForThread(this);
}

void WizDocumentWebViewLoaderThread::run()
{
    while (!m_stop)
    {
        if (m_stop)
            return;
        //
        QString kbGuid;
        QString docGuid;
        WizEditorMode editorMode = modeReader;
        peekCurrentDocGuid(kbGuid, docGuid, editorMode);
        if (m_stop)
            return;
        //
        if (docGuid.isEmpty())
            continue;
        //
        WizDatabase& db = m_dbMgr.db(kbGuid);
        WIZDOCUMENTDATA data;
        if (!db.documentFromGuid(docGuid, data))
        {
            continue;
        }
        //
        WizDocumentDataLocker locker(data.strGUID);
        //
        QString strHtmlFile;
        if (db.documentToTempHtmlFile(data, strHtmlFile))
        {
            emit loaded(kbGuid, docGuid, strHtmlFile, editorMode);
        }
        else
        {
            ::WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{
                //
                QMessageBox::critical(WizMainWindow::instance(), tr("Error"), tr("Can't view note: (Can't unzip note data)"));
                //
            });
        }
    }
}

void WizDocumentWebViewLoaderThread::setCurrentDoc(QString kbGUID, QString docGUID, WizEditorMode editorMode)
{
    //
    {
        QMutexLocker locker(&m_mutex);
        Q_UNUSED(locker);
        //
        m_strCurrentKbGUID = kbGUID;
        m_strCurrentDocGUID = docGUID;
        m_editorMode = editorMode;
    }
    //
    //
    m_waitEvent.wakeAll();
}

bool WizDocumentWebViewLoaderThread::isEmpty()
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    //
    return m_strCurrentDocGUID.isEmpty();
}

void WizDocumentWebViewLoaderThread::peekCurrentDocGuid(QString& kbGUID, QString& docGUID, WizEditorMode& editorMode)
{
    if (isEmpty())
    {
        m_waitEvent.wait();
    }
    //
    //
    {
        QMutexLocker locker(&m_mutex);
        Q_UNUSED(locker);
        //
        kbGUID = m_strCurrentKbGUID;
        docGUID = m_strCurrentDocGUID;
        editorMode = m_editorMode;
        //
        m_strCurrentKbGUID.clear();
        m_strCurrentDocGUID.clear();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////



WizDocumentWebViewSaverThread::WizDocumentWebViewSaverThread(WizDatabaseManager &dbMgr, QObject *parent)
    : QThread(parent)
    , m_dbMgr(dbMgr)
    , m_stop(false)
{
}

void WizDocumentWebViewSaverThread::save(const WIZDOCUMENTDATA& doc, const QString& strHtml,
                                          const QString& strHtmlFile, int nFlags)
{
    SAVEDATA data;
    data.doc = doc;
    data.html = strHtml;
    data.htmlFile = strHtmlFile;
    data.flags = nFlags;
    //
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    //
    m_arrayData.push_back(data);
    //
    m_waitEvent.wakeAll();

    if (!isRunning())
    {
        start();
    }
}
void WizDocumentWebViewSaverThread::waitForDone()
{
    stop();
    //
    WizWaitForThread(this);
}

void WizDocumentWebViewSaverThread::stop()
{
    m_stop = true;
    m_waitEvent.wakeAll();
}

bool WizDocumentWebViewSaverThread::isEmpty()
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    //
    return m_arrayData.empty();
}

WizDocumentWebViewSaverThread::SAVEDATA WizDocumentWebViewSaverThread::peekFirst()
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    //
    SAVEDATA data = m_arrayData[0];
    m_arrayData.erase(m_arrayData.begin());
    return data;
}

void WizDocumentWebViewSaverThread::peekData(SAVEDATA& data)
{
    while (1)
    {
        if (m_stop)
            return;
        //
        if (isEmpty())
        {
            m_waitEvent.wait();
        }
        //
        if (isEmpty())
        {
            if (m_stop)
                return;
            //
            continue;
        }
        //
        data = peekFirst();
        //
        break;
    }
}

void WizDocumentWebViewSaverThread::run()
{
    while (true)
    {
        SAVEDATA data;
        peekData(data);
        //
        if (data.doc.strGUID.isEmpty())
        {
            if (m_stop)
                return;
            //
            continue;
        }
        //
        WizDatabase& db = m_dbMgr.db(data.doc.strKbGUID);
        //
        WIZDOCUMENTDATA doc;
        if (!db.documentFromGuid(data.doc.strGUID, doc))
        {
            qDebug() << "fault error: can't find doc in database: " << doc.strGUID;
            continue;
        }
        //
        qDebug() << "Saving note: " << doc.strTitle;

        WizDocumentDataLocker locker(doc.strGUID);
        //
        bool notify = false;    //don't notify
        bool ok = db.updateDocumentData(doc, data.html, data.htmlFile, data.flags, notify);

        //
        if (ok)
        {
            qDebug() << "Save note done: " << doc.strTitle;
        }
        else
        {
            qDebug() << "Save note failed: " << doc.strTitle;
        }

        QString kbGuid = db.isGroup() ? db.kbGUID() : "";
        emit saved(kbGuid, doc.strGUID, ok);
    }
}

