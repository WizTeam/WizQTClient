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

CWizDocumentView::CWizDocumentView(CWizExplorerApp& app, QWidget* parent)
    : INoteView(parent)
    , m_app(app)
    , m_userSettings(app.userSettings())
    , m_dbMgr(app.databaseManager())
    , m_web(new CWizDocumentWebView(app, this))
    , m_comments(new QWebView(this))
    , m_title(new TitleBar(this))
    , m_passwordView(new CWizUserCipherForm(app, this))
    , m_viewMode(app.userSettings().noteViewMode())
    , m_bLocked(false)
    , m_bEditingMode(false)
    , m_noteLoaded(false)
    , m_editStatusSyncThread(new CWizDocumentEditStatusSyncThread(this))
    , m_editStatusCheckThread(new CWizDocumentStatusCheckThread(this))
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

    connect(Core::ICore::instance(), SIGNAL(viewNoteRequested(Core::INoteView*,const WIZDOCUMENTDATA&)),
            SLOT(onViewNoteRequested(Core::INoteView*,const WIZDOCUMENTDATA&)));

    connect(Core::ICore::instance(), SIGNAL(viewNoteLoaded(Core::INoteView*,WIZDOCUMENTDATA,bool)),
            SLOT(onViewNoteLoaded(Core::INoteView*,const WIZDOCUMENTDATA&,bool)));

    connect(Core::ICore::instance(), SIGNAL(closeNoteRequested(Core::INoteView*)),
            SLOT(onCloseNoteRequested(Core::INoteView*)));

    connect(m_web, SIGNAL(focusIn()), SLOT(on_webView_focus_changed()));

    connect(m_editStatusCheckThread, SIGNAL(checkFinished(QString,QStringList)),
            SLOT(on_checkEditStatus_finished(QString,QStringList)));
    connect(m_editStatusCheckThread, SIGNAL(checkDocumentChangedFinished(QString,bool,int)),
            SLOT(on_checkDocumentChanged_finished(QString,bool,int)));

    // open comments link by document webview
    connect(m_comments->page(), SIGNAL(linkClicked(const QUrl&)), m_web,
            SLOT(onEditorLinkClicked(const QUrl&)));
    //
    m_editStatusSyncThread->start(QThread::IdlePriority);
    m_editStatusCheckThread->start(QThread::IdlePriority);

}

CWizDocumentView::~CWizDocumentView()
{
}

void CWizDocumentView::waitForDone()
{
    m_web->saveDocument(m_note, false);
    //
    m_web->waitForDone();
    //
    m_editStatusSyncThread->waitForDone();
    m_editStatusCheckThread->waitForDone();
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
        nLockReason = NotifyBar::LockForGruop;
        m_bLocked = true;
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
    m_title->setLocked(m_bLocked, nLockReason, bGroup);
    if (NotifyBar::LockForGruop == nLockReason)
    {
        m_editStatusCheckThread->checkEditStatus(data.strKbGUID, data.strGUID);
    }
}

void CWizDocumentView::viewNote(const WIZDOCUMENTDATA& data, bool forceEdit)
{
    MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());

    m_web->saveDocument(m_note, false);

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

        window->downloaderHost()->download(data);
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
        window->downloaderHost()->download(m_note);
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

    m_bEditingMode = bEdit;

    m_title->setEditingDocument(bEdit);
    m_web->setEditingDocument(bEdit);

    if (m_bEditingMode)
    {
        CWizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
        if (db.IsGroup())
        {
            QString strUserAlias = db.GetUserAlias();
            m_editStatusSyncThread->setCurrentEditingDocument(strUserAlias, m_note.strKbGUID, m_note.strGUID);
        }
    }
    else
    {
        m_editStatusSyncThread->stopEditingDocument();
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

void CWizDocumentView::sendDocumentSavedSignal(const QString& strGUID)
{
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

void CWizDocumentView::loadNote(const WIZDOCUMENTDATA& doc)
{
    m_web->viewDocument(doc, m_bEditingMode);
    m_title->setNote(doc, m_bEditingMode, m_bLocked);

    // save last
    m_note = doc;
    //
    m_noteLoaded = true;

    m_editStatusSyncThread->stopEditingDocument();
    //
    if (m_bEditingMode && m_web->hasFocus())
    {
        CWizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
        if (db.IsGroup())
        {
            QString strUserAlias = db.GetUserAlias();
            m_editStatusSyncThread->setCurrentEditingDocument(strUserAlias, m_note.strKbGUID, m_note.strGUID);
        }
    }
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


void Core::CWizDocumentView::on_checkEditStatus_finished(QString strGUID, QStringList editors)
{
    //
    QString strCurrentUser = m_dbMgr.db(m_note.strKbGUID).GetUserAlias();
    editors.removeAll(strCurrentUser);

    if (strGUID == m_note.strGUID && !editors.isEmpty())
    {
        QString strEditor = editors.join(" , ");
        m_title->setDocumentEditingStatus(strEditor);
    }
    else
    {
        m_title->setDocumentEditingStatus("");
        if (strGUID == m_note.strGUID)
        {
            m_title->setEditButtonState(true, false);
        }
    }
}

void CWizDocumentView::on_checkDocumentChanged_finished(const QString& strGUID, bool changed, int versionOnServer)
{
    if (strGUID == m_note.strGUID)
    {
        if (changed)
        {
            CWizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
            m_note.nVersion = versionOnServer;
            db.ModifyDocumentInfoEx(m_note);

            // downlaod document data
            QString strDocumentFileName = db.GetDocumentFileName(m_note.strGUID);
            QFile::remove(strDocumentFileName);

            MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());
            window->downloaderHost()->download(m_note);
            window->showClient(false);
            window->transitionView()->showAsMode(m_note.strGUID, CWizDocumentTransitionView::Downloading);

            return;
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
        CWizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
        if (db.IsGroup())
        {
            QString strUserAlias = db.GetUserAlias();
            m_editStatusSyncThread->setCurrentEditingDocument(strUserAlias, m_note.strKbGUID, m_note.strGUID);
        }
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

