#include "WizTitleBar.h"

#include <QVBoxLayout>
#include <QUrl>
#include <QMenu>
#include <QNetworkConfigurationManager>
#include <QMessageBox>
#include <QSplitter>
#include <QList>
#include <QLabel>
#include "share/jsoncpp/json/json.h"

#include "widgets/WizTagBar.h"
#include "WizTitleEdit.h"
#include "WizCellButton.h"
#include "WizInfoBar.h"
#include "WizNotifyBar.h"
#include "WizEditorToolBar.h"
#include "WizDocumentView.h"
#include "WizTagListWidget.h"
#include "WizAttachmentListWidget.h"
#include "WizDocumentWebEngine.h"
#include "WizDocumentWebView.h"
#include "WizNoteInfoForm.h"
#include "share/WizMisc.h"
#include "share/WizDatabase.h"
#include "share/WizSettings.h"
#include "share/WizAnimateAction.h"
#include "share/WizAnalyzer.h"
#include "share/WizGlobal.h"
#include "share/WizThreads.h"
#include "sync/WizApiEntry.h"
#include "utils/WizStyleHelper.h"
#include "utils/WizPathResolve.h"
#include "widgets/WizLocalProgressWebView.h"
#include "widgets/WizTipsWidget.h"

#include "WizMessageCompleter.h"
#include "WizOEMSettings.h"
#include "WizMainWindow.h"
#include "share/WizSettings.h"
#include "share/WizWebEngineView.h"

#include "core/WizCommentManager.h"

#define WIZACTION_TITLEBAR_SHARE_DOCUMENT_BY_LINK QObject::tr("Share by Link")
#define WIZACTION_TITLEBAR_SHARE_DOCUMENT_BY_EMAIL QObject::tr("Share by Email")

#define TITLE_BUTTON_ICON_SIZE       WizSmartScaleUI(14)
static const WizIconOptions ICON_OPTIONS(WIZ_TINT_COLOR, "#a6a6a6", WIZ_TINT_COLOR);
static const WizIconOptions CHECKABLE_ICON_OPTIONS(WIZ_TINT_COLOR, "#a6a6a6", WIZ_TINT_COLOR);


QString getOptionKey()
{
#ifdef Q_OS_MAC
    return "⌥";
#else
    return "Alt+";
#endif
}

WizTitleBar::WizTitleBar(WizExplorerApp& app, QWidget *parent)
    : QWidget(parent)
    , m_app(app)
    , m_editTitle(new WizTitleEdit(this))
    , m_tagBar(new WizTagBar(app, this))
    , m_infoBar(new WizInfoBar(app, this))
    , m_notifyBar(new WizNotifyBar(this))
    , m_editorBar(new WizEditorToolBar(app, this))
    , m_editor(NULL)
    , m_tags(NULL)
    , m_info(NULL)
    , m_attachments(NULL)
    , m_editButtonAnimation(0)
    , m_commentManager(new WizCommentManager(this))
{
    m_editTitle->setCompleter(new WizMessageCompleter(m_editTitle));
    int nTitleHeight = Utils::WizStyleHelper::titleEditorHeight();
    m_editTitle->setFixedHeight(nTitleHeight);
    m_editTitle->setAlignment(Qt::AlignVCenter);
    m_editTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    //
    m_editorBar->layout()->setAlignment(Qt::AlignVCenter);

    QString strTheme = Utils::WizStyleHelper::themeName();

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 6);
    layout->setSpacing(0);
    setLayout(layout);

    QSize iconSize = QSize(TITLE_BUTTON_ICON_SIZE, TITLE_BUTTON_ICON_SIZE);
    m_editBtn = new WizRoundCellButton(this);
    QString shortcut = ::WizGetShortcut("EditNote", "Alt+1");
    m_editBtn->setShortcut(QKeySequence::fromString(shortcut));
    m_editBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_lock", iconSize, ICON_OPTIONS), tr("Edit"), tr("Switch to Editing View  %1%2").arg(getOptionKey()).arg(1));
    m_editBtn->setCheckedIcon(::WizLoadSkinIcon(strTheme, "document_unlock", iconSize, ICON_OPTIONS), tr("Read") , tr("Switch to Reading View  %1%2").arg(getOptionKey()).arg(1));
    m_editBtn->setBadgeIcon(::WizLoadSkinIcon(strTheme, "document_unlock", iconSize, ICON_OPTIONS), tr("Save & Read"), tr("Save and switch to Reading View  %1%2").arg(getOptionKey()).arg(1));
    connect(m_editBtn, SIGNAL(clicked()), SLOT(onEditButtonClicked()));    

    m_mindmapBtn = new WizCellButton(WizCellButton::ImageOnly, this);
    m_mindmapBtn->setCheckable(true);
    m_mindmapBtn->setChecked(false);
    m_mindmapBtn->setFixedHeight(nTitleHeight);
    m_mindmapBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "outline_mindmap", iconSize, ICON_OPTIONS), tr("View mindmap"));
    connect(m_mindmapBtn, SIGNAL(clicked()), SLOT(onViewMindMapClicked()));

    m_separateBtn = new WizCellButton(WizCellButton::ImageOnly, this);
    m_separateBtn->setFixedHeight(nTitleHeight);
    QString separateShortcut = ::WizGetShortcut("EditNoteSeparate", "Alt+2");
    m_separateBtn->setShortcut(QKeySequence::fromString(separateShortcut));
    m_separateBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_use_separate", iconSize, ICON_OPTIONS), tr("View note in seperate window  %1%2").arg(getOptionKey()).arg(2));
    connect(m_separateBtn, SIGNAL(clicked()), SLOT(onSeparateButtonClicked()));

    m_tagBtn = new WizCellButton(WizCellButton::ImageOnly, this);
    m_tagBtn->setFixedHeight(nTitleHeight);
    QString tagsShortcut = ::WizGetShortcut("EditNoteTags", "Alt+3");
    m_tagBtn->setShortcut(QKeySequence::fromString(tagsShortcut));
    m_tagBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_tag", iconSize, ICON_OPTIONS), tr("View and add tags  %1%2").arg(getOptionKey()).arg(3));
    connect(m_tagBtn, SIGNAL(clicked()), SLOT(onTagButtonClicked()));

    m_shareBtn = new WizCellButton(WizCellButton::ImageOnly, this);
    m_shareBtn->setFixedHeight(nTitleHeight);
    QString shareShortcut = ::WizGetShortcut("EditShare", "Alt+4");
    m_shareBtn->setShortcut(QKeySequence::fromString(shareShortcut));
    m_shareBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_share", iconSize, ICON_OPTIONS), tr("Share note  %1%2").arg(getOptionKey()).arg(4));
    connect(m_shareBtn, SIGNAL(clicked()), SLOT(onShareButtonClicked()));
    WizOEMSettings oemSettings(m_app.databaseManager().db().getAccountPath());
    m_shareBtn->setVisible(!oemSettings.isHideShare());
    m_shareMenu = new QMenu(m_shareBtn);
    m_shareMenu->addAction(WIZACTION_TITLEBAR_SHARE_DOCUMENT_BY_LINK, this, SLOT(onShareActionClicked()));
    m_shareMenu->addAction(WIZACTION_TITLEBAR_SHARE_DOCUMENT_BY_EMAIL, this, SLOT(onEmailActionClicked()));
//    m_shareBtn->setMenu(shareMenu);

    //隐藏历史版本按钮，给以后增加提醒按钮保留位置
//    WizCellButton* historyBtn = new WizCellButton(WizCellButton::ImageOnly, this);
//    historyBtn->setFixedHeight(nTitleHeight);
//    QString historyShortcut = ::WizGetShortcut("EditNoteHistory", "Alt+5");
//    historyBtn->setShortcut(QKeySequence::fromString(historyShortcut));
//    historyBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_history"), tr("View and recover note's history (Alt + 5)"));
//    connect(historyBtn, SIGNAL(clicked()), SLOT(onHistoryButtonClicked()));

//    m_emailBtn = new CellButton(CellButton::ImageOnly, this);
//    m_emailBtn->setFixedHeight(nTitleHeight);
//    QString emailShortcut = ::WizGetShortcut("EditNoteEmail", "Alt+6");
//    m_emailBtn->setShortcut(QKeySequence::fromString(emailShortcut));
//    m_emailBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_email"), tr("Share document by email (Alt + 6)"));
//    connect(m_emailBtn, SIGNAL(clicked()), SLOT(onEmailButtonClicked()));
//    m_emailBtn->setVisible(!oemSettings.isHideShareByEmail());

    m_infoBtn = new WizCellButton(WizCellButton::ImageOnly, this);
    m_infoBtn->setFixedHeight(nTitleHeight);
    QString infoShortcut = ::WizGetShortcut("EditNoteInfo", "Alt+5");
    m_infoBtn->setShortcut(QKeySequence::fromString(infoShortcut));
    m_infoBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_info", iconSize, ICON_OPTIONS), tr("View and modify note's info  %1%2").arg(getOptionKey()).arg(5));
    connect(m_infoBtn, SIGNAL(clicked()), SLOT(onInfoButtonClicked()));

    m_attachBtn = new WizCellButton(WizCellButton::WithCountInfo, this);
    m_attachBtn->setFixedHeight(nTitleHeight);
    QString attachmentShortcut = ::WizGetShortcut("EditNoteAttachments", "Alt+6");
    m_attachBtn->setShortcut(QKeySequence::fromString(attachmentShortcut));
    m_attachBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_attachment", iconSize, ICON_OPTIONS), tr("Add attachments  %1%2").arg(getOptionKey()).arg(6));
    connect(m_attachBtn, SIGNAL(clicked()), SLOT(onAttachButtonClicked()));

    // comments
    m_commentsBtn = new WizCellButton(WizCellButton::WithCountInfo, this);
    m_commentsBtn->setFixedHeight(nTitleHeight);
    QString commentShortcut = ::WizGetShortcut("ShowComment", "Alt+c");
    m_commentsBtn->setShortcut(QKeySequence::fromString(commentShortcut));
    m_commentsBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "comments", iconSize, CHECKABLE_ICON_OPTIONS), tr("Add comments  %1C").arg(getOptionKey()));
    connect(m_commentsBtn, SIGNAL(clicked()), SLOT(onCommentsButtonClicked()));
    connect(WizGlobal::instance(), SIGNAL(viewNoteLoaded(WizDocumentView*,const WIZDOCUMENTDATAEX&,bool)),
            SLOT(onViewNoteLoaded(WizDocumentView*,const WIZDOCUMENTDATAEX&,bool)));


    QHBoxLayout* layoutInfo2 = new QHBoxLayout();
    layoutInfo2->setContentsMargins(0, 0, 0, 0);
    layoutInfo2->setSpacing(0);
    layoutInfo2->addWidget(m_editTitle);
    layoutInfo2->addWidget(m_editBtn);
    layoutInfo2->addWidget(m_mindmapBtn);
    layoutInfo2->addSpacing(::WizSmartScaleUI(7));
    layoutInfo2->addWidget(m_separateBtn);
    layoutInfo2->addWidget(m_tagBtn);
    layoutInfo2->addWidget(m_shareBtn);
    //隐藏历史版本按钮，给以后增加提醒按钮保留位置
    //layoutInfo2->addWidget(historyBtn);
//    layoutInfo2->addWidget(m_emailBtn);
    layoutInfo2->addWidget(m_infoBtn);
    layoutInfo2->addWidget(m_attachBtn);
    layoutInfo2->addWidget(m_commentsBtn);
    //
    initPlugins(layoutInfo2);

    QVBoxLayout* layoutInfo1 = new QVBoxLayout();
    layoutInfo1->setContentsMargins(Utils::WizStyleHelper::editorBarMargins());
    layoutInfo1->setSpacing(0);
    layoutInfo1->addLayout(layoutInfo2);
    layoutInfo1->addWidget(m_tagBar);
    layoutInfo1->addWidget(m_infoBar);
    layoutInfo1->addWidget(m_editorBar);
    layoutInfo1->addWidget(m_notifyBar);
    m_editorBar->hide();

    layout->addLayout(layoutInfo1);
    //layout->addLayout(layoutInfo4);

    layout->addStretch();
    connect(m_notifyBar, SIGNAL(labelLink_clicked(QString)), SIGNAL(notifyBar_link_clicked(QString)));
    connect(m_tagBar, SIGNAL(widgetStatusChanged()), SLOT(updateTagButtonStatus()));

    connect(m_commentManager, SIGNAL(tokenAcquired(QString)),
            SLOT(on_commentTokenAcquired(QString)));
    connect(m_commentManager, SIGNAL(commentCountAcquired(QString,int)),
            SLOT(on_commentCountAcquired(QString,int)));
}

void WizTitleBar::initPlugins(QLayout* layout)
{
    int nTitleHeight = Utils::WizStyleHelper::titleEditorHeight();
    m_plugins = WizPlugins::plugins().pluginsByType("document");
    for (auto data : m_plugins) {
        //
        WizCellButton* button = new WizCellButton(WizCellButton::ImageOnly, this);
        button->setUserObject(data);
        button->setFixedHeight(nTitleHeight);
        button->setNormalIcon(data->icon(), data->name());
        connect(button, SIGNAL(clicked()), SLOT(onPluginButtonClicked()));
        layout->addWidget(button);
        //
    }
}

void WizTitleBar::onPluginButtonClicked()
{
    WizCellButton* button = dynamic_cast<WizCellButton *>(sender());
    if (!button) {
        return;
    }
    //
    WizPluginData* data = dynamic_cast<WizPluginData *>(button->userObject());
    if (!data) {
        return;
    }
    //
    QString guid = data->guid();
    auto it = m_pluginWidget.find(guid);
    WizPluginPopupWidget* widget;
    if (it == m_pluginWidget.end()) {
        widget = new WizPluginPopupWidget(m_app, data, this);
        m_pluginWidget.insert(std::make_pair(guid, widget));
    } else {
        widget = it->second;
    }
    //
    QPoint pt = mapToGlobal(button->geometry().center());
    pt.setY(pt.y() + button->rect().height() / 2);
    data->emitShowEvent();
    if (isDarkMode()) {
        widget->web()->setVisible(false);
        QTimer::singleShot(500, [=] {
            widget->web()->setVisible(true);
        });
    }
    widget->showAtPoint(pt);
}

WizDocumentView* WizTitleBar::noteView()
{
    QWidget* pParent = parentWidget();
    while (pParent) {
        WizDocumentView* view = dynamic_cast<WizDocumentView*>(pParent);
        if (view)
            return view;

        pParent = pParent->parentWidget();
    }

    Q_ASSERT(0);
    return 0;
}

WizEditorToolBar*WizTitleBar::editorToolBar()
{
    return m_editorBar;
}

void WizTitleBar::setLocked(bool bReadOnly, int nReason, bool bIsGroup)
{
    m_notifyBar->showPermissionNotify(nReason);
    m_editTitle->setReadOnly(bReadOnly);
    m_editBtn->setEnabled(!bReadOnly);

    if (nReason == WizNotifyBar::Deleted)
    {
        m_tagBtn->setEnabled(false);
        m_commentsBtn->setEnabled(false);
        foreach (QAction* action , m_shareMenu->actions())
        {
            action->setEnabled(false);
        }
    }
    else
    {
        WizOEMSettings oemSettings(m_app.databaseManager().db().getAccountPath());
        m_tagBtn->setVisible(bIsGroup ? false : true);
        m_tagBtn->setEnabled(bIsGroup ? false : true);
        m_commentsBtn->setEnabled(true);
        foreach (QAction* action , m_shareMenu->actions())
        {
            if (action->text() == WIZACTION_TITLEBAR_SHARE_DOCUMENT_BY_EMAIL)
            {
                action->setEnabled(!oemSettings.isHideShareByEmail());
                action->setVisible(!oemSettings.isHideShareByEmail());
            }
            else if (action->text() == WIZACTION_TITLEBAR_SHARE_DOCUMENT_BY_LINK)
            {
                action->setEnabled(!oemSettings.isHideShare());
                action->setVisible(!oemSettings.isHideShare());
            }
        }
    }
}

void WizTitleBar::setEditor(WizDocumentWebView* editor)
{
    Q_ASSERT(!m_editor);

    m_editorBar->setDelegate(editor);

    connect(editor, SIGNAL(focusIn()), SLOT(onEditorFocusIn()));
    connect(editor, SIGNAL(focusOut()), SLOT(onEditorFocusOut()));

    connect(m_editTitle, SIGNAL(titleEdited(QString)), editor, SLOT(onTitleEdited(QString)));

    m_editor = editor;
}

void WizTitleBar::onEditorFocusIn()
{
    showEditorBar();
}

void WizTitleBar::onEditorFocusOut()
{
    showEditorBar();
    if (!m_editorBar->hasFocus())
        showInfoBar();
}

void WizTitleBar::updateTagButtonStatus()
{
    if (m_tagBar && m_tagBtn)
    {
        m_tagBtn->setState(m_tagBar->isVisible() ? WizCellButton::Checked : WizCellButton::Normal);
    }
}

void WizTitleBar::updateAttachButtonStatus()
{
    if (m_attachments && m_attachBtn)
    {
        m_attachBtn->setState(m_attachments->isVisible() ? WizCellButton::Checked : WizCellButton::Normal);
    }
}

void WizTitleBar::updateInfoButtonStatus()
{
    if (m_info && m_infoBtn)
    {
        m_infoBtn->setState(m_info->isVisible() ? WizCellButton::Checked : WizCellButton::Normal);
    }
}

void WizTitleBar::updateCommentsButtonStatus()
{
    if (m_commentsBtn && noteView()->commentWidget())
    {
        m_commentsBtn->setState(noteView()->commentWidget()->isVisible() ? WizCellButton::Checked : WizCellButton::Normal);
    }
}

void WizTitleBar::onTitleEditFinished()
{
    m_editTitle->onTitleEditingFinished();
}

void WizTitleBar::showInfoBar()
{
    m_editorBar->hide();
    m_infoBar->show();
}

void WizTitleBar::showEditorBar()
{
    m_infoBar->hide();
    m_editorBar->show();
    m_editorBar->adjustButtonPosition();
}

void WizTitleBar::loadErrorPage()
{
    QWebEngineView* comments = noteView()->commentView();
    QString strFileName = Utils::WizPathResolve::resourcesPath() + "files/errorpage/load_fail_comments.html";
    QString strHtml;
    ::WizLoadUnicodeTextFromFile(strFileName, strHtml);
    // clear old url
    comments->load(QUrl());
    QUrl url = QUrl::fromLocalFile(strFileName);
    comments->load(url);
}

void WizTitleBar::setTagBarVisible(bool visible)
{
    m_tagBar->setVisible(visible);
}


void WizTitleBar::onEditorChanged()
{
    updateEditButton(noteView()->editorMode());
}

void WizTitleBar::setNote(const WIZDOCUMENTDATA& data, WizEditorMode editorMode, bool locked)
{
    updateInfo(data);
    setEditorMode(editorMode);
    //
    WizDatabase& db = m_app.databaseManager().db(data.strKbGUID);
    bool isGroup = db.isGroup();
    int nTagCount = db.getDocumentTagCount(data.strGUID);
    setTagBarVisible(!isGroup && nTagCount > 0);
    if (!isGroup)
    {
        m_tagBar->setDocument(data);
    }
    //
    m_mindmapBtn->setVisible(data.strType == "outline");
    m_mindmapBtn->setChecked(false);
}

void WizTitleBar::updateInfo(const WIZDOCUMENTDATA& doc)
{
    m_infoBar->setDocument(doc);
    m_editTitle->setText(doc.strTitle);
    m_attachBtn->setCount(doc.nAttachmentCount);
}

void WizTitleBar::setEditorMode(WizEditorMode editorMode)
{
    m_editTitle->setReadOnly(editorMode == modeReader);
    m_editBtn->setState(editorMode == modeEditor ? WizCellButton::Checked : WizCellButton::Normal);
    //
    if (editorMode == modeReader)
    {
        showInfoBar();
        m_editorBar->switchToNormalMode();
    }
    else
    {
        m_editorBar->switchToNormalMode();
        showEditorBar();
    }
}

void WizTitleBar::setEditButtonEnabled(bool enable)
{
    m_editBtn->setEnabled(enable);
}

void WizTitleBar::updateEditButton(WizEditorMode editorMode)
{
    m_editor->isModified([=](bool modified) {
        if (modified){
            m_editBtn->setState(WizCellButton::Badge);
        } else {
            m_editBtn->setState(editorMode == modeEditor ? WizCellButton::Checked : WizCellButton::Normal);
        }
    });
}

void WizTitleBar::resetTitle(const QString& strTitle)
{
    m_editTitle->resetTitle(strTitle);

}

void WizTitleBar::clearAndSetPlaceHolderText(const QString& text)
{
    m_editTitle->setPlaceholderText(text);
    m_editTitle->clear();
}

void WizTitleBar::clearPlaceHolderText()
{
    m_editTitle->setPlaceholderText("");
}

#define TITLEBARTIPSCHECKED        "TitleBarTipsChecked"

void WizTitleBar::showCoachingTips()
{
    bool showTips = false;
    if (WizMainWindow* mainWindow = WizMainWindow::instance())
    {
        showTips = mainWindow->userSettings().get(TITLEBARTIPSCHECKED).toInt() == 0;
    }

    if (showTips && !WizTipsWidget::isTipsExists(TITLEBARTIPSCHECKED))
    {
        WizTipsWidget* widget = new WizTipsWidget(TITLEBARTIPSCHECKED, this);
        connect(m_editBtn, SIGNAL(clicked(bool)), widget, SLOT(on_targetWidgetClicked()));
        widget->setAttribute(Qt::WA_DeleteOnClose, true);
        widget->setText(tr("Switch to reading mode"), tr("In reading mode, the note can not be "
                                                         "edited and markdown note can be redered."));
        widget->setSizeHint(QSize(290, 82));
        widget->setButtonVisible(false);       
        widget->bindCloseFunction([](){
            if (WizMainWindow* mainWindow = WizMainWindow::instance())
            {
                mainWindow->userSettings().set(TITLEBARTIPSCHECKED, "1");
            }
        });
        //
        widget->bindTargetWidget(m_editBtn, 0, -2);
        QTimer::singleShot(500, widget, SLOT(on_showRequest()));
    }
}

void WizTitleBar::startEditButtonAnimation()
{
    if (!m_editButtonAnimation)
    {
        m_editButtonAnimation = new WizAnimateAction(this);
        m_editButtonAnimation->setToolButton(m_editBtn);
        m_editButtonAnimation->setSingleIcons("editButtonProcessing");
    }
    m_editButtonAnimation->startPlay();
}

void WizTitleBar::stopEditButtonAnimation()
{
    if (!m_editButtonAnimation)
        return;
    if (m_editButtonAnimation->isPlaying())
    {
        m_editButtonAnimation->stopPlay();
    }
}

void WizTitleBar::applyButtonStateForSeparateWindow(bool inSeparateWindow)
{
    m_separateBtn->setVisible(!inSeparateWindow);
}

void WizTitleBar::onEditButtonClicked()
{
    noteView()->setEditorMode(noteView()->editorMode() == modeEditor ? modeReader: modeEditor);
    //
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    if (noteView()->isEditing())
    {
        analyzer.logAction("editNote");
    }
    else
    {
        analyzer.logAction("viewNote");
    }
}

void WizTitleBar::onSeparateButtonClicked()
{
    WizGetAnalyzer().logAction("titleBarViewInSeperateWindow");

    emit viewNoteInSeparateWindow_request();
}

void WizTitleBar::onTagButtonClicked()
{
//    if (!m_tags) {
//        m_tags = new CWizTagListWidget(topLevelWidget());
//    }

//    m_tags->setDocument(noteView()->note());

//    QRect rc = m_tagBtn->rect();
//    QPoint pt = m_tagBtn->mapToGlobal(QPoint(rc.width()/2, rc.height()));
//    m_tags->showAtPoint(pt);

    setTagBarVisible(!m_tagBar->isVisible());

    WizGetAnalyzer().logAction("showTags");
}

void WizTitleBar::onViewMindMapClicked()
{
    bool on = m_mindmapBtn->isChecked();
    //m_mindmapBtn->setChecked(on);
    emit onViewMindMap(on);
    m_mindmapBtn->setState(on ? WizCellButton::Checked : WizCellButton::Normal);
}

QAction* actionFromMenu(QMenu* menu, const QString& text)
{
    QList<QAction*> actionList = menu->actions();
    for (QAction* action : actionList)
    {
        if (action->text() == text)
            return action;
    }
    return nullptr;
}

void WizTitleBar::onShareButtonClicked()
{
    if (m_shareMenu)
    {
        const WIZDOCUMENTDATA& doc = noteView()->note();

        QAction* actionLink = actionFromMenu(m_shareMenu, WIZACTION_TITLEBAR_SHARE_DOCUMENT_BY_LINK);
        if (actionLink)
        {
            actionLink->setVisible(true);
            if (m_app.databaseManager().db(doc.strKbGUID).isGroup())
            {
                WIZGROUPDATA group;
                m_app.databaseManager().db().getGroupData(doc.strKbGUID, group);
                if (!group.isBiz())
                {
                    actionLink->setVisible(false);
                }
            }
        }

        QPoint pos = m_shareBtn->mapToGlobal(m_shareBtn->rect().bottomLeft());
        m_shareMenu->popup(pos);
    }
}

void WizTitleBar::onEmailActionClicked()
{
    WizGetAnalyzer().logAction("shareByEmail");

    m_editor->shareNoteByEmail();
}

void WizTitleBar::onShareActionClicked()
{
    WizGetAnalyzer().logAction("shareByLink");

    m_editor->shareNoteByLink();
}

void WizTitleBar::onAttachButtonClicked()
{
    if (!m_attachments) {
        m_attachments = new WizAttachmentListWidget(topLevelWidget());
        connect(m_attachments, SIGNAL(widgetStatusChanged()), SLOT(updateAttachButtonStatus()));
    }


    if (m_attachments->setDocument(noteView()->note()))
    {
        QRect rc = m_attachBtn->rect();
        QPoint pt = m_attachBtn->mapToGlobal(QPoint(rc.width()/2, rc.height()));
        m_attachments->showAtPoint(pt);
        updateAttachButtonStatus();
    }

    WizGetAnalyzer().logAction("showAttachments");
}

void WizTitleBar::onHistoryButtonClicked()
{
    const WIZDOCUMENTDATA& doc = noteView()->note();

    WizShowDocumentHistory(doc, noteView());

    WizGetAnalyzer().logAction("showHistory");
}


void WizTitleBar::onInfoButtonClicked()
{
    if (!m_info) {
        m_info = new WizNoteInfoForm(topLevelWidget());
        connect(m_info, SIGNAL(widgetStatusChanged()), SLOT(updateInfoButtonStatus()));
    }

    m_info->setDocument(noteView()->note());
    //
    noteView()->wordCount([=](const QString& json){
        //
        Json::Value d;
        Json::Reader reader;
        if (!reader.parse(json.toUtf8().constData(), d))
            return;

        try {
            int nWords = d["nWords"].asInt();
            int nChars = d["nChars"].asInt();
            int nCharsWithSpace = d["nCharsWithSpace"].asInt();
            int nNonAsianWords = d["nNonAsianWords"].asInt();
            int nAsianChars = d["nAsianChars"].asInt();
            //
            m_info->setWordCount(nWords, nChars, nCharsWithSpace, nNonAsianWords, nAsianChars);


        } catch (...) {

        }
    });

    QRect rc = m_infoBtn->rect();
    QPoint pt = m_infoBtn->mapToGlobal(QPoint(rc.width()/2, rc.height()));
    m_info->showAtPoint(pt);
    updateInfoButtonStatus();

    WizGetAnalyzer().logAction("showNoteInfo");
}

bool isNetworkAccessible()
{
    return true;
    //QNetworkConfigurationManager man;
    //return man.isOnline();
}

#define COMMENT_FRAME_WIDTH 315

void WizTitleBar::onCommentsButtonClicked()
{
    QWebEngineView* comments = noteView()->commentView();

    WizDocumentView* view = noteView();
    if (!view)
        return;

    WizLocalProgressWebView* commentWidget = view->commentWidget();
    connect(commentWidget, SIGNAL(widgetStatusChanged()), this,
            SLOT(updateCommentsButtonStatus()), Qt::UniqueConnection);


    if (commentWidget->isVisible()) {
        commentWidget->hide();

        WizGetAnalyzer().logAction("hideComments");

        return;
    }

    WizGetAnalyzer().logAction("showComments");

    if (isNetworkAccessible()) {
        QSplitter* splitter = qobject_cast<QSplitter*>(commentWidget->parentWidget());
        Q_ASSERT(splitter);
        QList<int> li = splitter->sizes();
        Q_ASSERT(li.size() == 2);
        QList<int> lin;
        lin.push_back(li.value(0) - COMMENT_FRAME_WIDTH);
        lin.push_back(li.value(1) + COMMENT_FRAME_WIDTH);
        splitter->setSizes(lin);
        commentWidget->show();

        m_commentManager->queryCommentCount(view->note().strKbGUID, view->note().strGUID, true);
    } else {
        m_commentsBtn->setEnabled(false);
    }
}

void WizTitleBar::onCommentPageLoaded(bool ok)
{
    WizLocalProgressWebView* commentWidget = noteView()->commentWidget();

    if (!ok)
    {
        qDebug() << "Wow, load comment page failed! " << commentWidget->web()->url();
        //失败的时候会造成死循环
        //loadErrorPage();
        commentWidget->show();
    }
}

void WizTitleBar::onViewNoteLoaded(WizDocumentView* view, const WIZDOCUMENTDATAEX& note, bool bOk)
{
    if (!bOk)
        return;    

    if (view != noteView()) {
        return;
    }    

    m_commentsBtn->setCount(0);
    m_commentManager->queryCommentCount(note.strKbGUID, note.strGUID, true);
}

void WizTitleBar::on_commentTokenAcquired(QString token)
{
    WizDocumentView* view = noteView();
    if (view)
    {
        WizLocalProgressWebView* commentWidget = view->commentWidget();
        if (commentWidget && commentWidget->isVisible())
        {
            if (token.isEmpty())
            {
                qDebug() << "Wow, query token= failed!";
                loadErrorPage();
            }
            else
            {
                WIZDOCUMENTDATA note = view->note();

                QString js = QString("updateCmt('%1','%2','%3')").arg(token).arg(note.strKbGUID).arg(note.strGUID);
#ifdef QT_DEBUG
                qDebug() << js;
#endif
                commentWidget->web()->page()->runJavaScript(js, [=](const QVariant& vRet){
                    if (!vRet.toBool())
                    {
                        QString commentUrlTemplate = m_commentManager->getCommentUrlTemplate();
                        if (!commentUrlTemplate.isEmpty())
                        {
                            QString strUrl = commentUrlTemplate;
                            strUrl.replace("{token}", token);
                            strUrl.replace("{kbGuid}", note.strKbGUID);
                            strUrl.replace("{documentGuid}", note.strGUID);
                            commentWidget->web()->load(strUrl);
                        }
                    }
                });
            }
        }
    }
}

void WizTitleBar::on_commentCountAcquired(QString GUID, int count)
{
    WizDocumentView* view = noteView();
    if (view && view->note().strGUID == GUID)
    {
        m_commentsBtn->setCount(count);
    }
}

void WizTitleBar::showMessageTips(Qt::TextFormat format, const QString& strInfo)
{
    m_notifyBar->showMessageTips(format, strInfo);
}

void WizTitleBar::hideMessageTips(bool useAnimation)
{
    m_notifyBar->hideMessageTips(useAnimation);
}
