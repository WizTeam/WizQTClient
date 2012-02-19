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
    , m_bLocked(true)
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

bool CWizDocumentWebView::viewDocument(const WIZDOCUMENTDATA& doc)
{
    if (m_bInited)
    {
        if (!saveDocument(false))
            return false;
    }

    if (!m_app.database().IsDocumentDownloaded(doc.strGUID)
        || !PathFileExists(m_app.database().GetDocumentFileName(doc.strGUID)))
    {

        WIZOBJECTDATA data;
        data.strObjectGUID = doc.strGUID;
        data.strDisplayName = doc.strTitle;
        data.eObjectType = wizobjectDocument;
        if (!WizDownloadObjectData(m_app.database(), data, m_app.mainWindow()))
            return false;
    }

    CString strHtmlFileName;
    if (!m_app.database().DocumentToTempHtmlFile(doc, strHtmlFileName))
        return false;
    //
    m_data = doc;
    m_strHtmlFileName = strHtmlFileName;
    //
    if (m_bInited)
    {
        m_bLocked = true;
        return viewDocumentInEditor();
    }
    else
    {
       m_bViewDocumentWhileFinished = true;
       init();
       return true;
    }
}
bool CWizDocumentWebView::newDocument()
{
    return viewDocument(WIZDOCUMENTDATA());
}

bool CWizDocumentWebView::viewDocumentInEditor()
{
    CString strScript;
    if (m_data.strGUID.IsEmpty())
    {
        strScript = ("newDocument()");
    }
    else
    {
        strScript = ("viewDocument('%1', '%2')");
        strScript = strScript.arg(m_data.strGUID, m_strHtmlFileName);
    }
    return page()->mainFrame()->evaluateJavaScript(strScript).toBool();
}

void CWizDocumentWebView::on_unlockBtnCliked()
{
    if (m_bLocked) {
        // show toolbar, remove content lock
        CString strScript("unlockDocument()");
        page()->mainFrame()->evaluateJavaScript(strScript);
        m_bLocked = false;
    } else {
        CString strScript("lockDocument()");
        page()->mainFrame()->evaluateJavaScript(strScript);
        m_bLocked = true;
    }
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
        viewDocumentInEditor();
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
