#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "websocketclientwrapper.h"
#include "websockettransport.h"
#include <QWebEngineView>
#include <QWebEnginePage>

class QWebChannel;

class WizWebEngineView;

class WizWebEnginePage: public QWebEnginePage
{
    Q_OBJECT
public:
    explicit WizWebEnginePage(QObject* parent = 0);
    //
    void stopCurrentNavigation() { m_continueNavigate = false; }
protected:
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID);
    virtual bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame);
Q_SIGNALS:
    void linkClicked(QUrl url, QWebEnginePage::NavigationType type, bool isMainFrame, WizWebEnginePage* page);
private:
    bool m_continueNavigate;
};


class WizWebEngineView : public QWebEngineView
{
    Q_OBJECT

public:
    WizWebEngineView(QWidget* parent);
    virtual ~WizWebEngineView();
public:
    void addToJavaScriptWindowObject(QString name, QObject* obj);
    void closeAll();
public Q_SLOTS:
    void innerLoadFinished(bool);
Q_SIGNALS:
    void loadFinishedEx(bool);
private:
    QWebSocketServer* m_server;
    WebSocketClientWrapper* m_clientWrapper;
    QWebChannel* m_channel;
    QString m_objectNames;
};

#endif // MAINWINDOW_H
