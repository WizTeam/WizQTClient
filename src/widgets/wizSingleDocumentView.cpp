#include "wizSingleDocumentView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QPainter>
#include <QDebug>
#include "utils/stylehelper.h"
#include "share/wizmisc.h"
#include "widgets/wizLocalProgressWebView.h"
#include "titlebar.h"
#include "wizDocumentView.h"
#include "wizDocumentWebView.h"
#include "wizmainwindow.h"
#include "wizEditorToolBar.h"

#ifdef Q_OS_MAC
#include "mac/wizmachelper.h"
#endif

using namespace Core;

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

CWizSingleDocumentViewer::CWizSingleDocumentViewer(CWizExplorerApp& app, const QString& guid, QWidget* parent) :
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
    //        m_webEngine = new CWizDocumentWebEngine(app, this);
    //        layout->addWidget(m_webEngine);
    //        m_edit = new QLineEdit(this);
    //        layout->addWidget(m_edit);
    //        connect(m_edit, SIGNAL(returnPressed()), SLOT(on_textInputFinished()));
    //        WIZDOCUMENTDATA doc;
    //        m_webEngine->viewDocument(doc, true);

    m_containerWgt = new QWidget(this);
    m_containerWgt->setStyleSheet(".QWidget{background-color:#F5F5F5;}");

    layout->addStretch(0);
    layout->addWidget(m_containerWgt);
    layout->addStretch(0);

    QHBoxLayout* containerLayout = new QHBoxLayout(m_containerWgt);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    m_containerWgt->setLayout(containerLayout);

    m_docView = new CWizDocumentView(app, m_containerWgt);
    m_docView->setStyleSheet(QString("QLineEdit{border:1px solid #DDDDDD; border-radius:2px;}"
                                     "QToolButton {border:0px; padding:0px; border-radius:0px;background-color:#F5F5F5;}"));
    m_docView->titleBar()->setStyleSheet(QString("QLineEdit{padding:0px; padding-left:-2px; padding-bottom:1px; border:0px;background-color:#F5F5F5;}"));
    m_docView->web()->setInSeperateWindow(true);
    if (WizIsHighPixel())
    {
        m_docView->setMaximumWidth(1095);
        m_docView->setSizeHint(QSize(1095, 1));
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

//#ifdef Q_OS_MAC
//    if (systemWidgetBlurAvailable())
//    {
//        setAutoFillBackground(false);
//        enableWidgetBehindBlur(this);
//    }
//#endif
}

CWizDocumentView* CWizSingleDocumentViewer::docView()
{        
    return m_docView;
}

void CWizSingleDocumentViewer::on_commentWidget_statusChanged()
{
    if ((windowState() & Qt::WindowFullScreen) && !m_docView->isVisible())
    {
        applyWidgetBackground(true);
    }
}

void CWizSingleDocumentViewer::on_commentWidget_willShow()
{
    m_containerWgt->clearMask();
}

void CWizSingleDocumentViewer::resizeEvent(QResizeEvent* ev)
{
    QWidget::resizeEvent(ev);

    m_docView->titleBar()->editorToolBar()->adjustButtonPosition();    
}

bool CWizSingleDocumentViewer::event(QEvent* ev)
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

void CWizSingleDocumentViewer::applyWidgetBackground(bool isFullScreen)
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

CWizSingleDocumentViewer::~CWizSingleDocumentViewer()
{
    emit documentViewerDeleted(m_guid);

    m_docView->waitForDone();
}


CWizSingleDocumentViewDelegate::CWizSingleDocumentViewDelegate(CWizExplorerApp& app, QObject* parent)
    : QObject(parent)
    ,m_app(app)
{
}

CWizSingleDocumentViewer*CWizSingleDocumentViewDelegate::getDocumentViewer(const QString& guid)
{
    return m_viewerMap.value(guid, nullptr);
}

QMap<QString, CWizSingleDocumentViewer*>& CWizSingleDocumentViewDelegate::getDocumentViewerMap()
{
    return m_viewerMap;
}

void CWizSingleDocumentViewDelegate::viewDocument(const WIZDOCUMENTDATA& doc)
{
    if (m_viewerMap.find(doc.strGUID) != m_viewerMap.end())
    {
        bringWidgetToFront(m_viewerMap.value(doc.strGUID));
        return;
    }
    else
    {
        Core::Internal::MainWindow* mainWindow = dynamic_cast<Core::Internal::MainWindow*>(m_app.mainWindow());
        CWizSingleDocumentViewer* wgt = new CWizSingleDocumentViewer(m_app, doc.strGUID);
        CWizDocumentView* docView = wgt->docView();
        connect(docView, SIGNAL(documentSaved(QString,CWizDocumentView*)), SIGNAL(documentChanged(QString,CWizDocumentView*)));
        connect(this, SIGNAL(documentChanged(QString,CWizDocumentView*)), docView, SLOT(on_document_data_changed(QString,CWizDocumentView*)));
        connect(docView->web(), SIGNAL(shareDocumentByLinkRequest(QString,QString)),
                mainWindow, SLOT(on_shareDocumentByLink_request(QString,QString)));
        connect(docView->web(), SIGNAL(statusChanged()), mainWindow,
                SLOT(on_editor_statusChanged()));
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

void CWizSingleDocumentViewDelegate::onDocumentViewerDeleted(QString guid)
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
