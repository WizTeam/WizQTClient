#include "wizmainwindow.h"

#ifdef Q_OS_MAC
#include <Carbon/Carbon.h>
#endif

#include "wizdocumentwebview.h"
#include "wizactions.h"
#include "wizaboutdialog.h"
#include "wizpreferencedialog.h"
#include "wizstatusbar.h"
#include "wizupgradenotifydialog.h"

#include "share/wizcommonui.h"
#include "share/wizui.h"
#include "share/wizmisc.h"
#include "share/wizuihelper.h"
#include "share/wizsettings.h"
#include "share/wizanimateaction.h"

#include "share/wizSearchIndexer.h"

#include "wizSearchBox.h"

#include "wiznotestyle.h"
#include "wizdocumenthistory.h"


MainWindow::MainWindow(CWizDatabase& db, QWidget *parent)
    : QMainWindow(parent)
    , m_db(db)
    , m_settings(new CWizUserSettings(db))
    , m_sync(new CWizSyncThread(*this, this))
    , m_syncTimer(new QTimer(this))
    , m_console(new CWizConsoleDialog(*this, this))
    , m_upgrade(new CWizUpgradeThread(this))
    , m_certManager(new CWizCertManager(*this))
    , m_cipherForm(new CWizUserCipherForm(*this, this))
    , m_objectDownloadDialog(new CWizDownloadObjectDataDialog(db, this))
    #ifndef Q_OS_MAC
    , m_toolBar(new QToolBar("Main", this))
    , m_labelNotice(NULL)
    , m_optionsAction(NULL)
    #else
    , m_toolBar(new QToolBar("Main", this))
    #endif
    , m_menuBar(new QMenuBar(this))
    , m_statusBar(new CWizStatusBar(*this, this))
    , m_actions(new CWizActions(*this, this))
    , m_category(new CWizCategoryView(*this, this))
    , m_documents(new CWizDocumentListView(*this, this))
    , m_doc(new CWizDocumentView(*this, this))
    , m_splitter(NULL)
    , m_options(NULL)
    , m_history(new CWizDocumentViewHistory())
    , m_animateSync(new CWizAnimateAction(*this, this))
    , m_bRestart(false)
    , m_bLogoutRestart(false)
    , m_bUpdatingSelection(false)
    , m_bReadyQuit(false)
    , m_bRequestQuit(false)
{
    // FTS engine thread
    m_searchIndexer = new CWizSearchIndexerThread(m_db, this);
    m_searchIndexer->start();

    connect(m_searchIndexer, SIGNAL(documentFind(const CWizDocumentDataArray&)), \
            SLOT(on_searchDocumentFind(const CWizDocumentDataArray&)));

    if (!m_db.isDocumentFTSEnabled()) {
       QTimer::singleShot(5 * 1000, m_searchIndexer->worker(), SLOT(rebuildFTSIndex()));
    }

    // upgrade check thread
#ifndef Q_OS_MAC
    // start update check thread
    QTimer::singleShot(3 * 1000, m_upgrade, SLOT(checkUpgradeBegin()));
    connect(m_upgrade, SIGNAL(finished()), SLOT(on_upgradeThread_finished()));
#endif // Q_OS_MAC

    // auto sync thread
    m_syncTimer->setInterval(15 * 60 * 1000);    //15 minutes
    connect(m_syncTimer, SIGNAL(timeout()), SLOT(on_actionSync_triggered()));
    if (m_settings->autoSync()) {
        QTimer::singleShot(3 * 1000, this, SLOT(on_actionSync_triggered()));
    }

    setStatusBar(m_statusBar);

    initActions();
    initMenuBar();
    initToolBar();
    initClient();

    setWindowTitle(tr("WizNote"));
    restoreStatus();
}

void MainWindow::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);

    m_statusBar->hide();
    m_cipherForm->hide();
}

bool MainWindow::requestThreadsQuit()
{
    bool bOk = true;
    if (m_sync->isRunning()) {
        m_sync->abort();
        bOk = false;
    }

    if (m_upgrade->isRunning()) {
        m_upgrade->abort();
        bOk = false;
    }

    if (m_searchIndexer->isRunning()) {
        m_searchIndexer->quit();
        bOk = false;
    }

    return bOk;
}

void MainWindow::on_quitTimeout()
{
    if (requestThreadsQuit()) {
        saveStatus();

        // FIXME : if document not valid will lead crash
        m_doc->view()->saveDocument(false);
        m_bReadyQuit = true;

        // back to closeEvent
        close();
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // internal close event
    if (m_bRequestQuit) {
        if (m_bReadyQuit) {
            event->accept();
            return;
        } else {
            event->ignore();
            hide();
            m_timerQuit.start();
            return;
        }
    }

    // system close event
    if (isActiveWindow()) {
        // This is bug of Qt!
        // use hide() directly lead window can't be shown when click dock icon
        // call native API instead
#ifdef Q_OS_MAC
        ProcessSerialNumber pn;
        GetFrontProcess(&pn);
        ShowHideProcess(&pn,false);
#else
        showMinimized();
#endif
    } else {
        on_actionExit_triggered();
    }

    event->ignore();
}

void MainWindow::on_actionExit_triggered()
{
    m_bRequestQuit = true;

    m_timerQuit.setInterval(100);
    connect(&m_timerQuit, SIGNAL(timeout()), SLOT(on_quitTimeout()));
    m_timerQuit.start();
}

void MainWindow::on_upgradeThread_finished()
{
    QString strUrl = m_upgrade->whatsNewUrl();
    if (strUrl.isEmpty()) {
        return;
    }

    CWizUpgradeNotifyDialog notifyDialog(strUrl, this);

    if (QDialog::Accepted == notifyDialog.exec()) {
        QFile file(::WizGetUpgradePath() + "WIZNOTE_READY_FOR_UPGRADE");
        file.open(QIODevice::WriteOnly | QIODevice::Truncate);
        file.close();

        // restart immediately
        m_bRestart = true;
        on_actionExit_triggered();
    } else {
        // skip for this session
        QFile file(::WizGetUpgradePath() + "WIZNOTE_SKIP_THIS_SESSION");
        file.open(QIODevice::WriteOnly | QIODevice::Truncate);
        file.close();
    }
}

MainWindow::~MainWindow()
{
    delete m_history;
}

void MainWindow::saveStatus()
{
    CWizSettings settings(WizGetSettingsFileName());
    settings.setValue("window/geometry", saveGeometry());
}

void MainWindow::restoreStatus()
{
    CWizSettings settings(WizGetSettingsFileName());
    QByteArray geometry = settings.value("window/geometry").toByteArray();

    if (geometry.isEmpty()) {
        setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, \
                                        sizeHint(), qApp->desktop()->availableGeometry()
                                        ));
    } else {
        restoreGeometry(geometry);
    }
}

void MainWindow::initActions()
{
    m_actions->init();
    m_animateSync->setAction(m_actions->actionFromName("actionSync"));
    m_animateSync->setIcons("sync");
}

void MainWindow::initMenuBar()
{
//#ifdef Q_WS_MAC
    setMenuBar(m_menuBar);
    m_actions->buildMenuBar(m_menuBar, ::WizGetResourcesPath() + "files/mainmenu.ini");
//#else
//    m_menuBar->hide();
//#endif
}

void MainWindow::initToolBar()
{
    m_toolBar->setMovable(false);
    m_toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toolBar->setContentsMargins(0, 0, 0, 0);

    m_toolBar->addWidget(new CWizFixedSpacer(QSize(20, 1), m_toolBar));
    m_toolBar->addAction(m_actions->actionFromName("actionSync"));
    m_toolBar->addWidget(new CWizFixedSpacer(QSize(20, 1), m_toolBar));
    m_toolBar->addAction(m_actions->actionFromName("actionNewNote"));
    m_toolBar->addAction(m_actions->actionFromName("actionDeleteCurrentNote"));

    m_toolBar->addWidget(new CWizSpacer(m_toolBar));

    m_searchBox = new CWizSearchBox(*this, this);
    connect(m_searchBox, SIGNAL(doSearch(const QString&)), SLOT(on_search_doSearch(const QString&)));

    m_toolBar->addWidget(m_searchBox);
    m_toolBar->layout()->setAlignment(m_searchBox, Qt::AlignBottom);

    m_toolBar->addWidget(new CWizFixedSpacer(QSize(20, 1), m_toolBar));

//#ifndef Q_OS_MAC
//    m_toolBar->addAction(m_actions->actionFromName("actionPopupMainMenu"));
//    m_toolBar->addWidget(new CWizFixedSpacer(QSize(2, 1), m_toolBar));
//#endif

    //m_toolBar->setStyle(WizGetStyle(m_settings->skin()));
    //CWizSettings settings(::WizGetSkinResourcePath(m_settings->skin()) + "skin.ini");
    //m_toolBar->layout()->setMargin(settings.GetInt("ToolBar", "Margin", m_toolBar->layout()->margin()));


#ifdef Q_OS_MAC
    m_toolBar->setIconSize(QSize(24, 24));
    setUnifiedTitleAndToolBarOnMac(true);
#else
    m_toolBar->setIconSize(QSize(32, 32));
#endif

    addToolBar(m_toolBar);

//#ifdef Q_OS_MAC
//    addToolBar(m_toolBar);
//
//    m_toolBar->addWidget(new CWizFixedSpacer(QSize(20, 1), this));
//    m_toolBar->addAction(m_actions->actionFromName("actionSync"));
//
//    m_toolBar->addWidget(new CWizFixedSpacer(QSize(20, 1), this));
//    m_toolBar->addAction(m_actions->actionFromName("actionNewNote"));
//    m_toolBar->addAction(m_actions->actionFromName("actionDeleteCurrentNote"));
//    m_toolBar->addWidget(new CWizSpacer(this));

//    CWizMacToolBar* m_toolBar = new CWizMacToolBar(this);
//
//    //QActionGroup* groupNavigate = new QActionGroup(this);
//    //groupNavigate->addAction(m_actions->actionFromName("actionGoBack"));
//    //groupNavigate->addAction(m_actions->actionFromName("actionGoForward"));
//    //toolbar->addActionGroup(groupNavigate);
//    //toolbar->addStandardItem(CWizMacToolBar::Space);
//
//    m_toolBar->addAction(m_actions->actionFromName("actionSync"));
//    m_toolBar->addStandardItem(CWizMacToolBar::Space);
//
//    m_toolBar->addAction(m_actions->actionFromName("actionNewNote"));
//    m_toolBar->addAction(m_actions->actionFromName("actionDeleteCurrentNote"));
//    m_toolBar->addStandardItem(CWizMacToolBar::FlexibleSpace);
//
//    //toolbar->addAction(m_actions->actionFromName("actionOptions"));
//    //toolbar->addStandardItem(CWizMacToolBar::Space);
//    m_toolBar->addSearch(tr("Search"), tr("Search your notes"));
//    connect(m_toolBar, SIGNAL(doSearch(const QString&)), SLOT(on_search_doSearch(const QString&)));
//
//    m_toolBar->showInWindow(this);

//    QAction* pCaptureScreenAction = m_actions->actionFromName("actionCaptureScreen");
//    m_actions->buildActionMenu(pCaptureScreenAction, this, ::WizGetAppPath() + "files/mainmenu.ini");
//    m_toolBar->addAction(pCaptureScreenAction);

//    m_labelNotice = new QLabel("", m_toolBar);
//    m_labelNotice->setOpenExternalLinks(true);
//    m_toolBar->addWidget(m_labelNotice);
//    m_toolBar->addWidget(new CWizSpacer(m_toolBar));
}

void MainWindow::initClient()
{
    QWidget* client = new QWidget(this);
    setCentralWidget(client);

    client->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    QPalette pal = client->palette();
    pal.setColor(QPalette::Window, WizGetClientBackgroundColor(m_settings->skin()));
    client->setPalette(pal);
    client->setAutoFillBackground(true);

    QHBoxLayout* layout = new QHBoxLayout(client);
    layout->setContentsMargins(0, 0, 0, 0);
    client->setLayout(layout);

#ifndef Q_OS_MAC
    WizInitWidgetMargins(m_settings->skin(), client, "Client");
    WizInitWidgetMargins(m_settings->skin(), m_doc, "Document");
#endif

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

    QVBoxLayout* layoutDocuments = new QVBoxLayout(client);
    layoutDocuments->setContentsMargins(1, 1, 1, 1);
    layoutDocuments->setSpacing(3);

    QWidget* documents = new QWidget(splitter);
    documents->setLayout(layoutDocuments);

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
    connect(m_sync, SIGNAL(syncStarted()), SLOT(on_syncStarted()));
    connect(m_sync, SIGNAL(syncLogined()), SLOT(on_syncLogined()));
    connect(m_sync, SIGNAL(processLog(const QString&)), SLOT(on_syncProcessLog(const QString&)));
    //connect(m_sync, SIGNAL(processDebugLog(const QString&)), SLOT(on_syncProcessDebugLog(const QString&)));
    connect(m_sync, SIGNAL(processErrorLog(const QString&)), SLOT(on_syncProcessErrorLog(const QString&)));
    connect(m_sync, SIGNAL(syncDone(bool)), SLOT(on_syncDone(bool)));

    connect(m_category, SIGNAL(itemSelectionChanged()), this, SLOT(on_category_itemSelectionChanged()));
    connect(m_documents, SIGNAL(itemSelectionChanged()), this, SLOT(on_documents_itemSelectionChanged()));

    m_category->init();
}



void MainWindow::on_syncStarted()
{
    m_statusBar->show();
    m_animateSync->startPlay();
    m_syncTimer->stop();
}

void MainWindow::on_syncLogined()
{
    //m_sync.userInfo().strNotice;
    //::WizSetNotice(m_sync.userInfo().strNotice);
    //
#ifdef QT_DEBUG
    //::WizSetNotice("/Title=test /Link=http://www.wiz.cn/");
#endif
    //resetNotice();
}

void MainWindow::on_syncDone(bool error)
{
    Q_UNUSED(error);

    m_statusBar->hide();
    m_animateSync->stopPlay();

    if (m_settings->autoSync()) {
        m_syncTimer->start();
    }
}

void MainWindow::on_syncProcessLog(const QString& msg)
{
    TOLOG(msg);

    QString strMsg = msg.left(50) + "..";
    m_statusBar->setText(strMsg);
}

void MainWindow::on_syncProcessDebugLog(const QString& strMsg)
{
    DEBUG_TOLOG(strMsg);
}

void MainWindow::on_syncProcessErrorLog(const QString& strMsg)
{
    TOLOG(strMsg);
}



void MainWindow::on_actionSync_triggered()
{
    m_certManager->downloadUserCert();
    m_sync->startSyncing();
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
    // save state
    m_settings->setAutoLogin(false);
    m_settings->setPassword();
    m_bLogoutRestart = true;
    on_actionExit_triggered();
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

void MainWindow::on_actionRebuildFTS_triggered()
{
    if (!QMetaObject::invokeMethod(m_searchIndexer->worker(), "rebuildFTSIndex")) {
        TOLOG("thread call to rebuild FTS failed");
    }
}

void MainWindow::on_actionSearch_triggered()
{
    m_searchBox->focus();
}

void MainWindow::on_actionResetSearch_triggered()
{
    m_searchBox->clear();
    m_searchBox->focus();
    m_category->restoreSelection();
}

void MainWindow::on_searchDocumentFind(const CWizDocumentDataArray& arrayDocument)
{
    m_documents->addDocuments(arrayDocument);
    on_documents_itemSelectionChanged();
}

void MainWindow::on_search_doSearch(const QString& keywords)
{
    if (keywords.isEmpty()) {
        m_category->restoreSelection();
        return;
    }

    m_category->search(keywords);
    m_documents->clear();

    if (!QMetaObject::invokeMethod(m_searchIndexer->worker(), "search", \
                                   Q_ARG(QString, keywords), \
                                   Q_ARG(int, 100))) {
        TOLOG("FTS search failed");
    }
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

        pt = m_splitter->mapToGlobal(pt);

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



void MainWindow::on_options_settingsChanged(WizOptionsType type)
{
    if (wizoptionsNoteView == type)
    {
        m_doc->settingsChanged();
    }
    else if (wizoptionsSync == type)
    {
        //m_sync->setDownloadAllNotesData(m_settings->downloadAllNotesData());
        m_sync->resetProxy();
    }
}

void MainWindow::on_options_restartForSettings()
{
    m_bRestart = true;
    on_actionExit_triggered();
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

    m_bUpdatingSelection = false;
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

//QObject* MainWindow::CategoryCtrl()
//{
//    return m_category;
//}
//
//QObject* MainWindow::DocumentsCtrl()
//{
//    return m_documents;
//}


//================================================================================
// WizExplorerApp APIs
//================================================================================

QObject* MainWindow::CreateWizObject(const QString& strObjectID)
{
    CString str(strObjectID);
    if (0 == str.CompareNoCase("WizKMControls.WizCommonUI"))
    {
        static CWizCommonUI* commonUI = new CWizCommonUI(this);
        return commonUI;
    }

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
        m_statusBar->setVisible(true);
        m_statusBar->setText(tr("Saving note..."));
        qApp->processEvents(QEventLoop::AllEvents);
    }
    else
    {
        m_statusBar->setVisible(false);
    }
}
