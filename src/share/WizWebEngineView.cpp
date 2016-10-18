#include <QWebEngineView>
#include <QWebSocketServer>
#include <QWebChannel>
#include "WizWebEngineView.h"
#include "WizMisc.h"
#include "utils/WizPathResolve.h"
#include <QKeyEvent>
#include <QApplication>

WizWebEnginePage::WizWebEnginePage(QObject* parent)
    : QWebEnginePage(parent)
    , m_continueNavigate(true)
{
}

void WizWebEnginePage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID)
{
    qDebug() << message;
}

bool WizWebEnginePage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    if (NavigationTypeLinkClicked != type)
        return true;
    //
    m_continueNavigate = true;
    emit linkClicked(url, type, isMainFrame, this);

    return m_continueNavigate;
}


WizWebEngineView::WizWebEngineView(QWidget* parent)
    : QWebEngineView(parent)
    , m_server(NULL)
    , m_clientWrapper(NULL)
    , m_channel(NULL)
{
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(innerLoadFinished(bool)));
    //
    // setup the QWebSocketServer
    m_server = new QWebSocketServer(QStringLiteral("WizNote QWebChannel Server"), QWebSocketServer::NonSecureMode);
    //
    if (!m_server->listen(QHostAddress::LocalHost)) {
        qFatal("Failed to open web socket server.");
        return;
    }

    // wrap WebSocket clients in QWebChannelAbstractTransport objects
    m_clientWrapper = new WebSocketClientWrapper(m_server);
    //
    // setup the channel
    m_channel = new QWebChannel();
    QObject::connect(m_clientWrapper, &WebSocketClientWrapper::clientConnected, m_channel, &QWebChannel::connectTo);
}

void WizWebEngineView::addToJavaScriptWindowObject(QString name, QObject* obj)
{
    m_channel->registerObject(name, obj);
    //
    if (m_objectNames.isEmpty())
    {
        m_objectNames = QString("\"%1\"").arg(name);
    }
    else
    {
        m_objectNames = m_objectNames + ", " + QString("\"%1\"").arg(name);
    }
}

WizWebEngineView::~WizWebEngineView()
{
    closeAll();
}

void WizWebEngineView::closeAll()
{
    if (m_server)
    {
        m_server->disconnect();
        m_server->close();
        //m_server->deleteLater();
        //m_server = NULL;
    }
    if (m_clientWrapper)
    {
        m_clientWrapper->disconnect();
        //m_clientWrapper->deleteLater();
        //m_clientWrapper = NULL;
    }
    if (m_channel)
    {
        m_channel->disconnect();
        //m_channel->deleteLater();
        //m_channel = NULL;
    }
}


void WizWebEngineView::innerLoadFinished(bool ret)
{
    //
    if (ret)
    {
        if (m_server && m_server->isListening()
                && m_clientWrapper
                && m_channel)
        {
            QString jsWebChannel ;
            if (WizLoadTextFromResource(":/qtwebchannel/qwebchannel.js", jsWebChannel))
            {
                page()->runJavaScript(jsWebChannel, [=](const QVariant&){
                    //
                    QString initFileName = Utils::WizPathResolve::resourcesPath() + "files/webengine/wizwebengineviewinit.js";
                    QString jsInit;
                    WizLoadUnicodeTextFromFile(initFileName, jsInit);
                    //
                    QString port = QString::asprintf("%d", int(m_server->serverPort()));
                    //
                    jsInit.replace("__port__", port).replace("__objectNames__", m_objectNames);
                    //
                    page()->runJavaScript(jsInit, [=](const QVariant&){
                        //
                        emit loadFinishedEx(ret);
                        //
                    });
                });
            }
            else
            {
                qDebug() << "Can't load wen channel.js";
                emit loadFinishedEx(ret);
            }
        }
    }
    else
    {
        emit loadFinishedEx(ret);
    }
}



static QWebEngineView* getActiveWeb()
{
    QWidget* focusWidget = qApp->focusWidget();
    if (!focusWidget)
        return nullptr;
    //
    while (focusWidget) {
        QWebEngineView* web =  dynamic_cast<QWebEngineView *>(focusWidget);
        if (web)
            return web;
        //
        focusWidget = focusWidget->parentWidget();
    }
    return nullptr;
}

bool WizWebEngineViewProgressKeyEvents(QKeyEvent* ev)
{
    if (ev->modifiers() && ev->key()) {
        if (QWebEngineView* web = getActiveWeb()) {
            if (ev->matches(QKeySequence::Copy))
            {
                web->page()->triggerAction(QWebEnginePage::Copy);
                return true;
            }
            else if (ev->matches(QKeySequence::Cut))
            {
                web->page()->triggerAction(QWebEnginePage::Cut);
                return true;
            }
            else if (ev->matches(QKeySequence::Paste))
            {
                web->page()->triggerAction(QWebEnginePage::Paste);
                return true;
            }
            else if (ev->matches(QKeySequence::Undo))
            {
                web->page()->triggerAction(QWebEnginePage::Undo);
                return true;
            }
            else if (ev->matches(QKeySequence::Redo))
            {
                web->page()->triggerAction(QWebEnginePage::Redo);
                return true;
            }
            else if (ev->matches(QKeySequence::SelectAll))
            {
                web->page()->triggerAction(QWebEnginePage::SelectAll);
                return true;
            }
        }
    }
    return false;
}

WizWebEngineViewContainerDialog::WizWebEngineViewContainerDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{

}

void WizWebEngineViewContainerDialog::keyPressEvent(QKeyEvent* ev)
{
    if (WizWebEngineViewProgressKeyEvents(ev))
        return;
    //
    QDialog::keyPressEvent(ev);
}

