#include "wizDocumentView.h"

#include <QWebElement>
#include <QWebFrame>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QStackedWidget>

#include <coreplugin/icore.h>

#include "wizmainwindow.h"
#include "wizDocumentTransitionView.h"
#include "share/wizObjectDataDownloader.h"
#include "share/wizDatabaseManager.h"
#include "widgets/wizScrollBar.h"
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
#include "sync/wizkmxmlrpc.h"

#include "titlebar.h"

using namespace Core;
using namespace Core::Internal;


#define DOCUMENT_PERSONAL           0x0000
#define DOCUMENT_GROUP                 0x0001
#define DOCUMENT_OFFLINE               0x0002
#define DOCUMENT_FIRSTTIMEVIEW     0x0004
#define DOCUMENT_EDITBYOTHERS   0x0010

CWizDocumentView::CWizDocumentView(CWizExplorerApp& app, QWidget* parent)
    : INoteView(parent)
    , m_app(app)
    , m_userSettings(app.userSettings())
    , m_dbMgr(app.databaseManager())
    , m_web(new CWizDocumentWebView(app, this))
    , m_comments(new QWebView(this))
    , m_title(new TitleBar(app, this))
    , m_passwordView(new CWizUserCipherForm(app, this))
    , m_viewMode(app.userSettings().noteViewMode())
    , m_bLocked(false)
    , m_bEditingMode(false)
    , m_noteLoaded(false)
    , m_editStatusSyncThread(new CWizDocumentEditStatusSyncThread(this))
    //, m_editStatusCheckThread(new CWizDocumentStatusCheckThread(this))
    , m_status(0)
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

    m_splitter = new CWizSplitter(this);
    m_splitter->addWidget(m_web);
    m_splitter->addWidget(m_comments);
    m_comments->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    m_comments->page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    m_comments->setAcceptDrops(false);
    m_comments->hide();

    layoutDoc->addWidget(m_title);
    layoutDoc->addWidget(m_splitter);

    layoutDoc->setStretchFactor(m_title, 0);
    layoutDoc->setStretchFactor(m_splitter, 1);

    QVBoxLayout* layoutMain = new QVBoxLayout(this);
    layoutMain->setContentsMargins(0, 0, 0, 0);
    setLayout(layoutMain);
    layoutMain->addWidget(m_tab);

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

//    connect(m_editStatusCheckThread, SIGNAL(checkFinished(QString,QStringList)),
//            SLOT(on_checkEditStatus_finished(QString,QStringList)));
//    connect(m_editStatusCheckThread, SIGNAL(checkDocumentChangedFinished(QString,bool)),
//            SLOT(on_checkDocumentChanged_finished(QString,bool)));
//    connect(m_editStatusCheckThread, SIGNAL(checkTimeOut(QString)),
//            SLOT(on_checkEditStatus_timeout(QString)));

    // open comments link by document webview
    connect(m_comments->page(), SIGNAL(linkClicked(const QUrl&)), m_web,
            SLOT(onEditorLinkClicked(const QUrl&)));
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
void CWizDocumentView::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
}

void CWizDocumentView::showClient(bool visible)
{
    m_tab->setVisible(visible);
}

void CWizDocumentView::onViewNoteRequested(INoteView* view, const WIZDOCUMENTDATA& doc)
{
    if (view != this)
        return;

    if (doc.tCreated.secsTo(QDateTime::currentDateTime()) == 0) {
        viewNote(doc, true);
    } else {
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
    m_status = m_status & DOCUMENT_PERSONAL;
    if (bGroup)
    {
        m_status = m_status | DOCUMENT_GROUP | DOCUMENT_FIRSTTIMEVIEW;
    }
    m_title->setLocked(m_bLocked, nLockReason, bGroup);
    if (NotifyBar::LockForGruop == nLockReason)
    {
        startCheckDocumentEditStatus();
    }
}

void CWizDocumentView::viewNote(const WIZDOCUMENTDATA& data, bool forceEdit)
{
    MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());

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
            !PathFileExists(strDocumentFileName)) {

        window->downloaderHost()->downloadData(data);
        window->showClient(false);
        window->transitionView()->showAsMode(data.strGUID, CWizDocumentTransitionView::Downloading);

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
            !PathFileExists(strDocumentFileName)) {
        MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
        window->downloaderHost()->downloadData(m_note);
        window->showClient(false);
        window->transitionView()->showAsMode(m_note.strGUID, CWizDocumentTransitionView::Downloading);

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

void CWizDocumentView::setEditorFocus()
{
    m_web->setFocus(Qt::MouseFocusReason);
    m_web->editorFocus();
}

QWebFrame* CWizDocumentView::noteFrame()
{
    return m_web->noteFrame();
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

void CWizDocumentView::downloadDocumentFromServer()
{
    CWizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
    QString strDocumentFileName = db.GetDocumentFileName(m_note.strGUID);
    QFile::remove(strDocumentFileName);

    MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
    window->downloaderHost()->downloadDocument(m_note);
    window->showClient(false);
    window->transitionView()->showAsMode(m_note.strGUID, CWizDocumentTransitionView::Downloading);
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
    startCheckDocumentEditStatus();
    loop.exec();
    //

    return !(m_status & DOCUMENT_EDITBYOTHERS);
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

    if (!bSucceed)
        return;

    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    mainWindow->transitionView()->setVisible(false);

    viewNote(m_note,m_bEditingMode);
}

void CWizDocumentView::on_document_data_modified(const WIZDOCUMENTDATA& data)
{
    //verify m_noteLoaded before reload
    if (note().strGUID != data.strGUID || !m_noteLoaded)
        return;

    reloadNote();
}

void CWizDocumentView::on_document_data_saved(const QString& strGUID,
                                              CWizDocumentView* viewer)
{
    if (viewer != this && strGUID == note().strGUID)
    {
        reloadNote();
    }
}


void Core::CWizDocumentView::on_documentEditingByOthers(QString strGUID, QStringList editors)
{
    //
    //QString strCurrentUser = m_dbMgr.db(m_note.strKbGUID).GetUserAlias();
    //editors.removeAll(strCurrentUser);

    if (strGUID == m_note.strGUID && !editors.isEmpty())
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
                m_status = m_status & ~DOCUMENT_EDITBYOTHERS;
            }
            else
            {
                m_title->showMessageTips(Qt::PlainText, QString(tr("%1 is currently editing this note. Note has been locked.")).arg(strEditor));
                m_status = m_status | DOCUMENT_EDITBYOTHERS;
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
        if (strGUID == m_note.strGUID)
        {
            m_title->setEditButtonState(!m_bLocked, false);
            m_status = m_status & ~DOCUMENT_EDITBYOTHERS;
        }
    }
}

void CWizDocumentView::on_checkEditStatus_timeout(const QString& strGUID)
{
    if (strGUID == m_note.strGUID)
    {
        if (!(m_status & DOCUMENT_OFFLINE))
        {
            m_title->setEditButtonState(true, false);
            m_title->showMessageTips(Qt::RichText, tr("The current network in poor condition, you are <b> offline editing mode </b>."));
            m_status = m_status | DOCUMENT_OFFLINE;
        }
    }
    stopCheckDocumentAnimations();
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
                m_title->showMessageTips(Qt::RichText, QString(tr("New version on server avalible. <a href='%1'>Click to down load new version.<a>")).arg(NOTIFYBAR_LABELLINK_DOWNLOAD));
//            }
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
        }
        m_status = m_status & ~DOCUMENT_FIRSTTIMEVIEW;
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
        downloadDocumentFromServer();
    }
}


WizFloatDocumentViewer::WizFloatDocumentViewer(CWizExplorerApp& app, QWidget* parent) : QWidget(parent)
{
        setAttribute(Qt::WA_DeleteOnClose);
        setContentsMargins(0, 0, 0, 0);
        setPalette(QPalette(Qt::white));
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        m_docView = new CWizDocumentView(app, this);
        layout->addWidget(m_docView);
        setLayout(layout);
}

WizFloatDocumentViewer::~WizFloatDocumentViewer()
{
    m_docView->waitForDone();
}

