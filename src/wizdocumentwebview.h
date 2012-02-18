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
    bool m_bLocked;
    bool m_bViewDocumentWhileFinished;
    CString m_strHtmlFileName;
    WIZDOCUMENTDATA m_data;
protected:
    void init();
    bool viewDocumentInEditor();
public:
    bool saveDocument(bool force);
    bool viewDocument(const WIZDOCUMENTDATA& doc);
    bool newDocument();
    const WIZDOCUMENTDATA& document() { return m_data; }
public slots:
    void on_unlockBtnCliked();
    void on_web_populateJavaScriptWindowObject();
    void on_web_loadFinished(bool ok);
    void on_web_linkClicked(const QUrl & url);
};



#endif // WIZDOCUMENTWEBVIEW_H
