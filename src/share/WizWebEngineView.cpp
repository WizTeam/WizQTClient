#include <QWebEngineView>
#include <QWebSocketServer>
#include <QWebChannel>
#include "WizWebEngineView.h"
#include "WizMisc.h"
#include "utils/WizPathResolve.h"
#include <QKeyEvent>
#include <QApplication>
#include <QDesktopServices>
#include <QClipboard>
#ifdef Q_OS_MAC
#include "mac/WizMacHelper.h"
#include <QTimer>
#include <QMimeData>
#endif
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QTimer>

class WizInvisibleWebEngineView : public QWebEngineView
{
    class WizInvisibleWebEnginePage : public QWebEnginePage
    {
        WizWebEnginePage* m_ownerPage;
    public:
        explicit WizInvisibleWebEnginePage(WizWebEnginePage* ownerPage, QObject *parent = Q_NULLPTR)
            : QWebEnginePage(parent)
            , m_ownerPage(ownerPage)
        {

        }

        bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
        {
            emit m_ownerPage->openLinkInNewWindow(url);
            //
            parent()->deleteLater();
            //
            return false;
        }

    };

public:
    explicit WizInvisibleWebEngineView(WizWebEnginePage* ownerPage, QWidget* parent = Q_NULLPTR)
        : QWebEngineView(parent)
    {
        WizInvisibleWebEnginePage* page = new WizInvisibleWebEnginePage(ownerPage, this);
        setPage(page);
    }
    virtual ~WizInvisibleWebEngineView()
    {

    }

public:
    static QWebEnginePage* create(WizWebEnginePage* ownerPage)
    {
        WizInvisibleWebEngineView* web = new WizInvisibleWebEngineView(ownerPage, nullptr);
        //
        web->setVisible(false);
        //
        return web->page();
    }
};


QWebEngineProfile* createWebEngineProfile(const WizWebEngineViewInjectObjects& objects, QObject* parent)
{
    if (objects.empty())
        return nullptr;
    //
    QWebEngineProfile *profile = new QWebEngineProfile("WizNoteWebEngineProfile", parent);
    //
    QString jsWebChannelFileName = Utils::WizPathResolve::resourcesPath() + "files/webengine/wizwebchannel.js";
    QString jsWebChannel;
    WizLoadUnicodeTextFromFile(jsWebChannelFileName, jsWebChannel);
    //
    QString initFileName = Utils::WizPathResolve::resourcesPath() + "files/webengine/wizwebengineviewinit.js";
    QString jsInit;
    WizLoadUnicodeTextFromFile(initFileName, jsInit);
    //
    CWizStdStringArray names;
    for (auto inject : objects) {
        names.push_back("\"" + inject.name + "\"");
    }
    //
    CString objectNames;
    WizStringArrayToText(names, objectNames, ", ");
    //
    jsInit.replace("__objectNames__", objectNames);
    //
    QString jsAll = jsWebChannel + "\n" + jsInit;
    //
    {
        QWebEngineScript script;
        script.setSourceCode(jsAll);
        script.setName("qwebchannel.js");
        script.setWorldId(QWebEngineScript::MainWorld);
        script.setInjectionPoint(QWebEngineScript::DocumentCreation);
        script.setRunsOnSubFrames(true);
        profile->scripts()->insert(script);
    }
    //
    return profile;
}

WizWebEngineAsyncMethodResultObject::WizWebEngineAsyncMethodResultObject(QObject* parent)
    : QObject(parent)
    , m_acquired(false)
{
}

WizWebEngineAsyncMethodResultObject::~WizWebEngineAsyncMethodResultObject()
{
}

void WizWebEngineAsyncMethodResultObject::setResult(const QVariant& result)
{
    m_acquired = true;
    m_result = result;
    emit resultAcquired(m_result);
}

WizWebEnginePage::WizWebEnginePage(QWebEngineProfile* profile, QObject* parent)
    : QWebEnginePage(profile, parent)
    , m_continueNavigate(true)
{
}
WizWebEnginePage::~WizWebEnginePage() {
    disconnect();
}

void WizWebEnginePage::init(const WizWebEngineViewInjectObjects& objects)
{
    if (!objects.empty()) {

        QWebChannel* channel = new QWebChannel();
        for (auto inject : objects) {
            channel->registerObject(inject.name, inject.object);
        }
        //
        setWebChannel(channel);
    }
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
QWebEnginePage *WizWebEnginePage::createWindow(WebWindowType type)
{
    return WizInvisibleWebEngineView::create(this);
}

void WizWebEnginePage::triggerAction(WizWebEnginePage::WebAction action, bool checked /*= false*/)
{
    QWebEnginePage::triggerAction(action, checked);
    //
    if (action == Copy)
    {
#ifdef Q_OS_MAC
        //fix
        processCopiedData();
#endif
    }
}

void WizWebEnginePage::processCopiedData()
{
    //从webengine复制的文字，粘贴到mac的备忘录的时候，中文会乱码。
    //webengine复制到剪贴板的纯文字有bug，编码有问题。因此延迟等到webengine处理完成后再重新粘贴纯文本
    //避免这个错误
    //
    //
#ifdef Q_OS_MAC
    QTimer::singleShot(500, [=]{
        //
        QClipboard* clipboard = QApplication::clipboard();
        const QMimeData *mimeData = clipboard->mimeData();
        QMimeData* newData = new QMimeData();
        for (auto format : mimeData->formats()) {
            //
            if (format == "text/html") {
                //
                QByteArray htmlData = mimeData->data(format);
                QString html = QString::fromUtf8(htmlData);
                html = "<meta content=\"text/html; charset=utf-8\" http-equiv=\"Content-Type\">" + html;
                newData->setHtml(html);
                //
            } else {
                newData->setData(format, mimeData->data(format));
            }
        }
        //
        clipboard->setMimeData(newData);
    });
#endif
}


WizWebEngineView::WizWebEngineView(QWidget* parent)
    : QWebEngineView(parent)
{
}

void WizWebEngineView::init(const WizWebEngineViewInjectObjects& objects)
{
    connect(page(), SIGNAL(openLinkInNewWindow(QUrl)), this, SLOT(openLinkInDefaultBrowser(QUrl)));
    connect(page(), SIGNAL(loadFinished(bool)), this, SLOT(innerLoadFinished(bool)));
    //
    if (WizWebEnginePage* p = dynamic_cast<WizWebEnginePage *>(page())) {
        p->init(objects);
    }
}


WizWebEngineView::~WizWebEngineView()
{
    disconnect();
}

QVariant WizWebEngineView::ExecuteScript(QString script)
{
    auto result = QSharedPointer<WizWebEngineAsyncMethodResultObject>(new WizWebEngineAsyncMethodResultObject(nullptr), &QObject::deleteLater);
    //
    page()->runJavaScript(script, [=](const QVariant &v) {
        result->setResult(v);
        QTimer::singleShot(1000, [=]{
            auto r = result;
            r = nullptr;
        });
    });

    QVariant v;
    v.setValue<QObject*>(result.data());
    return v;
}

QVariant WizWebEngineView::ExecuteScriptFile(QString fileName)
{
    QString script;
    if (!WizLoadUnicodeTextFromFile(fileName, script)) {
        return QVariant();
    }
    return ExecuteScript(script);
}

QVariant WizWebEngineView::ExecuteFunction0(QString function)
{
    QString script = QString("%1();").arg(function);
    return ExecuteScript(script);
}


QString toArgument(const QVariant& v)
{
    switch (v.type()) {
    case QVariant::Bool:
        return v.toBool() ? "true" : "false";
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
        return QString("%1").arg(v.toLongLong());
    case QVariant::Double: {
        double f = v.toDouble();
        QString str;
        str.sprintf("%f", f);
        return str;
    }
    case QVariant::Date:
    case QVariant::Time:
    case QVariant::DateTime:
        return QString("new Date(%1)").arg(v.toDateTime().toTime_t() * 1000);
    case QVariant::String: {
            QString s = v.toString();
            s.replace("\\", "\\\\");
            s.replace("\r", "\\r");
            s.replace("\n", "\\n");
            s.replace("\t", "\\t");
            s.replace("\"", "\\\"");
            return "\"" + s + "\"";
        }
    default:
        qDebug() << "Unsupport type: " << v.type();
        return "undefined";
    }
}

QVariant WizWebEngineView::ExecuteFunction1(QString function, const QVariant& arg1)
{
    QString script = QString("%1(%2);")
            .arg(function)
            .arg(toArgument(arg1))
            ;
    return ExecuteScript(script);
}

QVariant WizWebEngineView::ExecuteFunction2(QString function, const QVariant& arg1, const QVariant& arg2)
{
    QString script = QString("%1(%2, %3);")
            .arg(function)
            .arg(toArgument(arg1))
            .arg(toArgument(arg2))
            ;
    return ExecuteScript(script);
}

QVariant WizWebEngineView::ExecuteFunction3(QString function, const QVariant& arg1, const QVariant& arg2, const QVariant& arg3)
{
    QString script = QString("%1(%2, %3, %4);")
            .arg(function)
            .arg(toArgument(arg1))
            .arg(toArgument(arg2))
            .arg(toArgument(arg3))
            ;
    return ExecuteScript(script);
}

QVariant WizWebEngineView::ExecuteFunction4(QString function, const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, const QVariant& arg4)
{
    QString script = QString("%1(%2, %3, %4, %5);")
            .arg(function)
            .arg(toArgument(arg1))
            .arg(toArgument(arg2))
            .arg(toArgument(arg3))
            .arg(toArgument(arg4))
            ;
    return ExecuteScript(script);
}


void WizWebEngineView::innerLoadFinished(bool ret)
{
    //
    if (ret)
    {
        // 页面加载时设置合适的缩放比例
        qreal zFactor = (1.0 * WizSmartScaleUI(100)) / 100;
        setZoomFactor(zFactor);
        //
        //
        emit loadFinishedEx(ret);
    }
    else
    {
        emit loadFinishedEx(ret);
    }
}

void WizWebEngineView::openLinkInDefaultBrowser(QUrl url)
{
    QDesktopServices::openUrl(url);
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
    qDebug() << ev->key() << ", " << ev->text();
    //
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
            else if (ev->modifiers()&Qt::KeyboardModifier::ControlModifier && ev->key() == Qt::Key_Up)
            {
                //放大
                qreal factor = web->zoomFactor();
                factor += 0.1;
                factor = (factor > 5.0) ? 5.0 : factor;
                web->setZoomFactor(factor);
                return true;
            }
            else if (ev->modifiers()&Qt::KeyboardModifier::ControlModifier && ev->key() == Qt::Key_Down)
            {
                //缩小
                qreal factor = web->zoomFactor();
                factor -= 0.1;
                factor = (factor < 0.5) ? 0.5 : factor;
                web->setZoomFactor(factor);
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

void WizWebEngineView::wheelEvent(QWheelEvent *event)
{
    qreal factor = 0;

    if (event->modifiers()==Qt::ControlModifier) {
        factor = zoomFactor();
        if (event->delta() > 0) {
            //放大
            factor += 0.1;
            factor = (factor > 5.0)?5.0:factor;
        } else {
            //缩小
            factor -= 0.1;
            factor = (factor < 0.5)?0.5:factor;
        }
        //setZoomFactor(factor);
    } else {
        event->ignore();
    }
}
