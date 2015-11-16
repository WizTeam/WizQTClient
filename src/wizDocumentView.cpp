#include "wizDocumentView.h"

#include <QWebElement>
#include <QWebFrame>
#include <QWebHistory>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QApplication>
#include <QDesktopWidget>

#include <coreplugin/icore.h>

#include "wizmainwindow.h"
#include "wizDocumentTransitionView.h"
#include "wizDocumentWebEngine.h"
#include "wizEditorToolBar.h"
#include "share/wizObjectDataDownloader.h"
#include "share/wizDatabaseManager.h"
#include "widgets/wizScrollBar.h"
#include "widgets/wizLocalProgressWebView.h"
#include "wizDocumentWebView.h"
#include "wiznotestyle.h"
#include "widgets/wizSegmentedButton.h"
#include "wizButton.h"
#include "share/wizsettings.h"
#include "share/wizuihelper.h"
#include "wizusercipherform.h"
#include "wizDocumentEditStatus.h"
#include "notifybar.h"
#include "sync/token.h"
#include "sync/apientry.h"
#include "sync/wizKMServer.h"

#include "titlebar.h"

using namespace Core;
using namespace Core::Internal;


#define DOCUMENT_STATUS_NOSTATUS            0x0000
#define DOCUMENT_STATUS_GROUP                 0x0001
#define DOCUMENT_STATUS_OFFLINE               0x0002
#define DOCUMENT_STATUS_FIRSTTIMEVIEW     0x0004
#define DOCUMENT_STATUS_EDITBYOTHERS   0x0008
#define DOCUMENT_STATUS_NEWVERSIONFOUNDED      0x0010
#define DOCUMENT_STATUS_PERSONAL           0x0020
#define DOCUMENT_STATUS_ON_EDITREQUEST       0x0040

CWizDocumentView::CWizDocumentView(CWizExplorerApp& app, QWidget* parent)
    : INoteView(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_userSettings(app.userSettings())
    #ifdef USEWEBENGINE
    , m_web(new CWizDocumentWebEngine(app, this))
    , m_comments(new QWebEngineView(this))
    #else
    , m_web(new CWizDocumentWebView(app, this))
    , m_commentWidget(new CWizLocalProgressWebView(app.mainWindow()))
    #endif
    , m_title(new TitleBar(app, this))
    , m_passwordView(new CWizUserCipherForm(app, this))
    , m_viewMode(app.userSettings().noteViewMode())
    , m_transitionView(new CWizDocumentTransitionView(this))
    , m_bLocked(false)
    , m_bEditingMode(false)
    , m_noteLoaded(false)
    , m_editStatusSyncThread(new CWizDocumentEditStatusSyncThread(this))
    //, m_editStatusCheckThread(new CWizDocumentStatusCheckThread(this))
    , m_editStatus(0)
    , m_sizeHint(QSize(200, 1))
{
    m_title->setEditor(m_web);

    QVBoxLayout* layoutDoc = new QVBoxLayout();
    layoutDoc->setContentsMargins(0, 0, 0, 0);
    layoutDoc->setSpacing(0);

    m_docView = new QWidget(this);
    m_docView->setLayout(layoutDoc);

    m_tab = new QStackedWidget(this);
    //
    m_passwordView->setGeometry(this->geometry());
    connect(m_passwordView, SIGNAL(cipherCheckRequest()), SLOT(onCipherCheckRequest()));
    //
    m_msgWidget = new QWidget(this);
    QVBoxLayout* layoutMsg = new QVBoxLayout();
    m_msgWidget->setLayout(layoutMsg);
    m_msgLabel = new QLabel(m_msgWidget);
    m_msgLabel->setAlignment(Qt::AlignCenter);
    m_msgLabel->setWordWrap(true);
    layoutMsg->addWidget(m_msgLabel);
    //
    m_tab->addWidget(m_docView);
    m_tab->addWidget(m_passwordView);
    m_tab->addWidget(m_msgWidget);
    m_tab->setCurrentWidget(m_docView);    

    m_web->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_comments = m_commentWidget->web();
    QWebPage *commentPage = new QWebPage(m_comments);
    commentPage->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    m_comments->setPage(commentPage);
    m_comments->history()->setMaximumItemCount(0);
    m_comments->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    m_comments->settings()->globalSettings()->setAttribute(QWebSettings::LocalStorageEnabled, true);
    m_comments->settings()->globalSettings()->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, true);
    m_comments->page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    m_comments->setAcceptDrops(false);
    connect(m_comments, SIGNAL(loadFinished(bool)), m_title, SLOT(onCommentPageLoaded(bool)));
    connect(m_comments, SIGNAL(linkClicked(QUrl)), m_web, SLOT(onEditorLinkClicked(QUrl)));
    connect(m_comments->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            SLOT(on_comment_populateJavaScriptWindowObject()));
    connect(m_commentWidget, SIGNAL(widgetStatusChanged()), SLOT(on_commentWidget_statusChanged()));

    m_commentWidget->hide();

    QWidget* wgtEditor = new QWidget(m_docView);
    QVBoxLayout* layoutEditor = new QVBoxLayout(wgtEditor);
    layoutEditor->setSpacing(0);
    layoutEditor->setContentsMargins(0, 5, 0, 0);
    layoutEditor->addWidget(m_title);
    layoutEditor->addWidget(m_web);
    layoutEditor->setStretchFactor(m_title, 0);
    layoutEditor->setStretchFactor(m_web, 1);

    m_splitter = new CWizSplitter(this);
    m_splitter->addWidget(wgtEditor);
    m_splitter->addWidget(m_commentWidget);
    m_splitter->setOrientation(Qt::Horizontal);

    layoutDoc->addWidget(m_splitter);
//    layoutDoc->setStretchFactor(m_title, 0);
//    layoutDoc->setStretchFactor(m_splitter, 1);

#ifdef USEWEBENGINE
    QLineEdit *commandLine = new QLineEdit(this);
    layoutDoc->addWidget(commandLine);
    connect(commandLine, SIGNAL(returnPressed()), SLOT(on_command_request()));
#endif

    QVBoxLayout* layoutMain = new QVBoxLayout(this);
    layoutMain->setContentsMargins(0, 0, 0, 0);
    setLayout(layoutMain);
    layoutMain->addWidget(m_tab);

    //
    layoutMain->addWidget(m_transitionView);
    m_transitionView->hide();

    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    m_downloaderHost = mainWindow->downloaderHost();
    connect(m_downloaderHost, SIGNAL(downloadDone(const WIZOBJECTDATA&, bool)),
            SLOT(on_download_finished(const WIZOBJECTDATA&, bool)));

    connect(&m_dbMgr, SIGNAL(documentModified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)), \
            SLOT(on_document_modified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(documentDataModified(const WIZDOCUMENTDATA&)),
            SLOT(on_document_data_modified(const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(attachmentCreated(const WIZDOCUMENTATTACHMENTDATA&)), \
            SLOT(on_attachment_created(const WIZDOCUMENTATTACHMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(attachmentDeleted(const WIZDOCUMENTATTACHMENTDATA&)), \
            SLOT(on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(documentUploaded(QString,QString)), \
            m_editStatusSyncThread, SLOT(documentUploaded(QString,QString)));

    connect(Core::ICore::instance(), SIGNAL(viewNoteRequested(Core::INoteView*,const WIZDOCUMENTDATA&)),
            SLOT(onViewNoteRequested(Core::INoteView*,const WIZDOCUMENTDATA&)));

    connect(Core::ICore::instance(), SIGNAL(viewNoteLoaded(Core::INoteView*,WIZDOCUMENTDATA,bool)),
            SLOT(onViewNoteLoaded(Core::INoteView*,const WIZDOCUMENTDATA&,bool)));

    connect(Core::ICore::instance(), SIGNAL(closeNoteRequested(Core::INoteView*)),
            SLOT(onCloseNoteRequested(Core::INoteView*)));

    connect(m_web, SIGNAL(focusIn()), SLOT(on_webView_focus_changed()));

    connect(m_title, SIGNAL(notifyBar_link_clicked(QString)), SLOT(on_notifyBar_link_clicked(QString)));
    connect(m_title, SIGNAL(loadComment_request(QString)), SLOT(on_loadComment_request(QString)), Qt::QueuedConnection);

//    connect(m_editStatusCheckThread, SIGNAL(checkFinished(QString,QStringList)),
//            SLOT(on_checkEditStatus_finished(QString,QStringList)));
//    connect(m_editStatusCheckThread, SIGNAL(checkDocumentChangedFinished(QString,bool)),
//            SLOT(on_checkDocumentChanged_finished(QString,bool)));
//    connect(m_editStatusCheckThread, SIGNAL(checkTimeOut(QString)),
//            SLOT(on_checkEditStatus_timeout(QString)));

    //
    m_editStatusSyncThread->start(QThread::IdlePriority);
//    m_editStatusCheckThread->start(QThread::IdlePriority);

    m_editStatusChecker = new CWizDocumentStatusChecker();
    connect(this, SIGNAL(checkDocumentEditStatusRequest(QString,QString)), m_editStatusChecker,
            SLOT(checkEditStatus(QString,QString)));
    connect(this, SIGNAL(stopCheckDocumentEditStatusRequest(QString,QString)),
            m_editStatusChecker, SLOT(stopCheckStatus(QString,QString)));
    connect(m_editStatusChecker, SIGNAL(checkEditStatusFinished(QString,bool)), \
            SLOT(on_checkEditStatus_finished(QString,bool)));
    connect(m_editStatusChecker, SIGNAL(checkTimeOut(QString)), \
            SLOT(on_checkEditStatus_timeout(QString)));
    connect(m_editStatusChecker, SIGNAL(documentEditingByOthers(QString,QStringList)), \
            SLOT(on_documentEditingByOthers(QString,QStringList)));
    connect(m_editStatusChecker, SIGNAL(checkDocumentChangedFinished(QString,bool)), \
            SLOT(on_checkDocumentChanged_finished(QString,bool)));

    QThread* checkThread = new QThread(this);
    connect(checkThread, SIGNAL(started()), m_editStatusChecker, SLOT(initialise()));
    connect(checkThread, SIGNAL(finished()), m_editStatusChecker, SLOT(clearTimers()));
    m_editStatusChecker->moveToThread(checkThread);
    checkThread->start();
}

CWizDocumentView::~CWizDocumentView()
{
    if (m_editStatusChecker)
        delete m_editStatusChecker;
}

QSize CWizDocumentView::sizeHint() const
{
    return m_sizeHint;
}

void CWizDocumentView::setSizeHint(QSize size)
{
    m_sizeHint = size;
}

void CWizDocumentView::waitForDone()
{
    m_editStatusChecker->thread()->quit();

    m_web->saveDocument(m_note, false);
    //
    m_web->waitForDone();
    //
    m_editStatusSyncThread->waitForDone();
//    m_editStatusCheckThread->waitForDone();

}

QWidget* CWizDocumentView::client() const
{
    return m_tab;
}

QWebView*CWizDocumentView::commentView() const
{
    return m_commentWidget->web();
}

CWizLocalProgressWebView*CWizDocumentView::commentWidget() const
{
    return m_commentWidget;
}

CWizDocumentTransitionView* CWizDocumentView::transitionView()
{
    return m_transitionView;
}

TitleBar*CWizDocumentView::titleBar()
{
    return m_title;
}
void CWizDocumentView::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
}

void CWizDocumentView::resizeEvent(QResizeEvent* ev)
{
    QWidget::resizeEvent(ev);

    m_title->editorToolBar()->adjustButtonPosition();
}

void CWizDocumentView::showClient(bool visible)
{
    m_tab->setVisible(visible);
}

void CWizDocumentView::onViewNoteRequested(INoteView* view, const WIZDOCUMENTDATA& doc)
{
    if (view != this)
        return;

    if (doc.tCreated.secsTo(QDateTime::currentDateTime()) <= 1) {
        viewNote(doc, true);
        m_title->moveTitileTextToPlaceHolder();
    } else {
        m_title->clearPlaceHolderText();
        viewNote(doc, false);
    }
}

void CWizDocumentView::onViewNoteLoaded(INoteView* view, const WIZDOCUMENTDATA& doc, bool bOk)
{
    if (view != this)
        return;

    showClient(bOk);
}

bool CWizDocumentView::reload()
{
    bool ret = m_dbMgr.db(m_note.strKbGUID).DocumentFromGUID(m_note.strGUID, m_note);
    m_title->updateInfo(note());

    return ret;
}

void CWizDocumentView::reloadNote()
{
    reload();
    m_web->reloadNoteData(note());
}

bool CWizDocumentView::defaultEditingMode()
{
    switch (m_viewMode) {
    case viewmodeAlwaysEditing:
        return true;
    case viewmodeAlwaysReading:
        return false;
    default:
        return  m_bEditingMode; // default is Reading Mode
    }
}

void CWizDocumentView::initStat(const WIZDOCUMENTDATA& data, bool bEditing)
{
    m_bLocked = false;
    int nLockReason = -1;

    if (m_dbMgr.db(data.strKbGUID).IsGroup()) {
        if (!bEditing) {
            nLockReason = NotifyBar::LockForGruop;
            m_bLocked = true;
        }
    } else if (!m_dbMgr.db(data.strKbGUID).CanEditDocument(data)) {
        nLockReason = NotifyBar::PermissionLack;
        m_bLocked = true;
    } else if (CWizDatabase::IsInDeletedItems(data.strLocation)) {
        nLockReason = NotifyBar::Deleted;
        m_bLocked = true;
    }

    if (m_bLocked) {
        m_bEditingMode = false;
    } else {
        m_bEditingMode = bEditing ? true : defaultEditingMode();
    }

    bool bGroup = m_dbMgr.db(data.strKbGUID).IsGroup();
    m_editStatus = m_editStatus & DOCUMENT_STATUS_PERSONAL;
    if (bGroup)
    {
        m_editStatus = m_editStatus | DOCUMENT_STATUS_GROUP | DOCUMENT_STATUS_FIRSTTIMEVIEW;
    }
    if (::WizIsDocumentContainsFrameset(data))
    {
        m_bEditingMode = false;
    }
    m_title->setLocked(m_bLocked, nLockReason, bGroup);
    if (NotifyBar::LockForGruop == nLockReason)
    {
        startCheckDocumentEditStatus();
    }

    if(bEditing)
    {
        showCoachingTips();
    }
}

void CWizDocumentView::viewNote(const WIZDOCUMENTDATA& data, bool forceEdit)
{
    m_web->closeDocument(m_note);
    m_web->saveDocument(m_note, false);
    if (m_dbMgr.db(m_note.strKbGUID).IsGroup())
    {
        stopDocumentEditingStatus();
    }

    m_noteLoaded = false;
    m_note = data;
    initStat(data, forceEdit);

    if (m_tab->currentWidget() != m_docView) {
        m_tab->setCurrentWidget(m_docView);
    }
    m_tab->setVisible(true);

    // download document if not exist
    CWizDatabase& db = m_dbMgr.db(data.strKbGUID);
    QString strDocumentFileName = db.GetDocumentFileName(data.strGUID);
    if (!db.IsObjectDataDownloaded(data.strGUID, "document") || \
            !PathFileExists(strDocumentFileName))
    {
        downloadNoteFromServer(data);

        return;
    }

    // ask user cipher if needed
    if (data.nProtected) {
        if(!db.loadUserCert()) {
            return;
        }

        if (db.userCipher().isEmpty()) {
            m_passwordView->setHint(db.userCipherHint());
            m_tab->setCurrentWidget(m_passwordView);
            m_passwordView->setCipherEditorFocus();

            return;
        }
    }

    loadNote(data);
    WIZDOCUMENTDATA docData = data;
    docData.nReadCount ++;
    db.ModifyDocumentReadCount(docData);
    docData.tAccessed = WizGetCurrentTime();
    db.ModifyDocumentDateAccessed(docData);
}

void CWizDocumentView::reviewCurrentNote()
{
    Q_ASSERT(!m_note.strGUID.isEmpty());

    m_tab->setVisible(true);
    initStat(m_note, m_bEditingMode);

    // download document if not exist
    CWizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
    QString strDocumentFileName = db.GetDocumentFileName(m_note.strGUID);
    if (!db.IsObjectDataDownloaded(m_note.strGUID, "document") || \
            !PathFileExists(strDocumentFileName))
    {
        downloadNoteFromServer(m_note);

        return;
    }

    // ask user cipher if needed
    if (m_note.nProtected) {
        if(!db.loadUserCert()) {
            return;
        }

        if (db.userCipher().isEmpty()) {
            m_passwordView->setHint(db.userCipherHint());
            m_tab->setCurrentWidget(m_passwordView);
            m_passwordView->setCipherEditorFocus();

            return;
        }
    }

    if (m_tab->currentWidget() != m_docView) {
        m_tab->setCurrentWidget(m_docView);
    }
}

void CWizDocumentView::setEditNote(bool bEdit)
{
    if (m_bLocked)
        return;

    if (::WizIsDocumentContainsFrameset(m_note))
    {
        m_title->showMessageTips(Qt::PlainText, tr("Note type is %1, do not support edit mode.").arg(m_note.strFileType));
        return;
    }

    bool isGroupNote =m_dbMgr.db(m_note.strKbGUID).IsGroup();
    if (bEdit && isGroupNote)
    {
        // don not use message tips when check document editable
//        m_title->showMessageTips(Qt::PlainText, tr("Checking whether note is eiditable..."));
        m_title->startEditButtonAnimation();
        if (!checkDocumentEditable())
        {
            return;
        }
        // stop check document edit status while enter editing mode
        stopCheckDocumentEditStatus();
    }

    m_bEditingMode = bEdit;

    if (!bEdit)
    {
        m_editStatus = DOCUMENT_STATUS_NOSTATUS;

        // 保存标题，防止因多线程保存引起覆盖
        m_title->onTitleEditFinished();
        m_title->hideMessageTips(false);
    }
    m_title->setEditingDocument(bEdit);
    m_web->setEditingDocument(bEdit);

    if (isGroupNote)
    {
        if (m_bEditingMode)
        {
            sendDocumentEditingStatus();
        }
        else
        {
            stopDocumentEditingStatus();
            startCheckDocumentEditStatus();
        }
    }
}

void CWizDocumentView::setViewMode(int mode)
{
    m_viewMode = mode;

    switch (m_viewMode)
    {
    case viewmodeAlwaysEditing:
        setEditNote(true);
        break;
    case viewmodeAlwaysReading:
        setEditNote(false);
        break;
    default:
//        Q_ASSERT(0);           //when set to the auto mode,do nonthing
        break;
    }
}

void CWizDocumentView::settingsChanged()
{
    setViewMode(m_userSettings.noteViewMode());
}

void CWizDocumentView::sendDocumentSavedSignal(const QString& strGUID, const QString& strKbGUID)
{
    CWizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
    if (db.IsGroup())
    {
        QString strUserAlias = db.GetUserAlias();
        m_editStatusSyncThread->documentSaved(strUserAlias, strKbGUID, strGUID);
    }

    emit documentSaved(strGUID, this);
}

void CWizDocumentView::resetTitle(const QString& strTitle)
{
    m_title->resetTitle(strTitle);
}

void CWizDocumentView::promptMessage(const QString &strMsg)
{
    m_tab->setCurrentWidget(m_msgWidget);
    m_tab->setVisible(true);

    m_msgLabel->setText(strMsg);
}

bool CWizDocumentView::checkListClickable()
{
    CWizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
    if (db.CanEditDocument(m_note))
    {
        //m_title->showMessageTips(Qt::PlainText, tr("Checking whether checklist is clickable..."));
        setCursor(Qt::WaitCursor);
        return checkDocumentEditable();
    }
    return false;
}

void CWizDocumentView::setStatusToEditingByCheckList()
{
    stopCheckDocumentEditStatus();
    sendDocumentEditingStatus();
    m_title->showMessageTips(Qt::PlainText, tr("You have occupied this note by clicking checklist !  " \
                                               "Switch to other notes to free this note."));
}

void CWizDocumentView::showCoachingTips()
{
    m_title->editorToolBar()->showCoachingTips();
    m_title->showCoachingTips();
}

void CWizDocumentView::setEditorFocus()
{
    m_web->setFocus(Qt::MouseFocusReason);
    m_web->editorFocus();
}

QWebFrame* CWizDocumentView::noteFrame()
{
#ifdef USEWEBENGINE
    return 0;
#else
//    return m_web->noteFrame();
    return m_web->page()->mainFrame();
#endif
}

QWebEnginePage*CWizDocumentView::notePage()
{
#ifdef USEWEBENGINE
    return m_engine->page();
#else
    return 0;
#endif
}

void CWizDocumentView::on_attachment_created(const WIZDOCUMENTATTACHMENTDATA& attachment)
{
    if (attachment.strDocumentGUID != note().strGUID)
        return;

    reload();
}

void CWizDocumentView::on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA& attachment)
{
    if (attachment.strDocumentGUID != note().strGUID)
        return;

    reload();
}

void CWizDocumentView::on_checkEditStatus_finished(const QString& strGUID, bool editable)
{
//    qDebug() << "check eidt status finished , editable  : " << editable;
    if (strGUID == m_note.strGUID)
    {
        if (editable)
        {
            CWizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
            WIZDOCUMENTDATA doc;
            db.DocumentFromGUID(strGUID, doc);
            if (db.CanEditDocument(doc) && !CWizDatabase::IsInDeletedItems(doc.strLocation))
            {
                //            qDebug() << "document editable , hide message tips.";
                m_title->hideMessageTips(true);
                m_title->setEditButtonState(true, false);
                m_bLocked = false;
            }
        }
        stopCheckDocumentAnimations();
    }
}

void CWizDocumentView::loadNote(const WIZDOCUMENTDATA& doc)
{
    m_web->viewDocument(doc, m_bEditingMode);
    m_title->setNote(doc, m_bEditingMode, m_bLocked);

    // save last
    m_note = doc;
    //
    m_noteLoaded = true;
    //
    if (m_bEditingMode && m_web->hasFocus())
    {
        sendDocumentEditingStatus();
    }
}

void CWizDocumentView::downloadNoteFromServer(const WIZDOCUMENTDATA& note)
{
    connect(m_downloaderHost, SIGNAL(downloadProgress(QString,int,int)),
            m_transitionView, SLOT(onDownloadProgressChanged(QString,int,int)), Qt::UniqueConnection);
    m_downloaderHost->downloadDocument(note);
    showClient(false);
    m_transitionView->showAsMode(note.strGUID, CWizDocumentTransitionView::Downloading);
}

void CWizDocumentView::sendDocumentEditingStatus()
{
    CWizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
    if (db.IsGroup())
    {
        QString strUserAlias = db.GetUserAlias();
        m_editStatusSyncThread->startEditingDocument(strUserAlias, m_note.strKbGUID, m_note.strGUID);
    }
}

void CWizDocumentView::stopDocumentEditingStatus()
{
    WIZDOCUMENTDATA doc = m_note;
    if (m_dbMgr.db(doc.strKbGUID).DocumentFromGUID(doc.strGUID, doc))
    {
        bool bModified = doc.nVersion == -1;
        m_editStatusSyncThread->stopEditingDocument(doc.strKbGUID, doc.strGUID, bModified);
    }
}

void CWizDocumentView::startCheckDocumentEditStatus()
{
    m_editStatus = DOCUMENT_STATUS_NOSTATUS;
    emit checkDocumentEditStatusRequest(m_note.strKbGUID, m_note.strGUID);
}

void CWizDocumentView::stopCheckDocumentEditStatus()
{
    emit stopCheckDocumentEditStatusRequest(m_note.strKbGUID, m_note.strGUID);
}

bool CWizDocumentView::checkDocumentEditable()
{
    QEventLoop loop;
    connect(m_editStatusChecker, SIGNAL(checkEditStatusFinished(QString,bool)), &loop, SLOT(quit()));
    connect(m_editStatusChecker, SIGNAL(checkTimeOut(QString)), &loop, SLOT(quit()));
    startCheckDocumentEditStatus();
    m_editStatus = m_editStatus | DOCUMENT_STATUS_ON_EDITREQUEST;
    loop.exec();
    //

    bool editByOther = m_editStatus & DOCUMENT_STATUS_EDITBYOTHERS;
    bool newVersion = m_editStatus & DOCUMENT_STATUS_NEWVERSIONFOUNDED;
    bool isOffLine = m_editStatus & DOCUMENT_STATUS_OFFLINE;
    return !(editByOther || newVersion) || isOffLine;
}

void CWizDocumentView::stopCheckDocumentAnimations()
{
    if (cursor().shape() != Qt::ArrowCursor)
    {
        setCursor(Qt::ArrowCursor);
    }
    m_title->stopEditButtonAnimation();
}

void CWizDocumentView::on_document_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew)
{
    Q_UNUSED(documentOld);

    if (note().strGUID != documentNew.strGUID)
        return;

    reload();
}

void CWizDocumentView::onCloseNoteRequested(INoteView *view)
{
    Q_UNUSED(view)

    showClient(false);
}

void CWizDocumentView::onCipherCheckRequest()
{
    const WIZDOCUMENTDATA& noteData = note();
    CWizDatabase& db = m_dbMgr.db(noteData.strKbGUID);

    db.setUserCipher(m_passwordView->userCipher());
    db.setSaveUserCipher(m_passwordView->isSaveForSession());
    m_app.userSettings().setRememberNotePasswordForSession(m_passwordView->isSaveForSession());

    if (!db.IsFileAccessible(noteData))
    {
        m_passwordView->cipherError();
        db.setUserCipher(QString());
        db.setSaveUserCipher(false);
        return;
    }
    m_passwordView->cipherCorrect();

    m_tab->setCurrentWidget(m_docView);
    loadNote(noteData);
}

void CWizDocumentView::on_download_finished(const WIZOBJECTDATA &data, bool bSucceed)
{
    if (m_note.strKbGUID != data.strKbGUID
            || m_note.strGUID != data.strObjectGUID)
        return;

    m_transitionView->setVisible(false);

    if (!bSucceed)
    {
        m_transitionView->showAsMode(data.strObjectGUID, CWizDocumentTransitionView::ErrorOccured);
        return;
    }

    if (m_bEditingMode)
        return;


    bool onEditRequest = m_editStatus & DOCUMENT_STATUS_ON_EDITREQUEST;

    viewNote(m_note, onEditRequest ? true : m_bEditingMode);
}

void CWizDocumentView::on_document_data_modified(const WIZDOCUMENTDATA& data)
{
    //verify m_noteLoaded before reload
    if (note().strGUID != data.strGUID || !m_noteLoaded)
        return;

    reloadNote();
}

void CWizDocumentView::on_document_data_changed(const QString& strGUID,
                                              CWizDocumentView* viewer)
{
    if (viewer != this && strGUID == note().strGUID && !m_bEditingMode)
    {
        reloadNote();
    }
}


void Core::CWizDocumentView::on_documentEditingByOthers(QString strGUID, QStringList editors)
{
    //
    //QString strCurrentUser = m_dbMgr.db(m_note.strKbGUID).GetUserAlias();
    //editors.removeAll(strCurrentUser);

    if (strGUID == m_note.strGUID)
    {
        if (!editors.isEmpty())
        {
            QString strEditor = editors.join(" , ");
            if (!strEditor.isEmpty())
            {
                CWizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
                QString strUserAlias = db.GetUserAlias();
                if (editors.count() == 1 && editors.first() == strUserAlias)
                {
                    qDebug() << "[EditStatus]:editing by myself.";
                    m_title->hideMessageTips(true);
                    m_title->setEditButtonState(true, false);
                    m_editStatus = m_editStatus & ~DOCUMENT_STATUS_EDITBYOTHERS;
                }
                else
                {
                    m_title->showMessageTips(Qt::PlainText, QString(tr("%1 is currently editing this note. Note has been locked.")).arg(strEditor));
                    m_editStatus = m_editStatus | DOCUMENT_STATUS_EDITBYOTHERS;
                    if (m_note.nVersion == -1)
                    {
                        m_title->showMessageTips(Qt::PlainText, QString(tr("%1 is currently editing this note.")).arg(strEditor));
                        m_title->setEditButtonState(true, false);
                    }
                    else
                    {
                        m_title->setEditButtonState(false, false);
                    }
                }
            }
        }
        else
        {
            m_title->setEditButtonState(!m_bLocked, false);
            m_editStatus = m_editStatus & ~DOCUMENT_STATUS_EDITBYOTHERS;
        }
    }
}

void CWizDocumentView::on_checkEditStatus_timeout(const QString& strGUID)
{
    if (strGUID == m_note.strGUID)
    {
        if (m_editStatus & DOCUMENT_STATUS_ON_EDITREQUEST)
        {
            m_title->showMessageTips(Qt::RichText, tr("The current network in poor condition, you are <b> offline editing mode </b>."));
        }
        m_title->setEditButtonState(true, false);
        m_editStatus = m_editStatus | DOCUMENT_STATUS_OFFLINE;
        m_bLocked = false;
        stopCheckDocumentAnimations();
    }
}

void CWizDocumentView::on_checkDocumentChanged_finished(const QString& strGUID, bool changed)
{
    if (strGUID == m_note.strGUID)
    {       
        if (changed)
        {
//            if (m_status & DOCUMENT_FIRSTTIMEVIEW)
//            {
//                // downlaod document data when document changed at first time to view document
//                downloadDocumentFromServer();
//            }
//            else
//            {
                m_title->showMessageTips(Qt::RichText, QString(tr("New version on server avalible. <a href='%1'>Click to download new version.<a>")).arg(NOTIFYBAR_LABELLINK_DOWNLOAD));
//            }
                m_editStatus |= DOCUMENT_STATUS_NEWVERSIONFOUNDED;
        }
        else
        {
            m_bLocked = false;
            int nLockReason = -1;
            m_bEditingMode = false;

            const WIZDOCUMENTDATA& doc = note();
            if (!m_dbMgr.db(doc.strKbGUID).CanEditDocument(doc)) {
                nLockReason = NotifyBar::PermissionLack;
                m_bLocked = true;
            } else if (CWizDatabase::IsInDeletedItems(doc.strLocation)) {
                nLockReason = NotifyBar::Deleted;
                m_bLocked = true;
            }

            if (m_bLocked)
            {
                bool bGroup = m_dbMgr.db(doc.strKbGUID).IsGroup();
                m_title->setLocked(m_bLocked, nLockReason, bGroup);
            }

            m_editStatus &= ~DOCUMENT_STATUS_NEWVERSIONFOUNDED;
        }
        m_editStatus = m_editStatus & ~DOCUMENT_STATUS_FIRSTTIMEVIEW;
    }
}

void CWizDocumentView::on_syncDatabase_request(const QString& strKbGUID)
{
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    mainWindow->quickSyncKb(strKbGUID);
}

void CWizDocumentView::on_webView_focus_changed()
{
    if (m_web->hasFocus())
    {
        sendDocumentEditingStatus();
    }
}

void CWizDocumentView::on_notifyBar_link_clicked(const QString& link)
{
    if (link == NOTIFYBAR_LABELLINK_DOWNLOAD)
    {
        CWizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
        QString strDocumentFileName = db.GetDocumentFileName(m_note.strGUID);
        WizDeleteFile(strDocumentFileName);

        downloadNoteFromServer(m_note);
    }
}

void CWizDocumentView::on_command_request()
{
    QLineEdit* edit = qobject_cast<QLineEdit*>(sender());
    if (edit)
    {
#ifdef USEWEBENGINE
        m_web->page()->runJavaScript(edit->text());
#else
        m_web->page()->mainFrame()->evaluateJavaScript(edit->text());
#endif
    }
}

void CWizDocumentView::on_comment_populateJavaScriptWindowObject()
{
    m_comments->page()->mainFrame()->addToJavaScriptWindowObject("WizExplorerApp", m_app.object());
}

void CWizDocumentView::on_loadComment_request(const QString& url)
{
    m_comments->load(url);
}

void CWizDocumentView::on_commentWidget_statusChanged()
{    
    if (m_web->isInSeperateWindow())
    {
        int commentWidth = 271;
        int maxWidth = maximumWidth();
        if (!WizIsHighPixel())
        {
            if (qApp->desktop()->availableGeometry().width() < 1440)
            {
                maxWidth = 916;
            }
            maxWidth = m_commentWidget->isVisible() ? (maxWidth + commentWidth) : (maxWidth - commentWidth);
        }
        if (width() > 1000)
        {
            m_commentWidget->setFixedWidth(271);
        }
        else
        {
            m_commentWidget->setMinimumWidth(0);
            m_commentWidget->setMaximumWidth(500);
        }
        setMaximumWidth(maxWidth);
        setSizeHint(QSize(maxWidth, 1));

    }

    m_title->editorToolBar()->adjustButtonPosition();
}



