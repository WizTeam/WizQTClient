<<<<<<< HEAD:src/wizDocumentWebEngine.cpp
#if 0
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
//#include <QtWebSockets/QWebSocketServer>
#include <QSemaphore>

#include <QApplication>
#include <QUndoStack>
#include <QDesktopServices>

#ifdef Q_OS_MAC
#include <QMacPasteboardMime>
#endif

#include <coreplugin/icore.h>

#include "wizdef.h"
#include "wizDocumentView.h"
#include "wizmainwindow.h"
#include "share/wizmisc.h"
#include "wizEditorInsertLinkForm.h"
#include "wizEditorInsertTableForm.h"
#include "share/wizObjectDataDownloader.h"
#include "share/websocketclientwrapper.h"
#include "share/websockettransport.h"
#include "share/wizDatabaseManager.h"
#include "wizDocumentTransitionView.h"
#include "wizSearchReplaceWidget.h"
#include "widgets/WizCodeEditorDialog.h"
#include "widgets/wizScreenShotWidget.h"
#include "widgets/wizEmailShareDialog.h"
#include "sync/avatar.h"

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
    , m_bInSeperateWindow(false)
{
    m_parentView = dynamic_cast<CWizDocumentView*>(parent);
    Q_ASSERT(m_parentView);

    WebEnginePage *webPage = new WebEnginePage(this);
    setPage(webPage);

//    connect(webPage, SIGNAL(urlChanged(QUrl)), SLOT(onEditorLinkClicked(QUrl)));

    // FIXME: should accept drop picture, attachment, link etc.
    setAcceptDrops(true);

    // refers
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());

    m_transitionView = mainWindow->transitionView();

    m_docLoadThread = new CWizDocumentWebViewLoaderThread(m_dbMgr, this);
    connect(m_docLoadThread, SIGNAL(loaded(const QString&, const QString, const QString)),
            SLOT(onDocumentReady(const QString&, const QString, const QString)), Qt::QueuedConnection);
    //
    m_docSaverThread = new CWizDocumentWebViewSaverThread(m_dbMgr, this);
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


void CWizDocumentWebEngine::keyPressEvent(QKeyEvent* event)
{
    qDebug() << "key pressed : " << event;
//    sendEventToChildWidgets(event);

    if (event->key() == Qt::Key_Escape)
    {
        // FIXME: press esc will insert space at cursor if not clear focus
        clearFocus();
        return;
    }
    else if (event->key() == Qt::Key_S
             && event->modifiers() & Qt::ControlModifier)
    {
        saveDocument(view()->note(), false);
        return;
    }
    else if (event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))
    {
        qDebug() << "event modifier matched : " << (Qt::ControlModifier | Qt::ShiftModifier);
        if (event->key() == Qt::Key_Left)
        {
            QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Home , Qt::ShiftModifier, QString());
            sendEventToChildWidgets(&keyPress);
        }
        else if (event->key() == Qt::Key_Right)
        {
            QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_End , Qt::ShiftModifier, QString());
            sendEventToChildWidgets(&keyPress);
        }
    }

    QWebEngineView::keyPressEvent(event);

    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        tryResetTitle();
        if (event->key() == Qt::Key_Enter)
        {
            QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Return , Qt::ShiftModifier, QString());
            sendEventToChildWidgets(&keyPress);
        }
    }
}


void CWizDocumentWebEngine::focusInEvent(QFocusEvent *event)
{
    focusInEditor();

    QWebEngineView::focusInEvent(event);

    applySearchKeywordHighlight();
}

void CWizDocumentWebEngine::focusOutEvent(QFocusEvent *event)
{
    // because qt will clear focus when context menu popup, we need keep focus there.
    if (event->reason() == Qt::PopupFocusReason)
    {
        event->accept();
        return;
    }

    if (m_bEditingMode && view()->hasFocus())
    {
        event->accept();
        return;
    }

    focusOutEditor();
    QWebEngineView::focusOutEvent(event);
}

bool CWizDocumentWebEngine::event(QEvent* event)
{
//    qDebug() << "webengine event called ,  event type : " << event->type();
    return QWebEngineView::event(event);
}

bool CWizDocumentWebEngine::eventFilter(QObject* watched, QEvent* event)
{
    switch (event->type()) {
    case QEvent::FocusIn:
    {
        QFocusEvent* focusEvent = dynamic_cast<QFocusEvent*>(event);
        if (focusEvent)
        {
            focusInEvent(focusEvent);
        }
        break;
    }
    case QEvent::FocusOut:
    {
        QFocusEvent* focusEvent = dynamic_cast<QFocusEvent*>(event);
        if (focusEvent)
        {
            focusOutEvent(focusEvent);
        }
        break;
    }
    case QEvent::KeyPress:
    {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent)
        {
            keyPressEvent(keyEvent);
        }
        break;
    }
    default:
        break;
    }
    return QWebEngineView::eventFilter(watched, event);
}

void CWizDocumentWebEngine::childEvent(QChildEvent* event)
{
    if (event->type() == QEvent::ChildAdded)
    {
//        event->child()->installEventFilter(this);
        m_childWidgets.append(event->child());
        event->child()->installEventFilter(this);
    }
    else if (event->type() == QEvent::ChildRemoved)
    {
        m_childWidgets.removeOne(event->child());
    }

    QWebEngineView::childEvent(event);
}

void CWizDocumentWebEngine::contextMenuEvent(QContextMenuEvent *event)
{
    if (!m_bEditorInited)
        return;

    qDebug() << "contextMenu event called : " << event  << "\n event pos : " << event->pos();
    Q_EMIT showContextMenuRequest(mapToGlobal(event->pos()));
}

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

    page()->runJavaScript("editor.getPlainTxt();", [this](const QVariant& returnValue){
        QString strTitle = returnValue.toString();
        strTitle = WizStr2Title(strTitle.left(255));
        if (strTitle.isEmpty())
            return;

        view()->resetTitle(strTitle);
        m_bNewNoteTitleInited = true;
    });
}

void CWizDocumentWebEngine::dropEvent(QDropEvent* event)
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
            QImageReader reader(strFileName);
            if (reader.canRead())
            {
                QString strHtml;
                bool bUseCopyFile = true;
                if (WizImage2Html(strFileName, strHtml, bUseCopyFile)) {
                    editorCommandExecuteInsertHtml(strHtml, true);
                    nAccepted++;
                }
            }
            else
            {
                WIZDOCUMENTDATA doc = view()->note();
                CWizDatabase& db = m_dbMgr.db(doc.strKbGUID);
                WIZDOCUMENTATTACHMENTDATA data;
                data.strKbGUID = doc.strKbGUID; // needed by under layer
                if (!db.AddAttachment(doc, strFileName, data))
                {
                    TOLOG1("[drop] add attachment failed %1", strFileName);
                    continue;
                }
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
                    CWizDatabase& db = m_dbMgr.db(docIds.first());
                    if (db.DocumentFromGUID(docIds.last(), document))
                    {
                        QString strHtml, strLink;
                        db.DocumentToHtmlLink(document, strHtml, strLink);
                        strLinkHtml += "<p>" + strHtml + "</p>";
                    }
                }
            }

            editorCommandExecuteInsertHtml(strLinkHtml, false);

            nAccepted ++;
        }
    }

    if (nAccepted > 0) {
        event->accept();
        saveDocument(view()->note(), false);
    }
}

CWizDocumentView* CWizDocumentWebEngine::view() const
{
    return m_parentView;
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
        loadEditor();
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
    qDebug() << "view document called, init editor , guid " << doc.strGUID << "  editing mode : " << editing;

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

    qDebug() << "try to load document , guid : " << doc.strGUID;
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

void CWizDocumentWebEngine::closeDocument(const WIZDOCUMENTDATA& data)
{
    bool isSourceMode = editorCommandQueryCommandState("source");
    if (isSourceMode)
    {
        synchronousRunJavaScript("editor.execCommand('source')");
    }
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

    emit resetDefaultCss(m_strDefaultCssFilePath);

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
        page()->runJavaScript("editor.setDisabled();");
        clearFocus();
    }
}

void CWizDocumentWebEngine::on_ueditorInitFinished()
{
    qDebug() << "on init ueditor finished";
    if (!resetDefaultCss())
        return;

    m_bEditorInited = true;
    viewDocumentInEditor(m_bEditingMode);
    return;
}

void CWizDocumentWebEngine::on_viewDocumentFinished(bool ok)
{
    // show client
    MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
    if (!ok) {
        window->showClient(false);
        window->transitionView()->showAsMode(m_strCurrentNoteGUID, CWizDocumentTransitionView::ErrorOccured);
        return;
    }

    window->showClient(true);
    window->transitionView()->hide();

    m_timerAutoSave.start();
    initCheckListEnvironment();

    //Waiting for the editor initialization complete if it's the first time to load a document.
    QTimer::singleShot(100, this, SLOT(applySearchKeywordHighlight()));
    emit viewDocumentFinished();
}

void CWizDocumentWebEngine::runJavaScript(const QString& js, const QWebEngineCallback<const QVariant &> &resultCallback)
{
    page()->runJavaScript(js, resultCallback);
}

void CWizDocumentWebEngine::editorCommandQueryCommandState(const QString& strCommand,
                                                           const QWebEngineCallback<const QVariant&>& resultCallback)
{
    QString strExec = "editor.queryCommandState('" + strCommand +"');";
    page()->runJavaScript(strExec, resultCallback);
}

void CWizDocumentWebEngine::loadEditor()
{
    qDebug() << "initEditor called";
    if (m_bEditorInited)
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
        qDebug() << "init checklist in edting mode";
        QString strScript = QString("WizTodo.init('qt');");
        page()->runJavaScript(strScript, [this](const QVariant&) {
            //FIXME: 目前javascript无法直接取得c++函数的返回值，需要主动写入
            page()->runJavaScript(QString("WizTodo.setUserAlias('%1');").arg(getUserAlias()));
            page()->runJavaScript(QString("WizTodo.setUserAvatarFileName('%1');").arg(getUserAvatarFile(24)));
            page()->runJavaScript(QString("WizTodo.setCheckedImageFileName('%1');").arg(getSkinResourcePath() + "checked.png"));
            page()->runJavaScript(QString("WizTodo.setUnCheckedImageFileName('%1');").arg(getSkinResourcePath() + "unchecked.png"));
            page()->runJavaScript(QString("WizTodo.setIsPersonalDocument(%1);").arg(isPersonalDocument()));
        });
    }
    else
    {
        QString strScript = QString("WizTodoReadChecked.init('qt');");
        page()->runJavaScript(strScript, [this](const QVariant&) {
            //FIXME: 目前javascript无法直接取得c++函数的返回值，需要主动写入
            page()->runJavaScript(QString("WizTodoReadChecked.setUserAlias('%1');").arg(getUserAlias()));
            page()->runJavaScript(QString("WizTodoReadChecked.setUserAvatarFileName('%1');").arg(getUserAvatarFile(24)));
            page()->runJavaScript(QString("WizTodoReadChecked.setCheckedImageFileName('%1');").arg(getSkinResourcePath() + "checked.png"));
            page()->runJavaScript(QString("WizTodoReadChecked.setUnCheckedImageFileName('%1');").arg(getSkinResourcePath() + "unchecked.png"));
            page()->runJavaScript(QString("WizTodoReadChecked.setIsPersonalDocument(%1);").arg(isPersonalDocument()));
            page()->runJavaScript(QString("WizTodoReadChecked.setCanEdit(%1);").arg(hasEditPermissionOnCurrentNote()));
            QString strHtml = getCurrentNoteHtml();
            QString base64;
            WizBase64Encode(strHtml.toUtf8(), base64);
            page()->runJavaScript(QString("WizTodoReadChecked.setDocOriginalHtml('%1');").arg(base64));
//            emit setDocOriginalHtml(getCurrentNoteHtml());
        });
    }
}

bool CWizDocumentWebEngine::speakHelloWorld()
{
    qDebug() << "Hello world from web engine";
    return true;
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

QVariant CWizDocumentWebEngine::synchronousRunJavaScript(const QString& strExec)
{
    QVariant returnValue;
    QEventLoop loop;
    qDebug() << "create loop and loop locker before run js : " << strExec;
    page()->runJavaScript(strExec , [&](const QVariant& result) {
        returnValue = result;
        loop.quit();
        qDebug() << "javascript result : " << result;
    });
    qDebug() << "before loop exec";
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    return returnValue;
}

QString CWizDocumentWebEngine::getSkinResourcePath() const
{
    return ::WizGetSkinResourcePath(m_app.userSettings().skin());

}

QString CWizDocumentWebEngine::getUserAvatarFile(int size) const
{
    QString strFileName;
    QString strUserID = m_dbMgr.db().GetUserId();
    if (WizService::AvatarHost::customSizeAvatar(strUserID, size, size, strFileName))
        return strFileName;

    return QString();
}

QString CWizDocumentWebEngine::getUserAlias() const
{
    QString strKbGUID = view()->note().strKbGUID;
    return m_dbMgr.db(strKbGUID).GetUserAlias();

}

bool CWizDocumentWebEngine::isPersonalDocument() const
{
    QString strKbGUID = view()->note().strKbGUID;
    QString dbKbGUID = m_dbMgr.db().kbGUID();
    return strKbGUID.isEmpty() || (strKbGUID == dbKbGUID);
}

QString CWizDocumentWebEngine::getCurrentNoteHtml() const
{
    CWizDatabase& db = m_dbMgr.db(view()->note().strKbGUID);
    QString strFolder = Utils::PathResolve::tempDocumentFolder(view()->note().strGUID);
    if (db.ExtractZiwFileToFolder(view()->note(), strFolder))
    {
        QString strHtmlFile = strFolder + "index.html";
        QString strHtml;
        ::WizLoadUnicodeTextFromFile(strHtmlFile, strHtml);
        return strHtml;
    }

    return QString();
}

bool CWizDocumentWebEngine::hasEditPermissionOnCurrentNote() const
{
    WIZDOCUMENTDATA docData = view()->note();
    CWizDatabase& db = m_dbMgr.db(docData.strKbGUID);
    return db.CanEditDocument(docData) && !CWizDatabase::IsInDeletedItems(docData.strLocation);
}

bool CWizDocumentWebEngine::shareNoteByEmail()
{
    CWizEmailShareDialog dlg(m_app);
    dlg.setNote(view()->note());

    dlg.exec();

    return true;
}

void copyFileToFolder(const QString& strFileFoler, const QString& strIndexFile, \
                         const QStringList& strResourceList)
{
    //copy index file
    QString strFolderIndex = strFileFoler + "index.html";
    if (strIndexFile != strFolderIndex)
    {
        QFile::remove(strFolderIndex);
        QFile::copy(strIndexFile, strFolderIndex);
    }

    //copy resources to temp folder
    QString strResourcePath = strFileFoler + "index_files/";
    for (int i = 0; i < strResourceList.count(); i++)
    {
        if (QFile::exists(strResourceList.at(i)))
        {
            QFile::copy(strResourceList.at(i), strResourcePath + WizExtractFileName(strResourceList.at(i)));
        }
    }
}

void CWizDocumentWebEngine::saveHtmlToCurrentNote(const QString& strHtml, const QString& strResource)
{
    qDebug() << "save html to current note called : " << strHtml << "   resources  :  " << strResource;
    if (strHtml.isEmpty())
        return;

    WIZDOCUMENTDATA docData = view()->note();
    CWizDatabase& db = m_dbMgr.db(docData.strKbGUID);
    QString strFolder = Utils::PathResolve::tempDocumentFolder(docData.strGUID);
    //
    QString strHtmlFile = strFolder + "index.html";
    ::WizSaveUnicodeTextToUtf8File(strHtmlFile, strHtml);
    QStringList strResourceList = strResource.split('*');
    copyFileToFolder(strFolder, strHtmlFile, strResourceList);

    db.CompressFolderToZiwFile(docData, strFolder);
    bool bNotify = false;
    QString strZiwFile = db.GetDocumentFileName(docData.strGUID);
    db.UpdateDocumentDataMD5(docData, strZiwFile, bNotify);
    MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());
    mainWindow->quickSyncKb(docData.strKbGUID);

    updateNoteHtml();
}

void CWizDocumentWebEngine::setCurrentDocumentType(const QString& strType)
{
    WIZDOCUMENTDATA docData = view()->note();
    CWizDatabase& db = m_dbMgr.db(docData.strKbGUID);
    docData.strType = strType;
    db.ModifyDocumentInfoEx(docData);
}

bool CWizDocumentWebEngine::checkListClickable()
{
    if (!m_dbMgr.db(view()->note().strKbGUID).IsGroup())
    {
        emit clickingTodoCallBack(false, false);
        return true;
    }

    if (view()->checkListClickable())
    {
        qDebug() << "check list clickable";
        view()->setStatusToEditingByCheckList();
        emit clickingTodoCallBack(false, false);
        return true;
    }
    emit clickingTodoCallBack(true, true);
    return false;
}

void CWizDocumentWebEngine::insertCssForCode()
{
    QString strCss = "file://" + Utils::PathResolve::resourcesPath() + "files/wiz_code_highlight.css";
    page()->runJavaScript(QString("WizAddCssForCode('%1');").arg(strCss));
//    synchronousRunJavaScript(QString("WizAddCssForCode(%1);").arg(strCss));
}

void CWizDocumentWebEngine::onEditorLoadFinished(bool ok)
{
    qDebug() << "on editor load finished , ok : " << ok;
    if (ok)
    {
        registerJavaScriptWindowObject();
        return;
    }

    TOLOG("Wow, loading editor failed!");
}

void CWizDocumentWebEngine::onEditorContentChanged()
{
    setContentsChanged(true);
    //
    Q_EMIT statusChanged();
}

void CWizDocumentWebEngine::onEditorSelectionChanged()
{
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
    qDebug() << "save editing veiw document , guid : " <<  data.strGUID <<  " \n from thread : " << thread();
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
    QEventLoop loop;
    qDebug() << "create mutex and start to run java script";
    page()->runJavaScript("editor.document.head.innerHTML;", [&](const QVariant& headData) {
        m_strCurrentNoteHead = headData.toString();
        //
        page()->runJavaScript("editor.getContent();", [&](const QVariant& bodyData) {
            m_strCurrentNoteHtml = bodyData.toString();
//            qDebug() <<"get html data from editor , head : " << m_strCurrentNoteHead << " \n body : " << m_strCurrentNoteHtml;
            //
            QString strExec = QString("updateCurrentNoteHtml('%1', '%2', '%3');")
                    .arg(m_strCurrentNoteGUID).arg(m_strCurrentNoteHtml).arg(m_strCurrentNoteHead);
            page()->runJavaScript(strExec, [&](const QVariant&) {
                QString strHead = m_strCurrentNoteHead;
                QRegExp regHead("<link[^>]*" + m_strDefaultCssFilePath + "[^>]*>", Qt::CaseInsensitive);
                strHead.replace(regHead, "");
                //
                QString strHtml = m_strCurrentNoteHtml;
                strHtml = "<html><head>" + strHead + "</head><body>" + strHtml + "</body></html>";
                //
                QString strFileName = m_mapFile.value(data.strKbGUID);
                m_docSaverThread->save(data, strHtml, strFileName, 0);
                qDebug() << "get document data finished,  unlock the meutex";
                loop.quit();
            });
            // update current html to js variant
        });
    });
    //  save document

    loop.exec(QEventLoop::ExcludeUserInputEvents);
    qDebug() << "quit save editing view document ";
}

void CWizDocumentWebEngine::saveReadingViewDocument(const WIZDOCUMENTDATA &data, bool force)
{
    Q_UNUSED(force);

    QEventLoop loop;
    QString strScript = QString("WizTodoReadChecked.onDocumentClose();");
    page()->runJavaScript(strScript, [&](const QVariant& returnValue) {
        QString strHtml = returnValue.toString();
        if (!strHtml.isEmpty())
        {
            qDebug() << "save reading document ,get html : " << strHtml;
            QString strFileName = m_mapFile.value(data.strKbGUID);
            m_docSaverThread->save(data, strHtml, strFileName, 0);
        }
        loop.quit();
    });
    loop.exec(QEventLoop::ExcludeUserInputEvents);
}

QString CWizDocumentWebEngine::currentNoteGUID()
{
    qDebug() << "currentNoteGUID called ; " << m_strCurrentNoteGUID;
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
        QString strExec = QString("updateCurrentNoteHtml(%1, %2, %3);")
                .arg(m_strCurrentNoteGUID).arg(m_strCurrentNoteHtml).arg(m_strCurrentNoteHead);
        page()->runJavaScript(strExec);
        //        page()->runJavaScript(("updateCurrentNoteHtml();"));
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

void CWizDocumentWebEngine::on_insertCodeHtml_requset(QString strCodeHtml)
{
//    QString strHtml = strCodeHtml;
//    if (WizGetBodyContentFromHtml(strHtml, false))
//    {
    insertCssForCode();
    editorCommandExecuteInsertHtml(strCodeHtml, true);
//    }
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

    m_strCurrentNoteHead.clear();
    m_strCurrentNoteHtml.clear();
    splitHtmlToHeadAndBody(strHtml, m_strCurrentNoteHead, m_strCurrentNoteHtml);

    m_strCurrentNoteHead = m_strCurrentNoteHead + "<link rel=\"stylesheet\" type=\"text/css\" href=\"" +
            m_strDefaultCssFilePath + "\">";

    qDebug() << "view document in editor , current note head : " << m_strCurrentNoteHead;

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

    emit viewNoteRequest(m_strCurrentNoteGUID, m_bCurrentEditing, m_strCurrentNoteHtml, m_strCurrentNoteHead);
    qDebug() << "after send view note request";
//    QString strExec = QString("viewCurrentNote();");
//    QString strExec = QString("viewNote('%1', '%2', '%3', '%4');").arg(m_strCurrentNoteGUID).arg(m_bCurrentEditing).arg(m_strCurrentNoteHtml).arg(m_strCurrentNoteHead);
//    qDebug() << "view note js : " << strExec;
//    page()->runJavaScript(strExec, [&ret, this](const QVariant& returnValue) {
//        ret = returnValue.toBool();
//        // show client
//        MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
//        if (!ret) {
//            window->showClient(false);
//            window->transitionView()->showAsMode(m_strCurrentNoteGUID, CWizDocumentTransitionView::ErrorOccured);
//            return;
//        }

//        window->showClient(true);
//        window->transitionView()->hide();

//        m_timerAutoSave.start();
//        initCheckListEnvironment();

//        //Waiting for the editor initialization complete if it's the first time to load a document.
//        QTimer::singleShot(100, this, SLOT(applySearchKeywordHighlight()));
//        emit viewDocumentFinished();
//    });
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
    page()->runJavaScript(strScript, [&](const QVariant&) {
        initCheckListEnvironment();

        if (editing) {
            setFocus(Qt::MouseFocusReason);
            editorFocus();
        }

        Q_EMIT statusChanged();
    });
}

void CWizDocumentWebEngine::saveDocument(const WIZDOCUMENTDATA& data, bool force)
{
    qDebug() << "web engne save document, guid :  " << data.strGUID << "  editor inited : " << m_bEditorInited << "  noteloaded : " << view()->noteLoaded() << "  editing mode : " << m_bEditingMode;
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
    return synchronousRunJavaScript(strExec).toString();
}

int CWizDocumentWebEngine::editorCommandQueryCommandState(const QString& strCommand)
{
    QString strExec = "editor.queryCommandState('" + strCommand +"');";
    return synchronousRunJavaScript(strExec).toInt();
}

void CWizDocumentWebEngine::editorCommandQueryCommandValue(const QString& strCommand,
                                                           const QWebEngineCallback<const QVariant&>& resultCallback)
{
    QString strExec = "editor.queryCommandValue('" + strCommand +"');";
    page()->runJavaScript(strExec, resultCallback);
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
    QString strUrl = synchronousRunJavaScript("WizGetLinkUrl();").toString();
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
    //TODO:  需要同步执行插叙程序
    editorCommandQueryCommandState("pasteplain", [&](const QVariant& varState) {
        int nState = varState.toInt();

        if ((!bEnable && nState == 1) || (bEnable && nState != 1))
        {
            editorCommandExecuteCommand("pasteplain");
        }
    });

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

    page()->runJavaScript("WizGetLinkUrl();", [this](const QVariant& returnValue) {
        QString strUrl = returnValue.toString();
        m_editorInsertLinkForm->setUrl(strUrl);
        m_editorInsertLinkForm->exec();
    });

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

void CWizDocumentWebEngine::findPre(QString strTxt, bool bCasesensitive)
{
    if (bCasesensitive)
    {
        findText(strTxt, QWebEnginePage::FindBackward & QWebEnginePage::FindCaseSensitively);
    }
    else
    {
        findText(strTxt, QWebEnginePage::FindBackward);
    }
}

void CWizDocumentWebEngine::findNext(QString strTxt, bool bCasesensitive)
{
    if (bCasesensitive)
    {
        findText(strTxt, QWebEnginePage::FindCaseSensitively);
    }
    else
    {
        findText(strTxt);
    }
}

void CWizDocumentWebEngine::replaceCurrent(QString strSource, QString strTarget)
{
    QString strExec = QString("WizReplaceText('%1', '%2', true)").arg(strSource).arg(strTarget);
    page()->runJavaScript(strExec);
}

void CWizDocumentWebEngine::replaceAndFindNext(QString strSource, QString strTarget, bool bCasesensitive)
{
    QString strExec = QString("WizReplaceText('%1', '%2', %3)").arg(strSource).arg(strTarget).arg(bCasesensitive);
    synchronousRunJavaScript(strExec);

    findNext(strSource, bCasesensitive);
    setContentsChanged(true);
}

void CWizDocumentWebEngine::replaceAll(QString strSource, QString strTarget, bool bCasesensitive)
{
    QString strExec = QString("WizRepalceAll('%1', '%2', %3)").arg(strSource).arg(strTarget).arg(bCasesensitive);
    page()->runJavaScript(strExec, [&](const QVariant&) {
        setContentsChanged(true);
    });
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
    page()->runJavaScript("editor.execCommand('saveScene');", [this](const QVariant&) {
        //
        QString strExec = "WizTodo.insertOneTodo();";
        page()->runJavaScript(strExec, [this](const QVariant&) {

            // after insert first checklist, should manual notify editor to save current sence for undo.
            page()->runJavaScript("editor.execCommand('saveScene');", [this](const QVariant&) {
                emit statusChanged();
            });

        });
    });

    return true;
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
    page()->runJavaScript("editor.getPlainTxt()", [this](const QVariant& retTxt) {
        QString strText = retTxt.toString();
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
        emit viewNoteRequest(m_strCurrentNoteGUID, m_bCurrentEditing, m_strCurrentNoteHtml, m_strCurrentNoteHead);

    });
    return true;
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
    WizCodeEditorDialog *dialog = new WizCodeEditorDialog(m_app, this);
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

void CWizDocumentWebEngine::saveAsPDF()
{        //
    page()->runJavaScript("editor.getAllHtml()", [this](const QVariant& html) {
        QString strHtml = html.toString();
        if (strHtml.isEmpty())
            return;

        QString	fileName = QFileDialog::getSaveFileName (this, QString(), QDir::homePath() + "/untitled.pdf", tr("PDF Files (*.pdf)"));
        if (!fileName.isEmpty())
        {
            if (::PathFileExists(fileName))
            {
                ::DeleteFile(fileName);
            }

            QTextDocument textDoc;
            textDoc.setHtml(strHtml);
            QPrinter printer;
            QPrinter::Unit marginUnit =  (QPrinter::Unit)m_app.userSettings().printMarginUnit();
            double marginTop = m_app.userSettings().printMarginValue(wizPositionTop);
            double marginBottom = m_app.userSettings().printMarginValue(wizPositionBottom);
            double marginLeft = m_app.userSettings().printMarginValue(wizPositionLeft);
            double marginRight = m_app.userSettings().printMarginValue(wizPositionRight);
            printer.setPageMargins(marginLeft, marginTop, marginRight, marginBottom, marginUnit);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setColorMode(QPrinter::Color);
            printer.setOutputFileName(fileName);
            //
            textDoc.print(&printer);
        }
    });
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
    //TODO: 需要使用同步查询方式
    QPoint ptPos = mapFromGlobal(point);
    QString strImgSrc = synchronousRunJavaScript(QString("WizGetImgElementByPoint(%1, %2)").
                                                                arg(ptPos.x()).arg(ptPos.y())).toString();

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


void CWizDocumentWebEngine::focusInEditor()
{
    if (m_bEditingMode) {
        Q_EMIT focusIn();
        Q_EMIT statusChanged();
    }
}

void CWizDocumentWebEngine::focusOutEditor()
{
    Q_EMIT focusOut();
    Q_EMIT statusChanged();
}

void CWizDocumentWebEngine::sendEventToChildWidgets(QEvent* event)
{
    foreach (QObject* obj, m_childWidgets)
    {
        QCoreApplication::sendEvent(obj, event);
    }
}

void CWizDocumentWebEngine::setInSeperateWindow(bool inSeperateWindow)
{
    m_bInSeperateWindow = inSeperateWindow;

    if (inSeperateWindow)
    {
        QUrl url = QUrl::fromLocalFile(Utils::PathResolve::skinResourcesPath(Utils::StyleHelper::themeName())
                                       + "webkit_separate_scrollbar.css");
        settings()->setUserStyleSheetUrl(url);
    }
    else
    {
        QUrl url = QUrl::fromLocalFile(Utils::PathResolve::skinResourcesPath(Utils::StyleHelper::themeName())
                                       + "webkit_scrollbar.css");
        settings()->setUserStyleSheetUrl(url);
    }
}

WebEnginePage::WebEnginePage(QObject* parent)
    : QWebEnginePage(parent)
{
    connect(this, SIGNAL(urlChanged(QUrl)), SLOT(on_urlChanged(QUrl)));
}

WebEnginePage::~WebEnginePage()
{

}

void WebEnginePage::on_urlChanged(const QUrl& url)
{
    qDebug() << "web page url changed : " << url;
}

void WebEnginePage::javaScriptConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID)
{
    qDebug() << "console at line : " << lineNumber << "  message :  " << message << " source id : " << sourceID.left(200);
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

void WebEnginePage::load(const QUrl& url)
{
    qDebug() << "web page load called : " << url;
}


#endif
=======
﻿#if 0
#include "WizDocumentWebEngine.h"
#include "WizDocumentWebView.h"
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
//#include <QtWebSockets/QWebSocketServer>
#include <QSemaphore>

#include <QApplication>
#include <QUndoStack>
#include <QDesktopServices>

#ifdef Q_OS_MAC
#include <QMacPasteboardMime>
#endif

#include <coreplugin/icore.h>

#include "WizDef.h"
#include "WizDocumentView.h"
#include "WizMainWindow.h"
#include "share/WizMisc.h"
#include "WizEditorInsertLinkForm.h"
#include "WizEditorInsertTableForm.h"
#include "share/WizObjectDataDownloader.h"
#include "share/WebSocketClientWrapper.h"
#include "share/WebSocketTransport.h"
#include "share/WizDatabaseManager.h"
#include "WizDocumentTransitionView.h"
#include "WizSearchReplaceWidget.h"
#include "widgets/WizCodeEditorDialog.h"
#include "widgets/WizScreenShotWidget.h"
#include "widgets/WizEmailShareDialog.h"
#include "sync/avatar.h"

#include "utils/pathresolve.h"
#include "utils/logger.h"

#include "mac/WizMacHelper.h"

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


WizDocumentWebEngine::WizDocumentWebEngine(WizExplorerApp& app, QWidget* parent)
    : QWebEngineView(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_bEditorInited(false)
    , m_bNewNote(false)
    , m_bNewNoteTitleInited(false)
    , m_bCurrentEditing(false)
    , m_bContentsChanged(false)
    , m_searchReplaceWidget(0)
    , m_bInSeperateWindow(false)
{
    m_parentView = dynamic_cast<WizDocumentView*>(parent);
    Q_ASSERT(m_parentView);

    WizWebEnginePage *webPage = new WizWebEnginePage(this);
    setPage(webPage);

//    connect(webPage, SIGNAL(urlChanged(QUrl)), SLOT(onEditorLinkClicked(QUrl)));

    // FIXME: should accept drop picture, attachment, link etc.
    setAcceptDrops(true);

    // refers
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(m_app.mainWindow());

    m_transitionView = mainWindow->transitionView();

    m_docLoadThread = new WizDocumentWebViewLoaderThread(m_dbMgr, this);
    connect(m_docLoadThread, SIGNAL(loaded(const QString&, const QString, const QString)),
            SLOT(onDocumentReady(const QString&, const QString, const QString)), Qt::QueuedConnection);
    //
    m_docSaverThread = new WizDocumentWebViewSaverThread(m_dbMgr, this);
    connect(m_docSaverThread, SIGNAL(saved(const QString, const QString,bool)),
            SLOT(onDocumentSaved(const QString, const QString,bool)), Qt::QueuedConnection);

    // loading and saving thread
    m_timerAutoSave.setInterval(5*60*1000); // 5 minutes
    connect(&m_timerAutoSave, SIGNAL(timeout()), SLOT(onTimerAutoSaveTimout()));
}

WizDocumentWebEngine::~WizDocumentWebEngine()
{
    if (m_searchReplaceWidget)
        delete m_searchReplaceWidget;
}
void WizDocumentWebEngine::waitForDone()
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


void WizDocumentWebEngine::keyPressEvent(QKeyEvent* event)
{
    qDebug() << "key pressed : " << event;
//    sendEventToChildWidgets(event);

    if (event->key() == Qt::Key_Escape)
    {
        // FIXME: press esc will insert space at cursor if not clear focus
        clearFocus();
        return;
    }
    else if (event->key() == Qt::Key_S
             && event->modifiers() & Qt::ControlModifier)
    {
        saveDocument(view()->note(), false);
        return;
    }
    else if (event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))
    {
        qDebug() << "event modifier matched : " << (Qt::ControlModifier | Qt::ShiftModifier);
        if (event->key() == Qt::Key_Left)
        {
            QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Home , Qt::ShiftModifier, QString());
            sendEventToChildWidgets(&keyPress);
        }
        else if (event->key() == Qt::Key_Right)
        {
            QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_End , Qt::ShiftModifier, QString());
            sendEventToChildWidgets(&keyPress);
        }
    }

    QWebEngineView::keyPressEvent(event);

    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        tryResetTitle();
        if (event->key() == Qt::Key_Enter)
        {
            QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Return , Qt::ShiftModifier, QString());
            sendEventToChildWidgets(&keyPress);
        }
    }
}


void WizDocumentWebEngine::focusInEvent(QFocusEvent *event)
{
    focusInEditor();

    QWebEngineView::focusInEvent(event);

    applySearchKeywordHighlight();
}

void WizDocumentWebEngine::focusOutEvent(QFocusEvent *event)
{
    // because qt will clear focus when context menu popup, we need keep focus there.
    if (event->reason() == Qt::PopupFocusReason)
    {
        event->accept();
        return;
    }

    if (m_bEditingMode && view()->hasFocus())
    {
        event->accept();
        return;
    }

    focusOutEditor();
    QWebEngineView::focusOutEvent(event);
}

bool WizDocumentWebEngine::event(QEvent* event)
{
//    qDebug() << "webengine event called ,  event type : " << event->type();
    return QWebEngineView::event(event);
}

bool WizDocumentWebEngine::eventFilter(QObject* watched, QEvent* event)
{
    switch (event->type()) {
    case QEvent::FocusIn:
    {
        QFocusEvent* focusEvent = dynamic_cast<QFocusEvent*>(event);
        if (focusEvent)
        {
            focusInEvent(focusEvent);
        }
        break;
    }
    case QEvent::FocusOut:
    {
        QFocusEvent* focusEvent = dynamic_cast<QFocusEvent*>(event);
        if (focusEvent)
        {
            focusOutEvent(focusEvent);
        }
        break;
    }
    case QEvent::KeyPress:
    {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent)
        {
            keyPressEvent(keyEvent);
        }
        break;
    }
    default:
        break;
    }
    return QWebEngineView::eventFilter(watched, event);
}

void WizDocumentWebEngine::childEvent(QChildEvent* event)
{
    if (event->type() == QEvent::ChildAdded)
    {
//        event->child()->installEventFilter(this);
        m_childWidgets.append(event->child());
        event->child()->installEventFilter(this);
    }
    else if (event->type() == QEvent::ChildRemoved)
    {
        m_childWidgets.removeOne(event->child());
    }

    QWebEngineView::childEvent(event);
}

void WizDocumentWebEngine::contextMenuEvent(QContextMenuEvent *event)
{
    if (!m_bEditorInited)
        return;

    qDebug() << "contextMenu event called : " << event  << "\n event pos : " << event->pos();
    Q_EMIT showContextMenuRequest(mapToGlobal(event->pos()));
}

void WizDocumentWebEngine::dragEnterEvent(QDragEnterEvent *event)
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

void WizDocumentWebEngine::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS)) {
            if (!event->mimeData()->data(WIZNOTE_MIMEFORMAT_DOCUMENTS).isEmpty()) {
                setFocus();
                event->acceptProposedAction();
            }
    }
}

void WizDocumentWebEngine::onActionTriggered(QWebEnginePage::WebAction act)
{
    if (act == QWebEnginePage::Paste)
        tryResetTitle();
}

void WizDocumentWebEngine::tryResetTitle()
{
    if (m_bNewNoteTitleInited)
        return;

    // if note already modified, maybe title changed by use manuallly
    if (view()->note().tCreated.secsTo(view()->note().tModified) != 0)
        return;

    page()->runJavaScript("editor.getPlainTxt();", [this](const QVariant& returnValue){
        QString strTitle = returnValue.toString();
        strTitle = WizStr2Title(strTitle.left(255));
        if (strTitle.isEmpty())
            return;

        view()->resetTitle(strTitle);
        m_bNewNoteTitleInited = true;
    });
}

void WizDocumentWebEngine::dropEvent(QDropEvent* event)
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
            QImageReader reader(strFileName);
            if (reader.canRead())
            {
                QString strHtml;
                bool bUseCopyFile = true;
                if (WizImage2Html(strFileName, strHtml, bUseCopyFile)) {
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
                        db.DocumentToHtmlLink(document, strHtml, strLink);
                        strLinkHtml += "<p>" + strHtml + "</p>";
                    }
                }
            }

            editorCommandExecuteInsertHtml(strLinkHtml, false);

            nAccepted ++;
        }
    }

    if (nAccepted > 0) {
        event->accept();
        saveDocument(view()->note(), false);
    }
}

WizDocumentView* WizDocumentWebEngine::view() const
{
    return m_parentView;
}

void WizDocumentWebEngine::onTimerAutoSaveTimout()
{
    saveDocument(view()->note(), false);
}

void WizDocumentWebEngine::onTitleEdited(QString strTitle)
{
    WIZDOCUMENTDATA document = view()->note();
    document.strTitle = strTitle;
    // Only sync when contents unchanged. If contents changed would sync after document saved.
    if (!isContentsChanged())
    {
        WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
        mainWindow->quickSyncKb(document.strKbGUID);
    }
}

void WizDocumentWebEngine::onDocumentReady(const QString kbGUID, const QString strGUID, const QString strFileName)
{

    qDebug() << "onDocumentReady";
    m_mapFile.insert(strGUID, strFileName);

    if (m_bEditorInited) {
        resetCheckListEnvironment();
        viewDocumentInEditor(m_bEditingMode);
    } else {
        loadEditor();
    }
}

void WizDocumentWebEngine::onDocumentSaved(const QString kbGUID, const QString strGUID, bool ok)
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

void WizDocumentWebEngine::viewDocument(const WIZDOCUMENTDATA& doc, bool editing)
{
    qDebug() << "view document called, init editor , guid " << doc.strGUID << "  editing mode : " << editing;

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

    qDebug() << "try to load document , guid : " << doc.strGUID;
    // ask extract and load
    m_docLoadThread->load(doc);
}

void WizDocumentWebEngine::reloadNoteData(const WIZDOCUMENTDATA& data)
{
    Q_ASSERT(!data.strGUID.isEmpty());

    // reset only if user not in editing mode
    if (m_bEditingMode && hasFocus())
        return;

    // reload may triggered when update from server or locally reflected by modify
    m_docLoadThread->load(data);
}

void WizDocumentWebEngine::closeDocument(const WIZDOCUMENTDATA& data)
{
    bool isSourceMode = editorCommandQueryCommandState("source");
    if (isSourceMode)
    {
        synchronousRunJavaScript("editor.execCommand('source')");
    }
}

bool WizDocumentWebEngine::resetDefaultCss()
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

    QString strPath = Utils::WizPathResolve::cachePath() + "editor/"+m_dbMgr.db().getUserId()+"/";
    Utils::WizPathResolve::ensurePathExists(strPath);

    m_strDefaultCssFilePath = strPath + "default.css";

    QFile f2(m_strDefaultCssFilePath);
    if (!f2.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        qDebug() << "[Editor]Fail to setup default css code";
        return false;
    }

    f2.write(strCss.toUtf8());
    f2.close();

    emit resetDefaultCss(m_strDefaultCssFilePath);

    return true;
}

void WizDocumentWebEngine::editorResetFont()
{
    resetDefaultCss();
    page()->runJavaScript("updateCss();");
}

void WizDocumentWebEngine::editorFocus()
{
    page()->runJavaScript("editor.focus();");
    emit focusIn();
}

void WizDocumentWebEngine::setEditorEnable(bool enalbe)
{
    if (enalbe)
    {
        page()->runJavaScript("editor.setEnabled();");
        setFocus();
    }
    else
    {
        page()->runJavaScript("editor.setDisabled();");
        clearFocus();
    }
}

void WizDocumentWebEngine::on_ueditorInitFinished()
{
    qDebug() << "on init ueditor finished";
    if (!resetDefaultCss())
        return;

    m_bEditorInited = true;
    viewDocumentInEditor(m_bEditingMode);
    return;
}

void WizDocumentWebEngine::on_viewDocumentFinished(bool ok)
{
    // show client
    WizMainWindow* window = qobject_cast<WizMainWindow *>(m_app.mainWindow());
    if (!ok) {
        window->showClient(false);
        window->transitionView()->showAsMode(m_strCurrentNoteGUID, WizDocumentTransitionView::ErrorOccured);
        return;
    }

    window->showClient(true);
    window->transitionView()->hide();

    m_timerAutoSave.start();
    initCheckListEnvironment();

    //Waiting for the editor initialization complete if it's the first time to load a document.
    QTimer::singleShot(100, this, SLOT(applySearchKeywordHighlight()));
    emit viewDocumentFinished();
}

void WizDocumentWebEngine::runJavaScript(const QString& js, const QWebEngineCallback<const QVariant &> &resultCallback)
{
    page()->runJavaScript(js, resultCallback);
}

void WizDocumentWebEngine::editorCommandQueryCommandState(const QString& strCommand,
                                                           const QWebEngineCallback<const QVariant&>& resultCallback)
{
    QString strExec = "editor.queryCommandState('" + strCommand +"');";
    page()->runJavaScript(strExec, resultCallback);
}

void WizDocumentWebEngine::loadEditor()
{
    qDebug() << "initEditor called";
    if (m_bEditorInited)
        return;



    QString strFileName = Utils::WizPathResolve::resourcesPath() + "files/editor/index.html";
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

void WizDocumentWebEngine::registerJavaScriptWindowObject()
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

void WizDocumentWebEngine::resetCheckListEnvironment()
{
    if (!m_bEditingMode)
    {
        QString strScript = QString("WizTodoReadChecked.clear();");
        page()->runJavaScript(strScript);
    }
}

void WizDocumentWebEngine::initCheckListEnvironment()
{
    if (m_bEditingMode)
    {
        qDebug() << "init checklist in edting mode";
        QString strScript = QString("WizTodo.init('qt');");
        page()->runJavaScript(strScript, [this](const QVariant&) {
            //FIXME: 目前javascript无法直接取得c++函数的返回值，需要主动写入
            page()->runJavaScript(QString("WizTodo.setUserAlias('%1');").arg(getUserAlias()));
            page()->runJavaScript(QString("WizTodo.setUserAvatarFileName('%1');").arg(getUserAvatarFile(24)));
            page()->runJavaScript(QString("WizTodo.setCheckedImageFileName('%1');").arg(getSkinResourcePath() + "checked.png"));
            page()->runJavaScript(QString("WizTodo.setUnCheckedImageFileName('%1');").arg(getSkinResourcePath() + "unchecked.png"));
            page()->runJavaScript(QString("WizTodo.setIsPersonalDocument(%1);").arg(isPersonalDocument()));
        });
    }
    else
    {
        QString strScript = QString("WizTodoReadChecked.init('qt');");
        page()->runJavaScript(strScript, [this](const QVariant&) {
            //FIXME: 目前javascript无法直接取得c++函数的返回值，需要主动写入
            page()->runJavaScript(QString("WizTodoReadChecked.setUserAlias('%1');").arg(getUserAlias()));
            page()->runJavaScript(QString("WizTodoReadChecked.setUserAvatarFileName('%1');").arg(getUserAvatarFile(24)));
            page()->runJavaScript(QString("WizTodoReadChecked.setCheckedImageFileName('%1');").arg(getSkinResourcePath() + "checked.png"));
            page()->runJavaScript(QString("WizTodoReadChecked.setUnCheckedImageFileName('%1');").arg(getSkinResourcePath() + "unchecked.png"));
            page()->runJavaScript(QString("WizTodoReadChecked.setIsPersonalDocument(%1);").arg(isPersonalDocument()));
            page()->runJavaScript(QString("WizTodoReadChecked.setCanEdit(%1);").arg(hasEditPermissionOnCurrentNote()));
            QString strHtml = getCurrentNoteHtml();
            QString base64;
            WizBase64Encode(strHtml.toUtf8(), base64);
            page()->runJavaScript(QString("WizTodoReadChecked.setDocOriginalHtml('%1');").arg(base64));
//            emit setDocOriginalHtml(getCurrentNoteHtml());
        });
    }
}

bool WizDocumentWebEngine::speakHelloWorld()
{
    qDebug() << "Hello world from web engine";
    return true;
}

void WizDocumentWebEngine::setWindowVisibleOnScreenShot(bool bVisible)
{
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(m_app.mainWindow());
    if (mainWindow)
    {
        bVisible ? mainWindow->show() : mainWindow->hide();
    }
}

bool WizDocumentWebEngine::insertImage(const QString& strFileName, bool bCopyFile)
{
    QString strHtml;
    if (WizImage2Html(strFileName, strHtml, bCopyFile)) {
        return editorCommandExecuteInsertHtml(strHtml, true);
    }
    return false;
}

QVariant WizDocumentWebEngine::synchronousRunJavaScript(const QString& strExec)
{
    QVariant returnValue;
    QEventLoop loop;
    qDebug() << "create loop and loop locker before run js : " << strExec;
    page()->runJavaScript(strExec , [&](const QVariant& result) {
        returnValue = result;
        loop.quit();
        qDebug() << "javascript result : " << result;
    });
    qDebug() << "before loop exec";
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    return returnValue;
}

QString WizDocumentWebEngine::getSkinResourcePath() const
{
    return ::WizGetSkinResourcePath(m_app.userSettings().skin());

}

QString WizDocumentWebEngine::getUserAvatarFile(int size) const
{
    QString strFileName;
    QString strUserID = m_dbMgr.db().getUserId();
    if (WizService::WizAvatarHost::customSizeAvatar(strUserID, size, size, strFileName))
        return strFileName;

    return QString();
}

QString WizDocumentWebEngine::getUserAlias() const
{
    QString strKbGUID = view()->note().strKbGUID;
    return m_dbMgr.db(strKbGUID).getUserAlias();

}

bool WizDocumentWebEngine::isPersonalDocument() const
{
    QString strKbGUID = view()->note().strKbGUID;
    QString dbKbGUID = m_dbMgr.db().kbGUID();
    return strKbGUID.isEmpty() || (strKbGUID == dbKbGUID);
}

QString WizDocumentWebEngine::getCurrentNoteHtml() const
{
    WizDatabase& db = m_dbMgr.db(view()->note().strKbGUID);
    QString strFolder = Utils::WizPathResolve::tempDocumentFolder(view()->note().strGUID);
    if (db.extractZiwFileToFolder(view()->note(), strFolder))
    {
        QString strHtmlFile = strFolder + "index.html";
        QString strHtml;
        ::WizLoadUnicodeTextFromFile(strHtmlFile, strHtml);
        return strHtml;
    }

    return QString();
}

bool WizDocumentWebEngine::hasEditPermissionOnCurrentNote() const
{
    WIZDOCUMENTDATA docData = view()->note();
    WizDatabase& db = m_dbMgr.db(docData.strKbGUID);
    return db.canEditDocument(docData) && !WizDatabase::isInDeletedItems(docData.strLocation);
}

bool WizDocumentWebEngine::shareNoteByEmail()
{
    WizEmailShareDialog dlg(m_app);
    dlg.setNote(view()->note());

    dlg.exec();

    return true;
}

void copyFileToFolder(const QString& strFileFoler, const QString& strIndexFile, \
                         const QStringList& strResourceList)
{
    //copy index file
    QString strFolderIndex = strFileFoler + "index.html";
    if (strIndexFile != strFolderIndex)
    {
        QFile::remove(strFolderIndex);
        QFile::copy(strIndexFile, strFolderIndex);
    }

    //copy resources to temp folder
    QString strResourcePath = strFileFoler + "index_files/";
    for (int i = 0; i < strResourceList.count(); i++)
    {
        if (QFile::exists(strResourceList.at(i)))
        {
            QFile::copy(strResourceList.at(i), strResourcePath + WizExtractFileName(strResourceList.at(i)));
        }
    }
}

void WizDocumentWebEngine::saveHtmlToCurrentNote(const QString& strHtml, const QString& strResource)
{
    qDebug() << "save html to current note called : " << strHtml << "   resources  :  " << strResource;
    if (strHtml.isEmpty())
        return;

    WIZDOCUMENTDATA docData = view()->note();
    WizDatabase& db = m_dbMgr.db(docData.strKbGUID);
    QString strFolder = Utils::WizPathResolve::tempDocumentFolder(docData.strGUID);
    //
    QString strHtmlFile = strFolder + "index.html";
    ::WizSaveUnicodeTextToUtf8File(strHtmlFile, strHtml);
    QStringList strResourceList = strResource.split('*');
    copyFileToFolder(strFolder, strHtmlFile, strResourceList);

    db.compressFolderToZiwFile(docData, strFolder);
    bool bNotify = false;
    QString strZiwFile = db.getDocumentFileName(docData.strGUID);
    db.updateDocumentDataMD5(docData, strZiwFile, bNotify);
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
    mainWindow->quickSyncKb(docData.strKbGUID);

    updateNoteHtml();
}

void WizDocumentWebEngine::setCurrentDocumentType(const QString& strType)
{
    WIZDOCUMENTDATA docData = view()->note();
    WizDatabase& db = m_dbMgr.db(docData.strKbGUID);
    docData.strType = strType;
    db.modifyDocumentInfoEx(docData);
}

bool WizDocumentWebEngine::checkListClickable()
{
    if (!m_dbMgr.db(view()->note().strKbGUID).isGroup())
    {
        emit clickingTodoCallBack(false, false);
        return true;
    }

    if (view()->checkListClickable())
    {
        qDebug() << "check list clickable";
        view()->setStatusToEditingByCheckList();
        emit clickingTodoCallBack(false, false);
        return true;
    }
    emit clickingTodoCallBack(true, true);
    return false;
}

void WizDocumentWebEngine::insertCssForCode()
{
    QString strCss = "file://" + Utils::WizPathResolve::resourcesPath() + "files/wiz_code_highlight.css";
    page()->runJavaScript(QString("WizAddCssForCode('%1');").arg(strCss));
//    synchronousRunJavaScript(QString("WizAddCssForCode(%1);").arg(strCss));
}

void WizDocumentWebEngine::onEditorLoadFinished(bool ok)
{
    qDebug() << "on editor load finished , ok : " << ok;
    if (ok)
    {
        registerJavaScriptWindowObject();
        return;
    }

    TOLOG("Wow, loading editor failed!");
}

void WizDocumentWebEngine::onEditorContentChanged()
{
    setContentsChanged(true);
    //
    Q_EMIT statusChanged();
}

void WizDocumentWebEngine::onEditorSelectionChanged()
{
    Q_EMIT statusChanged();
}

void WizDocumentWebEngine::onEditorLinkClicked(const QUrl& url)
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

bool WizDocumentWebEngine::isInternalUrl(const QUrl& url)
{
    return url.scheme().toLower() == "wiz";
}

void WizDocumentWebEngine::viewDocumentByUrl(const QUrl& url)
{
    if (!url.isValid())
        return;

    QString strUrl = url.toString();
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(m_app.mainWindow());
    mainWindow->viewDocumentByWizKMURL(strUrl);
}

void WizDocumentWebEngine::splitHtmlToHeadAndBody(const QString& strHtml, QString& strHead, QString& strBody)
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

void WizDocumentWebEngine::saveEditingViewDocument(const WIZDOCUMENTDATA &data, bool force)
{
    qDebug() << "save editing veiw document , guid : " <<  data.strGUID <<  " \n from thread : " << thread();
    //FIXME: remove me, just for find a image losses bug.
    Q_ASSERT(!data.strGUID.isEmpty());

    if (!force && !isContentsChanged())
        return;

    // check note permission
    if (!m_dbMgr.db(data.strKbGUID).canEditDocument(data)) {
        return;
    }

    //
    setContentsChanged(false);
    //
    QEventLoop loop;
    qDebug() << "create mutex and start to run java script";
    page()->runJavaScript("editor.document.head.innerHTML;", [&](const QVariant& headData) {
        m_strCurrentNoteHead = headData.toString();
        //
        page()->runJavaScript("editor.getContent();", [&](const QVariant& bodyData) {
            m_strCurrentNoteHtml = bodyData.toString();
//            qDebug() <<"get html data from editor , head : " << m_strCurrentNoteHead << " \n body : " << m_strCurrentNoteHtml;
            //
            QString strExec = QString("updateCurrentNoteHtml('%1', '%2', '%3');")
                    .arg(m_strCurrentNoteGUID).arg(m_strCurrentNoteHtml).arg(m_strCurrentNoteHead);
            page()->runJavaScript(strExec, [&](const QVariant&) {
                QString strHead = m_strCurrentNoteHead;
                QRegExp regHead("<link[^>]*" + m_strDefaultCssFilePath + "[^>]*>", Qt::CaseInsensitive);
                strHead.replace(regHead, "");
                //
                QString strHtml = m_strCurrentNoteHtml;
                strHtml = "<html><head>" + strHead + "</head><body>" + strHtml + "</body></html>";
                //
                QString strFileName = m_mapFile.value(data.strKbGUID);
                m_docSaverThread->save(data, strHtml, strFileName, 0);
                qDebug() << "get document data finished,  unlock the meutex";
                loop.quit();
            });
            // update current html to js variant
        });
    });
    //  save document

    loop.exec(QEventLoop::ExcludeUserInputEvents);
    qDebug() << "quit save editing view document ";
}

void WizDocumentWebEngine::saveReadingViewDocument(const WIZDOCUMENTDATA &data, bool force)
{
    Q_UNUSED(force);

    QEventLoop loop;
    QString strScript = QString("WizTodoReadChecked.onDocumentClose();");
    page()->runJavaScript(strScript, [&](const QVariant& returnValue) {
        QString strHtml = returnValue.toString();
        if (!strHtml.isEmpty())
        {
            qDebug() << "save reading document ,get html : " << strHtml;
            QString strFileName = m_mapFile.value(data.strKbGUID);
            m_docSaverThread->save(data, strHtml, strFileName, 0);
        }
        loop.quit();
    });
    loop.exec(QEventLoop::ExcludeUserInputEvents);
}

QString WizDocumentWebEngine::currentNoteGUID()
{
    qDebug() << "currentNoteGUID called ; " << m_strCurrentNoteGUID;
    return m_strCurrentNoteGUID;
}
QString WizDocumentWebEngine::currentNoteHtml()
{
    return m_strCurrentNoteHtml;
}
QString WizDocumentWebEngine::currentNoteHead()
{
    return m_strCurrentNoteHead;
}
bool WizDocumentWebEngine::currentIsEditing()
{
    return m_bCurrentEditing;
}

void WizDocumentWebEngine::updateNoteHtml()
{
    WIZDOCUMENTDATA doc = view()->note();
    WizDatabase& db = m_dbMgr.db(doc.strKbGUID);
    QString strFileName;
    if (db.documentToTempHtmlFile(doc, strFileName))
    {
        QString strHtml;
        if (!WizLoadUnicodeTextFromFile(strFileName, strHtml))
            return;

        splitHtmlToHeadAndBody(strHtml, m_strCurrentNoteHead, m_strCurrentNoteHtml);
        m_strCurrentNoteHead += "<link rel=\"stylesheet\" type=\"text/css\" href=\"" + m_strDefaultCssFilePath + "\">";

        m_strCurrentNoteGUID = doc.strGUID;
        QString strExec = QString("updateCurrentNoteHtml(%1, %2, %3);")
                .arg(m_strCurrentNoteGUID).arg(m_strCurrentNoteHtml).arg(m_strCurrentNoteHead);
        page()->runJavaScript(strExec);
        //        page()->runJavaScript(("updateCurrentNoteHtml();"));
    }
}

void WizDocumentWebEngine::applySearchKeywordHighlight()
{
    WizMainWindow* window = qobject_cast<WizMainWindow *>(m_app.mainWindow());
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

void WizDocumentWebEngine::clearSearchKeywordHighlight()
{
    findText("");
}

void WizDocumentWebEngine::on_insertCodeHtml_requset(QString strCodeHtml)
{
//    QString strHtml = strCodeHtml;
//    if (WizGetBodyContentFromHtml(strHtml, false))
//    {
    insertCssForCode();
    editorCommandExecuteInsertHtml(strCodeHtml, true);
//    }
}

void WizDocumentWebEngine::viewDocumentInEditor(bool editing)
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

    m_strCurrentNoteHead.clear();
    m_strCurrentNoteHtml.clear();
    splitHtmlToHeadAndBody(strHtml, m_strCurrentNoteHead, m_strCurrentNoteHtml);

    m_strCurrentNoteHead = m_strCurrentNoteHead + "<link rel=\"stylesheet\" type=\"text/css\" href=\"" +
            m_strDefaultCssFilePath + "\">";

    qDebug() << "view document in editor , current note head : " << m_strCurrentNoteHead;

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

    emit viewNoteRequest(m_strCurrentNoteGUID, m_bCurrentEditing, m_strCurrentNoteHtml, m_strCurrentNoteHead);
    qDebug() << "after send view note request";
//    QString strExec = QString("viewCurrentNote();");
//    QString strExec = QString("viewNote('%1', '%2', '%3', '%4');").arg(m_strCurrentNoteGUID).arg(m_bCurrentEditing).arg(m_strCurrentNoteHtml).arg(m_strCurrentNoteHead);
//    qDebug() << "view note js : " << strExec;
//    page()->runJavaScript(strExec, [&ret, this](const QVariant& returnValue) {
//        ret = returnValue.toBool();
//        // show client
//        MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
//        if (!ret) {
//            window->showClient(false);
//            window->transitionView()->showAsMode(m_strCurrentNoteGUID, CWizDocumentTransitionView::ErrorOccured);
//            return;
//        }

//        window->showClient(true);
//        window->transitionView()->hide();

//        m_timerAutoSave.start();
//        initCheckListEnvironment();

//        //Waiting for the editor initialization complete if it's the first time to load a document.
//        QTimer::singleShot(100, this, SLOT(applySearchKeywordHighlight()));
//        emit viewDocumentFinished();
//    });
}

void WizDocumentWebEngine::onNoteLoadFinished()
{
    ICore::instance()->emitViewNoteLoaded(view(), view()->note(), true);
}

void WizDocumentWebEngine::setEditingDocument(bool editing)
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
    WizDatabase& db = m_dbMgr.db(view()->note().strKbGUID);
    if (!db.documentFromGuid(view()->note().strGUID, docData))
        return;

    saveDocument(docData, false);

    resetCheckListEnvironment();
    m_bEditingMode = editing;
    //
    QString strScript = QString("setEditing(%1);").arg(editing ? "true" : "false");
    page()->runJavaScript(strScript, [&](const QVariant&) {
        initCheckListEnvironment();

        if (editing) {
            setFocus(Qt::MouseFocusReason);
            editorFocus();
        }

        Q_EMIT statusChanged();
    });
}

void WizDocumentWebEngine::saveDocument(const WIZDOCUMENTDATA& data, bool force)
{
    qDebug() << "web engne save document, guid :  " << data.strGUID << "  editor inited : " << m_bEditorInited << "  noteloaded : " << view()->noteLoaded() << "  editing mode : " << m_bEditingMode;
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

QString WizDocumentWebEngine::editorCommandQueryCommandValue(const QString& strCommand)
{
    QString strExec = "editor.queryCommandValue('" + strCommand +"');";
    return synchronousRunJavaScript(strExec).toString();
}

int WizDocumentWebEngine::editorCommandQueryCommandState(const QString& strCommand)
{
    QString strExec = "editor.queryCommandState('" + strCommand +"');";
    return synchronousRunJavaScript(strExec).toInt();
}

void WizDocumentWebEngine::editorCommandQueryCommandValue(const QString& strCommand,
                                                           const QWebEngineCallback<const QVariant&>& resultCallback)
{
    QString strExec = "editor.queryCommandValue('" + strCommand +"');";
    page()->runJavaScript(strExec, resultCallback);
}

/*
 * Execute command and also save status to undostack.
 * All commands execute from client which may modify document MUST invoke this
 * instead of use frame's evaluateJavascript.
 */
bool WizDocumentWebEngine::editorCommandExecuteCommand(const QString& strCommand,
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

bool WizDocumentWebEngine::editorCommandQueryLink()
{
    QString strUrl = synchronousRunJavaScript("WizGetLinkUrl();").toString();
    if (strUrl.isEmpty())
        return false;

    return true;
}

bool WizDocumentWebEngine::editorCommandQueryMobileFileReceiverState()
{
    return m_app.userSettings().receiveMobileFile();
}

bool WizDocumentWebEngine::editorCommandExecuteInsertHtml(const QString& strHtml, bool bNotSerialize)
{
    QString s = bNotSerialize ? "true" : "false";
    return editorCommandExecuteCommand("insertHtml", "'" + strHtml + "'", s);
}

void WizDocumentWebEngine::setPastePlainTextEnable(bool bEnable)
{
    //TODO:  需要同步执行插叙程序
    editorCommandQueryCommandState("pasteplain", [&](const QVariant& varState) {
        int nState = varState.toInt();

        if ((!bEnable && nState == 1) || (bEnable && nState != 1))
        {
            editorCommandExecuteCommand("pasteplain");
        }
    });

}

bool WizDocumentWebEngine::editorCommandExecuteIndent()
{
    return editorCommandExecuteCommand("indent");
}

bool WizDocumentWebEngine::editorCommandExecuteOutdent()
{
    return editorCommandExecuteCommand("outdent");
}

bool WizDocumentWebEngine::editorCommandExecuteLinkInsert()
{
    if (!m_editorInsertLinkForm) {
        m_editorInsertLinkForm = new WizEditorInsertLinkForm(window());
        connect(m_editorInsertLinkForm, SIGNAL(accepted()), SLOT(on_editorCommandExecuteLinkInsert_accepted()));
    }

    page()->runJavaScript("WizGetLinkUrl();", [this](const QVariant& returnValue) {
        QString strUrl = returnValue.toString();
        m_editorInsertLinkForm->setUrl(strUrl);
        m_editorInsertLinkForm->exec();
    });

    return true;
}

void WizDocumentWebEngine::on_editorCommandExecuteLinkInsert_accepted()
{
    // append http if not exist
    QString strUrl = m_editorInsertLinkForm->getUrl();
    if (strUrl.lastIndexOf("http://", 0, Qt::CaseInsensitive) == -1)
        strUrl = "http://" + strUrl;

    editorCommandExecuteCommand("link", QString("{href: '%1'}").arg(strUrl));
}

bool WizDocumentWebEngine::editorCommandExecuteLinkRemove()
{
    return editorCommandExecuteCommand("unlink");
}

bool WizDocumentWebEngine::editorCommandExecuteFindReplace()
{
    if (!m_searchReplaceWidget)
    {
        m_searchReplaceWidget = new WizSearchReplaceWidget();
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

void WizDocumentWebEngine::findPre(QString strTxt, bool bCasesensitive)
{
    if (bCasesensitive)
    {
        findText(strTxt, QWebEnginePage::FindBackward & QWebEnginePage::FindCaseSensitively);
    }
    else
    {
        findText(strTxt, QWebEnginePage::FindBackward);
    }
}

void WizDocumentWebEngine::findNext(QString strTxt, bool bCasesensitive)
{
    if (bCasesensitive)
    {
        findText(strTxt, QWebEnginePage::FindCaseSensitively);
    }
    else
    {
        findText(strTxt);
    }
}

void WizDocumentWebEngine::replaceCurrent(QString strSource, QString strTarget)
{
    QString strExec = QString("WizReplaceText('%1', '%2', true)").arg(strSource).arg(strTarget);
    page()->runJavaScript(strExec);
}

void WizDocumentWebEngine::replaceAndFindNext(QString strSource, QString strTarget, bool bCasesensitive)
{
    QString strExec = QString("WizReplaceText('%1', '%2', %3)").arg(strSource).arg(strTarget).arg(bCasesensitive);
    synchronousRunJavaScript(strExec);

    findNext(strSource, bCasesensitive);
    setContentsChanged(true);
}

void WizDocumentWebEngine::replaceAll(QString strSource, QString strTarget, bool bCasesensitive)
{
    QString strExec = QString("WizRepalceAll('%1', '%2', %3)").arg(strSource).arg(strTarget).arg(bCasesensitive);
    page()->runJavaScript(strExec, [&](const QVariant&) {
        setContentsChanged(true);
    });
}

bool WizDocumentWebEngine::editorCommandExecuteFontFamily(const QString& strFamily)
{
    return editorCommandExecuteCommand("fontFamily", "'" + strFamily + "'");
}

bool WizDocumentWebEngine::editorCommandExecuteFontSize(const QString& strSize)
{
    return editorCommandExecuteCommand("fontSize", "'" + strSize + "'");
}

void WizDocumentWebEngine::editorCommandExecuteBackColor()
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

void WizDocumentWebEngine::on_editorCommandExecuteBackColor_accepted(const QColor& color)
{
    editorCommandExecuteCommand("backColor", "'" + color.name() + "'");
}

void WizDocumentWebEngine::editorCommandExecuteForeColor()
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

void WizDocumentWebEngine::on_editorCommandExecuteForeColor_accepted(const QColor& color)
{
    editorCommandExecuteCommand("foreColor", "'" + color.name() + "'");
}

bool WizDocumentWebEngine::editorCommandExecuteBold()
{
    return editorCommandExecuteCommand("bold");
}

bool WizDocumentWebEngine::editorCommandExecuteItalic()
{
    return editorCommandExecuteCommand("italic");
}

bool WizDocumentWebEngine::editorCommandExecuteUnderLine()
{
    return editorCommandExecuteCommand("underline");
}

bool WizDocumentWebEngine::editorCommandExecuteStrikeThrough()
{
    return editorCommandExecuteCommand("strikethrough");
}

bool WizDocumentWebEngine::editorCommandExecuteJustifyLeft()
{
    return editorCommandExecuteCommand("justify", "'left'");
}

bool WizDocumentWebEngine::editorCommandExecuteJustifyRight()
{
    return editorCommandExecuteCommand("justify", "'right'");
}

bool WizDocumentWebEngine::editorCommandExecuteJustifyCenter()
{
    return editorCommandExecuteCommand("justify", "'center'");
}

bool WizDocumentWebEngine::editorCommandExecuteJustifyJustify()
{
    return editorCommandExecuteCommand("justify", "'justify'");
}

bool WizDocumentWebEngine::editorCommandExecuteInsertOrderedList()
{
    return editorCommandExecuteCommand("insertOrderedList");
}

bool WizDocumentWebEngine::editorCommandExecuteInsertUnorderedList()
{
    return editorCommandExecuteCommand("insertUnorderedList");
}

bool WizDocumentWebEngine::editorCommandExecuteTableInsert()
{
    if (!m_editorInsertTableForm) {
        m_editorInsertTableForm = new WizEditorInsertTableForm(window());
        connect(m_editorInsertTableForm, SIGNAL(accepted()), SLOT(on_editorCommandExecuteTableInsert_accepted()));
    }

    m_editorInsertTableForm->clear();
    m_editorInsertTableForm->exec();

    return true;
}

void WizDocumentWebEngine::on_editorCommandExecuteTableInsert_accepted()
{
    int nRows = m_editorInsertTableForm->getRows();
    int nCols = m_editorInsertTableForm->getCols();

    if (!nRows && !nCols)
        return;

    editorCommandExecuteCommand("insertTable", QString("{numRows:%1, numCols:%2, border:1, borderStyle:'1px solid #dddddd;'}").arg(nRows).arg(nCols));
}

void WizDocumentWebEngine::on_editorCommandExecuteScreenShot_imageAccepted(QPixmap pix)
{
    QObject *ssSender = qobject_cast<QObject*>(sender());
    if (ssSender)
        delete ssSender;

    setWindowVisibleOnScreenShot(true);

    if (pix.isNull())
        return;

    QString strTempPath = Utils::WizPathResolve::tempPath();
    CString strFileName = strTempPath + WizIntToStr(GetTickCount()) + ".png";
    if (!pix.save(strFileName)) {
        TOLOG("ERROR: Can't save clipboard image to file");
        return;
    }

    insertImage(strFileName, false);
}

void WizDocumentWebEngine::on_editorCommandExecuteScreenShot_finished()
{
    QObject *ssSender = qobject_cast<QObject*>(sender());
    if (ssSender)
        delete ssSender;

    setWindowVisibleOnScreenShot(true);
}

bool WizDocumentWebEngine::editorCommandExecuteInsertHorizontal()
{
    return editorCommandExecuteCommand("horizontal");
}

bool WizDocumentWebEngine::editorCommandExecuteInsertCheckList()
{
    // before insert first checklist, should manual notify editor to save current sence for undo.
    page()->runJavaScript("editor.execCommand('saveScene');", [this](const QVariant&) {
        //
        QString strExec = "WizTodo.insertOneTodo();";
        page()->runJavaScript(strExec, [this](const QVariant&) {

            // after insert first checklist, should manual notify editor to save current sence for undo.
            page()->runJavaScript("editor.execCommand('saveScene');", [this](const QVariant&) {
                emit statusChanged();
            });

        });
    });

    return true;
}

bool WizDocumentWebEngine::editorCommandExecuteInsertImage()
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

bool WizDocumentWebEngine::editorCommandExecuteInsertDate()
{
    return editorCommandExecuteCommand("date");
}

bool WizDocumentWebEngine::editorCommandExecuteInsertTime()
{
    return editorCommandExecuteCommand("time");
}

bool WizDocumentWebEngine::editorCommandExecuteRemoveFormat()
{
    return editorCommandExecuteCommand("removeFormat");
}

bool WizDocumentWebEngine::editorCommandExecutePlainText()
{
    page()->runJavaScript("editor.getPlainTxt()", [this](const QVariant& retTxt) {
        QString strText = retTxt.toString();
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
        emit viewNoteRequest(m_strCurrentNoteGUID, m_bCurrentEditing, m_strCurrentNoteHtml, m_strCurrentNoteHead);

    });
    return true;
}

bool WizDocumentWebEngine::editorCommandExecuteFormatMatch()
{
    return editorCommandExecuteCommand("formatMatch");
}

bool WizDocumentWebEngine::editorCommandExecuteViewSource()
{
    return editorCommandExecuteCommand("source");
}

bool WizDocumentWebEngine::editorCommandExecuteInsertCode()
{
    QString strSelectHtml = page()->selectedText();
    WizCodeEditorDialog *dialog = new WizCodeEditorDialog(m_app, this);
    connect(dialog, SIGNAL(insertHtmlRequest(QString)), SLOT(on_insertCodeHtml_requset(QString)));
    dialog->show();
    dialog->setWindowState(dialog->windowState() & ~Qt::WindowFullScreen | Qt::WindowActive);
    dialog->setCode(strSelectHtml);

    return true;
}

bool WizDocumentWebEngine::editorCommandExecuteMobileImage(bool bReceiveImage)
{
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(m_app.mainWindow());
    if (bReceiveImage && m_app.userSettings().needShowMobileFileReceiverUserGuide())
    {
        mainWindow->showMobileFileReceiverUserGuide();
    }

    m_app.userSettings().setReceiveMobileFile(bReceiveImage);
    mainWindow->setMobileFileReceiverEnable(bReceiveImage);

    return true;
}

bool WizDocumentWebEngine::editorCommandExecuteScreenShot()
{
    WizScreenShotHelper* helper = new WizScreenShotHelper();

    connect(helper, SIGNAL(screenShotCaptured(QPixmap)),
            SLOT(on_editorCommandExecuteScreenShot_imageAccepted(QPixmap)));
    connect(helper, SIGNAL(shotScreenQuit()), SLOT(on_editorCommandExecuteScreenShot_finished()));

    setWindowVisibleOnScreenShot(false);
    QTimer::singleShot(200, helper, SLOT(startScreenShot()));
    return true;
}

#ifdef Q_OS_MAC
bool WizDocumentWebEngine::editorCommandExecuteRemoveStartOfLine()
{
//    triggerPageAction(QWebEnginePage::SelectStartOfLine);
    QKeyEvent delKeyPress(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier, QString());
    return QApplication::sendEvent(this, &delKeyPress);
}
#endif

bool WizDocumentWebEngine::editorCommandExecuteTableDelete()
{
    return editorCommandExecuteCommand("deletetable");
}

bool WizDocumentWebEngine::editorCommandExecuteTableDeleteRow()
{
    return editorCommandExecuteCommand("deleterow");
}

bool WizDocumentWebEngine::editorCommandExecuteTableDeleteCol()
{
    return editorCommandExecuteCommand("deletecol");
}

bool WizDocumentWebEngine::editorCommandExecuteTableInsertRow()
{
    return editorCommandExecuteCommand("insertrow");
}

bool WizDocumentWebEngine::editorCommandExecuteTableInsertRowNext()
{
    return editorCommandExecuteCommand("insertrownext");
}

bool WizDocumentWebEngine::editorCommandExecuteTableInsertCol()
{
    return editorCommandExecuteCommand("insertcol");
}

bool WizDocumentWebEngine::editorCommandExecuteTableInsertColNext()
{
    return editorCommandExecuteCommand("insertcolnext");
}

bool WizDocumentWebEngine::editorCommandExecuteTableInsertCaption()
{
    return editorCommandExecuteCommand("insertcaption");
}

bool WizDocumentWebEngine::editorCommandExecuteTableDeleteCaption()
{
    return editorCommandExecuteCommand("deletecaption");
}

bool WizDocumentWebEngine::editorCommandExecuteTableInsertTitle()
{
    return editorCommandExecuteCommand("inserttitle");
}

bool WizDocumentWebEngine::editorCommandExecuteTableDeleteTitle()
{
    return editorCommandExecuteCommand("deletetitle");
}

bool WizDocumentWebEngine::editorCommandExecuteTableMergeCells()
{
    return editorCommandExecuteCommand("mergecells");
}

bool WizDocumentWebEngine::editorCommandExecuteTalbeMergeRight()
{
    return editorCommandExecuteCommand("mergeright");
}

bool WizDocumentWebEngine::editorCommandExecuteTableMergeDown()
{
    return editorCommandExecuteCommand("mergedown");
}

bool WizDocumentWebEngine::editorCommandExecuteTableSplitCells()
{
    return editorCommandExecuteCommand("splittocells");
}

bool WizDocumentWebEngine::editorCommandExecuteTableSplitRows()
{
    return editorCommandExecuteCommand("splittorows");
}

bool WizDocumentWebEngine::editorCommandExecuteTableSplitCols()
{
    return editorCommandExecuteCommand("splittocols");
}

bool WizDocumentWebEngine::editorCommandExecuteTableAverageRows()
{
    return editorCommandExecuteCommand("averagedistributerow");
}

bool WizDocumentWebEngine::editorCommandExecuteTableAverageCols()
{
    return editorCommandExecuteCommand("averagedistributecol");
}

void WizDocumentWebEngine::saveAsPDF()
{        //
    page()->runJavaScript("editor.getAllHtml()", [this](const QVariant& html) {
        QString strHtml = html.toString();
        if (strHtml.isEmpty())
            return;

        QString	fileName = QFileDialog::getSaveFileName (this, QString(), QDir::homePath() + "/untited.pdf", tr("PDF Files (*.pdf)"));
        if (!fileName.isEmpty())
        {
            if (::PathFileExists(fileName))
            {
                ::DeleteFile(fileName);
            }

            QTextDocument textDoc;
            textDoc.setHtml(strHtml);
            QPrinter printer;
            QPrinter::Unit marginUnit =  (QPrinter::Unit)m_app.userSettings().printMarginUnit();
            double marginTop = m_app.userSettings().printMarginValue(wizPositionTop);
            double marginBottom = m_app.userSettings().printMarginValue(wizPositionBottom);
            double marginLeft = m_app.userSettings().printMarginValue(wizPositionLeft);
            double marginRight = m_app.userSettings().printMarginValue(wizPositionRight);
            printer.setPageMargins(marginLeft, marginTop, marginRight, marginBottom, marginUnit);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setColorMode(QPrinter::Color);
            printer.setOutputFileName(fileName);
            //
            textDoc.print(&printer);
        }
    });
}

void WizDocumentWebEngine::saveAsHtml(const QString& strDirPath)
{
    const WIZDOCUMENTDATA& doc = view()->note();
    WizDatabase& db = m_dbMgr.db(doc.strKbGUID);
    db.exportToHtmlFile(doc, strDirPath);
}

void WizDocumentWebEngine::printDocument()
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

bool WizDocumentWebEngine::findIMGElementAt(QPoint point, QString& strSrc)
{
    //TODO: 需要使用同步查询方式
    QPoint ptPos = mapFromGlobal(point);
    QString strImgSrc = synchronousRunJavaScript(QString("WizGetImgElementByPoint(%1, %2)").
                                                                arg(ptPos.x()).arg(ptPos.y())).toString();

    if (strImgSrc.isEmpty())
        return false;

    strSrc = strImgSrc;
    return true;
}

void WizDocumentWebEngine::undo()
{
    page()->runJavaScript("editor.execCommand('undo')");
    emit statusChanged();
}

void WizDocumentWebEngine::redo()
{
    page()->runJavaScript("editor.execCommand('redo')");
    emit statusChanged();
}


void WizDocumentWebEngine::focusInEditor()
{
    if (m_bEditingMode) {
        Q_EMIT focusIn();
        Q_EMIT statusChanged();
    }
}

void WizDocumentWebEngine::focusOutEditor()
{
    Q_EMIT focusOut();
    Q_EMIT statusChanged();
}

void WizDocumentWebEngine::sendEventToChildWidgets(QEvent* event)
{
    foreach (QObject* obj, m_childWidgets)
    {
        QCoreApplication::sendEvent(obj, event);
    }
}

void WizDocumentWebEngine::setInSeperateWindow(bool inSeperateWindow)
{
    m_bInSeperateWindow = inSeperateWindow;

    if (inSeperateWindow)
    {
        QUrl url = QUrl::fromLocalFile(Utils::WizPathResolve::skinResourcesPath(Utils::WizStyleHelper::themeName())
                                       + "webkit_separate_scrollbar.css");
        settings()->setUserStyleSheetUrl(url);
    }
    else
    {
        QUrl url = QUrl::fromLocalFile(Utils::WizPathResolve::skinResourcesPath(Utils::WizStyleHelper::themeName())
                                       + "webkit_scrollbar.css");
        settings()->setUserStyleSheetUrl(url);
    }
}

WizWebEnginePage::WizWebEnginePage(QObject* parent)
    : QWebEnginePage(parent)
{
    connect(this, SIGNAL(urlChanged(QUrl)), SLOT(on_urlChanged(QUrl)));
}

WizWebEnginePage::~WizWebEnginePage()
{

}

void WizWebEnginePage::on_urlChanged(const QUrl& url)
{
    qDebug() << "web page url changed : " << url;
}

void WizWebEnginePage::javaScriptConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID)
{
    qDebug() << "console at line : " << lineNumber << "  message :  " << message << " source id : " << sourceID.left(200);
}

void WizWebEnginePage::triggerAction(QWebEnginePage::WebAction action, bool checked)
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

void WizWebEnginePage::load(const QUrl& url)
{
    qDebug() << "web page load called : " << url;
}


#endif
>>>>>>> v2.4.0:src/WizDocumentWebEngine.cpp
