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
    , m_viewMode(app.userSettings().noteViewMode())
    , m_bLocked(false)
    , m_bEditingMode(false)
{
    m_title->setEditor(m_web);

    QVBoxLayout* layoutDoc = new QVBoxLayout();
    layoutDoc->setContentsMargins(0, 0, 0, 0);
    layoutDoc->setSpacing(0);

    m_docView = new QWidget(this);
    m_docView->setLayout(layoutDoc);

    m_tab = new QStackedWidget(this);
    //
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    m_passwordView = mainWindow->cipherForm();
    m_passwordView->setGeometry(this->geometry());
    connect(m_passwordView, SIGNAL(cipherCheckRequest()), SLOT(onCipherCheckRequest()));
    //
    m_tab->addWidget(m_docView);
    m_tab->addWidget(m_passwordView);
    m_tab->setCurrentWidget(m_docView);

    m_splitter = new CWizSplitter(this);
    m_splitter->addWidget(m_web);
    m_splitter->addWidget(m_comments);
    m_comments->page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    m_comments->hide();

    layoutDoc->addWidget(m_title);
    layoutDoc->addWidget(m_splitter);

    layoutDoc->setStretchFactor(m_title, 0);
    layoutDoc->setStretchFactor(m_splitter, 1);

    QVBoxLayout* layoutMain = new QVBoxLayout(this);
    layoutMain->setContentsMargins(0, 0, 0, 0);
    setLayout(layoutMain);
    layoutMain->addWidget(m_tab);

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
}

CWizDocumentView::~CWizDocumentView()
{
    m_web->saveDocument(m_note, false);
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

    if (CWizDatabase::IsInDeletedItems(data.strLocation)) {
        nLockReason = 1;
        m_bLocked = true;
    }

    if (!m_dbMgr.db(data.strKbGUID).CanEditDocument(data)) {
        nLockReason = 2;
        m_bLocked = true;
    }

    if (m_bLocked) {
        m_bEditingMode = false;
    } else {
        m_bEditingMode = bEditing ? true : defaultEditingMode();
    }

    bool bGroup = m_dbMgr.db(data.strKbGUID).IsGroup();
    m_title->setLocked(m_bLocked, nLockReason, bGroup);
}

void CWizDocumentView::viewNote(const WIZDOCUMENTDATA& data, bool forceEdit)
{
    MainWindow* window = qobject_cast<MainWindow *>(m_app.mainWindow());

    m_web->saveDocument(m_note, false);

    m_note = data;
    initStat(data, forceEdit);
    m_tab->setCurrentWidget(m_docView);
    m_tab->setVisible(true);

    // download document if not exist
    CWizDatabase& db = m_dbMgr.db(data.strKbGUID);
    QString strDocumentFileName = db.GetDocumentFileName(data.strGUID);
    if (!db.IsObjectDataDownloaded(data.strGUID, "document") || \
            !PathFileExists(strDocumentFileName)) {

        window->downloaderHost()->download(data);
        window->showClient(false);
        window->transitionView()->showAsMode(CWizDocumentTransitionView::Downloading);

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
}

void CWizDocumentView::setEditNote(bool bEdit)
{
    if (m_bLocked) {
        return;
    }

    m_bEditingMode = bEdit;

    m_title->setEditingDocument(bEdit);
    m_web->setEditingDocument(bEdit);
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

void CWizDocumentView::resetTitle(const QString& strTitle)
{
    m_title->resetTitle(strTitle);
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
    if (note().strGUID != data.strGUID)
        return;

    reloadNote();
}
