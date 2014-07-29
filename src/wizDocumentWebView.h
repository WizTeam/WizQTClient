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

struct WIZODUCMENTDATA;

namespace Core {
class CWizDocumentView;
} // namespace Core


class CWizDocumentWebViewLoaderThread : public QThread
{
    Q_OBJECT
public:
    CWizDocumentWebViewLoaderThread(CWizDatabaseManager& dbMgr);

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
    CWizDocumentWebViewSaverThread(CWizDatabaseManager& dbMgr);

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
    explicit CWizDocumentWebViewPage(QObject* parent = 0) : QWebPage(parent) {}
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

    bool isInited() const { return m_bEditorInited; }
    bool isEditing() const { return m_bEditingMode; }

    Q_INVOKABLE QString currentNoteGUID();
    Q_INVOKABLE QString currentNoteHtml();
    Q_INVOKABLE QString currentNoteHead();
    Q_INVOKABLE bool currentIsEditing();

    //only update Html in JS editor, wouldn't refresh WebView display
    void updateNoteHtml();

    // initialize editor style before render, only invoke once.
    bool resetDefaultCss();
    Q_INVOKABLE QString getDefaultCssFilePath() const;

    /* editor related */
    void editorResetFont();
    void editorFocus();
    void setEditorEnable(bool enalbe);

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

    bool editorCommandExecuteFontFamily(const QString& strFamily);
    bool editorCommandExecuteFontSize(const QString& strSize);
    bool editorCommandExecuteInsertHtml(const QString& strHtml, bool bNotSerialize);

    //
    void saveAsPDF(const QString& fileName);
    bool findIMGElementAt(QPoint point, QString& strSrc);
    //
    Q_INVOKABLE bool isContentsChanged() { return m_bContentsChanged; }
    Q_INVOKABLE void setContentsChanged(bool b) { m_bContentsChanged = b; }

    //use undo func provied by editor
    void undo();
    void redo();

private:
    void initEditor();
    void viewDocumentInEditor(bool editing);
    void tryResetTitle();

    bool isInternalUrl(const QUrl& url);
    void viewDocumentByUrl(const QUrl& url);

    void splitHtmlToHeadAndBody(const QString& strHtml, QString& strHead, QString& strBody);

    //
    void saveEditingViewDocument(const WIZDOCUMENTDATA& data, bool force);
    void saveReadingViewDocument(const WIZDOCUMENTDATA& data, bool force);

protected:
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void inputMethodEvent(QInputMethodEvent* event);
    virtual void focusInEvent(QFocusEvent* event);
    virtual void focusOutEvent(QFocusEvent* event);
    virtual void contextMenuEvent(QContextMenuEvent* event);
    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dropEvent(QDropEvent* event);

private:
    bool image2Html(const QString& strImageFile, QString& strHtml);

private:
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;
    QMap<QString, QString> m_mapFile;

    QString m_strDefaultCssFilePath;

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

    CWizDocumentTransitionView* m_transitionView;
    CWizDocumentWebViewLoaderThread* m_docLoadThread;
    CWizDocumentWebViewSaverThread* m_docSaverThread;

    QPointer<CWizEditorInsertLinkForm> m_editorInsertLinkForm;
    QPointer<CWizEditorInsertTableForm> m_editorInsertTableForm;
    QPointer<QColorDialog> m_colorDialog;

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

    void onDocumentReady(const QString kbGUID, const QString strGUID, const QString strFileName);
    void onDocumentSaved(const QString kbGUID, const QString strGUID, bool ok);

    void on_editorCommandExecuteLinkInsert_accepted();
    void on_editorCommandExecuteTableInsert_accepted();

    void applySearchKeywordHighlight();
    void clearSearchKeywordHighlight();

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
    bool editorCommandExecuteSearchReplace();
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
    bool editorCommandExecuteFormatMatch();
    bool editorCommandExecuteInsertHorizontal();
    bool editorCommandExecuteInsertCheckList();
    bool editorCommandExecuteInsertImage();
    bool editorCommandExecuteViewSource();

    // js func
    void initCheckListEnvironment();

Q_SIGNALS:
    // signals for notify command reflect status, triggered when selection, focus, editing mode changed
    void statusChanged();

    // signals used request reset info toolbar and editor toolbar
    void focusIn();
    void focusOut();
    //

    void requestShowContextMenu(const QPoint& pos);
};

#endif // WIZDOCUMENTWEBVIEW_H
