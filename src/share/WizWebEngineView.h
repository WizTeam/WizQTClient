#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "WebSocketClientWrapper.h"
#include "WebSocketTransport.h"
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineProfile>
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
    Q_PROPERTY(QVariant acquired READ acquired)
public:
    void setResult(const QVariant& result);
private:
    QVariant m_result;
    bool m_acquired;
    QVariant result() const { return m_result; }
    bool acquired() const { return m_acquired; }
Q_SIGNALS:
    void resultAcquired(const QVariant& ret);
};

class WizWebEnginePage: public QWebEnginePage
{
    Q_OBJECT
public:
    explicit WizWebEnginePage(QWebEngineProfile* profile, QObject* parent = nullptr);
    virtual ~WizWebEnginePage();
    //
    virtual void init(const WizWebEngineViewInjectObjects& objects);
    void stopCurrentNavigation() { m_continueNavigate = false; }
    static void processCopiedData();
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


QWebEngineProfile* createWebEngineProfile(const WizWebEngineViewInjectObjects& objects, QObject* parent);

class WizWebEngineView : public QWebEngineView
{
    Q_OBJECT

public:
    WizWebEngineView(QWidget* parent);
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
    //
protected:
    virtual void init(const WizWebEngineViewInjectObjects& objects);
public:


    //bugs fixed:
    //https://phabricator.kde.org/D19559?id=53357
    //
    template <class TWebEngineView = WizWebEngineView, class TWebEnginePage = WizWebEnginePage>
    static inline TWebEngineView* create(const WizWebEngineViewInjectObjects& objects, QWidget* parent)
    {
        TWebEngineView* webView = new TWebEngineView(parent);
        initWebEngineView<TWebEngineView, TWebEnginePage>(webView, objects);
        return webView;
    }
    //

    template <class TWebEngineView = WizWebEngineView, class TWebEnginePage = WizWebEnginePage>
    static inline void initWebEngineView(TWebEngineView* webView, const WizWebEngineViewInjectObjects& objects)
    {
        QWebEngineProfile* profile = createWebEngineProfile(objects, nullptr);
        TWebEnginePage* page = new TWebEnginePage(profile, webView);
        if (profile) {
            profile->setParent(page);
        }
        webView->setPage(page);
        if (auto view =  dynamic_cast<WizWebEngineView*>(webView)) {
            view->init(objects);
        }
    }

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
