#include "wizdocumentwebview.h"

#include <QtGui>
#include <QtWebKit>

#include "share/wizmisc.h"
#include "share/wizdownloadobjectdata.h"


CWizDocumentWebView::CWizDocumentWebView(CWizExplorerApp& app, QWidget* parent /*= 0*/)
    : QWebView(parent)
    , m_app(app)
    , m_bInited(false)
{
    setAcceptDrops(false);
    setAttribute(Qt::WA_InputMethodEnabled);
}

// This is bug of QWebView: can't receive input method commit event
void CWizDocumentWebView::inputMethodEvent(QInputMethodEvent* event)
{
    if (!event->commitString().isEmpty()) {
        QString strScript("setModified(true);");
        page()->mainFrame()->evaluateJavaScript(strScript);
    }

    update();
    QWebView::inputMethodEvent(event);
}

// This also maybe a bug inside QWebView, QWebView did not update on Mac
void CWizDocumentWebView::keyPressEvent(QKeyEvent* event)
{
    update();
    QWebView::keyPressEvent(event);
}

void CWizDocumentWebView::init()
{
    if (m_bInited)
        return;

    QString strFileName = WizGetResourcesPath() + "files/editor/index.html";
    QString strHtml;
    ::WizLoadUnicodeTextFromFile(strFileName, strHtml);
    QUrl url = QUrl::fromLocalFile(strFileName);

    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), \
            SLOT(on_editor_populateJavaScriptWindowObject()));

    connect(page()->mainFrame(), SIGNAL(loadFinished(bool)), \
            SLOT(on_editor_loadFinished(bool)));

    connect(this, SIGNAL(linkClicked(const QUrl&)), \
            SLOT(on_editor_linkClicked(const QUrl&)));

    setHtml(strHtml, url);

    m_bInited = true;
}

bool CWizDocumentWebView::saveDocument(bool force)
{
    QString strScript("saveDocument(%1);");
    strScript = strScript.arg(force ? "true" : "false");
    return page()->mainFrame()->evaluateJavaScript(strScript).toBool();
}

bool CWizDocumentWebView::viewDocument(const WIZDOCUMENTDATA& doc, bool editing)
{
    if (doc.nProtected) {
        QString strFileName = WizGetResourcesPath() + "transitions/commingsoon.html";
        QString strHtml;
        ::WizLoadUnicodeTextFromFile(strFileName, strHtml);
        QUrl url = QUrl::fromLocalFile(strFileName);
        setHtml(strHtml, url);

        m_bInited = false;
        return true;
    }

    if (m_bInited) {
        if (!saveDocument(false))
            return false;
    }

    if (!::WizPrepareDocument(m_app.database(), doc, m_app.mainWindow()))
        return false;

    QString strHtmlFileName;
    if (!m_app.database().DocumentToTempHtmlFile(doc, strHtmlFileName))
        return false;

    m_data = doc;
    m_strHtmlFileName = strHtmlFileName;

    if (m_bInited) {
        return viewDocumentInEditor(editing);
    } else {
        m_bEditDocumentWhileFinished = editing;
        init();
        return true;
    }
}

void CWizDocumentWebView::setEditingDocument(bool editing)
{
    CString strScript = CString("setEditing(%1);").arg(editing ? "true" : "false");
    page()->mainFrame()->evaluateJavaScript(strScript).toBool();

    if (editing) {
        setFocus();
    }

    updateSize();
}

void CWizDocumentWebView::reloadDocument()
{
    m_app.database().DocumentFromGUID(m_data.strGUID, m_data);
}

bool CWizDocumentWebView::viewDocumentInEditor(bool editing)
{
    Q_ASSERT(!m_data.strGUID.IsEmpty());

    QString strScript = ("viewDocument('%1', '%2', %3);");
    strScript = strScript.arg(m_data.strGUID,
                              m_strHtmlFileName,
                              editing ? "true" : "false");

    bool ret = page()->mainFrame()->evaluateJavaScript(strScript).toBool();

    if (editing) {
        setFocus();
    }

    updateSize();

    return ret;
}

void CWizDocumentWebView::on_editor_populateJavaScriptWindowObject()
{
    page()->mainFrame()->addToJavaScriptWindowObject("WizExplorerApp", m_app.object());
}

void CWizDocumentWebView::on_editor_loadFinished(bool ok)
{
    if (!ok) {
        TOLOG("Wow, loading editor failed!");
        return;
    }

    viewDocumentInEditor(m_bEditDocumentWhileFinished);
}

void CWizDocumentWebView::on_editor_linkClicked(const QUrl& url)
{
    QDesktopServices::openUrl(url);
}

void CWizDocumentWebView::updateSize()
{
    // show & hide ueditor toolbar cause UI issue, force update.
    QRect rc = geometry();
    setGeometry(rc.adjusted(0, 0, 0, 100));
}
