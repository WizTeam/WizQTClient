#include "wizdocumentview.h"

#include <QWebElement>
#include <QWebFrame>
#include <QBoxLayout>
#include <QLineEdit>
#include <QApplication>

#include "share/wizimagepushbutton.h"
#include "wizdocumentwebview.h"
#include "wiztaglistwidget.h"
#include "wizattachmentlistwidget.h"
#include "wiznoteinfoform.h"
#include "wiznotestyle.h"
#include "wizEditorToolBar.h"

class CWizTitleBar
    : public QWidget
{
public:
    CWizTitleBar(CWizExplorerApp& app, QWidget* parent)
        : QWidget(parent)
        , m_app(app)
        , m_editDocumentButton(NULL)
        , m_tagsButton(NULL)
        , m_attachmentButton(NULL)
        , m_editing(false)
    {
        QHBoxLayout* layout = new QHBoxLayout(this);
        setLayout(layout);
        layout->setMargin(0);

        setContentsMargins(4, 4, 4, 4);
        //setAutoFillBackground(true);

        //QPalette pal = palette();
        //QPixmap pixmapBg;
        //pixmapBg.load(::WizGetResourcesPath() + "skins/main_bg.png");
        //QBrush brushBg(pixmapBg);
        //pal.setBrush(QPalette::Window, brushBg);
        //setPalette(pal);

        m_titleEdit = new QLineEdit(this);
//#ifdef Q_OS_MAC

        m_titleEdit->setStyleSheet(
            "QLineEdit {\
                margin:4 0 0 0;\
                padding:4 4 4 4;\
                border-style:none;\
                background-image: url(" + ::WizGetResourcesPath() + "skins/leftview_bg.png); \
            } \
            QLineEdit:hover { \
                border-color:#bbbbbb; \
                border-width:1; \
                border-style:solid; \
            }");

        m_titleEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
//#else
//        m_titleEdit->setStyleSheet("QLineEdit{padding:4 4 4 4;border-color:#ffffff;border-width:1;border-style:solid;}QLineEdit:hover{border-color:#bbbbbb;border-width:1;border-style:solid;}");
//#endif

        m_editIcon = ::WizLoadSkinIcon(m_app.userSettings().skin(), "lock");
        m_commitIcon = ::WizLoadSkinIcon(m_app.userSettings().skin(), "unlock");
        m_tagsIcon = ::WizLoadSkinIcon(m_app.userSettings().skin(), "tag");
        m_attachmentIcon = ::WizLoadSkinIcon(m_app.userSettings().skin(), "attachment");
        m_infoIcon = ::WizLoadSkinIcon(m_app.userSettings().skin(), "noteinfo");

        m_editDocumentButton = new CWizImagePushButton(m_editIcon, "", this);
        updateEditDocumentButtonIcon(false);
        m_editDocumentButton->setStyle(::WizGetStyle(m_app.userSettings().skin()));
        m_editDocumentButton->setRedFlag(true);

        m_tagsButton = new CWizImagePushButton(m_tagsIcon, "", this);
        m_tagsButton->setStyle(::WizGetStyle(m_app.userSettings().skin()));

        m_attachmentButton = new CWizImagePushButton(m_attachmentIcon, "", this);
        m_attachmentButton->setStyle(::WizGetStyle(m_app.userSettings().skin()));

        m_infoButton = new CWizImagePushButton(m_infoIcon, "", this);
        m_infoButton->setStyle(::WizGetStyle(m_app.userSettings().skin()));

        layout->addWidget(m_titleEdit);
        layout->addWidget(m_editDocumentButton);
        layout->addWidget(m_tagsButton);
        layout->addWidget(m_attachmentButton);
        layout->addWidget(m_infoButton);
    }

private:
    CWizExplorerApp& m_app;
    QPointer<QLineEdit> m_titleEdit;
    CWizImagePushButton* m_editDocumentButton;
    CWizImagePushButton* m_tagsButton;
    CWizImagePushButton* m_attachmentButton;
    CWizImagePushButton* m_infoButton;

    QIcon m_editIcon;
    QIcon m_commitIcon;
    QIcon m_tagsIcon;
    QIcon m_attachmentIcon;
    QIcon m_infoIcon;

    bool m_editing;
private:
    void updateEditDocumentButtonIcon(bool editing)
    {
        m_editDocumentButton->setIcon(editing ? m_commitIcon : m_editIcon);
        m_editing = editing;

        updateEditDocumentButtonTooltip();
    }
    void updateEditDocumentButtonTooltip()
    {
        QString shortcut = ::WizGetShortcut("EditNote", "Alt+1");
        QString strSaveAndRead = QObject::tr("Save & Switch to Reading View");
        QString strRead = QObject::tr("Switch to Reading View");
        QString strEditNote = QObject::tr("Switch to Editing View");
        QString strSwitchRead = m_editDocumentButton->text().isEmpty() ? strRead : strSaveAndRead;
        QString strToolTip = m_editing ? strSwitchRead : strEditNote;
        strToolTip += " (" + shortcut + ")";
        m_editDocumentButton->setToolTip(strToolTip);
        m_editDocumentButton->setShortcut(QKeySequence::fromString(shortcut));
    }

public:
    QLineEdit* titleEdit() const { return m_titleEdit; }
    QPushButton* editDocumentButton() const { return m_editDocumentButton; }
    QPushButton* tagsButton() const { return m_tagsButton; }
    QPushButton* attachmentButton() const { return m_attachmentButton; }
    QPushButton* infoButton() const { return m_infoButton; }

    void setEditingDocument(bool editing) { updateEditDocumentButtonIcon(editing); }

    void setTitle(const QString& str)
    {
        m_titleEdit->setText(str);
    }

    void updateInformation(CWizDatabase& db, const WIZDOCUMENTDATA& doc)
    {
        // retrieve document info and reset
        WIZDOCUMENTDATA data;
        if (!db.DocumentFromGUID(doc.strGUID, data)) {
            return;
        }

        //title
        if (m_titleEdit->text() != data.strTitle) {
            m_titleEdit->setText(data.strTitle);
        }

        // update tags count only if it's enabled
        if (m_tagsButton->isEnabled()) {
            CWizStdStringArray arrayTagGUID;
            db.GetDocumentTags(data.strGUID, arrayTagGUID);
            QString strTagText = arrayTagGUID.empty() ? QString() : QString::number(arrayTagGUID.size());
            m_tagsButton->setText(strTagText);
        }

        QString tagsShortcut = ::WizGetShortcut("EditNoteTags", "Alt+2");
        QString strTagsToolTip = QObject::tr("Tags (%1)").arg(tagsShortcut);
        m_tagsButton->setToolTip(strTagsToolTip);
        m_tagsButton->setShortcut(QKeySequence::fromString(tagsShortcut));

        //attachments
        int nAttachmentCount = db.GetDocumentAttachmentCount(data.strGUID);
        CString strAttachmentText = nAttachmentCount ? WizIntToStr(nAttachmentCount) : CString();
        m_attachmentButton->setText(strAttachmentText);
        QString attachmentShortcut = ::WizGetShortcut("EditNoteAttachments", "Alt+3");
        m_attachmentButton->setToolTip(QObject::tr("Attachments (%1)").arg(attachmentShortcut));
        m_attachmentButton->setShortcut(QKeySequence::fromString(attachmentShortcut));
    }

    void setModified(bool modified)
    {
        m_editDocumentButton->setText(modified ? "*" : "");
        updateEditDocumentButtonTooltip();
    }
};



CWizDocumentView::CWizDocumentView(CWizExplorerApp& app, QWidget* parent)
    : QWidget(parent)
    , m_app(app)
    , m_userSettings(app.userSettings())
    , m_dbMgr(app.databaseManager())
    , m_title(new CWizTitleBar(app, this))
    , m_web(new CWizDocumentWebView(app, this))
    , m_editorToolBar(new CWizEditorToolBar(app, this))
    , m_client(NULL)
    , m_tags(NULL)
    , m_attachments(NULL)
    , m_editingDocument(true)
    , m_viewMode(app.userSettings().noteViewMode())
{
    m_client = createClient();

    QVBoxLayout* layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->addWidget(m_client);
    layout->setContentsMargins(0, 0, 0, 0);

    m_title->setEditingDocument(m_editingDocument);

    m_timerDelay.setSingleShot(true);
    connect(&m_timerDelay, SIGNAL(timeout()), SLOT(on_titleEdit_textEdit_writeDelay()));

    connect(m_title->titleEdit(), SIGNAL(textChanged(const QString&)), \
            SLOT(on_titleEdit_textChanged(const QString&)));

    connect(m_title->editDocumentButton(), SIGNAL(clicked()), \
            SLOT(on_editDocumentButton_clicked()));

    connect(m_title->tagsButton(), SIGNAL(clicked()), \
            SLOT(on_tagsButton_clicked()));

    connect(m_title->attachmentButton(), SIGNAL(clicked()), \
            SLOT(on_attachmentButton_clicked()));

    connect(m_title->infoButton(), SIGNAL(clicked()), \
            SLOT(on_infoButton_clicked()));

    qRegisterMetaType<WIZDOCUMENTDATA>("WIZDOCUMENTDATA");
    qRegisterMetaType<WIZDOCUMENTATTACHMENTDATA>("WIZDOCUMENTATTACHMENTDATA");

    connect(&m_dbMgr, SIGNAL(documentModified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)), \
            SLOT(on_document_modified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(attachmentCreated(const WIZDOCUMENTATTACHMENTDATA&)), \
            SLOT(on_attachment_created(const WIZDOCUMENTATTACHMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(attachmentDeleted(const WIZDOCUMENTATTACHMENTDATA&)), \
            SLOT(on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA&)));
}


QWidget* CWizDocumentView::createClient()
{
    QWidget* client = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(client);
    client->setLayout(layout);

    //client->setAutoFillBackground(true);

    //QPalette pal = palette();
    //QPixmap pixmapBg;
    //pixmapBg.load(::WizGetResourcesPath() + "skins/main_bg.png");
    //QBrush brushBg(pixmapBg);
    //pal.setBrush(QPalette::Base, brushBg);
    //client->setPalette(pal);
    //QPalette pal = palette();
    //pal.setColor(QPalette::Window, QColor(0xff, 0xff, 0xff));
    //client->setPalette(pal);

    QWidget* line = new QWidget(this);
    line->setMaximumHeight(1);
    line->setMinimumHeight(1);
    line->setStyleSheet("border-bottom-width:1;border-bottom-style:solid;border-bottom-color:#bbbbbb");

    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_title);
    layout->addWidget(line);
    layout->addWidget(m_editorToolBar);
    //layout->addWidget(line);
    layout->addWidget(m_web);

    layout->setStretchFactor(m_title, 0);
    layout->setStretchFactor(m_web, 1);

    return client;
}

CWizEditorToolBar* CWizDocumentView::editorToolBar() const
{
    return m_editorToolBar;
}

void CWizDocumentView::showClient(bool visible)
{
    if (visible) {
        m_client->show();
    } else {
        m_client->hide();
    }
}

void CWizDocumentView::setReadOnly(bool b, bool isGroup)
{
    m_title->titleEdit()->setReadOnly(b);
    m_title->editDocumentButton()->setEnabled(!b);

    // tag is not avaliable for group
    if (isGroup) {
        m_title->tagsButton()->setText(QString());
        m_title->tagsButton()->setEnabled(false);
    } else {
        m_title->tagsButton()->setEnabled(true);
    }
}

bool CWizDocumentView::viewDocument(const WIZDOCUMENTDATA& data, bool forceEdit)
{
    bool edit = false;

    if (forceEdit) {
        edit = true;
    } else {
        switch (m_viewMode) {
            case viewmodeAlwaysEditing:
                edit = true;
                break;
            case viewmodeAlwaysReading:
                edit = false;
                break;
            default:
                edit = m_editingDocument;
                break;
        }
    }

    // check user permission
    int perm = m_dbMgr.db(data.strKbGUID).permission();
    QString strUserId = m_dbMgr.db().getUserId();
    // only reading is permit
    if (perm > WIZ_USERGROUP_AUTHOR ||
            (perm == WIZ_USERGROUP_AUTHOR && data.strOwner != strUserId)) {
        edit = false;
    }

    m_web->viewDocument(data, edit);
    m_editingDocument = edit;
    m_title->setEditingDocument(m_editingDocument);
    m_title->updateInformation(m_dbMgr.db(data.strKbGUID), data);

    return true;
}

const WIZDOCUMENTDATA& CWizDocumentView::document()
{
    static WIZDOCUMENTDATA empty;
    if (!isVisible())
        return empty;

    return m_web->document();
}

void CWizDocumentView::editDocument(bool editing)
{
    m_editingDocument = editing;
    m_title->setEditingDocument(m_editingDocument);
    m_web->setEditingDocument(m_editingDocument);
}

void CWizDocumentView::setViewMode(WizDocumentViewMode mode)
{
    m_viewMode = mode;

    switch (m_viewMode)
    {
    case viewmodeAlwaysEditing:
        editDocument(true);
        break;
    case viewmodeAlwaysReading:
        editDocument(false);
        break;
    default:
        break;
    }
}

void CWizDocumentView::setModified(bool modified)
{
    m_title->setModified(modified);
    m_web->setModified(modified);
}

void CWizDocumentView::settingsChanged()
{
    setViewMode(m_userSettings.noteViewMode());
}

void CWizDocumentView::on_titleEdit_textChanged(const QString& strTitle)
{
    // apply modify immediately to avoid user switch document
    if (!m_dataDelay.strGUID.isEmpty() && m_dataDelay.strGUID != m_web->document().strGUID) {
        // FIXME: other thread will try to read and set document info during delay time
        // delay 1/10 second for them finished writing.
        m_timerDelay.start(100);
    }

    // programmatic change, switch document
    if (m_web->document().strTitle == strTitle) {
        return;
    }

    if (strTitle.isEmpty()) {
       return;
    }

    // Only 255 max chars accept by title
    m_dataDelay = m_web->document();
    m_dataDelay.strTitle = strTitle.left(255);
    m_timerDelay.start(1000);
}

void CWizDocumentView::on_titleEdit_textEdit_writeDelay()
{
    Q_ASSERT(!m_dataDelay.strGUID.isEmpty());

    WIZDOCUMENTDATA data;
    CWizDatabase& db = m_dbMgr.db(m_dataDelay.strKbGUID);
    if (db.DocumentFromGUID(m_dataDelay.strGUID, data)) {
        data.strTitle = m_dataDelay.strTitle;
        db.ModifyDocumentInfo(data);
    }

    m_dataDelay = WIZDOCUMENTDATA();

    m_timerDelay.stop();
}

void CWizDocumentView::on_editDocumentButton_clicked()
{
    editDocument(!m_editingDocument);
}

void CWizDocumentView::on_attachmentButton_clicked()
{
    if (!m_attachments) {
        m_attachments = new CWizAttachmentListWidget(m_app, topLevelWidget());
    }

    m_attachments->setDocument(m_web->document());

    QPushButton* btn = m_title->attachmentButton();
    QRect rc = btn->geometry();
    QPoint pt = btn->mapToGlobal(QPoint(rc.width() / 2, rc.height()));
    m_attachments->showAtPoint(pt);
}

void CWizDocumentView::on_tagsButton_clicked()
{
    if (!m_tags) {
        m_tags = new CWizTagListWidget(m_dbMgr, topLevelWidget());
    }

    m_tags->setDocument(m_web->document());

    QPushButton* btn = m_title->tagsButton();
    QRect rc = btn->geometry();
    QPoint pt = btn->mapToGlobal(QPoint(rc.width() / 2, rc.height()));
    m_tags->showAtPoint(pt);
}

void CWizDocumentView::on_infoButton_clicked()
{
    if (!m_info) {
        m_info = new CWizNoteInfoForm(m_dbMgr, topLevelWidget());
    }

    m_info->setDocument(m_web->document());

    QPushButton* btn = m_title->infoButton();
    QRect rc = btn->geometry();
    QPoint pt = btn->mapToGlobal(QPoint(rc.width() / 2, rc.height()));
    m_info->showAtPoint(pt);
}

void CWizDocumentView::on_attachment_created(const WIZDOCUMENTATTACHMENTDATA& attachment)
{
    if (attachment.strDocumentGUID == document().strGUID) {
        QString strKbGUID = document().strKbGUID;
        m_title->updateInformation(m_dbMgr.db(strKbGUID), document());
    }
}

void CWizDocumentView::on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA& attachment)
{
    if (attachment.strDocumentGUID == document().strGUID) {
        QString strKbGUID = document().strKbGUID;
        m_title->updateInformation(m_dbMgr.db(strKbGUID), document());
    }
}

void CWizDocumentView::on_document_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew)
{
    Q_UNUSED(documentOld);

    if (document().strGUID == documentNew.strGUID) {
        m_web->reloadDocument();

        QString strKbGUID = document().strKbGUID;
        m_title->updateInformation(m_dbMgr.db(strKbGUID), document());
    }
}
