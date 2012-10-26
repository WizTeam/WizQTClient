#include "wizmainwindow.h"

#include "wizcategoryview.h"
#include "wizdocumentlistview.h"
#include "wizdocumentview.h"
#include "wizdocumentwebview.h"
#include "wizactions.h"
#include "wizaboutdialog.h"
#include "wizpreferencedialog.h"

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
#include <QApplication>
#include <QTimer>
#include <share/wizcommonui.h>

#include "mac/wizmactoolbar.h"

#include "wiznotestyle.h"
#include "wizdocumenthistory.h"

#include "share/wizui.h"
#include "share/wizmisc.h"
#include "share/wizuihelper.h"
#include "share/wizsettings.h"
#include "share/wizanimateaction.h"

MainWindow::MainWindow(CWizDatabase& db, QWidget *parent) :
    QMainWindow(parent),
    m_db(db),
    m_settings(new CWizUserSettings(db)),
    m_console(new CWizConsoleDialog(*this, this)),
    m_sync(m_db, WIZ_API_URL, *this),
    m_menuBar(new QMenuBar(this)),
    #ifndef Q_OS_MAC
    m_toolBar(new QToolBar("Main", this)),
    m_labelNotice(NULL),
    m_optionsAction(NULL),
    #endif
    m_statusBar(new QStatusBar(this)),
    m_labelStatus(new QLabel(this)),
    m_progressSync(new QProgressBar(this)),
    m_actions(new CWizActions(*this, this)),
    m_category(new CWizCategoryView(*this, this)),
    m_documents(new CWizDocumentListView(*this, this)),
    m_doc(new CWizDocumentView(*this, this)),
    m_splitter(NULL),
    m_options(NULL),
    m_history(new CWizDocumentViewHistory()),
    m_animateSync(new CWizAnimateAction(*this, this)),
    m_syncTimer(new QTimer(this)),
    m_updater(new CWizUpdater(this)),
    m_bRestart(false),
    m_bLogoutRestart(false),
    m_bUpdatingSelection(false)
{
    initActions();
    initMenuBar();
    initToolBar();
    initClient();
    initStatusBar();

    setWindowTitle(tr("WizNote"));
}

MainWindow::~MainWindow()
{
    delete m_history;
}

void MainWindow::center(int width, int height)
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int x = (screenGeometry.width() - width) / 2;
    int y = (screenGeometry.height() - height) / 2;
    setGeometry(x, y, width, height);
}

void MainWindow::initActions()
{
    m_actions->init();
    //
    m_animateSync->setAction(m_actions->actionFromName("actionSync"));
    m_animateSync->setIcons("sync");
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
    toolbar->addAction(m_actions->actionFromName("actionOptions"));

    toolbar->addStandardItem(CWizMacToolBar::Space);
    //
    toolbar->addSearch(tr("Search"), tr("Search your notes"));
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
    //m_toolBar->addAction(m_actions->actionFromName("actionGoBack"));
    //m_toolBar->addAction(m_actions->actionFromName("actionGoForward"));
    //
    m_toolBar->addWidget(new CWizFixedSpacer(QSize(20, 1), m_toolBar));
    //
    m_toolBar->addAction(m_actions->actionFromName("actionSync"));
    //
    m_toolBar->addWidget(new CWizFixedSpacer(QSize(20, 1), m_toolBar));
    //
    m_toolBar->addAction(m_actions->actionFromName("actionNewNote"));
    m_toolBar->addAction(m_actions->actionFromName("actionDeleteCurrentNote"));
    //
    //
#if 0
    QAction* pCaptureScreenAction = m_actions->actionFromName("actionCaptureScreen");
    m_actions->buildActionMenu(pCaptureScreenAction, this, ::WizGetAppPath() + "files/mainmenu.ini");
    m_toolBar->addAction(pCaptureScreenAction);
#endif
    //
    m_toolBar->addWidget(new CWizSpacer(m_toolBar));
    //
    m_labelNotice = new QLabel("", m_toolBar);
    m_labelNotice->setOpenExternalLinks(true);
    m_toolBar->addWidget(m_labelNotice);
    //
    m_toolBar->addWidget(new CWizSpacer(m_toolBar));
    //
    //if (QAction* actionOptions = m_actions->actionFromName("actionOptions"))
    //{
    //    QToolButton* buttonOptions = new QToolButton(m_toolBar);
    //    buttonOptions->setIcon(actionOptions->icon());
    //    buttonOptions->setText("");
    //    buttonOptions->setToolTip(actionOptions->text());
    //    m_optionsAction = m_toolBar->addWidget(buttonOptions);
    //    //
    //    connect(buttonOptions, SIGNAL(clicked()), this, SLOT(on_actionOptions_triggered()));
    //}

    //CWizSearchBox* searchBox = new CWizSearchBox();
    //connect(searchBox, SIGNAL(doSearch(const QString&)), this, SLOT(on_search_doSearch(const QString&)));
    //m_toolBar->addWidget(searchBox);

    m_toolBar->addWidget(new CWizFixedSpacer(QSize(20, 1), m_toolBar));

    m_toolBar->addAction(m_actions->actionFromName("actionPopupMainMenu"));

    m_toolBar->addWidget(new CWizFixedSpacer(QSize(2, 1), m_toolBar));

    m_toolBar->setStyle(WizGetStyle(m_settings->skin()));

    CWizSettings settings(::WizGetSkinResourcePath(m_settings->skin()) + "skin.ini");
    m_toolBar->layout()->setMargin(settings.GetInt("ToolBar", "Margin", m_toolBar->layout()->margin()));

#endif // #ifdef Q_OS_MAC

    //resetNotice();
}

void MainWindow::initClient()
{
    QWidget* client = new QWidget(this);
    setCentralWidget(client);

    QPalette pal = client->palette();
    pal.setColor(QPalette::Window, WizGetClientBackgroundColor(m_settings->skin()));
    client->setPalette(pal);
    client->setAutoFillBackground(true);

#ifndef Q_OS_MAC
    WizInitWidgetMargins(m_settings->skin(), client, "Client");
    WizInitWidgetMargins(m_settings->skin(), m_doc, "Document");
#endif

    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight, client);
    client->setLayout(layout);
    layout->setMargin(0);;

    CWizSplitter *splitter = new CWizSplitter(client);
    layout->addWidget(splitter);

#ifndef Q_OS_MAC
    CWizSettings settings(::WizGetSkinResourcePath(m_settings->skin()) + "skin.ini");

    int splitterWidth = settings.GetInt("splitter", "Width", splitter->splitterWidth());
    splitter->setSplitterWidth(splitterWidth);
    QColor defSplitterColor = client->palette().color(QPalette::Window);
    QColor splitterColor = settings.GetColor("splitter", "Color", defSplitterColor);
    splitter->setSplitterColor(splitterColor);
#endif

#ifndef Q_OS_MAC
    splitter->addWidget(WizInitWidgetMarginsEx(m_settings->skin(), m_category, "Category"));
    //splitter->addWidget(WizInitWidgetMarginsEx(m_documents, "Documents"));
#else
    splitter->addWidget(m_category);
    //splitter->addWidget(m_documents);
#endif

    CWizSearchBox* searchBox = new CWizSearchBox(*this);
    connect(searchBox, SIGNAL(doSearch(const QString&)), this, SLOT(on_search_doSearch(const QString&)));

    QBoxLayout* layoutDocuments = new QBoxLayout(QBoxLayout::TopToBottom, client);

    QWidget* documents = new QWidget(splitter);
    documents->setLayout(layoutDocuments);

    layoutDocuments->addWidget(searchBox);
    layoutDocuments->addWidget(m_documents);

    splitter->addWidget(documents);

    splitter->addWidget(m_doc);

    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 0);
    splitter->setStretchFactor(2, 1);

    m_splitter = splitter;

#ifndef Q_OS_MAC
    //connect(splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(on_client_splitterMoved(int, int)));
#endif

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

//void MainWindow::resetNotice()
//{
//#ifdef Q_OS_MAC
//#else
//    if (!m_labelNotice)
//        return;
//    //
//    m_labelNotice->setText("");
//    //
//    QString notice;
//    if (!::WizGetNotice(notice)
//        || notice.isEmpty())
//        return;
//    //
//    CString strTitle = ::WizGetCommandLineValue(notice, "Title");
//    CString strLink = ::WizGetCommandLineValue(notice, "Link");
//    //
//    CString strText = CString("<a href=\"%1\">%2</a>").arg(strLink, strTitle);
//    //
//    m_labelNotice->setText(strText);
//#endif
//}

void MainWindow::init()
{
    connect(m_category, SIGNAL(itemSelectionChanged()), this, SLOT(on_category_itemSelectionChanged()));
    connect(m_documents, SIGNAL(itemSelectionChanged()), this, SLOT(on_documents_itemSelectionChanged()));

    m_sync.setDownloadAllNotesData(m_settings->downloadAllNotesData());
    m_syncTimer->setInterval(3 * 60 * 1000);    //3 minutes
    connect(m_syncTimer, SIGNAL(timeout()), this, SLOT(on_syncTimer_timeout()));
    QTimer::singleShot(30 * 1000, this, SLOT(on_syncTimer_timeout()));  //30 seconds

    m_category->init();
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
    //
    m_animateSync->startPlay();
}

void MainWindow::syncLogin()
{
    m_sync.userInfo().strNotice;
    //::WizSetNotice(m_sync.userInfo().strNotice);
    //
#ifdef QT_DEBUG
    //::WizSetNotice("/Title=test /Link=http://www.wiz.cn/");
#endif
    //resetNotice();
}

void MainWindow::syncDone(bool error)
{
    Q_UNUSED(error);
    //
    m_statusBar->hide();
    //
    m_progressSync->setVisible(error);
    m_labelStatus->setVisible(error);
    //
    m_animateSync->stopPlay();
}

void MainWindow::addLog(const CString& strMsg)
{
    qDebug() << strMsg;
    //
    m_labelStatus->setText(strMsg);
}
void MainWindow::addDebugLog(const CString& strMsg)
{
    DEBUG_TOLOG(strMsg);
}

void MainWindow::addErrorLog(const CString& strMsg)
{
    TOLOG(strMsg);
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

    CWizFolder* pFolder = m_category->SelectedFolder();
    if (!pFolder)
    {
        m_category->addAndSelectFolder("/My Notes/");
        pFolder = m_category->SelectedFolder();

        if (!pFolder)
            return;
    }

    m_db.CreateDocumentAndInit("<body><div>&nbsp;</div></body>", "", 0, tr("New note"), "newnote", pFolder->Location(), "", data);

    m_documentForEditing = data;
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

void MainWindow::on_actionConsole_triggered()
{
    m_console->show();
    m_console->vScroll->setValue(m_console->vScroll->maximum());
}

void MainWindow::on_actionLogout_triggered()
{
    m_settings->setAutoLogin(false);
    m_settings->setPassword();

    m_bLogoutRestart = true;
    close();
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dlg(*this, this);
    dlg.exec();
}

void MainWindow::on_actionPreference_triggered()
{
    CWizPreferenceWindow* preference = new CWizPreferenceWindow(*this, this);

    connect(preference, SIGNAL(settingsChanged(WizOptionsType)), SLOT(on_options_settingsChanged(WizOptionsType)));
    connect(preference, SIGNAL(restartForSettings()), SLOT(on_options_restartForSettings()));
    preference->exec();
}

#ifndef Q_OS_MAC
void MainWindow::on_actionPopupMainMenu_triggered()
{
    QAction* pAction = m_actions->actionFromName("actionPopupMainMenu");
    QRect rc = m_toolBar->actionGeometry(pAction);
    QPoint pt = m_toolBar->mapToGlobal(QPoint(rc.left(), rc.bottom()));

    CWizSettings settings(::WizGetResourcesPath() + "files/mainmenu.ini");

    QMenu* pMenu = new QMenu(this);
    m_actions->buildMenu(pMenu, settings, pAction->objectName());

    pMenu->popup(pt);
}

void MainWindow::on_client_splitterMoved(int pos, int index)
{
    if (0 == index)
    {
    }
    else if (1 == index)
    {
        QPoint pt(pos, 0);
        //
        pt = m_splitter->mapToGlobal(pt);
        //
        adjustToolBarSpacerToPos(0, pt.x());
    }
    else if (2 == index)
    {
        QPoint pt(pos, 0);
        //
        pt = m_splitter->mapToGlobal(pt);
        //
        adjustToolBarSpacerToPos(1, pt.x());
    }
}

#endif

void MainWindow::on_actionGoBack_triggered()
{
    if (!m_history->canBack())
        return;

    WIZDOCUMENTDATA data = m_history->back();
    viewDocument(data, false);
    locateDocument(data);
}

void MainWindow::on_actionGoForward_triggered()
{
    if (!m_history->canForward())
        return;

    WIZDOCUMENTDATA data = m_history->forward();
    viewDocument(data, false);
    locateDocument(data);
}

void MainWindow::on_category_itemSelectionChanged()
{
    CWizDocumentDataArray arrayDocument;
    m_category->getDocuments(arrayDocument);
    m_documents->setDocuments(arrayDocument);

    if (arrayDocument.empty()) {
        on_documents_itemSelectionChanged();
    }
}

void MainWindow::on_documents_itemSelectionChanged()
{
    CWizDocumentDataArray arrayDocument;
    m_documents->getSelectedDocuments(arrayDocument);

    if (arrayDocument.size() == 1) {
        m_doc->showClient(true);

        if (!m_bUpdatingSelection) {
            viewDocument(arrayDocument[0], true);
        }
    } else {
        m_doc->showClient(false);
    }
}

void MainWindow::on_search_doSearch(const QString& keywords)
{
    m_category->search(keywords);
}

void MainWindow::on_options_settingsChanged(WizOptionsType type)
{
    if (wizoptionsNoteView == type)
    {
        m_doc->settingsChanged();
    }
    else if (wizoptionsSync == type)
    {
        m_sync.setDownloadAllNotesData(m_settings->downloadAllNotesData());
        m_sync.resetProxy();
    }
}

void MainWindow::on_options_restartForSettings()
{
    m_bRestart = true;
    QTimer::singleShot(100, this, SLOT(close()));
}

void MainWindow::on_syncTimer_timeout()
{
    if (!m_settings->autoSync())
        return;

    on_actionSync_triggered();
}

void MainWindow::viewDocument(const WIZDOCUMENTDATA& data, bool addToHistory)
{
    CWizDocument* doc = new CWizDocument(m_db, data);

    if (doc->GUID() == m_doc->document().strGUID)
        return;

    bool forceEdit = false;
    if (doc->GUID() == m_documentForEditing.strGUID) {
        m_documentForEditing = WIZDOCUMENTDATA();
        forceEdit = true;
    }

    // stub here
    if (doc->isProtected()) {
        QString strFileName = WizGetResourcesPath() + "transitions/commingsoon.html";
        m_doc->loadSpecialPage(strFileName);
        return;
    }

    if (!m_doc->viewDocument(data, forceEdit))
        return;

    if (addToHistory) {
        m_history->addHistory(data);
    }

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

    m_bUpdatingSelection = FALSE;
}

#ifndef Q_OS_MAC
CWizFixedSpacer* MainWindow::findFixedSpacer(int index)
{
    if (!m_toolBar)
        return NULL;
    //
    int i = 0;
    //
    QList<QAction*> actions = m_toolBar->actions();
    foreach (QAction* action, actions)
    {
        QWidget* widget = m_toolBar->widgetForAction(action);
        if (!widget)
            continue;
        //
        if (CWizFixedSpacer* spacer = dynamic_cast<CWizFixedSpacer*>(widget))
        {
            if (index == i)
                return spacer;
            //
            i++;
        }
    }
    //
    return NULL;
}


void MainWindow::adjustToolBarSpacerToPos(int index, int pos)
{
    if (!m_toolBar)
        return;
    //
    CWizFixedSpacer* spacer = findFixedSpacer(index);
    if (!spacer)
        return;
    //
    QPoint pt = spacer->mapToGlobal(QPoint(0, 0));
    //
    if (pt.x() > pos)
        return;
    //
    int width = pos - pt.x();
    //
    spacer->adjustWidth(width);
}


#endif

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
        static CWizCommonUI* commonUI = new CWizCommonUI(this);
        return commonUI;
    }
    //
    return NULL;
}
void MainWindow::SetDocumentModified(bool modified)
{
    m_doc->setModified(modified);
}
void MainWindow::SetSavingDocument(bool saving)
{
    m_statusBar->setVisible(saving);
    if (saving)
    {
        m_labelStatus->setVisible(true);
        m_labelStatus->setText(tr("Saving note..."));
        qApp->processEvents(QEventLoop::AllEvents);
    }
    else
    {
        m_labelStatus->setVisible(false);
    }
    //
}


