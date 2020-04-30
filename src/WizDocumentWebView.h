#ifndef WIZDOCUMENTWEBVIEW_H
#define WIZDOCUMENTWEBVIEW_H

#include <QWebEngineView>
#include <QTimer>
#include <QPointer>
#include <QMutex>
#include <QColorDialog>
#include <QMap>
#include <QThread>
#include <QWaitCondition>

//#include "WizDownloadObjectDataDialog.h"
#include "WizDef.h"
#include "share/WizObject.h"
#include "share/WizWebEngineView.h"


class WizObjectDownloaderHost;
class WizEditorInsertLinkForm;
class WizEditorInsertTableForm;
class WizDocumentWebView;
class WizDocumentTransitionView;
class CWizDocumentWebViewWorker;
class QNetworkDiskCache;
class WizSearchReplaceWidget;

struct WIZODUCMENTDATA;

class WizDocumentView;

class WizWaitEvent
{
private:
    QMutex m_mutex;
    QWaitCondition m_waitForData;
public:
    void wait()
    {
        QMutexLocker locker(&m_mutex);
        Q_UNUSED(locker);
        //
        m_waitForData.wait(&m_mutex);
    }

    void wakeAll()
    {
        QMutexLocker locker(&m_mutex);
        Q_UNUSED(locker);
        //
        m_waitForData.wakeAll();
    }
};


class WizDocumentWebViewLoaderThread : public QThread
{
    Q_OBJECT
    //
public:
    WizDocumentWebViewLoaderThread(WizDatabaseManager& dbMgr, QObject* parent);

    void load(const WIZDOCUMENTDATA& doc, WizEditorMode editorMode);
    //
    void stop();
    //
    void waitForDone();

protected:
    virtual void run();
    //
    void setCurrentDoc(QString kbGuid, QString docGuid, WizEditorMode editorMode);
    void peekCurrentDocGuid(QString& kbGUID, QString& docGUID, WizEditorMode& editorMode);
Q_SIGNALS:
    void loaded(const QString kbGUID, const QString strGUID, const QString strFileName, WizEditorMode editorMode);
private:
    bool isEmpty();
private:
    WizDatabaseManager& m_dbMgr;
    QString m_strCurrentKbGUID;
    QString m_strCurrentDocGUID;
    WizEditorMode m_editorMode;
    QMutex m_mutex;
    WizWaitEvent m_waitEvent;
    bool m_stop;
};

class WizDocumentWebViewSaverThread : public QThread
{
    Q_OBJECT
public:
    WizDocumentWebViewSaverThread(WizDatabaseManager& dbMgr, QObject* parent);

    void save(const WIZDOCUMENTDATA& doc, const QString& strHtml,
              const QString& strHtmlFile, int nFlags);

    //
    void waitForDone();

private:
    struct SAVEDATA
    {
        WIZDOCUMENTDATA doc;
        QString html;
        QString htmlFile;
        int flags;
    };
    //
    std::vector<SAVEDATA> m_arrayData;
    //
    bool isEmpty();
    SAVEDATA peekFirst();
protected:
    virtual void run();
    //
    void stop();
    void peekData(SAVEDATA& data);
Q_SIGNALS:
    void saved(const QString kbGUID, const QString strGUID, bool ok);
private:
    WizDatabaseManager& m_dbMgr;
    QMutex m_mutex;
    WizWaitEvent m_waitEvent;
    bool m_stop;
};

class WizDocumentWebViewPage: public WizWebEnginePage
{
    Q_OBJECT

public:
    explicit WizDocumentWebViewPage(QWebEngineProfile* profile, QWidget* parent);
    virtual bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame);
    virtual void triggerAction(WebAction action, bool checked = false);
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID);
    //
    void setDocumentWebView(WizDocumentWebView* view) { m_engineView = view; }

Q_SIGNALS:
    void actionTriggered(WebAction act);
private:
    WizDocumentWebView* m_engineView;
};


class WizDocumentWebView : public WizWebEngineView
{
    Q_OBJECT

public:
    WizDocumentWebView(WizExplorerApp& app, QWidget* parent);
    ~WizDocumentWebView();
    //
    WizDocumentView* view() const;
    //
    void clear();
    //
    friend class WizDocumentWebViewPage;
    //
    void waitForDone();

    // view and save
    void viewDocument(const WIZDOCUMENTDATA& doc, WizEditorMode editorMode);
    void setEditorMode(WizEditorMode editorMode);
    void trySaveDocument(const WIZDOCUMENTDATA& data, bool force, std::function<void(const QVariant &)> callback);
    void reloadNoteData(const WIZDOCUMENTDATA& data);

    bool isEditing() const { return m_currentEditorMode == modeEditor; }

    void setNoteTitleInited(bool inited);

    void setInSeperateWindow(bool inSeperateWindow);
    bool isInSeperateWindow() const;

    // initialize editor style before render, only invoke once.
    void replaceDefaultCss(QString& strHtml);

    /* editor related */
    void editorResetFont();
    void editorFocus();
    void enableEditor(bool enalbe);
    QString noteResourcesPath();
    void updateSvg(QString data);
    //
    void editorResetSpellCheck();

    void setIgnoreActiveWindowEvent(bool igoreEvent);

    bool evaluateJavaScript(const QString& js);

    // -1: command invalid
    // 0: available
    // 1: executed before
    void editorCommandQueryCommandState(const QString& strCommand, std::function<void(int state)> callback);
    void editorCommandQueryCommandValue(const QString& strCommand, std::function<void(const QString& value)> callback);
    //
    void editorCommandExecuteCommand(const QString& strCommand,
                                     const QString& arg1 = QString(),
                                     const QString& arg2 = QString(),
                                     const QString& arg3 = QString());

    bool editorCommandQueryMobileFileReceiverState();

    void editorCommandExecuteParagraph(const QString& strType);
    void editorCommandExecuteFontFamily(const QString& strFamily);
    void editorCommandExecuteFontSize(const QString& strSize);
    void editorCommandExecuteInsertHtml(const QString& strHtml, bool bNotSerialize);

    void editorCommandExecutePastePlainText();
    //
    void saveAsPDF();
    void saveAsMarkdown();
    void saveAsHtml();
    void shareNoteByEmail();
    void shareNoteByLink();
    //
    void isModified(std::function<void(bool modified)> callback);
    Q_INVOKABLE void setModified(bool b);

    //use undo func provied by editor
    void undo();
    void redo();

    //js environment func
    Q_INVOKABLE QString getUserGuid();
    Q_INVOKABLE QString getUserAvatarFilePath();
    Q_INVOKABLE QString getUserAlias();
    Q_INVOKABLE bool isPersonalDocument();
    Q_INVOKABLE QString getCurrentNoteHtml();
    Q_INVOKABLE bool hasEditPermissionOnCurrentNote();
    Q_INVOKABLE void changeCurrentDocumentType(const QString& strType);
    Q_INVOKABLE bool checkListClickable();
    Q_INVOKABLE bool shouldAddCustomCSS();
    Q_INVOKABLE bool canRenderMarkdown();
    Q_INVOKABLE bool canEditNote();
    Q_INVOKABLE QString getLocalLanguage();
    Q_INVOKABLE void onSelectionChange(const QString& currentStyle);
    Q_INVOKABLE void onClickedSvg(const QString& data);
    Q_INVOKABLE void saveCurrentNote();
    Q_INVOKABLE void onReturn();
    Q_INVOKABLE void doPaste();
    Q_INVOKABLE void doCopy();
    Q_INVOKABLE void afterCopied();

    Q_INVOKABLE void onMarkerUndoStatusChanged(QString data);
    Q_INVOKABLE void onMarkerInitiated(QString data);

    Q_PROPERTY(QString userGuid READ getUserGuid)
    Q_PROPERTY(QString userAlias READ getUserAlias)
    Q_PROPERTY(QString userAvatarFilePath READ getUserAvatarFilePath)
    Q_PROPERTY(bool isPersonalDocument READ isPersonalDocument)
    Q_PROPERTY(QString canEditNote READ canEditNote)
    Q_PROPERTY(QString currentNoteHtml READ getCurrentNoteHtml NOTIFY currentHtmlChanged STORED false)
    Q_PROPERTY(bool hasEditPermissionOnCurrentNote READ hasEditPermissionOnCurrentNote)
    //
private:
    //
    void saveAsPDFCore(std::function<void()> callback);
    //
    void addDefaultScriptsToDocumentHtml(QString htmlFileName);
    void loadDocumentInWeb(WizEditorMode editorMode);
    //
    void getAllEditorScriptAndStypeFileName(std::map<QString, QString>& arrayFile);
    void insertScriptAndStyleCore(QString& strHtml, const std::map<QString, QString>& files);
    //
    void tryResetTitle();
    bool onPasteCommand();

    bool isInternalUrl(const QUrl& url);
    void viewDocumentByUrl(const QString& strUrl);
    void viewAttachmentByUrl(const QString& strKbGUID, const QString& strUrl);
    //
    void saveEditingViewDocument(const WIZDOCUMENTDATA& data, bool force, const std::function<void(const QVariant &)> callback);
    void saveReadingViewDocument(const WIZDOCUMENTDATA& data, bool force, std::function<void(const QVariant &)> callback);

protected:
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void inputMethodEvent(QInputMethodEvent* event);
    virtual void focusInEvent(QFocusEvent* event);
    virtual void focusOutEvent(QFocusEvent* event);
    virtual void contextMenuEvent(QContextMenuEvent* event);
    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dragMoveEvent(QDragMoveEvent* event);
    virtual void dropEvent(QDropEvent* event);

private:
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;
    QMap<QString, QString> m_mapFile;

    QTimer m_timerAutoSave;
    WizEditorMode m_currentEditorMode;
    bool m_bNewNote;
    bool m_bNewNoteTitleInited;
    //
    QString m_strNoteHtmlFileName;
    QString m_currentNoteHtml;
    //
    bool m_bContentsChanged;
    //
    bool m_ignoreActiveWindowEvent;

    // flag : if current webview is in seperate window, editor background-color will
    //different with webview in mainwindow
    bool m_bInSeperateWindow;

    int m_nWindowID;

    WizDocumentWebViewLoaderThread* m_docLoadThread;
    WizDocumentWebViewSaverThread* m_docSaverThread;
    //
    QPointer<WizEditorInsertLinkForm> m_editorInsertLinkForm;

    WizSearchReplaceWidget* m_searchReplaceWidget;
public:
    Q_INVOKABLE void onNoteLoadFinished(); // editor callback

public Q_SLOTS:
    void onActionTriggered(QWebEnginePage::WebAction act);

    void onEditorLoadFinished(bool ok);
    void onEditorLinkClicked(QUrl url, QWebEnginePage::NavigationType navigationType, bool isMainFrame, WizWebEnginePage* page);
    void onOpenLinkInNewWindow(QUrl url);

    void onTimerAutoSaveTimout();

    void onTitleEdited(QString strTitle);

    void onDocumentReady(const QString kbGUID, const QString strGUID, const QString strFileName, WizEditorMode editorMode);
    void onDocumentSaved(const QString kbGUID, const QString strGUID, bool ok);

    void on_editorCommandExecuteLinkInsert_accepted();

    void on_insertCodeHtml_requset(QString strOldHtml);

    /* editor API */

    // font
    void editorCommandExecuteBackColor(const QColor& color);
    void editorCommandExecuteForeColor(const QColor& color);
    void editorCommandExecuteBold();
    void editorCommandExecuteItalic();
    void editorCommandExecuteUnderLine();
    void editorCommandExecuteStrikeThrough();

    void editorCommandExecuteLinkInsert();
    void editorCommandExecuteLinkRemove();

    // search and repalce
    void editorCommandExecuteFindReplace();
    void findPre(QString strTxt, bool bCasesensitive);
    void findNext(QString strTxt, bool bCasesensitive);
    void replaceCurrent(QString strSource, QString strTarget);
    void replaceAndFindNext(QString strSource, QString strTarget, bool bCasesensitive);
    void replaceAll(QString strSource, QString strTarget, bool bCasesensitive);

    // format
    void editorCommandExecuteIndent();
    void editorCommandExecuteOutdent();

    void editorCommandExecuteJustifyLeft();
    void editorCommandExecuteJustifyRight();
    void editorCommandExecuteJustifyCenter();
    void editorCommandExecuteJustifyJustify();

    void editorCommandExecuteInsertOrderedList();
    void editorCommandExecuteInsertUnorderedList();
    //
    void editorCommandExecuteTableInsert(int row, int col);

    // fast operation
    void editorCommandExecuteInsertDate();
    void editorCommandExecuteInsertTime();
    void editorCommandExecuteRemoveFormat();
    void editorCommandExecuteFormatPainterOn(bool multi);
    void editorCommandExecuteFormatPainterOff();
    void editorCommandExecuteInsertHorizontal();
    void editorCommandExecuteInsertCheckList();
    void editorCommandExecuteInsertImage();
    void editorCommandExecuteStartMarkup();
    void editorCommandExecuteStopMarkup();
    void editorCommandExecuteInsertPainter();
    void editorCommandExecuteInsertCode();
    void editorCommandExecuteMobileImage(bool bReceiveImage);
    void editorCommandExecuteScreenShot();
    void on_editorCommandExecuteScreenShot_imageAccepted(QPixmap pix);
    void on_editorCommandExecuteScreenShot_finished();
    //
    void editorExecJs(QString js);
    void onViewMindMap(bool on);

Q_SIGNALS:
    // signals for notify command reflect status, triggered when selection, focus, editing mode changed
    void statusChanged(const QString& currentStyle);
    void markerUndoStatusChanged(const QString& data);
    void markerInitiated(const QString& data);
    //
    void selectAllKeyPressed();
    // signals used request reset info toolbar and editor toolbar
    void focusIn();
    void focusOut();
    //
    void showContextMenuRequest(const QPoint& pos);
    void updateEditorToolBarRequest();
    //
    void viewDocumentFinished();
    //
    void shareDocumentByLinkRequest(const QString& strKbGUID, const QString& strGUID);

    // signal connect to checklist in javascript
    void clickingTodoCallBack(bool cancel, bool needCallAgain);
    //
    void currentHtmlChanged();

private slots:
    void on_insertCommentToNote_request(const QString& docGUID, const QString& comment);

private:
    void setWindowVisibleOnScreenShot(bool bVisible);
    void insertImage(const QString& strFileName);
    void addAttachmentThumbnail(const QString strFile, const QString& strGuid);
    void openVipPageInWebBrowser();
    QString getNoteType();

    void getMailSender(std::function<void(QString)> callback);

    //
    void innerFindText(QString text, bool next, bool matchCase);
    //
    QString getHighlightKeywords();
    //
//    bool shouldAddUserDefaultCSS();
public:
    bool isOutline() const;
};


#endif // WIZDOCUMENTWEBVIEW_H
