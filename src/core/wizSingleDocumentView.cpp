#include "wizSingleDocumentView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QDebug>
#include "wizDocumentView.h"
#include "wizDocumentWebView.h"
#include "wizmainwindow.h"
#include "share/wizmisc.h"
#include "titlebar.h"
#include "wizEditorToolBar.h"

using namespace Core;

CWizSingleDocumentViewer::CWizSingleDocumentViewer(CWizExplorerApp& app, const QString& guid, QWidget* parent) :
    QWidget(parent)
  , m_guid(guid)
{
        setAttribute(Qt::WA_DeleteOnClose);
        setContentsMargins(0, 0, 0, 0);
        QPalette pal = palette();
        pal.setColor(QPalette::Window, QColor("#DDDDDD"));
        setPalette(pal);
        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        setLayout(layout);
//        m_webEngine = new CWizDocumentWebEngine(app, this);
//        layout->addWidget(m_webEngine);
//        m_edit = new QLineEdit(this);
//        layout->addWidget(m_edit);
//        connect(m_edit, SIGNAL(returnPressed()), SLOT(on_textInputFinished()));
//        WIZDOCUMENTDATA doc;
//        m_webEngine->viewDocument(doc, true);

        QWidget* containerWgt = new QWidget(this);
        containerWgt->setStyleSheet(".QWidget{background-color:#F5F5F5;}");
//        pal = containerWgt->palette();
//        pal.setColor(QPalette::Window, QColor("#DDDDDD"));
//        containerWgt->setPalette(pal);
        containerWgt->setMaximumWidth(1095);

        layout->addStretch(0);
        layout->addWidget(containerWgt);
        layout->addStretch(0);

        QHBoxLayout* containerLayout = new QHBoxLayout(containerWgt);
        containerLayout->setContentsMargins(0, 0, 0, 0);
        containerWgt->setLayout(containerLayout);

        m_docView = new CWizDocumentView(app, containerWgt);
        m_docView->setStyleSheet(QString("QLineEdit{padding:0px; padding-left:-2px; padding-bottom:1px; border:0px;background-color:#F5F5F5;}"
                              "QToolButton {border:0px; padding:0px; border-radius:0px;background-color:#F5F5F5;}"));
        m_docView->web()->setInSeperateWindow(true);
//        m_docView->setMaximumWidth(1070);
        m_docView->setSizeHint(QSize(1095, 1));
        m_docView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_docView->titleBar()->applyButtonStateForSeparateWindow(true);

        containerLayout->addStretch(0);
        containerLayout->addWidget(m_docView);
        containerLayout->addStretch(0);
}

CWizDocumentView*CWizSingleDocumentViewer::docView()
{
    return m_docView;
}

void CWizSingleDocumentViewer::resizeEvent(QResizeEvent* ev)
{
    QWidget::resizeEvent(ev);

    m_docView->titleBar()->editorToolBar()->adjustButtonPosition();
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
