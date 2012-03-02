#include "wizdocumentwebview.h"

#include "share/wizmisc.h"
#include "share/wizdownloadobjectdata.h"

#include <QWebElement>
#include <QWebFrame>
#include <QApplication>
#include <QDesktopServices>

CWizDocumentWebView::CWizDocumentWebView(CWizExplorerApp& app, QWidget* parent /*= 0*/)
    : QWebView(parent)
    , m_app(app)
    , m_bInited(false)
    , m_bViewDocumentWhileFinished(false)
{
}

void CWizDocumentWebView::init()
{
    if (m_bInited)
        return;
    //
    CString strFileName = WizGetResourcesPath() + "files/editor/index.html";
    CString strHtml;
    ::WizLoadUnicodeTextFromFile(strFileName, strHtml);
    QUrl url = QUrl::fromLocalFile(strFileName);
    //
    connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(on_web_populateJavaScriptWindowObject()));
    connect(page()->mainFrame(), SIGNAL(loadFinished(bool)), this, SLOT(on_web_loadFinished(bool)));
    //
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(this, SIGNAL(linkClicked(const QUrl&)), this, SLOT(on_web_linkClicked(const QUrl&)));


    setHtml(strHtml, url);
    //
    m_bInited = true;
}

bool CWizDocumentWebView::saveDocument(bool force)
{
    CString strScript("saveDocument(%1)");
    strScript = strScript.arg(force ? "true" : "false");
    return page()->mainFrame()->evaluateJavaScript(strScript).toBool();
}

bool CWizDocumentWebView::viewDocument(const WIZDOCUMENTDATA& doc, bool editing)
{
    if (m_bInited)
    {
        if (!saveDocument(false))
            return false;
    }
    //
    if (!::WizPrepareDocument(m_app.database(), doc, m_app.mainWindow()))
        return false;
    //
    CString strHtmlFileName;
    if (!m_app.database().DocumentToTempHtmlFile(doc, strHtmlFileName))
        return false;
    //
    m_data = doc;
    m_strHtmlFileName = strHtmlFileName;
    //
    if (m_bInited)
    {
        return viewDocumentInEditor(editing);
    }
    else
    {
        m_bViewDocumentWhileFinished = true;
        m_bEditDocumentWhileFinished = editing;
        //
        init();
        return true;
    }
}
bool CWizDocumentWebView::newDocument()
{
    return viewDocument(WIZDOCUMENTDATA(), true);
}

void CWizDocumentWebView::setEditingDocument(bool editing)
{
    CString strScript = CString("setEditing(%1)").arg(editing ? "true" : "false");
    page()->mainFrame()->evaluateJavaScript(strScript).toBool();
    //
    if (editing)
    {
        setFocus();
    }
}

void CWizDocumentWebView::reloadDocument()
{
    m_app.database().DocumentFromGUID(m_data.strGUID, m_data);
}

bool CWizDocumentWebView::viewDocumentInEditor(bool editing)
{
    ATLASSERT(!m_data.strGUID.IsEmpty());
    //
    CString strScript = ("viewDocument('%1', '%2', %3)");
    strScript = strScript.arg(m_data.strGUID,
                              m_strHtmlFileName,
                              editing ? "true" : "false");
    //
    bool ret = page()->mainFrame()->evaluateJavaScript(strScript).toBool();
    //
    if (editing)
    {
        setFocus();
    }
    //
    return ret;
}


void CWizDocumentWebView::updateSize()
{
    ////force to re-align controls////
    QRect rc = geometry();
    setGeometry(rc.adjusted(0, 0, 0, 100));
    qApp->processEvents(QEventLoop::AllEvents);
    setGeometry(rc);
    qApp->processEvents(QEventLoop::AllEvents);
}


void CWizDocumentWebView::on_web_populateJavaScriptWindowObject()
{
    page()->mainFrame()->addToJavaScriptWindowObject("WizExplorerApp", m_app.object());
}

void CWizDocumentWebView::on_web_loadFinished(bool ok)
{
    if (ok)
    {
        if (!m_bViewDocumentWhileFinished)
            return;
        //
        viewDocumentInEditor(m_bEditDocumentWhileFinished);
        //
        updateSize();
    }
}
void CWizDocumentWebView::on_web_linkClicked(const QUrl & url)
{
    Qt::KeyboardModifiers mod = QApplication::keyboardModifiers ();
    if (mod.testFlag(Qt::ControlModifier))
    {
        QDesktopServices::openUrl(url);
    }
}
