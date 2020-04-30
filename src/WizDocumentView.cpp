#include "WizDocumentView.h"

#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QApplication>
#include <QDesktopWidget>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QSpacerItem>

#include "share/WizGlobal.h"

#include "share/WizObjectDataDownloader.h"
#include "share/WizDatabaseManager.h"
#include "share/WizSettings.h"
#include "share/WizUIHelper.h"
#include "sync/WizToken.h"
#include "sync/WizApiEntry.h"
#include "sync/WizKMServer.h"
#include "widgets/WizScrollBar.h"
#include "widgets/WizLocalProgressWebView.h"
#include "widgets/WizSegmentedButton.h"
#include "widgets/WizTipsWidget.h"
#include "WizDocumentWebView.h"
#include "WizEditorToolBar.h"
#include "WizNoteStyle.h"
#include "WizDocumentTransitionView.h"
#include "WizDocumentWebView.h"
#include "WizButton.h"
#include "WizUserCipherForm.h"
#include "WizDocumentEditStatus.h"
#include "WizNotifyBar.h"
#include "WizMainWindow.h"

#include "WizTitleBar.h"

#include "share/WizThreads.h"
#include "share/WizWebEngineView.h"
#include "share/WizEnc.h"


#define DOCUMENT_STATUS_NOSTATUS            0x0000
#define DOCUMENT_STATUS_GROUP                 0x0001
#define DOCUMENT_STATUS_OFFLINE               0x0002
#define DOCUMENT_STATUS_FIRSTTIMEVIEW     0x0004
#define DOCUMENT_STATUS_EDITBYOTHERS   0x0008
#define DOCUMENT_STATUS_NEWVERSIONFOUNDED      0x0010
#define DOCUMENT_STATUS_PERSONAL           0x0020
#define DOCUMENT_STATUS_ON_EDITREQUEST       0x0040
#define DOCUMENT_STATUS_ON_CHECKLIST       0x0080

WizDocumentView::WizDocumentView(WizExplorerApp& app, QWidget* parent)
    : QWidget(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_userSettings(app.userSettings())
    , m_commentWidget(new WizLocalProgressWebView({{"WizExplorerApp", m_app.object()}}, app.mainWindow()))
    , m_title(new WizTitleBar(app, this))
    , m_passwordView(new WizUserCipherForm(app, this))
    , m_defaultViewMode(app.userSettings().noteViewMode())
    , m_transitionView(new WizDocumentTransitionView(this))
    , m_bLocked(false)
    , m_editorMode(modeReader)
    , m_noteLoaded(false)
    , m_editStatusSyncThread(new WizDocumentEditStatusSyncThread(this))
    , m_editStatus(0)
    , m_sizeHint(QSize(200, 1))
    , m_comments(NULL)
{
    QVBoxLayout* layoutDoc = new QVBoxLayout();
    layoutDoc->setContentsMargins(0, 0, 0, 0);
    layoutDoc->setSpacing(0);

    m_docView = new QWidget(this);
    m_docView->setLayout(layoutDoc);
    //
    m_tab = new QStackedWidget(this);
    //
    if (isDarkMode()) {
        setAutoFillBackground(true);
        setStyleSheet("background-color:#272727;");
        m_tab->setAutoFillBackground(true);
        m_tab->setStyleSheet("background-color:#272727;");
    }

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
    m_blankView = new QWidget(this);
    //
    m_tab->addWidget(m_docView);
    m_tab->addWidget(m_passwordView);
    m_tab->addWidget(m_msgWidget);
    m_tab->addWidget(m_transitionView);
    m_tab->addWidget(m_blankView);
    m_tab->setCurrentWidget(m_blankView);
    m_tab->setBackgroundRole(QPalette::HighlightedText);

    m_comments = m_commentWidget->web();
    //m_comments->history()->setMaximumItemCount(0);
    m_comments->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    //m_comments->page()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    m_comments->setAcceptDrops(false);
    connect(m_comments, SIGNAL(loadFinishedEx(bool)), m_title, SLOT(onCommentPageLoaded(bool)));

    connect(m_commentWidget, SIGNAL(widgetStatusChanged()), SLOT(on_commentWidget_statusChanged()));

    m_commentWidget->hide();

    QWidget* wgtEditor = new QWidget(m_docView);
    //
    //使用一个widget包含webview，否则夜间模式下新建编辑，界面容易出现晃动
    QWidget* webContainer = new QWidget(wgtEditor);
    webContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_web = new WizDocumentWebView(app, webContainer);
    m_web->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout* webContainerLayout = new QVBoxLayout(webContainer);
    webContainerLayout->setMargin(0);
    webContainerLayout->setSpacing(0);
    webContainerLayout->addWidget(m_web);
    //
    m_title->setEditor(m_web);
    //
    QWebEnginePage* commentsPage = m_comments->page();
    connect(commentsPage, SIGNAL(linkClicked(QUrl, QWebEnginePage::NavigationType, bool, WizWebEnginePage*)), m_web, SLOT(onEditorLinkClicked(QUrl, QWebEnginePage::NavigationType, bool, WizWebEnginePage*)));

    QVBoxLayout* layoutEditor = new QVBoxLayout(wgtEditor);
    //
    layoutEditor->setSpacing(0);
    layoutEditor->setContentsMargins(0, 5, 0, 0);
    layoutEditor->addWidget(m_title);
    layoutEditor->addWidget(webContainer);
    layoutEditor->setStretchFactor(m_title, 0);
    layoutEditor->setStretchFactor(webContainer, 1);

    //
    m_splitter = new WizSplitter(this);
    m_splitter->addWidget(wgtEditor);
    m_splitter->addWidget(m_commentWidget);
    m_splitter->setOrientation(Qt::Horizontal);

    layoutDoc->addWidget(m_splitter);
//    layoutDoc->setStretchFactor(m_title, 0);
//    layoutDoc->setStretchFactor(m_splitter, 1);

    QVBoxLayout* layoutMain = new QVBoxLayout(this);
    layoutMain->setContentsMargins(0, 0, 0, 0);
    setLayout(layoutMain);
    layoutMain->addWidget(m_tab);

    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(m_app.mainWindow());
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

    connect(WizGlobal::instance(), SIGNAL(viewNoteRequested(WizDocumentView*,const WIZDOCUMENTDATAEX&,bool)),
            SLOT(onViewNoteRequested(WizDocumentView*,const WIZDOCUMENTDATAEX&,bool)));

    connect(WizGlobal::instance(), SIGNAL(viewNoteLoaded(WizDocumentView*,WIZDOCUMENTDATAEX,bool)),
            SLOT(onViewNoteLoaded(WizDocumentView*,const WIZDOCUMENTDATAEX&,bool)));

    connect(WizGlobal::instance(), SIGNAL(closeNoteRequested(WizDocumentView*)),
            SLOT(onCloseNoteRequested(WizDocumentView*)));

    connect(m_web, SIGNAL(focusIn()), SLOT(on_webView_focus_changed()));

    connect(m_title, SIGNAL(notifyBar_link_clicked(QString)), SLOT(on_notifyBar_link_clicked(QString)));
    connect(m_title, SIGNAL(loadComment_request(QString)), SLOT(on_loadComment_request(QString)), Qt::QueuedConnection);

    //
    m_editStatusSyncThread->start(QThread::IdlePriority);

    m_editStatusChecker = new WizDocumentStatusChecker();
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

WizDocumentView::~WizDocumentView()
{
    m_comments->disconnect();
    disconnect();
    //
    if (m_editStatusChecker)
        delete m_editStatusChecker;
}

QSize WizDocumentView::sizeHint() const
{
    return m_sizeHint;
}

void WizDocumentView::setSizeHint(QSize size)
{
    m_sizeHint = size;
}

void WizDocumentView::waitForDone()
{
    m_editStatusChecker->thread()->quit();
    m_editStatusSyncThread->waitForDone();
    //
    //
    bool done = false;
    m_web->trySaveDocument(m_note, false, [=, &done](const QVariant& ret){

        m_web->waitForDone();
        //
        done = true;
    });
    //
    while (!done)
    {
        QApplication::processEvents();
    }
}

QWidget* WizDocumentView::client() const
{
    return m_tab;
}

WizWebEngineView*WizDocumentView::commentView() const
{
    return m_commentWidget->web();
}

WizLocalProgressWebView*WizDocumentView::commentWidget() const
{
    return m_commentWidget;
}

WizDocumentTransitionView* WizDocumentView::transitionView()
{
    return m_transitionView;
}

WizTitleBar*WizDocumentView::titleBar()
{
    return m_title;
}
void WizDocumentView::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
}

void WizDocumentView::resizeEvent(QResizeEvent* ev)
{
    QWidget::resizeEvent(ev);
    qDebug() << "oldSize: " << ev->oldSize() << ", newSize: " << ev->size();

    m_title->editorToolBar()->adjustButtonPosition();
}

void WizDocumentView::onViewNoteRequested(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool forceEditing)
{
    if (view != this)
        return;

    if (doc.tCreated.secsTo(QDateTime::currentDateTime()) <= 1) {
        //new note
        viewNote(doc, forceEditing);
        m_title->clearAndSetPlaceHolderText(doc.strTitle);
    } else {
        m_title->clearPlaceHolderText();
        viewNote(doc, forceEditing);
    }
}

void WizDocumentView::onViewNoteLoaded(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool bOk)
{
}

bool WizDocumentView::reload()
{
    bool ret = m_dbMgr.db(m_note.strKbGUID).documentFromGuid(m_note.strGUID, m_note);
    m_title->updateInfo(note());

    return ret;
}

void WizDocumentView::reloadNote()
{
    reload();
    m_web->reloadNoteData(note());
}


void WizDocumentView::initStat(const WIZDOCUMENTDATA& data, bool forceEdit)
{
    m_bLocked = false;
    int nLockReason = -1;

    if (m_dbMgr.db(data.strKbGUID).isGroup()) {
        if (!forceEdit) {
            nLockReason = WizNotifyBar::LockForGruop;
            m_bLocked = true;
        }
    } else if (!m_dbMgr.db(data.strKbGUID).canEditDocument(data)) {
        nLockReason = WizNotifyBar::PermissionLack;
        m_bLocked = true;
    } else if (WizDatabase::isInDeletedItems(data.strLocation)) {
        nLockReason = WizNotifyBar::Deleted;
        m_bLocked = true;
    }

    m_editorMode = modeReader;
    if (!m_bLocked) {
        if (forceEdit || m_defaultViewMode == viewmodeAlwaysEditing) {
            m_editorMode = modeEditor;
        }
    }

    bool bGroup = m_dbMgr.db(data.strKbGUID).isGroup();
    m_editStatus = m_editStatus & DOCUMENT_STATUS_PERSONAL;
    if (bGroup)
    {
        m_editStatus = m_editStatus | DOCUMENT_STATUS_GROUP | DOCUMENT_STATUS_FIRSTTIMEVIEW;
    }

    m_title->setLocked(m_bLocked, nLockReason, bGroup);
    //
    //   个人笔记检查笔记是否已经更新了
    //暂时禁止，等待服务器优化速度
    if (WizNotifyBar::LockForGruop == nLockReason)
    {
        startCheckDocumentEditStatus();
    }

    if (m_editorMode == modeEditor)
    {
        showCoachingTips();
    }
}

void WizDocumentView::viewNote(const WIZDOCUMENTDATAEX& wizDoc, bool forceEdit)
{
    WIZDOCUMENTDATAEX dataTemp = wizDoc;
    //
    m_web->trySaveDocument(m_note, false, [=](const QVariant& ret){
        //
        WIZDOCUMENTDATAEX data = dataTemp;

        if (m_dbMgr.db(m_note.strKbGUID).isGroup())
        {
            stopDocumentEditingStatus();
        }

        m_noteLoaded = false;
        m_note = data;
        initStat(data, forceEdit);

        // download document if not exist
        WizDatabase& db = m_dbMgr.db(data.strKbGUID);
        QString strDocumentFileName = db.getDocumentFileName(data.strGUID);
        if (!db.isObjectDataDownloaded(data.strGUID, "document") || \
                !WizPathFileExists(strDocumentFileName))
        {
            m_web->clear();

            m_tab->setCurrentWidget(m_transitionView);
            downloadNoteFromServer(data);
            //
            return;
        }

        // ask user cipher if needed
        //
        data.nProtected = WizZiwReader::isEncryptedFile(strDocumentFileName) ? 1 : 0;
        if (data.nProtected || db.isEncryptAllData()) {
            //
            db.initCert(false);
            //
            if(!db.loadUserCert()) {
                return;
            }
            //
            if (db.getCertPassword().isEmpty()) {
                //
                if (db.isGroup()) {
                    m_passwordView->setPasswordText(tr("Encrypted group password"));
                } else {
                    m_passwordView->setPasswordText(tr("Personal notes certificate password"));
                }
                //

                m_passwordView->setHint(db.getCertPasswordHint());
                m_tab->setCurrentWidget(m_passwordView);
                m_passwordView->setCipherEditorFocus();
                //
                return;
            }
        }
        //
        m_tab->setCurrentWidget(m_docView);

        loadNote(data);
        WIZDOCUMENTDATA docData = data;
        docData.nReadCount ++;
        db.modifyDocumentReadCount(docData);
        docData.tAccessed = WizGetCurrentTime();
        db.modifyDocumentDateAccessed(docData);
    });
}

void WizDocumentView::reviewCurrentNote()
{
    Q_ASSERT(!m_note.strGUID.isEmpty());

    initStat(m_note, m_editorMode == modeEditor);

    // download document if not exist
    WizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
    QString strDocumentFileName = db.getDocumentFileName(m_note.strGUID);
    if (!db.isObjectDataDownloaded(m_note.strGUID, "document") || \
            !WizPathFileExists(strDocumentFileName))
    {
        downloadNoteFromServer(m_note);

        return;
    }

    // ask user cipher if needed
    if (m_note.nProtected) {
        if(!db.loadUserCert()) {
            return;
        }

        if (db.getCertPassword().isEmpty()) {
            m_passwordView->setHint(db.getCertPasswordHint());
            m_tab->setCurrentWidget(m_passwordView);
            m_passwordView->setCipherEditorFocus();

            return;
        }
    }

    if (m_tab->currentWidget() != m_docView) {
        m_tab->setCurrentWidget(m_docView);
    }
}

void WizDocumentView::setEditorMode(WizEditorMode editorMode)
{
    if (m_bLocked)
        return;
    //
    if (m_editorMode == editorMode)
        return;
    //
    bool edit = editorMode == modeEditor;
    bool read = !edit;

    bool isGroupNote =m_dbMgr.db(m_note.strKbGUID).isGroup();
    if (edit && isGroupNote)
    {
        // don not use message tips when check document editable
//        m_title->showMessageTips(Qt::PlainText, tr("Checking whether note is eiditable..."));
        m_title->startEditButtonAnimation();
        if (!checkDocumentEditable(false))
        {
            return;
        }
        // stop check document edit status while enter editing mode
        stopCheckDocumentEditStatus();
    }
    //
    m_editorMode = editorMode;

    if (read)
    {
        m_editStatus = DOCUMENT_STATUS_NOSTATUS;

        // 保存标题，防止因多线程保存引起覆盖
        m_title->onTitleEditFinished();
        m_title->hideMessageTips(false);
    }
    m_title->setEditorMode(editorMode);
    m_web->setEditorMode(editorMode);

    if (isGroupNote)
    {
        if (m_editorMode == modeEditor)
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

void WizDocumentView::setDefaultViewMode(WizDocumentViewMode mode)
{
    m_defaultViewMode = mode;

    switch (m_defaultViewMode)
    {
    case viewmodeAlwaysEditing:
        setEditorMode(modeEditor);
        break;
    case viewmodeAlwaysReading:
        setEditorMode(modeReader);
        break;
    default:
        //Q_ASSERT(0);           //when set to the auto mode,do nonthing
        break;
    }
}

void WizDocumentView::settingsChanged()
{
    setDefaultViewMode(m_userSettings.noteViewMode());
}

void WizDocumentView::sendDocumentSavedSignal(const QString& strGUID, const QString& strKbGUID)
{
    WizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
    if (db.isGroup())
    {
        QString strUserAlias = db.getUserAlias();
        m_editStatusSyncThread->documentSaved(strUserAlias, strKbGUID, strGUID);
    }

    emit documentSaved(strGUID, this);
}

void WizDocumentView::resetTitle(const QString& strTitle)
{
    m_title->resetTitle(strTitle);
}

void WizDocumentView::promptMessage(const QString &strMsg)
{
    m_tab->setCurrentWidget(m_msgWidget);
    m_msgLabel->setText(strMsg);
}

bool WizDocumentView::checkListClickable()
{
    WizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
    if (db.canEditDocument(m_note))
    {
        //m_title->showMessageTips(Qt::PlainText, tr("Checking whether checklist is clickable..."));
        setCursor(Qt::WaitCursor);
        return checkDocumentEditable(true);
    }
    return false;
}

void WizDocumentView::setStatusToEditingByCheckList()
{
    stopCheckDocumentEditStatus();
    sendDocumentEditingStatus();
    m_title->showMessageTips(Qt::PlainText, tr("You have occupied this note by clicking checklist !  " \
                                               "Switch to other notes to free this note."));
}

void WizDocumentView::showCoachingTips()
{
    if (WizTipsWidget* tipWidget = m_title->editorToolBar()->showCoachingTips())
    {
        connect(tipWidget, SIGNAL(finished()), m_title, SLOT(showCoachingTips()));
    }
    else
    {
        m_title->showCoachingTips();
    }
}

void WizDocumentView::wordCount(std::function<void(const QString&)> callback)
{
    m_web->page()->runJavaScript("WizReader.getWordCount()", [=](const QVariant& vRet){
        if (vRet.type() == QVariant::String) {
            QString json = vRet.toString();
            callback(json);
        }
    });
}

void WizDocumentView::setEditorFocus()
{
    m_web->setFocus(Qt::MouseFocusReason);
    m_web->editorFocus();
}


void WizDocumentView::on_attachment_created(const WIZDOCUMENTATTACHMENTDATA& attachment)
{
    if (attachment.strDocumentGUID != note().strGUID)
        return;

    reload();
}

void WizDocumentView::on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA& attachment)
{
    if (attachment.strDocumentGUID != note().strGUID)
        return;

    reload();
}

void WizDocumentView::on_checkEditStatus_finished(const QString& strGUID, bool editable)
{
//    qDebug() << "check eidt status finished , editable  : " << editable;
    if (strGUID == m_note.strGUID)
    {
        if (editable)
        {
            WizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
            WIZDOCUMENTDATA doc;
            db.documentFromGuid(strGUID, doc);
            if (db.canEditDocument(doc) && !WizDatabase::isInDeletedItems(doc.strLocation))
            {
                //            qDebug() << "document editable , hide message tips.";
                if (0 == (m_editStatus & DOCUMENT_STATUS_NEWVERSIONFOUNDED))
                {
                    m_title->hideMessageTips(true);
                }
                m_title->setEditButtonEnabled(true);
                m_bLocked = false;
            }
        }
        stopCheckDocumentAnimations();
    }
}

void WizDocumentView::loadNote(const WIZDOCUMENTDATAEX& doc)
{
    m_web->viewDocument(doc, m_editorMode);
    m_title->setNote(doc, m_editorMode, m_bLocked);
    //
    // save last
    m_note = doc;
    //
    m_noteLoaded = true;
    //
    if (m_editorMode == modeEditor && m_web->hasFocus())
    {
        sendDocumentEditingStatus();
    }
}

void WizDocumentView::downloadNoteFromServer(const WIZDOCUMENTDATA& note)
{
    connect(m_downloaderHost, SIGNAL(downloadProgress(QString,int,int)),
            m_transitionView, SLOT(onDownloadProgressChanged(QString,int,int)), Qt::UniqueConnection);
    m_downloaderHost->downloadDocument(note);
    m_transitionView->showAsMode(note.strGUID, WizDocumentTransitionView::Downloading);
}

void WizDocumentView::sendDocumentEditingStatus()
{
    WizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
    if (db.isGroup())
    {
        QString strUserAlias = db.getUserAlias();
        m_editStatusSyncThread->startEditingDocument(strUserAlias, m_note.strKbGUID, m_note.strGUID);
    }
}

void WizDocumentView::stopDocumentEditingStatus()
{
    WIZDOCUMENTDATA doc = m_note;
    if (m_dbMgr.db(doc.strKbGUID).documentFromGuid(doc.strGUID, doc))
    {
        bool bModified = doc.nVersion == -1;
        m_editStatusSyncThread->stopEditingDocument(doc.strKbGUID, doc.strGUID, bModified);
    }
}

void WizDocumentView::startCheckDocumentEditStatus()
{
    //首先检查笔记是否是待下载状态，如果一篇笔记一直打开，自动同步会把笔记状态设置为待下载，
    //但是其版本号是服务器的版本号，检查新版本无效果

    WizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
    if (!db.isDocumentDownloaded(m_note.strGUID))
    {
        on_checkDocumentChanged_finished(m_note.strGUID, true);
        m_title->stopEditButtonAnimation();
        return;
    }

    m_editStatus = DOCUMENT_STATUS_NOSTATUS;
    emit checkDocumentEditStatusRequest(m_note.strKbGUID, m_note.strGUID);
}

void WizDocumentView::stopCheckDocumentEditStatus()
{
    emit stopCheckDocumentEditStatusRequest(m_note.strKbGUID, m_note.strGUID);
}

bool WizDocumentView::checkDocumentEditable(bool checklist)
{
    QEventLoop loop;
    connect(m_editStatusChecker, SIGNAL(checkEditStatusFinished(QString,bool)), &loop, SLOT(quit()));
    connect(m_editStatusChecker, SIGNAL(checkTimeOut(QString)), &loop, SLOT(quit()));
    startCheckDocumentEditStatus();
    m_editStatus = m_editStatus | DOCUMENT_STATUS_ON_EDITREQUEST;
    if (checklist)
    {
        m_editStatus |= DOCUMENT_STATUS_ON_CHECKLIST;
    }
    loop.exec();
    //

    bool editByOther = m_editStatus & DOCUMENT_STATUS_EDITBYOTHERS;
    bool newVersion = m_editStatus & DOCUMENT_STATUS_NEWVERSIONFOUNDED;
    bool isOffLine = m_editStatus & DOCUMENT_STATUS_OFFLINE;
    return !(editByOther || newVersion) || isOffLine;
}

void WizDocumentView::stopCheckDocumentAnimations()
{
    if (cursor().shape() != Qt::ArrowCursor)
    {
        setCursor(Qt::ArrowCursor);
    }
    m_title->stopEditButtonAnimation();
}

void WizDocumentView::on_document_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew)
{
    Q_UNUSED(documentOld);

    if (note().strGUID != documentNew.strGUID)
        return;

    reload();
}

void WizDocumentView::onCloseNoteRequested(WizDocumentView *view)
{
    Q_UNUSED(view)

    m_tab->setCurrentWidget(m_blankView);
}

void WizDocumentView::onCipherCheckRequest()
{
    const WIZDOCUMENTDATA& noteData = note();
    WizDatabase& db = m_dbMgr.db(noteData.strKbGUID);
    //
    QString password = m_passwordView->userCipher();
    if (!db.verifyCertPassword(password))
    {
        m_passwordView->cipherError();
        WizMessageBox::information(qApp->activeWindow(), tr("WizNote"), tr("Invalid password."));

        return;
    }
    //
    m_passwordView->cipherCorrect();

    m_tab->setCurrentWidget(m_docView);
    loadNote(noteData);
}

void WizDocumentView::on_download_finished(const WIZOBJECTDATA &data, bool bSucceed)
{
    if (m_note.strKbGUID != data.strKbGUID
            || m_note.strGUID != data.strObjectGUID)
        return;

    if (!bSucceed)
    {
        m_transitionView->showAsMode(data.strObjectGUID, WizDocumentTransitionView::ErrorOccured);
        return;
    }

    if (m_editorMode == modeEditor)
        return;


    bool onEditRequest = m_editStatus & DOCUMENT_STATUS_ON_EDITREQUEST;
    if (onEditRequest)
    {
        if (m_editStatus & DOCUMENT_STATUS_ON_CHECKLIST)
        {
            onEditRequest = false;
        }
    }
    //
    bool forceEdit = onEditRequest ? true : false;

    viewNote(m_note, forceEdit);
}

void WizDocumentView::on_document_data_modified(const WIZDOCUMENTDATA& data)
{
    //verify m_noteLoaded before reload
    if (note().strGUID != data.strGUID || !m_noteLoaded)
        return;

    reloadNote();
    //
    WizMainWindow::instance()->quickSyncKb(data.strKbGUID);
}



void WizDocumentView::on_document_data_changed(const QString& strGUID,
                                              WizDocumentView* viewer)
{
    if (viewer != this && strGUID == note().strGUID && m_editorMode == modeReader)
    {
        reloadNote();
    }
}


void WizDocumentView::on_documentEditingByOthers(QString strGUID, QStringList editors)
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
                WizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
                QString strUserAlias = db.getUserAlias();
                if (editors.count() == 1 && editors.first() == strUserAlias)
                {
                    qDebug() << "[EditStatus]:editing by myself.";
                    m_title->hideMessageTips(true);
                    m_title->setEditButtonEnabled(true);
                    m_editStatus = m_editStatus & ~DOCUMENT_STATUS_EDITBYOTHERS;
                }
                else
                {
                    m_title->showMessageTips(Qt::PlainText, QString(tr("%1 is currently editing this note. Note has been locked.")).arg(strEditor));
                    m_editStatus = m_editStatus | DOCUMENT_STATUS_EDITBYOTHERS;
                    if (m_note.nVersion == -1)
                    {
                        m_title->showMessageTips(Qt::PlainText, QString(tr("%1 is currently editing this note.")).arg(strEditor));
                        m_title->setEditButtonEnabled(true);
                    }
                    else
                    {
                        m_title->setEditButtonEnabled(false);
                    }
                }
            }
        }
        else
        {
            m_title->setEditButtonEnabled(!m_bLocked);
            m_editStatus = m_editStatus & ~DOCUMENT_STATUS_EDITBYOTHERS;
        }
    }
}

void WizDocumentView::on_checkEditStatus_timeout(const QString& strGUID)
{
    if (strGUID == m_note.strGUID)
    {
        if (m_editStatus & DOCUMENT_STATUS_ON_EDITREQUEST)
        {
            m_title->showMessageTips(Qt::RichText, tr("The current network in poor condition, you are <b> offline editing mode </b>."));
        }
        m_title->setEditButtonEnabled(true);
        m_editStatus = m_editStatus | DOCUMENT_STATUS_OFFLINE;
        m_bLocked = false;
        stopCheckDocumentAnimations();
    }
}

void WizDocumentView::on_checkDocumentChanged_finished(const QString& strGUID, bool changed)
{
    if (strGUID == m_note.strGUID)
    {       
        if (changed)
        {
            m_title->showMessageTips(Qt::RichText, QString(tr("New version on server avalible. <a href='%1'>Click to download new version.<a>")).arg(NOTIFYBAR_LABELLINK_DOWNLOAD));
            m_editStatus |= DOCUMENT_STATUS_NEWVERSIONFOUNDED;
        }
        else
        {
            m_bLocked = false;
            int nLockReason = -1;

            const WIZDOCUMENTDATA& doc = note();
            if (!m_dbMgr.db(doc.strKbGUID).canEditDocument(doc)) {
                nLockReason = WizNotifyBar::PermissionLack;
                m_bLocked = true;
            } else if (WizDatabase::isInDeletedItems(doc.strLocation)) {
                nLockReason = WizNotifyBar::Deleted;
                m_bLocked = true;
            }

            if (m_bLocked)
            {
                bool bGroup = m_dbMgr.db(doc.strKbGUID).isGroup();
                m_title->setLocked(m_bLocked, nLockReason, bGroup);
            }
            //
            setEditorMode(modeReader);

            m_editStatus &= ~DOCUMENT_STATUS_NEWVERSIONFOUNDED;
        }
        m_editStatus = m_editStatus & ~DOCUMENT_STATUS_FIRSTTIMEVIEW;
    }
}

void WizDocumentView::on_syncDatabase_request(const QString& strKbGUID)
{
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(m_app.mainWindow());
    mainWindow->quickSyncKb(strKbGUID);
}

void WizDocumentView::on_webView_focus_changed()
{
    if (m_web->hasFocus())
    {
        sendDocumentEditingStatus();
    }
}

void WizDocumentView::on_notifyBar_link_clicked(const QString& link)
{
    if (link == NOTIFYBAR_LABELLINK_DOWNLOAD)
    {
        WizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
        QString strDocumentFileName = db.getDocumentFileName(m_note.strGUID);
        WizDeleteFile(strDocumentFileName);

        downloadNoteFromServer(m_note);
    }
}


void WizDocumentView::on_loadComment_request(const QString& url)
{
    m_comments->load(url);
}

void WizDocumentView::on_commentWidget_statusChanged()
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



