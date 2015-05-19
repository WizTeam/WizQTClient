#include "wizmainwindow.h"

#include <QToolBar>
#include <QMenuBar>
#include <QApplication>
#include <QDesktopWidget>
#include <QUndoStack>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QPushButton>
#include <QHostInfo>
#include <QSystemTrayIcon>
#include <QPrintDialog>
#include <QPrinter>
#include <QWebFrame>
#include <QCheckBox>

#ifdef Q_OS_MAC
#include <Carbon/Carbon.h>
#include "mac/wizmachelper.h"
#include "mac/wizmactoolbar.h"
#include "mac/wizSearchWidget_mm.h"
#else
#endif
#include "wizSearchWidget.h"
#include "share/wizMessageBox.h"

#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/aboutdialog.h>

#include "wizUpgrade.h"
#include "wizconsoledialog.h"
#include "wizCategoryView.h"
#include "wizDocumentListView.h"
#include "wizcertmanager.h"
#include "wizusercipherform.h"
#include "wizDocumentView.h"

#include "wizDocumentWebEngine.h"
#include "wizDocumentWebView.h"
#include "wizactions.h"
#include "wizpreferencedialog.h"
#include "wizupgradenotifydialog.h"

#include "share/wizcommonui.h"
#include "share/wizui.h"
#include "share/wizmisc.h"
#include "share/wizuihelper.h"
#include "share/wizsettings.h"
#include "share/wizanimateaction.h"
#include "share/wizSearchIndexer.h"
#include "share/wizObjectDataDownloader.h"
#include "utils/pathresolve.h"
#include "utils/stylehelper.h"
#include "utils/misc.h"
#include "widgets/wizFramelessWebDialog.h"
#include "widgets/wizScreenShotWidget.h"
#include "widgets/wizImageButton.h"
#include "widgets/wizIAPDialog.h"

#include "wiznotestyle.h"
#include "wizdocumenthistory.h"

#include "wizButton.h"

#include "wizEditorToolBar.h"
#include "wizProgressDialog.h"
#include "wizDocumentSelectionView.h"
#include "wizDocumentTransitionView.h"
#include "messagelistview.h"

#include "wizPopupButton.h"
#include "widgets/wizUserInfoWidget.h"
#include "sync/apientry.h"
#include "sync/wizkmsync.h"
#include "sync/avatar.h"
#include "sync/token.h"

#include "wizUserVerifyDialog.h"
#include "plugindialog.h"
#include "notecomments.h"
#include "wizMobileFileReceiver.h"
#include "wizDocTemplateDialog.h"
#include "share/wizFileMonitor.h"
#include "share/wizAnalyzer.h"
#include "share/wizTranslater.h"
#include "widgets/wizShareLinkDialog.h"

using namespace Core;
using namespace Core::Internal;
using namespace WizService::Internal;

static MainWindow* windowInstance = 0;

MainWindow::MainWindow(CWizDatabaseManager& dbMgr, QWidget *parent)
    : _baseClass(parent)
    , m_core(new ICore(this))
    , m_dbMgr(dbMgr)
    , m_progress(new CWizProgressDialog(this))
    , m_settings(new CWizUserSettings(dbMgr.db()))
    , m_sync(new CWizKMSyncThread(dbMgr.db(), this))
    , m_searchIndexer(new CWizSearchIndexer(m_dbMgr, this))
    , m_searcher(new CWizSearcher(m_dbMgr, this))
#ifndef BUILD4APPSTORE
    , m_upgrade(new CWizUpgrade(this))
#else
    , m_upgrade(0)
#endif
    //, m_certManager(new CWizCertManager(*this))
    , m_objectDownloaderHost(new CWizObjectDataDownloaderHost(dbMgr, this))
    //, m_avatarDownloaderHost(new CWizUserAvatarDownloaderHost(dbMgr.db().GetAvatarPath(), this))
    , m_transitionView(new CWizDocumentTransitionView(this))
    , m_iapDialog(0)
#ifndef Q_OS_MAC
    , m_labelNotice(NULL)
    , m_optionsAction(NULL)
#endif
    , m_menuBar(0)
#ifdef Q_OS_MAC
    #ifdef USECOCOATOOLBAR
    , m_toolBar(new CWizMacToolBar(this))
    #else
    , m_toolBar(new QToolBar(this))
    #endif
    , m_useSystemBasedStyle(true)
#else
    , m_toolBar(new QToolBar("Main", titleBar()))
    , m_menu(new QMenu(clientWidget()))
    , m_spacerForToolButtonAdjust(NULL)
    , m_useSystemBasedStyle(m_settings->useSystemBasedStyle())
#endif
    , m_actions(new CWizActions(*this, this))
    , m_category(new CWizCategoryView(*this, this))
    , m_documents(new CWizDocumentListView(*this, this))
    , m_noteListWidget(NULL)
    , m_msgList(new MessageListView(this))
    , m_documentSelection(new CWizDocumentSelectionView(*this, this))
    , m_doc(new CWizDocumentView(*this, this))
    , m_history(new CWizDocumentViewHistory())
    , m_animateSync(new CWizAnimateAction(*this, this))
    , m_bRestart(false)
    , m_bLogoutRestart(false)
    , m_bUpdatingSelection(false)
    , m_tray(NULL)
    , m_trayMenu(NULL)
    , m_mobileFileReceiver(nullptr)
{
#ifndef Q_OS_MAC
    clientLayout()->addWidget(m_toolBar);
    setWindowStyleForLinux(m_useSystemBasedStyle);
#endif
    windowInstance = this;
    //
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(on_application_aboutToQuit()));
    connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit())); // Qt bug: Qt5 bug
    qApp->installEventFilter(this);
#ifdef Q_OS_MAC
    installEventFilter(this);
#endif

    //CWizCloudPool::instance()->init(&m_dbMgr);

    connect(m_objectDownloaderHost, SIGNAL(downloadProgress(QString, int,int)),
            m_transitionView, SLOT(onDownloadProgressChanged(QString, int,int)));

    // search and full text search
    m_searchIndexer->start(QThread::IdlePriority);
    m_searcher->start(QThread::HighPriority);

    // syncing thread
    m_sync->setFullSyncInterval(userSettings().syncInterval());
    connect(m_sync, SIGNAL(processLog(const QString&)), SLOT(on_syncProcessLog(const QString&)));
    connect(m_sync, SIGNAL(promptMessageRequest(int, const QString&, const QString&)),
            SLOT(on_promptMessage_request(int, QString, QString)));
    connect(m_sync, SIGNAL(syncStarted(bool)), SLOT(on_syncStarted(bool)));
    connect(m_sync, SIGNAL(syncFinished(int, QString)), SLOT(on_syncDone(int, QString)));

    connect(m_searcher, SIGNAL(searchProcess(const QString&, const CWizDocumentDataArray&, bool, bool)),
        SLOT(on_searchProcess(const QString&, const CWizDocumentDataArray&, bool, bool)));

    connect(m_doc, SIGNAL(documentSaved(QString,CWizDocumentView*)), SIGNAL(documentSaved(QString,CWizDocumentView*)));
//    connect(m_doc->web(), SIGNAL(selectAllKeyPressed()), SLOT(on_actionEditingSelectAll_triggered()));
    connect(m_doc->web(), SIGNAL(shareDocumentByLinkRequest(QString,QString)),
            SLOT(on_shareDocumentByLink_request(QString,QString)));
    connect(this, SIGNAL(documentSaved(QString,CWizDocumentView*)),
            m_doc, SLOT(on_document_data_saved(QString,CWizDocumentView*)));

    // GUI
    initActions();
#ifdef Q_OS_MAC
    initMenuBar();
#else
    if (m_useSystemBasedStyle) {
        initMenuBar();
    } else {
        initMenuList();
    }
#endif
    initToolBar();
    initClient();

    setWindowTitle(tr("WizNote"));

    restoreStatus();

    client()->hide();

    // upgrade check
#ifndef BUILD4APPSTORE
    connect(m_upgrade, SIGNAL(checkFinished(bool)), SLOT(on_checkUpgrade_finished(bool)));
    if (userSettings().autoCheckUpdate()) {
        checkWizUpdate();
    }
#endif

#ifdef Q_OS_MAC
    setupFullScreenMode(this);
#endif

    WizService::NoteComments::init();
    //
    m_sync->start(QThread::IdlePriority);
    //
    setSystemTrayIconVisible(userSettings().showSystemTrayIcon());

    setMobileFileReceiverEnable(userSettings().receiveMobileFile());

    if (needShowNewFeatureGuide())
    {
        m_settings->setNewFeatureGuideVersion(WIZ_NEW_FEATURE_GUIDE_VERSION);
        QTimer::singleShot(3000, this, SLOT(showNewFeatureGuide()));
    }
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    // Qt issue: issue? User quit for mac dock send close event to qApp?
    // I was throught close event only send to widget.
    if (watched == qApp) {
        if (event->type() == QEvent::Close)
        {
            qApp->quit();
            return true;

        }
        else if (event->type() == QEvent::FileOpen)
        {
            if (QFileOpenEvent* fileEvent = dynamic_cast<QFileOpenEvent*>(event))
            {
                if (!fileEvent->url().isEmpty())
                {
                    QString strUrl = fileEvent->url().toString();
                    if (strUrl.left(5) == "file:")
                    {
                        strUrl.remove(0, 5);
                        strUrl.replace("open_document%3F", "open_document?");
                    }

                    if (IsWizKMURL(strUrl))
                    {
                        viewDocumentByWizKMURL(strUrl);
                        return true;
                    }
                }
            }
        }
        else
        {
            return false;
        }
    }
#ifdef Q_OS_MAC
    else if (watched == this)
    {
        if (event->type() == QEvent::WindowStateChange)
        {
            if (QWindowStateChangeEvent* stateEvent = dynamic_cast<QWindowStateChangeEvent*>(event))
            {
                // 使用程序右上角按钮将窗口最大化时，需要修改按钮名称
                static int state = -1;
                int oldState = stateEvent->oldState();
                if (state != oldState && (oldState == Qt::WindowFullScreen || windowState() == Qt::WindowFullScreen))
                {
                    state = oldState;
                    m_actions->toggleActionText(WIZACTION_GLOBAL_TOGGLE_FULLSCREEN);
                }
            }
        }
    }
#endif

    //
    return _baseClass::eventFilter(watched, event);
}

void MainWindow::on_application_aboutToQuit()
{
    cleanOnQuit();
}


void MainWindow::cleanOnQuit()
{
    m_category->saveExpandState();
    saveStatus();
    //
    m_sync->waitForDone();
    //
    m_searchIndexer->waitForDone();
    m_searcher->waitForDone();
    //
    m_doc->waitForDone();
    //
    QThreadPool::globalInstance()->waitForDone();
    WizService::AvatarHost::waitForDone();

    if (m_mobileFileReceiver)
    {
        m_mobileFileReceiver->waitForDone();
    }
}

CWizSearcher*MainWindow::searcher()
{    
    return m_searcher;
}

void MainWindow::rebuildFTS()
{
    m_searchIndexer->rebuild();
}

MainWindow*MainWindow::instance()
{
    return windowInstance;
}

QNetworkDiskCache*MainWindow::webViewNetworkCache()
{
    return 0;
//    return m_doc->web()->networkCache();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
#ifdef Q_OS_MAC
    if (event->spontaneous())
    {
        wizMacHideCurrentApplication();
        event->ignore();
        return;
    }
#else
    if (m_settings->showSystemTrayIcon())
    {
        setVisible(false);
        event->ignore();
    }
#endif
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    if (m_useSystemBasedStyle)
        QMainWindow::mousePressEvent(event);
    else
        _baseClass::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (m_useSystemBasedStyle)
        QMainWindow::mouseMoveEvent(event);
    else
        _baseClass::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_useSystemBasedStyle)
        QMainWindow::mouseReleaseEvent(event);
    else
        _baseClass::mouseReleaseEvent(event);
}

void MainWindow::changeEvent(QEvent* event)
{
    if (m_useSystemBasedStyle)
        QMainWindow::changeEvent(event);
    else
        _baseClass::changeEvent(event);
}

#ifdef USECOCOATOOLBAR
void MainWindow::showEvent(QShowEvent* event)
{
    m_toolBar->showInWindow(this);
    QMainWindow::showEvent(event);
}
#endif

void MainWindow::on_actionExit_triggered()
{
    WizGetAnalyzer().LogAction("exit");

    qApp->exit();
}

void MainWindow::on_actionClose_triggered()
{
    WizGetAnalyzer().LogAction("close");

#ifdef Q_OS_MAC
    QWidget* wgt = qApp->activeWindow();
    if (wgt && wgt != this)
    {
       wgt->close();
    }
    else
    {
        wizMacHideCurrentApplication();
//        setVisible(false);
    }
#else
    QWidget* wgt = qApp->activeWindow();
    if (wgt && wgt != this)
    {
       wgt->close();
    }
    else
    {
        if (m_settings->showSystemTrayIcon())
        {
            setVisible(false);
        }
    }

#endif
}

//void MainWindow::resizeEvent(QResizeEvent *event)
//{
//    Q_UNUSED(event);
//    updateHistoryButtonStatus();
//}

void MainWindow::on_checkUpgrade_finished(bool bUpgradeAvaliable)
{
    if (!bUpgradeAvaliable)
        return;

    QString strUrl = m_upgrade->getWhatsNewUrl();
    CWizUpgradeNotifyDialog notifyDialog(strUrl, this);
    if (QDialog::Accepted == notifyDialog.exec()) {
        QString url = WizService::ApiEntry::standardCommandUrl("link", true);
#if defined(Q_OS_MAC)
        url += "&name=wiznote-mac.html";
#elif defined(Q_OS_LINUX)
        url += "&name=wiznote-linux.html";
#else
        Q_ASSERT(0);
#endif
        QDesktopServices::openUrl(QUrl(url));
    }
}

void MainWindow::on_TokenAcquired(const QString& strToken)
{
    WizService::Token::instance()->disconnect(this);

    if (strToken.isEmpty())
    {
        int nErrorCode = WizService::Token::lastErrorCode();
        // network unavailable
        if (QNetworkReply::ProtocolUnknownError == nErrorCode)
        {
            QMessageBox::critical(this, tr("Info"), tr("Connection is not available, please check your network connection."));
        }
        else if (errorTokenInvalid == nErrorCode)
        {
            //try to relogin wiz server, but failed. may be password error
            m_settings->setPassword("");
            if (!m_userVerifyDialog)
            {
                m_userVerifyDialog = new CWizUserVerifyDialog(m_dbMgr.db().GetUserId(), tr("sorry, sync failed. please input your password and try again."), this);
                connect(m_userVerifyDialog, SIGNAL(accepted()), SLOT(on_syncDone_userVerified()));
            }

            m_userVerifyDialog->exec();
        }
    }

}

void MainWindow::on_quickSync_request(const QString& strKbGUID)
{
    CWizKMSyncThread::quickSyncKb(strKbGUID);
}

void MainWindow::setSystemTrayIconVisible(bool bVisible)
{
    //FIXME: There is a bug. Must delete trayicon at hide, otherwise will crash when show it again.
//    if (bVisible)
//    {
//        //
//        if (m_tray)
//        {
//            if (m_tray->isVisible())
//                return;

//            delete m_tray;
//        }

//        //
    if (!m_tray)
    {
        m_tray = new QSystemTrayIcon(QApplication::windowIcon(), this);
        initTrayIcon(m_tray);
        m_tray->show();
    }
//    }
//    else
//    {
//        if (m_tray)
//        {
//            m_tray->hide();
//            delete m_tray;
//            m_tray = 0;
//        }
//    }
    m_tray->setVisible(bVisible);
}

void MainWindow::showTrayIconMessage(const QString& strTitle, const QString& strInfo)
{
    if (m_tray && m_tray->isVisible())
    {
        m_tray->showMessage(strTitle, strInfo, QSystemTrayIcon::Information);
    }
}

void MainWindow::showTrayIconMenu()
{
    if (m_trayMenu)
    {
        m_trayMenu->popup(QCursor::pos());
    }
}

void MainWindow::on_trayIcon_newDocument_clicked()
{
    setVisible(true);
    QApplication::setActiveWindow(this);
    raise();

    on_actionNewNote_triggered();
}

void MainWindow::on_hideTrayIcon_clicked()
{
    setSystemTrayIconVisible(false);
    userSettings().setShowSystemTrayIcon(false);
}

void MainWindow::on_trayIcon_actived(QSystemTrayIcon::ActivationReason reason)
{
    static QTimer trayTimer;
    trayTimer.setSingleShot(true);
    connect(&trayTimer, SIGNAL(timeout()), SLOT(showTrayIconMenu()), Qt::UniqueConnection);
    switch (reason) {
    case QSystemTrayIcon::DoubleClick:
    {
        trayTimer.stop();
        qDebug() << "trayicon double clicked";        
    }
        break;
    case QSystemTrayIcon::Trigger:
    {
        trayTimer.stop();
        trayTimer.start(400);
        qDebug() << "trayicon triggered";
    }
        break;
    default:
        break;
    }
}

void MainWindow::shiftVisableStatus()
{
#ifdef Q_OS_MAC
    bool appVisible = wizMacIsCurrentApplicationVisible();
    //
    if (appVisible && QApplication::activeWindow() != this)
    {
        raise();
        return;
    }
    //
    if (appVisible)
    {
        wizMacHideCurrentApplication();
    }
    else
    {
        wizMacShowCurrentApplication();
        // wait for process finished
        QCoreApplication::processEvents(QEventLoop::AllEvents, 200);
        raise();
    }
    //

#else
//    setVisible(!isVisible());
//    if (isVisible())
//    {
//        raise();
//    }
    if (Qt::WindowMinimized & windowState())
    {
        setWindowState(Qt::WindowActive);
        raise();
        showNormal();
    }
    else if (!isActiveWindow())
    {
        setWindowState(Qt::WindowActive);
        raise();
        showNormal();
    }
    else
    {
        setWindowState(Qt::WindowMinimized);
    }
#endif

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
#ifdef Q_OS_LINUX
    m_actions->init(!m_useSystemBasedStyle);
#else
    m_actions->init();
#endif
    m_animateSync->setAction(m_actions->actionFromName(WIZACTION_GLOBAL_SYNC));
    m_animateSync->setSingleIcons("sync");

    connect(m_doc->web(), SIGNAL(statusChanged()), SLOT(on_editor_statusChanged()));
//#ifndef USEWEBENGINE
//    connect(m_doc->web()->page(), SIGNAL(contentsChanged()), SLOT(on_document_contentChanged()));
//    connect(m_doc->web()->page(), SIGNAL(selectionChanged()), SLOT(on_document_contentChanged()));
//#endif

    on_editor_statusChanged();
}


void MainWindow::initMenuBar()
{
    m_menuBar = new QMenuBar(this);
    setMenuBar(m_menuBar);
    m_actions->buildMenuBar(m_menuBar, Utils::PathResolve::resourcesPath() + "files/mainmenu.ini");
}

void MainWindow::on_editor_statusChanged()
{
#ifdef USEWEBENGINE
    CWizDocumentWebEngine* editor = m_doc->web();
#else
    CWizDocumentWebView* editor = m_doc->web();
#endif

    if (!editor->isInited() || !editor->hasFocus()) {
        m_actions->actionFromName(WIZACTION_EDITOR_UNDO)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_REDO)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_CUT)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_COPY)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_PASTE)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_PASTE_PLAIN)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_DELETE)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_FIND_REPLACE)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_SELECT_ALL)->setEnabled(false);

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
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_CHECKLIST)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_CODE)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_IMAGE)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_REMOVE_FORMAT)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_PLAINTEXT)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_VIEW_SOURCE)->setEnabled(false);

        return;
    }

    m_actions->actionFromName(WIZACTION_EDITOR_FIND_REPLACE)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_EDITOR_SELECT_ALL)->setEnabled(true);

    m_actions->actionFromName(WIZACTION_GLOBAL_SAVE_AS_PDF)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_GLOBAL_SAVE_AS_HTML)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_GLOBAL_PRINT)->setEnabled(true);

#ifdef USEWEBENGINE
    editor->editorCommandQueryCommandState("undo", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_EDITOR_UNDO)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_EDITOR_UNDO)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("redo", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_EDITOR_REDO)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_EDITOR_REDO)->setEnabled(true);
        }
    });

    if (!editor->isEditing()) {
        m_actions->actionFromName(WIZACTION_EDITOR_CUT)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_PASTE)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_PASTE_PLAIN)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_EDITOR_DELETE)->setEnabled(false);

        if (!editor->page()->selectedText().isEmpty()) {
            m_actions->actionFromName(WIZACTION_EDITOR_COPY)->setEnabled(true);
        } else {
            m_actions->actionFromName(WIZACTION_EDITOR_COPY)->setEnabled(false);
        }
    } else {
        m_actions->actionFromName(WIZACTION_EDITOR_PASTE)->setEnabled(true);
        m_actions->actionFromName(WIZACTION_EDITOR_PASTE_PLAIN)->setEnabled(true);

        if (!editor->page()->selectedText().isEmpty()) {
            m_actions->actionFromName(WIZACTION_EDITOR_CUT)->setEnabled(true);
            m_actions->actionFromName(WIZACTION_EDITOR_COPY)->setEnabled(true);
            m_actions->actionFromName(WIZACTION_EDITOR_DELETE)->setEnabled(true);
        } else {
            m_actions->actionFromName(WIZACTION_EDITOR_CUT)->setEnabled(false);
            m_actions->actionFromName(WIZACTION_EDITOR_COPY)->setEnabled(false);
            m_actions->actionFromName(WIZACTION_EDITOR_DELETE)->setEnabled(false);
        }
    }

    editor->editorCommandQueryCommandState("bold", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_BOLD)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_BOLD)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("italic", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_ITALIC)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_ITALIC)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("underline", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_UNDERLINE)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_UNDERLINE)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("strikethrough", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_STRIKETHROUGH)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_STRIKETHROUGH)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("insertUnorderedList", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_UNORDEREDLIST)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_UNORDEREDLIST)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("insertOrderedList", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_ORDEREDLIST)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_ORDEREDLIST)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("justify", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
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
    });

    editor->editorCommandQueryCommandState("indent", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_INDENT)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_INDENT)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("outdent", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_OUTDENT)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_OUTDENT)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("inserttable", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_TABLE)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_TABLE)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("link", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_LINK)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_LINK)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("horizontal", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_HORIZONTAL)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_HORIZONTAL)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("date", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_DATE)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_DATE)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("time", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_TIME)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_TIME)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("removeformat", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_REMOVE_FORMAT)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_REMOVE_FORMAT)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("plaintext", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_PLAINTEXT)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_PLAINTEXT)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("source", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_VIEW_SOURCE)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_VIEW_SOURCE)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("checklist", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_CHECKLIST)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_CHECKLIST)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("insertCode", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_CODE)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_CODE)->setEnabled(true);
        }
    });

    editor->editorCommandQueryCommandState("insertImage", [this](const QVariant& returnValue) {
        if (-1 == returnValue.toInt()) {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_IMAGE)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_IMAGE)->setEnabled(true);
        }
    });

#else
        if (editor->editorCommandQueryCommandState("undo") == -1) {
            m_actions->actionFromName(WIZACTION_EDITOR_UNDO)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_EDITOR_UNDO)->setEnabled(true);
        }

        if (editor->editorCommandQueryCommandState("redo") == -1) {
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

        if (-1 ==editor->editorCommandQueryCommandState("plaintext")) {
            m_actions->actionFromName(WIZACTION_FORMAT_PLAINTEXT)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_PLAINTEXT)->setEnabled(true);
        }

        if (-1 ==editor->editorCommandQueryCommandState("source")) {
            m_actions->actionFromName(WIZACTION_FORMAT_VIEW_SOURCE)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_VIEW_SOURCE)->setEnabled(true);
        }

        if (-1 ==editor->editorCommandQueryCommandState("checklist")) {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_CHECKLIST)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_CHECKLIST)->setEnabled(true);
        }

        if (-1 ==editor->editorCommandQueryCommandState("insertCode")) {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_CODE)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_CODE)->setEnabled(true);
        }

        if (-1 ==editor->editorCommandQueryCommandState("insertImage")) {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_IMAGE)->setEnabled(false);
        } else {
            m_actions->actionFromName(WIZACTION_FORMAT_INSERT_IMAGE)->setEnabled(true);
        }
#endif

}

void MainWindow::createDocumentByTemplate(const QString& strFile)
{
    initVariableBeforCreateNote();
    WIZDOCUMENTDATA data;
    if (!m_category->createDocumentByTemplate(data, strFile))
    {
        return;
    }

    setFocusForNewNote(data);
}

void MainWindow::on_mobileFileRecived(const QString& strFile)
{
    //目前只支持在有编辑状态的笔记时插入图片，其他时候删除掉接受到的图片
    /*
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Info"));
    msgBox.setText(tr("Mobile file received : ") + strFile);
    QAbstractButton *ignoreButton = msgBox.addButton(tr("Do nothing"), QMessageBox::ActionRole);
    QAbstractButton *delButton = msgBox.addButton(tr("Delete file"), QMessageBox::ActionRole);
    QAbstractButton *newNoteButton = msgBox.addButton(tr("Create new note whith the file"), QMessageBox::ActionRole);
    QAbstractButton *insertButton = msgBox.addButton(tr("Insert into current note"), QMessageBox::ActionRole);

    QImageReader imageReader(strFile);
    bool isImageFile = imageReader.canRead();

    msgBox.exec();
   // Q_UNUSED(ignoreButton);
    if (msgBox.clickedButton() == delButton)
    {
        QFile::remove(strFile);
    }
    else if (msgBox.clickedButton() == newNoteButton)
    {
        if (isImageFile)
        {
            createNoteWithImage(strFile);
        }
        else
        {
            createNoteWithAttachments(QStringList(strFile));
        }
    }
    else if (msgBox.clickedButton() == insertButton && m_doc->web()->isEditing())
    {
        if (isImageFile)
        {
            QString strHtml;
            if (WizImage2Html(strFile, strHtml))
            {
                m_doc->web()->editorCommandExecuteInsertHtml(strHtml, false);
            }
        }
        else
        {
            const WIZDOCUMENTDATA& doc = m_doc->note();
            CWizDatabase& db = m_dbMgr.db(doc.strKbGUID);
            WIZDOCUMENTATTACHMENTDATA attach;
            db.AddAttachment(doc, strFile, attach);
        }
    }
    */
    if (m_doc->web()->isEditing())
    {
        QImageReader imageReader(strFile);
        bool isImageFile = imageReader.canRead();

        if (isImageFile)
        {
            QString strHtml;
            if (WizImage2Html(strFile, strHtml))
            {
                m_doc->web()->editorCommandExecuteInsertHtml(strHtml, false);
            }
        }
        else
        {
            const WIZDOCUMENTDATA& doc = m_doc->note();
            CWizDatabase& db = m_dbMgr.db(doc.strKbGUID);
            WIZDOCUMENTATTACHMENTDATA attach;
            db.AddAttachment(doc, strFile, attach);
        }
    }
    else
    {
        QFile::remove(strFile);
    }
}

void MainWindow::on_shareDocumentByLink_request(const QString& strKbGUID, const QString& strGUID)
{
    if (!m_dbMgr.db().IsVip())
    {
        openVipPageInWebBrowser();
        return;
    }

    WIZDOCUMENTDATA doc;
    if (!m_dbMgr.db(strKbGUID).DocumentFromGUID(strGUID, doc))
    {
        qDebug() << "[ShareLink] can not find doc " << strGUID;
        return;
    }

    if (doc.nProtected == 1)
    {
        CWizMessageBox::information(this, tr("Info"), tr("Can not share encrpyted notes."));
        return;
    }

    CWizShareLinkDialog dlg(userSettings());
    dlg.shareDocument(doc);
    dlg.exec();
}

void MainWindow::openVipPageInWebBrowser()
{
    QMessageBox msg(this);
    msg.setWindowTitle(tr("Upgrading to VIP"));
    msg.setIcon(QMessageBox::Information);
    msg.setText(tr("Only VIP user can create link, please retry after upgrading to VIP and syncing to server."));
    msg.addButton(tr("Cancel"), QMessageBox::NoRole);
    QPushButton *actionBuy = msg.addButton(tr("Upgrade now"), QMessageBox::YesRole);
    msg.setDefaultButton(actionBuy);
    msg.exec();

    if (msg.clickedButton() == actionBuy)
    {
#ifndef BUILD4APPSTORE
        QString strToken = WizService::Token::token();
        QString strUrl = WizService::ApiEntry::standardCommandUrl("vip", strToken);
        QDesktopServices::openUrl(QUrl(strUrl));
#else
    CWizIAPDialog* dlg = iapDialog();
    dlg->loadIAPPage();
    dlg->exec();
#endif
    }
}

bool MainWindow::checkListClickable()
{
    if (!m_dbMgr.db(m_doc->note().strKbGUID).IsGroup())
    {
        emit clickingTodoCallBack(false, false);
        return true;
    }

    if (m_doc->checkListClickable())
    {
        emit clickingTodoCallBack(false, false);
        m_doc->setStatusToEditingByCheckList();
        return true;
    }
    emit clickingTodoCallBack(true, true);
    return false;
}

/** web页面调用该方法，打开URL
  * @brief MainWindow::OpenURLInDefaultBrowser
 * @param strUrl
 */
void MainWindow::OpenURLInDefaultBrowser(const QString& strUrl)
{
    m_doc->web()->onEditorLinkClicked(strUrl);
}

/** web页面调用该方法，token失效时重新获取token
 * @brief MainWindow::GetToken
 * @param strFunctionName
 */
void MainWindow::GetToken(const QString& strFunctionName)
{
    QString strToken = WizService::Token::token();
    QString strExec = strFunctionName + QString("('%1')").arg(strToken);
    qDebug() << "cpp get token callled : " << strExec;
    m_doc->commentView()->page()->mainFrame()->evaluateJavaScript(strExec);
}

/**   web页面调用该方法，将页面的结果返回
 * @brief MainWindow::SetDialogResult
 * @param result  web页面返回结果，如需更新笔记数据，会返回1
 */
void MainWindow::SetDialogResult(int nResult)
{
    if (nResult > 0)
    {
        //
        const WIZDOCUMENTDATA& doc = m_doc->note();
        if (doc.strKbGUID.isEmpty())
            return;

        m_dbMgr.db(doc.strKbGUID).SetObjectDataDownloaded(doc.strGUID, _T("document"), false);
        m_doc->viewNote(doc, false);
    }
}

void MainWindow::AppStoreIAP()
{
#ifdef BUILD4APPSTORE
    CWizIAPDialog* dlg = iapDialog();
    dlg->loadIAPPage();
    if (!dlg->isActiveWindow())
    {
        dlg->exec();
    }
#endif
}

#ifndef Q_OS_MAC
void MainWindow::layoutTitleBar()
{
    CWizTitleBar* title = titleBar();
    title->titleLabel()->setVisible(false);
    //
    //
    QLayout* layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    //
    QLayout* layoutTitle = new QHBoxLayout();
    layoutTitle->setContentsMargins(0, 0, 0, 0);
    //
    QLayout* layoutTitleBar = new QHBoxLayout();
    layoutTitleBar->setContentsMargins(10, 10, 10, 10);
    layoutTitleBar->addWidget(m_toolBar);
    layoutTitle->addItem(layoutTitleBar);
    //
    QVBoxLayout* layoutRight = new QVBoxLayout();
    layoutTitle->addItem(layoutRight);
    //
    QLayout* layoutBox = new QHBoxLayout();
    layoutBox->setContentsMargins(0, 6, 6, 0);
    layoutBox->setSpacing(6);
    layoutRight->addItem(layoutBox);
    //
    m_menuButton = new QToolButton(this);
    connect(m_menuButton, SIGNAL(clicked()), SLOT(on_menuButtonClicked()));
    layoutBox->addWidget(m_menuButton);
    layoutBox->addWidget(title->minButton());
    layoutBox->addWidget(title->maxButton());
    layoutBox->addWidget(title->closeButton());

    QString themeName = Utils::StyleHelper::themeName();
    QString strButtonMenu = ::WizGetSkinResourceFileName(themeName, "linuxwindowmenu");
    QString strButtonMenuOn = ::WizGetSkinResourceFileName(themeName, "linuxwindowmenu_on");
    QString strButtonMenuSelected = ::WizGetSkinResourceFileName(themeName, "linuxwindowmenu_selected");

    m_menuButton->setStyleSheet(QString("QToolButton{ border-image:url(%1);}"
                                   "QToolButton:hover{border-image:url(%2); background:none;}"
                                   "QToolButton::pressed{border-image:url(%3); background:none;}")
                           .arg(strButtonMenu).arg(strButtonMenuOn).arg(strButtonMenuSelected));
    m_menuButton->setFixedSize(16, 16);
    if (m_settings->useSystemBasedStyle())
        m_menuButton->setVisible(false);
    //
    layoutRight->addStretch();
    //
    QLabel* label = new QLabel(this);
    label->setFixedHeight(1);
    label->setStyleSheet(QString("QLabel{background-color:#aeaeae; border: none;}"));

    layout->addItem(layoutTitle);
    layout->addWidget(label);
    title->setLayout(layout);
}

void MainWindow::initMenuList()
{
    m_actions->buildMenu(m_menu, Utils::PathResolve::resourcesPath() + "files/mainmenu.ini");
}

#endif

void MainWindow::initToolBar()
{
#ifdef Q_OS_MAC
    #ifdef USECOCOATOOLBAR
    m_toolBar->showInWindow(this);

    CWizUserInfoWidget* info = new CWizUserInfoWidget(*this, m_toolBar);
    m_toolBar->addWidget(info, "", "");

    m_toolBar->addStandardItem(CWizMacToolBar::Space);
    m_toolBar->addAction(m_actions->actionFromName(WIZACTION_GLOBAL_SYNC));
    m_toolBar->addAction(m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT));
    m_spacerForToolButtonAdjust = new CWizMacFixedSpacer(QSize(120, 1), m_toolBar);
    m_toolBar->addWidget(m_spacerForToolButtonAdjust, "", "");
    m_toolBar->addAction(m_actions->actionFromName(WIZACTION_GLOBAL_GOBACK));
    m_toolBar->addAction(m_actions->actionFromName(WIZACTION_GLOBAL_GOFORWARD));
    m_toolBar->addStandardItem(CWizMacToolBar::FlexibleSpace);
    m_toolBar->addSearch(tr("Search"), "");
    //
    m_search = m_toolBar->getSearchWidget();
    #else
    setUnifiedTitleAndToolBarOnMac(true);
    setContextMenuPolicy(Qt::NoContextMenu);
    addToolBar(m_toolBar);
    m_toolBar->setAllowedAreas(Qt::TopToolBarArea);
    m_toolBar->setMovable(false);

    m_toolBar->addWidget(new CWizFixedSpacer(QSize(10, 1), m_toolBar));

    CWizUserInfoWidget* info = new CWizUserInfoWidget(*this, m_toolBar);
    m_toolBar->addWidget(info);

    m_toolBar->addWidget(new CWizFixedSpacer(QSize(20, 1), m_toolBar));

    m_toolBar->addAction(m_actions->actionFromName(WIZACTION_GLOBAL_SYNC));
    m_toolBar->addAction(m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT));

    //m_toolBar->addWidget(new CWizSpacer(m_toolBar));
    m_spacerForToolButtonAdjust = new CWizFixedSpacer(QSize(20, 1), m_toolBar);
    m_toolBar->addWidget(m_spacerForToolButtonAdjust);

    m_toolBar->addAction(m_actions->actionFromName(WIZACTION_GLOBAL_GOBACK));
    m_toolBar->addAction(m_actions->actionFromName(WIZACTION_GLOBAL_GOFORWARD));
    updateHistoryButtonStatus();

    m_toolBar->addWidget(new CWizSpacer(m_toolBar));

    m_searchWidget = new CWizSearchWidget(this);
    m_searchWidget->setWidthHint(280);
    m_toolBar->addWidget(m_searchWidget);

    m_toolBar->addWidget(new CWizFixedSpacer(QSize(20, 1), m_toolBar));
    #endif
#else
    layoutTitleBar();
    //
    m_toolBar->setIconSize(QSize(24, 24));
    m_toolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    m_toolBar->setMovable(false);
    m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // align with categoryview's root item.
    m_toolBar->addWidget(new CWizFixedSpacer(QSize(3, 1), m_toolBar));

    CWizUserInfoWidget* info = new CWizUserInfoWidget(*this, m_toolBar);
    m_toolBar->addWidget(info);

    m_toolBar->addWidget(new CWizFixedSpacer(QSize(20, 1), m_toolBar));

    CWizButton* buttonSync = new CWizButton(m_toolBar);
    buttonSync->setAction(m_actions->actionFromName(WIZACTION_GLOBAL_SYNC));
    m_toolBar->addWidget(buttonSync);

    m_spacerForToolButtonAdjust = new CWizFixedSpacer(QSize(20, 1), m_toolBar);
    m_toolBar->addWidget(m_spacerForToolButtonAdjust);

    m_search = new CWizSearchWidget(this);

    m_toolBar->addWidget(m_search);

    m_toolBar->layout()->setAlignment(m_search, Qt::AlignBottom);
    m_toolBar->addWidget(new CWizFixedSpacer(QSize(20, 1), m_toolBar));

    CWizButton* buttonNew = new CWizButton(m_toolBar);
    buttonNew->setAction(m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT));
    m_toolBar->addWidget(buttonNew);

    CWizButton* buttonBack = new CWizButton(m_toolBar);
    buttonBack->setAction(m_actions->actionFromName(WIZACTION_GLOBAL_GOBACK));
    m_toolBar->addWidget(buttonBack);

    CWizButton* buttonForward = new CWizButton(m_toolBar);
    buttonForward->setAction(m_actions->actionFromName(WIZACTION_GLOBAL_GOFORWARD));
    m_toolBar->addWidget(buttonForward);

    updateHistoryButtonStatus();

    m_toolBar->addWidget(new CWizSpacer(m_toolBar));

    //
#endif
    //
    connect(m_searchWidget, SIGNAL(doSearch(const QString&)), SLOT(on_search_doSearch(const QString&)));
    connect(m_searchWidget, SIGNAL(advancedSearchRequest()), SLOT(on_actionAdvancedSearch_triggered()));
    connect(m_searchWidget, SIGNAL(addCustomSearchRequest()), SLOT(on_actionAddCustomSearch_triggered()));
}

void MainWindow::initClient()
{
#ifdef Q_OS_MAC
    QWidget* client = new QWidget(this);
    setCentralWidget(client);
#else
    setCentralWidget(rootWidget());
    //
    QWidget* main = clientWidget();
    //
    QWidget* client = new QWidget(main);
    clientLayout()->addWidget(client);
#endif

    client->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QPalette pal = client->palette();
    pal.setBrush(QPalette::Window, QBrush("#FFFFFF"));
    client->setPalette(pal);
    client->setAutoFillBackground(true);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    client->setLayout(layout);

    m_splitter = std::make_shared<CWizSplitter>();
    layout->addWidget(m_splitter.get());

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

    m_docListContainer = new QWidget(this);
    QHBoxLayout* layoutList = new QHBoxLayout();
    layoutList->setContentsMargins(0, 0, 0, 0);
    layoutList->setSpacing(0);
    layoutList->addWidget(createNoteListView());
    layoutList->addWidget(createMessageListView());
    m_docListContainer->setLayout(layoutList);
    m_splitter->addWidget(m_docListContainer);
    m_splitter->addWidget(documentPanel);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 0);
    m_splitter->setStretchFactor(2, 1);

    m_msgListWidget->hide();
    //
    connect(m_splitter.get(), SIGNAL(splitterMoved(int, int)), SLOT(on_client_splitterMoved(int, int)));
}

QWidget* MainWindow::createNoteListView()
{
    m_noteListWidget = new QWidget(this);
    m_noteListWidget->setMinimumWidth(100);
    QVBoxLayout* layoutList = new QVBoxLayout();
    layoutList->setContentsMargins(0, 0, 0, 0);
    layoutList->setSpacing(0);
    m_noteListWidget->setLayout(layoutList);

    QHBoxLayout* layoutActions = new QHBoxLayout();
    layoutActions->setContentsMargins(0, 0, 0, 0);
    layoutList->setSpacing(0);

    CWizViewTypePopupButton* viewBtn = new CWizViewTypePopupButton(*this, this);
    viewBtn->setFixedHeight(Utils::StyleHelper::listViewSortControlWidgetHeight());
    connect(viewBtn, SIGNAL(viewTypeChanged(int)), SLOT(on_documents_viewTypeChanged(int)));
    layoutActions->addWidget(viewBtn);
    QWidget* line = new QWidget(this);
    line->setFixedWidth(1);
    line->setStyleSheet("border-left-width:1;border-left-style:solid;border-left-color:#DADAD9");
    layoutActions->addWidget(line);
    CWizSortingPopupButton* sortBtn = new CWizSortingPopupButton(*this, this);
    connect(sortBtn, SIGNAL(sortingTypeChanged(int)), SLOT(on_documents_sortingTypeChanged(int)));
    layoutActions->addWidget(sortBtn);
    layoutActions->addStretch(0);

    m_labelDocumentsHint = new QLabel(this);
    m_labelDocumentsHint->setMargin(5);
    layoutActions->addWidget(m_labelDocumentsHint);
    connect(m_category, SIGNAL(documentsHint(const QString&)), SLOT(on_documents_hintChanged(const QString&)));

    m_labelDocumentsCount = new QLabel("", this);
    m_labelDocumentsCount->setMargin(5);
    layoutActions->addWidget(m_labelDocumentsCount);
    connect(m_documents, SIGNAL(documentCountChanged()), SLOT(on_documents_documentCountChanged()));
    connect(m_documents, SIGNAL(changeUploadRequest(QString)), SLOT(on_quickSync_request(QString)));


    //sortBtn->setStyleSheet("padding-top:10px;");
    m_labelDocumentsHint->setStyleSheet("color: #787878;padding-bottom:1px;"); //font: 12px;
    m_labelDocumentsCount->setStyleSheet("color: #787878;padding-bottom:1px;"); //font: 12px;


    QWidget* line2 = new QWidget(this);
    line2->setFixedHeight(1);
    line2->setStyleSheet("border-top-width:1;border-top-style:solid;border-top-color:#DADAD9");

    layoutList->addLayout(layoutActions);
    layoutList->addWidget(line2);
    layoutList->addWidget(m_documents);

    return m_noteListWidget;
}

QWidget*MainWindow::createMessageListView()
{
    m_msgListWidget = new QWidget(this);
    m_msgListWidget->setMinimumWidth(100);
    QVBoxLayout* layoutList = new QVBoxLayout();
    layoutList->setContentsMargins(0, 0, 0, 0);
    layoutList->setSpacing(0);
    m_msgListWidget->setLayout(layoutList);

    m_msgListUnreadBar = new QWidget(this);
    m_msgListUnreadBar->setFixedHeight(Utils::StyleHelper::titleEditorHeight());
    QHBoxLayout* layoutActions = new QHBoxLayout();
    layoutActions->setContentsMargins(0, 0, 0, 0);
    layoutActions->setSpacing(0);
    m_msgListUnreadBar->setLayout(layoutActions);


    int nMargin = 15;
    QSize szMarkMessageBtn(16, 16);
    layoutActions->setContentsMargins(szMarkMessageBtn.width() + nMargin, 0, nMargin, 0);

    QLabel* labelHint = new QLabel(this);
    labelHint->setText(tr("Unread Messages"));
    labelHint->setAlignment(Qt::AlignCenter);
    labelHint->setStyleSheet("color: #787878;padding-bottom:1px;");
    layoutActions->addWidget(labelHint);

    wizImageButton* readBtn = new wizImageButton(this);
    QIcon btnIcon = ::WizLoadSkinIcon(userSettings().skin(), "actionMarkMessagesRead");
    readBtn->setIcon(btnIcon);
    readBtn->setFixedSize(szMarkMessageBtn);
    readBtn->setToolTip(tr("Mark all messages read"));
    connect(readBtn, SIGNAL(clicked()), SLOT(on_actionMarkAllMessageRead_triggered()));
    layoutActions->addWidget(readBtn);

    QWidget* line2 = new QWidget(this);
    line2->setFixedHeight(1);
    line2->setStyleSheet("border-top-width:1;border-top-style:solid;border-top-color:#DADAD9");

    layoutList->addWidget(m_msgListUnreadBar);
    layoutList->addWidget(line2);
    layoutList->addWidget(m_msgList);
    m_msgList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    return m_msgListWidget;
}

QWidget*MainWindow::client() const
{
    return m_doc->client();
}

void MainWindow::showClient(bool visible) const
{
    return m_doc->showClient(visible);
}

CWizIAPDialog*MainWindow::iapDialog()
{
#ifdef Q_OS_MAC
    if (m_iapDialog == 0) {
        m_iapDialog = new CWizIAPDialog(this);
    }
    return m_iapDialog;
#else
    return 0;
#endif
}

void MainWindow::on_documents_documentCountChanged()
{
    QString text;
    int count = m_documents->count();
    if (count == 1)
    {
        text = tr("1 note");
    }
    else if (count > 1)
    {
        if (count >= 1000)
        {
            text = tr("%1 notes").arg("1000+");
        }
        else
        {
            text = tr("%1 notes").arg(count);
        }
    }
    m_labelDocumentsCount->setText(text);
}

void MainWindow::on_documents_lastDocumentDeleted()
{
    ICore::instance()->emitCloseNoteRequested(m_doc);
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

void MainWindow::init()
{
    connect(m_category, SIGNAL(itemSelectionChanged()), SLOT(on_category_itemSelectionChanged()));
    connect(m_category, SIGNAL(newDocument()), SLOT(on_actionNewNote_triggered()));
    connect(m_category, SIGNAL(categoryItemPositionChanged(QString)), SLOT(on_quickSync_request(QString)));
    m_category->init();

    connect(m_msgList, SIGNAL(itemSelectionChanged()), SLOT(on_message_itemSelectionChanged()));
    connect(m_msgList, SIGNAL(loacteDocumetRequest(QString,QString)), SLOT(locateDocument(QString,QString)));
    connect(m_documents, SIGNAL(documentsSelectionChanged()), SLOT(on_documents_itemSelectionChanged()));
    connect(m_documents, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(on_documents_itemDoubleClicked(QListWidgetItem*)));
    connect(m_documents, SIGNAL(lastDocumentDeleted()), SLOT(on_documents_lastDocumentDeleted()));
    connect(m_documents, SIGNAL(shareDocumentByLinkRequest(QString,QString)),
            SLOT(on_shareDocumentByLink_request(QString,QString)));

    QTimer::singleShot(100, this, SLOT(adjustToolBarLayout()));
}

void MainWindow::on_actionAutoSync_triggered()
{
    m_sync->startSyncAll();
}

void MainWindow::on_actionSync_triggered()
{
    WizGetAnalyzer().LogAction("syncAll");

//    if (::WizIsOffline())
//    {
//        QMessageBox::information(this, tr("Info"), tr("Connection is not available, please check your network connection."));
//    }
//    else
//    {
        syncAllData();
//    }
}

void MainWindow::on_syncLogined()
{
    // FIXME: show user notify message send from server
}

void MainWindow::on_syncStarted(bool syncAll)
{
    if (!m_animateSync->isPlaying())
    {
        m_animateSync->startPlay();
    }
    //
    if (syncAll)
    {
        qDebug() << "[Sync] Syncing all notes...";
    }
    else
    {
        qDebug() << "[Sync] Quick syncing notes...";
    }
}

void MainWindow::on_syncDone(int nErrorCode, const QString& strErrorMsg)
{
    Q_UNUSED(strErrorMsg);

    m_animateSync->stopPlay();

    //
    if (errorTokenInvalid == nErrorCode)
    {
        reconnectServer();
        return;
    }
    else if (QNetworkReply::ProtocolUnknownError == nErrorCode)
    {
//        //network avaliable, show message once
//        static bool showMessageAgain = true;
//        if (showMessageAgain) {
//            QMessageBox messageBox(this);
//            messageBox.setIcon(QMessageBox::Information);
//            messageBox.setText(tr("Connection is not available, please check your network connection."));
//            QAbstractButton *btnDontShowAgain =
//                    messageBox.addButton(tr("Don't show this again"), QMessageBox::ActionRole);
//            messageBox.addButton(QMessageBox::Ok);
//            messageBox.exec();
//            showMessageAgain = messageBox.clickedButton() != btnDontShowAgain;
//        }
    }

    m_documents->viewport()->update();
    m_category->updateGroupsData();
    m_category->viewport()->update();
}

void MainWindow::on_syncDone_userVerified()
{
    m_userVerifyDialog->deleteLater();

    if (m_dbMgr.db().SetPassword(m_userVerifyDialog->password())) {
        m_sync->clearCurrentToken();
        syncAllData();
    }
}

void MainWindow::on_syncProcessLog(const QString& strMsg)
{
    Q_UNUSED(strMsg);
}

void MainWindow::on_promptMessage_request(int nType, const QString& strTitle, const QString& strMsg)
{
    switch (nType) {
    case wizSyncMessageNormal:
        showTrayIconMessage(strTitle, strMsg);
        break;
    case wizSyncMessageWarning:
        CWizMessageBox::warning(this, strTitle.isEmpty() ? tr("Info") : strTitle, strMsg);
        break;
    case wizSyncMeesageError:
        CWizMessageBox::critical(this, strTitle.isEmpty() ? tr("Info") : strTitle, strMsg);
        break;
    default:
        break;
    }
}

void MainWindow::on_actionNewNote_triggered()
{
    WizGetAnalyzer().LogAction("newNote");

    initVariableBeforCreateNote();
    WIZDOCUMENTDATA data;
    if (!m_category->createDocument(data))
    {
        return;
    }

    setFocusForNewNote(data);
    m_doc->web()->setEditorEnable(true);
    m_history->addHistory(data);
}

void MainWindow::on_actionNewNoteByTemplate_triggered()
{
    WizGetAnalyzer().LogAction("newNoteByTemplate");

    //通过模板创建笔记
    CWizDocTemplateDialog dlg;
    connect(&dlg, SIGNAL(documentTemplateSelected(QString)), SLOT(createDocumentByTemplate(QString)));
    dlg.exec();
}

void MainWindow::on_actionEditingUndo_triggered()
{
    WizGetAnalyzer().LogAction("undo");

    m_doc->web()->undo();
}

void MainWindow::on_actionEditingRedo_triggered()
{
    WizGetAnalyzer().LogAction("redo");

    m_doc->web()->redo();
}

#ifdef USEWEBENGINE
void MainWindow::on_actionEditingCut_triggered()
{
    m_doc->web()->triggerPageAction(QWebEnginePage::Cut);
}

void MainWindow::on_actionEditingCopy_triggered()
{
    m_doc->web()->triggerPageAction(QWebEnginePage::Copy);
}

void MainWindow::on_actionEditingPaste_triggered()
{
    m_doc->web()->setPastePlainTextEnable(false);
    m_doc->web()->triggerPageAction(QWebEnginePage::Paste);
}

void MainWindow::on_actionEditingPastePlain_triggered()
{
    m_doc->web()->setPastePlainTextEnable(true);
    m_doc->web()->triggerPageAction(QWebEnginePage::Paste);
}

void MainWindow::on_actionEditingSelectAll_triggered()
{
    m_doc->web()->triggerPageAction(QWebEnginePage::SelectAll);
}

void MainWindow::on_actionMoveToPageStart_triggered()
{
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_PageUp , Qt::AltModifier, QString());
    m_doc->web()->sendEventToChildWidgets(&keyPress);
}

void MainWindow::on_actionMoveToPageEnd_triggered()
{
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_PageDown , Qt::AltModifier, QString());
    m_doc->web()->sendEventToChildWidgets(&keyPress);
}

void MainWindow::on_actionMoveToLineStart_triggered()
{
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Home, Qt::NoModifier, QString());
    m_doc->web()->sendEventToChildWidgets(&keyPress);
}

void MainWindow::on_actionMoveToLineEnd_triggered()
{
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_End, Qt::NoModifier, QString());
    m_doc->web()->sendEventToChildWidgets(&keyPress);
}

#else

void MainWindow::on_actionEditingCut_triggered()
{
    WizGetAnalyzer().LogAction("cut");

    m_doc->web()->triggerPageAction(QWebPage::Cut);
}

void MainWindow::on_actionEditingCopy_triggered()
{
    WizGetAnalyzer().LogAction("copy");

    m_doc->web()->triggerPageAction(QWebPage::Copy);
}

void MainWindow::on_actionEditingPaste_triggered()
{
    WizGetAnalyzer().LogAction("paste");

    m_doc->web()->setPastePlainTextEnable(false);
    m_doc->web()->triggerPageAction(QWebPage::Paste);
}

void MainWindow::on_actionEditingPastePlain_triggered()
{
    WizGetAnalyzer().LogAction("pastePlain");

    m_doc->web()->setPastePlainTextEnable(true);
    m_doc->web()->triggerPageAction(QWebPage::Paste);
}

void MainWindow::on_actionEditingSelectAll_triggered()
{
    WizGetAnalyzer().LogAction("selectAll");

    m_doc->web()->triggerPageAction(QWebPage::SelectAll);
}
#endif

void MainWindow::on_actionEditingDelete_triggered()
{
    qDebug() << "delete...";
}


void MainWindow::on_actionViewToggleCategory_triggered()
{
    WizGetAnalyzer().LogAction("toggleCategory");

    QWidget* category = m_splitter->widget(0);
    if (category->isVisible()) {
        category->hide();
    } else {
        category->show();
    }

    if (m_docListContainer->isVisible()) {
        m_docListContainer->hide();
    } else {
        m_docListContainer->show();
    }

    m_actions->toggleActionText(WIZACTION_GLOBAL_TOGGLE_CATEGORY);
}

void MainWindow::on_actionViewToggleFullscreen_triggered()
{
    WizGetAnalyzer().LogAction("fullscreen");

#ifdef Q_OS_MAC
    //toggleFullScreenMode(this);
    setWindowState(windowState() ^ Qt::WindowFullScreen);
//    if (windowState() == Qt::WindowFullScreen)
//    {
//        m_toolBar->hide();
//        m_splitter->widget(0)->hide();
//        m_splitter->widget(1)->hide();
//    }
#endif // Q_OS_MAC
}

void MainWindow::on_actionViewMinimize_triggered()
{
    WizGetAnalyzer().LogAction("minimize");

    while (true) {
        QWidget* wgt = qApp->activeWindow();
        if (wgt == 0)
            return;

        wgt->setWindowState(Qt::WindowMinimized);
    }
}

void MainWindow::on_actionMarkAllMessageRead_triggered()
{
    WizGetAnalyzer().LogAction("markAllMessagesRead");

    m_msgList->markAllMessagesReaded();
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

void MainWindow::on_actionFormatPlainText_triggered()
{
    m_doc->web()->editorCommandExecutePlainText();
}

void MainWindow::on_actionEditorViewSource_triggered()
{
    m_doc->web()->editorCommandExecuteViewSource();
}

void MainWindow::on_actionFormatInsertCheckList_triggered()
{
    m_doc->web()->editorCommandExecuteInsertCheckList();
}

void MainWindow::on_actionFormatInsertCode_triggered()
{
    m_doc->web()->editorCommandExecuteInsertCode();
}

void MainWindow::on_actionFormatInsertImage_triggered()
{
    m_doc->web()->editorCommandExecuteInsertImage();
}

void MainWindow::on_actionFormatScreenShot_triggered()
{
    m_doc->web()->editorCommandExecuteScreenShot();
}

void MainWindow::on_actionConsole_triggered()
{
    if (!m_console) {
      m_console = new CWizConsoleDialog(*this, window());
    }

    m_console->show();

    WizGetAnalyzer().LogAction("console");
}

void MainWindow::on_actionLogout_triggered()
{
    WizGetAnalyzer().LogAction("logout");

    // save state
    m_settings->setAutoLogin(false);
    m_bLogoutRestart = true;
    on_actionExit_triggered();
}

void MainWindow::on_actionAbout_triggered()
{
    WizGetAnalyzer().LogAction("aboutWiz");

    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::on_actionDeveloper_triggered()
{
    m_doc->web()->settings()->globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

    WizGetAnalyzer().LogAction("developerMode");
}


void MainWindow::on_actionPreference_triggered()
{
    WizGetAnalyzer().LogAction("preference");

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

    //FIXME: special handle for support.html, shuold append displayName in url.
    CWizDatabase& personDb = m_dbMgr.db();
    QString strUserName = "Unkown";
    personDb.GetUserDisplayName(strUserName);
    strUrl.replace(QHostInfo::localHostName(), strUserName);

    QDesktopServices::openUrl(strUrl);

    WizGetAnalyzer().LogAction("feedback");
}

void MainWindow::on_actionSupport_triggered()
{
    QString strUrl = WizService::ApiEntry::supportUrl();

    if (strUrl.isEmpty())
        return;

    QDesktopServices::openUrl(strUrl);

    WizGetAnalyzer().LogAction("support");
}

void MainWindow::on_actionManual_triggered()
{
    QString strUrl = WizService::ApiEntry::standardCommandUrl("link", true);

    if (strUrl.isEmpty())
        return;

    strUrl += "&site=www&name=manual/mac/index.html";
    QDesktopServices::openUrl(strUrl);

    WizGetAnalyzer().LogAction("manual");
}

void MainWindow::on_actionRebuildFTS_triggered()
{
    WizGetAnalyzer().LogAction("rebuildFTS");

    QMessageBox msg;
    msg.setIcon(QMessageBox::Warning);
    msg.setWindowTitle(tr("Rebuild full text search index"));
    msg.addButton(QMessageBox::Ok);
    msg.addButton(QMessageBox::Cancel);
    msg.setText(tr("Rebuild full text search is quit slow if you have quite a few notes or attachments, you do not have to use this function while search should work as expected."));

    if (QMessageBox::Ok == msg.exec())
    {
        WizGetAnalyzer().LogAction("rebuildFTSConfirm");

        rebuildFTS();
    }
}

void MainWindow::on_actionSearch_triggered()
{
    m_searchWidget->focus();

    WizGetAnalyzer().LogAction("search");
}

void MainWindow::on_actionResetSearch_triggered()
{
    cancelSearchStatus();
    m_searchWidget->clear();
    m_searchWidget->focus();
    m_category->restoreSelection();
    m_doc->web()->applySearchKeywordHighlight();

    WizGetAnalyzer().LogAction("resetSearch");
}

void MainWindow::on_actionAdvancedSearch_triggered()
{
    m_category->on_action_advancedSearch();
}

void MainWindow::on_actionAddCustomSearch_triggered()
{
    m_category->on_action_addCustomSearch();
}

void MainWindow::on_actionFindReplace_triggered()
{    
    m_doc->web()->editorCommandExecuteFindReplace();
}

void MainWindow::on_actionSaveAsPDF_triggered()
{
    if (m_doc->web())
    {
        m_doc->web()->saveAsPDF();
    }
}

void MainWindow::on_actionSaveAsHtml_triggered()
{
    if (m_doc->web())
    {
        QString strPath = QFileDialog::getExistingDirectory(0, tr("Open Directory"),
                                                           QDir::homePath(),
                                                            QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks);
        if (!strPath.isEmpty())
        {
            m_doc->web()->saveAsHtml(strPath + "/");
        }
    }
}

void MainWindow::on_actionPrint_triggered()
{
    m_doc->web()->printDocument();
}

void MainWindow::on_actionPrintMargin_triggered()
{
    CWizPreferenceWindow preference(*this, this);
    preference.showPrintMarginPage();
    connect(&preference, SIGNAL(settingsChanged(WizOptionsType)), SLOT(on_options_settingsChanged(WizOptionsType)));
    connect(&preference, SIGNAL(restartForSettings()), SLOT(on_options_restartForSettings()));
    preference.exec();
}

//void MainWindow::on_searchDocumentFind(const WIZDOCUMENTDATAEX& doc)
//{
//    m_documents->addDocument(doc, true);
//    on_documents_itemSelectionChanged();
//}

void MainWindow::on_search_doSearch(const QString& keywords)
{
    m_strSearchKeywords = keywords;
    if (keywords.isEmpty()) {
        on_actionResetSearch_triggered();
        return;
    }
    //
    if (IsWizKMURL(keywords)) {
        QString strUrl = keywords;
        strUrl.remove("\n");
        viewDocumentByWizKMURL(strUrl);
        return;
    }

    m_category->saveSelection();
    m_documents->clear();
    //
    m_noteListWidget->show();
    m_msgListWidget->hide();
    //
    m_searcher->search(keywords, 500);
    startSearchStatus();
}


void MainWindow::on_searchProcess(const QString& strKeywords, const CWizDocumentDataArray& arrayDocument, bool bStart, bool bEnd)
{
    if (bEnd) {
        m_doc->web()->clearSearchKeywordHighlight(); //need clear hightlight first
        m_doc->web()->applySearchKeywordHighlight();
    }

//    if (strKeywords != m_strSearchKeywords) {
//        return;
//    }

    if (bStart) {
        m_documents->setDocuments(arrayDocument);
    } else {
//        m_documents->setDocuments(arrayDocument);
        m_documents->appendDocuments(arrayDocument);
    }
    on_documents_itemSelectionChanged();
}

#ifndef Q_OS_MAC
void MainWindow::on_actionPopupMainMenu_triggered()
{
    QAction* pAction = m_actions->actionFromName("actionPopupMainMenu");
    QRect rc = m_toolBar->actionGeometry(pAction);
    QPoint pt = m_toolBar->mapToGlobal(QPoint(rc.left(), rc.bottom()));

    CWizSettings settings(Utils::PathResolve::resourcesPath() + "files/mainmenu.ini");

    QMenu* pMenu = new QMenu(this);
    m_actions->buildMenu(pMenu, settings, pAction->objectName(), false);

    pMenu->popup(pt);
}

void MainWindow::on_menuButtonClicked()
{
    QWidget* wgt = qobject_cast<QWidget*>(sender());
    if (wgt)
    {
        QPoint popupPoint = clientWidget()->mapToGlobal(QPoint(wgt->pos().x(),
                                                                 wgt->pos().y() + wgt->height()));
        popupPoint.setY(popupPoint.y() - titleBar()->height());
        m_menu->popup(popupPoint);
    }
}

#endif

void MainWindow::on_client_splitterMoved(int pos, int index)
{
    adjustToolBarLayout();
}

void MainWindow::on_actionGoBack_triggered()
{
    WizGetAnalyzer().LogAction("goBack");

    if (!m_history->canBack())
        return;

    WIZDOCUMENTDATA data = m_history->back();
    CWizDatabase &db = m_dbMgr.db(data.strKbGUID);
    if (db.DocumentFromGUID(data.strGUID, data))
    {
        viewDocument(data, false);
        locateDocument(data);
    }
    else
    {
        on_actionGoBack_triggered();
    }

    updateHistoryButtonStatus();
    m_doc->setFocus();
}

void MainWindow::on_actionGoForward_triggered()
{
    WizGetAnalyzer().LogAction("goForward");

    if (!m_history->canForward())
        return;

    WIZDOCUMENTDATA data = m_history->forward();
    CWizDatabase &db = m_dbMgr.db(data.strKbGUID);
    if (db.DocumentFromGUID(data.strGUID, data))
    {
        viewDocument(data, false);
        locateDocument(data);
    }
    else
    {
        on_actionGoForward_triggered();
    }

    updateHistoryButtonStatus();
    m_doc->setFocus();
}

void MainWindow::on_category_itemSelectionChanged()
{
    CWizCategoryBaseView* category = qobject_cast<CWizCategoryBaseView *>(sender());
    if (!category)
        return;
    cancelSearchStatus();
    /*
     * 在点击MessageItem的时候,为了重新刷新当前消息,强制发送了itemSelectionChanged消息
     * 因此需要在这个地方避免重复刷新两次消息列表
     */
    static QTime lastTime(0, 0, 0);
    QTreeWidgetItem *currentItem = category->currentItem();
    static QTreeWidgetItem *oldItem = currentItem;
    QTime last = lastTime;
    QTime now = QTime::currentTime();
    lastTime = now;
    if (last.msecsTo(now) < 300 && oldItem == currentItem) {
        return;
    } else {
        oldItem = currentItem;
    }

    QTreeWidgetItem* categoryItem = category->currentItem();
    switch (categoryItem->type()) {
    case ItemType_MessageItem:
    {
        CWizCategoryViewMessageItem* pItem = dynamic_cast<CWizCategoryViewMessageItem*>(categoryItem);
        if (pItem)
        {
            showMessageList(pItem);
        }
    }
        break;
    case ItemType_ShortcutItem:
    {
        CWizCategoryViewShortcutItem* pShortcut = dynamic_cast<CWizCategoryViewShortcutItem*>(categoryItem);
        if (pShortcut)
        {
            viewDocumentByShortcut(pShortcut);
        }
    }
        break;
    case ItemType_QuickSearchItem:
    {
        CWizCategoryViewSearchItem* pSearchItem = dynamic_cast<CWizCategoryViewSearchItem*>(categoryItem);
        if (pSearchItem)
        {
            searchNotesBySQL(pSearchItem->getSQLWhere());
        }
    }
        break;
    case ItemType_QuickSearchCustomItem:
    {
        CWizCategoryViewCustomSearchItem* pSearchItem = dynamic_cast<CWizCategoryViewCustomSearchItem*>(categoryItem);
        if (pSearchItem)
        {
            searchNotesBySQLAndKeyword(pSearchItem->getSQLWhere(), pSearchItem->getKeyword(), pSearchItem->searchScope());
        }
    }
        break;
    default:
        showDocmentList(category);
        break;
    }
}

void MainWindow::on_documents_itemSelectionChanged()
{
    CWizDocumentDataArray arrayDocument;
    m_documents->getSelectedDocuments(arrayDocument);

    if (arrayDocument.size() == 1)
    {
        if (!m_bUpdatingSelection)
        {
            viewDocument(arrayDocument[0], true);
            resortDocListAfterViewDocument(arrayDocument[0]);
        }
    }

    updateHistoryButtonStatus();
}

void MainWindow::on_documents_itemDoubleClicked(QListWidgetItem* item)
{
    CWizDocumentListViewItem* pItem = dynamic_cast<CWizDocumentListViewItem*>(item);
    if (pItem)
    {
        WIZDOCUMENTDATA doc = pItem->document();
        if (m_dbMgr.db(doc.strKbGUID).IsDocumentDownloaded(doc.strGUID))
        {
            viewDocumentInFloatWidget(doc);
            resortDocListAfterViewDocument(doc);
        }
    }
}

void MainWindow::on_message_itemSelectionChanged()
{
    QList<WIZMESSAGEDATA> listMsg;
    m_msgList->selectedMessages(listMsg);

    if (listMsg.size() == 1) {
        WIZMESSAGEDATA msg(listMsg[0]);
        WIZDOCUMENTDATA doc;
        if (!m_dbMgr.db(msg.kbGUID).DocumentFromGUID(msg.documentGUID, doc)) {
            m_doc->promptMessage(tr("Can't find note %1 , may be it has been deleted.").arg(msg.title));
            return;
        }
        viewDocument(doc, true);
    }
}

void MainWindow::on_options_settingsChanged(WizOptionsType type)
{
    switch (type) {
    case wizoptionsNoteView:
        m_doc->settingsChanged();
        break;
    case wizoptionsSync:
        m_sync->setFullSyncInterval(userSettings().syncInterval());
        break;
    case wizoptionsFont:
        m_doc->web()->editorResetFont();
        break;
    case wizoptionsFolders:
        m_category->sortItems(0, Qt::AscendingOrder);
        break;
    case wizoptionsMarkdown:
        Core::ICore::instance()->emitMarkdownSettingChanged();
        break;
    default:
        break;
    }
}

void MainWindow::on_options_restartForSettings()
{
    m_bRestart = true;
    on_actionExit_triggered();
}

void MainWindow::resetPermission(const QString& strKbGUID, const QString& strOwner)
{
    int nPerm = m_dbMgr.db(strKbGUID).permission();
    bool isGroup = m_dbMgr.db().kbGUID() != strKbGUID;

    // Admin, Super, do anything
    if (nPerm == WIZ_USERGROUP_ADMIN || nPerm == WIZ_USERGROUP_SUPER)
    {
        // enable editing
        //m_doc->setReadOnly(false, isGroup);

        // enable create tag

        // enable new document
        m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT)->setEnabled(true);

        // enable delete document
        //m_actions->actionFromName("actionDeleteCurrentNote")->setEnabled(true);

        // Editor, only disable create tag
    }
    else if (nPerm == WIZ_USERGROUP_EDITOR)
    {
        //m_doc->setReadOnly(false, isGroup);
        m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT)->setEnabled(true);
        //m_actions->actionFromName("actionDeleteCurrentNote")->setEnabled(true);

        // Author
    }
    else if (nPerm == WIZ_USERGROUP_AUTHOR)
    {
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
    }
    else if (nPerm == WIZ_USERGROUP_READER)
    {
        //m_doc->setReadOnly(true, isGroup);
        m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT)->setEnabled(false);
        //m_actions->actionFromName("actionDeleteCurrentNote")->setEnabled(false);
    }
    else
    {
       Q_ASSERT(0);
    }
}

void MainWindow::viewDocument(const WIZDOCUMENTDATA& data, bool addToHistory)
{
    Q_ASSERT(!data.strGUID.isEmpty());

    if (data.strGUID == m_doc->note().strGUID)
    {
        m_doc->reviewCurrentNote();
        return;
    }

    resetPermission(data.strKbGUID, data.strOwner);

    ICore::emitViewNoteRequested(m_doc, data);

    if (addToHistory) {
        m_history->addHistory(data);
    }
}

void MainWindow::locateDocument(const WIZDOCUMENTDATA& data)
{
    try
    {
        m_bUpdatingSelection = true;
        if (m_category->setCurrentIndex(data))
        {
            m_documents->addAndSelectDocument(data);
        }
    }
    catch (...)
    {

    }

    m_bUpdatingSelection = false;
}

void MainWindow::locateDocument(const QString& strKbGuid, const QString& strGuid)
{
    WIZDOCUMENTDATA doc;
    if (m_dbMgr.db(strKbGuid).DocumentFromGUID(strGuid, doc))
    {
        locateDocument(doc);
    }
}

QWidget*MainWindow::mainWindow()
{
    return this;
}

QObject*MainWindow::object()
{
    return this;
}

CWizCategoryBaseView&MainWindow::category()
{
    return *m_category;
}

CWizUserSettings&MainWindow::userSettings()
{
    return *m_settings;
}

QObject*MainWindow::CategoryCtrl()
{
    return m_category;
}

void MainWindow::on_application_messageAvailable(const QString& strMsg)
{
    qDebug() << "application message received : " << strMsg;
    if (strMsg == WIZ_SINGLE_APPLICATION)
    {
        shiftVisableStatus();
    }
}

void MainWindow::checkWizUpdate()
{
#ifndef BUILD4APPSTORE
    m_upgrade->startCheck();
#endif
}


void MainWindow::adjustToolBarLayout()
{
#ifdef Q_OS_LINUX
    if (!m_toolBar)
        return;
    //
    QWidget* list = m_documents->isVisible() ? (QWidget*)m_documents : (QWidget*)m_msgList;
    //
    QPoint ptSearch = list->mapToGlobal(QPoint(0, 0));
    QPoint ptSpacerBeforeSearch = m_spacerForToolButtonAdjust->mapToGlobal(QPoint(0, 0));
    //
    int spacerWidth = ptSearch.x() - ptSpacerBeforeSearch.x();
    int searchWidth = list->size().width();
    if (spacerWidth > 0)
    {
        m_spacerForToolButtonAdjust->adjustWidth(spacerWidth);
    }
    else
    {
        searchWidth += spacerWidth;
    }
    //
    if (searchWidth > 100)
    {
        m_search->setFixedWidth(searchWidth);
    }
#else
//#ifndef USECOCOATOOLBAR
    if (!m_toolBar)
        return;
    //
    QWidget* list = m_documents->isVisible() ? (QWidget*)m_documents : (QWidget*)m_msgList;
    //
    QPoint ptSearch = list->mapToGlobal(QPoint(0, 0));
    QPoint ptSpacerBeforeSearch = m_spacerForToolButtonAdjust->mapToGlobal(QPoint(0, 0));
    //
    int spacerWidth = ptSearch.x() - ptSpacerBeforeSearch.x();
    spacerWidth += list->size().width();
    if (spacerWidth < 0)
        return;
    //
    m_spacerForToolButtonAdjust->adjustWidth(spacerWidth);
//#endif
#endif
}



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

QString MainWindow::TranslateString(const QString& string)
{
    return ::WizTranlateString(string);
}

void MainWindow::syncAllData()
{
    m_sync->startSyncAll(false);
    m_animateSync->startPlay();
}

void MainWindow::reconnectServer()
{
    CWizDatabase& db = m_dbMgr.db();
    WizService::Token::setUserId(db.GetUserId());
    WizService::Token::setPasswd(m_settings->password());

    m_sync->clearCurrentToken();
    connect(WizService::Token::instance(), SIGNAL(tokenAcquired(QString)),
            SLOT(on_TokenAcquired(QString)), Qt::QueuedConnection);
    WizService::Token::requestToken();
}

void MainWindow::setFocusForNewNote(WIZDOCUMENTDATA doc)
{
    m_documentForEditing = doc;
    m_documents->addAndSelectDocument(doc);
    m_documents->clearFocus();
    m_doc->setFocus(Qt::MouseFocusReason);
    m_doc->web()->setFocus(Qt::MouseFocusReason);
}

void MainWindow::viewDocumentByWizKMURL(const QString &strKMURL)
{
    if (GetWizUrlType(strKMURL) != WizUrl_Document)
        return;

    QString strKbGUID = GetParamFromWizKMURL(strKMURL, "kbguid");
    QString strGUID = GetParamFromWizKMURL(strKMURL, "guid");
    CWizDatabase& db = m_dbMgr.db(strKbGUID);

    WIZDOCUMENTDATA document;
    if (db.DocumentFromGUID(strGUID, document))
    {
        //m_category->setCurrentItem();
        m_documents->blockSignals(true);
        m_documents->setCurrentItem(0);
        m_documents->blockSignals(false);
        viewDocument(document, true);
        locateDocument(document);
        activateWindow();
        raise();
    }
}

void MainWindow::viewAttachmentByWizKMURL(const QString& strKbGUID, const QString& strKMURL)
{

    if (GetWizUrlType(strKMURL) != WizUrl_Attachment)
        return;

    CWizDatabase& db = m_dbMgr.db(strKbGUID);
    QString strGUID = GetParamFromWizKMURL(strKMURL, "guid");

    WIZDOCUMENTATTACHMENTDATA attachment;
    if (db.AttachmentFromGUID(strGUID, attachment))
    {
        bool bIsLocal = db.IsObjectDataDownloaded(attachment.strGUID, "attachment");
        QString strFileName = db.GetAttachmentFileName(attachment.strGUID);
        bool bExists = PathFileExists(strFileName);
        if (!bIsLocal || !bExists)
        {
            downloadAttachment(attachment);

#if QT_VERSION > 0x050000
            // try to set the attachement read-only.
            QFile file(strFileName);
            if (file.exists() && !db.CanEditAttachment(attachment) && (file.permissions() & QFileDevice::WriteUser))
            {
                QFile::Permissions permissions = file.permissions();
                permissions = permissions & ~QFileDevice::WriteOwner & ~QFileDevice::WriteUser
                        & ~QFileDevice::WriteGroup & ~QFileDevice::WriteOther;
                file.setPermissions(permissions);
            }
#endif
        }

        openAttachment(attachment, strFileName);
    }
}

void MainWindow::createNoteWithAttachments(const QStringList& strAttachList)
{
    initVariableBeforCreateNote();
    WIZDOCUMENTDATA data;
    if (!m_category->createDocumentByAttachments(data, strAttachList))
        return;

    setFocusForNewNote(data);
}

void MainWindow::createNoteWithText(const QString& strText)
{
    initVariableBeforCreateNote();
#if QT_VERSION > 0x050000
    QString strHtml = strText.toHtmlEscaped();
#else
    QString strHtml = strText;
#endif
    QString strTitle = strHtml.left(strHtml.indexOf("\n"));
    if (strTitle.isEmpty())
    {
        strTitle = "New note";
    }
    else if (strTitle.length() > 200)
    {
        strTitle = strTitle.left(200);
    }
    strHtml = "<div>" + strHtml + "</div>";
    strHtml.replace(" ", "&nbsp;");
    strHtml.replace("\n", "<br />");
    WIZDOCUMENTDATA data;
    if (!m_category->createDocument(data, strHtml, strTitle))
    {
        return;
    }
    setFocusForNewNote(data);
}

void MainWindow::createNoteWithImage(const QString& strImageFile)
{
    initVariableBeforCreateNote();

    QString strTitle = Utils::Misc::extractFileTitle(strImageFile);
    if (strTitle.isEmpty())
    {
        strTitle = "New note";
    }

    QString strHtml;
    bool bUseCopyFile = true;
    if (WizImage2Html(strImageFile, strHtml, bUseCopyFile))
    {
        WIZDOCUMENTDATA data;
        if (!m_category->createDocument(data, strHtml, strTitle))
        {
            return;
        }
        setFocusForNewNote(data);
    }
}

void MainWindow::showNewFeatureGuide()
{
    QString strUrl = WizService::ApiEntry::standardCommandUrl("link", true);
    strUrl += "&name=newfeature-mac";

    CWizFramelessWebDialog *dlg = new CWizFramelessWebDialog();
    dlg->loadAndShow(strUrl);
}

void MainWindow::showMobileFileReceiverUserGuide()
{
    QString strUrl = WizService::ApiEntry::standardCommandUrl("link", true);
    strUrl += "&name=guidemap_sendimage.html";

    CWizFramelessWebDialog *dlg = new CWizFramelessWebDialog();
    connect(dlg, SIGNAL(doNotShowThisAgain(bool)),
            SLOT(setDoNotShowMobileFileReceiverUserGuideAgain(bool)));
    dlg->loadAndShow(strUrl);
}

void MainWindow::setDoNotShowMobileFileReceiverUserGuideAgain(bool bNotAgain)
{
    m_settings->setNeedShowMobileFileReceiverUserGuide(!bNotAgain);
}

void MainWindow::initTrayIcon(QSystemTrayIcon* trayIcon)
{
    Q_ASSERT(trayIcon);
    m_trayMenu = new QMenu(this);
    QAction* actionShow = m_trayMenu->addAction(tr("Show/Hide MainWindow"));
    connect(actionShow, SIGNAL(triggered()), SLOT(shiftVisableStatus()));

    QAction* actionNewNote = m_trayMenu->addAction(tr("New Note"));
    connect(actionNewNote, SIGNAL(triggered()), SLOT(on_trayIcon_newDocument_clicked()));    

    //
    m_trayMenu->addSeparator();
    QAction* actionHideTrayIcon = m_trayMenu->addAction(tr("Hide TrayIcon"));
    connect(actionHideTrayIcon, SIGNAL(triggered()), SLOT(on_hideTrayIcon_clicked()));
    //
    m_trayMenu->addSeparator();
    QAction* actionLogout = m_trayMenu->addAction(tr("Logout"));
    connect(actionLogout, SIGNAL(triggered()), SLOT(on_actionLogout_triggered()));
    QAction* actionExit = m_trayMenu->addAction(tr("Exit"));
    connect(actionExit, SIGNAL(triggered()), SLOT(on_actionExit_triggered()));


#ifdef Q_OS_MAC
    trayIcon->setContextMenu(m_trayMenu);
    //
    QString normal = WizGetSkinResourceFileName(userSettings().skin(), "trayIcon");
    QString selected = WizGetSkinResourceFileName(userSettings().skin(), "trayIcon_selected");
    QIcon icon;
    icon.addFile(normal, QSize(), QIcon::Normal, QIcon::Off);
    icon.addFile(selected, QSize(), QIcon::Selected, QIcon::Off);
    if (!icon.isNull())
    {
        trayIcon->setIcon(icon);
    }
#else
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            SLOT(on_trayIcon_actived(QSystemTrayIcon::ActivationReason)));
//    QString normal = WizGetSkinResourceFileName(userSettings().skin(), "trayIcon_grey");
//    QIcon icon(normal);
//    if (!icon.isNull())
//    {
//        trayIcon->setIcon(icon);
//    }
#endif
}

#ifdef Q_OS_LINUX
void MainWindow::setWindowStyleForLinux(bool bUseSystemStyle)
{
    if (bUseSystemStyle)
    {
        setAttribute(Qt::WA_TranslucentBackground, false); //enable MainWindow to be transparent
        {
        QMainWindow window;
        setWindowFlags(window.windowFlags());
        }
        rootWidget()->setContentsMargins(0, 0, 0, 0);
        titleBar()->maxButton()->setVisible(false);
        titleBar()->minButton()->setVisible(false);
        titleBar()->closeButton()->setVisible(false);
    }
}
#endif

void MainWindow::setMobileFileReceiverEnable(bool bEnable)
{
    if (bEnable)
    {
        if (!m_mobileFileReceiver)
        {
            m_mobileFileReceiver = new CWizMobileFileReceiver(this);
            connect(m_mobileFileReceiver, SIGNAL(fileReceived(QString)),
                    SLOT(on_mobileFileRecived(QString)));
            m_mobileFileReceiver->start();
        }
    }
    else
    {
        if (m_mobileFileReceiver)
        {
            m_mobileFileReceiver->waitForDone();
            delete m_mobileFileReceiver;
            m_mobileFileReceiver = 0;
        }
    }
}

void MainWindow::startSearchStatus()
{
    m_documents->setAcceptAllItems(true);
}

void MainWindow::cancelSearchStatus()
{
    m_documents->setAcceptAllItems(false);
    if (m_category->selectedItems().count() > 0)
    {
        m_category->setFocus();
        m_category->setCurrentItem(m_category->selectedItems().first());
    }
}

void MainWindow::initVariableBeforCreateNote()
{
    cancelSearchStatus();
}

bool MainWindow::needShowNewFeatureGuide()
{
    QString strGuideVserion = m_settings->newFeatureGuideVersion();
    if (strGuideVserion.isEmpty())
        return true;

    //FIXME:  之前的版本误将版本号写入了新特性的数据中，此处将其忽略 。2015年07月之后可以将其移除
    if (strGuideVserion.contains("2.1"))
        strGuideVserion = "0";

    return strGuideVserion.compare(WIZ_NEW_FEATURE_GUIDE_VERSION) < 0;
}

void MainWindow::resortDocListAfterViewDocument(const WIZDOCUMENTDATA& doc)
{
    if (m_documents->isSortedByAccessDate())
    {
        m_documents->reloadItem(doc.strKbGUID, doc.strGUID);
        m_documents->sortItems();
    }
}

void MainWindow::showDocmentList(CWizCategoryBaseView* category)
{
    if (!m_noteListWidget->isVisible())
    {
        m_docListContainer->show();
        m_noteListWidget->show();
        m_msgListWidget->hide();
    }
    QString kbGUID = category->selectedItemKbGUID();
    if (!kbGUID.isEmpty())
    {
        resetPermission(kbGUID, "");
    }

    CWizDocumentDataArray arrayDocument;
    category->getDocuments(arrayDocument);
    m_documents->setDocuments(arrayDocument);

    if (arrayDocument.empty())
    {
        on_documents_itemSelectionChanged();
    }
}

void MainWindow::showMessageList(CWizCategoryViewMessageItem* pItem)
{
    if (!m_msgListWidget->isVisible())
    {
        m_docListContainer->show();
        m_msgListWidget->show();
        m_noteListWidget->hide();
    }

    CWizMessageDataArray arrayMsg;
    pItem->getMessages(m_dbMgr.db(), arrayMsg);
    m_msgList->setMessages(arrayMsg);

    bool showUnreadBar = pItem->hitTestUnread();
    m_msgListUnreadBar->setVisible(showUnreadBar);
}

void MainWindow::viewDocumentByShortcut(CWizCategoryViewShortcutItem* pShortcut)
{
    CWizDatabase &db = m_dbMgr.db(pShortcut->kbGUID());
    WIZDOCUMENTDATA doc;
    if (db.DocumentFromGUID(pShortcut->guid(), doc))
    {
        viewDocument(doc, true);
        m_docListContainer->hide();
    }
}

void MainWindow::searchNotesBySQL(const QString& strSQLWhere)
{
    if (strSQLWhere.isEmpty())
        return;
    m_searcher->searchBySQLWhere(strSQLWhere, 500);
}

void MainWindow::searchNotesBySQLAndKeyword(const QString& strSQLWhere, const QString& strKeyword, int searchScope)
{
    qDebug() << "search by sql and keyword : " << strSQLWhere << strKeyword;
    if (strSQLWhere.isEmpty())
    {
        m_searcher->search(strKeyword, 500, (SearchScope)searchScope);
    }
    else if (strKeyword.isEmpty())
    {
        m_searcher->searchBySQLWhere(strSQLWhere, 500, (SearchScope)searchScope);
    }
    else
    {
        m_searcher->searchByKeywordAndWhere(strKeyword, strSQLWhere, 500, (SearchScope)searchScope);
    }
}

void MainWindow::updateHistoryButtonStatus()
{
    bool canGoBack = m_history->canBack();
    m_actions->actionFromName(WIZACTION_GLOBAL_GOBACK)->setEnabled(canGoBack);
    bool canGoForward = m_history->canForward();
    m_actions->actionFromName(WIZACTION_GLOBAL_GOFORWARD)->setEnabled(canGoForward);
}

void MainWindow::openAttachment(const WIZDOCUMENTATTACHMENTDATA& attachment,
                                const QString& strFileName)
{
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(strFileName)))
    {
        qDebug() << "Can not open attachment file : " << strFileName;
    }

    CWizFileMonitor& monitor = CWizFileMonitor::instance();
    connect(&monitor, SIGNAL(fileModified(QString,QString,QString,QString,QDateTime)),
            &m_dbMgr.db(), SLOT(onAttachmentModified(QString,QString,QString,QString,QDateTime)), Qt::UniqueConnection);

    /*需要使用文件的修改日期来判断文件是否被改动,从服务器上下载下的文件修改日期必定大于数据库中日期.*/
    QFileInfo info(strFileName);
    monitor.addFile(attachment.strKbGUID, attachment.strGUID, strFileName,
                    attachment.strDataMD5, info.lastModified());
}

void MainWindow::downloadAttachment(const WIZDOCUMENTATTACHMENTDATA& attachment)
{
    CWizProgressDialog *dlg = progressDialog();
    dlg->setProgress(100,0);
    dlg->setActionString(tr("Downloading attachment file  %1 ").arg(attachment.strName));
    dlg->setNotifyString(tr("downloading..."));
    connect(m_objectDownloaderHost, SIGNAL(downloadProgress(QString,int,int)),
            dlg, SLOT(setProgress(QString,int,int)));
    connect(m_objectDownloaderHost, SIGNAL(downloadDone(WIZOBJECTDATA,bool)),
            dlg, SLOT(accept()));
    m_objectDownloaderHost->downloadData(attachment);
    dlg->exec();
}

void MainWindow::viewDocumentInFloatWidget(const WIZDOCUMENTDATA& data)
{
    WizFloatDocumentViewer* wgt = new WizFloatDocumentViewer(*this);
    CWizDocumentView* docView = wgt->docView();
    connect(docView, SIGNAL(documentSaved(QString,CWizDocumentView*)), SIGNAL(documentSaved(QString,CWizDocumentView*)));
    connect(this, SIGNAL(documentSaved(QString,CWizDocumentView*)), docView, SLOT(on_document_data_saved(QString,CWizDocumentView*)));
    connect(docView->web(), SIGNAL(shareDocumentByLinkRequest(QString,QString)),
            SLOT(on_shareDocumentByLink_request(QString,QString)));

    wgt->setGeometry((width() - m_doc->width())  / 2, (height() - wgt->height()) / 2,
                     m_doc->width(), wgt->height());
    wgt->setWindowTitle(data.strTitle);
    wgt->show();
    //
    docView->viewNote(data, false);
}

void MainWindow::quickSyncKb(const QString& kbGuid)
{
    CWizKMSyncThread::quickSyncKb(kbGuid);
}


