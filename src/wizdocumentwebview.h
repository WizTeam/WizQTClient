#ifndef WIZDOCUMENTWEBVIEW_H
#define WIZDOCUMENTWEBVIEW_H

#include <QWebView>
#include <QTimer>
#include <QPointer>

#include "wizdef.h"
#include "wizdownloadobjectdatadialog.h"
#include "wizusercipherform.h"

class CWizDocumentWebView : public QWebView
{
    Q_OBJECT

public:
    CWizDocumentWebView(CWizExplorerApp& app, QWidget* parent = 0);

    bool viewDocument(const WIZDOCUMENTDATA& doc, bool editing);
    bool viewDocumentImpl();

    void setEditingDocument(bool editing);
    bool saveDocument(bool force);
    const WIZDOCUMENTDATA& document() { return m_data; }
    void reloadDocument();

private:
    CWizExplorerApp& m_app;
    CWizDatabase& m_db;

    bool m_bEditorInited;
    bool m_bEditingMode;
    CString m_strHtmlFileName;

    WIZDOCUMENTDATA m_data;

    QTimer m_timerAutoSave;

    QPointer<CWizDownloadObjectDataDialog> m_downloadDialog;
    QPointer<CWizUserCipherForm> m_cipherDialog;

protected:
    void initEditorAndLoadDocument();
    bool viewDocumentInEditor(bool editing);

    void updateSize();

    virtual void inputMethodEvent(QInputMethodEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);

public Q_SLOTS:
    void on_editor_populateJavaScriptWindowObject();
    void on_editor_loadFinished(bool ok);
    void on_editor_linkClicked(const QUrl& url);

    void onSelectionChanged();
    void onTimerAutoSaveTimout();

    void onCipherDialogClosed();
    void onDownloadDialogClosed(int result);
};


#endif // WIZDOCUMENTWEBVIEW_H
