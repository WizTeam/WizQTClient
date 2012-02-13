#include "mainwindow.h"

#include "wizcategoryview.h"
#include "wizdocumentlistview.h"
#include "wizdocumentview.h"
#include "wizactions.h"


#include <QDir>
#include <QMessageBox>
#include <QSplitter>
#include <QLabel>
#include <QStatusBar>
#include <QProgressBar>
#include <QListView>
#include <QHBoxLayout>
#ifndef Q_OS_MAC
#include <QToolBar>
#endif
#include <QMenuBar>
#include <QAction>
#include <QDebug>
#include <QBoxLayout>

#include "mac/wizmactoolbar.h"

#include "wiznotestyle.h"
#include "wizdocumenthistory.h"

#include "share/wizuihelper.h"
#include "share/wizsettings.h"

MainWindow::MainWindow(CWizDatabase& db, QWidget *parent) :
    QMainWindow(parent),
    m_db(db),
    m_sync(m_db, WIZ_API_URL, *this),
    m_menuBar(new QMenuBar(this)),
    #ifndef Q_OS_MAC
    m_toolBar(new QToolBar("Main", this)),
    #endif
    m_statusBar(new QStatusBar(this)),
    m_labelStatus(new QLabel(this)),
    m_progressSync(new QProgressBar(this)),
    m_actions(new CWizActions(this)),
    m_category(new CWizCategoryView(*this, this)),
    m_documents(new CWizDocumentListView(*this, this)),
    m_doc(new CWizDocumentView(*this, this)),
    m_history(new CWizDocumentViewHistory()),
    m_bRestart(false),
    m_bUpdatingSelection(false)
{
    setWindowTitle(tr("WizNote"));
    //
    initActions();
    initMenuBar();
    initToolBar();
    initClient();
    initStatusBar();
    //
    connect(m_category, SIGNAL(itemSelectionChanged()), this, SLOT(on_category_itemSelectionChanged()));
    connect(m_documents, SIGNAL(itemSelectionChanged()), this, SLOT(on_documents_itemSelectionChanged()));
    //
    m_category->init();
}

MainWindow::~MainWindow()
{
    delete m_history;
}

void MainWindow::initActions()
{
    m_actions->init();
}

void MainWindow::initMenuBar()
{
#if defined(Q_WS_MAC)
    setMenuBar(m_menuBar);
    m_actions->buildMenuBar(m_menuBar, ::WizGetAppPath() + "files/mainmenu.ini");
#else
    m_menuBar->hide();
#endif
}

void MainWindow::initToolBar()
{
#ifdef Q_OS_MAC
    //

    CWizMacToolBar* toolbar = new CWizMacToolBar(this);
    //
    QActionGroup* groupNavigate = new QActionGroup(this);
    groupNavigate->addAction(m_actions->actionFromName("actionGoBack"));
    groupNavigate->addAction(m_actions->actionFromName("actionGoForward"));
    toolbar->addActionGroup(groupNavigate);
    //
    toolbar->addStandardItem(CWizMacToolBar::Space);

    toolbar->addAction(m_actions->actionFromName("actionSync"));
    //
    toolbar->addStandardItem(CWizMacToolBar::Space);
    //
    toolbar->addAction(m_actions->actionFromName("actionNewNote"));
    toolbar->addAction(m_actions->actionFromName("actionDeleteCurrentNote"));
    //
    toolbar->addStandardItem(CWizMacToolBar::FlexibleSpace);
    //
    toolbar->addSearch();
    //
    connect(toolbar, SIGNAL(doSearch(const QString&)), this, SLOT(on_search_doSearch(const QString&)));
    //
    toolbar->showInWindow(this);
    //
#else
    addToolBar(m_toolBar);

    m_toolBar->setMovable(false);
    m_toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    //
    m_toolBar->addAction(m_actions->actionFromName("actionPopupMainMenu"));
    m_toolBar->addWidget(new CWizFixedSpacer(QSize(10, 1), m_toolBar));
    m_toolBar->addAction(m_actions->actionFromName("actionSync"));
    //
    m_toolBar->addWidget(new CWizFixedSpacer(QSize(10, 1), m_toolBar));
    //
    m_toolBar->addAction(m_actions->actionFromName("actionNewNote"));
    m_toolBar->addAction(m_actions->actionFromName("actionDeleteCurrentNote"));
    //
    m_toolBar->addWidget(new CWizFixedSpacer(QSize(10, 1), m_toolBar));
    //
    m_toolBar->addAction(m_actions->actionFromName("actionGoBack"));
    m_toolBar->addAction(m_actions->actionFromName("actionGoForward"));
#if 0
    QAction* pCaptureScreenAction = m_actions->actionFromName("actionCaptureScreen");
    m_actions->buildActionMenu(pCaptureScreenAction, this, ::WizGetAppPath() + "files/mainmenu.ini");
    m_toolBar->addAction(pCaptureScreenAction);
#endif
    //
    m_toolBar->addWidget(new CWizSpacer(m_toolBar));
    //
    CWizSearchBox* searchBox = new CWizSearchBox();
    connect(searchBox, SIGNAL(doSearch(const QString&)), this, SLOT(on_search_doSearch(const QString&)));
    m_toolBar->addWidget(searchBox);
    //
#ifndef Q_WS_MAC
    m_toolBar->addWidget(new CWizFixedSpacer(QSize(2, 1), m_toolBar));
#endif
    //
    m_toolBar->setStyle(WizGetStyle());
#endif
}

void MainWindow::initClient()
{
    QWidget* client = new QWidget(this);
    setCentralWidget(client);

#ifndef Q_OS_MAC
    client->setContentsMargins(4, 4, 4, 4);
#endif
    //
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight, client);
    client->setLayout(layout);
    layout->setMargin(0);;
    //
    CWizSplitter *splitter = new CWizSplitter(client);
    layout->addWidget(splitter);
    //
    splitter->addWidget(m_category);
    splitter->addWidget(m_documents);
    splitter->addWidget(m_doc);
    //
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 0);
    splitter->setStretchFactor(2, 1);
    //
    QPalette pal = client->palette();
    pal.setColor(QPalette::Window, QColor(0x80, 0x80, 0x80));
    client->setPalette(pal);
    client->setAutoFillBackground(true);
}

void MainWindow::initStatusBar()
{
    setStatusBar(m_statusBar);
    //
    m_progressSync->setRange(0, 100);
    m_statusBar->addWidget(m_progressSync, 20);
    m_statusBar->addWidget(m_labelStatus, 80);
    //
    m_progressSync->setVisible(false);
    m_labelStatus->setVisible(false);
    //
    m_statusBar->hide();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    m_sync.abort();
    //
    QMainWindow::closeEvent(e);
}

QSize MainWindow::sizeHint() const
{
    QDesktopWidget * desk = QApplication::desktop();
    QRect rc = desk->rect();
#ifdef Q_OS_MAC
    rc.adjust(100, 100, -100, -200);
#else
    rc.adjust(100, 100, -100, -100);
#endif
    //
    return rc.size();
}

QWidget* MainWindow::mainWindow()
{
    return this;
}

QObject* MainWindow::object()
{
    return this;
}

CWizDatabase& MainWindow::database()
{
    return m_db;
}

void MainWindow::syncStarted()
{
    m_progressSync->setValue(0);
    m_progressSync->setVisible(true);
    m_labelStatus->setVisible(true);
    m_labelStatus->setText("");
    //
    m_statusBar->show();
}

void MainWindow::syncDone(bool error)
{
    if (!error)
    {
        m_statusBar->hide();
    }
    //
    m_progressSync->setVisible(false);
    m_labelStatus->setVisible(false);
}

void MainWindow::addLog(const CString& strMsg)
{
    qDebug() << strMsg;
    //
    m_labelStatus->setText(strMsg);
}
void MainWindow::addDebugLog(const CString& strMsg)
{
    //qDebug() << strMsg;
    //
    Q_UNUSED(strMsg);
}

void MainWindow::addErrorLog(const CString& strMsg)
{
    qDebug() << strMsg;
    //
    QMessageBox::critical(this, "", strMsg);
}
void MainWindow::changeProgress(int pos)
{
    m_progressSync->setValue(pos);
}


void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionSync_triggered()
{
    m_sync.startSync();
}
void MainWindow::on_actionNewNote_triggered()
{
    WIZDOCUMENTDATA data;
    //
    CWizFolder* pFolder = m_category->SelectedFolder();
    if (!pFolder)
    {
        m_category->addAndSelectFolder("/My Notes/");
        pFolder = m_category->SelectedFolder();
        //
        if (!pFolder)
            return;
    }
    //
    m_db.CreateDocumentAndInit("<body></body>", "", 0, tr("New note"), "newnote", pFolder->Location(), "", data);
    //
    m_documents->addAndSelectDocument(data);
}

void MainWindow::on_actionDeleteCurrentNote_triggered()
{
    WIZDOCUMENTDATA document = m_doc->document();
    if (document.strGUID.IsEmpty())
        return;
    //
    CWizDocument doc(m_db, document);
    doc.Delete();
}

void MainWindow::on_actionLogout_triggered()
{
    WizSetEncryptedString("Common", "Password", "");
    m_bRestart = true;
    //
    close();
}

void MainWindow::on_actionAbout_triggered()
{

}

#ifndef Q_OS_MAC
void MainWindow::on_actionPopupMainMenu_triggered()
{
    QAction* pAction = m_actions->actionFromName("actionPopupMainMenu");
    QRect rc = m_toolBar->actionGeometry(pAction);
    QPoint pt = m_toolBar->mapToGlobal(QPoint(rc.left(), rc.bottom()));
    //
    CWizSettings settings(::WizGetResourcesPath() + "files/mainmenu.ini");
    //
    QMenu* pMenu = new QMenu(this);
    m_actions->buildMenu(pMenu, settings, pAction->objectName());
    //
    pMenu->popup(pt);
}
#endif

void MainWindow::on_actionGoBack_triggered()
{
    if (!m_history->canBack())
        return;
    //
    WIZDOCUMENTDATA data = m_history->back();
    viewDocument(data, false);
    locateDocument(data);
}

void MainWindow::on_actionGoForward_triggered()
{
    if (!m_history->canForward())
        return;
    //
    WIZDOCUMENTDATA data = m_history->forward();
    viewDocument(data, false);
    locateDocument(data);
}

void MainWindow::on_category_itemSelectionChanged()
{
    CWizDocumentDataArray arrayDocument;
    m_category->getDocuments(arrayDocument);
    m_documents->setDocuments(arrayDocument);
    //
    if (arrayDocument.empty())
    {
        on_documents_itemSelectionChanged();
    }
}

void MainWindow::on_documents_itemSelectionChanged()
{
    CWizDocumentDataArray arrayDocument;
    m_documents->getSelectedDocuments(arrayDocument);
    if (arrayDocument.size() == 1)
    {
        m_doc->showClient(true);
        //
        if (!m_bUpdatingSelection)
        {
            viewDocument(arrayDocument[0], true);
        }
    }
    else
    {
        m_doc->showClient(false);
    }
}
void MainWindow::on_search_doSearch(const QString& keywords)
{
    m_category->search(keywords);
}

void MainWindow::viewDocument(const WIZDOCUMENTDATA& data, bool addToHistory)
{
    if (data.strGUID == m_doc->document().strGUID)
        return;
    //
    if (!m_doc->viewDocument(data))
        return;
    //
    if (addToHistory)
    {
        m_history->addHistory(data);
    }
    //
    m_actions->actionFromName("actionGoBack")->setEnabled(m_history->canBack());
    m_actions->actionFromName("actionGoForward")->setEnabled(m_history->canForward());
}

void MainWindow::locateDocument(const WIZDOCUMENTDATA& data)
{
    try
    {
        m_bUpdatingSelection = true;
        m_category->addAndSelectFolder(data.strLocation);
        m_documents->addAndSelectDocument(data);
    }
    catch (...)
    {

    }
    //
    m_bUpdatingSelection = FALSE;
}

QObject* MainWindow::CategoryCtrlObject()
{
    return m_category;
}

QObject* MainWindow::DocumentsCtrlObject()
{
    return m_documents;
}

QObject* MainWindow::CreateWizObject(const QString& strObjectID)
{
    CString str(strObjectID);
    if (0 == str.CompareNoCase("WizKMControls.WizCommonUI"))
    {
        return this;
    }
    //
    return NULL;
}
QString MainWindow::LoadTextFromFile(const QString& strFileName)
{
    CString strText;
    ::WizLoadUnicodeTextFromFile(strFileName, strText);
    return strText;
}


