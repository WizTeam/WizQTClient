#include "wizSingleDocumentView.h"
#include <QVBoxLayout>
#include "wizDocumentView.h"
#include "wizDocumentWebView.h"
#include "wizmainwindow.h"
#include "share/wizmisc.h"

using namespace Core;

CWizSingleDocumentViewer::CWizSingleDocumentViewer(CWizExplorerApp& app, QWidget* parent) :
    QWidget(parent)
  , m_guid(WizGenGUIDLowerCaseLetterOnly())
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
}

CWizDocumentView*CWizSingleDocumentViewer::docView()
{
    return m_docView;
}

CWizSingleDocumentViewer::~CWizSingleDocumentViewer()
{
        m_docView->waitForDone();
}


CWizSingleDocumentViewDelegate::CWizSingleDocumentViewDelegate(CWizExplorerApp& app, QObject* parent)
    : m_app(app)
    , QObject(parent)
{
}

void CWizSingleDocumentViewDelegate::viewDocument(const WIZDOCUMENTDATA& doc)
{
    if (m_viewerMap.find(doc.strGUID) != m_viewerMap.end())
    {
        m_viewerMap.value(doc.strGUID)->raise();
        return;
    }
    else
    {
        Core::Internal::MainWindow* mainWindow = dynamic_cast<Core::Internal::MainWindow*>(m_app.mainWindow());
        CWizSingleDocumentViewer* wgt = new CWizSingleDocumentViewer(m_app, mainWindow);
        CWizDocumentView* docView = wgt->docView();
        connect(docView, SIGNAL(documentSaved(QString,CWizDocumentView*)), SIGNAL(documentChanged(QString,CWizDocumentView*)));
        connect(this, SIGNAL(documentChanged(QString,CWizDocumentView*)), docView, SLOT(on_document_data_changed(QString,CWizDocumentView*)));
        connect(docView->web(), SIGNAL(shareDocumentByLinkRequest(QString,QString)),
                mainWindow, SLOT(on_shareDocumentByLink_request(QString,QString)));

        wgt->setGeometry((mainWindow->width() - mainWindow->documentView()->width())  / 2,
                         (mainWindow->height() - wgt->height()) / 2,
                         mainWindow->documentView()->width(), wgt->height());
        wgt->setWindowTitle(doc.strTitle);
        wgt->show();
        //
        docView->viewNote(doc, false);
        docView->raise();
        m_viewerMap.insert(doc.strGUID, wgt);
    }
}
