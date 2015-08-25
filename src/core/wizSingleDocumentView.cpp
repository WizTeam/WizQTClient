#include "wizSingleDocumentView.h"
#include <QVBoxLayout>
#include <QAction>
#include <QDebug>
#include "wizDocumentView.h"
#include "wizDocumentWebView.h"
#include "wizmainwindow.h"
#include "share/wizmisc.h"

using namespace Core;

CWizSingleDocumentViewer::CWizSingleDocumentViewer(CWizExplorerApp& app, const QString& guid, QWidget* parent) :
    QWidget(parent)
  , m_guid(guid)
{
        setAttribute(Qt::WA_DeleteOnClose);
        setContentsMargins(0, 0, 0, 0);
        setPalette(QPalette(Qt::white));
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
//        m_webEngine = new CWizDocumentWebEngine(app, this);
//        layout->addWidget(m_webEngine);
//        m_edit = new QLineEdit(this);
//        layout->addWidget(m_edit);
//        connect(m_edit, SIGNAL(returnPressed()), SLOT(on_textInputFinished()));
//        WIZDOCUMENTDATA doc;
//        m_webEngine->viewDocument(doc, true);
        m_docView = new CWizDocumentView(app, this);
        layout->addWidget(m_docView);
        setLayout(layout);

#ifdef Q_OS_MAC
        //追踪单独窗口中编辑器消失的问题
   connect(m_docView, &QWidget::destroyed, [](){
      qDebug() << "doc view destroyed";
   });
#endif
}

CWizDocumentView*CWizSingleDocumentViewer::docView()
{
    return m_docView;
}

CWizSingleDocumentViewer::~CWizSingleDocumentViewer()
{
    emit documentViewerDeleted(m_guid);

    m_docView->waitForDone();
}


CWizSingleDocumentViewDelegate::CWizSingleDocumentViewDelegate(CWizExplorerApp& app, QObject* parent)
    : m_app(app)
    , QObject(parent)
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
