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

#include <QApplication>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QUndoStack>

#include <coreplugin/icore.h>

#include "wizdef.h"
#include "share/wizmisc.h"
#include "wizmainwindow.h"
#include "wizEditorInsertLinkForm.h"
#include "wizEditorInsertTableForm.h"
#include "share/wizObjectDataDownloader.h"
#include "wizDocumentTransitionView.h"
#include "share/wizDatabaseManager.h"
#include "wizDocumentView.h"

#include "utils/pathresolve.h"
#include "utils/logger.h"

using namespace Core;
using namespace Core::Internal;

/*
 * Load and save document
 */

class CWizDocumentWebViewWorker : public QRunnable
{
public:
    enum WorkType
    {
        Loader,
        Saver
    };

    CWizDocumentWebViewWorker(WorkType type)
        : m_type(type)
        , m_bFinished(false)
    {
    }

    WorkType type() const { return m_type; }
    bool isFinished() const { return m_bFinished; }

protected:
    bool m_bFinished;
    WorkType m_type;
};

class CWizDocumentWebViewSaver : public CWizDocumentWebViewWorker
{
public:
    CWizDocumentWebViewSaver(CWizDatabase& db, const QString& strGUID,
                             const QString& strHtml, const QString& strHtmlFile,
                             int nFlags)
        : m_db(db)
        , m_strGUID(strGUID)
        , m_strHtml(strHtml)
        , m_strHtmlFile(strHtmlFile)
        , m_nFlags(nFlags)
        , m_bOk(false)
        , CWizDocumentWebViewWorker(Saver)
    {
        setAutoDelete(false);
    }

    virtual void run()
    {
        WIZDOCUMENTDATA data;
        if (!m_db.DocumentFromGUID(m_strGUID, data)) {
            return;
        }

        m_bOk = m_db.UpdateDocumentData(data, m_strHtml, m_strHtmlFile, m_nFlags);

        m_bFinished = true;
    }

    bool result() const { return m_bOk; }
    QString guid() const { return m_strGUID; }

private:
    CWizDatabase& m_db;
    QString m_strGUID;
    QString m_strHtml;
    QString m_strHtmlFile;
    int m_nFlags;
    bool m_bOk;
};

/*
class CWizDocumentWebViewLoader : public CWizDocumentWebViewWorker
{
public:
    CWizDocumentWebViewLoader(CWizDatabase& db, const QString& strGUID)
        : m_db(db)
        , m_strGUID(strGUID)
        , CWizDocumentWebViewWorker(Loader)
    {
        setAutoDelete(false);
    }

    virtual void run()
    {
        WIZDOCUMENTDATA data;
        if (!m_db.DocumentFromGUID(m_strGUID, data)) {
            return;
        }

        m_db.DocumentToTempHtmlFile(data, m_strHtmlFile);

        m_bFinished = true;
    }

    QString result() const { return m_strHtmlFile; }
    QString guid() const { return m_strGUID; }

private:
    CWizDatabase& m_db;
    QString m_strGUID;
    QString m_strHtmlFile;
};
*/

CWizDocumentWebViewWorkerPool::CWizDocumentWebViewWorkerPool(CWizExplorerApp& app, QObject* parent)
    : m_dbMgr(app.databaseManager())
    , QObject(parent)
{
    m_timer.setInterval(100);
    connect(&m_timer, SIGNAL(timeout()), SLOT(on_timer_timeout()));
}

/*
void CWizDocumentWebViewWorkerPool::load(const WIZDOCUMENTDATA& doc)
{
    if (isDocInLoadingQueue(doc)) {
        return;
    }

    CWizDatabase& db = m_dbMgr.db(doc.strKbGUID);
    CWizDocumentWebViewLoader* loader = new CWizDocumentWebViewLoader(db, doc.strGUID);
    m_workers.push_back(loader);

    QThreadPool::globalInstance()->start(loader);
    m_timer.start();
}
*/

/*
bool CWizDocumentWebViewWorkerPool::isDocInLoadingQueue(const WIZDOCUMENTDATA &doc)
{
    for (int i = m_workers.count() -1; i >= 0; i--) {
        CWizDocumentWebViewLoader* loader = dynamic_cast<CWizDocumentWebViewLoader*>(m_workers.at(i));
        if (loader && loader->guid() == doc.strGUID) {
            return true;
        }
    }

    return false;
}
*/

void CWizDocumentWebViewWorkerPool::save(const WIZDOCUMENTDATA& doc, const QString& strHtml,
          const QString& strHtmlFile, int nFlags)
{
    CWizDatabase& db = m_dbMgr.db(doc.strKbGUID);
    CWizDocumentWebViewSaver* saver = new CWizDocumentWebViewSaver(db, doc.strGUID, strHtml, strHtmlFile, nFlags);
    m_workers.push_back(saver);

    QThreadPool::globalInstance()->start(saver);
    m_timer.start();
}

void CWizDocumentWebViewWorkerPool::on_timer_timeout()
{
    foreach(CWizDocumentWebViewWorker* worker, m_workers) {
        if (worker->isFinished()) {
            if (worker->type() == CWizDocumentWebViewWorker::Saver) {
                CWizDocumentWebViewSaver* saver = dynamic_cast<CWizDocumentWebViewSaver*>(worker);
                Q_EMIT saved(saver->guid(), saver->result());
//            } else if (worker->type() == CWizDocumentWebViewWorker::Loader) {
//                CWizDocumentWebViewLoader* loader = dynamic_cast<CWizDocumentWebViewLoader*>(worker);
//                Q_EMIT loaded(loader->guid(), loader->result());
            } else {
                Q_ASSERT(0);
            }

            m_workers.removeOne(worker);
            delete worker;
        }
    }

    if (m_workers.isEmpty())
        m_timer.stop();
}




/*
 * QWebKit and Editor both have it's own undo stack, when use QWebkit to monitor
 * the modified status and undo/redo stack, QWebPage will ask QUndoStack undoable
 * if require modify status. this means we must push the undo command to undo
 * stack when execute command which will modify the document, otherwise webkit
 * can't be notified.
 * here, we just delegate this action to editor itself.
 */
class EditorUndoCommand : public QUndoCommand
{
public:
    EditorUndoCommand(QWebPage* page) : m_page(page) {}

    virtual void undo()
    {
        m_page->mainFrame()->evaluateJavaScript("editor.execCommand('undo')");
    }

    virtual void redo()
    {
        m_page->mainFrame()->evaluateJavaScript("editor.execCommand('redo')");
    }

private:
    QWebPage* m_page;
};


void CWizDocumentWebViewPage::triggerAction(QWebPage::WebAction typeAction, bool checked)
{
    if (typeAction == QWebPage::Back || typeAction == QWebPage::Forward) {
        return;
    }

    if (typeAction == QWebPage::Paste) {
        on_editorCommandPaste_triggered();
    }

    QWebPage::triggerAction(typeAction, checked);

    Q_EMIT actionTriggered(typeAction);
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
        QString strTempPath = Utils::PathResolve::tempPath();
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



CWizDocumentWebView::CWizDocumentWebView(CWizExplorerApp& app, QWidget* parent)
    : QWebView(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_bEditorInited(false)
    , m_bNewNote(false)
    , m_bNewNoteTitleInited(false)
    , m_noteFrame(0)
    , m_bCurrentEditing(false)
{
    CWizDocumentWebViewPage* page = new CWizDocumentWebViewPage(this);
    setPage(page);

#ifdef QT_DEBUG
    settings()->globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
#endif

    connect(page, SIGNAL(actionTriggered(QWebPage::WebAction)), SLOT(onActionTriggered(QWebPage::WebAction)));

    // minimum page size hint
    setMinimumSize(400, 250);

    // only accept focus by mouse click as the best way to trigger toolbar reset
    setFocusPolicy(Qt::ClickFocus);
    setAttribute(Qt::WA_AcceptTouchEvents, false);

    // FIXME: should accept drop picture, attachment, link etc.
    setAcceptDrops(true);

    // refers
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());



    m_transitionView = mainWindow->transitionView();

    m_docLoadThread = new CWizDocumentWebViewLoaderThread(m_dbMgr);
    connect(m_docLoadThread, SIGNAL(loaded(const QString, const QString)),
            SLOT(onDocumentReady(const QString, const QString)), Qt::QueuedConnection);

    // loading and saving thread
    m_timerAutoSave.setInterval(5*60*1000); // 5 minutes
    connect(&m_timerAutoSave, SIGNAL(timeout()), SLOT(onTimerAutoSaveTimout()));

    m_workerPool = new CWizDocumentWebViewWorkerPool(m_app, this);
    connect(m_workerPool, SIGNAL(loaded(const QString&, const QString&)),
            SLOT(onDocumentReady(const QString&, const QString&)));
    connect(m_workerPool, SIGNAL(saved(const QString&, bool)),
            SLOT(onDocumentSaved(const QString&, bool)));
}

CWizDocumentWebView::~CWizDocumentWebView()
{
    if (0 != m_docLoadThread) {
        delete m_docLoadThread;
        m_docLoadThread = 0;
    }
}

void CWizDocumentWebView::inputMethodEvent(QInputMethodEvent* event)
{
    // On X windows, fcitx flick while preediting, only update while webview end process.
    // maybe it's a QT-BUG?
#ifdef Q_OS_LINUX
    setUpdatesEnabled(false);
    QWebView::inputMethodEvent(event);
    setUpdatesEnabled(true);
#else
    QWebView::inputMethodEvent(event);
#endif

#ifdef Q_OS_MAC
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
#endif // Q_OS_MAC
}

void CWizDocumentWebView::keyPressEvent(QKeyEvent* event)
{
    // special cases process
    if (event->key() == Qt::Key_Escape) {
        // FIXME: press esc will insert space at cursor if not clear focus
        clearFocus();
        return;
    }

#ifdef Q_OS_LINUX
    setUpdatesEnabled(false);
    QWebView::keyPressEvent(event);
    setUpdatesEnabled(true);
#else
    QWebView::keyPressEvent(event);
#endif

    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        tryResetTitle();
    }
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

void CWizDocumentWebView::dragEnterEvent(QDragEnterEvent *event)
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
    }
}

void CWizDocumentWebView::onActionTriggered(QWebPage::WebAction act)
{
    if (act == QWebPage::Paste)
        tryResetTitle();
}

QString str2title(const QString& str)
{
    int idx = str.size() - 1;
    static QString eol("，。？~!#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of line
    foreach(QChar c, eol) {
        int i = str.indexOf(c, 0, Qt::CaseInsensitive);
        if (i != -1 && i < idx) {
            idx = i;
        }
    }

    return str.left(idx);
}

void CWizDocumentWebView::tryResetTitle()
{
    if (m_bNewNoteTitleInited)
        return;

    // if note already modified, maybe title changed by use manuallly
    if (view()->note().tCreated.secsTo(view()->note().tModified) != 0)
        return;

    QWebFrame* f = noteFrame();
    if (!f)
        return;

    QString strTitle = f->documentElement().findFirst("body").findFirst("p").toPlainText();
    strTitle = str2title(strTitle.left(255));

    if (strTitle.isEmpty())
        return;

    view()->resetTitle(strTitle);

    m_bNewNoteTitleInited = true;
}

bool CWizDocumentWebView::image2Html(const QString& strImageFile, QString& strHtml)
{
    QString strDestFile = Utils::PathResolve::tempPath() + qrand() + ".png";

    qDebug() << "[Editor] copy to: " << strDestFile;

    QImage img(strImageFile);
    if (!img.save(strDestFile)) {
        return false;
    }

    strHtml = QString("<img border=\"0\" src=\"file://%1\" />").arg(strDestFile);
    return true;
}

void CWizDocumentWebView::dropEvent(QDropEvent* event)
{
    int nAccepted = 0;
    QList<QUrl> li = event->mimeData()->urls();
    QList<QUrl>::const_iterator it;
    for (it = li.begin(); it != li.end(); it++) {
        QUrl url = *it;
        url.setScheme(0);

        qDebug() << "[Editor] drop: " << url.toString();

        // only process image currently
        QImageReader reader(url.toString());
        if (!reader.canRead())
            continue;

        QString strHtml;
        if (image2Html(url.toString(), strHtml)) {
            editorCommandExecuteInsertHtml(strHtml, true);
            nAccepted++;
        }
    }

    if (nAccepted == li.size()) {
        event->accept();
    }
}

CWizDocumentView* CWizDocumentWebView::view()
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

void CWizDocumentWebView::onTimerAutoSaveTimout()
{
    saveDocument(view()->note(), false);
}

void CWizDocumentWebView::onDocumentReady(const QString strGUID, const QString strFileName)
{
    m_mapFile.insert(strGUID, strFileName);

    if (m_bEditorInited) {
        viewDocumentInEditor(m_bEditingMode);
    } else {
        initEditor();
    }
}

void CWizDocumentWebView::onDocumentSaved(const QString& strGUID, bool ok)
{
    Q_UNUSED(strGUID);

    if (!ok) {
        TOLOG("Save document failed");
    }
}

void CWizDocumentWebView::viewDocument(const WIZDOCUMENTDATA& doc, bool editing)
{
    // set data
    m_bEditingMode = editing;
    m_bNewNote = doc.tCreated.secsTo(QDateTime::currentDateTime()) == 0 ? true : false;
    m_bNewNoteTitleInited = m_bNewNote ? false : true;

    // ask extract and load
//    m_workerPool->load(doc);
    m_docLoadThread->load(doc);
}

void CWizDocumentWebView::reloadNoteData(const WIZDOCUMENTDATA& data)
{
    Q_ASSERT(!data.strGUID.isEmpty());

    // reset only if user not in editing mode
    if (m_bEditingMode && hasFocus())
        return;

    // reload may triggered when update from server or locally reflected by modify
//    m_workerPool->load(data);
    m_docLoadThread->load(data);
}

QString CWizDocumentWebView::getDefaultCssFilePath() const
{
    return m_strDefaultCssFilePath;
}

bool CWizDocumentWebView::resetDefaultCss()
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

    QString strPath = Utils::PathResolve::cachePath() + "editor/";
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

void CWizDocumentWebView::editorResetFont()
{
    resetDefaultCss();
    page()->mainFrame()->evaluateJavaScript("updateCss();");
}

void CWizDocumentWebView::editorFocus()
{
    page()->mainFrame()->evaluateJavaScript("editor.focus();");
}

void CWizDocumentWebView::initEditor()
{
    if (m_bEditorInited)
        return;

    if (!resetDefaultCss())
        return;

    QString strFileName = WizGetResourcesPath() + "files/editor/index.html";
    QString strHtml;
    ::WizLoadUnicodeTextFromFile(strFileName, strHtml);
    QUrl url = QUrl::fromLocalFile(strFileName);

    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            SLOT(onEditorPopulateJavaScriptWindowObject()));

    connect(page()->mainFrame(), SIGNAL(loadFinished(bool)),
            SLOT(onEditorLoadFinished(bool)));

    connect(page(), SIGNAL(linkClicked(const QUrl&)),
            SLOT(onEditorLinkClicked(const QUrl&)));

    connect(page(), SIGNAL(selectionChanged()),
            SLOT(onEditorSelectionChanged()));

    connect(page(), SIGNAL(contentsChanged()),
            SLOT(onEditorContentChanged()));

    page()->mainFrame()->setHtml(strHtml, url);
}

void CWizDocumentWebView::onEditorLoadFinished(bool ok)
{
    if (!ok) {
        m_bEditorInited = false;
        TOLOG("Wow, loading editor failed!");
        return;
    }

    m_bEditorInited = true;
    viewDocumentInEditor(m_bEditingMode);
}

QWebFrame* CWizDocumentWebView::noteFrame()
{
    QList<QWebFrame*> frames = page()->mainFrame()->childFrames();
    for (int i = 0; i < frames.size(); i++) {
        if (frames.at(i)->frameName() == "ueditor_0")
            return frames.at(i);
    }

    Q_ASSERT(0);
    return 0;
}

void CWizDocumentWebView::onEditorPopulateJavaScriptWindowObject()
{
    page()->mainFrame()->addToJavaScriptWindowObject("WizExplorerApp", m_app.object());
    page()->mainFrame()->addToJavaScriptWindowObject("WizEditor", this);
}

void CWizDocumentWebView::onEditorContentChanged()
{
    Q_EMIT statusChanged();
}

void CWizDocumentWebView::onEditorSelectionChanged()
{

#ifdef Q_OS_MAC
    // FIXME: every time change content shuld tell webview to clean the canvas
    if (hasFocus()) {
        update();
    }
#endif // Q_OS_MAC

    Q_EMIT statusChanged();
}

void CWizDocumentWebView::onEditorLinkClicked(const QUrl& url)
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

        ICore::instance()->emitViewNoteRequested(view(), doc);
   }
}

QString escapeJavascriptString(const QString & str)
{
    QString out;
    QRegExp rx("(\\r|\\n|\\\\|\"|\')");
    int pos = 0, lastPos = 0;

    while ((pos = rx.indexIn(str, pos)) != -1)
    {
        out += str.mid(lastPos, pos - lastPos);

        switch (rx.cap(1).at(0).unicode())
        {
        case '\r':
            out += "\\r";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '"':
            out += "\\\"";
            break;
        case '\'':
            out += "\"";
            break;
        }
        pos++;
        lastPos = pos;
    }
    out += str.mid(lastPos);
    return out;
}

QString CWizDocumentWebView::currentNoteGUID()
{
    return m_strCurrentNoteGUID;
}
QString CWizDocumentWebView::currentNoteHtml()
{
    return m_strCurrentNoteHtml;
}
QString CWizDocumentWebView::currentNoteHead()
{
    return m_strCurrentNoteHead;
}
bool CWizDocumentWebView::currentIsEditing()
{
    return m_bCurrentEditing;
}

void CWizDocumentWebView::viewDocumentInEditor(bool editing)
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

    QString strHead;
    QRegExp regh("<head.*>([\\s\\S]*)</head>", Qt::CaseInsensitive);
    if (regh.indexIn(strHtml) != -1) {
        strHead = regh.cap(1).simplified();
    }
    strHead += "<link rel=\"stylesheet\" type=\"text/css\" href=\"" + m_strDefaultCssFilePath + "\">";

    QRegExp regex("<body.*>([\\s\\S]*)</body>", Qt::CaseInsensitive);
    if (regex.indexIn(strHtml) != -1) {
        strHtml = regex.cap(1);
    }

    m_strCurrentNoteGUID = strGUID;
    m_strCurrentNoteHead = strHead;
    m_strCurrentNoteHtml = strHtml;
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

    ret = page()->mainFrame()->evaluateJavaScript(strExec).toBool();
    if (!ret) {
        qDebug() << "[Editor] failed to load note: " << strExec;
        // hide client and show error
        return;
    }

    // show client
    MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
    if (!ret) {
        window->showClient(false);
        window->transitionView()->showAsMode(CWizDocumentTransitionView::ErrorOccured);
        return;
    }

    window->showClient(true);
    window->transitionView()->hide();

    page()->undoStack()->clear();
    m_timerAutoSave.start();

//    if (editing) {                //shouldn't focus the editor,otherwise the titleBar will twinkle.
//        setFocus(Qt::MouseFocusReason);
//        editorFocus();
//    }

    //update();
}

void CWizDocumentWebView::onNoteLoadFinished()
{
    ICore::instance()->emitViewNoteLoaded(view(), view()->note(), true);
}

void CWizDocumentWebView::setEditingDocument(bool editing)
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

    if (m_bEditingMode) {
        saveDocument(view()->note(), false);
    }

    m_bEditingMode = editing;

    QString strScript = QString("setEditing(%1);").arg(editing ? "true" : "false");
    page()->mainFrame()->evaluateJavaScript(strScript);

    if (editing) {
        setFocus(Qt::MouseFocusReason);
        editorFocus();
    }

    Q_EMIT statusChanged();
}

void CWizDocumentWebView::saveDocument(const WIZDOCUMENTDATA& data, bool force)
{
    if (!m_bEditorInited)
        return;

    if (!force && !page()->isModified())
        return;

    // check note permission
    if (!m_dbMgr.db(data.strKbGUID).CanEditDocument(data)) {
        return;
    }

    QString strFileName = m_mapFile.value(data.strGUID);
    QString strHead = page()->mainFrame()->evaluateJavaScript("editor.document.head.innerHTML;").toString();
    QRegExp regHead("<link[^>]*" + m_strDefaultCssFilePath + "[^>]*>", Qt::CaseInsensitive);
    strHead.replace(regHead, "");

    QString strHtml = page()->mainFrame()->evaluateJavaScript("editor.getContent();").toString();
    //QString strPlainTxt = page()->mainFrame()->evaluateJavaScript("editor.getPlainTxt();").toString();
    strHtml = "<html><head>" + strHead + "</head><body>" + strHtml + "</body></html>";

    m_workerPool->save(data, strHtml, strFileName, 0);
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

/*
 * Execute command and also save status to undostack.
 * All commands execute from client which may modify document MUST invoke this
 * instead of use frame's evaluateJavascript.
 */
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

    qDebug() << strExec;

    bool ret = page()->mainFrame()->evaluateJavaScript(strExec).toBool();

    EditorUndoCommand * cmd = new EditorUndoCommand(page());
    page()->undoStack()->push(cmd);

    return ret;
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
    return editorCommandExecuteCommand("insertHtml", "'" + strHtml + "'", s);
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
        m_editorInsertLinkForm = new CWizEditorInsertLinkForm(window());
        connect(m_editorInsertLinkForm, SIGNAL(accepted()), SLOT(on_editorCommandExecuteLinkInsert_accepted()));
    }

    QString strUrl = page()->mainFrame()->evaluateJavaScript("WizGetLinkUrl();").toString();
    m_editorInsertLinkForm->setUrl(strUrl);

    m_editorInsertLinkForm->exec();

    return true;
}

void CWizDocumentWebView::on_editorCommandExecuteLinkInsert_accepted()
{
    // append http if not exist
    QString strUrl = m_editorInsertLinkForm->getUrl();
    if (strUrl.lastIndexOf("http://", 0, Qt::CaseInsensitive) == -1)
        strUrl = "http://" + strUrl;

    editorCommandExecuteCommand("link", QString("{href: '%1'}").arg(strUrl));
}

bool CWizDocumentWebView::editorCommandExecuteLinkRemove()
{
    return editorCommandExecuteCommand("unlink");
}

bool CWizDocumentWebView::editorCommandExecuteFontFamily(const QString& strFamily)
{
    return editorCommandExecuteCommand("fontFamily", "'" + strFamily + "'");
}

bool CWizDocumentWebView::editorCommandExecuteFontSize(const QString& strSize)
{
    return editorCommandExecuteCommand("fontSize", "'" + strSize + "'");
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

    m_colorDialog->exec();
}

void CWizDocumentWebView::on_editorCommandExecuteBackColor_accepted(const QColor& color)
{
    editorCommandExecuteCommand("backColor", "'" + color.name() + "'");
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

    m_colorDialog->exec();
}

void CWizDocumentWebView::on_editorCommandExecuteForeColor_accepted(const QColor& color)
{
    editorCommandExecuteCommand("foreColor", "'" + color.name() + "'");
}

bool CWizDocumentWebView::editorCommandExecuteBold()
{
    return editorCommandExecuteCommand("bold");
}

bool CWizDocumentWebView::editorCommandExecuteItalic()
{
    return editorCommandExecuteCommand("italic");
}

bool CWizDocumentWebView::editorCommandExecuteUnderLine()
{
    return editorCommandExecuteCommand("underline");
}

bool CWizDocumentWebView::editorCommandExecuteStrikeThrough()
{
    return editorCommandExecuteCommand("strikethrough");
}

bool CWizDocumentWebView::editorCommandExecuteJustifyLeft()
{
    return editorCommandExecuteCommand("justify", "'left'");
}

bool CWizDocumentWebView::editorCommandExecuteJustifyRight()
{
    return editorCommandExecuteCommand("justify", "'right'");
}

bool CWizDocumentWebView::editorCommandExecuteJustifyCenter()
{
    return editorCommandExecuteCommand("justify", "'center'");
}

bool CWizDocumentWebView::editorCommandExecuteJustifyJustify()
{
    return editorCommandExecuteCommand("justify", "'justify'");
}

bool CWizDocumentWebView::editorCommandExecuteInsertOrderedList()
{
    return editorCommandExecuteCommand("insertOrderedList");
}

bool CWizDocumentWebView::editorCommandExecuteInsertUnorderedList()
{
    return editorCommandExecuteCommand("insertUnorderedList");
}

bool CWizDocumentWebView::editorCommandExecuteTableInsert()
{
    if (!m_editorInsertTableForm) {
        m_editorInsertTableForm = new CWizEditorInsertTableForm(window());
        connect(m_editorInsertTableForm, SIGNAL(accepted()), SLOT(on_editorCommandExecuteTableInsert_accepted()));
    }

    m_editorInsertTableForm->clear();
    m_editorInsertTableForm->exec();

    return true;
}

void CWizDocumentWebView::on_editorCommandExecuteTableInsert_accepted()
{
    int nRows = m_editorInsertTableForm->getRows();
    int nCols = m_editorInsertTableForm->getCols();

    if (!nRows && !nCols)
        return;

    editorCommandExecuteCommand("insertTable", QString("{numRows:%1, numCols:%2, border:1}").arg(nRows).arg(nCols));
}

bool CWizDocumentWebView::editorCommandExecuteInsertHorizontal()
{
    return editorCommandExecuteCommand("horizontal");
}

bool CWizDocumentWebView::editorCommandExecuteInsertDate()
{
    return editorCommandExecuteCommand("date");
}

bool CWizDocumentWebView::editorCommandExecuteInsertTime()
{
    return editorCommandExecuteCommand("time");
}

bool CWizDocumentWebView::editorCommandExecuteRemoveFormat()
{
    return editorCommandExecuteCommand("removeFormat");
}

bool CWizDocumentWebView::editorCommandExecuteFormatMatch()
{
    return editorCommandExecuteCommand("formatMatch");
}

bool CWizDocumentWebView::editorCommandExecuteViewSource()
{
    return editorCommandExecuteCommand("source");
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


CWizDocumentWebViewLoaderThread::CWizDocumentWebViewLoaderThread(CWizDatabaseManager &dbMgr):m_dbMgr(dbMgr)
{
}

void CWizDocumentWebViewLoaderThread::load(const WIZDOCUMENTDATA &doc)
{
    setCurrentDoc(doc.strKbGUID, doc.strGUID);

    if (!isRunning())
    {
        start();
    }
}

void CWizDocumentWebViewLoaderThread::run()
{
    while (true) {
        QString kbGuid;
        QString docGuid;
        PeekCurrentDocGUID(kbGuid, docGuid);
        //
        CWizDatabase& db = m_dbMgr.db(kbGuid);
        WIZDOCUMENTDATA data;
        if (!db.DocumentFromGUID(docGuid, data)) {
            continue;
        }
        //
        QString strHtmlFile;
        if (db.DocumentToTempHtmlFile(data, strHtmlFile))
        {
            emit loaded(docGuid, strHtmlFile);
        }
    };
}

void CWizDocumentWebViewLoaderThread::setCurrentDoc(QString kbGUID, QString docGUID)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    //
    m_strCurrentKbGUID = kbGUID;
    m_strCurrentDocGUID = docGUID;
    //
    m_waitForData.wakeAll();
}

void CWizDocumentWebViewLoaderThread::PeekCurrentDocGUID(QString& kbGUID, QString& docGUID)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    //
    if (m_strCurrentKbGUID.isEmpty())
        m_waitForData.wait(&m_mutex);
    //
    kbGUID = m_strCurrentKbGUID;
    docGUID = m_strCurrentDocGUID;
    //
    m_strCurrentKbGUID.clear();
    m_strCurrentDocGUID.clear();
}

