#include "WizCodeEditorDialog.h"
#include "wizdef.h"
#include "utils/pathresolve.h"
#include "share/wizsettings.h"
#include "wizDocumentWebView.h"
#include "share/websocketclientwrapper.h"
#include "share/websockettransport.h"
#include "wizWebEngineInjectObject.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QFile>
#include <QDir>
#include <QPlainTextEdit>
#include <QEvent>
#include <QWebSocketServer>
#include <QWebChannel>
#include <QDebug>

#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/pluginspec.h>
#include <coreplugin/icore.h>


#define LASTUSEDCODETYPE "LASTUSEDCODETYPE"

WizCodeEditorDialog::WizCodeEditorDialog(CWizExplorerApp& app, QObject* external, QWidget *parent) :
    QDialog(parent)
  , m_app(app)
  , m_external(new CWizCodeExternal(this, this))
  , m_codeBrowser(new QWebEngineView(this))
{

    setAttribute(Qt::WA_DeleteOnClose);
    //setWindowFlags(Qt::WindowStaysOnTopHint);          //could cause fullscreen problem on mac when mainwindow was fullscreen
    setWindowState(windowState() & ~Qt::WindowFullScreen);
    resize(650, 550);
    //
    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(6);
    verticalLayout->setContentsMargins(5, 5, 5, 5);

    WebEnginePage* page = new WebEnginePage(m_codeBrowser);
    m_codeBrowser->setPage(page);
    verticalLayout->addWidget(m_codeBrowser);

    m_edit = new QLineEdit(this);
    verticalLayout->addWidget(m_edit);
    connect(m_edit, SIGNAL(returnPressed()), SLOT(runJs()));

    QString strFileName = Utils::PathResolve::resourcesPath() + "files/code/insert_code.htm";
    QString strHtml;
    ::WizLoadUnicodeTextFromFile(strFileName, strHtml);
    QUrl url = QUrl::fromLocalFile(strFileName);

    connect(m_codeBrowser->page(), SIGNAL(loadFinished(bool)), SLOT(onHtmlLoaded(bool)));
    m_codeBrowser->page()->setHtml(strHtml, url);

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

void WizCodeEditorDialog::insertHtml(const QString& strResultDiv)
{
    QString strHtml = strResultDiv;
//    QString strCss = Utils::PathResolve::resourcesPath() + "files/wiz_code_highlight.css";
//    QString strHtml = QString("<html></html><head><link rel='stylesheet' type='text/css' "
//                              "href='%1'> </head><body>%2</body>").arg(strCss).arg(strResultDiv);
    qDebug() << "insert html before : " << strHtml;
//    page.mainFrame()->setHtml(strHtml);
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

