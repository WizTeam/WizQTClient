#ifndef WIZDOCUMENTWEBVIEW_H
#define WIZDOCUMENTWEBVIEW_H

#include <QWebView>
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


class CWizObjectDataDownloaderHost;
class CWizEditorInsertLinkForm;
class CWizEditorInsertTableForm;
class CWizDocumentWebView;
class CWizDocumentTransitionView;
class CWizDocumentWebViewWorker;
class QNetworkDiskCache;
class CWizSearchReplaceWidget;

struct WIZODUCMENTDATA;

namespace Core {
class CWizDocumentView;
} // namespace Core


class CWizDocumentWebViewLoaderThread : public QThread
{
    Q_OBJECT
public:
    CWizDocumentWebViewLoaderThread(CWizDatabaseManager& dbMgr, QObject* parent);

    void load(const WIZDOCUMENTDATA& doc);
    //
    void stop();
    //
    void waitForDone();

protected:
    virtual void run();
    //
    void setCurrentDoc(QString kbGuid, QString docGuid);
    void PeekCurrentDocGUID(QString& kbGUID, QString& docGUID);
Q_SIGNALS:
    void loaded(const QString kbGUID, const QString strGUID, const QString strFileName);
private:
    CWizDatabaseManager& m_dbMgr;
    QString m_strCurrentKbGUID;
    QString m_strCurrentDocGUID;
    QMutex m_mutex;
    QWaitCondition m_waitForData;
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
    QWaitCondition m_waitForData;
    bool m_stop;
};

class CWizDocumentWebViewPage: public QWebPage
{
    Q_OBJECT

public:
    explicit CWizDocumentWebViewPage(QObject* parent = 0);
    virtual void triggerAction(QWebPage::WebAction typeAction, bool checked = false);
    virtual void javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID);

    void on_editorCommandPaste_triggered();

Q_SIGNALS:
    void actionTriggered(QWebPage::WebAction act);
};

class CWizDocumentWebView : public QWebView
{
    Q_OBJECT

public:
    CWizDocumentWebView(CWizExplorerApp& app, QWidget* parent);
    ~CWizDocumentWebView();
    Core::CWizDocumentView* view();
    QWebFrame* noteFrame();
    //
    void waitForDone();

    // view and save
    void viewDocument(const WIZDOCUMENTDATA& doc, bool editing);
    void setEditingDocument(bool editing);
    void saveDocument(const WIZDOCUMENTDATA& data, bool force);
    void reloadNoteData(const WIZDOCUMENTDATA& data);
    void closeDocument(const WIZDOCUMENTDATA& doc);

    bool isInited() const { return m_bEditorInited; }
    bool isEditing() const { return m_bEditingMode; }

    void setInSeperateWindow(bool inSeperateWindow);
    bool isInSeperateWindow() const;

    Q_INVOKABLE QString currentNoteGUID();
    Q_INVOKABLE QString currentNoteHtml();
    Q_INVOKABLE QString currentNoteHead();
    Q_INVOKABLE bool currentIsEditing();

    //only update Html in JS editor, wouldn't refresh WebView display
    void updateNoteHtml();

    // initialize editor style before render, only invoke once.
    bool resetDefaultCss();
    Q_INVOKABLE QString getDefaultCssFilePath() const;
    Q_INVOKABLE QString getWizReaderDependencyFilePath() const;
    Q_INVOKABLE QString getWizReaderFilePath() const;
    Q_INVOKABLE QString getMarkdownCssFilePath() const;
    void resetMarkdownCssPath();

    /* editor related */
    void editorResetFont();
    void editorFocus();
    void setEditorEnable(bool enalbe);

    void setIgnoreActiveWindowEvent(bool igoreEvent);

    bool evaluateJavaScript(const QString& js);

    // -1: command invalid
    // 0: available
    // 1: executed before
    int editorCommandQueryCommandState(const QString& strCommand);
    QString editorCommandQueryCommandValue(const QString& strCommand);
    bool editorCommandExecuteCommand(const QString& strCommand,
                                     const QString& arg1 = QString(),
                                     const QString& arg2 = QString(),
                                     const QString& arg3 = QString());

    // UEditor still miss link discover api
    bool editorCommandQueryLink();

    bool editorCommandQueryMobileFileReceiverState();

    bool editorCommandExecuteParagraph(const QString& strType);
    bool editorCommandExecuteFontFamily(const QString& strFamily);
    bool editorCommandExecuteFontSize(const QString& strSize);
    bool editorCommandExecuteInsertHtml(const QString& strHtml, bool bNotSerialize);

    void setPastePlainTextEnable(bool bEnable);
    //
    void saveAsPDF();
    void saveAsHtml(const QString& strDirPath);
    void printDocument();
    void shareNoteByEmail();
    void shareNoteByLink();
    bool findIMGElementAt(QPoint point, QString& strSrc);
    //
    Q_INVOKABLE bool isContentsChanged() { return m_bContentsChanged; }
    Q_INVOKABLE void setContentsChanged(bool b);

    //use undo func provied by editor
    void undo();
    void redo();

    //js environment func
    Q_INVOKABLE QString getSkinResourcePath();
    Q_INVOKABLE QString getUserAvatarFilePath(int size);
    Q_INVOKABLE QString getUserAlias();
    Q_INVOKABLE QString getFormatedDateTime();
    Q_INVOKABLE bool isPersonalDocument();
    Q_INVOKABLE QString getCurrentNoteHtml();
    Q_INVOKABLE void saveHtmlToCurrentNote(const QString& strHtml, const QString& strResource);
    Q_INVOKABLE bool hasEditPermissionOnCurrentNote();
    Q_INVOKABLE void setCurrentDocumentType(const QString& strType);
    Q_INVOKABLE bool checkListClickable();
    //
    QNetworkDiskCache* networkCache();

private:
    void initEditor();
    void resetEditor();
    void viewDocumentInEditor(bool editing);
    void viewDocumentWithoutEditor();
    void tryResetTitle();    

    bool isInternalUrl(const QUrl& url);
    void viewDocumentByUrl(const QString& strUrl);
    void viewAttachmentByUrl(const QString& strKbGUID, const QString& strUrl);
    //
    void saveEditingViewDocument(const WIZDOCUMENTDATA& data, bool force);
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
    virtual void wheelEvent(QWheelEvent* ev);

private:
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;
    QMap<QString, QString> m_mapFile;

    QWebFrame* m_noteFrame;

    QTimer m_timerAutoSave;
    bool m_bEditorInited;
    bool m_bEditingMode;
    bool m_bNewNote;
    bool m_bNewNoteTitleInited;
    //
    QString m_strCurrentNoteGUID;
    QString m_strCurrentNoteHead;
    QString m_strCurrentNoteHtml;
    bool m_bCurrentEditing;
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
    void onActionTriggered(QWebPage::WebAction act);

    void onEditorPopulateJavaScriptWindowObject();
    void onEditorLoadFinished(bool ok);
    void onEditorLinkClicked(const QUrl& url);
    void onEditorContentChanged();
    void onEditorSelectionChanged();

    void onTimerAutoSaveTimout();

    void onTitleEdited(QString strTitle);

    void onDocumentReady(const QString kbGUID, const QString strGUID, const QString strFileName);
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
    bool editorCommandExecuteBold();
    bool editorCommandExecuteItalic();
    bool editorCommandExecuteUnderLine();
    bool editorCommandExecuteStrikeThrough();

    bool editorCommandExecuteLinkInsert();
    bool editorCommandExecuteLinkRemove();

    // search and repalce
    bool editorCommandExecuteFindReplace();
    void findPre(QString strTxt, bool bCasesensitive);
    void findNext(QString strTxt, bool bCasesensitive);
    void replaceCurrent(QString strSource, QString strTarget);
    void replaceAndFindNext(QString strSource, QString strTarget, bool bCasesensitive);
    void replaceAll(QString strSource, QString strTarget, bool bCasesensitive);

    // format
    bool editorCommandExecuteIndent();
    bool editorCommandExecuteOutdent();

    bool editorCommandExecuteJustifyLeft();
    bool editorCommandExecuteJustifyRight();
    bool editorCommandExecuteJustifyCenter();
    bool editorCommandExecuteJustifyJustify();

    bool editorCommandExecuteInsertOrderedList();
    bool editorCommandExecuteInsertUnorderedList();

    // table
    bool editorCommandExecuteTableInsert();
    bool editorCommandExecuteTableDelete();
    bool editorCommandExecuteTableDeleteRow();
    bool editorCommandExecuteTableDeleteCol();
    bool editorCommandExecuteTableInsertRow();
    bool editorCommandExecuteTableInsertRowNext();
    bool editorCommandExecuteTableInsertCol();
    bool editorCommandExecuteTableInsertColNext();
    bool editorCommandExecuteTableInsertCaption();
    bool editorCommandExecuteTableDeleteCaption();
    bool editorCommandExecuteTableInsertTitle();
    bool editorCommandExecuteTableDeleteTitle();
    bool editorCommandExecuteTableMergeCells();
    bool editorCommandExecuteTalbeMergeRight();
    bool editorCommandExecuteTableMergeDown();
    bool editorCommandExecuteTableSplitCells();
    bool editorCommandExecuteTableSplitRows();
    bool editorCommandExecuteTableSplitCols();
    bool editorCommandExecuteTableAverageRows();
    bool editorCommandExecuteTableAverageCols();
    bool editorCommandExecuteTableCellAlignLeftTop();
    bool editorCommandExecuteTableCellAlignTop();
    bool editorCommandExecuteTableCellAlignRightTop();
    bool editorCommandExecuteTableCellAlignLeft();
    bool editorCommandExecuteTableCellAlignCenter();
    bool editorCommandExecuteTableCellAlignRight();
    bool editorCommandExecuteTableCellAlignLeftBottom();
    bool editorCommandExecuteTableCellAlignBottom();
    bool editorCommandExecuteTableCellAlignRightBottom();

    // fast operation
    bool editorCommandExecuteInsertDate();
    bool editorCommandExecuteInsertTime();
    bool editorCommandExecuteRemoveFormat();
    bool editorCommandExecutePlainText();
    bool editorCommandExecuteFormatMatch();
    bool editorCommandExecuteInsertHorizontal();
    bool editorCommandExecuteInsertCheckList();
    bool editorCommandExecuteInsertImage();
    bool editorCommandExecuteViewSource();
    bool editorCommandExecuteInsertCode();
    bool editorCommandExecuteMobileImage(bool bReceiveImage);
    bool editorCommandExecuteScreenShot();
    void on_editorCommandExecuteScreenShot_imageAccepted(QPixmap pix);
    void on_editorCommandExecuteScreenShot_finished();

#ifdef Q_OS_MAC
    bool editorCommandExecuteRemoveStartOfLine();
#endif

    // js func
    void resetCheckListEnvironment();
    void initCheckListEnvironment();

Q_SIGNALS:
    // signals for notify command reflect status, triggered when selection, focus, editing mode changed
    void statusChanged();
    void selectAllKeyPressed();
    // signals used request reset info toolbar and editor toolbar
    void focusIn();
    void focusOut();
    //
    void contentsChanged();


    void showContextMenuRequest(const QPoint& pos);
    void updateEditorToolBarRequest();
    //
    void viewDocumentFinished();
    //
    void shareDocumentByLinkRequest(const QString& strKbGUID, const QString& strGUID);

    // signal connect to checklist in javascript
    void clickingTodoCallBack(bool cancel, bool needCallAgain);

private:
    void setWindowVisibleOnScreenShot(bool bVisible);
    bool insertImage(const QString& strFileName, bool bCopyFile);
    void closeSourceMode();
    void addAttachmentThumbnail(const QString strFile, const QString& strGuid);
    void openVipPageInWebBrowser();

    QString getMailSender();
};

#endif // WIZDOCUMENTWEBVIEW_H
