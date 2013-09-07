#ifndef WIZDOCUMENTWEBVIEW_H
#define WIZDOCUMENTWEBVIEW_H

#include <QWebView>
#include <QTimer>
#include <QPointer>
#include <QMutex>
#include <QColorDialog>

#include "wizdownloadobjectdatadialog.h"
#include "wizusercipherform.h"

class CWizObjectDataDownloaderHost;
class CWizEditorInsertLinkForm;
class CWizEditorInsertTableForm;
class CWizDocumentWebView;
class CWizDocumentTransitionView;

// FIXME: class name is not corret for this usage.
// Renderer thread responsible for loading, saving document
class CWizDocumentWebViewRenderer : public QObject
{
    Q_OBJECT

public:
    CWizDocumentWebViewRenderer(CWizExplorerApp& app);

    const WIZDOCUMENTDATA& data() { return m_data; }
    void setData(const WIZDOCUMENTDATA& doc);

    void load();
    void save(const WIZDOCUMENTDATA& data,
              const QString& strHtml,
              const QString& strHtmlFile,
              int nFlags);

protected:
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;
    WIZDOCUMENTDATA m_data;

public Q_SLOTS:
    void viewDocumentImpl();
    void saveDocument(QString strKbGUID, QString strGUID,
                      QString strHtml, QString strHtmlFile, int nFlags);

Q_SIGNALS:
    void startLoading();
    void endLoading(const QString& strFileName, bool bOk);

    void startSaving();
    void documentSaved(bool ok);
};


class CWizDocumentWebViewPage: public QWebPage
{
public:
    explicit CWizDocumentWebViewPage(QObject* parent = 0) : QWebPage(parent) {}
    virtual void triggerAction(QWebPage::WebAction action, bool checked = false);
    virtual void javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID);
};


class CWizDocumentWebView : public QWebView
{
    Q_OBJECT

public:
    CWizDocumentWebView(CWizExplorerApp& app, QWidget* parent = 0);

    // view and save
    void viewDocument(const WIZDOCUMENTDATA& doc, bool editing);
    void setEditingDocument(bool editing);
    void setModified(bool bModified) { m_bModified = bModified; }
    void saveDocument(bool force);

    const WIZDOCUMENTDATA& document() { return m_renderer->data(); }
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

    QTimer m_timerAutoSave;
    QString m_strHtmlFileName;
    bool m_bEditorInited;
    bool m_bEditingMode;
    bool m_bModified;

    CWizDocumentWebViewRenderer* m_renderer;
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

    void onTimerAutoSaveTimout();

    void on_documentReady(const QString& strFileName, bool bOk);
    void on_documentSaved(bool ok);

    void on_editorCommandExecuteLinkInsert_accepted();
    void on_editorCommandExecuteTableInsert_accepted();

    void editorCommandExecuteCut();
    void editorCommandExecuteCopy();
    void editorCommandExecutePaste();

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
    bool editorCommandExecuteUndo();
    bool editorCommandExecuteRedo();
    bool editorCommandExecuteInsertDate();
    bool editorCommandExecuteInsertTime();
    bool editorCommandExecuteRemoveFormat();
    bool editorCommandExecuteFormatMatch();
    bool editorCommandExecuteInsertHorizontal();
    bool editorCommandExecuteViewSource();

Q_SIGNALS:
    // signals used request reset info toolbar and editor toolbar
    void focusIn();
    void focusOut();

    void requestView(const WIZDOCUMENTDATA& data);
};


#endif // WIZDOCUMENTWEBVIEW_H
