#include "titlebar.h"

#include <QVBoxLayout>
#include <QUrl>
#include <QDebug>
#include <QNetworkConfigurationManager>
#include <QSplitter>
#include <QList>

#include <coreplugin/icore.h>

#include "titleedit.h"
#include "cellbutton.h"
#include "infobar.h"
#include "notifybar.h"
#include "wizEditorToolBar.h"
#include "wizDocumentView.h"
#include "wiztaglistwidget.h"
#include "wizattachmentlistwidget.h"
#include "wizDocumentWebView.h"
#include "wiznoteinfoform.h"
#include "share/wizmisc.h"
#include "share/wizDatabase.h"
#include "share/wizsettings.h"

#include "sync/token.h"
#include "sync/apientry.h"
#include "sync/asyncapi.h"
#include "messagecompleter.h"

using namespace Core;
using namespace Core::Internal;

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
    , m_editTitle(new TitleEdit(this))
    , m_infoBar(new InfoBar(this))
    , m_notifyBar(new NotifyBar(this))
    , m_editorBar(new EditorToolBar(this))
    , m_editor(NULL)
    , m_tags(NULL)
    , m_info(NULL)
    , m_attachments(NULL)
{
    m_editTitle->setCompleter(new WizService::MessageCompleter(m_editTitle));

    // FIXME
    QString strTheme = "default";

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    m_editBtn = new CellButton(CellButton::Left, this);
    QString shortcut = ::WizGetShortcut("EditNote", "Alt+1");
    m_editBtn->setShortcut(QKeySequence::fromString(shortcut));
    m_editBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_lock"), tr("Switch to Editing View"));
    m_editBtn->setCheckedIcon(::WizLoadSkinIcon(strTheme, "document_unlock"), tr("Switch to Reading View"));
    m_editBtn->setBadgeIcon(::WizLoadSkinIcon(strTheme, "document_unlock_modified"), tr("Save and switch to Reading View"));
    connect(m_editBtn, SIGNAL(clicked()), SLOT(onEditButtonClicked()));

    m_tagBtn = new CellButton(CellButton::Center, this);
    QString tagsShortcut = ::WizGetShortcut("EditNoteTags", "Alt+2");
    m_tagBtn->setShortcut(QKeySequence::fromString(tagsShortcut));
    m_tagBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_tag"), tr("View and add tags"));
    connect(m_tagBtn, SIGNAL(clicked()), SLOT(onTagButtonClicked()));

    m_attachBtn = new CellButton(CellButton::Center, this);
    QString attachmentShortcut = ::WizGetShortcut("EditNoteAttachments", "Alt+3");
    m_attachBtn->setShortcut(QKeySequence::fromString(attachmentShortcut));
    m_attachBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_attachment"), tr("Add attachments"));
    m_attachBtn->setBadgeIcon(::WizLoadSkinIcon(strTheme, "document_attachment_exist"), tr("View and add attachments"));
    connect(m_attachBtn, SIGNAL(clicked()), SLOT(onAttachButtonClicked()));

    m_infoBtn = new CellButton(CellButton::Center, this);
    QString infoShortcut = ::WizGetShortcut("EditNoteInfo", "Alt+4");
    m_infoBtn->setShortcut(QKeySequence::fromString(infoShortcut));
    m_infoBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_info"), tr("View and modify note's info"));
    connect(m_infoBtn, SIGNAL(clicked()), SLOT(onInfoButtonClicked()));

    // comments
    m_commentsBtn = new CellButton(CellButton::Right, this);
    m_commentsBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "comments"), tr("Add comments"));
    m_commentsBtn->setBadgeIcon(::WizLoadSkinIcon(strTheme, "comments_exist"), tr("View and add comments"));
    connect(m_commentsBtn, SIGNAL(clicked()), SLOT(onCommentsButtonClicked()));
    connect(ICore::instance(), SIGNAL(viewNoteLoaded(Core::INoteView*,const WIZDOCUMENTDATA&,bool)),
            SLOT(onViewNoteLoaded(Core::INoteView*,const WIZDOCUMENTDATA&,bool)));

    QWidget* line1 = new QWidget(this);
    line1->setFixedHeight(12);
    line1->setStyleSheet("border-top-width:1;border-top-style:solid;border-top-color:#d9dcdd");

    QWidget* line2 = new QWidget(this);
    line2->setFixedHeight(1);
    line2->setFixedWidth(10);
    line2->setStyleSheet("border-bottom-width:1;border-bottom-style:solid;border-bottom-color:#d9dcdd");

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
    layoutInfo2->addWidget(m_editBtn);
    layoutInfo2->addWidget(m_tagBtn);
    layoutInfo2->addWidget(m_attachBtn);
    layoutInfo2->addWidget(m_infoBtn);
    layoutInfo2->addWidget(m_commentsBtn);
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
    layout->addStretch();
}

CWizDocumentView* TitleBar::noteView()
{
    QWidget* pParent = parentWidget();
    while (pParent) {
        CWizDocumentView* view = dynamic_cast<CWizDocumentView*>(pParent);
        if (view)
            return view;

        pParent = pParent->parentWidget();
    }

    Q_ASSERT(0);
    return 0;
}

void TitleBar::setLocked(bool bReadOnly, int nReason, bool bIsGroup)
{
    m_notifyBar->showNotify(nReason);
    m_editTitle->setReadOnly(bReadOnly);
    m_editBtn->setEnabled(!bReadOnly);
    m_tagBtn->setEnabled(!bIsGroup ? true : false);
}

void TitleBar::setEditor(CWizDocumentWebView* editor)
{
    Q_ASSERT(!m_editor);

    m_editorBar->setDelegate(editor);

    connect(editor, SIGNAL(focusIn()), SLOT(onEditorFocusIn()));
    connect(editor, SIGNAL(focusOut()), SLOT(onEditorFocusOut()));

    //connect(editor->page(), SIGNAL(selectionChanged()), SLOT(onEditorChanged()));
    connect(editor->page(), SIGNAL(contentsChanged()), SLOT(onEditorChanged()));

    m_editor = editor;
}

void TitleBar::onEditorFocusIn()
{
    m_infoBar->hide();
    m_editorBar->show();
}

void TitleBar::onEditorFocusOut()
{
    m_editorBar->hide();
    m_infoBar->show();
}

void TitleBar::onEditorChanged()
{
    if (m_editor->isModified()) {
        m_editBtn->setState(CellButton::Badge);
    } else {
        m_editBtn->setState(noteView()->isEditing() ? CellButton::Checked : CellButton::Normal);
    }
}

void TitleBar::setNote(const WIZDOCUMENTDATA& data, bool editing, bool locked)
{
    updateInfo(data);
    setEditingDocument(editing);
}

void TitleBar::updateInfo(const WIZDOCUMENTDATA& doc)
{
    m_infoBar->setDocument(doc);
    m_editTitle->setText(doc.strTitle);
    m_attachBtn->setState(doc.nAttachmentCount > 0 ? CellButton::Badge : CellButton::Normal);
}

void TitleBar::setEditingDocument(bool editing)
{
    m_editTitle->setReadOnly(!editing);
    m_editBtn->setState(editing ? CellButton::Checked : CellButton::Normal);
}

void TitleBar::updateEditButton(bool editing)
{
    if (m_editor->isModified()) {
        m_editBtn->setState(CellButton::Badge);
    } else {
        m_editBtn->setState(editing ? CellButton::Checked : CellButton::Normal);
    }
}

void TitleBar::resetTitle(const QString& strTitle)
{
    m_editTitle->resetTitle(strTitle);
}

void TitleBar::onEditButtonClicked()
{
    noteView()->setEditNote(!m_editBtn->state());
}

void TitleBar::onTagButtonClicked()
{
    if (!m_tags) {
        m_tags = new CWizTagListWidget(topLevelWidget());
    }

    m_tags->setDocument(noteView()->note());

    QRect rc = m_tagBtn->rect();
    QPoint pt = m_tagBtn->mapToGlobal(QPoint(rc.width()/2, rc.height()));
    m_tags->showAtPoint(pt);
}

void TitleBar::onAttachButtonClicked()
{
    if (!m_attachments) {
        m_attachments = new CWizAttachmentListWidget(topLevelWidget());
    }

    m_attachments->setDocument(noteView()->note());

    QRect rc = m_attachBtn->rect();
    QPoint pt = m_attachBtn->mapToGlobal(QPoint(rc.width()/2, rc.height()));
    m_attachments->showAtPoint(pt);
}


void TitleBar::onInfoButtonClicked()
{
    if (!m_info) {
        m_info = new CWizNoteInfoForm(topLevelWidget());
    }

    m_info->setDocument(noteView()->note());

    QRect rc = m_infoBtn->rect();
    QPoint pt = m_infoBtn->mapToGlobal(QPoint(rc.width()/2, rc.height()));
    m_info->showAtPoint(pt);
}

bool isNetworkAccessible()
{
    return true;
    //QNetworkConfigurationManager man;
    //return man.isOnline();
}

#define COMMENT_FRAME_WIDTH 300

void TitleBar::onCommentsButtonClicked()
{
    QWebView* comments = noteView()->commentView();
    if (comments->isVisible()) {
        comments->hide();
        return;
    }

    if (isNetworkAccessible()) {
        comments->load(m_commentsUrl);
        QSplitter* splitter = qobject_cast<QSplitter*>(comments->parentWidget());
        Q_ASSERT(splitter);
        QList<int> li = splitter->sizes();
        Q_ASSERT(li.size() == 2);
        QList<int> lin;
        lin.push_back(li.value(0) - COMMENT_FRAME_WIDTH);
        lin.push_back(li.value(1) + COMMENT_FRAME_WIDTH);
        splitter->setSizes(lin);
        comments->show();
    } else {
        m_commentsBtn->setEnabled(false);
    }
}

void TitleBar::onViewNoteLoaded(INoteView* view, const WIZDOCUMENTDATA& note, bool bOk)
{
    Q_UNUSED(note);

    if (!bOk)
        return;

    if (view != noteView()) {
        return;
    }

    if (!isNetworkAccessible()) {
        noteView()->commentView()->hide();
        m_commentsBtn->setEnabled(false);
    } else {
        m_commentsUrl.clear();
        m_commentsBtn->setEnabled(true);

        connect(WizService::Token::instance(), SIGNAL(tokenAcquired(QString)),
                SLOT(onTokenAcquired(QString)), Qt::QueuedConnection);
        WizService::Token::requestToken();
    }
}

void TitleBar::onTokenAcquired(const QString& strToken)
{
    WizService::Token::instance()->disconnect(this);

    QWebView* comments = noteView()->commentView();

    if (strToken.isEmpty()) {
        comments->hide();
        return;
    }

    QString strKbGUID = noteView()->note().strKbGUID;
    QString strGUID = noteView()->note().strGUID;
    m_commentsUrl =  WizService::ApiEntry::commentUrl(strToken, strKbGUID, strGUID);

    if (comments->isVisible()) {
        comments->load(m_commentsUrl);
    }

    QUrl kUrl(WizService::ApiEntry::kUrlFromGuid(strToken, strKbGUID));
    QString strCountUrl = WizService::ApiEntry::commentCountUrl(kUrl.host(), strToken, strKbGUID, strGUID);

    WizService::AsyncApi* api = new WizService::AsyncApi(this);
    connect(api, SIGNAL(getCommentsCountFinished(int)), SLOT(onGetCommentsCountFinished(int)));
    api->getCommentsCount(strCountUrl);
}

void TitleBar::onGetCommentsCountFinished(int nCount)
{
    WizService::AsyncApi* api = dynamic_cast<WizService::AsyncApi*>(sender());
    api->disconnect(this);
    api->deleteLater();

    if (nCount) {
        m_commentsBtn->setState(CellButton::Badge);
    } else {
        m_commentsBtn->setState(CellButton::Normal);
    }
}
