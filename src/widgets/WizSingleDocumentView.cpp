#include "WizSingleDocumentView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QPainter>
#include <QDebug>
#include "utils/WizStyleHelper.h"
#include "share/WizMisc.h"
#include "share/WizThreads.h"
#include "widgets/WizLocalProgressWebView.h"
#include "WizTitleBar.h"
#include "WizDocumentView.h"
#include "WizDocumentWebView.h"
#include "WizMainWindow.h"
#include "WizEditorToolBar.h"
#include "WizDocumentWebEngine.h"

#ifdef Q_OS_MAC
#include "mac/WizMacHelper.h"
#endif

QRegion creteRoundMask(const QRectF& rect)
{
    QPolygon polygon;
    polygon.append(QPoint(rect.x(), rect.y() + 1));
    polygon.append(QPoint(rect.x() + 1, rect.y()));
    polygon.append(QPoint(rect.right() - 1, rect.y()));
    polygon.append(QPoint(rect.right(), rect.y() + 1));

    polygon.append(QPoint(rect.right(), rect.bottom()));
    polygon.append(QPoint(rect.left(), rect.bottom()));

    return QRegion(polygon);
}

WizSingleDocumentViewer::WizSingleDocumentViewer(WizExplorerApp& app, const QString& guid, QWidget* parent) :
    QWidget(parent)
  , m_guid(guid)
  , m_docView(nullptr)
  , m_containerWgt(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setContentsMargins(0, 0, 0, 0);
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    applyWidgetBackground(false);

    m_containerWgt = new QWidget(this);
    if (isDarkMode()) {
        m_containerWgt->setStyleSheet(".QWidget{background-color:#666666;}");
    } else {
        m_containerWgt->setStyleSheet(".QWidget{background-color:#F5F5F5;}");
    }

    layout->addStretch(0);
    layout->addWidget(m_containerWgt);
    layout->addStretch(0);

    QHBoxLayout* containerLayout = new QHBoxLayout(m_containerWgt);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    m_containerWgt->setLayout(containerLayout);

    m_docView = new WizDocumentView(app, m_containerWgt);
    if (isDarkMode()) {
        m_docView->setStyleSheet(QString("QLineEdit{border:1px solid #DDDDDD; border-radius:2px;}"
                                         "QToolButton {border:0px; padding:0px; border-radius:0px;background-color:#666666;}"));
        m_docView->titleBar()->setStyleSheet(QString("QLineEdit{padding:0px; padding-left:-2px; padding-bottom:1px; border:0px;background-color:#000000;}"));

    } else {
        m_docView->setStyleSheet(QString("QLineEdit{border:1px solid #DDDDDD; border-radius:2px;}"
                                         "QToolButton {border:0px; padding:0px; border-radius:0px;background-color:#F5F5F5;}"));
        m_docView->titleBar()->setStyleSheet(QString("QLineEdit{padding:0px; padding-left:-2px; padding-bottom:1px; border:0px;background-color:#F5F5F5;}"));

    }
    //
    if (WizIsHighPixel())
    {
        m_docView->setMaximumWidth(QWIDGETSIZE_MAX);
        m_docView->setSizeHint(QSize(QWIDGETSIZE_MAX, 1));
    }
    else
    {
        m_docView->setMaximumWidth(1095);
        m_docView->setSizeHint(QSize(1095, 1));
    }
    m_docView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_docView->titleBar()->applyButtonStateForSeparateWindow(true);

    connect(m_docView->commentWidget(), SIGNAL(widgetStatusChanged()), SLOT(on_commentWidget_statusChanged()));
    connect(m_docView->commentWidget(), SIGNAL(willShow()), SLOT(on_commentWidget_willShow()));

    containerLayout->addStretch(0);
    containerLayout->addWidget(m_docView);
    containerLayout->addStretch(0);
}

WizSingleDocumentViewer::~WizSingleDocumentViewer()
{
    emit documentViewerDeleted(m_guid);
}
//


WizDocumentView* WizSingleDocumentViewer::docView()
{        
    return m_docView;
}

void WizSingleDocumentViewer::on_commentWidget_statusChanged()
{
    if ((windowState() & Qt::WindowFullScreen) && !m_docView->isVisible())
    {
        applyWidgetBackground(true);
    }
}

void WizSingleDocumentViewer::on_commentWidget_willShow()
{
    m_containerWgt->clearMask();
}

void WizSingleDocumentViewer::resizeEvent(QResizeEvent* ev)
{
    QWidget::resizeEvent(ev);

    m_docView->titleBar()->editorToolBar()->adjustButtonPosition();    
}

void WizSingleDocumentViewer::closeEvent(QCloseEvent *ev)
{
    m_docView->waitForDone();
    //
    QWidget::closeEvent(ev);
}

bool WizSingleDocumentViewer::event(QEvent* ev)
{
    if (ev->type() == QEvent::WindowStateChange)
    {
        if (QWindowStateChangeEvent* stateEvent = dynamic_cast<QWindowStateChangeEvent*>(ev))
        {
            static int state = -1;
            int oldState = stateEvent->oldState();
            if (state != oldState)
            {
                //quit full screen
                if ((oldState & Qt::WindowFullScreen) && !(windowState() & Qt::WindowFullScreen))
                {
                    applyWidgetBackground(false);
                }
                else if (!(oldState & Qt::WindowFullScreen) && (windowState() & Qt::WindowFullScreen))
                {
                    applyWidgetBackground(true);
                }
            }
        }
    }

    return QWidget::event(ev);
}

void WizSingleDocumentViewer::keyPressEvent(QKeyEvent* ev)
{
    if (WizWebEngineViewProgressKeyEvents(ev))
        return;
    //
    QWidget::keyPressEvent(ev);
}


void WizSingleDocumentViewer::applyWidgetBackground(bool isFullScreen)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Window, isFullScreen ? QColor("#3e3e3e") : QColor("#DDDDDD"));
    setPalette(pal);

    layout()->setContentsMargins(0, isFullScreen ? 20 : 0, 0, 0);

    if (m_containerWgt)
    {
        if (isFullScreen)
        {
            QRectF rect = m_containerWgt->rect();
            QRegion region =  creteRoundMask(rect);
            m_containerWgt->setMask(region);
        }
        else
        {
            m_containerWgt->clearMask();
        }
    }
}


WizSingleDocumentViewDelegate::WizSingleDocumentViewDelegate(WizExplorerApp& app, QObject* parent)
    : QObject(parent)
    ,m_app(app)
{
}

WizSingleDocumentViewer*WizSingleDocumentViewDelegate::getDocumentViewer(const QString& guid)
{
    return m_viewerMap.value(guid, nullptr);
}

QMap<QString, WizSingleDocumentViewer*>& WizSingleDocumentViewDelegate::getDocumentViewerMap()
{
    return m_viewerMap;
}

void WizSingleDocumentViewDelegate::viewDocument(const WIZDOCUMENTDATA& doc)
{
    if (m_viewerMap.find(doc.strGUID) != m_viewerMap.end())
    {
        bringWidgetToFront(m_viewerMap.value(doc.strGUID));
        return;
    }
    else
    {
        WizMainWindow* mainWindow = dynamic_cast<WizMainWindow*>(m_app.mainWindow());
        WizSingleDocumentViewer* wgt = new WizSingleDocumentViewer(m_app, doc.strGUID);
        WizDocumentView* docView = wgt->docView();
        connect(docView, SIGNAL(documentSaved(QString,WizDocumentView*)), SIGNAL(documentChanged(QString,WizDocumentView*)));
        connect(this, SIGNAL(documentChanged(QString,WizDocumentView*)), docView, SLOT(on_document_data_changed(QString,WizDocumentView*)));
        connect(docView->web(), SIGNAL(shareDocumentByLinkRequest(QString,QString)),
                mainWindow, SLOT(on_shareDocumentByLink_request(QString,QString)));
        connect(docView->web(), SIGNAL(statusChanged(const QString&)), mainWindow,
                SLOT(on_editor_statusChanged(const QString&)));
        connect(wgt, SIGNAL(documentViewerDeleted(QString)), SLOT(onDocumentViewerDeleted(QString)));
        static int nOffset = 0;
        wgt->setGeometry((mainWindow->width() - mainWindow->documentView()->width())  / 2 + nOffset,
                         (mainWindow->height() - wgt->height()) / 2 + nOffset,
                         mainWindow->documentView()->width(), wgt->height());
        wgt->setWindowTitle(doc.strTitle);
        wgt->show();
        nOffset += 22;
        nOffset > 250 ? nOffset = 0 : 0;
        //
        docView->viewNote(doc, false);
        bringWidgetToFront(wgt);
        m_viewerMap.insert(doc.strGUID, wgt);

        bindESCToQuitFullScreen(wgt);
    }
}

void WizSingleDocumentViewDelegate::onDocumentViewerDeleted(QString guid)
{
    m_viewerMap.remove(guid);

    emit documentViewerClosed(guid);
}


void bindESCToQuitFullScreen(QWidget* wgt)
{
    //ESC键退出全屏
#ifdef Q_OS_MAC
    QAction* action = new QAction(wgt);
    action->setShortcut(QKeySequence(Qt::Key_Escape));
    QObject::connect(action, &QAction::triggered, [wgt](){
        if (!wgt->isActiveWindow())
            return;
        //
        if (wgt->windowState() & Qt::WindowFullScreen)
        {
            wgt->setWindowState(wgt->windowState() & ~Qt::WindowFullScreen);
        }
    });
    wgt->addAction(action);
#endif
}


void bringWidgetToFront(QWidget* wgt)
{
    wgt->setVisible(true);
    if (wgt->windowState() & Qt::WindowMinimized)
    {
        wgt->setWindowState(wgt->windowState() & ~Qt::WindowMinimized);
    }
    wgt->raise();
    wgt->activateWindow();
}
