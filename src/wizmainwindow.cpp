#include "wizmainwindow.h"

#include <QToolBar>
#include <QMenuBar>
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QUndoStack>
#include <QEvent>
#include <QHBoxLayout>

#ifdef Q_OS_MAC
#include <Carbon/Carbon.h>
#include "mac/wizmachelper.h"
#include "mac/wizmactoolbar.h"
#include "mac/wizSearchWidget_mm.h"
#else
#include "wizSearchWidget.h"
#endif

#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>

#include "wizDocumentWebView.h"
#include "wizactions.h"
#include "wizAboutDialog.h"
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

#include "wiznotestyle.h"
#include "wizdocumenthistory.h"

#include "wizButton.h"

#include "wizEditorToolBar.h"
#include "wizProgressDialog.h"
#include "wizDocumentSelectionView.h"
#include "share/wizObjectDataDownloader.h"
#include "wizDocumentTransitionView.h"
#include "messagelistview.h"

#include "wizPopupButton.h"
#include "widgets/wizUserInfoWidget.h"
#include "sync/apientry.h"
#include "sync/wizkmsync.h"

#include "wizUserVerifyDialog.h"

#include "plugindialog.h"

#include "notecomments.h"

using namespace Core;
using namespace Core::Internal;
using namespace WizService::Internal;

MainWindow::MainWindow(CWizDatabaseManager& dbMgr, QWidget *parent)
    : QMainWindow(parent)
    , m_core(new ICore(this))
    , m_dbMgr(dbMgr)
    , m_progress(new CWizProgressDialog(this))
    , m_settings(new CWizUserSettings(dbMgr.db()))
    , m_sync(new CWizKMSyncThread(dbMgr.db(), this))
    , m_syncTimer(new QTimer(this))
    , m_searchIndexer(new CWizSearchIndexer(m_dbMgr))
    , m_upgrade(new CWizUpgrade())
    //, m_certManager(new CWizCertManager(*this))
    , m_cipherForm(new CWizUserCipherForm(*this, this))
    , m_objectDownloaderHost(new CWizObjectDataDownloaderHost(dbMgr, this))
    //, m_avatarDownloaderHost(new CWizUserAvatarDownloaderHost(dbMgr.db().GetAvatarPath(), this))
    , m_transitionView(new CWizDocumentTransitionView(this))
    #ifndef Q_OS_MAC
    , m_labelNotice(NULL)
    , m_optionsAction(NULL)
    #endif
    #ifdef Q_OS_MAC
    , m_toolBar(new CWizMacToolBar(this))
    #else
    , m_toolBar(new QToolBar("Main", this))
    #endif
    , m_menuBar(new QMenuBar(this))
    , m_statusBar(new CWizStatusBar(*this, this))
    , m_actions(new CWizActions(*this, this))
    , m_category(new CWizCategoryView(*this, this))
    , m_documents(new CWizDocumentListView(*this, this))
    , m_noteList(NULL)
    , m_msgList(new MessageListView(this))
    , m_documentSelection(new CWizDocumentSelectionView(*this, this))
    , m_doc(new CWizDocumentView(*this, this))
    , m_history(new CWizDocumentViewHistory())
    , m_animateSync(new CWizAnimateAction(*this, this))
    , m_bRestart(false)
    , m_bLogoutRestart(false)
    , m_bUpdatingSelection(false)
{
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(on_application_aboutToQuit()));
    connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit())); // Qt bug: Qt5 bug
    qApp->installEventFilter(this);

    //CWizCloudPool::instance()->init(&m_dbMgr);

    // search and full text search
    QThread *threadFTS = new QThread();
    m_searchIndexer->moveToThread(threadFTS);
    threadFTS->start(QThread::IdlePriority);

    // upgrade check
    QThread *thread = new QThread();
    m_upgrade->moveToThread(thread);
    connect(m_upgrade, SIGNAL(checkFinished(bool)), SLOT(on_checkUpgrade_finished(bool)));
    thread->start(QThread::IdlePriority);

    // syncing thread
    connect(m_sync, SIGNAL(processLog(const QString&)), SLOT(on_syncProcessLog(const QString&)));
    connect(m_sync, SIGNAL(syncFinished(int, QString)), SLOT(on_syncDone(int, QString)));
    connect(m_syncTimer, SIGNAL(timeout()), SLOT(on_actionAutoSync_triggered()));
    int nInterval = m_settings->syncInterval();
    if (nInterval == 0) {
        m_syncTimer->setInterval(15 * 60 * 1000);   // default 15 minutes
    } else {
        m_syncTimer->setInterval(nInterval * 60 * 1000);
    }

    if (nInterval != -1) {
        QTimer::singleShot(3 * 1000, this, SLOT(on_actionAutoSync_triggered()));
    }

    // misc settings
    //m_avatarDownloaderHost->setDefault(::WizGetSkinResourcePath(userSettings().skin()) + "avatar_default.png");

    // GUI
    initActions();
    initMenuBar();
    initToolBar();
    initClient();

    setWindowTitle(tr("WizNote"));

    restoreStatus();

    client()->hide();

#ifdef Q_OS_MAC
    setupFullScreenMode(this);
#endif // Q_OS_MAC

    WizService::NoteComments::init();
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    // Qt issue: issue? User quit for mac dock send close event to qApp?
    // I was throught close event only send to widget.
    if (watched == qApp) {
        if (event->type() == QEvent::Close) {
            qApp->quit();
            return true;
        } else {
            return false;
        }
    } else {
        return QMainWindow::eventFilter(watched, event);
    }
}

void MainWindow::on_application_aboutToQuit()
{
    cleanOnQuit();
}

void MainWindow::cleanOnQuit()
{
    m_category->saveState();
    saveStatus();

    // FIXME : if document not valid will lead crash
    //m_doc->web()->saveDocument(false);

    m_sync->stopSync();
    m_searchIndexer->abort();

    m_sync->wait();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
#ifdef Q_OS_MAC
    // Qt issue: use hide() directly lead window can't be shown when click dock icon
    // call native API instead and ignore issue event.
    ProcessSerialNumber pn;
    GetFrontProcess(&pn);
    ShowHideProcess(&pn,false);
    event->ignore();
#else
    event->accept();
    qApp->quit();
#endif
}

void MainWindow::on_actionExit_triggered()
{
    qApp->quit();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    m_statusBar->adjustPosition();
}

void MainWindow::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);

    m_statusBar->hide();
    m_cipherForm->hide();
}

void MainWindow::on_checkUpgrade_finished(bool bUpgradeAvaliable)
{
    if (!bUpgradeAvaliable)
        return;

    QString strUrl = m_upgrade->getWhatsNewUrl();
    CWizUpgradeNotifyDialog notifyDialog(strUrl, this);
    if (QDialog::Accepted == notifyDialog.exec()) {
#if defined(Q_OS_MAC)
        QUrl url("http://www.wiz.cn/wiznote-mac.html");
#elif defined(Q_OS_LINUX)
        QUrl url("http://www.wiz.cn/wiznote-linux.html");
#else
        Q_ASSERT(0);
#endif
        QDesktopServices::openUrl(url);
    }
}

#ifdef WIZ_OBOSOLETE
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
#endif

MainWindow::~MainWindow()
{
    delete m_history;
}

void MainWindow::saveStatus()
{
    QSettings* settings = ExtensionSystem::PluginManager::globalSettings();
    settings->setValue("Window/Geometry", saveGeometry());
    settings->setValue("Window/Splitter", m_splitter->saveState());
}

void MainWindow::restoreStatus()
{
    QSettings* settings = ExtensionSystem::PluginManager::globalSettings();
    QByteArray geometry = settings->value("Window/Geometry").toByteArray();

    // main window
    if (geometry.isEmpty()) {
        setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, \
                                        sizeHint(), qApp->desktop()->availableGeometry()
                                        ));
    } else {
        restoreGeometry(geometry);
    }

    m_splitter->restoreState(settings->value("Window/Splitter").toByteArray());
}

void MainWindow::initActions()
{
    m_actions->init();
    m_animateSync->setAction(m_actions->actionFromName(WIZACTION_GLOBAL_SYNC));
    m_animateSync->setIcons("sync");

    connect(m_doc->web(), SIGNAL(statusChanged()), SLOT(on_editor_statusChanged()));
    //connect(m_doc->web()->page(), SIGNAL(contentsChanged()), SLOT(on_document_contentChanged()));
    //connect(m_doc->web()->page(), SIGNAL(selectionChanged()), SLOT(on_document_contentChanged()));

    on_editor_statusChanged();
}

void MainWindow::initMenuBar()
{
    setMenuBar(m_menuBar);
    m_actions->buildMenuBar(m_menuBar, ::WizGetResourcesPath() + "files/mainmenu.ini");
}

void MainWindow::on_editor_statusChanged()
{
    CWizDocumentWebView* editor = m_doc->web();

    if (!editor->isInited() || !editor->isEditing() || !editor->hasFocus()) {
        m_actions->actionFromName(WIZACTION_EDITOR_UNDO)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_REDO)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_CUT)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_COPY)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_PASTE)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_PASTE_PLAIN)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_DELETE)->setEnabled(false);

        m_actions->actionFromName(WIZACTION_FORMAT_BOLD)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_ITALIC)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_UNDERLINE)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_STRIKETHROUGH)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_UNORDEREDLIST)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_ORDEREDLIST)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_JUSTIFYLEFT)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_JUSTIFYRIGHT)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_JUSTIFYCENTER)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_JUSTIFYJUSTIFY)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_INDENT)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_OUTDENT)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_TABLE)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_LINK)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_HORIZONTAL)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_DATE)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_TIME)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_REMOVE_FORMAT)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_VIEW_SOURCE)->setEnabled(false);

        return;
    }

    if (!editor->page()->undoStack()->canUndo()) {
        m_actions->actionFromName(WIZACTION_EDITOR_UNDO)->setEnabled(false);
    } else {
        m_actions->actionFromName(WIZACTION_EDITOR_UNDO)->setEnabled(true);
    }

    if (!editor->page()->undoStack()->canRedo()) {
        m_actions->actionFromName(WIZACTION_EDITOR_REDO)->setEnabled(false);
    } else {
        m_actions->actionFromName(WIZACTION_EDITOR_REDO)->setEnabled(true);
    }

    if (!editor->isEditing()) {
        m_actions->actionFromName(WIZACTION_EDITOR_CUT)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_PASTE)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_PASTE_PLAIN)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_DELETE)->setEnabled(false);

        if (!editor->page()->selectedHtml().isEmpty()) {
            m_actions->actionFromName(WIZACTION_EDITOR_COPY)->setEnabled(true);
        } else {
            m_actions->actionFromName(WIZACTION_EDITOR_COPY)->setEnabled(false);
        }
    } else {
        m_actions->actionFromName(WIZACTION_EDITOR_PASTE)->setEnabled(true);
        m_actions->actionFromName(WIZACTION_EDITOR_PASTE_PLAIN)->setEnabled(true);

        if (!editor->page()->selectedHtml().isEmpty()) {
            m_actions->actionFromName(WIZACTION_EDITOR_CUT)->setEnabled(true);
            m_actions->actionFromName(WIZACTION_EDITOR_COPY)->setEnabled(true);
            m_actions->actionFromName(WIZACTION_EDITOR_DELETE)->setEnabled(true);
        } else {
            m_actions->actionFromName(WIZACTION_EDITOR_CUT)->setEnabled(false);
            m_actions->actionFromName(WIZACTION_EDITOR_COPY)->setEnabled(false);
            m_actions->actionFromName(WIZACTION_EDITOR_DELETE)->setEnabled(false);
        }
    }

    if (-1 == editor->editorCommandQueryCommandState("bold")) {
        m_actions->actionFromName(WIZACTION_FORMAT_BOLD)->setEnabled(false);
    } else {
        m_actions->actionFromName(WIZACTION_FORMAT_BOLD)->setEnabled(true);
    }

    if (-1 ==editor->editorCommandQueryCommandState("italic")) {
        m_actions->actionFromName(WIZACTION_FORMAT_ITALIC)->setEnabled(false);
    } else {
        m_actions->actionFromName(WIZACTION_FORMAT_ITALIC)->setEnabled(true);
    }

    if (-1 ==editor->editorCommandQueryCommandState("underline")) {
        m_actions->actionFromName(WIZACTION_FORMAT_UNDERLINE)->setEnabled(false);
    } else {
        m_actions->actionFromName(WIZACTION_FORMAT_UNDERLINE)->setEnabled(true);
    }

    if (-1 ==editor->editorCommandQueryCommandState("strikethrough")) {
        m_actions->actionFromName(WIZACTION_FORMAT_STRIKETHROUGH)->setEnabled(false);
    } else {
        m_actions->actionFromName(WIZACTION_FORMAT_STRIKETHROUGH)->setEnabled(true);
    }

    if (-1 ==editor->editorCommandQueryCommandState("insertUnorderedList")) {
        m_actions->actionFromName(WIZACTION_FORMAT_UNORDEREDLIST)->setEnabled(false);
    } else {
        m_actions->actionFromName(WIZACTION_FORMAT_UNORDEREDLIST)->setEnabled(true);
    }

    if (-1 ==editor->editorCommandQueryCommandState("insertOrderedList")) {
        m_actions->actionFromName(WIZACTION_FORMAT_ORDEREDLIST)->setEnabled(false);
    } else {
        m_actions->actionFromName(WIZACTION_FORMAT_ORDEREDLIST)->setEnabled(true);
    }

    if (-1 ==editor->editorCommandQueryCommandState("justify")) {
        m_actions->actionFromName(WIZACTION_FORMAT_JUSTIFYLEFT)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_JUSTIFYRIGHT)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_JUSTIFYCENTER)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_JUSTIFYJUSTIFY)->setEnabled(false);
    } else {
        m_actions->actionFromName(WIZACTION_FORMAT_JUSTIFYLEFT)->setEnabled(true);
        m_actions->actionFromName(WIZACTION_FORMAT_JUSTIFYRIGHT)->setEnabled(true);
        m_actions->actionFromName(WIZACTION_FORMAT_JUSTIFYCENTER)->setEnabled(true);
        m_actions->actionFromName(WIZACTION_FORMAT_JUSTIFYJUSTIFY)->setEnabled(true);
    }

    if (-1 ==editor->editorCommandQueryCommandState("indent")) {
        m_actions->actionFromName(WIZACTION_FORMAT_INDENT)->setEnabled(false);
    } else {
        m_actions->actionFromName(WIZACTION_FORMAT_INDENT)->setEnabled(true);
    }

    if (-1 ==editor->editorCommandQueryCommandState("outdent")) {
        m_actions->actionFromName(WIZACTION_FORMAT_OUTDENT)->setEnabled(false);
    } else {
        m_actions->actionFromName(WIZACTION_FORMAT_OUTDENT)->setEnabled(true);
    }

    if (-1 ==editor->editorCommandQueryCommandState("inserttable")) {
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_TABLE)->setEnabled(false);
    } else {
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_TABLE)->setEnabled(true);
    }

    if (-1 ==editor->editorCommandQueryCommandState("link")) {
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_LINK)->setEnabled(false);
    } else {
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_LINK)->setEnabled(true);
    }

    if (-1 ==editor->editorCommandQueryCommandState("horizontal")) {
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_HORIZONTAL)->setEnabled(false);
    } else {
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_HORIZONTAL)->setEnabled(true);
    }

    if (-1 ==editor->editorCommandQueryCommandState("date")) {
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_DATE)->setEnabled(false);
    } else {
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_DATE)->setEnabled(true);
    }

    if (-1 ==editor->editorCommandQueryCommandState("time")) {
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_TIME)->setEnabled(false);
    } else {
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_TIME)->setEnabled(true);
    }

    if (-1 ==editor->editorCommandQueryCommandState("removeformat")) {
        m_actions->actionFromName(WIZACTION_FORMAT_REMOVE_FORMAT)->setEnabled(false);
    } else {
        m_actions->actionFromName(WIZACTION_FORMAT_REMOVE_FORMAT)->setEnabled(true);
    }

    if (-1 ==editor->editorCommandQueryCommandState("source")) {
        m_actions->actionFromName(WIZACTION_FORMAT_VIEW_SOURCE)->setEnabled(false);
    } else {
        m_actions->actionFromName(WIZACTION_FORMAT_VIEW_SOURCE)->setEnabled(true);
    }
}

void MainWindow::initToolBar()
{
#ifdef Q_OS_MAC
    setUnifiedTitleAndToolBarOnMac(true);
#endif

#ifdef Q_OS_MAC
    m_toolBar->showInWindow(this);

    CWizUserInfoWidget* info = new CWizUserInfoWidget(*this, m_toolBar);
    m_toolBar->addWidget(info, "", "");

    m_toolBar->addStandardItem(CWizMacToolBar::Space);
    m_toolBar->addAction(m_actions->actionFromName(WIZACTION_GLOBAL_SYNC));
    m_toolBar->addStandardItem(CWizMacToolBar::Space);
    m_toolBar->addAction(m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT));
    m_toolBar->addStandardItem(CWizMacToolBar::FlexibleSpace);
    m_toolBar->addSearch(tr("Search"), "");
    //
    m_search = m_toolBar->getSearchWidget();
#else
    addToolBar(m_toolBar);

    m_toolBar->setIconSize(QSize(32, 32));
    m_toolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    m_toolBar->setMovable(false);
    m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // align with categoryview's root item.
    m_toolBar->addWidget(new CWizFixedSpacer(QSize(3, 1), m_toolBar));

    CWizUserInfoWidget* info = new CWizUserInfoWidget(*this, m_toolBar);
    m_toolBar->addWidget(info);

    m_toolBar->addWidget(new CWizFixedSpacer(QSize(60, 1), m_toolBar));

    CWizButton* buttonSync = new CWizButton(m_toolBar);
    buttonSync->setAction(m_actions->actionFromName(WIZACTION_GLOBAL_SYNC));
    m_toolBar->addWidget(buttonSync);

    m_toolBar->addWidget(new CWizFixedSpacer(QSize(20, 1), m_toolBar));

    CWizButton* buttonNew = new CWizButton(m_toolBar);
    buttonNew->setAction(m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT));
    m_toolBar->addWidget(buttonNew);

    m_toolBar->addWidget(new CWizSpacer(m_toolBar));

    m_search = new CWizSearchWidget(this);

    m_toolBar->addWidget(m_search);

    m_toolBar->layout()->setAlignment(m_search, Qt::AlignBottom);
    m_toolBar->addWidget(new CWizFixedSpacer(QSize(20, 1), m_toolBar));
#endif
    //
    connect(m_search, SIGNAL(doSearch(const QString&)), SLOT(on_search_doSearch(const QString&)));
}

void MainWindow::initClient()
{
    QWidget* client = new QWidget(this);
    setCentralWidget(client);

    client->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QPalette pal = client->palette();
    pal.setBrush(QPalette::Window, QBrush("#FFFFFF"));
    client->setPalette(pal);
    client->setAutoFillBackground(true);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    client->setLayout(layout);

    m_splitter = new CWizSplitter();
    layout->addWidget(m_splitter);

    QWidget* documentPanel = new QWidget();
    QHBoxLayout* layoutDocument = new QHBoxLayout();
    layoutDocument->setContentsMargins(0, 0, 0, 0);
    layoutDocument->setSpacing(0);
    documentPanel->setLayout(layoutDocument);
    layoutDocument->addWidget(m_doc);
    layoutDocument->addWidget(m_documentSelection);
    m_documentSelection->hide();
    // append after client
    m_doc->layout()->addWidget(m_transitionView);
    m_transitionView->hide();

    m_splitter->addWidget(m_category);

    QWidget* wlist = new QWidget(this);
    QHBoxLayout* layoutList = new QHBoxLayout();
    layoutList->setContentsMargins(0, 0, 0, 0);
    layoutList->setSpacing(0);
    layoutList->addWidget(createListView());
    layoutList->addWidget(m_msgList);
    wlist->setLayout(layoutList);
    m_splitter->addWidget(wlist);
    m_splitter->addWidget(documentPanel);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 0);
    m_splitter->setStretchFactor(2, 1);

    m_msgList->hide();
}

QWidget* MainWindow::createListView()
{
    m_noteList = new QWidget(this);
    m_noteList->setMinimumWidth(100);
    QVBoxLayout* layoutList = new QVBoxLayout();
    layoutList->setContentsMargins(0, 0, 0, 0);
    layoutList->setSpacing(0);
    m_noteList->setLayout(layoutList);

    QHBoxLayout* layoutActions = new QHBoxLayout();
    layoutActions->setContentsMargins(0, 0, 0, 0);
    layoutList->setSpacing(0);

    CWizViewTypePopupButton* viewBtn = new CWizViewTypePopupButton(*this, this);
    connect(viewBtn, SIGNAL(viewTypeChanged(int)), SLOT(on_documents_viewTypeChanged(int)));
    layoutActions->addWidget(viewBtn);
    QWidget* line = new QWidget(this);
    line->setFixedWidth(1);
    line->setStyleSheet("border-left-width:1;border-left-style:solid;border-left-color:#d9dcdd");
    layoutActions->addWidget(line);
    CWizSortingPopupButton* sortBtn = new CWizSortingPopupButton(*this, this);
    connect(sortBtn, SIGNAL(sortingTypeChanged(int)), SLOT(on_documents_sortingTypeChanged(int)));
    layoutActions->addWidget(sortBtn);
    layoutActions->addStretch(0);

    m_labelDocumentsHint = new QLabel(this);
    m_labelDocumentsHint->setStyleSheet("font: 12px; color: #787878");
    m_labelDocumentsHint->setMargin(5);
    layoutActions->addWidget(m_labelDocumentsHint);
    connect(m_category, SIGNAL(documentsHint(const QString&)), SLOT(on_documents_hintChanged(const QString&)));

    m_labelDocumentsCount = new QLabel(tr("0 articles"), this);
    m_labelDocumentsCount->setStyleSheet("font: 12px; color: #787878");
    m_labelDocumentsCount->setMargin(5);
    layoutActions->addWidget(m_labelDocumentsCount);
    connect(m_documents, SIGNAL(documentCountChanged()), SLOT(on_documents_documentCountChanged()));

    QWidget* line2 = new QWidget(this);
    line2->setFixedHeight(1);
    line2->setStyleSheet("border-top-width:1;border-top-style:solid;border-top-color:#d9dcdd");

    layoutList->addLayout(layoutActions);
    layoutList->addWidget(line2);
    layoutList->addWidget(m_documents);

    return m_noteList;
}

void MainWindow::on_documents_documentCountChanged()
{
    QString strCount = m_labelDocumentsCount->text().replace(QRegExp("\\d+"), QString::number(m_documents->count()));
    m_labelDocumentsCount->setText(strCount);
}

void MainWindow::on_documents_hintChanged(const QString& strHint)
{
    QFontMetrics fmx(font());
    QString strMsg = fmx.elidedText(strHint, Qt::ElideRight, 150);
    m_labelDocumentsHint->setText(strMsg);
}

void MainWindow::on_documents_viewTypeChanged(int type)
{
    m_documents->resetItemsViewType(type);
}

void MainWindow::on_documents_sortingTypeChanged(int type)
{
    m_documents->resetItemsSortingType(type);
}

//void MainWindow::on_document_contentChanged()
//{
//    m_doc->setModified(m_doc->web()->page()->isModified());
//}


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
    connect(m_category, SIGNAL(itemSelectionChanged()), SLOT(on_category_itemSelectionChanged()));
    connect(m_category, SIGNAL(newDocument()), SLOT(on_actionNewNote_triggered()));
    m_category->init();

    connect(m_msgList, SIGNAL(itemSelectionChanged()), SLOT(on_message_itemSelectionChanged()));
    connect(m_documents, SIGNAL(itemSelectionChanged()), SLOT(on_documents_itemSelectionChanged()));
}

void MainWindow::on_actionAutoSync_triggered()
{
    m_sync->startSync();
    m_animateSync->startPlay();
    m_syncTimer->stop();
}

void MainWindow::on_actionSync_triggered()
{
    m_sync->startSync(false);
    m_animateSync->startPlay();
    m_syncTimer->stop();
}

void MainWindow::on_syncLogined()
{
    // FIXME: show user notify message send from server
}

void MainWindow::on_syncDone(int nErrorCode, const QString& strErrorMsg)
{
    Q_UNUSED(strErrorMsg);

    m_animateSync->stopPlay();
    if (m_settings->syncInterval() != -1) {
        m_syncTimer->start();
    }

    // password changed
    if (nErrorCode == 301) {
        if (!m_userVerifyDialog) {
            m_userVerifyDialog = new CWizUserVerifyDialog(m_dbMgr.db().GetUserId(), tr("sorry, sync failed. please input your password and try again."), this);
            connect(m_userVerifyDialog, SIGNAL(accepted()), SLOT(on_syncDone_userVerified()));
        }

        m_userVerifyDialog->exec();
    }
}

void MainWindow::on_syncDone_userVerified()
{
    m_userVerifyDialog->deleteLater();

    if (m_dbMgr.db().SetPassword(m_userVerifyDialog->password())) {
        on_actionSync_triggered();
    }
}

void MainWindow::on_syncProcessLog(const QString& strMsg)
{
    Q_UNUSED(strMsg);
    m_statusBar->showText(tr("Syncing..."));
    //m_statusBar->showText(strMsg.left(40));
}

void MainWindow::on_actionNewNote_triggered()
{
    WIZDOCUMENTDATA data;
    m_category->createDocument(data);

    m_documentForEditing = data;
    m_documents->addAndSelectDocument(data);
}

void MainWindow::on_actionEditingUndo_triggered()
{
    m_doc->web()->page()->undoStack()->undo();
}

void MainWindow::on_actionEditingRedo_triggered()
{
    m_doc->web()->page()->undoStack()->redo();
}

void MainWindow::on_actionEditingCut_triggered()
{
    m_doc->web()->triggerPageAction(QWebPage::Cut);
}

void MainWindow::on_actionEditingCopy_triggered()
{
    m_doc->web()->triggerPageAction(QWebPage::Copy);
}

void MainWindow::on_actionEditingPaste_triggered()
{
    m_doc->web()->triggerPageAction(QWebPage::Paste);
}

void MainWindow::on_actionEditingPastePlain_triggered()
{
    qDebug() << "paste plain...";
}

void MainWindow::on_actionEditingDelete_triggered()
{
    qDebug() << "delete...";
}

void MainWindow::on_actionEditingSelectAll_triggered()
{
    m_doc->web()->triggerPageAction(QWebPage::SelectAll);
}

void MainWindow::on_actionViewToggleCategory_triggered()
{
    QWidget* category = m_splitter->widget(0);
    if (category->isVisible()) {
        category->hide();
    } else {
        category->show();
    }

    m_actions->toggleActionText(WIZACTION_GLOBAL_TOGGLE_CATEGORY);
}

void MainWindow::on_actionViewToggleFullscreen_triggered()
{
#ifdef Q_OS_MAC
    toggleFullScreenMode(this);
    m_actions->toggleActionText(WIZACTION_GLOBAL_TOGGLE_FULLSCREEN);
#endif // Q_OS_MAC
}

void MainWindow::on_actionFormatJustifyLeft_triggered()
{
    m_doc->web()->editorCommandExecuteJustifyLeft();
}

void MainWindow::on_actionFormatJustifyRight_triggered()
{
    m_doc->web()->editorCommandExecuteJustifyRight();
}

void MainWindow::on_actionFormatJustifyCenter_triggered()
{
    m_doc->web()->editorCommandExecuteJustifyCenter();
}

void MainWindow::on_actionFormatJustifyJustify_triggered()
{
    m_doc->web()->editorCommandExecuteJustifyJustify();
}

void MainWindow::on_actionFormatInsertOrderedList_triggered()
{
    m_doc->web()->editorCommandExecuteInsertOrderedList();
}

void MainWindow::on_actionFormatInsertUnorderedList_triggered()
{
    m_doc->web()->editorCommandExecuteInsertUnorderedList();
}

void MainWindow::on_actionFormatInsertTable_triggered()
{
    m_doc->web()->editorCommandExecuteTableInsert();
}

void MainWindow::on_actionFormatInsertLink_triggered()
{
    m_doc->web()->editorCommandExecuteLinkInsert();
}

void MainWindow::on_actionFormatForeColor_triggered()
{
    m_doc->web()->editorCommandExecuteForeColor();
}

void MainWindow::on_actionFormatBold_triggered()
{
    m_doc->web()->editorCommandExecuteBold();
}

void MainWindow::on_actionFormatItalic_triggered()
{
    m_doc->web()->editorCommandExecuteItalic();
}

void MainWindow::on_actionFormatUnderLine_triggered()
{
    m_doc->web()->editorCommandExecuteUnderLine();
}

void MainWindow::on_actionFormatStrikeThrough_triggered()
{
    m_doc->web()->editorCommandExecuteStrikeThrough();
}

void MainWindow::on_actionFormatInsertHorizontal_triggered()
{
    m_doc->web()->editorCommandExecuteInsertHorizontal();
}

void MainWindow::on_actionFormatInsertDate_triggered()
{
    m_doc->web()->editorCommandExecuteInsertDate();
}

void MainWindow::on_actionFormatInsertTime_triggered()
{
    m_doc->web()->editorCommandExecuteInsertTime();
}

void MainWindow::on_actionFormatIndent_triggered()
{
    m_doc->web()->editorCommandExecuteIndent();
}

void MainWindow::on_actionFormatOutdent_triggered()
{
    m_doc->web()->editorCommandExecuteOutdent();
}

void MainWindow::on_actionFormatRemoveFormat_triggered()
{
    m_doc->web()->editorCommandExecuteRemoveFormat();
}

void MainWindow::on_actionEditorViewSource_triggered()
{
    m_doc->web()->editorCommandExecuteViewSource();
}

void MainWindow::on_actionConsole_triggered()
{
    if (!m_console) {
      m_console = new CWizConsoleDialog(*this, window());
    }

    m_console->show();
}

void MainWindow::on_actionLogout_triggered()
{
    // save state
    m_settings->setAutoLogin(false);
    m_bLogoutRestart = true;
    on_actionExit_triggered();
}

void MainWindow::on_actionAbout_triggered()
{
    CWizAboutDialog dialog(this);
    dialog.exec();
}

#ifdef QT_DEBUG
void MainWindow::on_actionAboutPlugins_triggered()
{
    Core::Internal::PluginDialog dialog(this);
    dialog.exec();
}
#endif

void MainWindow::on_actionPreference_triggered()
{
    CWizPreferenceWindow preference(*this, this);

    connect(&preference, SIGNAL(settingsChanged(WizOptionsType)), SLOT(on_options_settingsChanged(WizOptionsType)));
    connect(&preference, SIGNAL(restartForSettings()), SLOT(on_options_restartForSettings()));
    preference.exec();
}

void MainWindow::on_actionFeedback_triggered()
{
    QString strUrl = WizService::ApiEntry::feedbackUrl();

    if (strUrl.isEmpty())
        return;

    QDesktopServices::openUrl(strUrl);
}

void MainWindow::on_actionRebuildFTS_triggered()
{
    QMessageBox msg;
    msg.setIcon(QMessageBox::Warning);
    msg.setWindowTitle(tr("Rebuild full text search index"));
    msg.addButton(QMessageBox::Ok);
    msg.addButton(QMessageBox::Cancel);
    msg.setText(tr("Rebuild full text search is quit slow if you have quite a few \
                   documents or attachments, you do not have to use this function \
                   while search should work as expected, this fuction is only used \
                   as developer's issue triage purpose, use it only if you know \
                   what you are doing!"));

    if (QMessageBox::Ok == msg.exec()) {
        m_searchIndexer->rebuild();
    }
}

void MainWindow::on_actionSearch_triggered()
{
    m_search->focus();
}

void MainWindow::on_actionResetSearch_triggered()
{
    if (m_searcher) {
        m_searcher->abort();
    }

    m_search->clear();
    m_search->focus();
    m_category->restoreSelection();
}

//void MainWindow::on_searchDocumentFind(const WIZDOCUMENTDATAEX& doc)
//{
//    m_documents->addDocument(doc, true);
//    on_documents_itemSelectionChanged();
//}

void MainWindow::on_search_doSearch(const QString& keywords)
{
    if (keywords.isEmpty()) {
        on_actionResetSearch_triggered();
        return;
    }

    if (m_searcher) {
        m_searcher->disconnect(this);
        m_searcher->abort();
        m_searchThread.quit();
    }

    m_category->saveSelection();
    m_documents->clear();

    if (!m_searchTimer) {
        m_searchTimer = new QTimer(this);
        m_searchTimer->setSingleShot(true);
        m_searchTimer->setInterval(300);
        connect(m_searchTimer, SIGNAL(timeout()), SLOT(on_search_timeout()));
    }

    m_strSearchKeywords = keywords;
    m_searchTimer->start();
}

void MainWindow::on_search_timeout()
{
    if (m_searcher) {
        m_searchTimer->start();
        return;
    }

    m_searcher = new CWizSearcher(m_dbMgr);
    connect(m_searcher, SIGNAL(searchProcess(const QString&, const CWizDocumentDataArray&, bool)),
            SLOT(on_searchProcess(const QString&, const CWizDocumentDataArray&, bool)));

    m_searcher->moveToThread(&m_searchThread);
    connect(&m_searchThread, SIGNAL(finished()), m_searcher, SLOT(deleteLater()));
    m_searchThread.start();

    m_searcher->search(m_strSearchKeywords, 10000);
}

void MainWindow::on_searchProcess(const QString& strKeywords, const CWizDocumentDataArray& arrayDocument, bool bEnd)
{
    Q_ASSERT(m_searcher);

    if (bEnd) {
        m_searchThread.exit();
    }

    if (strKeywords != m_strSearchKeywords) {
        return;
    }

    m_documents->addDocuments(arrayDocument);
    on_documents_itemSelectionChanged();
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

    CWizCategoryBaseView* category = qobject_cast<CWizCategoryBaseView *>(sender());
    if (!category)
        return;

    CWizCategoryViewMessageItem* pItem = category->currentCategoryItem<CWizCategoryViewMessageItem>();
    if (pItem) {
        m_msgList->show();
        m_noteList->hide();

        CWizMessageDataArray arrayMsg;
        pItem->getMessages(m_dbMgr.db(), arrayMsg);
        m_msgList->setMessages(arrayMsg);
        return;

    // FIXME: use id instead of name.
    //QString strName = category->currentItem()->text(0);
    //if (strName == CATEGORY_MESSAGES_ALL ||
    //        strName == CATEGORY_MESSAGES_SEND_TO_ME ||
    //        strName == CATEGORY_MESSAGES_MODIFY ||
    //        strName == CATEGORY_MESSAGES_COMMENTS ||
    //        strName == CATEGORY_MESSAGES_SEND_FROM_ME) {
    //    m_msgList->show();
    //    m_noteList->hide();

    //    CWizMessageDataArray arrayMsg;
    //    m_dbMgr.db().getLastestMessages(arrayMsg);
    //    m_msgList->setMessages(arrayMsg);

    //    return;
    } else {
        m_noteList->show();
        m_msgList->hide();
        QString kbGUID = category->selectedItemKbGUID();
        if (!kbGUID.isEmpty()) {
            resetPermission(kbGUID, "");
        }

        category->getDocuments(arrayDocument);
        m_documents->setDocuments(arrayDocument);

        if (arrayDocument.empty()) {
            on_documents_itemSelectionChanged();
        }
    }
}

void MainWindow::on_documents_itemSelectionChanged()
{
    // hide other form
    m_cipherForm->hide();

    CWizDocumentDataArray arrayDocument;
    m_documents->getSelectedDocuments(arrayDocument);

    if (arrayDocument.size() == 1) {
        if (!m_bUpdatingSelection) {
            viewDocument(arrayDocument[0], true);
        }
    }
}

void MainWindow::on_message_itemSelectionChanged()
{
    m_cipherForm->hide();

    QList<WIZMESSAGEDATA> listMsg;
    m_msgList->selectedMessages(listMsg);

    if (listMsg.size() == 1) {
        WIZMESSAGEDATA msg(listMsg[0]);
        WIZDOCUMENTDATA doc;
        if (!m_dbMgr.db(msg.kbGUID).DocumentFromGUID(msg.documentGUID, doc)) {
            qDebug() << "can't find note from message info: " << msg.title;
            return;
        }

        viewDocument(doc, true);
    }
}

void MainWindow::on_options_settingsChanged(WizOptionsType type)
{
    if (wizoptionsNoteView == type) {
        m_doc->settingsChanged();
    } else if (wizoptionsSync == type) {

        int nInterval = m_settings->syncInterval();
        if (nInterval == -1) {
            m_syncTimer->stop();
        } else {
            nInterval = nInterval < 5 ? 5 : nInterval;
            m_syncTimer->setInterval(nInterval * 60 * 1000);
        }

    } else if (wizoptionsFont == type) {
        m_doc->web()->editorResetFont();
    }
}

void MainWindow::on_options_restartForSettings()
{
    m_bRestart = true;
    on_actionExit_triggered();
}

void MainWindow::resetPermission(const QString& strKbGUID, const QString& strOwner)
{
    Q_ASSERT(!strKbGUID.isEmpty());

    int nPerm = m_dbMgr.db(strKbGUID).permission();
    bool isGroup = m_dbMgr.db().kbGUID() != strKbGUID;

    // Admin, Super, do anything
    if (nPerm == WIZ_USERGROUP_ADMIN || nPerm == WIZ_USERGROUP_SUPER) {
        // enable editing
        //m_doc->setReadOnly(false, isGroup);

        // enable create tag

        // enable new document
        m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT)->setEnabled(true);

        // enable delete document
        //m_actions->actionFromName("actionDeleteCurrentNote")->setEnabled(true);

    // Editor, only disable create tag
    } else if (nPerm == WIZ_USERGROUP_EDITOR) {
        //m_doc->setReadOnly(false, isGroup);
        m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT)->setEnabled(true);
        //m_actions->actionFromName("actionDeleteCurrentNote")->setEnabled(true);

    // Author
    } else if (nPerm == WIZ_USERGROUP_AUTHOR) {
        m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT)->setEnabled(true);

        // author is owner
        //QString strUserId = m_dbMgr.db().getUserId();
        //if (strOwner == strUserId) {
        //    m_doc->setReadOnly(false, isGroup);
        //    //m_actions->actionFromName("actionDeleteCurrentNote")->setEnabled(true);

        //// not owner
        //} else {
        //    m_doc->setReadOnly(true, isGroup);
        //    //m_actions->actionFromName("actionDeleteCurrentNote")->setEnabled(false);
        //}

    // reader
    } else if (nPerm == WIZ_USERGROUP_READER) {
        //m_doc->setReadOnly(true, isGroup);
        m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT)->setEnabled(false);
        //m_actions->actionFromName("actionDeleteCurrentNote")->setEnabled(false);
    } else {
        Q_ASSERT(0);
    }
}

void MainWindow::viewDocument(const WIZDOCUMENTDATA& data, bool addToHistory)
{
    Q_ASSERT(!data.strKbGUID.isEmpty());

    CWizDocument* doc = new CWizDocument(m_dbMgr.db(data.strKbGUID), data);

    if (doc->GUID() == m_doc->note().strGUID)
        return;

    //bool forceEdit = false;
    //if (doc->GUID() == m_documentForEditing.strGUID) {
    //    m_documentForEditing = WIZDOCUMENTDATA();
    //    forceEdit = true;
    //}

    resetPermission(data.strKbGUID, data.strOwner);

    ICore::emitViewNoteRequested(m_doc, data);

    //if (!m_doc->viewDocument(data, forceEdit))
    //    return;

    //if (addToHistory) {
    //    m_history->addHistory(data);
    //}

    //m_actions->actionFromName("actionGoBack")->setEnabled(m_history->canBack());
    //m_actions->actionFromName("actionGoForward")->setEnabled(m_history->canForward());
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

QObject* MainWindow::DocumentsCtrl()
{
    return m_documents;
}

QObject* MainWindow::DatabaseManager()
{
    return &m_dbMgr;
}

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

void MainWindow::SetSavingDocument(bool saving)
{
    //m_statusBar->setVisible(saving);
    if (saving) {
        //m_statusBar->setVisible(true);
        m_statusBar->showText(tr("Saving note..."));
        //qApp->processEvents(QEventLoop::AllEvents);
    } else {
        m_statusBar->hide();
    }
}

void MainWindow::ProcessClipboardBeforePaste(const QVariantMap& data)
{
    Q_UNUSED(data);
    // QVariantMap =  {html: text, textContent: text};
    //qDebug() << data.value("html").toString();
    //qDebug() << data.value("textContent").toString();

//    QClipboard* clipboard = QApplication::clipboard();
//
//#ifdef Q_OS_LINUX
//    // on X, copy action carry formats: ("TARGETS", "MULTIPLE", "text/html", "image/bmp", "SAVE_TARGETS", "TIMESTAMP", "application/x-qt-image")
//    // paste image copy from chromium or firefox will insert html to the end, we should remove all text if image exist
//    const QMimeData *mimeData = clipboard->mimeData();
//    if (mimeData->hasImage() && mimeData->hasHtml()) {
//        QImage image = clipboard->image();
//        clipboard->clear();
//        clipboard->setImage(image);
//    }
//#endif
//
//    if (!clipboard->image().isNull()) {
//        // save clipboard image to $TMPDIR
//        QString strTempPath = WizGlobal()->GetTempPath();
//        CString strFileName = strTempPath + WizIntToStr(GetTickCount()) + ".png";
//        if (!clipboard->image().save(strFileName)) {
//            TOLOG("ERROR: Can't save clipboard image to file");
//            return;
//        }
//
//        QString strHtml = QString("<img border=\"0\" src=\"file://%1\" />").arg(strFileName);
//        web()->editorCommandExecuteInsertHtml(strHtml, true);
//    }
}
