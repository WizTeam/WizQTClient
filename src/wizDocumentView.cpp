#include "wizDocumentView.h"

#include <QtGui>
#include <QWebElement>
#include <QWebFrame>

#include "widgets/wizScrollBar.h"

#include "share/wizimagepushbutton.h"
#include "wizDocumentWebView.h"
#include "wiztaglistwidget.h"
#include "wizattachmentlistwidget.h"
#include "wiznoteinfoform.h"
#include "wiznotestyle.h"
#include "wizEditorToolBar.h"
#include "widgets/wizSegmentedButton.h"
#include "widgets/qsegmentcontrol.h"


class CWizTitleEdit : public QLineEdit
{
public:
    CWizTitleEdit(CWizExplorerApp& app, QWidget* parent = 0)
        : QLineEdit(parent)
        , m_dbMgr(app.databaseManager())
    {
        setStyleSheet("font-size: 12px;");
        setContentsMargins(5, 0, 0, 0);
        setAlignment(Qt::AlignLeft | Qt::AlignBottom);
        setAttribute(Qt::WA_MacShowFocusRect, false);
        setFrame(false);
    }

    void setDocument(const WIZDOCUMENTDATA& data, bool bEdit)
    {
    }

protected:
    virtual void inputMethodEvent(QInputMethodEvent* event)
    {
        // FIXME: This should be a QT bug
        // when use input method, input as normal is fine, but input as selection text
        // will lose blink cursor, we should reset cursor position first!!!
        if (hasSelectedText()) {
            del();
        }

        QLineEdit::inputMethodEvent(event);
    }

    virtual QSize sizeHint() const
    {
        return QSize(fontMetrics().width(text()), fontMetrics().height() + 10);
    }

private:
    CWizDatabaseManager& m_dbMgr;
};


class CWizInfoToolBar : public QWidget
{
public:
    CWizInfoToolBar(CWizExplorerApp& app, QWidget* parent = 0)
        : QWidget(parent)
        , m_app(app)
    {
        setStyleSheet("font-size: 11px; color: #646464;");
        setContentsMargins(5, 0, 0, 0);

        // FIXME: should be the same as editor toolbar
        setFixedHeight(32);

        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(10);
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
    QLabel* m_labelCreatedTime;
    QLabel* m_labelModifiedTime;
    QLabel* m_labelAuthor;
    QLabel* m_labelSize;
};


class CWizNotifyToolbar : public QWidget
{
public:
    enum NotifyType
    {
        DocumentLocked,
        DocumentIsDeleted,
        DocumentPermissionLack
    };

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


class CWizTitleBar : public QWidget
{
public:
    enum Buttons {
        ButtonEdit,
        ButtonTag,
        ButtonAttachment,
        ButtonInfo
    };

    CWizTitleBar(CWizExplorerApp& app, QWidget* parent)
        : QWidget(parent)
        , m_app(app)
        , m_dbMgr(app.databaseManager())
        , m_editing(false)
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        setLayout(layout);

        m_editTitle = new CWizTitleEdit(app, this);
        m_infoBar = new CWizInfoToolBar(app, this);
        m_editorBar = new CWizEditorToolBar(app, this);
        m_notifyBar = new CWizNotifyToolbar(app, this);

        m_unlockIcon = ::WizLoadSkinIcon(m_app.userSettings().skin(), "document_unlock");
        m_lockIcon = ::WizLoadSkinIcon(m_app.userSettings().skin(), "document_lock");
        m_tagsIcon = ::WizLoadSkinIcon(m_app.userSettings().skin(), "document_tag");
        m_attachNoneIcon = ::WizLoadSkinIcon(m_app.userSettings().skin(), "document_attachment");
        m_attachExistIcon = ::WizLoadSkinIcon(m_app.userSettings().skin(), "document_attachment_exist");
        m_infoIcon = ::WizLoadSkinIcon(m_app.userSettings().skin(), "document_info");

        m_segment = new QtSegmentControl(this);
        m_segment->setSelectionBehavior(QtSegmentControl::SelectNone);
        m_segment->setCount(4);
        m_segment->setSegmentIcon(0, m_lockIcon);
        m_segment->setSegmentIcon(1, m_tagsIcon);
        m_segment->setSegmentIcon(2, m_attachNoneIcon);
        m_segment->setSegmentIcon(3, m_infoIcon);
        m_segment->setIconSize(QSize(16, 16));

        QWidget* line1 = new QWidget(this);
        line1->setFixedHeight(12);
        line1->setStyleSheet("border-top-width:1;border-top-style:solid;border-top-color:#646464");

        QWidget* line2 = new QWidget(this);
        line2->setFixedHeight(1);
        line2->setFixedWidth(10);
        line2->setStyleSheet("border-bottom-width:1;border-bottom-style:solid;border-bottom-color:#646464");

        QVBoxLayout* layoutInfo1 = new QVBoxLayout();
        layoutInfo1->setContentsMargins(0, 0, 0, 0);
        layoutInfo1->setSpacing(0);
        layoutInfo1->addWidget(m_infoBar);
        layoutInfo1->addWidget(m_editorBar);
        layoutInfo1->addWidget(line1);
        m_editorBar->hide();

        QHBoxLayout* layoutInfo2 = new QHBoxLayout();
        layoutInfo2->setContentsMargins(0, 0, 0, 0);
        layoutInfo2->setSpacing(0);
        layoutInfo2->addWidget(m_segment);
        layoutInfo2->addWidget(line2);

        QVBoxLayout* layoutInfo3 = new QVBoxLayout();
        layoutInfo3->addStretch();
        layoutInfo3->addLayout(layoutInfo2);

        QHBoxLayout* layoutInfo4 = new QHBoxLayout();
        layoutInfo4->setContentsMargins(0, 0, 0, 0);
        layoutInfo4->setSpacing(0);
        layoutInfo4->addLayout(layoutInfo1);
        layoutInfo4->addLayout(layoutInfo3);

        layout->addWidget(m_editTitle);
        layout->addLayout(layoutInfo4);
        layout->addWidget(m_notifyBar);
        m_notifyBar->hide();
    }

private:
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;

    CWizTitleEdit* m_editTitle;
    CWizInfoToolBar* m_infoBar;
    CWizEditorToolBar* m_editorBar;
    CWizNotifyToolbar* m_notifyBar;
    QtSegmentControl* m_segment;

    QIcon m_unlockIcon;
    QIcon m_lockIcon;
    QIcon m_tagsIcon;
    QIcon m_attachNoneIcon;
    QIcon m_attachExistIcon;
    QIcon m_infoIcon;

    bool m_editing;
    bool m_bLocked;

private:
    void updateEditDocumentButtonIcon(bool editing)
    {
        m_segment->setSegmentIcon(ButtonEdit, editing ? m_unlockIcon : m_lockIcon);
        m_editing = editing;

        updateEditDocumentButtonTooltip();
    }

    void updateEditDocumentButtonTooltip()
    {
        //QString shortcut = ::WizGetShortcut("EditNote", "Alt+1");
        //QString strSaveAndRead = QObject::tr("Save & Switch to Reading View");
        //QString strRead = QObject::tr("Switch to Reading View");
        //QString strEditNote = QObject::tr("Switch to Editing View");
        //QString strSwitchRead = m_editDocumentButton->text().isEmpty() ? strRead : strSaveAndRead;
        //QString strToolTip = m_editing ? strSwitchRead : strEditNote;
        //strToolTip += " (" + shortcut + ")";
        //m_segment->setSegmentToolTip(ButtonEdit, strToolTip);
        //m_editDocumentButton->setShortcut(QKeySequence::fromString(shortcut));
    }

public:
    CWizTitleEdit* titleEdit() const { return m_editTitle; }
    CWizInfoToolBar* infoBar() const { return m_infoBar; }
    CWizEditorToolBar* editorBar() const { return m_editorBar; }
    CWizNotifyToolbar* notifyBar() const { return m_notifyBar; }
    QtSegmentControl* segmentButton() const { return m_segment; }

    void showNotify(CWizNotifyToolbar::NotifyType type)
    {
        switch (type) {
        case CWizNotifyToolbar::DocumentLocked:
            m_notifyBar->setNotifyText(tr("The document is locked and read only, press unlock button if you need edit."));
            break;
        case CWizNotifyToolbar::DocumentIsDeleted:
            m_notifyBar->setNotifyText(tr("This document is deleted, You can edit after move to other folders."));
            break;
        case CWizNotifyToolbar::DocumentPermissionLack:
            m_notifyBar->setNotifyText(tr("Your permission is not enough to edit this document."));
            break;
        defaut:
            Q_ASSERT(0);
        }

        m_notifyBar->show();
    }

    void setDocument(const WIZDOCUMENTDATA& data, bool editing, bool locked)
    {
        // indicate document is editable or not
        m_bLocked = locked;

        m_editTitle->setText(data.strTitle);
        m_segment->setSegmentEnabled(ButtonEdit, !locked);
        m_segment->setSegmentIcon(ButtonAttachment, data.nAttachmentCount > 0 ? m_attachExistIcon : m_attachNoneIcon);
        m_infoBar->setDocument(data);

        setEditingDocument(editing);
    }

    void setEditingDocument(bool editing)
    {
        // editing locked document is not allowed
        if (editing && m_bLocked)
            return;

        // Qt-bug: Must always set this flag after setReadOnly
        m_editTitle->setReadOnly(!editing);
        m_editTitle->setAttribute(Qt::WA_MacShowFocusRect, false);

        updateEditDocumentButtonIcon(editing);
    }

    void updateInformation(CWizDatabase& db, const WIZDOCUMENTDATA& doc)
    {
        // retrieve document info and reset
        WIZDOCUMENTDATA data;
        if (!db.DocumentFromGUID(doc.strGUID, data)) {
            return;
        }

        //title
//        if (m_titleEdit->text() != data.strTitle) {
//            m_titleEdit->setText(data.strTitle);
//        }

        // update tags count only if it's enabled
        //if (m_tagsButton->isEnabled()) {
        //    CWizStdStringArray arrayTagGUID;
        //    db.GetDocumentTags(data.strGUID, arrayTagGUID);
        //    QString strTagText = arrayTagGUID.empty() ? QString() : QString::number(arrayTagGUID.size());
        //    m_tagsButton->setText(strTagText);
        //}

        //QString tagsShortcut = ::WizGetShortcut("EditNoteTags", "Alt+2");
        //QString strTagsToolTip = QObject::tr("Tags (%1)").arg(tagsShortcut);
        //m_tagsButton->setToolTip(strTagsToolTip);
        //m_tagsButton->setShortcut(QKeySequence::fromString(tagsShortcut));

        //attachments
        int nAttachmentCount = db.GetDocumentAttachmentCount(data.strGUID);
        m_segment->setSegmentIcon(ButtonAttachment, nAttachmentCount > 0 ? m_attachExistIcon : m_attachNoneIcon);
        //CString strAttachmentText = nAttachmentCount ? WizIntToStr(nAttachmentCount) : CString();
        //m_attachmentButton->setText(strAttachmentText);
        //QString attachmentShortcut = ::WizGetShortcut("EditNoteAttachments", "Alt+3");
        //m_attachmentButton->setToolTip(QObject::tr("Attachments (%1)").arg(attachmentShortcut));
        //m_attachmentButton->setShortcut(QKeySequence::fromString(attachmentShortcut));
    }

    void setModified(bool modified)
    {
        //m_editDocumentButton->setText(modified ? "*" : "");
        //updateEditDocumentButtonTooltip();
    }
};


CWizDocumentView::CWizDocumentView(CWizExplorerApp& app, QWidget* parent)
    : QWidget(parent)
    , m_app(app)
    , m_userSettings(app.userSettings())
    , m_dbMgr(app.databaseManager())
    , m_title(new CWizTitleBar(app, this))
    , m_web(new CWizDocumentWebView(app, this))
    , m_editingDocument(true)
    , m_viewMode(app.userSettings().noteViewMode())
{
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_client = new QWidget(this);
    m_client->setLayout(layout);

    layout->addWidget(m_title);
    layout->addWidget(m_web);

    layout->setStretchFactor(m_title, 0);
    layout->setStretchFactor(m_web, 1);

    QVBoxLayout* layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    setLayout(layoutMain);
    layoutMain->addWidget(m_client);

    m_title->setEditingDocument(m_editingDocument);

    connect(m_title->titleEdit(), SIGNAL(editingFinished()),
            SLOT(on_titleEdit_editingFinished()));

    connect(m_title->titleEdit(), SIGNAL(returnPressed()),
            SLOT(on_titleEdit_returnPressed()));

    connect(m_title->segmentButton(), SIGNAL(segmentClicked(int)),
            SLOT(on_segmentButton_clicked(int)));

    connect(&m_dbMgr, SIGNAL(documentModified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)), \
            SLOT(on_document_modified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(attachmentCreated(const WIZDOCUMENTATTACHMENTDATA&)), \
            SLOT(on_attachment_created(const WIZDOCUMENTATTACHMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(attachmentDeleted(const WIZDOCUMENTATTACHMENTDATA&)), \
            SLOT(on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA&)));

    // webview related
    connect(m_web, SIGNAL(focusIn()), SLOT(on_webview_focusIn()));
    connect(m_web, SIGNAL(focusOut()), SLOT(on_webview_focusOut()));
}

CWizEditorToolBar* CWizDocumentView::editorToolBar() const
{
    return m_title->editorBar();
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
    m_title->segmentButton()->setSegmentEnabled(CWizTitleBar::ButtonEdit, !b);

    // tag is not avaliable for group
    if (isGroup) {
        m_title->segmentButton()->setSegmentEnabled(CWizTitleBar::ButtonTag, false);
    } else {
        m_title->segmentButton()->setSegmentEnabled(CWizTitleBar::ButtonTag, true);
    }
}

bool CWizDocumentView::viewDocument(const WIZDOCUMENTDATA& data, bool forceEdit)
{
    bool edit = false;
    bool locked = false;

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


    if (!edit) {
        m_title->showNotify(CWizNotifyToolbar::DocumentLocked);
        edit = false;
    }

    if (CWizDatabase::IsInDeletedItems(data.strLocation)) {
        m_title->showNotify(CWizNotifyToolbar::DocumentIsDeleted);
        edit = false;
        locked = true;
    }

    if (!m_dbMgr.db(data.strKbGUID).CanEditDocument(data)) {
        m_title->showNotify(CWizNotifyToolbar::DocumentPermissionLack);
        edit = false;
        locked = true;
    }

    if (edit) {
        m_title->notifyBar()->hide();
    }

    // load document
    m_title->setDocument(data, edit, locked);
    m_web->viewDocument(data, edit);

    m_editingDocument = edit;
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
    if (!editing) {
        m_title->showNotify(CWizNotifyToolbar::DocumentLocked);
        //m_editTitle->setReadOnly(true);
    } else {
        m_title->notifyBar()->hide();
        //m_editTitle->setReadOnly(false);
    }

    m_title->setEditingDocument(editing);
    m_web->setEditingDocument(editing);

    m_editingDocument = editing;
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

void CWizDocumentView::on_titleEdit_editingFinished()
{
    WIZDOCUMENTDATA data;
    CWizDatabase& db = m_dbMgr.db(m_web->document().strKbGUID);
    if (db.DocumentFromGUID(m_web->document().strGUID, data)) {
        QString strNewTitle = m_title->titleEdit()->text().left(255);
        if (strNewTitle != data.strTitle) {
            data.strTitle = strNewTitle;
            db.ModifyDocumentInfo(data);
        }
    }
}

void CWizDocumentView::on_titleEdit_returnPressed()
{
    m_web->setFocus(Qt::MouseFocusReason);
    m_web->editorFocus();
}

void CWizDocumentView::on_segmentButton_clicked(int index)
{
    switch (index) {
    case CWizTitleBar::ButtonEdit:
        editDocument(!m_editingDocument);
        break;
    case CWizTitleBar::ButtonTag:
        showListTag();
        break;
    case CWizTitleBar::ButtonAttachment:
        showListAttachment();
        break;
    case CWizTitleBar::ButtonInfo:
        showListInfo();
        break;
    default:
        Q_ASSERT(0);
    }
}

void CWizDocumentView::showListTag()
{
    if (!m_tags) {
        m_tags = new CWizTagListWidget(m_dbMgr, topLevelWidget());
    }

    m_tags->setDocument(m_web->document());

    QRect rc = m_title->segmentButton()->segmentRect(CWizTitleBar::ButtonTag);
    QPoint pt = m_title->segmentButton()->mapToGlobal(QPoint(rc.x() + rc.width()/2, rc.y() + rc.height()));
    m_tags->showAtPoint(pt);
}

void CWizDocumentView::showListAttachment()
{
    if (!m_attachments) {
        m_attachments = new CWizAttachmentListWidget(m_app, topLevelWidget());
    }

    m_attachments->setDocument(m_web->document());

    QRect rc = m_title->segmentButton()->segmentRect(CWizTitleBar::ButtonAttachment);
    QPoint pt = m_title->segmentButton()->mapToGlobal(QPoint(rc.x() + rc.width()/2, rc.y() + rc.height()));
    m_attachments->showAtPoint(pt);
}

void CWizDocumentView::showListInfo()
{
    if (!m_info) {
        m_info = new CWizNoteInfoForm(m_dbMgr, topLevelWidget());
    }

    m_info->setDocument(m_web->document());

    QRect rc = m_title->segmentButton()->segmentRect(CWizTitleBar::ButtonInfo);
    QPoint pt = m_title->segmentButton()->mapToGlobal(QPoint(rc.x() + rc.width()/2, rc.y() + rc.height()));
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
    m_title->infoBar()->hide();
    m_title->editorBar()->show();
}

void CWizDocumentView::on_webview_focusOut()
{
    m_title->editorBar()->hide();
    m_title->infoBar()->show();
}
