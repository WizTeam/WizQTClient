#include <QWebEngineView>
#include <QWebSocketServer>
#include <QWebChannel>
#include "wizwebengineview.h"
#include "wizmisc.h"
#include "utils/pathresolve.h"

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
{
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(innerLoadFinished(bool)));
    //
    // setup the QWebSocketServer
    m_server = new QWebSocketServer(QStringLiteral("QWebChannel Standalone Example Server"), QWebSocketServer::NonSecureMode);
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
    m_channel->deregisterObject(obj);
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
    if (m_server)
    {
        m_server->close();
        m_server->deleteLater();
    }
    if (m_clientWrapper)
    {
        m_clientWrapper->deleteLater();
    }
    if (m_channel)
    {
        m_channel->deleteLater();
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
                    QString initFileName = Utils::PathResolve::resourcesPath() + "files/webengine/wizwebengineviewinit.js";
                    QString jsInit;
                    WizLoadUnicodeTextFromFile(initFileName, jsInit);
                    //
                    QString port = QString::asprintf("%d", int(m_server->serverPort()));
                    //
                    jsInit.replace("__port__", port).replace("__objectNames__", m_objectNames);
                    //
                    qDebug() << jsInit;
                    //
                    page()->runJavaScript(jsInit);
                });
            }
        }
    }
    //
    emit loadFinishedEx(ret);
}


