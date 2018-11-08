#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "WebSocketClientWrapper.h"
#include "WebSocketTransport.h"
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QDialog>

class QWebChannel;

class WizWebEngineView;

struct WizWebEngineViewInjectObject
{
    QString name;
    QObject* object;
};

typedef std::vector<WizWebEngineViewInjectObject> WizWebEngineViewInjectObjects;

class WizWebEngineAsyncMethodResultObject: public QObject
{
    Q_OBJECT
public:
    WizWebEngineAsyncMethodResultObject(QObject* parent);
    virtual ~WizWebEngineAsyncMethodResultObject();
    Q_PROPERTY(QVariant result READ result NOTIFY resultAcquired)
public:
    void setResult(const QVariant& result);
private:
    QVariant m_result;
    QVariant result() const { return m_result; }
Q_SIGNALS:
    void resultAcquired(const QVariant& ret);
};

class WizWebEnginePage: public QWebEnginePage
{
    Q_OBJECT
public:
    explicit WizWebEnginePage(const WizWebEngineViewInjectObjects& objects, QObject* parent = 0);
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
    WizWebEngineView(const WizWebEngineViewInjectObjects& objects, QWidget* parent);
    virtual ~WizWebEngineView();
public:
    Q_INVOKABLE QVariant ExecuteScript(QString script);
    Q_INVOKABLE QVariant ExecuteScriptFile(QString fileName);
    Q_INVOKABLE QVariant ExecuteFunction0(QString function);
    Q_INVOKABLE QVariant ExecuteFunction1(QString function, const QVariant& arg1);
    Q_INVOKABLE QVariant ExecuteFunction2(QString function, const QVariant& arg1, const QVariant& arg2);
    Q_INVOKABLE QVariant ExecuteFunction3(QString function, const QVariant& arg1, const QVariant& arg2, const QVariant& arg3);
    Q_INVOKABLE QVariant ExecuteFunction4(QString function, const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, const QVariant& arg4);
public Q_SLOTS:
    void innerLoadFinished(bool);
    void openLinkInDefaultBrowser(QUrl url);
Q_SIGNALS:
    void loadFinishedEx(bool);
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
