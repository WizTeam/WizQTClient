#include "wizdocumentview.h"

#include <QtGui>
#include <QWebElement>
#include <QWebFrame>

#include "share/wizimagepushbutton.h"
#include "wizdocumentwebview.h"
#include "wiztaglistwidget.h"
#include "wizattachmentlistwidget.h"
#include "wiznoteinfoform.h"
#include "wiznotestyle.h"
#include "wizEditorToolBar.h"

class CWizTitleBar : public QWidget
{
public:
    CWizTitleBar(CWizExplorerApp& app, QWidget* parent)
        : QWidget(parent)
        , m_app(app)
        , m_editing(false)
    {
        setContentsMargins(4, 4, 4, 4);

        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        setLayout(layout);

        m_titleEdit = new QLineEdit(this);
        m_titleEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
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
    QPointer<CWizImagePushButton> m_editDocumentButton;
    QPointer<CWizImagePushButton> m_tagsButton;
    QPointer<CWizImagePushButton> m_attachmentButton;
    QPointer<CWizImagePushButton> m_infoButton;

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

class CWizInfoToolBar : public QWidget
{
public:
    CWizInfoToolBar(CWizExplorerApp& app, QWidget* parent = 0)
        : QWidget(parent)
        , m_app(app)
    {
        setStyleSheet("font-size: 11px; color: #646464;");

        // the height of Editor Toolbar is 32, hard-coded here.
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(8, 10, 8, 9);
        layout->setSpacing(15);
        setLayout(layout);

        m_labelCreatedTime = new QLabel(this);
        m_labelModifiedTime = new QLabel(this);
        m_labelAuthor = new QLabel(this);
        m_labelSize = new QLabel(this);

        layout->addWidget(m_labelCreatedTime);
        layout->addWidget(m_labelModifiedTime);
        layout->addWidget(m_labelAuthor);
        layout->addWidget(m_labelSize);
        layout->addStretch();
    }

    void setDocument(const WIZDOCUMENTDATA& data)
    {
        QString strCreateTime = QObject::tr("Create time: ") + data.tCreated.toString("yyyy-MM-dd");
        m_labelCreatedTime->setText(strCreateTime);

        QString strModifiedTime = QObject::tr("Update time: ") + data.tModified.toString("yyyy-MM-dd");
        m_labelModifiedTime->setText(strModifiedTime);

        QString strAuthor = QObject::tr("Author: ") + data.strOwner;
        strAuthor = fontMetrics().elidedText(strAuthor, Qt::ElideRight, 150);
        m_labelAuthor->setText(strAuthor);

        QString strFile = m_app.databaseManager().db(data.strKbGUID).GetDocumentFileName(data.strGUID);
        QString strSize = QObject::tr("Size: ") + ::WizGetFileSizeHumanReadalbe(strFile);
        m_labelSize->setText(strSize);
    }

private:
    CWizExplorerApp& m_app;
    QPointer<QLabel> m_labelCreatedTime;
    QPointer<QLabel> m_labelModifiedTime;
    QPointer<QLabel> m_labelAuthor;
    QPointer<QLabel> m_labelSize;
};

class CWizNotifyToolbar : public QWidget
{
public:
    CWizNotifyToolbar(CWizExplorerApp& app, QWidget* parent = 0)
        : QWidget(parent)
        , m_app(app)
    {
        setStyleSheet("* {font-size:12px; color: #FFFFFF;}\
                      *:active {background-image: url(" + ::WizGetResourcesPath() + "skins/notify_bg.png);}\
                      *:!active {background-image: url(" + ::WizGetResourcesPath() + "skins/notify_bg_inactive.png);}");

        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(8, 5, 8, 5);
        layout->setSpacing(15);
        setLayout(layout);

        m_labelNotify = new QLabel(this);
        m_labelNotify->setAttribute(Qt::WA_NoSystemBackground, true);
        layout->addWidget(m_labelNotify);
        layout->addStretch();
    }

    void setNotifyText(const QString& strMsg)
    {
        m_labelNotify->setText(strMsg);
    }

private:
    CWizExplorerApp& m_app;
    QPointer<QLabel> m_labelNotify;
};



CWizDocumentView::CWizDocumentView(CWizExplorerApp& app, QWidget* parent)
    : QWidget(parent)
    , m_app(app)
    , m_userSettings(app.userSettings())
    , m_dbMgr(app.databaseManager())
    , m_title(new CWizTitleBar(app, this))
    , m_infoToolBar(new CWizInfoToolBar(app))
    , m_notifyToolBar(new CWizNotifyToolbar(app))
    , m_web(new CWizDocumentWebView(app, this))
    , m_editorToolBar(new CWizEditorToolBar(app))
    , m_editingDocument(true)
    , m_viewMode(app.userSettings().noteViewMode())
{
    m_client = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    m_client->setLayout(layout);

    QWidget* line = new QWidget(this);
    line->setMaximumHeight(1);
    line->setMinimumHeight(1);
    line->setStyleSheet("border-bottom-width:1;border-bottom-style:solid;border-bottom-color:#bbbbbb");

    layout->addWidget(m_title);
    layout->addWidget(line);
    layout->addWidget(m_editorToolBar);
    layout->addWidget(m_infoToolBar);
    layout->addWidget(m_notifyToolBar);
    layout->addWidget(m_web);

    layout->setStretchFactor(m_title, 0);
    layout->setStretchFactor(m_web, 1);

    QVBoxLayout* layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    setLayout(layoutMain);
    layoutMain->addWidget(m_client);

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

    // webview focus
    connect(m_web, SIGNAL(focusIn()), SLOT(on_webview_focusIn()));
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

    // reset
    m_notifyToolBar->hide();
    m_title->editDocumentButton()->show();

    if (CWizDatabase::IsInDeletedItems(data.strLocation)) {
        m_notifyToolBar->setNotifyText(tr("This document is deleted, You can edit after move to other folders."));
        m_notifyToolBar->show();
        m_title->editDocumentButton()->hide();
        edit = false;
    }

    // check user permission
    int perm = m_dbMgr.db(data.strKbGUID).permission();
    QString strUserId = m_dbMgr.db().getUserId();
    // only reading is permit
    if (perm > WIZ_USERGROUP_AUTHOR ||
            (perm == WIZ_USERGROUP_AUTHOR && data.strOwner != strUserId)) {
        m_notifyToolBar->setNotifyText(tr("Your permission is not enough to edit this document."));
        m_title->editDocumentButton()->hide();
        m_notifyToolBar->show();
        edit = false;
    }

    // load document
    m_web->viewDocument(data, edit);

    m_editingDocument = edit;
    m_title->setEditingDocument(m_editingDocument);
    m_title->updateInformation(m_dbMgr.db(data.strKbGUID), data);
    m_infoToolBar->setDocument(data);

    m_infoToolBar->show();
    m_editorToolBar->hide();

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

void CWizDocumentView::on_webview_focusIn()
{
    m_infoToolBar->hide();
    m_editorToolBar->show();
}
