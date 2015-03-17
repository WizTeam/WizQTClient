#ifndef CWIZDOCUMENTWEBENGINE_H
#define CWIZDOCUMENTWEBENGINE_H

#ifdef USEWEBENGINE
#include <QWebEngineView>
#include <QTimer>
#include <QPointer>
#include <QMutex>
#include <QColorDialog>
#include <QMap>
#include <QThread>
#include <QWaitCondition>
#include <QVariant>


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

class CWizDocumentWebViewLoaderThread;
class CWizDocumentWebViewSaverThread;


class WebEnginePage : public QWebEnginePage
{
    Q_OBJECT
public:
    explicit WebEnginePage(QObject *parent = 0);
    ~WebEnginePage();

public slots:
    void on_urlChanged(const QUrl& url);

protected:
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID);
    virtual void triggerAction(WebAction action, bool checked = false);
    void load(const QUrl &url);
};

class CWizDocumentWebEngine : public QWebEngineView
{
    Q_OBJECT
public:
    CWizDocumentWebEngine(CWizExplorerApp& app, QWidget* parent = 0);
    ~CWizDocumentWebEngine();
    Core::CWizDocumentView* view() const;
    //
    void waitForDone();

    // view and save
    void viewDocument(const WIZDOCUMENTDATA& doc, bool editing);
    void setEditingDocument(bool editing);
    void saveDocument(const WIZDOCUMENTDATA& data, bool force);
    void reloadNoteData(const WIZDOCUMENTDATA& data);
    void closeDocument(const WIZDOCUMENTDATA& data);

    bool isInited() const { return m_bEditorInited; }
    bool isEditing() const { return m_bEditingMode; }

    bool isModified() const { return false;}

    Q_INVOKABLE QString currentNoteGUID();
    Q_INVOKABLE QString currentNoteHtml();
    Q_INVOKABLE QString currentNoteHead();
    Q_INVOKABLE bool currentIsEditing();

    //only update Html in JS editor, wouldn't refresh WebView display
    void updateNoteHtml();

    // initialize editor style before render, only invoke once.
    bool resetDefaultCss();
    Q_SIGNAL void resetDefaultCss(const QString& strPath);

    /* editor related */
    void editorResetFont();
    void editorFocus();
    void setEditorEnable(bool enalbe);
    Q_INVOKABLE void on_ueditorInitFinished();
    Q_INVOKABLE void on_viewDocumentFinished(bool ok);

    Q_INVOKABLE void runJavaScript(const QString& js, const QWebEngineCallback<const QVariant&>& resultCallback);

    // -1: command invalid
    // 0: available
    // 1: executed before
    QString editorQueryStateCommand(const QString &strCommand);
    QString editorQueryValueCommand(const QString& strCommand);
    void editorCommandQueryCommandState(const QString& strCommand,
                                        const QWebEngineCallback<const QVariant &> &resultCallback);
    void editorCommandQueryCommandValue(const QString& strCommand,
                                        const QWebEngineCallback<const QVariant &> &resultCallback);
    bool editorCommandExecuteCommand(const QString& strCommand,
                                     const QString& arg1 = QString(),
                                     const QString& arg2 = QString(),
                                     const QString& arg3 = QString());

    /** these functions could block event loop, cannot be used in event process function
     */
    int editorCommandQueryCommandState(const QString& strCommand);
    QString editorCommandQueryCommandValue(const QString& strCommand);
    // UEditor still miss link discover api
    bool editorCommandQueryLink();
    bool findIMGElementAt(QPoint point, QString& strSrc);



    bool editorCommandQueryMobileFileReceiverState();

    bool editorCommandExecuteFontFamily(const QString& strFamily);
    bool editorCommandExecuteFontSize(const QString& strSize);
    bool editorCommandExecuteInsertHtml(const QString& strHtml, bool bNotSerialize);

    void setPastePlainTextEnable(bool bEnable);

    // functions would called by javascript
    Q_INVOKABLE void saveHtmlToCurrentNote(const QString& strHtml, const QString& strResource);
    Q_INVOKABLE void setCurrentDocumentType(const QString& strType);
    Q_INVOKABLE bool checkListClickable();
    Q_SIGNAL void clickingTodoCallBack(bool cancel, bool needCallAgain);
    Q_SIGNAL void setDocOriginalHtml(const QString& strHtml);
    Q_INVOKABLE void insertCssForCode();

    //
    void saveAsPDF();
    void saveAsHtml(const QString& strDirPath);
    void printDocument();
    bool shareNoteByEmail();
    //
    Q_INVOKABLE bool isContentsChanged() { return m_bContentsChanged; }
    Q_INVOKABLE void setContentsChanged(bool b) { m_bContentsChanged = b; }


    //use undo func provied by editor
    void undo();
    void redo();

    //
    Q_INVOKABLE void focusInEditor();
    Q_INVOKABLE void focusOutEditor();

    //
    void sendEventToChildWidgets(QEvent* event);

private:
    void loadEditor();
    void registerJavaScriptWindowObject();
    void viewDocumentInEditor(bool editing);
    void tryResetTitle();

    bool isInternalUrl(const QUrl& url);
    void viewDocumentByUrl(const QUrl& url);

    void splitHtmlToHeadAndBody(const QString& strHtml, QString& strHead, QString& strBody);

    //
    void saveEditingViewDocument(const WIZDOCUMENTDATA& data, bool force);
    void saveReadingViewDocument(const WIZDOCUMENTDATA& data, bool force);

protected:
    virtual void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
//    virtual void inputMethodEvent(QInputMethodEvent* event);
    virtual void focusInEvent(QFocusEvent* event)  Q_DECL_OVERRIDE;
    virtual void focusOutEvent(QFocusEvent* event) Q_DECL_OVERRIDE;

    virtual bool event(QEvent* event) Q_DECL_OVERRIDE;
    virtual bool eventFilter(QObject* watched, QEvent* event) Q_DECL_OVERRIDE;
    virtual void childEvent(QChildEvent* event) Q_DECL_OVERRIDE;
    virtual void contextMenuEvent(QContextMenuEvent* event) Q_DECL_OVERRIDE;
    virtual void dragEnterEvent(QDragEnterEvent* event) Q_DECL_OVERRIDE;
    virtual void dragMoveEvent(QDragMoveEvent* event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent* event) Q_DECL_OVERRIDE;

private:
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;
    Core::CWizDocumentView* m_parentView;
    QMap<QString, QString> m_mapFile;

    //NOTE: noraml key events is processed by child widgets, if we want create key events by ourself
    // we need to send key events to child widgets
    QList<QObject*> m_childWidgets;

    QString m_strDefaultCssFilePath;

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

    CWizDocumentTransitionView* m_transitionView;
    CWizDocumentWebViewLoaderThread* m_docLoadThread;
    CWizDocumentWebViewSaverThread* m_docSaverThread;

    QPointer<CWizEditorInsertLinkForm> m_editorInsertLinkForm;
    QPointer<CWizEditorInsertTableForm> m_editorInsertTableForm;
    QPointer<QColorDialog> m_colorDialog;

    CWizSearchReplaceWidget* m_searchReplaceWidget;

public:
    Q_INVOKABLE void onNoteLoadFinished(); // editor callback

public Q_SLOTS:
    void onActionTriggered(QWebEnginePage::WebAction act);

//    void onEditorPopulateJavaScriptWindowObject();
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

    void on_insertCodeHtml_requset(QString strCodeHtml);

    /* editor API */

    // font
    void editorCommandExecuteBackColor();
    void on_editorCommandExecuteBackColor_accepted(const QColor& color);
    void editorCommandExecuteForeColor();
    void on_editorCommandExecuteForeColor_accepted(const QColor& color);
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
    bool speakHelloWorld();

Q_SIGNALS:
    // signals for notify command reflect status, triggered when selection, focus, editing mode changed
    void statusChanged();
    void selectAllKeyPressed();
    // signals used request reset info toolbar and editor toolbar
    void focusIn();
    void focusOut();
    //

    void showContextMenuRequest(const QPoint& pos);

    //
    void viewDocumentFinished();
    //
    void viewNoteRequest(const QString& strGUID, bool editMode, const QString& strHtml, const QString& strHead);

private:
    void setWindowVisibleOnScreenShot(bool bVisible);
    bool insertImage(const QString& strFileName, bool bCopyFile);

    //NOTE: this fucntion will block thread, it can not used in event process fucntion, such as contextmenuevent.
    //could cause deadlock
    QVariant synchronousRunJavaScript(const QString& strExec);

    //
    QString getSkinResourcePath() const;
    QString getUserAvatarFile(int size) const;
    QString getUserAlias() const;
    bool isPersonalDocument() const;
    QString getCurrentNoteHtml() const;
    bool hasEditPermissionOnCurrentNote() const;
};

#endif

#endif // CWIZDOCUMENTWEBENGINE_H
