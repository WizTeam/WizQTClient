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
    bool m_bViewDocumentWhileFinished;
    bool m_bEditDocumentWhileFinished;
    CString m_strHtmlFileName;
    WIZDOCUMENTDATA m_data;

protected:
    void init();
    bool viewDocumentInEditor(bool editing);

    virtual void inputMethodEvent(QInputMethodEvent* event);

public:
    bool saveDocument(bool force);
    bool viewDocument(const WIZDOCUMENTDATA& doc, bool editing);
    bool newDocument();
    void setEditingDocument(bool editing);
    const WIZDOCUMENTDATA& document() { return m_data; }
    void reloadDocument();
    CWizExplorerApp& app() { return m_app; }
    void updateSize();

public Q_SLOTS:
    void on_web_populateJavaScriptWindowObject();
    void on_web_loadFinished(bool ok);
    void on_web_linkClicked(const QUrl& url);
};


#endif // WIZDOCUMENTWEBVIEW_H
