#ifndef WIZDOCUMENTWEBVIEW_H
#define WIZDOCUMENTWEBVIEW_H

#include <QWebView>

#include "wizdef.h"


class CWizDocumentWebView : public QWebView
{
    Q_OBJECT

public:
    CWizDocumentWebView(CWizExplorerApp& app, QWidget* parent = 0);

private:
    CWizExplorerApp& m_app;
    bool m_bInited;
    bool m_bEditDocumentWhileFinished;
    CString m_strHtmlFileName;
    WIZDOCUMENTDATA m_data;

protected:
    void init();
    bool viewDocumentInEditor(bool editing);

    virtual void inputMethodEvent(QInputMethodEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);

public:
    bool saveDocument(bool force);
    bool viewDocument(const WIZDOCUMENTDATA& doc, bool editing);
    void setEditingDocument(bool editing);
    const WIZDOCUMENTDATA& document() { return m_data; }
    void reloadDocument();

    void updateSize();

public Q_SLOTS:
    void on_editor_populateJavaScriptWindowObject();
    void on_editor_loadFinished(bool ok);
    void on_editor_linkClicked(const QUrl& url);
};


#endif // WIZDOCUMENTWEBVIEW_H
