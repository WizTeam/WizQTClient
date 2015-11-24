#include "titlebar.h"

#include <QVBoxLayout>
#include <QUrl>
#include <QMenu>
#include <QNetworkConfigurationManager>
#include <QMessageBox>
#include <QWebHistory>
#include <QSplitter>
#include <QList>
#include <QLabel>
#if QT_VERSION > 0x050000
#include <QtConcurrent>
#else
#include <QtConcurrentRun>
#endif

//#include "share/websocketclientwrapper.h"
//#include "share/websockettransport.h"
//#include "wizWebEngineInjectObject.h"

#include <coreplugin/icore.h>

#include "widgets/wizTagBar.h"
#include "titleedit.h"
#include "cellbutton.h"
#include "infobar.h"
#include "notifybar.h"
#include "wizEditorToolBar.h"
#include "wizDocumentView.h"
#include "wiztaglistwidget.h"
#include "wizattachmentlistwidget.h"
#include "wizDocumentWebEngine.h"
#include "wizDocumentWebView.h"
#include "wiznoteinfoform.h"
#include "share/wizmisc.h"
#include "share/wizDatabase.h"
#include "share/wizsettings.h"
#include "share/wizanimateaction.h"
#include "share/wizAnalyzer.h"
#include "utils/stylehelper.h"
#include "utils/pathresolve.h"
#include "widgets/wizLocalProgressWebView.h"
#include "widgets/wizTipsWidget.h"

#include "messagecompleter.h"
#include "wizOEMSettings.h"
#include "wizmainwindow.h"
#include "share/wizsettings.h"

#include "core/wizCommentManager.h"

using namespace Core;
using namespace Core::Internal;

#define WIZACTION_TITLEBAR_SHARE_DOCUMENT_BY_LINK QObject::tr("Share by Link")
#define WIZACTION_TITLEBAR_SHARE_DOCUMENT_BY_EMAIL QObject::tr("Share by Email")

QString getOptionKey()
{
#ifdef Q_OS_MAC
    return "⌥";
#else
    return "Alt+";
#endif
}

TitleBar::TitleBar(CWizExplorerApp& app, QWidget *parent)
    : QWidget(parent)
    , m_app(app)
    , m_editTitle(new TitleEdit(this))
    , m_tagBar(new CWizTagBar(app, this))
    , m_infoBar(new InfoBar(app, this))
    , m_notifyBar(new NotifyBar(this))
    , m_editorBar(new EditorToolBar(app, this))
    , m_editor(NULL)
    , m_tags(NULL)
    , m_info(NULL)
    , m_attachments(NULL)
    , m_editButtonAnimation(0)
    , m_commentManager(new CWizCommentManager(this))
{
    m_editTitle->setCompleter(new WizService::MessageCompleter(m_editTitle));
    int nTitleHeight = Utils::StyleHelper::titleEditorHeight();
    m_editTitle->setFixedHeight(nTitleHeight);
    m_editTitle->setAlignment(Qt::AlignVCenter);
    m_editTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

//    m_editorBar->setFixedHeight(nEditToolBarHeight);
    m_editorBar->layout()->setAlignment(Qt::AlignVCenter);

    QString strTheme = Utils::StyleHelper::themeName();

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 6);
    layout->setSpacing(0);
    setLayout(layout);

    m_editBtn = new RoundCellButton(this);
    QString shortcut = ::WizGetShortcut("EditNote", "Alt+1");
    m_editBtn->setShortcut(QKeySequence::fromString(shortcut));
    m_editBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_lock"), tr("Edit"), tr("Switch to Editing View  %1%2").arg(getOptionKey()).arg(1));
    m_editBtn->setCheckedIcon(::WizLoadSkinIcon(strTheme, "document_unlock"), tr("Read") , tr("Switch to Reading View  %1%2").arg(getOptionKey()).arg(1));
    m_editBtn->setBadgeIcon(::WizLoadSkinIcon(strTheme, "document_unlock"), tr("Save & Read"), tr("Save and switch to Reading View  %1%2").arg(getOptionKey()).arg(1));
    connect(m_editBtn, SIGNAL(clicked()), SLOT(onEditButtonClicked()));    

    m_separateBtn = new CellButton(CellButton::ImageOnly, this);
    m_separateBtn->setFixedHeight(nTitleHeight);
    QString separateShortcut = ::WizGetShortcut("EditNoteSeparate", "Alt+2");
    m_separateBtn->setShortcut(QKeySequence::fromString(separateShortcut));
    m_separateBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_use_separate"), tr("View note in seperate window  %1%2").arg(getOptionKey()).arg(2));
    connect(m_separateBtn, SIGNAL(clicked()), SLOT(onSeparateButtonClicked()));

    m_tagBtn = new CellButton(CellButton::ImageOnly, this);
    m_tagBtn->setFixedHeight(nTitleHeight);
    QString tagsShortcut = ::WizGetShortcut("EditNoteTags", "Alt+3");
    m_tagBtn->setShortcut(QKeySequence::fromString(tagsShortcut));
    m_tagBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_tag"), tr("View and add tags  %1%2").arg(getOptionKey()).arg(3));
    m_tagBtn->setCheckedIcon(::WizLoadSkinIcon(strTheme, "document_tag_on"), tr("View and add tags  %1%2").arg(getOptionKey()).arg(3));
    connect(m_tagBtn, SIGNAL(clicked()), SLOT(onTagButtonClicked()));

    m_shareBtn = new CellButton(CellButton::ImageOnly, this);
    m_shareBtn->setFixedHeight(nTitleHeight);
    QString shareShortcut = ::WizGetShortcut("EditShare", "Alt+4");
    m_shareBtn->setShortcut(QKeySequence::fromString(shareShortcut));
    m_shareBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_share"), tr("Share note  %1%2").arg(getOptionKey()).arg(4));
    connect(m_shareBtn, SIGNAL(clicked()), SLOT(onShareButtonClicked()));
    CWizOEMSettings oemSettings(m_app.databaseManager().db().GetAccountPath());
    m_shareBtn->setVisible(!oemSettings.isHideShare());
    m_shareMenu = new QMenu(m_shareBtn);
    m_shareMenu->addAction(WIZACTION_TITLEBAR_SHARE_DOCUMENT_BY_LINK, this, SLOT(onShareActionClicked()));
    m_shareMenu->addAction(WIZACTION_TITLEBAR_SHARE_DOCUMENT_BY_EMAIL, this, SLOT(onEmailActionClicked()));
//    m_shareBtn->setMenu(shareMenu);

//    m_historyBtn = new CellButton(CellButton::ImageOnly, this);
//    m_historyBtn->setFixedHeight(nTitleHeight);
//    QString historyShortcut = ::WizGetShortcut("EditNoteHistory", "Alt+5");
//    m_historyBtn->setShortcut(QKeySequence::fromString(historyShortcut));
//    m_historyBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_history"), tr("View and recover note's history (Alt + 5)"));
//    connect(m_historyBtn, SIGNAL(clicked()), SLOT(onHistoryButtonClicked()));

//    m_emailBtn = new CellButton(CellButton::ImageOnly, this);
//    m_emailBtn->setFixedHeight(nTitleHeight);
//    QString emailShortcut = ::WizGetShortcut("EditNoteEmail", "Alt+6");
//    m_emailBtn->setShortcut(QKeySequence::fromString(emailShortcut));
//    m_emailBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_email"), tr("Share document by email (Alt + 6)"));
//    connect(m_emailBtn, SIGNAL(clicked()), SLOT(onEmailButtonClicked()));
//    m_emailBtn->setVisible(!oemSettings.isHideShareByEmail());

    m_infoBtn = new CellButton(CellButton::ImageOnly, this);
    m_infoBtn->setFixedHeight(nTitleHeight);
    QString infoShortcut = ::WizGetShortcut("EditNoteInfo", "Alt+5");
    m_infoBtn->setShortcut(QKeySequence::fromString(infoShortcut));
    m_infoBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_info"), tr("View and modify note's info  %1%2").arg(getOptionKey()).arg(5));
    m_infoBtn->setCheckedIcon(::WizLoadSkinIcon(strTheme, "document_info_on"), tr("View and modify note's info  %1%2").arg(getOptionKey()).arg(5));
    connect(m_infoBtn, SIGNAL(clicked()), SLOT(onInfoButtonClicked()));

    m_attachBtn = new CellButton(CellButton::WithCountInfo, this);
    m_attachBtn->setFixedHeight(nTitleHeight);
    QString attachmentShortcut = ::WizGetShortcut("EditNoteAttachments", "Alt+6");
    m_attachBtn->setShortcut(QKeySequence::fromString(attachmentShortcut));
    m_attachBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_attachment"), tr("Add attachments  %1%2").arg(getOptionKey()).arg(6));
    m_attachBtn->setCheckedIcon(::WizLoadSkinIcon(strTheme, "document_attachment_on"), tr("Add attachments  %1%2").arg(getOptionKey()).arg(6));
    connect(m_attachBtn, SIGNAL(clicked()), SLOT(onAttachButtonClicked()));

    // comments
    m_commentsBtn = new CellButton(CellButton::WithCountInfo, this);
    m_commentsBtn->setFixedHeight(nTitleHeight);
    QString commentShortcut = ::WizGetShortcut("ShowComment", "Alt+c");
    m_commentsBtn->setShortcut(QKeySequence::fromString(commentShortcut));
    m_commentsBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "comments"), tr("Add comments  %1C").arg(getOptionKey()));
    m_commentsBtn->setCheckedIcon(::WizLoadSkinIcon(strTheme, "comments_on"), tr("Add comments  %1C").arg(getOptionKey()));
    connect(m_commentsBtn, SIGNAL(clicked()), SLOT(onCommentsButtonClicked()));
    connect(ICore::instance(), SIGNAL(viewNoteLoaded(Core::INoteView*,const WIZDOCUMENTDATA&,bool)),
            SLOT(onViewNoteLoaded(Core::INoteView*,const WIZDOCUMENTDATA&,bool)));


    QHBoxLayout* layoutInfo2 = new QHBoxLayout();
    layoutInfo2->setContentsMargins(0, 0, 0, 0);
    layoutInfo2->setSpacing(0);
    layoutInfo2->addWidget(m_editTitle);
    layoutInfo2->addWidget(m_editBtn);
    layoutInfo2->addSpacing(7);
    layoutInfo2->addWidget(m_separateBtn);
    layoutInfo2->addWidget(m_tagBtn);
    layoutInfo2->addWidget(m_shareBtn);
//    layoutInfo2->addWidget(m_historyBtn);
//    layoutInfo2->addWidget(m_emailBtn);
    layoutInfo2->addWidget(m_infoBtn);
    layoutInfo2->addWidget(m_attachBtn);
    layoutInfo2->addWidget(m_commentsBtn);    

    QVBoxLayout* layoutInfo1 = new QVBoxLayout();
    layoutInfo1->setContentsMargins(Utils::StyleHelper::editorBarMargins());
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

    connect(m_commentManager, SIGNAL(commentUrlAcquired(QString,QString)),
            SLOT(on_commentUrlAcquired(QString,QString)));
    connect(m_commentManager, SIGNAL(commentCountAcquired(QString,int)),
            SLOT(on_commentCountAcquired(QString,int)));
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

EditorToolBar*TitleBar::editorToolBar()
{
    return m_editorBar;
}

void TitleBar::setLocked(bool bReadOnly, int nReason, bool bIsGroup)
{
    m_notifyBar->showPermissionNotify(nReason);
    m_editTitle->setReadOnly(bReadOnly);
    m_editBtn->setEnabled(!bReadOnly);

    if (nReason == NotifyBar::Deleted)
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
        CWizOEMSettings oemSettings(m_app.databaseManager().db().GetAccountPath());
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

#ifdef USEWEBENGINE
void TitleBar::setEditor(CWizDocumentWebEngine* editor)
{
    Q_ASSERT(!m_editor);

    m_editorBar->setDelegate(editor);

    connect(editor, SIGNAL(focusIn()), SLOT(onEditorFocusIn()));
    connect(editor, SIGNAL(focusOut()), SLOT(onEditorFocusOut()));

    connect(editor->page(), SIGNAL(contentsChanged()), SLOT(onEditorChanged()));

    connect(m_editTitle, SIGNAL(titleEdited(QString)), editor, SLOT(onTitleEdited(QString)));

    m_editor = editor;
}
#else
void TitleBar::setEditor(CWizDocumentWebView* editor)
{
    Q_ASSERT(!m_editor);

    m_editorBar->setDelegate(editor);

    connect(editor, SIGNAL(focusIn()), SLOT(onEditorFocusIn()));
    connect(editor, SIGNAL(focusOut()), SLOT(onEditorFocusOut()));
    connect(editor, SIGNAL(contentsChanged()), SLOT(onEditorChanged()));

//    connect(editor->page(), SIGNAL(selectionChanged()), SLOT(onEditorChanged()));
    connect(editor->page(), SIGNAL(contentsChanged()), SLOT(onEditorChanged()));

    connect(m_editTitle, SIGNAL(titleEdited(QString)), editor, SLOT(onTitleEdited(QString)));

    m_editor = editor;
}

void TitleBar::setBackgroundColor(QColor color)
{
    QPalette pal = m_editTitle->palette();
    pal.setColor(QPalette::Window, color);
    m_editTitle->setPalette(pal);

    m_editTitle->setStyleSheet("QLineEdit{background:#F5F5F5; border: 1px solid red;}");

//    pal = m_infoBar->palette();
//    pal.setColor(QPalette::Window, color);
//    m_infoBar->setPalette(pal);
}
#endif

void TitleBar::onEditorFocusIn()
{
    showEditorBar();
}

void TitleBar::onEditorFocusOut()
{
    showEditorBar();
    if (!m_editorBar->hasFocus())
        showInfoBar();
}

void TitleBar::updateTagButtonStatus()
{
    if (m_tagBar && m_tagBtn)
    {
        m_tagBtn->setState(m_tagBar->isVisible() ? CellButton::Checked : CellButton::Normal);
    }
}

void TitleBar::updateAttachButtonStatus()
{
    if (m_attachments && m_attachBtn)
    {
        m_attachBtn->setState(m_attachments->isVisible() ? CellButton::Checked : CellButton::Normal);
    }
}

void TitleBar::updateInfoButtonStatus()
{
    if (m_info && m_infoBtn)
    {
        m_infoBtn->setState(m_info->isVisible() ? CellButton::Checked : CellButton::Normal);
    }
}

void TitleBar::updateCommentsButtonStatus()
{
    if (m_commentsBtn && noteView()->commentWidget())
    {
        m_commentsBtn->setState(noteView()->commentWidget()->isVisible() ? CellButton::Checked : CellButton::Normal);
    }
}

void TitleBar::onTitleEditFinished()
{
    m_editTitle->onTitleEditingFinished();
}

void TitleBar::showInfoBar()
{
    m_editorBar->hide();
    m_infoBar->show();
}

void TitleBar::showEditorBar()
{
    m_infoBar->hide();
    m_editorBar->show();
    m_editorBar->adjustButtonPosition();
}

void TitleBar::loadErrorPage()
{
#ifdef USEWEBENGINE
    QWebEngineView* comments = noteView()->commentView();
#else
    noteView()->commentWidget()->hideLocalProgress();
    QWebView* comments = noteView()->commentView();
    comments->setVisible(true);
#endif
    QString strFileName = Utils::PathResolve::resourcesPath() + "files/errorpage/load_fail_comments.html";
    QString strHtml;
    ::WizLoadUnicodeTextFromFile(strFileName, strHtml);
    // clear old url
    comments->load(QUrl());
    QUrl url = QUrl::fromLocalFile(strFileName);
    comments->load(url);
}

void TitleBar::setTagBarVisible(bool visible)
{
    m_tagBar->setVisible(visible);
}

#ifdef USEWEBENGINE
void TitleBar::initWebChannel()
{
    QWebSocketServer *server = new QWebSocketServer(QStringLiteral("Wiz Socket Server"), QWebSocketServer::NonSecureMode, this);
    if (!server->listen(QHostAddress::LocalHost, 0)) {
        qFatal("Failed to open web socket server.");
        return;
    }

    // wrap WebSocket clients in QWebChannelAbstractTransport objects
    WebSocketClientWrapper *clientWrapper  = new WebSocketClientWrapper(server, this);

    // setup the dialog and publish it to the QWebChannel
    QWebChannel *webChannel = new QWebChannel(this);
    // setup the channel
    QObject::connect(clientWrapper, &WebSocketClientWrapper::clientConnected,
                     webChannel, &QWebChannel::connectTo);
    CWizCommentsExternal* exteranl = new CWizCommentsExternal(this);
    webChannel->registerObject(QStringLiteral("externalObject"), exteranl);

    m_strWebchannelUrl = server->serverUrl().toString();
}

void TitleBar::registerWebChannel()
{
    QString strFile = Utils::PathResolve::resourcesPath() + "files/editor/qwebchannel.js";
    QFile f(strFile);
    if (!f.open(QIODevice::ReadOnly))
    {
        qDebug() << "[Comments]Failed to get execute code";
        return;
    }

    QTextStream ts(&f);
    QString strExec = ts.readAll();
    f.close();

    Q_ASSERT(strExec.indexOf("//${INIT_COMMAND}") != -1);
    if (m_strWebchannelUrl.isEmpty())
    {
        initWebChannel();
    }
    QString strCommand = QString("initWebChannel('%1')").arg(m_strWebchannelUrl);
    strExec.replace("//${INIT_COMMAND}", strCommand);
    noteView()->commentView()->page()->runJavaScript(strExec);
}
#endif

void TitleBar::onEditorChanged()
{
    if (m_editor->isModified() || m_editor->isContentsChanged()) {
        m_editBtn->setState(CellButton::Badge);
    } else {
        m_editBtn->setState(noteView()->isEditing() ? CellButton::Checked : CellButton::Normal);
    }
}

void TitleBar::setNote(const WIZDOCUMENTDATA& data, bool editing, bool locked)
{
    updateInfo(data);
    setEditingDocument(editing);
    //
    CWizDatabase& db = m_app.databaseManager().db(data.strKbGUID);
    bool isGroup = db.IsGroup();
    int nTagCount = db.GetDocumentTagCount(data.strGUID);
    setTagBarVisible(!isGroup && nTagCount > 0);
    if (!isGroup)
    {
        m_tagBar->setDocument(data);

    }
}

void TitleBar::updateInfo(const WIZDOCUMENTDATA& doc)
{
    m_infoBar->setDocument(doc);
    m_editTitle->setText(doc.strTitle);
    m_attachBtn->setCount(doc.nAttachmentCount);
}

void TitleBar::setEditingDocument(bool editing)
{
    //
    m_editTitle->setReadOnly(!editing);
    m_editBtn->setState(editing ? CellButton::Checked : CellButton::Normal);
}

void TitleBar::setEditButtonState(bool enable, bool editing)
{
    m_editBtn->setEnabled(enable);
    setEditingDocument(editing);
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

void TitleBar::moveTitileTextToPlaceHolder()
{
    m_editTitle->setPlaceholderText(m_editTitle->text());
    m_editTitle->clear();
}

void TitleBar::clearPlaceHolderText()
{
    m_editTitle->setPlaceholderText("");
}

#define TITLEBARTIPSCHECKED        "TitleBarTipsChecked"

void TitleBar::showCoachingTips()
{
    bool showTips = false;
    if (Core::Internal::MainWindow* mainWindow = Core::Internal::MainWindow::instance())
    {
        showTips = mainWindow->userSettings().get(TITLEBARTIPSCHECKED).toInt() == 0;
    }

    if (showTips)
    {
        CWizTipListManager* manager = CWizTipListManager::instance();
        if (manager->tipsWidgetExists(TITLEBARTIPSCHECKED))
            return;

        CWizTipsWidget* widget = new CWizTipsWidget(TITLEBARTIPSCHECKED, this);
        connect(m_editBtn, SIGNAL(clicked(bool)), widget, SLOT(onTargetWidgetClicked()));
        widget->setAttribute(Qt::WA_DeleteOnClose, true);
        widget->setText(tr("Switch to reading mode"), tr("In reading mode, the note can not be "
                                                         "edited and markdown note can be redered."));
        widget->setSizeHint(QSize(280, 82));
        widget->setButtonVisible(false);
        widget->bindCloseFunction([](){
            if (Core::Internal::MainWindow* mainWindow = Core::Internal::MainWindow::instance())
            {
                mainWindow->userSettings().set(TITLEBARTIPSCHECKED, "1");
            }
        });
        //
        widget->addToTipListManager(m_editBtn, 0, -2);
    }
}

void TitleBar::startEditButtonAnimation()
{
    if (!m_editButtonAnimation)
    {
        m_editButtonAnimation = new CWizAnimateAction(this);
        m_editButtonAnimation->setToolButton(m_editBtn);
        m_editButtonAnimation->setSingleIcons("editButtonProcessing");
    }
    m_editButtonAnimation->startPlay();
}

void TitleBar::stopEditButtonAnimation()
{
    if (!m_editButtonAnimation)
        return;
    if (m_editButtonAnimation->isPlaying())
    {
        m_editButtonAnimation->stopPlay();
    }
}

void TitleBar::applyButtonStateForSeparateWindow(bool inSeparateWindow)
{
    m_separateBtn->setVisible(!inSeparateWindow);
}

void TitleBar::onEditButtonClicked()
{
    bool bEdit = !m_editBtn->state();
    noteView()->setEditNote(bEdit);
    CWizAnalyzer& analyzer = CWizAnalyzer::GetAnalyzer();
    if (bEdit)
    {
        analyzer.LogAction("editNote");
    }
    else
    {
        analyzer.LogAction("viewNote");
    }
}

void TitleBar::onSeparateButtonClicked()
{
    WizGetAnalyzer().LogAction("titleBarViewInSeperateWindow");

    emit viewNoteInSeparateWindow_request();
}

void TitleBar::onTagButtonClicked()
{
//    if (!m_tags) {
//        m_tags = new CWizTagListWidget(topLevelWidget());
//    }

//    m_tags->setDocument(noteView()->note());

//    QRect rc = m_tagBtn->rect();
//    QPoint pt = m_tagBtn->mapToGlobal(QPoint(rc.width()/2, rc.height()));
//    m_tags->showAtPoint(pt);

    setTagBarVisible(!m_tagBar->isVisible());

    WizGetAnalyzer().LogAction("showTags");
}

void TitleBar::onShareButtonClicked()
{
    if (m_shareMenu)
    {
        QPoint pos = m_shareBtn->mapToGlobal(m_shareBtn->rect().bottomLeft());
        m_shareMenu->popup(pos);
    }
}

void TitleBar::onEmailActionClicked()
{
    WizGetAnalyzer().LogAction("shareByEmail");

    m_editor->shareNoteByEmail();
}

void TitleBar::onShareActionClicked()
{
    WizGetAnalyzer().LogAction("shareByLink");

    m_editor->shareNoteByLink();
}

void TitleBar::onAttachButtonClicked()
{
    if (!m_attachments) {
        m_attachments = new CWizAttachmentListWidget(topLevelWidget());
        connect(m_attachments, SIGNAL(widgetStatusChanged()), SLOT(updateAttachButtonStatus()));
    }


    if (m_attachments->setDocument(noteView()->note()))
    {
        QRect rc = m_attachBtn->rect();
        QPoint pt = m_attachBtn->mapToGlobal(QPoint(rc.width()/2, rc.height()));
        m_attachments->showAtPoint(pt);
        updateAttachButtonStatus();
    }

    WizGetAnalyzer().LogAction("showAttachments");
}

void TitleBar::onHistoryButtonClicked()
{
    const WIZDOCUMENTDATA& doc = noteView()->note();

    WizShowDocumentHistory(doc, noteView());

    WizGetAnalyzer().LogAction("showHistory");
}


void TitleBar::onInfoButtonClicked()
{
    if (!m_info) {
        m_info = new CWizNoteInfoForm(topLevelWidget());
        connect(m_info, SIGNAL(widgetStatusChanged()), SLOT(updateInfoButtonStatus()));
    }

    m_info->setDocument(noteView()->note());

    QRect rc = m_infoBtn->rect();
    QPoint pt = m_infoBtn->mapToGlobal(QPoint(rc.width()/2, rc.height()));
    m_info->showAtPoint(pt);
    updateInfoButtonStatus();

    WizGetAnalyzer().LogAction("showNoteInfo");
}

bool isNetworkAccessible()
{
    return true;
    //QNetworkConfigurationManager man;
    //return man.isOnline();
}

#define COMMENT_FRAME_WIDTH 315

void TitleBar::onCommentsButtonClicked()
{
#ifdef USEWEBENGINE
    QWebEngineView* comments = noteView()->commentView();
#else
    CWizDocumentView* view = noteView();
    if (!view)
        return;

    CWizLocalProgressWebView* commentWidget = view->commentWidget();
    connect(commentWidget, SIGNAL(widgetStatusChanged()), this,
            SLOT(updateCommentsButtonStatus()), Qt::UniqueConnection);

#endif
    if (commentWidget->isVisible()) {
        commentWidget->hide();

        WizGetAnalyzer().LogAction("hideComments");

        return;
    }

    WizGetAnalyzer().LogAction("showComments");

    if (isNetworkAccessible()) {
        commentWidget->showLocalProgress();
        QSplitter* splitter = qobject_cast<QSplitter*>(commentWidget->parentWidget());
        Q_ASSERT(splitter);
        QList<int> li = splitter->sizes();
        Q_ASSERT(li.size() == 2);
        QList<int> lin;
        lin.push_back(li.value(0) - COMMENT_FRAME_WIDTH);
        lin.push_back(li.value(1) + COMMENT_FRAME_WIDTH);
        splitter->setSizes(lin);
        commentWidget->show();

        m_commentManager->queryCommentUrl(view->note().strKbGUID, view->note().strGUID);
    } else {
        m_commentsBtn->setEnabled(false);
    }
}

void TitleBar::onCommentPageLoaded(bool ok)
{
#ifdef USEWEBENGINE
    QWebEngineView* comments = noteView()->commentView();
#else
    CWizLocalProgressWebView* commentWidget = noteView()->commentWidget();
    commentWidget->web()->history()->clear();
#endif
    if (!ok)
    {
        qDebug() << "Wow, load comment page failed! " << commentWidget->web()->url();
        loadErrorPage();
        commentWidget->show();
    }
    else
    {
        commentWidget->hideLocalProgress();
    }
#ifdef USEWEBENGINE
    else
    {
        comments->page()->runJavaScript("location.protocol",[this](const QVariant& returnValue) {
            qDebug() << "lcation protocol :  " << returnValue.toString();
            if ("file:" != returnValue.toString())
            {
                registerWebChannel();
            }
        });
    }
#endif
}

void TitleBar::onViewNoteLoaded(INoteView* view, const WIZDOCUMENTDATA& note, bool bOk)
{
    if (!bOk)
        return;    

    if (view != noteView()) {
        return;
    }    

    m_commentsBtn->setCount(0);
    m_commentManager->queryCommentCount(note.strKbGUID, note.strGUID, true);

    CWizLocalProgressWebView* commentWidget = noteView()->commentWidget();
    if (commentWidget && commentWidget->isVisible())
    {
        commentWidget->showLocalProgress();
        m_commentManager->queryCommentUrl(note.strKbGUID, note.strGUID);
    }
}

void TitleBar::on_commentUrlAcquired(QString GUID, QString url)
{
    CWizDocumentView* view = noteView();
    if (view && view->note().strGUID == GUID)
    {
        CWizLocalProgressWebView* commentWidget = noteView()->commentWidget();
        if (commentWidget && commentWidget->isVisible())
        {
            if (url.isEmpty())
            {
                qDebug() << "Wow, query comment url failed!";
                loadErrorPage();
            }
            else
            {
                commentWidget->web()->load(url);
            }
        }
    }
}

void TitleBar::on_commentCountAcquired(QString GUID, int count)
{
    CWizDocumentView* view = noteView();
    if (view && view->note().strGUID == GUID)
    {
        m_commentsBtn->setCount(count);
    }
}

void TitleBar::showMessageTips(Qt::TextFormat format, const QString& strInfo)
{
    m_notifyBar->showMessageTips(format, strInfo);
}

void TitleBar::hideMessageTips(bool useAnimation)
{
    m_notifyBar->hideMessageTips(useAnimation);
}
