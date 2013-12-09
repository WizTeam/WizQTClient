#ifndef WIZDOCUMENTWEBVIEW_H
#define WIZDOCUMENTWEBVIEW_H

#include <QWebView>
#include <QTimer>
#include <QPointer>
#include <QMutex>
#include <QColorDialog>

//#include "wizdownloadobjectdatadialog.h"
#include "wizusercipherform.h"
#include "share/wizobject.h"

class CWizObjectDataDownloaderHost;
class CWizEditorInsertLinkForm;
class CWizEditorInsertTableForm;
class CWizDocumentWebView;
class CWizDocumentTransitionView;
class CWizDocumentWebViewWorker;

namespace Core {
class CWizDocumentView;
} // namespace Core


class CWizDocumentWebViewWorkerPool : public QObject
{
    Q_OBJECT

public:
    CWizDocumentWebViewWorkerPool(CWizExplorerApp& app, QObject* parent);

    void load(const WIZDOCUMENTDATA& doc);
    void save(const WIZDOCUMENTDATA& doc, const QString& strHtml,
              const QString& strHtmlFile, int nFlags);

public Q_SLOTS:
    void on_timer_timeout();

Q_SIGNALS:
    void loaded(const QString& strFileName);
    void saved(bool ok);

private:
    CWizDatabaseManager& m_dbMgr;
    QTimer m_timer;
    QList<CWizDocumentWebViewWorker*> m_workers;
};


class CWizDocumentWebViewPage: public QWebPage
{
public:
    explicit CWizDocumentWebViewPage(QObject* parent = 0) : QWebPage(parent) {}
    virtual void triggerAction(QWebPage::WebAction typeAction, bool checked = false);
    virtual void javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID);

    void on_editorCommandPaste_triggered();
};


class CWizDocumentWebView : public QWebView
{
    Q_OBJECT

public:
    CWizDocumentWebView(CWizExplorerApp& app, QWidget* parent);
    Core::CWizDocumentView* view();

    // view and save
    void viewDocument(const WIZDOCUMENTDATA& doc, bool editing);
    void setEditingDocument(bool editing);
    void saveDocument(bool force);

    bool isInited() const { return m_bEditorInited; }
    bool isEditing() const { return m_bEditingMode; }

    //const WIZDOCUMENTDATA& document() { return m_data; }
    void reloadDocument();

    // initialize editor style before render, only invoke once.
    void initEditorStyle();

    /* editor related */
    void editorResetFont();
    void editorFocus();

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

private:
    void viewDocumentInEditor(bool editing);
    void initEditorAndLoadDocument();

    bool isInternalUrl(const QUrl& url);
    void viewDocumentByUrl(const QUrl& url);

protected:
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void inputMethodEvent(QInputMethodEvent* event);
    virtual void focusInEvent(QFocusEvent* event);
    virtual void focusOutEvent(QFocusEvent* event);
    virtual void contextMenuEvent(QContextMenuEvent* event);

private:
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;
    //WIZDOCUMENTDATA m_data;

    QTimer m_timerAutoSave;
    QString m_strHtmlFileName;
    bool m_bEditorInited;
    bool m_bEditingMode;

    CWizDocumentWebViewWorkerPool* m_workerPool;
    CWizObjectDataDownloaderHost* m_downloaderHost;
    CWizDocumentTransitionView* m_transitionView;

    QPointer<CWizUserCipherForm> m_cipherDialog;
    QPointer<CWizEditorInsertLinkForm> m_editorInsertLinkForm;
    QPointer<CWizEditorInsertTableForm> m_editorInsertTableForm;
    QPointer<QColorDialog> m_colorDialog;

public Q_SLOTS:
    void on_editor_selectionChanged();
    void onCipherDialogClosed();
    void on_download_finished(const WIZOBJECTDATA& data, bool bSucceed);

    void on_editor_populateJavaScriptWindowObject();
    void on_editor_loadFinished(bool ok);
    void on_editor_linkClicked(const QUrl& url);
    void on_editor_contentChanged();

    void onTimerAutoSaveTimout();

    void on_documentReady(const QString& strFileName);
    void on_documentSaved(bool ok);

    void on_editorCommandExecuteLinkInsert_accepted();
    void on_editorCommandExecuteTableInsert_accepted();

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
    bool editorCommandExecuteViewSource();

Q_SIGNALS:
    // signals for notify command reflect status, triggered when selection, focus, editing mode changed
    void statusChanged();

    // signals used request reset info toolbar and editor toolbar
    void focusIn();
    void focusOut();

    void requestShowContextMenu(const QPoint& pos);
};


#endif // WIZDOCUMENTWEBVIEW_H
