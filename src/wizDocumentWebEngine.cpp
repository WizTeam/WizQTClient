#include "wizDocumentWebEngine.h"
#include "wizDocumentWebView.h"
#include <QRunnable>
#include <QThreadPool>
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
#include <QWebChannel>
#include <QWebEngineSettings>
#include <QtWebSockets/QWebSocketServer>

#include <QApplication>
#include <QUndoStack>
#include <QDesktopServices>

#ifdef Q_OS_MAC
#include <QMacPasteboardMime>
#endif

#include <coreplugin/icore.h>

#include "wizdef.h"
#include "share/wizmisc.h"
#include "wizmainwindow.h"
#include "wizEditorInsertLinkForm.h"
#include "wizEditorInsertTableForm.h"
#include "share/wizObjectDataDownloader.h"
#include "share/websocketclientwrapper.h"
#include "share/websockettransport.h"
#include "wizDocumentTransitionView.h"
#include "share/wizDatabaseManager.h"
#include "wizDocumentView.h"
#include "wizSearchReplaceWidget.h"
#include "widgets/WizCodeEditorDialog.h"
#include "widgets/wizScreenShotWidget.h"
#include "widgets/wizEmailShareDialog.h"

#include "utils/pathresolve.h"
#include "utils/logger.h"

#include "mac/wizmachelper.h"

using namespace Core;
using namespace Core::Internal;


enum WizLinkType {
    WizLink_Doucment,
    WizLink_Attachment
};


//CWizDocumentWebEngine::CWizDocumentWebEngine(CWizExplorerApp& app, QWidget* parent)
//    : QWebEngineView(parent)
//    , m_app(app)
//    , m_dbMgr(app.databaseManager())
//    , m_bEditorInited(false)
//    , m_bNewNote(false)
//    , m_bNewNoteTitleInited(false)
//    , m_bCurrentEditing(false)
//    , m_bContentsChanged(false)
//    , m_searchReplaceWidget(0)
//{

////#ifdef QT_DEBUG
////    settings()->globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
////#endif

////    connect(page, SIGNAL(actionTriggered(QWebEnginePage::WebAction)), SLOT(onActionTriggered(QWebEnginePage::WebAction)));

////    QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
////    QString location = Utils::PathResolve::tempPath();
////    diskCache->setCacheDirectory(location);
////    page->networkAccessManager()->setCache(diskCache);


//    // minimum page size hint
//    setMinimumSize(400, 250);

//    // only accept focus by mouse click as the best way to trigger toolbar reset
//    setFocusPolicy(Qt::ClickFocus);
//    setAttribute(Qt::WA_AcceptTouchEvents, false);

//    // FIXME: should accept drop picture, attachment, link etc.
//    setAcceptDrops(true);

//    // refers
//    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());

//    m_transitionView = mainWindow->transitionView();

//    m_docLoadThread = new CWizDocumentWebViewLoaderThread(m_dbMgr);
//    connect(m_docLoadThread, SIGNAL(loaded(const QString&, const QString, const QString)),
//            SLOT(onDocumentReady(const QString&, const QString, const QString)), Qt::QueuedConnection);
//    //
//    m_docSaverThread = new CWizDocumentWebViewSaverThread(m_dbMgr);
//    connect(m_docSaverThread, SIGNAL(saved(const QString, const QString,bool)),
//            SLOT(onDocumentSaved(const QString, const QString,bool)), Qt::QueuedConnection);

//    // loading and saving thread
//    m_timerAutoSave.setInterval(5*60*1000); // 5 minutes
//    connect(&m_timerAutoSave, SIGNAL(timeout()), SLOT(onTimerAutoSaveTimout()));
//}

CWizDocumentWebEngine::CWizDocumentWebEngine(CWizExplorerApp& app, QWidget* parent)
    : QWebEngineView(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_bEditorInited(false)
    , m_bNewNote(false)
    , m_bNewNoteTitleInited(false)
    , m_bCurrentEditing(false)
    , m_bContentsChanged(false)
    , m_searchReplaceWidget(0)
{

    WebEnginePage *webPage = new WebEnginePage(this);
    setPage(webPage);

    // FIXME: should accept drop picture, attachment, link etc.
    setAcceptDrops(true);

    // refers
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());

    m_transitionView = mainWindow->transitionView();

    m_docLoadThread = new CWizDocumentWebViewLoaderThread(m_dbMgr);
    connect(m_docLoadThread, SIGNAL(loaded(const QString&, const QString, const QString)),
            SLOT(onDocumentReady(const QString&, const QString, const QString)), Qt::QueuedConnection);
    //
    m_docSaverThread = new CWizDocumentWebViewSaverThread(m_dbMgr);
    connect(m_docSaverThread, SIGNAL(saved(const QString, const QString,bool)),
            SLOT(onDocumentSaved(const QString, const QString,bool)), Qt::QueuedConnection);

    // loading and saving thread
    m_timerAutoSave.setInterval(5*60*1000); // 5 minutes
    connect(&m_timerAutoSave, SIGNAL(timeout()), SLOT(onTimerAutoSaveTimout()));
}

CWizDocumentWebEngine::~CWizDocumentWebEngine()
{
    if (m_searchReplaceWidget)
        delete m_searchReplaceWidget;
}
void CWizDocumentWebEngine::waitForDone()
{
    if (m_docLoadThread) {
        m_docLoadThread->waitForDone();
    }
    if (m_docSaverThread) {
        m_docSaverThread->waitForDone();
    }
}


//void CWizDocumentWebEngine::inputMethodEvent(QInputMethodEvent* event)
//{
//    // On X windows, fcitx flick while preediting, only update while webview end process.
//    // maybe it's a QT-BUG?
//#ifdef Q_OS_LINUX
//    setUpdatesEnabled(false);
//    QWebEngineView::inputMethodEvent(event);
//    setUpdatesEnabled(true);
//#else
//    QWebEngineView::inputMethodEvent(event);
//#endif

//#ifdef Q_OS_MAC
//    /*
//    ///暂时注释代码，移动输入光标会导致极高的CPU占用率，导致输入卡顿。

//    //int nLength = 0;
//    int nOffset = 0;
//    for (int i = 0; i < event->attributes().size(); i++) {
//        const QInputMethodEvent::Attribute& a = event->attributes().at(i);
//        if (a.type == QInputMethodEvent::Cursor) {
//            //nLength = a.length;
//            nOffset = a.start;
//            break;
//        }
//    }

//    // Move cursor
//    // because every time input method event triggered, old preedit text will be
//    // deleted and the new one inserted, this action made cursor restore to the
//    // beginning of the input context, move it as far as offset indicated after
//    // default implementation should correct this issue!!!
//    for (int i = 0; i < nOffset; i++) {
//        page()->triggerAction(QWebEnginePage::MoveToNextChar);
//    }
//    */
//#endif // Q_OS_MAC
//}

/*
void CWizDocumentWebEngine::keyPressEvent(QKeyEvent* event)
{
//    // special cases process
//    if (event->key() == Qt::Key_Escape)
//    {
//        // FIXME: press esc will insert space at cursor if not clear focus
//        clearFocus();
//        return;
//    }
//    else if (event->key() == Qt::Key_S
//             && event->modifiers() == Qt::ControlModifier)
//    {
//        saveDocument(view()->note(), false);
//        return;
//    }
//    else if (event->key() == Qt::Key_A && event->modifiers() == Qt::ControlModifier && !isEditing())
//    {
//        //阅读模式下selectall无法触发，强制触发阅读模式下的selectall。
//        emit selectAllKeyPressed();
//        return;
//    }
//    else if (event->key() == Qt::Key_Tab)
//    {
//        //set contentchanged
//        setContentsChanged(true);
//        emit statusChanged();
//        return;
//    }
//#if QT_VERSION < 0x050000
//    #ifdef Q_OS_MAC
//    else if (event->key() == Qt::Key_Z)
//    {
//        //Ctrl+Shift+Z,  shortcut for redo can't catch by actions in QT4
//        Qt::KeyboardModifiers keyMod = QApplication::keyboardModifiers();
//        bool isSHIFT = keyMod.testFlag(Qt::ShiftModifier);
//        bool isCTRL = keyMod.testFlag(Qt::ControlModifier);
//        if (isCTRL && isSHIFT) {
//            redo();
//            return;
//        } else if (isCTRL && !isSHIFT) {
//            undo();
//            return;
//        }
//    }
//    #endif

//#endif

////    int keyValue = event->key();
////    QString keyText = event->text();
////    qDebug() << keyValue << " text : " << keyText;

//#ifdef Q_OS_LINUX
//    setUpdatesEnabled(false);
//    QWebEngineView::keyPressEvent(event);
//    setUpdatesEnabled(true);
//#else

//    if (event->key() == Qt::Key_Backspace)
//    {
//        if (event->modifiers() == Qt::ControlModifier)
//        {
//            editorCommandExecuteRemoveStartOfLine();
//            return;
//        }
//        else
//        {
//            //FIXME: would not trigger content change event, when delete row and image by backspace
//            setContentsChanged(true);
//        }
//    }

//    //special handled for qt4,case capslock doesn't work
//#if QT_VERSION < 0x050000
////    if (65 <= keyValue && 90 >= keyValue)
////    {
////        if (event->key() & Qt::Key_CapsLock)
////        {
////            qDebug() << "capslock pressed";
////            QKeyEvent newKeyEvent(event->type(), keyValue, event->modifiers(),
////                                  keyText.toUpper(), event->isAutoRepeat(), event->count());
////            QWebEngineView::keyPressEvent(&newKeyEvent);
////            return;
////        }
////    }
//#endif
//    QWebEngineView::keyPressEvent(event);
//#endif

//    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
//        tryResetTitle();
//    }
}

void CWizDocumentWebEngine::focusInEvent(QFocusEvent *event)
{
    if (m_bEditingMode) {
        Q_EMIT focusIn();
        Q_EMIT statusChanged();
    }

    QWebEngineView::focusInEvent(event);

    applySearchKeywordHighlight();
}

void CWizDocumentWebEngine::focusOutEvent(QFocusEvent *event)
{
    // because qt will clear focus when context menu popup, we need keep focus there.
    if (event->reason() == Qt::PopupFocusReason) {
        return;
    }

    Q_EMIT focusOut();
    Q_EMIT statusChanged();
    QWebEngineView::focusOutEvent(event);
}

//void CWizDocumentWebEngine::contextMenuEvent(QContextMenuEvent *event)
//{
//    if (!m_bEditorInited)
//        return;

//    Q_EMIT requestShowContextMenu(mapToGlobal(event->pos()));
//}

void CWizDocumentWebEngine::dragEnterEvent(QDragEnterEvent *event)
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

void CWizDocumentWebEngine::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS)) {
            if (!event->mimeData()->data(WIZNOTE_MIMEFORMAT_DOCUMENTS).isEmpty()) {
                setFocus();
                event->acceptProposedAction();
            }
    }
}
*/

void CWizDocumentWebEngine::onActionTriggered(QWebEnginePage::WebAction act)
{
    if (act == QWebEnginePage::Paste)
        tryResetTitle();
}

void CWizDocumentWebEngine::tryResetTitle()
{
    if (m_bNewNoteTitleInited)
        return;

    // if note already modified, maybe title changed by use manuallly
    if (view()->note().tCreated.secsTo(view()->note().tModified) != 0)
        return;


//    QWebFrame* f = noteFrame();
//    if (!f)
//        return;

//    // remove baidu bookmark
//    QWebElement docPElement = f->documentElement().findFirst("body").findFirst("p");
//    QWebElement docSpanElement = docPElement.findFirst("span");
//    QString spanClass = docSpanElement.attribute("id");
//    if (spanClass.indexOf("baidu_bookmark") != -1)
//    {
//        docPElement.removeFromDocument();
//    }

    QString strTitle = getDataByJavaScriptFromPage<QString>("editor.getPlainTxt();");
//    strTitle = str2title(strTitle.left(255));
    if (strTitle.isEmpty())
        return;

    view()->resetTitle(strTitle);

    m_bNewNoteTitleInited = true;
}

//void CWizDocumentWebEngine::dropEvent(QDropEvent* event)
//{
//    const QMimeData* mimeData = event->mimeData();

//    int nAccepted = 0;
//    if (mimeData->hasUrls())
//    {
//        QList<QUrl> li = mimeData->urls();
//        QList<QUrl>::const_iterator it;
//        for (it = li.begin(); it != li.end(); it++) {
//            QUrl url = *it;
//            url.setScheme(0);

//            //paste image file as images, add attachment for other file
//            QString strFileName = url.toString();
//#ifdef Q_OS_MAC
//            if (wizIsYosemiteFilePath(strFileName))
//            {
//                strFileName = wizConvertYosemiteFilePathToNormalPath(strFileName);
//            }
//#endif
//            QImageReader reader(strFileName);
//            if (reader.canRead())
//            {
//                QString strHtml;
//                bool bUseCopyFile = true;
//                if (WizImage2Html(strFileName, strHtml, bUseCopyFile)) {
//                    editorCommandExecuteInsertHtml(strHtml, true);
//                    nAccepted++;
//                }
//            }
//            else
//            {
//                WIZDOCUMENTDATA doc = view()->note();
//                CWizDatabase& db = m_dbMgr.db(doc.strKbGUID);
//                WIZDOCUMENTATTACHMENTDATA data;
//                data.strKbGUID = doc.strKbGUID; // needed by under layer
//                if (!db.AddAttachment(doc, strFileName, data))
//                {
//                    TOLOG1("[drop] add attachment failed %1", strFileName);
//                    continue;
//                }
//                nAccepted ++;
//            }
//        }
//    }
//    else if (mimeData->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS))
//    {
//        QString strData(mimeData->data(WIZNOTE_MIMEFORMAT_DOCUMENTS));
//        if (!strData.isEmpty())
//        {
//            QString strLinkHtml;
//            QStringList doclist = strData.split(';');
//            foreach (QString doc, doclist) {
//                QStringList docIds = doc.split(':');
//                if (docIds.count() == 2)
//                {
//                    WIZDOCUMENTDATA document;
//                    CWizDatabase& db = m_dbMgr.db(docIds.first());
//                    if (db.DocumentFromGUID(docIds.last(), document))
//                    {
//                        QString strHtml, strLink;
//                        db.DocumentToHtmlLink(document, strHtml, strLink);
//                        strLinkHtml += "<p>" + strHtml + "</p>";
//                    }
//                }
//            }

//            editorCommandExecuteInsertHtml(strLinkHtml, false);

//            nAccepted ++;
//        }
//    }

//    if (nAccepted > 0) {
//        event->accept();
//        saveDocument(view()->note(), false);
//    }
//}

CWizDocumentView* CWizDocumentWebEngine::view()
{
    QWidget* pParent = parentWidget();
    while(pParent) {
        if (CWizDocumentView* view = dynamic_cast<CWizDocumentView*>(pParent)) {
            return view;
        }

        pParent = pParent->parentWidget();
    }

    return 0;
}

void CWizDocumentWebEngine::onTimerAutoSaveTimout()
{
    saveDocument(view()->note(), false);
}

void CWizDocumentWebEngine::onTitleEdited(QString strTitle)
{
    WIZDOCUMENTDATA document = view()->note();
    document.strTitle = strTitle;
    // Only sync when contents unchanged. If contents changed would sync after document saved.
    if (!isContentsChanged())
    {
        MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());
        mainWindow->quickSyncKb(document.strKbGUID);
    }
}

void CWizDocumentWebEngine::onDocumentReady(const QString kbGUID, const QString strGUID, const QString strFileName)
{

    qDebug() << "onDocumentReady";
    m_mapFile.insert(strGUID, strFileName);

    if (m_bEditorInited) {
        resetCheckListEnvironment();
        viewDocumentInEditor(m_bEditingMode);
    } else {
        initEditor();
    }
}

void CWizDocumentWebEngine::onDocumentSaved(const QString kbGUID, const QString strGUID, bool ok)
{
    if (!ok)
    {
        TOLOG("Save document failed");
    }
    //
    view()->sendDocumentSavedSignal(strGUID, kbGUID);
    //
    MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());
    mainWindow->quickSyncKb(kbGUID);
}

void CWizDocumentWebEngine::viewDocument(const WIZDOCUMENTDATA& doc, bool editing)
{
    if (!m_bEditorInited)
    {
        initEditor();
    }
    return;

    // set data
    m_bEditingMode = editing;
    m_bNewNote = doc.tCreated.secsTo(QDateTime::currentDateTime()) <= 1 ? true : false;
    m_bNewNoteTitleInited = m_bNewNote ? false : true;
    //
    setContentsChanged(false);

    if (m_bNewNote)
    {
        setEditorEnable(true);
        setFocus();
    }

    // ask extract and load
    m_docLoadThread->load(doc);
}

void CWizDocumentWebEngine::reloadNoteData(const WIZDOCUMENTDATA& data)
{
    Q_ASSERT(!data.strGUID.isEmpty());

    // reset only if user not in editing mode
    if (m_bEditingMode && hasFocus())
        return;

    // reload may triggered when update from server or locally reflected by modify
    m_docLoadThread->load(data);
}

QString CWizDocumentWebEngine::getDefaultCssFilePath() const
{
    return m_strDefaultCssFilePath;
}

bool CWizDocumentWebEngine::resetDefaultCss()
{
    QFile f(":/default.css");
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "[Editor]Failed to get default css code";
        return false;
    }

    QTextStream ts(&f);
    QString strCss = ts.readAll();
    f.close();

    QString strFont = m_app.userSettings().defaultFontFamily();
    int nSize = m_app.userSettings().defaultFontSize();

    strCss.replace("/*default-font-family*/", QString("font-family:%1;").arg(strFont));
    strCss.replace("/*default-font-size*/", QString("font-size:%1px;").arg(nSize));
    strCss.replace("/*default-background-color*/", QString("background-color:%1;").arg(
                   m_app.userSettings().editorBackgroundColor()));

    QString strPath = Utils::PathResolve::cachePath() + "editor/"+m_dbMgr.db().GetUserId()+"/";
    Utils::PathResolve::ensurePathExists(strPath);

    m_strDefaultCssFilePath = strPath + "default.css";

    QFile f2(m_strDefaultCssFilePath);
    if (!f2.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        qDebug() << "[Editor]Fail to setup default css code";
        return false;
    }

    f2.write(strCss.toUtf8());
    f2.close();

    return true;
}

void CWizDocumentWebEngine::editorResetFont()
{
    resetDefaultCss();
    page()->runJavaScript("updateCss();");
}

void CWizDocumentWebEngine::editorFocus()
{
    page()->runJavaScript("editor.focus();");
    emit focusIn();
}

void CWizDocumentWebEngine::setEditorEnable(bool enalbe)
{
    if (enalbe)
    {
        page()->runJavaScript("editor.setEnabled();");
        setFocus();
    }
    else
    {
        //    page()->runJavaScript("editor.reset();");
        page()->runJavaScript("editor.setDisabled();");
        clearFocus();
    }
}

bool CWizDocumentWebEngine::evaluateJavaScript(const QString& js)
{
    page()->runJavaScript(js);
    return true;
}

void CWizDocumentWebEngine::initEditor()
{
    qDebug() << "initEditor called";
    if (m_bEditorInited)
        return;

    if (!resetDefaultCss())
        return;


    QString strFileName = Utils::PathResolve::resourcesPath() + "files/editor/index.html";
    QString strHtml;
    ::WizLoadUnicodeTextFromFile(strFileName, strHtml);
    QUrl url = QUrl::fromLocalFile(strFileName);

    page()->settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
    page()->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    connect(page(), SIGNAL(loadFinished(bool)), SLOT(onEditorLoadFinished(bool)));

//    page()->setLinkDelegationPolicy(QWebEnginePage::DelegateAllLinks);

//    connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
//            SLOT(onEditorPopulateJavaScriptWindowObject()));

//    connect(page()->mainFrame(), SIGNAL(loadFinished(bool)),
//            SLOT(onEditorLoadFinished(bool)));

//    connect(page(), SIGNAL(linkClicked(const QUrl&)),
//            SLOT(onEditorLinkClicked(const QUrl&)));

//    connect(page(), SIGNAL(selectionChanged()),
//            SLOT(onEditorSelectionChanged()));

//    connect(page(), SIGNAL(contentsChanged()),
//            SLOT(onEditorContentChanged()));

    page()->setHtml(strHtml,url);
//    page()->setHtml("http://ueditor.baidu.com/website/onlinedemo.html");
//    load(url);
}

void CWizDocumentWebEngine::registerJavaScriptWindowObject()
{
    QWebSocketServer *server = new QWebSocketServer(QStringLiteral("Wiz Socket Server"), QWebSocketServer::NonSecureMode, this);
    if (!server->listen(QHostAddress::LocalHost, 0)) {
        qFatal("Failed to open web socket server.");
        return;
    }

    // wrap WebSocket clients in QWebChannelAbstractTransport objects
    WebSocketClientWrapper *clientWrapper  = new WebSocketClientWrapper(server, this);

    // setup the dialog and publish it to the QWebChannel
    QWebChannel *webChannel = new QWebChannel(this);
    // setup the channel
    QObject::connect(clientWrapper, &WebSocketClientWrapper::clientConnected,
                     webChannel, &QWebChannel::connectTo);
    webChannel->registerObject(QStringLiteral("WizEditor"), this);

    QString strUrl = server->serverUrl().toString();
    page()->runJavaScript(QString("initializeJSObject('%1');").arg(strUrl));
}

void CWizDocumentWebEngine::resetCheckListEnvironment()
{
    if (!m_bEditingMode)
    {
        QString strScript = QString("WizTodoReadChecked.clear();");
        page()->runJavaScript(strScript);
    }
}

void CWizDocumentWebEngine::initCheckListEnvironment()
{
    if (m_bEditingMode)
    {
        QString strScript = QString("WizTodo.init('qt');");
        page()->runJavaScript(strScript);
    }
    else
    {
        QString strScript = QString("WizTodoReadChecked.init('qt');");
        page()->runJavaScript(strScript);
    }
}

void CWizDocumentWebEngine::speakHelloWorld()
{
    qDebug() << "Hello world from web engine";
}

void CWizDocumentWebEngine::setWindowVisibleOnScreenShot(bool bVisible)
{
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    if (mainWindow)
    {
        bVisible ? mainWindow->show() : mainWindow->hide();
    }
}

bool CWizDocumentWebEngine::insertImage(const QString& strFileName, bool bCopyFile)
{
    QString strHtml;
    if (WizImage2Html(strFileName, strHtml, bCopyFile)) {
        return editorCommandExecuteInsertHtml(strHtml, true);
    }
    return false;
}

QString CWizDocumentWebEngine::getStringDataByJavaScriptFromPage(const QString& scriptSource)
{
   QString result;
   page()->runJavaScript(scriptSource, [&result](const QVariant& returnValue){ result = returnValue.toString(); });
   return result;
}

bool CWizDocumentWebEngine::shareNoteByEmail()
{
    CWizEmailShareDialog dlg(m_app);
    dlg.setNote(view()->note());

    dlg.exec();

    return true;
}

void CWizDocumentWebEngine::onEditorLoadFinished(bool ok)
{

    if (ok)
    {
        registerJavaScriptWindowObject();
        m_bEditorInited = true;
//        viewDocumentInEditor(m_bEditingMode);
        return;
    }

    TOLOG("Wow, loading editor failed!");
}

//void CWizDocumentWebEngine::onEditorPopulateJavaScriptWindowObject()
//{
//    page()->addToJavaScriptWindowObject("WizExplorerApp", m_app.object());
//    page()->mainFrame()->addToJavaScriptWindowObject("WizEditor", this);
//}

void CWizDocumentWebEngine::onEditorContentChanged()
{
    setContentsChanged(true);
    //
    Q_EMIT statusChanged();
}

void CWizDocumentWebEngine::onEditorSelectionChanged()
{

#ifdef Q_OS_MAC
    // FIXME: every time change content shuld tell webview to clean the canvas
    if (hasFocus()) {
        update();
    }
#endif // Q_OS_MAC

    Q_EMIT statusChanged();
}

void CWizDocumentWebEngine::onEditorLinkClicked(const QUrl& url)
{
    if (isInternalUrl(url))
    {
        viewDocumentByUrl(url);
        return;
    }
    else
    {
        QString strUrl = url.toString();
        if (strUrl.left(12) == "http://file/")
        {
            strUrl.replace(0, 12, "file:/");
            QDesktopServices::openUrl(strUrl);
            return;
        }
    }

    QDesktopServices::openUrl(url);
}

bool CWizDocumentWebEngine::isInternalUrl(const QUrl& url)
{
    return url.scheme().toLower() == "wiz";
}

void CWizDocumentWebEngine::viewDocumentByUrl(const QUrl& url)
{
    if (!url.isValid())
        return;

    QString strUrl = url.toString();
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    mainWindow->viewDocumentByWizKMURL(strUrl);
}

void CWizDocumentWebEngine::splitHtmlToHeadAndBody(const QString& strHtml, QString& strHead, QString& strBody)
{
    QRegExp regh("<head.*>([\\s\\S]*)</head>", Qt::CaseInsensitive);
    if (regh.indexIn(strHtml) != -1) {
        strHead = regh.cap(1).simplified();
    }

    QRegExp regex("<body.*>([\\s\\S]*)</body>", Qt::CaseInsensitive);
    if (regex.indexIn(strHtml) != -1) {
        strBody = regex.cap(1);
    } else {
        strBody = strHtml;
    }
}

void CWizDocumentWebEngine::saveEditingViewDocument(const WIZDOCUMENTDATA &data, bool force)
{
    //FIXME: remove me, just for find a image losses bug.
    Q_ASSERT(!data.strGUID.isEmpty());

    if (!force && !isContentsChanged())
        return;

    // check note permission
    if (!m_dbMgr.db(data.strKbGUID).CanEditDocument(data)) {
        return;
    }
    //
    setContentsChanged(false);
    //

    QString strFileName = m_mapFile.value(data.strGUID);
    QString strHead = getDataByJavaScriptFromPage<QString>("editor.document.head.innerHTML;");
    m_strCurrentNoteHead = strHead;
    QRegExp regHead("<link[^>]*" + m_strDefaultCssFilePath + "[^>]*>", Qt::CaseInsensitive);
    strHead.replace(regHead, "");

    QString strHtml = getDataByJavaScriptFromPage<QString>("editor.getContent();");
    //
    m_strCurrentNoteHtml = strHtml;
    //
    page()->runJavaScript(("updateCurrentNoteHtml();"));

    //
    strHtml = "<html><head>" + strHead + "</head><body>" + strHtml + "</body></html>";

    m_docSaverThread->save(data, strHtml, strFileName, 0);
}

void CWizDocumentWebEngine::saveReadingViewDocument(const WIZDOCUMENTDATA &data, bool force)
{
    Q_UNUSED(data);
    Q_UNUSED(force);

    QString strScript = QString("WizTodoReadChecked.onDocumentClose();");
    page()->runJavaScript(strScript);
}

QString CWizDocumentWebEngine::currentNoteGUID()
{
    return m_strCurrentNoteGUID;
}
QString CWizDocumentWebEngine::currentNoteHtml()
{
    return m_strCurrentNoteHtml;
}
QString CWizDocumentWebEngine::currentNoteHead()
{
    return m_strCurrentNoteHead;
}
bool CWizDocumentWebEngine::currentIsEditing()
{
    return m_bCurrentEditing;
}

void CWizDocumentWebEngine::updateNoteHtml()
{
    WIZDOCUMENTDATA doc = view()->note();
    CWizDatabase& db = m_dbMgr.db(doc.strKbGUID);
    QString strFileName;
    if (db.DocumentToTempHtmlFile(doc, strFileName))
    {
        QString strHtml;
        if (!WizLoadUnicodeTextFromFile(strFileName, strHtml))
            return;

        splitHtmlToHeadAndBody(strHtml, m_strCurrentNoteHead, m_strCurrentNoteHtml);
        m_strCurrentNoteHead += "<link rel=\"stylesheet\" type=\"text/css\" href=\"" + m_strDefaultCssFilePath + "\">";

        m_strCurrentNoteGUID = doc.strGUID;
        page()->runJavaScript(("updateCurrentNoteHtml();"));
    }
}

void CWizDocumentWebEngine::applySearchKeywordHighlight()
{
    MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
    QString strKeyWords = window->searchKeywords();
    if (!strKeyWords.isEmpty() && (!m_bCurrentEditing || !hasFocus()))
    {
        QStringList keyList = strKeyWords.split(getWizSearchSplitChar());
        foreach (QString strText, keyList)
        {
            findText(strText);
        }
    }
    else
    {
        findText("");
    }
}

void CWizDocumentWebEngine::clearSearchKeywordHighlight()
{
    findText("");
}

void CWizDocumentWebEngine::on_insertCodeHtml_requset(QString strOldHtml)
{
    QString strHtml = strOldHtml;
    if (WizGetBodyContentFromHtml(strHtml, false))
    {
        editorCommandExecuteInsertHtml(strHtml, true);
        //FiXME:插入代码时li的属性会丢失，此处需要特殊处理，在head中增加li的属性
        page()->runJavaScript("WizAddCssForCodeLi();");
    }
}

void CWizDocumentWebEngine::viewDocumentInEditor(bool editing)
{
    Q_ASSERT(m_bEditorInited);

    QString strGUID = view()->note().strGUID;
    QString strFileName = m_mapFile.value(strGUID);
    if (strFileName.isEmpty()) {
        return;
    }

    QString strHtml;
    bool ret = WizLoadUnicodeTextFromFile(strFileName, strHtml);
    if (!ret) {
        // hide client and show error
        return;
    }

    //strHtml = escapeJavascriptString(strHtml);

    m_strCurrentNoteHead.clear();
    m_strCurrentNoteHtml.clear();
    splitHtmlToHeadAndBody(strHtml, m_strCurrentNoteHead, m_strCurrentNoteHtml);

    m_strCurrentNoteHead = m_strCurrentNoteHead + "<link rel=\"stylesheet\" type=\"text/css\" href=\"" +
            m_strDefaultCssFilePath + "\">";

    m_strCurrentNoteGUID = strGUID;
    m_bCurrentEditing = editing;
    //
    /*
    QString strExec = QString("viewNote('%1', %2, '%3', '%4');")
            .arg(strGUID)
            .arg(editing ? "true" : "false")
            .arg(strHtml)
            .arg(strHead);
            */
    QString strExec = QString("viewCurrentNote();");

    ret = getDataByJavaScriptFromPage<bool>(strExec);
    if (!ret) {
        qDebug() << "[Editor] failed to load note: " << strExec;
        // hide client and show error
        return;
    }

    // show client
    MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
    if (!ret) {
        window->showClient(false);
        window->transitionView()->showAsMode(strGUID, CWizDocumentTransitionView::ErrorOccured);
        return;
    }

    window->showClient(true);
    window->transitionView()->hide();

    m_timerAutoSave.start();

    //Waiting for the editor initialization complete if it's the first time to load a document.
    QTimer::singleShot(100, this, SLOT(applySearchKeywordHighlight()));
    emit viewDocumentFinished();
}

void CWizDocumentWebEngine::onNoteLoadFinished()
{
    ICore::instance()->emitViewNoteLoaded(view(), view()->note(), true);
}

void CWizDocumentWebEngine::setEditingDocument(bool editing)
{
    //Q_ASSERT(m_bEditorInited);      //
    if(!m_bEditorInited)
        return;             //If editor wasn't initialized,just return.

    // show editor toolbar properly
    if (!editing && hasFocus()) {
        Q_EMIT focusOut();
    }

    if (editing && hasFocus()) {
        Q_EMIT focusIn();
    }

    WIZDOCUMENTDATA docData;
    CWizDatabase& db = m_dbMgr.db(view()->note().strKbGUID);
    if (!db.DocumentFromGUID(view()->note().strGUID, docData))
        return;

    saveDocument(docData, false);

    resetCheckListEnvironment();
    m_bEditingMode = editing;
    //
    QString strScript = QString("setEditing(%1);").arg(editing ? "true" : "false");
    page()->runJavaScript(strScript);
    initCheckListEnvironment();

    if (editing) {
        setFocus(Qt::MouseFocusReason);
        editorFocus();
    }


    Q_EMIT statusChanged();
}

void CWizDocumentWebEngine::saveDocument(const WIZDOCUMENTDATA& data, bool force)
{
    if (!m_bEditorInited)
        return;
    //
    if (!view()->noteLoaded())  //encrypting note & has been loaded
        return;

    if (m_bEditingMode)
    {
        saveEditingViewDocument(data, force);
    }
    else
    {
        saveReadingViewDocument(data, force);
    }
}

QString CWizDocumentWebEngine::editorCommandQueryCommandValue(const QString& strCommand)
{
    QString strExec = "editor.queryCommandValue('" + strCommand +"');";
    return getDataByJavaScriptFromPage<QString>(strExec);
}

int CWizDocumentWebEngine::editorCommandQueryCommandState(const QString& strCommand)
{
    QString strExec = "editor.queryCommandState('" + strCommand +"');";
    return getDataByJavaScriptFromPage<int>(strExec);
}

/*
 * Execute command and also save status to undostack.
 * All commands execute from client which may modify document MUST invoke this
 * instead of use frame's evaluateJavascript.
 */
bool CWizDocumentWebEngine::editorCommandExecuteCommand(const QString& strCommand,
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

    qDebug() << strExec;

    page()->runJavaScript(strExec);

    //
    setContentsChanged(true);
    // notify mainwindow to update action status
    emit statusChanged();

    return true;
}

bool CWizDocumentWebEngine::editorCommandQueryLink()
{
    QString strUrl = getDataByJavaScriptFromPage<QString>("WizGetLinkUrl();");
    if (strUrl.isEmpty())
        return false;

    return true;
}

bool CWizDocumentWebEngine::editorCommandQueryMobileFileReceiverState()
{
    return m_app.userSettings().receiveMobileFile();
}

bool CWizDocumentWebEngine::editorCommandExecuteInsertHtml(const QString& strHtml, bool bNotSerialize)
{
    QString s = bNotSerialize ? "true" : "false";
    return editorCommandExecuteCommand("insertHtml", "'" + strHtml + "'", s);
}

void CWizDocumentWebEngine::setPastePlainTextEnable(bool bEnable)
{
    int nState = editorCommandQueryCommandState("pasteplain");
    if ((!bEnable && nState == 1) || (bEnable && nState != 1))
    {
        editorCommandExecuteCommand("pasteplain");
    }
}

bool CWizDocumentWebEngine::editorCommandExecuteIndent()
{
    return editorCommandExecuteCommand("indent");
}

bool CWizDocumentWebEngine::editorCommandExecuteOutdent()
{
    return editorCommandExecuteCommand("outdent");
}

bool CWizDocumentWebEngine::editorCommandExecuteLinkInsert()
{
    if (!m_editorInsertLinkForm) {
        m_editorInsertLinkForm = new CWizEditorInsertLinkForm(window());
        connect(m_editorInsertLinkForm, SIGNAL(accepted()), SLOT(on_editorCommandExecuteLinkInsert_accepted()));
    }

    QString strUrl = getDataByJavaScriptFromPage<QString>("WizGetLinkUrl();");
    m_editorInsertLinkForm->setUrl(strUrl);

    m_editorInsertLinkForm->exec();

    return true;
}

void CWizDocumentWebEngine::on_editorCommandExecuteLinkInsert_accepted()
{
    // append http if not exist
    QString strUrl = m_editorInsertLinkForm->getUrl();
    if (strUrl.lastIndexOf("http://", 0, Qt::CaseInsensitive) == -1)
        strUrl = "http://" + strUrl;

    editorCommandExecuteCommand("link", QString("{href: '%1'}").arg(strUrl));
}

bool CWizDocumentWebEngine::editorCommandExecuteLinkRemove()
{
    return editorCommandExecuteCommand("unlink");
}

bool CWizDocumentWebEngine::editorCommandExecuteFindReplace()
{
    if (!m_searchReplaceWidget)
    {
        m_searchReplaceWidget = new CWizSearchReplaceWidget();
    }
    connect(m_searchReplaceWidget, SIGNAL(findPre(QString,bool)), SLOT(findPre(QString,bool)));
    connect(m_searchReplaceWidget, SIGNAL(findNext(QString,bool)), SLOT(findNext(QString,bool)));
    connect(m_searchReplaceWidget, SIGNAL(replaceCurrent(QString,QString)), SLOT(replaceCurrent(QString,QString)));
    connect(m_searchReplaceWidget, SIGNAL(replaceAndFindNext(QString,QString,bool)), SLOT(replaceAndFindNext(QString,QString,bool)));
    connect(m_searchReplaceWidget, SIGNAL(replaceAll(QString,QString,bool)), SLOT(replaceAll(QString,QString,bool)));

    QRect rect = geometry();
    rect.moveTo(mapToGlobal(pos()));
    m_searchReplaceWidget->showInEditor(rect);

    return true;
}

static QString strOldSearchText = "";
static bool strOldCase = false;
void CWizDocumentWebEngine::findPre(QString strTxt, bool bCasesensitive)
{
    //FIXME:  there is a problem here, HighlightAllOccurrences can not be used togethor with find one.
    if (strOldSearchText != strTxt || strOldCase != bCasesensitive)
    {
        // clear highlight
        findText("");
        strOldSearchText = strTxt;
        strOldCase = bCasesensitive;
    }

//    findText(strTxt, (bCasesensitive ? QWebEnginePage::FindCaseSensitively | QWebEnginePage::FindBackward
//                                     : QWebEnginePage::FindBackward));
}

void CWizDocumentWebEngine::findNext(QString strTxt, bool bCasesensitive)
{
    if (strOldSearchText != strTxt || strOldCase != bCasesensitive)
    {
        findText("");
        strOldSearchText = strTxt;
        strOldCase = bCasesensitive;
    }
//    findText(strTxt, (bCasesensitive ? QWebEnginePage::FindCaseSensitively : 0));
}

void CWizDocumentWebEngine::replaceCurrent(QString strSource, QString strTarget)
{
    QString strExec = QString("WizReplaceText('%1', '%2', true)").arg(strSource).arg(strTarget);
    page()->runJavaScript(strExec);
}

void CWizDocumentWebEngine::replaceAndFindNext(QString strSource, QString strTarget, bool bCasesensitive)
{
    QString strExec = QString("WizReplaceText('%1', '%2', %3)").arg(strSource).arg(strTarget).arg(bCasesensitive);
    if (!getDataByJavaScriptFromPage<bool>(strExec))
    {
        TOLOG1("[Console] Javascript error : %1", strExec);
        return;
    }
    findNext(strSource, bCasesensitive);
    setContentsChanged(true);
}

void CWizDocumentWebEngine::replaceAll(QString strSource, QString strTarget, bool bCasesensitive)
{
    QString strExec = QString("WizRepalceAll('%1', '%2', %3)").arg(strSource).arg(strTarget).arg(bCasesensitive);
    page()->runJavaScript(strExec);
    setContentsChanged(true);
}

bool CWizDocumentWebEngine::editorCommandExecuteFontFamily(const QString& strFamily)
{
    return editorCommandExecuteCommand("fontFamily", "'" + strFamily + "'");
}

bool CWizDocumentWebEngine::editorCommandExecuteFontSize(const QString& strSize)
{
    return editorCommandExecuteCommand("fontSize", "'" + strSize + "'");
}

void CWizDocumentWebEngine::editorCommandExecuteBackColor()
{
    if (!m_colorDialog) {
        m_colorDialog = new QColorDialog(this);
    }

    m_colorDialog->disconnect();

    connect(m_colorDialog, SIGNAL(currentColorChanged(const QColor &)),
            SLOT(on_editorCommandExecuteBackColor_accepted(const QColor&)));
    connect(m_colorDialog, SIGNAL(colorSelected(const QColor &)),
            SLOT(on_editorCommandExecuteBackColor_accepted(const QColor&)));

    m_colorDialog->exec();
}

void CWizDocumentWebEngine::on_editorCommandExecuteBackColor_accepted(const QColor& color)
{
    editorCommandExecuteCommand("backColor", "'" + color.name() + "'");
}

void CWizDocumentWebEngine::editorCommandExecuteForeColor()
{
    if (!m_colorDialog) {
        m_colorDialog = new QColorDialog(this);
    }

    m_colorDialog->disconnect();
    connect(m_colorDialog, SIGNAL(currentColorChanged(const QColor &)),
            SLOT(on_editorCommandExecuteForeColor_accepted(const QColor&)));
    connect(m_colorDialog, SIGNAL(colorSelected(const QColor &)),
            SLOT(on_editorCommandExecuteForeColor_accepted(const QColor&)));

    m_colorDialog->exec();
}

void CWizDocumentWebEngine::on_editorCommandExecuteForeColor_accepted(const QColor& color)
{
    editorCommandExecuteCommand("foreColor", "'" + color.name() + "'");
}

bool CWizDocumentWebEngine::editorCommandExecuteBold()
{
    return editorCommandExecuteCommand("bold");
}

bool CWizDocumentWebEngine::editorCommandExecuteItalic()
{
    return editorCommandExecuteCommand("italic");
}

bool CWizDocumentWebEngine::editorCommandExecuteUnderLine()
{
    return editorCommandExecuteCommand("underline");
}

bool CWizDocumentWebEngine::editorCommandExecuteStrikeThrough()
{
    return editorCommandExecuteCommand("strikethrough");
}

bool CWizDocumentWebEngine::editorCommandExecuteJustifyLeft()
{
    return editorCommandExecuteCommand("justify", "'left'");
}

bool CWizDocumentWebEngine::editorCommandExecuteJustifyRight()
{
    return editorCommandExecuteCommand("justify", "'right'");
}

bool CWizDocumentWebEngine::editorCommandExecuteJustifyCenter()
{
    return editorCommandExecuteCommand("justify", "'center'");
}

bool CWizDocumentWebEngine::editorCommandExecuteJustifyJustify()
{
    return editorCommandExecuteCommand("justify", "'justify'");
}

bool CWizDocumentWebEngine::editorCommandExecuteInsertOrderedList()
{
    return editorCommandExecuteCommand("insertOrderedList");
}

bool CWizDocumentWebEngine::editorCommandExecuteInsertUnorderedList()
{
    return editorCommandExecuteCommand("insertUnorderedList");
}

bool CWizDocumentWebEngine::editorCommandExecuteTableInsert()
{
    if (!m_editorInsertTableForm) {
        m_editorInsertTableForm = new CWizEditorInsertTableForm(window());
        connect(m_editorInsertTableForm, SIGNAL(accepted()), SLOT(on_editorCommandExecuteTableInsert_accepted()));
    }

    m_editorInsertTableForm->clear();
    m_editorInsertTableForm->exec();

    return true;
}

void CWizDocumentWebEngine::on_editorCommandExecuteTableInsert_accepted()
{
    int nRows = m_editorInsertTableForm->getRows();
    int nCols = m_editorInsertTableForm->getCols();

    if (!nRows && !nCols)
        return;

    editorCommandExecuteCommand("insertTable", QString("{numRows:%1, numCols:%2, border:1, borderStyle:'1px solid #dddddd;'}").arg(nRows).arg(nCols));
}

void CWizDocumentWebEngine::on_editorCommandExecuteScreenShot_imageAccepted(QPixmap pix)
{
    QObject *ssSender = qobject_cast<QObject*>(sender());
    if (ssSender)
        delete ssSender;

    setWindowVisibleOnScreenShot(true);

    if (pix.isNull())
        return;

    QString strTempPath = Utils::PathResolve::tempPath();
    CString strFileName = strTempPath + WizIntToStr(GetTickCount()) + ".png";
    if (!pix.save(strFileName)) {
        TOLOG("ERROR: Can't save clipboard image to file");
        return;
    }

    insertImage(strFileName, false);
}

void CWizDocumentWebEngine::on_editorCommandExecuteScreenShot_finished()
{
    QObject *ssSender = qobject_cast<QObject*>(sender());
    if (ssSender)
        delete ssSender;

    setWindowVisibleOnScreenShot(true);
}

bool CWizDocumentWebEngine::editorCommandExecuteInsertHorizontal()
{
    return editorCommandExecuteCommand("horizontal");
}

bool CWizDocumentWebEngine::editorCommandExecuteInsertCheckList()
{
    // before insert first checklist, should manual notify editor to save current sence for undo.
    page()->runJavaScript("editor.execCommand('saveScene');");

    QString strExec = "WizTodo.insertOneTodo();";
    bool ret = getDataByJavaScriptFromPage<bool>(strExec);

    // after insert first checklist, should manual notify editor to save current sence for undo.
    page()->runJavaScript("editor.execCommand('saveScene');");

    emit statusChanged();

    return ret;
}

bool CWizDocumentWebEngine::editorCommandExecuteInsertImage()
{
    QStringList strImgFileList = QFileDialog::getOpenFileNames(0, tr("Image File"), QDir::homePath(), tr("Images (*.png *.bmp *.gif *.jpg)"));
    if (strImgFileList.isEmpty())
        return false;

    foreach (QString strImgFile, strImgFileList)
    {
        bool bUseCopyFile = true;
        insertImage(strImgFile, bUseCopyFile);
    }
    return true;
}

bool CWizDocumentWebEngine::editorCommandExecuteInsertDate()
{
    return editorCommandExecuteCommand("date");
}

bool CWizDocumentWebEngine::editorCommandExecuteInsertTime()
{
    return editorCommandExecuteCommand("time");
}

bool CWizDocumentWebEngine::editorCommandExecuteRemoveFormat()
{
    return editorCommandExecuteCommand("removeFormat");
}

bool CWizDocumentWebEngine::editorCommandExecutePlainText()
{
    QString strText = getDataByJavaScriptFromPage<QString>("editor.getPlainTxt()");
    QRegExp exp("<[^>]*>");
    strText.replace(exp, "");
#if QT_VERSION > 0x050000
    strText = "<div>" + strText.toHtmlEscaped() + "</div>";
#else
    strText = "<div>" + strText + "</div>";
#endif
    strText.replace(" ", "&nbsp;");
    strText.replace("\n", "<br />");

    setContentsChanged(true);
    m_strCurrentNoteHtml = strText;
    QString strExec = QString("viewCurrentNote();");
    return getDataByJavaScriptFromPage<bool>(strExec);
}

bool CWizDocumentWebEngine::editorCommandExecuteFormatMatch()
{
    return editorCommandExecuteCommand("formatMatch");
}

bool CWizDocumentWebEngine::editorCommandExecuteViewSource()
{
    return editorCommandExecuteCommand("source");
}

bool CWizDocumentWebEngine::editorCommandExecuteInsertCode()
{
    QString strSelectHtml = page()->selectedText();
    WizCodeEditorDialog *dialog = new WizCodeEditorDialog(m_app);
    connect(dialog, SIGNAL(insertHtmlRequest(QString)), SLOT(on_insertCodeHtml_requset(QString)));
    dialog->show();
    dialog->setWindowState(dialog->windowState() & ~Qt::WindowFullScreen | Qt::WindowActive);
    dialog->setCode(strSelectHtml);

    return true;
}

bool CWizDocumentWebEngine::editorCommandExecuteMobileImage(bool bReceiveImage)
{
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    if (bReceiveImage && m_app.userSettings().needShowMobileFileReceiverUserGuide())
    {
        mainWindow->showMobileFileReceiverUserGuide();
    }

    m_app.userSettings().setReceiveMobileFile(bReceiveImage);
    mainWindow->setMobileFileReceiverEnable(bReceiveImage);

    return true;
}

bool CWizDocumentWebEngine::editorCommandExecuteScreenShot()
{
    CWizScreenShotHelper* helper = new CWizScreenShotHelper();

    connect(helper, SIGNAL(screenShotCaptured(QPixmap)),
            SLOT(on_editorCommandExecuteScreenShot_imageAccepted(QPixmap)));
    connect(helper, SIGNAL(shotScreenQuit()), SLOT(on_editorCommandExecuteScreenShot_finished()));

    setWindowVisibleOnScreenShot(false);
    QTimer::singleShot(200, helper, SLOT(startScreenShot()));
    return true;
}

#ifdef Q_OS_MAC
bool CWizDocumentWebEngine::editorCommandExecuteRemoveStartOfLine()
{
//    triggerPageAction(QWebEnginePage::SelectStartOfLine);
    QKeyEvent delKeyPress(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier, QString());
    return QApplication::sendEvent(this, &delKeyPress);
}
#endif

bool CWizDocumentWebEngine::editorCommandExecuteTableDelete()
{
    return editorCommandExecuteCommand("deletetable");
}

bool CWizDocumentWebEngine::editorCommandExecuteTableDeleteRow()
{
    return editorCommandExecuteCommand("deleterow");
}

bool CWizDocumentWebEngine::editorCommandExecuteTableDeleteCol()
{
    return editorCommandExecuteCommand("deletecol");
}

bool CWizDocumentWebEngine::editorCommandExecuteTableInsertRow()
{
    return editorCommandExecuteCommand("insertrow");
}

bool CWizDocumentWebEngine::editorCommandExecuteTableInsertRowNext()
{
    return editorCommandExecuteCommand("insertrownext");
}

bool CWizDocumentWebEngine::editorCommandExecuteTableInsertCol()
{
    return editorCommandExecuteCommand("insertcol");
}

bool CWizDocumentWebEngine::editorCommandExecuteTableInsertColNext()
{
    return editorCommandExecuteCommand("insertcolnext");
}

bool CWizDocumentWebEngine::editorCommandExecuteTableInsertCaption()
{
    return editorCommandExecuteCommand("insertcaption");
}

bool CWizDocumentWebEngine::editorCommandExecuteTableDeleteCaption()
{
    return editorCommandExecuteCommand("deletecaption");
}

bool CWizDocumentWebEngine::editorCommandExecuteTableInsertTitle()
{
    return editorCommandExecuteCommand("inserttitle");
}

bool CWizDocumentWebEngine::editorCommandExecuteTableDeleteTitle()
{
    return editorCommandExecuteCommand("deletetitle");
}

bool CWizDocumentWebEngine::editorCommandExecuteTableMergeCells()
{
    return editorCommandExecuteCommand("mergecells");
}

bool CWizDocumentWebEngine::editorCommandExecuteTalbeMergeRight()
{
    return editorCommandExecuteCommand("mergeright");
}

bool CWizDocumentWebEngine::editorCommandExecuteTableMergeDown()
{
    return editorCommandExecuteCommand("mergedown");
}

bool CWizDocumentWebEngine::editorCommandExecuteTableSplitCells()
{
    return editorCommandExecuteCommand("splittocells");
}

bool CWizDocumentWebEngine::editorCommandExecuteTableSplitRows()
{
    return editorCommandExecuteCommand("splittorows");
}

bool CWizDocumentWebEngine::editorCommandExecuteTableSplitCols()
{
    return editorCommandExecuteCommand("splittocols");
}

bool CWizDocumentWebEngine::editorCommandExecuteTableAverageRows()
{
    return editorCommandExecuteCommand("averagedistributerow");
}

bool CWizDocumentWebEngine::editorCommandExecuteTableAverageCols()
{
    return editorCommandExecuteCommand("averagedistributecol");
}

void CWizDocumentWebEngine::saveAsPDF(const QString& strFileName)
{
//    if (QWebFrame* frame = noteFrame())
//    {
//        if (::PathFileExists(strFileName))
//        {
//            ::DeleteFile(strFileName);
//        }
//        //
//        QPrinter printer;
//        QPrinter::Unit marginUnit =  (QPrinter::Unit)m_app.userSettings().printMarginUnit();
//        double marginTop = m_app.userSettings().printMarginValue(wizPositionTop);
//        double marginBottom = m_app.userSettings().printMarginValue(wizPositionBottom);
//        double marginLeft = m_app.userSettings().printMarginValue(wizPositionLeft);
//        double marginRight = m_app.userSettings().printMarginValue(wizPositionRight);
//        printer.setPageMargins(marginLeft, marginTop, marginRight, marginBottom, marginUnit);
//        printer.setOutputFormat(QPrinter::PdfFormat);
//        printer.setColorMode(QPrinter::Color);
//        printer.setOutputFileName(strFileName);
//        //
//        frame->print(&printer);
//    }
}

void CWizDocumentWebEngine::saveAsHtml(const QString& strDirPath)
{
    const WIZDOCUMENTDATA& doc = view()->note();
    CWizDatabase& db = m_dbMgr.db(doc.strKbGUID);
    db.ExportToHtmlFile(doc, strDirPath);
}

void CWizDocumentWebEngine::printDocument()
{
//    if (QWebFrame* frame = noteFrame())
//    {
//        QPrinter printer(QPrinter::HighResolution);
//        QPrinter::Unit marginUnit =  (QPrinter::Unit)m_app.userSettings().printMarginUnit();
//        double marginTop = m_app.userSettings().printMarginValue(wizPositionTop);
//        double marginBottom = m_app.userSettings().printMarginValue(wizPositionBottom);
//        double marginLeft = m_app.userSettings().printMarginValue(wizPositionLeft);
//        double marginRight = m_app.userSettings().printMarginValue(wizPositionRight);
//        printer.setPageMargins(marginLeft, marginTop, marginRight, marginBottom, marginUnit);
//        printer.setOutputFormat(QPrinter::NativeFormat);

//#ifdef Q_OS_MAC
//        QPrinterInfo info(printer);
//        if (info.printerName().isEmpty())
//        {
//            QMessageBox::information(0, tr("Inof"), tr("No available printer founded! Please add"
//                                                       " printer to system printer list."));
//            return;
//        }
//#endif

//        QPrintDialog dlg(&printer,0);
//        dlg.setWindowTitle(QObject::tr("Print Document"));
//        if(dlg.exec() == QDialog::Accepted)
//        {
//            frame->print(&printer);
//        }
//    }
}

bool CWizDocumentWebEngine::findIMGElementAt(QPoint point, QString& strSrc)
{
    QPoint ptPos = mapFromGlobal(point);
    QString strImgSrc = getDataByJavaScriptFromPage<QString>(QString("WizGetImgElementByPoint(%1, %2)").
                                                                arg(ptPos.x()).arg(ptPos.y()));

    if (strImgSrc.isEmpty())
        return false;

    strSrc = strImgSrc;
    return true;
}

void CWizDocumentWebEngine::undo()
{
    page()->runJavaScript("editor.execCommand('undo')");
    emit statusChanged();
}

void CWizDocumentWebEngine::redo()
{
    page()->runJavaScript("editor.execCommand('redo')");
    emit statusChanged();
}


WebEnginePage::WebEnginePage(QObject* parent)
    : QWebEnginePage(parent)
{

}

WebEnginePage::~WebEnginePage()
{

}

void WebEnginePage::javaScriptConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID)
{
    qDebug() << "console at line : " << lineNumber << "  message :  " << message << " source id : " << sourceID.left(15);
}

void WebEnginePage::triggerAction(QWebEnginePage::WebAction action, bool checked)
{
    if (action == QWebEnginePage::Reload)
    {
        QString strFileName = "/Users/lxn/editor/index.html";
        QFile file(strFileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QUrl url = QUrl::fromLocalFile(strFileName);
            QTextStream stream(&file);
            QString strText = stream.readAll();
            file.close();


            setHtml(strText, url);
        }
        return;
    }
    else if (action == QWebEnginePage::Paste)
    {
        qDebug() << "paste action triggered";
    }
    else if (action == QWebEnginePage::PasteAndMatchStyle)
    {
        qDebug() << "paste and match called";
    }
    QWebEnginePage::triggerAction(action, checked);
}
