#ifndef WIZDOCUMENTWEBVIEW_H
#define WIZDOCUMENTWEBVIEW_H

#include <QWebView>
#include <QTimer>
#include <QPointer>
#include <QMutex>

#include "wizdef.h"
#include "wizdownloadobjectdatadialog.h"
#include "wizusercipherform.h"

class CWizDocumentWebView;

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
    void documentReady(const QString& strFileName);
    void documentSaved(bool ok);
};


class CWizDocumentWebView : public QWebView
{
    Q_OBJECT

public:
    CWizDocumentWebView(CWizExplorerApp& app, QWidget* parent = 0);

    void viewDocument(const WIZDOCUMENTDATA& doc, bool editing);
    void setEditingDocument(bool editing);
    void setModified(bool bModified) { m_bModified = bModified; }
    void saveDocument(bool force);

    void updateSize();

    const WIZDOCUMENTDATA& document() { return m_renderer->data(); }
    void reloadDocument();

private:
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;
    QTimer m_timerAutoSave;
    QString m_strHtmlFileName;
    bool m_bEditorInited;
    bool m_bEditingMode;

    bool m_bModified;

    QPointer<CWizDocumentWebViewRenderer> m_renderer;
    QPointer<CWizDownloadObjectDataDialog> m_downloadDialog;
    QPointer<CWizUserCipherForm> m_cipherDialog;

    virtual void inputMethodEvent(QInputMethodEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void focusOutEvent(QFocusEvent *event);

    void viewDocumentInEditor(bool editing);
    void initEditorAndLoadDocument();

public Q_SLOTS:
    void onSelectionChanged();
    void onCipherDialogClosed();
    void onDownloadDialogClosed(int result);

    void on_editor_populateJavaScriptWindowObject();
    void on_editor_loadFinished(bool ok);
    void on_editor_linkClicked(const QUrl& url);

    void onTimerAutoSaveTimout();

    void on_documentReady(const QString& strFileName);
    void on_documentSaved(bool ok);
};


#endif // WIZDOCUMENTWEBVIEW_H
