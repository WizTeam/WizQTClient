#include "wizmainwindow.h"

#include <QToolBar>
#include <QMenuBar>
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QUndoStack>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QPushButton>
#include <QHostInfo>
#include <QSystemTrayIcon>

#ifdef Q_OS_MAC
#include <Carbon/Carbon.h>
#include "mac/wizmachelper.h"
#include "mac/wizmactoolbar.h"
#include "mac/wizSearchWidget_mm.h"
#else
#endif
#include "wizSearchWidget.h"

#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/aboutdialog.h>

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

#include "wizUserVerifyDialog.h"

#include "plugindialog.h"

#include "notecomments.h"

using namespace Core;
using namespace Core::Internal;
using namespace WizService::Internal;

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
    , m_upgrade(new CWizUpgrade())
    #else
    , m_upgrade(0)
    #endif
    //, m_certManager(new CWizCertManager(*this))
    , m_objectDownloaderHost(new CWizObjectDataDownloaderHost(dbMgr, this))
    //, m_avatarDownloaderHost(new CWizUserAvatarDownloaderHost(dbMgr.db().GetAvatarPath(), this))
    , m_transitionView(new CWizDocumentTransitionView(this))
    #ifndef Q_OS_MAC
    , m_labelNotice(NULL)
    , m_optionsAction(NULL)
    #endif
    #ifdef Q_OS_MAC
    , m_menuBar(new QMenuBar(this))
    , m_toolBar(new QToolBar(this))//(new CWizMacToolBar(this))
    #else
    , m_toolBar(new QToolBar("Main", titleBar()))
    , m_menu(new QMenu(clientWidget()))
    , m_spacerBeforeSearch(NULL)
    #endif
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
    , m_tray(new QSystemTrayIcon(QApplication::windowIcon(), this))
{
#ifndef Q_OS_MAC
    clientLayout()->addWidget(m_toolBar);
#endif
    //
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(on_application_aboutToQuit()));
    connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit())); // Qt bug: Qt5 bug
    qApp->installEventFilter(this);

    //CWizCloudPool::instance()->init(&m_dbMgr);

    connect(m_objectDownloaderHost, SIGNAL(downloadProgress(QString, int,int)),
            m_transitionView, SLOT(onDownloadProgressChanged(QString, int,int)));

    // search and full text search
    m_searchIndexer->start(QThread::IdlePriority);
    m_searcher->start(QThread::HighPriority);

    // syncing thread
    connect(m_sync, SIGNAL(processLog(const QString&)), SLOT(on_syncProcessLog(const QString&)));
    connect(m_sync, SIGNAL(syncStarted(bool)), SLOT(on_syncStarted(bool)));
    connect(m_sync, SIGNAL(syncFinished(int, QString)), SLOT(on_syncDone(int, QString)));

    connect(m_searcher, SIGNAL(searchProcess(const QString&, const CWizDocumentDataArray&, bool)),
        SLOT(on_searchProcess(const QString&, const CWizDocumentDataArray&, bool)));

    // misc settings
    //m_avatarDownloaderHost->setDefault(::WizGetSkinResourcePath(userSettings().skin()) + "avatar_default.png");

    // GUI
    initActions();
#ifdef Q_OS_MAC
    initMenuBar();
#else
    initMenuList();
#endif
    initToolBar();
    initClient();

    setWindowTitle(tr("WizNote"));

    restoreStatus();

    client()->hide();

    // upgrade check
#ifndef BUILD4APPSTORE
    QThread *thread = new QThread(this);
    m_upgrade->moveToThread(thread);
    connect(m_upgrade, SIGNAL(checkFinished(bool)), SLOT(on_checkUpgrade_finished(bool)));
    thread->start(QThread::IdlePriority);
    if (userSettings().autoCheckUpdate()) {
        checkWizUpdate();
    }
#endif

#ifdef Q_OS_MAC
    setupFullScreenMode(this);
#endif // Q_OS_MAC

    WizService::NoteComments::init();
    //
    m_sync->start(QThread::IdlePriority);
    //
    initTrayIcon(m_tray);
    m_tray->show();
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
                    if (WizIsKMURL(strUrl))
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

    //
    return _baseClass::eventFilter(watched, event);
}

void MainWindow::on_application_aboutToQuit()
{
    cleanOnQuit();
}


void MainWindow::cleanOnQuit()
{
    m_category->saveState();
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
    if (!event->spontaneous())
    {
        //setVisible(false);
        setWindowState(Qt::WindowMinimized);
        event->ignore();
        return;
    }
#endif
}

void MainWindow::on_actionExit_triggered()
{
    qApp->exit();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
}

void MainWindow::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);

    //
#ifdef Q_OS_MAC
    //m_toolBar->showInWindow(this);
    //m_toolBar->setToolBarVisible(true);
#endif
}

void MainWindow::on_checkUpgrade_finished(bool bUpgradeAvaliable)
{
    if (!bUpgradeAvaliable)
        return;

    QString strUrl = m_upgrade->getWhatsNewUrl();
    CWizUpgradeNotifyDialog notifyDialog(strUrl, this);
    if (QDialog::Accepted == notifyDialog.exec()) {
        QString url = WizService::ApiEntry::standardCommandUrl("link");
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

void MainWindow::on_trayIcon_newDocument_clicked()
{
    setVisible(true);
    QApplication::setActiveWindow(this);
    raise();

    on_actionNewNote_triggered();
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
    m_actions->init();
    m_animateSync->setAction(m_actions->actionFromName(WIZACTION_GLOBAL_SYNC));
    m_animateSync->setIcons("sync");

    connect(m_doc->web(), SIGNAL(statusChanged()), SLOT(on_editor_statusChanged()));
    //connect(m_doc->web()->page(), SIGNAL(contentsChanged()), SLOT(on_document_contentChanged()));
    //connect(m_doc->web()->page(), SIGNAL(selectionChanged()), SLOT(on_document_contentChanged()));

    on_editor_statusChanged();
}

#ifdef Q_OS_MAC
void MainWindow::initMenuBar()
{
    setMenuBar(m_menuBar);
    m_actions->buildMenuBar(m_menuBar, Utils::PathResolve::resourcesPath() + "files/mainmenu.ini");
}
#endif

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
        m_actions->actionFromName(WIZACTION_FORMAT_INSERT_CHECKLIST)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_REMOVE_FORMAT)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_FORMAT_VIEW_SOURCE)->setEnabled(false);

        return;
    }

    //if (!editor->page()->undoStack()->canUndo()) {
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
}

QString MainWindow::getSkinResourcePath() const
{
    return ::WizGetSkinResourcePath(m_settings->skin());
}

QString MainWindow::getUserAvatarFilePath(int size) const
{
    QString strFileName;
    QString strUserID = m_dbMgr.db().GetUserId();
    if (WizService::AvatarHost::customSizeAvatar(strUserID, size, size, strFileName))
        return strFileName;


    return QString();
}

QString MainWindow::getUserAlias() const
{
    QString strKbGUID = m_doc->note().strKbGUID;
    return m_dbMgr.db(strKbGUID).getUserAlias();
}

QString MainWindow::getFormatedDateTime() const
{
    COleDateTime time = QDateTime::currentDateTime();
    return ::WizDateToLocalString(time);
}

bool MainWindow::isPersonalDocument() const
{
    QString strKbGUID = m_doc->note().strKbGUID;
    QString dbKbGUID = m_dbMgr.db().kbGUID();
    return strKbGUID.isEmpty() || (strKbGUID == dbKbGUID);
}

QString MainWindow::getCurrentNoteHtml() const
{
    CWizDatabase& db = m_dbMgr.db(m_doc->note().strKbGUID);
    QString strFolder;
    if (db.extractZiwFileToTempFolder(m_doc->note(), strFolder))
    {
        QString strHtmlFile = strFolder + "index.html";
        QString strHtml;
        ::WizLoadUnicodeTextFromFile(strHtmlFile, strHtml);
        return strHtml;
    }

    return QString();
}

void MainWindow::saveHtmlToCurrentNote(const QString &strHtml, const QString& strResource)
{
    WIZDOCUMENTDATA docData = m_doc->note();
    CWizDatabase& db = m_dbMgr.db(docData.strKbGUID);
    QString strFolder;
    if (db.extractZiwFileToTempFolder(m_doc->note(), strFolder))
    {
        QString strHtmlFile = strFolder + "index.html";
        ::WizSaveUnicodeTextToUtf8File(strHtmlFile, strHtml);
        QStringList strResourceList = strResource.split('*');
        db.encryptTempFolderToZiwFile(docData, strFolder, strHtmlFile, strResourceList);
        quickSyncKb(docData.strKbGUID);
    }

    m_doc->web()->updateNoteHtml();
}

bool MainWindow::hasEditPermissionOnCurrentNote() const
{
    WIZDOCUMENTDATA docData = m_doc->note();
    CWizDatabase& db = m_dbMgr.db(docData.strKbGUID);
    return db.CanEditDocument(docData) && !CWizDatabase::IsInDeletedItems(docData.strLocation);
}

void MainWindow::setCurrentDocumentType(const QString &strType)
{
    WIZDOCUMENTDATA docData = m_doc->note();
    CWizDatabase& db = m_dbMgr.db(docData.strKbGUID);
    docData.strType = strType;
    db.ModifyDocumentInfoEx(docData);
}

void MainWindow::OpenURLInDefaultBrowser(const QString& strURL)
{
    bool result = QDesktopServices::openUrl(QUrl(strURL));
    if (!result)
    {
        qDebug() << "failed to open " << strURL;
    }
}

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
    setUnifiedTitleAndToolBarOnMac(true);
#endif

#ifdef Q_OS_MAC
    //m_toolBar->showInWindow(this);
    addToolBar(m_toolBar);
    m_toolBar->setAllowedAreas(Qt::TopToolBarArea);
    m_toolBar->setMovable(false);

    m_toolBar->addWidget(new CWizFixedSpacer(QSize(10, 1), m_toolBar));

    CWizUserInfoWidget* info = new CWizUserInfoWidget(*this, m_toolBar);
    m_toolBar->addWidget(info);

    //m_toolBar->addStandardItem(CWizMacToolBar::Space);
    m_toolBar->addWidget(new CWizFixedSpacer(QSize(20, 1), m_toolBar));

    m_toolBar->addAction(m_actions->actionFromName(WIZACTION_GLOBAL_SYNC));
    //m_toolBar->addStandardItem(CWizMacToolBar::Space);
    m_toolBar->addAction(m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT));
    //m_toolBar->addStandardItem(CWizMacToolBar::FlexibleSpace);
    //m_toolBar->addSearch(tr("Search"), "");
    //
    //m_search = m_toolBar->getSearchWidget();


    m_toolBar->addWidget(new CWizSpacer(m_toolBar));

    m_spacerBeforeSearch = new CWizSpacer(m_toolBar);
    m_toolBar->addWidget(m_spacerBeforeSearch);

    m_search = new CWizSearchWidget(this);

    m_toolBar->addWidget(m_search);

    m_toolBar->addWidget(new CWizFixedSpacer(QSize(20, 1), m_toolBar));

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

    m_spacerBeforeSearch = new CWizFixedSpacer(QSize(20, 1), m_toolBar);
    m_toolBar->addWidget(m_spacerBeforeSearch);

    m_search = new CWizSearchWidget(this);

    m_toolBar->addWidget(m_search);

    m_toolBar->layout()->setAlignment(m_search, Qt::AlignBottom);
    m_toolBar->addWidget(new CWizFixedSpacer(QSize(20, 1), m_toolBar));

    CWizButton* buttonNew = new CWizButton(m_toolBar);
    buttonNew->setAction(m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT));
    m_toolBar->addWidget(buttonNew);

    m_toolBar->addWidget(new CWizSpacer(m_toolBar));

    //
#endif
    //
    connect(m_search, SIGNAL(doSearch(const QString&)), SLOT(on_search_doSearch(const QString&)));
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
    //
#ifndef Q_OS_MAC
    connect(m_splitter, SIGNAL(splitterMoved(int, int)), SLOT(on_client_splitterMoved(int, int)));
#endif

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


    //sortBtn->setStyleSheet("padding-top:10px;");
    m_labelDocumentsHint->setStyleSheet("color: #787878;padding-bottom:1px;"); //font: 12px;
    m_labelDocumentsCount->setStyleSheet("color: #787878;padding-bottom:1px;"); //font: 12px;


    QWidget* line2 = new QWidget(this);
    line2->setFixedHeight(1);
    line2->setStyleSheet("border-top-width:1;border-top-style:solid;border-top-color:#DADAD9");

    layoutList->addLayout(layoutActions);
    layoutList->addWidget(line2);
    layoutList->addWidget(m_documents);

    return m_noteList;
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
    connect(m_documents, SIGNAL(lastDocumentDeleted()), SLOT(on_documents_lastDocumentDeleted()));

#ifndef Q_OS_MAC
    QTimer::singleShot(100, this, SLOT(adjustToolBarLayout()));
#endif
}

void MainWindow::on_actionAutoSync_triggered()
{
    m_sync->startSyncAll();
}

void MainWindow::on_actionSync_triggered()
{
    if (::WizIsOffline())
    {
        QMessageBox::information(this, tr("Info"), tr("Connection is not available, please check your network connection."));
    }
    else
    {
        syncAllData();
    }
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

    // password changed
    if (errorTokenInvalid == nErrorCode) {
        m_settings->setPassword("");
        if (!m_userVerifyDialog) {
            m_userVerifyDialog = new CWizUserVerifyDialog(m_dbMgr.db().GetUserId(), tr("sorry, sync failed. please input your password and try again."), this);
            connect(m_userVerifyDialog, SIGNAL(accepted()), SLOT(on_syncDone_userVerified()));
        }

        m_userVerifyDialog->exec();
    } else if (QNetworkReply::ProtocolUnknownError == nErrorCode) {
        //network avaliable, show message once
        static bool showMessageAgain = true;
        if (showMessageAgain) {
            QMessageBox messageBox(this);
            messageBox.setIcon(QMessageBox::Information);
            messageBox.setText(tr("Connection is not available, please check your network connection."));
            QAbstractButton *btnDontShowAgain =
                    messageBox.addButton(tr("Don't show this again"), QMessageBox::ActionRole);
            messageBox.addButton(QMessageBox::Ok);
            messageBox.exec();
            showMessageAgain = messageBox.clickedButton() != btnDontShowAgain;
        }
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

void MainWindow::on_actionNewNote_triggered()
{
    WIZDOCUMENTDATA data;
    if (!m_category->createDocument(data))
    {
        return;
    }

    //FIXME:这个地方存在Bug,只能在Editor为disable的情况下才能设置焦点.
    m_doc->web()->setEditorEnable(false);

    m_documentForEditing = data;
    m_documents->addAndSelectDocument(data);
    m_doc->web()->setFocus(Qt::MouseFocusReason);
    setActionsEnableForNewNote();
}

void MainWindow::on_actionEditingUndo_triggered()
{
    m_doc->web()->undo();
}

void MainWindow::on_actionEditingRedo_triggered()
{
    m_doc->web()->redo();
}

void MainWindow::on_actionEditingCut_triggered()
{
    qDebug() << "actionEditingCut called";
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

void MainWindow::on_actionFormatInsertCheckList_triggered()
{
    m_doc->web()->editorCommandExecuteInsertCheckList();
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
    AboutDialog dialog(this);
    dialog.exec();
}


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

    //FIXME: special handle for support.html, shuold append displayName in url.
    CWizDatabase& personDb = m_dbMgr.db();
    QString strUserName = "Unkown";
    personDb.GetUserDisplayName(strUserName);
    strUrl.replace(QHostInfo::localHostName(), strUserName);

    QDesktopServices::openUrl(strUrl);
}

void MainWindow::on_actionSupport_triggered()
{
    QString strUrl = WizService::ApiEntry::supportUrl();

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
    msg.setText(tr("Rebuild full text search is quit slow if you have quite a few notes or attachments, you do not have to use this function while search should work as expected."));

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
    m_search->clear();
    m_search->focus();
    m_category->restoreSelection();
    m_doc->web()->applySearchKeywordHighlight();
}

void MainWindow::on_actionSaveAsPDF_triggered()
{
    if (CWizDocumentWebView* web = m_doc->web())
    {
        QString	fileName = QFileDialog::getSaveFileName (this, QString(), QDir::homePath(), tr("PDF Files (*.pdf)"));
        if (!fileName.isEmpty())
        {
            web->saveAsPDF(fileName);
        }
    }
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
    if (WizIsKMURL(keywords)) {
        viewDocumentByWizKMURL(keywords);
        return;
    }

    m_category->saveSelection();
    m_documents->clear();
    //
    m_noteList->show();
    m_msgList->hide();
    //
    m_searcher->search(keywords, 500);
}


void MainWindow::on_searchProcess(const QString& strKeywords, const CWizDocumentDataArray& arrayDocument, bool bEnd)
{
    if (bEnd) {
        m_doc->web()->clearSearchKeywordHighlight(); //need clear hightlight first
        m_doc->web()->applySearchKeywordHighlight();
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

    CWizSettings settings(Utils::PathResolve::resourcesPath() + "files/mainmenu.ini");

    QMenu* pMenu = new QMenu(this);
    m_actions->buildMenu(pMenu, settings, pAction->objectName());

    pMenu->popup(pt);
}

void MainWindow::on_client_splitterMoved(int pos, int index)
{
    adjustToolBarLayout();
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
        if (!m_msgList->isVisible())
        {
            m_msgList->show();
            m_noteList->hide();
        }
        /*
         * 在点击MessageItem的时候,为了重新刷新当前消息,强制发送了itemSelectionChanged消息
         * 因此需要在这个地方避免重复刷新两次消息列表
         */
        static QTime lastTime(0, 0, 0);
        QTime last = lastTime;
        QTime now = QTime::currentTime();
        lastTime = now;
        if (last.msecsTo(now) < 300)
            return;

        CWizMessageDataArray arrayMsg;
        pItem->getMessages(m_dbMgr.db(), arrayMsg);
        m_msgList->setMessages(arrayMsg);
        return;
    }
    else
    {
        if (!m_noteList->isVisible())
        {
            m_noteList->show();
            m_msgList->hide();
        }
        QString kbGUID = category->selectedItemKbGUID();
        if (!kbGUID.isEmpty())
        {
            resetPermission(kbGUID, "");
        }

        category->getDocuments(arrayDocument);
        m_documents->setDocuments(arrayDocument);

        if (arrayDocument.empty())
        {
            on_documents_itemSelectionChanged();
        }
    }
}

void MainWindow::on_documents_itemSelectionChanged()
{
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
    if (wizoptionsNoteView == type) {
        m_doc->settingsChanged();
    } else if (wizoptionsSync == type) {

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
    Q_ASSERT(!data.strKbGUID.isEmpty());

    CWizDocument* doc = new CWizDocument(m_dbMgr.db(data.strKbGUID), data);

    if (doc->GUID() == m_doc->note().strGUID)
    {
        m_doc->reviewCurrentNote();
        return;
    }

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

void MainWindow::checkWizUpdate()
{
#ifndef BUILD4APPSTORE
    m_upgrade->startCheck();
#endif
}

#ifndef Q_OS_MAC


void MainWindow::adjustToolBarLayout()
{
    if (!m_toolBar)
        return;
    //
    QWidget* list = m_documents->isVisible() ? (QWidget*)m_documents : (QWidget*)m_msgList;
    //
    QPoint ptSearch = list->mapToGlobal(QPoint(0, 0));
    QPoint ptSpacerBeforeSearch = m_spacerBeforeSearch->mapToGlobal(QPoint(0, 0));
    //
    int spacerWidth = ptSearch.x() - ptSpacerBeforeSearch.x();
    if (spacerWidth < 0)
        return;
    //
    m_spacerBeforeSearch->adjustWidth(spacerWidth);
    //
    int searchWidth = list->size().width();
    if (searchWidth > 100)
    {
        m_search->setFixedWidth(searchWidth);
    }
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

void MainWindow::syncAllData()
{
    m_sync->startSyncAll(false);
    m_animateSync->startPlay();
}

void MainWindow::setActionsEnableForNewNote()
{
    m_actions->actionFromName(WIZACTION_FORMAT_BOLD)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_ITALIC)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_UNDERLINE)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_STRIKETHROUGH)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_UNORDEREDLIST)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_ORDEREDLIST)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_JUSTIFYLEFT)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_JUSTIFYRIGHT)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_JUSTIFYCENTER)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_JUSTIFYJUSTIFY)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_INDENT)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_OUTDENT)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_INSERT_TABLE)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_INSERT_LINK)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_INSERT_HORIZONTAL)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_INSERT_DATE)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_INSERT_TIME)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_INSERT_CHECKLIST)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_REMOVE_FORMAT)->setEnabled(true);
    m_actions->actionFromName(WIZACTION_FORMAT_VIEW_SOURCE)->setEnabled(true);
}

void MainWindow::viewDocumentByWizKMURL(const QString &strKMURL)
{
    CWizDatabase& db = m_dbMgr.db();
    if (!WizIsKMURLOpenDocument(strKMURL))
        return;

    QString strKbGUID = db.GetParamFromWizKMURL(strKMURL, "kbguid");
    QString strGUID = db.GetParamFromWizKMURL(strKMURL, "guid");

    WIZDOCUMENTDATA document;
    if (m_dbMgr.db(strKbGUID).DocumentFromGUID(strGUID, document))
    {
        //m_category->setCurrentItem();
        m_documents->blockSignals(true);
        m_documents->setCurrentItem(0);
        m_documents->blockSignals(false);
        viewDocument(document, true);
    }
}

void MainWindow::initTrayIcon(QSystemTrayIcon* trayIcon)
{
    Q_ASSERT(trayIcon);
    QMenu* menu = new QMenu(this);
    QAction* actionShow = menu->addAction(tr("Show/Hide MainWindow"));
    connect(actionShow, SIGNAL(triggered()), SLOT(shiftVisableStatus()));

    QAction* actionNewNote = menu->addAction(tr("New Note"));
    connect(actionNewNote, SIGNAL(triggered()), SLOT(on_trayIcon_newDocument_clicked()));

    //
    menu->addSeparator();
    QAction* actionLogout = menu->addAction(tr("Logout"));
    connect(actionLogout, SIGNAL(triggered()), SLOT(on_actionLogout_triggered()));
    QAction* actionExit = menu->addAction(tr("Exit"));
    connect(actionExit, SIGNAL(triggered()), SLOT(on_actionExit_triggered()));

    trayIcon->setContextMenu(menu);

    //
#ifdef Q_OS_MAC
    QString normal = WizGetSkinResourceFileName(userSettings().skin(), "trayIcon");
    QString selected = WizGetSkinResourceFileName(userSettings().skin(), "trayIcon_selected");
    QIcon icon;
    icon.addFile(normal, QSize(), QIcon::Normal, QIcon::Off);
    icon.addFile(selected, QSize(), QIcon::Selected, QIcon::Off);
    if (!icon.isNull())
    {
        trayIcon->setIcon(icon);
    }
#endif
}

void MainWindow::quickSyncKb(const QString& kbGuid)
{
    CWizKMSyncThread::quickSyncKb(kbGuid);
}
