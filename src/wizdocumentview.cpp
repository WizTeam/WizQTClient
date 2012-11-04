#include "wizdocumentview.h"
#include "wizdocumentwebview.h"
#include "wiztaglistwidget.h"
#include "wizattachmentlistwidget.h"
#include "wiznotestyle.h"
#include "share/wizsettings.h"

#include <QWebElement>
#include <QWebFrame>

#include <QBoxLayout>
#include <QLineEdit>
#include <QApplication>
#include "share/wizimagepushbutton.h"

class CWizTitleBar
    : public QWidget
{
public:
    CWizTitleBar(CWizExplorerApp& app, QWidget* parent)
        : QWidget(parent)
        , m_app(app)
        , m_titleEdit(NULL)
        , m_editDocumentButton(NULL)
        , m_tagsButton(NULL)
        , m_attachmentButton(NULL)
        , m_editing(false)
    {
        QHBoxLayout* layout = new QHBoxLayout(this);
        setLayout(layout);
        layout->setMargin(0);

        setContentsMargins(4, 4, 4, 4);
        //
        QPalette pal = palette();
        pal.setColor(QPalette::Window, QColor(0xff, 0xff, 0xff));
        setPalette(pal);
        //
        m_titleEdit = new QLineEdit(this);
#ifdef Q_OS_MAC
        m_titleEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
        m_titleEdit->setStyleSheet("QLineEdit{margin:4 0 0 0; padding:4 4 4 4;border-color:#ffffff;border-width:1;border-style:solid;}QLineEdit:hover{border-color:#bbbbbb;border-width:1;border-style:solid;}");
        //m_titleEdit->setAlignment(Qt::AlignVCenter);
#else
        m_titleEdit->setStyleSheet("QLineEdit{padding:4 4 4 4;border-color:#ffffff;border-width:1;border-style:solid;}QLineEdit:hover{border-color:#bbbbbb;border-width:1;border-style:solid;}");
#endif
        //
        m_editIcon = ::WizLoadSkinIcon(m_app.userSettings().skin(), "unlock");
        m_commitIcon = ::WizLoadSkinIcon(m_app.userSettings().skin(), "lock");
        m_tagsIcon = ::WizLoadSkinIcon(m_app.userSettings().skin(), "document_tags");
        m_attachmentIcon = ::WizLoadSkinIcon(m_app.userSettings().skin(), "attachment");
        //
        m_editDocumentButton = new CWizImagePushButton(m_editIcon, "", this);
        updateEditDocumentButtonIcon(false);
        m_editDocumentButton->setStyle(::WizGetStyle(m_app.userSettings().skin()));
        m_editDocumentButton->setRedFlag(true);
        //
        m_tagsButton = new CWizImagePushButton(m_tagsIcon, "", this);
        m_tagsButton->setStyle(::WizGetStyle(m_app.userSettings().skin()));
        //
        m_attachmentButton = new CWizImagePushButton(m_attachmentIcon, "", this);
        m_attachmentButton->setStyle(::WizGetStyle(m_app.userSettings().skin()));
        //
        layout->addWidget(m_titleEdit);
        layout->addWidget(m_editDocumentButton);
        layout->addWidget(m_tagsButton);
        layout->addWidget(m_attachmentButton);
        layout->setAlignment(m_titleEdit, Qt::AlignVCenter);
        layout->setAlignment(m_editDocumentButton, Qt::AlignVCenter);
        layout->setAlignment(m_tagsButton, Qt::AlignVCenter);
        layout->setAlignment(m_attachmentButton, Qt::AlignVCenter);
        //
    }

private:
    CWizExplorerApp& m_app;
    QLineEdit* m_titleEdit;
    CWizImagePushButton* m_editDocumentButton;
    CWizImagePushButton* m_tagsButton;
    CWizImagePushButton* m_attachmentButton;
    //
    QIcon m_editIcon;
    QIcon m_commitIcon;
    QIcon m_tagsIcon;
    QIcon m_attachmentIcon;
    //
    bool m_editing;
private:
    void updateEditDocumentButtonIcon(bool editing)
    {
        m_editDocumentButton->setIcon(editing ? m_commitIcon : m_editIcon);
        m_editing = editing;
        //
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
    //
    void setEditingDocument(bool editing) { updateEditDocumentButtonIcon(editing); }
    void setTitle(const QString& str) { m_titleEdit->setText(str); }
    //
    void updateInformation(CWizDatabase& db, const WIZDOCUMENTDATA& data)
    {
        //title
        m_titleEdit->setText(data.strTitle);

        //tags
        CWizStdStringArray arrayTagGUID;
        db.GetDocumentTags(data.strGUID, arrayTagGUID);

        QString strTagText = arrayTagGUID.empty() ? QString() : QString::number(arrayTagGUID.size());
        m_tagsButton->setText(strTagText);

        QString tagsShortcut = ::WizGetShortcut("EditNoteTags", "Alt+2");
        QString strTagsToolTip = QObject::tr("Tags (%1)").arg(tagsShortcut);
        //strTagsToolTip = strTagsToolTip.arg(tagsShortcut).arg(db.GetDocumentTagDisplayNameText(data.strGUID));
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
    //
    void setModified(bool modified)
    {
        m_editDocumentButton->setText(modified ? "*" : "");
        updateEditDocumentButtonTooltip();
    }
};



CWizDocumentView::CWizDocumentView(CWizExplorerApp& app, QWidget* parent)
    : QWidget(parent)
    , m_userSettings(app.userSettings())
    , m_db(app.database())
    , m_title(new CWizTitleBar(app, this))
    , m_web(new CWizDocumentWebView(app, this))
    , m_client(NULL)
    , m_tags(NULL)
    , m_attachments(NULL)
    , m_editingDocument(true)
    , m_viewMode(app.userSettings().noteViewMode())
{
    m_specialPage = new QWebView(this);

    m_client = createClient();

    QBoxLayout* layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->addWidget(m_client);
    layout->addWidget(m_specialPage);
    layout->setMargin(0);

    m_specialPage->hide();

    m_title->setEditingDocument(m_editingDocument);

    connect(m_title->titleEdit(), SIGNAL(editingFinished()), \
            SLOT(on_titleEdit_editingFinished()));

    connect(m_title->editDocumentButton(), SIGNAL(clicked()), \
            SLOT(on_editDocumentButton_clicked()));

    connect(m_title->tagsButton(), SIGNAL(clicked()), \
            SLOT(on_tagsButton_clicked()));

    connect(m_title->attachmentButton(), SIGNAL(clicked()), \
            SLOT(on_attachmentButton_clicked()));

    qRegisterMetaType<WIZDOCUMENTDATA>("WIZDOCUMENTDATA");
    qRegisterMetaType<WIZDOCUMENTATTACHMENTDATA>("WIZDOCUMENTATTACHMENTDATA");

    connect(&m_db, SIGNAL(documentModified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)), \
            SLOT(on_document_modified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)));

    connect(&m_db, SIGNAL(attachmentCreated(const WIZDOCUMENTATTACHMENTDATA&)), \
            SLOT(on_attachment_created(const WIZDOCUMENTATTACHMENTDATA&)));

    connect(&m_db, SIGNAL(attachmentDeleted(const WIZDOCUMENTATTACHMENTDATA&)), \
            SLOT(on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA&)));
}


QWidget* CWizDocumentView::createClient()
{
    QWidget* client = new QWidget(this);
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, client);
    client->setLayout(layout);

    client->setAutoFillBackground(true);

    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(0xff, 0xff, 0xff));
    client->setPalette(pal);

    QWidget* line = new QWidget(this);
    line->setMaximumHeight(1);
    line->setMinimumHeight(1);
    line->setStyleSheet("border-bottom-width:1;border-bottom-style:solid;border-bottom-color:#bbbbbb");

    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_title);
    layout->addWidget(line);
    layout->addWidget(m_web);

    layout->setStretchFactor(m_title, 0);
    layout->setStretchFactor(m_web, 1);

    return client;
}

void CWizDocumentView::showClient(bool visible)
{
    if (visible) {
        m_client->show();
        m_specialPage->hide();
    } else {
        m_client->hide();
    }
}

void CWizDocumentView::loadSpecialPage(const QString& strFileName)
{
    showClient(false);

    QString strHtml;
    ::WizLoadUnicodeTextFromFile(strFileName, strHtml);
    QUrl url = QUrl::fromLocalFile(strFileName);
    m_specialPage->setHtml(strHtml, url);

    m_specialPage->show();
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

    bool ret = m_web->viewDocument(data, edit);
    if (!ret) {
        showClient(false);
        return false;
    }

    showClient(true);
    editDocument(edit);
    m_title->updateInformation(m_db, data);

    return true;
}

const WIZDOCUMENTDATA& CWizDocumentView::document()
{
    static WIZDOCUMENTDATA empty;
    if (!isVisible())
        return empty;
    //
    return m_web->document();
}

void CWizDocumentView::editDocument(bool editing)
{
    m_editingDocument = editing;
    m_title->setEditingDocument(m_editingDocument);
    m_web->setEditingDocument(m_editingDocument);
    m_web->updateSize();
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
}

void CWizDocumentView::settingsChanged()
{
    setViewMode(m_userSettings.noteViewMode());
}

void CWizDocumentView::on_titleEdit_editingFinished()
{
    QString title = m_title->titleEdit()->text();
    if (title.length() > 255)
    {
        title = title.left(255);
    }
    //
    WIZDOCUMENTDATA data;
    if (m_db.DocumentFromGUID(m_web->document().strGUID, data))
    {
        data.strTitle = title;
        m_db.ModifyDocumentInfo(data);
    }
}

void CWizDocumentView::on_editDocumentButton_clicked()
{
    editDocument(!m_editingDocument);
}

void CWizDocumentView::on_attachmentButton_clicked()
{
    if (!m_attachments) {
        m_attachments = new CWizAttachmentListWidget(m_web->app(), topLevelWidget());
    }

    m_attachments->setDocument(m_web->document());

    QPushButton* btn = m_title->attachmentButton();
    QRect rc = btn->geometry();
    QPoint pt = btn->mapToGlobal(QPoint(rc.width() / 2, rc.height()));
    m_attachments->setGeometry(QRect(QPoint(0, 0), m_attachments->sizeHint()));
    m_attachments->showAtPoint(pt);
}

void CWizDocumentView::on_tagsButton_clicked()
{
    if (!m_tags) {
        m_tags = new CWizTagListWidget(m_db, topLevelWidget());
    }

    m_tags->setDocument(m_web->document());

    QPushButton* btn = m_title->tagsButton();
    QRect rc = btn->geometry();
    QPoint pt = btn->mapToGlobal(QPoint(rc.width() / 2, rc.height()));
    m_tags->setGeometry(QRect(QPoint(0, 0), m_tags->sizeHint()));
    m_tags->showAtPoint(pt);
}

void CWizDocumentView::on_attachment_created(const WIZDOCUMENTATTACHMENTDATA& attachment)
{
    if (attachment.strDocumentGUID == document().strGUID)
    {
        m_title->updateInformation(m_db, document());
    }
}

void CWizDocumentView::on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA& attachment)
{
    if (attachment.strDocumentGUID == document().strGUID)
    {
        m_title->updateInformation(m_db, document());
    }
}

void CWizDocumentView::on_document_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew)
{
    Q_UNUSED(documentOld);
    //
    if (document().strGUID == documentNew.strGUID)
    {
        m_web->reloadDocument();
        m_title->updateInformation(m_db, document());
    }
}
