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

//#include "wizdownloadobjectdatadialog.h"
#include "wizdef.h"
#include "share/wizobject.h"
#include "share/wizwebengineview.h"


class CWizObjectDownloaderHost;
class CWizEditorInsertLinkForm;
class CWizEditorInsertTableForm;
class CWizDocumentWebView;
class CWizDocumentTransitionView;
class CWizDocumentWebViewWorker;
class QNetworkDiskCache;
class CWizSearchReplaceWidget;

struct WIZODUCMENTDATA;

class CWizDocumentView;

class CWizWaitEvent
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


class CWizDocumentWebViewLoaderThread : public QThread
{
    Q_OBJECT
    //
public:
    CWizDocumentWebViewLoaderThread(CWizDatabaseManager& dbMgr, QObject* parent);

    void load(const WIZDOCUMENTDATA& doc, WizEditorMode editorMode);
    //
    void stop();
    //
    void waitForDone();

protected:
    virtual void run();
    //
    void setCurrentDoc(QString kbGuid, QString docGuid, WizEditorMode editorMode);
    void PeekCurrentDocGUID(QString& kbGUID, QString& docGUID, WizEditorMode& editorMode);
Q_SIGNALS:
    void loaded(const QString kbGUID, const QString strGUID, const QString strFileName, WizEditorMode editorMode);
private:
    bool isEmpty();
private:
    CWizDatabaseManager& m_dbMgr;
    QString m_strCurrentKbGUID;
    QString m_strCurrentDocGUID;
    WizEditorMode m_editorMode;
    QMutex m_mutex;
    CWizWaitEvent m_waitEvent;
    bool m_stop;
};

class CWizDocumentWebViewSaverThread : public QThread
{
    Q_OBJECT
public:
    CWizDocumentWebViewSaverThread(CWizDatabaseManager& dbMgr, QObject* parent);

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
    void PeekData(SAVEDATA& data);
Q_SIGNALS:
    void saved(const QString kbGUID, const QString strGUID, bool ok);
private:
    CWizDatabaseManager& m_dbMgr;
    QMutex m_mutex;
    CWizWaitEvent m_waitEvent;
    bool m_stop;
};

class CWizDocumentWebViewPage: public WizWebEnginePage
{
    Q_OBJECT

public:
    explicit CWizDocumentWebViewPage(CWizDocumentWebView* parent);
    virtual void triggerAction(WebAction action, bool checked = false);
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID);

Q_SIGNALS:
    void actionTriggered(WebAction act);
private:
    CWizDocumentWebView* m_engineView;
};


class CWizDocumentWebView : public WizWebEngineView
{
    Q_OBJECT

public:
    CWizDocumentWebView(CWizExplorerApp& app, QWidget* parent);
    ~CWizDocumentWebView();
    //
    CWizDocumentView* view();
    //
    friend class CWizDocumentWebViewPage;
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

    //only update Html in JS editor, wouldn't refresh WebView display
    void updateNoteHtml();

    // initialize editor style before render, only invoke once.
    bool resetDefaultCss();
    Q_INVOKABLE QString getDefaultCssFilePath() const;
    Q_INVOKABLE QString getMarkdownCssFilePath() const;
    Q_INVOKABLE QString getWizTemplateJsFile() const;
    void resetMarkdownCssPath();

    /* editor related */
    void editorResetFont();
    void editorFocus();
    void enableEditor(bool enalbe);
    QString noteResourcesPath();

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

    void setPastePlainTextEnable(bool bEnable);
    //
    void saveAsPDF();
    void saveAsHtml(const QString& strDirPath);
    void printDocument();
    void shareNoteByEmail();
    void shareNoteByLink();
    //
    void isModified(std::function<void(bool modified)> callback);
    Q_INVOKABLE void setModified(bool b);

    //use undo func provied by editor
    void undo();
    void redo();

    //js environment func
    Q_INVOKABLE QString getSkinResourcePath();
    Q_INVOKABLE QString getUserAvatarFilePath();
    Q_INVOKABLE QString getUserAlias();
    Q_INVOKABLE QString getFormatedDateTime();
    Q_INVOKABLE bool isPersonalDocument();
    Q_INVOKABLE QString getCurrentNoteHtml();
    Q_INVOKABLE bool hasEditPermissionOnCurrentNote();
    Q_INVOKABLE void setCurrentDocumentType(const QString& strType);
    Q_INVOKABLE bool checkListClickable();
    Q_INVOKABLE bool shouldAddCustomCSS();
    Q_INVOKABLE bool canRenderMarkdown();
    Q_INVOKABLE bool canEditNote();
    Q_INVOKABLE QString getLocalLanguage();
    Q_INVOKABLE void OnSelectionChange(const QString& currentStyle);

    Q_PROPERTY(QString userAlias READ getUserAlias)
    Q_PROPERTY(QString userAvatarFilePath READ getUserAvatarFilePath)
    Q_PROPERTY(bool isPersonalDocument READ isPersonalDocument)
    Q_PROPERTY(QString formatedDateTime READ getFormatedDateTime)
    Q_PROPERTY(QString skinResourcePath READ getSkinResourcePath)
    Q_PROPERTY(QString canEditNote READ canEditNote)
    Q_PROPERTY(QString currentNoteHtml READ getCurrentNoteHtml)
    Q_PROPERTY(bool hasEditPermissionOnCurrentNote READ hasEditPermissionOnCurrentNote)
    //
    QNetworkDiskCache* networkCache();    

private:
    void loadDocumentInWeb(WizEditorMode editorMode);
    //
    void getAllEditorScriptAndStypeFileName(QStringList& arrayFile);
    void insertScriptAndStyleCore(QString& strHtml, const QStringList& arrayFiles);
    //
    void tryResetTitle();
    bool onPasteCommand();

    bool isInternalUrl(const QUrl& url);
    void viewDocumentByUrl(const QString& strUrl);
    void viewAttachmentByUrl(const QString& strKbGUID, const QString& strUrl);
    //
    void saveEditingViewDocument(const WIZDOCUMENTDATA& data, bool force, const std::function<void(const QVariant &)> callback);
    void saveReadingViewDocument(const WIZDOCUMENTDATA& data, bool force);    

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
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;
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
    QString m_strDefaultCssFilePath;
    QString m_strMarkdownCssFilePath;

    int m_nWindowID;

    CWizDocumentWebViewLoaderThread* m_docLoadThread;
    CWizDocumentWebViewSaverThread* m_docSaverThread;

    QPointer<CWizEditorInsertLinkForm> m_editorInsertLinkForm;
    QPointer<CWizEditorInsertTableForm> m_editorInsertTableForm;

    CWizSearchReplaceWidget* m_searchReplaceWidget;

public:
    Q_INVOKABLE void onNoteLoadFinished(); // editor callback

public Q_SLOTS:
    void onActionTriggered(QWebEnginePage::WebAction act);

    void onEditorLoadFinished(bool ok);
    void onEditorLinkClicked(QUrl url, QWebEnginePage::NavigationType navigationType, bool isMainFrame, WizWebEnginePage* page);

    void onTimerAutoSaveTimout();

    void onTitleEdited(QString strTitle);

    void onDocumentReady(const QString kbGUID, const QString strGUID, const QString strFileName, WizEditorMode editorMode);
    void onDocumentSaved(const QString kbGUID, const QString strGUID, bool ok);

    void on_editorCommandExecuteLinkInsert_accepted();
    void on_editorCommandExecuteTableInsert_accepted();

    void applySearchKeywordHighlight();
    void clearSearchKeywordHighlight();

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
    void editorCommandExecuteTableInsert();

    // fast operation
    void editorCommandExecuteInsertDate();
    void editorCommandExecuteInsertTime();
    void editorCommandExecuteRemoveFormat();
    void editorCommandExecutePlainText();
    void editorCommandExecuteFormatMatch();
    void editorCommandExecuteInsertHorizontal();
    void editorCommandExecuteInsertCheckList();
    void editorCommandExecuteInsertImage();
    void editorCommandExecuteInsertCode();
    void editorCommandExecuteMobileImage(bool bReceiveImage);
    void editorCommandExecuteScreenShot();
    void on_editorCommandExecuteScreenShot_imageAccepted(QPixmap pix);
    void on_editorCommandExecuteScreenShot_finished();

    // js func
    void resetCheckListEnvironment();
    void initCheckListEnvironment();

Q_SIGNALS:
    // signals for notify command reflect status, triggered when selection, focus, editing mode changed
    void statusChanged(const QString& currentStyle);
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

private slots:
    void on_insertCommentToNote_request(const QString& docGUID, const QString& comment);

private:
    void setWindowVisibleOnScreenShot(bool bVisible);
    void insertImage(const QString& strFileName);
    void addAttachmentThumbnail(const QString strFile, const QString& strGuid);
    void openVipPageInWebBrowser();
    QString getNoteType();

    QString getMailSender();

    //
//    bool shouldAddUserDefaultCSS();
};


#endif // WIZDOCUMENTWEBVIEW_H
