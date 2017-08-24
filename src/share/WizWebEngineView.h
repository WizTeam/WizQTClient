#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "WebSocketClientWrapper.h"
#include "WebSocketTransport.h"
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QDialog>

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
    virtual QWebEnginePage *createWindow(WebWindowType type);
    virtual void triggerAction(WebAction action, bool checked = false);
Q_SIGNALS:
    void linkClicked(QUrl url, QWebEnginePage::NavigationType type, bool isMainFrame, WizWebEnginePage* page);
    void openLinkInNewWindow(QUrl url);
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
    void openLinkInDefaultBrowser(QUrl url);
Q_SIGNALS:
    void loadFinishedEx(bool);
private:
    QWebSocketServer* m_server;
    WebSocketClientWrapper* m_clientWrapper;
    QWebChannel* m_channel;
    QString m_objectNames;
protected:
    void wheelEvent(QWheelEvent *event);
};

bool WizWebEngineViewProgressKeyEvents(QKeyEvent* ev);

class WizWebEngineViewContainerDialog: public QDialog
{
public:
    WizWebEngineViewContainerDialog(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
protected:
    virtual void keyPressEvent(QKeyEvent* ev);
};

#endif // MAINWINDOW_H
