#include "WizCodeEditorDialog.h"
#include "wizdef.h"
#include "utils/pathresolve.h"
#include "share/wizsettings.h"
#include "wizDocumentWebView.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include <QWebPage>
#include <QWebFrame>
#include <QWebView>
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QFile>
#include <QDir>
#include <QPlainTextEdit>
#include <QEvent>
#include <QDebug>

#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/pluginspec.h>
#include <coreplugin/icore.h>

#define LASTUSEDCODETYPE "LASTUSEDCODETYPE"

WizCodeEditorDialog::WizCodeEditorDialog(CWizExplorerApp& app, CWizDocumentWebView* external, QWidget *parent) :
    QDialog(parent)
  , m_app(app)
  , m_external(external)
  , m_codeBrowser(new QWebView(this))
{

    setAttribute(Qt::WA_DeleteOnClose);
    //setWindowFlags(Qt::WindowStaysOnTopHint);          //could cause fullscreen problem on mac when mainwindow was fullscreen
    setWindowState(windowState() & ~Qt::WindowFullScreen);
    resize(650, 550);
    //
    connect(m_codeBrowser->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            SLOT(registerJSObject()));

    //
    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(6);
    verticalLayout->setContentsMargins(5, 5, 5, 5);

    verticalLayout->addWidget(m_codeBrowser);

    QString strFileName = Utils::PathResolve::resourcesPath() + "files/code/insert_code.htm";
    QString strHtml;
    ::WizLoadUnicodeTextFromFile(strFileName, strHtml);
    strHtml.replace("Wiz_Language_Replace", tr("Language"));
    strHtml.replace("Wiz_OK_Replace", tr("OK"));
    strHtml.replace("Wiz_Cancel_Replace", tr("Cancel"));
    QUrl url = QUrl::fromLocalFile(strFileName);

    m_codeBrowser->page()->mainFrame()->setHtml(strHtml, url);

}

void WizCodeEditorDialog::setCode(const QString& strCode)
{
//    if (!strCode.isEmpty())
//    {
//        //m_codeEditor->page()->mainFrame()->setHtml(strCode);
//        m_codeEditor->setPlainText(strCode);
//        renderCodeToHtml();
//    }
}

void WizCodeEditorDialog::registerJSObject()
{
    m_codeBrowser->page()->mainFrame()->addToJavaScriptWindowObject("codeEditor", this);
    m_codeBrowser->page()->mainFrame()->addToJavaScriptWindowObject("external", m_external);
}

void WizCodeEditorDialog::insertHtml(const QString& strResultDiv)
{
    QString strHtml = strResultDiv;
    strHtml.replace("\\", "\\\\");
    strHtml.replace("'", "\\'");
    insertHtmlRequest(strHtml);
}

void WizCodeEditorDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
    }
}

QString WizCodeEditorDialog::getLastCodeType()
{
    QString strLastType = m_app.userSettings().get((LASTUSEDCODETYPE));
    if (!strLastType.isEmpty())
    {
        return strLastType.toLower();
    }

    return "c";
}

void WizCodeEditorDialog::saveLastCodeType(const QString& codeType)
{
    m_app.userSettings().set(LASTUSEDCODETYPE, codeType);
}

/* if use webengine
void WizCodeEditorDialog::onHtmlLoaded(bool ok)
{
    if (!ok)
        return;

    QWebSocketServer *server = new QWebSocketServer(QStringLiteral("Wiz Socket Server"), QWebSocketServer::NonSecureMode, this);
    if (!server->listen(QHostAddress::LocalHost, 0)) {
        qFatal("Failed to open web socket server.");
        return;
    }

    // wrap WebSocket clients in QWebChannelAbstractTransport objects
    WebSocketClientWrapper *clientWrapper  = new WebSocketClientWrapper(server, this);

    // setup the dialog and publish it to the QWebChannel
    QWebChannel *webChannel = new QWebChannel(this);
    // setup the channel
    QObject::connect(clientWrapper, &WebSocketClientWrapper::clientConnected,
                     webChannel, &QWebChannel::connectTo);
    webChannel->registerObject(QStringLiteral("codeEditor"), m_external);

    QString strUrl = server->serverUrl().toString();
    qDebug() << "try to init js in code editor by url : " << strUrl;
    m_codeBrowser->page()->runJavaScript(QString("initializeJSObject('%1');").arg(strUrl));
}

void WizCodeEditorDialog::runJs()
{
    QString strHtml = m_edit->text();
    m_codeBrowser->page()->runJavaScript(strHtml);
}
*/


