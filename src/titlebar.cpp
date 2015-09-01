#include "titlebar.h"

#include <QVBoxLayout>
#include <QUrl>
#include <QNetworkConfigurationManager>
#include <QMessageBox>
#include <QWebHistory>
#include <QSplitter>
#include <QList>
#if QT_VERSION > 0x050000
#include <QtConcurrent>
#else
#include <QtConcurrentRun>
#endif

#include "share/websocketclientwrapper.h"
#include "share/websockettransport.h"
#include "wizWebEngineInjectObject.h"

#include <coreplugin/icore.h>

#include "core/wizTagBar.h"
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

#include "sync/token.h"
#include "sync/apientry.h"
#include "sync/asyncapi.h"
#include "messagecompleter.h"
#include "wizOEMSettings.h"

using namespace Core;
using namespace Core::Internal;

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
{
    m_editTitle->setCompleter(new WizService::MessageCompleter(m_editTitle));
    int nTitleHeight = Utils::StyleHelper::titleEditorHeight();
    m_editTitle->setFixedHeight(nTitleHeight);
    m_editTitle->setAlignment(Qt::AlignVCenter);
    m_editTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    int nEditToolBarHeight = Utils::StyleHelper::editToolBarHeight();
    m_editorBar->setFixedHeight(nEditToolBarHeight);
    m_editorBar->layout()->setAlignment(Qt::AlignVCenter);
    m_infoBar->setFixedHeight(nEditToolBarHeight);

    // FIXME
    QString strTheme = "default";

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    m_editBtn = new CellButton(CellButton::Left, this);
    m_editBtn->setFixedHeight(nTitleHeight);
    QString shortcut = ::WizGetShortcut("EditNote", "Alt+1");
    m_editBtn->setShortcut(QKeySequence::fromString(shortcut));
    m_editBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_lock"), tr("Switch to Editing View (Alt + 1)"));
    m_editBtn->setCheckedIcon(::WizLoadSkinIcon(strTheme, "document_unlock"), tr("Switch to Reading View (Alt + 1)"));
    m_editBtn->setBadgeIcon(::WizLoadSkinIcon(strTheme, "document_unlock_modified"), tr("Save and switch to Reading View (Alt + 1)"));
    connect(m_editBtn, SIGNAL(clicked()), SLOT(onEditButtonClicked()));

    m_tagBtn = new CellButton(CellButton::Center, this);
    m_tagBtn->setFixedHeight(nTitleHeight);
    QString tagsShortcut = ::WizGetShortcut("EditNoteTags", "Alt+2");
    m_tagBtn->setShortcut(QKeySequence::fromString(tagsShortcut));
    m_tagBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_tag"), tr("View and add tags (Alt + 2)"));
    connect(m_tagBtn, SIGNAL(clicked()), SLOT(onTagButtonClicked()));


    m_attachBtn = new CellButton(CellButton::Center, this);
    m_attachBtn->setFixedHeight(nTitleHeight);
    QString attachmentShortcut = ::WizGetShortcut("EditNoteAttachments", "Alt+3");
    m_attachBtn->setShortcut(QKeySequence::fromString(attachmentShortcut));
    m_attachBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_attachment"), tr("Add attachments (Alt + 3)"));
    m_attachBtn->setBadgeIcon(::WizLoadSkinIcon(strTheme, "document_attachment_exist"), tr("View and add attachments (Alt + 3)"));
    connect(m_attachBtn, SIGNAL(clicked()), SLOT(onAttachButtonClicked()));

    m_historyBtn = new CellButton(CellButton::Center, this);
    m_historyBtn->setFixedHeight(nTitleHeight);
    QString historyShortcut = ::WizGetShortcut("EditNoteHistory", "Alt+4");
    m_historyBtn->setShortcut(QKeySequence::fromString(historyShortcut));
    m_historyBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_history"), tr("View and recover note's history (Alt + 4)"));
    connect(m_historyBtn, SIGNAL(clicked()), SLOT(onHistoryButtonClicked()));

    m_infoBtn = new CellButton(CellButton::Center, this);
    m_infoBtn->setFixedHeight(nTitleHeight);
    QString infoShortcut = ::WizGetShortcut("EditNoteInfo", "Alt+5");
    m_infoBtn->setShortcut(QKeySequence::fromString(infoShortcut));
    m_infoBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_info"), tr("View and modify note's info (Alt + 5)"));
    connect(m_infoBtn, SIGNAL(clicked()), SLOT(onInfoButtonClicked()));

    m_emailBtn = new CellButton(CellButton::Center, this);
    m_emailBtn->setFixedHeight(nTitleHeight);
    QString emailShortcut = ::WizGetShortcut("EditNoteEmail", "Alt+6");
    m_emailBtn->setShortcut(QKeySequence::fromString(emailShortcut));
    m_emailBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_email"), tr("Share document by email (Alt + 6)"));
    connect(m_emailBtn, SIGNAL(clicked()), SLOT(onEmailButtonClicked()));
    CWizOEMSettings oemSettings(m_app.databaseManager().db().GetAccountPath());
    m_emailBtn->setVisible(!oemSettings.isHideShareByEmail());

    m_shareBtn = new CellButton(CellButton::Center, this);
    m_shareBtn->setFixedHeight(nTitleHeight);
    QString shareShortcut = ::WizGetShortcut("EditShare", "Alt+7");
    m_shareBtn->setShortcut(QKeySequence::fromString(shareShortcut));
    m_shareBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_share"), tr("Share document (Alt + 7)"));
    connect(m_shareBtn, SIGNAL(clicked()), SLOT(onShareButtonClicked()));
    m_shareBtn->setVisible(!oemSettings.isHideShare());

    // comments
    m_commentsBtn = new CellButton(CellButton::Right, this);
    m_commentsBtn->setFixedHeight(nTitleHeight);
    QString commentShortcut = ::WizGetShortcut("ShowComment", "Alt+c");
    m_commentsBtn->setShortcut(QKeySequence::fromString(commentShortcut));
    m_commentsBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "comments"), tr("Add comments (Alt + c)"));
    m_commentsBtn->setBadgeIcon(::WizLoadSkinIcon(strTheme, "comments_exist"), tr("View and add comments (Alt + c)"));
    connect(m_commentsBtn, SIGNAL(clicked()), SLOT(onCommentsButtonClicked()));
    connect(ICore::instance(), SIGNAL(viewNoteLoaded(Core::INoteView*,const WIZDOCUMENTDATA&,bool)),
            SLOT(onViewNoteLoaded(Core::INoteView*,const WIZDOCUMENTDATA&,bool)));

    QWidget* line1 = new QWidget(this);
    line1->setFixedHeight(1);
    line1->setStyleSheet("border-top-width:1;border-top-style:solid;border-top-color:#DFDFD7;");

    m_tagBarSpacer = new QWidget(this);
    m_tagBarSpacer->setFixedHeight(1);
    m_tagBarSpacer->setStyleSheet("border-top-width:1;border-top-style:solid;border-top-color:#DFDFD7;");


    QWidget* line3 = new QWidget(this);
    line3->setFixedHeight(1);
    line3->setStyleSheet("border-top-width:1;border-top-style:solid;border-top-color:#d9dcdd");

    QHBoxLayout* layoutInfo2 = new QHBoxLayout();
    layoutInfo2->setContentsMargins(0, 0, 0, 0);
    layoutInfo2->setSpacing(0);
    layoutInfo2->addWidget(m_editTitle);
    layoutInfo2->addWidget(m_editBtn);
    layoutInfo2->addWidget(m_tagBtn);
    layoutInfo2->addWidget(m_attachBtn);
    layoutInfo2->addWidget(m_historyBtn);
    layoutInfo2->addWidget(m_infoBtn);
    layoutInfo2->addWidget(m_emailBtn);
    layoutInfo2->addWidget(m_shareBtn);
    layoutInfo2->addWidget(m_commentsBtn);


    QVBoxLayout* layoutInfo1 = new QVBoxLayout();
    layoutInfo1->setContentsMargins(0, 0, 0, 0);
    layoutInfo1->setSpacing(0);
    layoutInfo1->addLayout(layoutInfo2);
    layoutInfo1->addWidget(line1);
    layoutInfo1->addWidget(m_tagBar);
    layoutInfo1->addWidget(m_tagBarSpacer);
    layoutInfo1->addWidget(m_infoBar);
    layoutInfo1->addWidget(m_editorBar);
    layoutInfo1->addWidget(line3);
    m_editorBar->hide();

    layout->addLayout(layoutInfo1);
    //layout->addLayout(layoutInfo4);
    layout->addWidget(m_notifyBar);

    layout->addStretch();
    connect(m_notifyBar, SIGNAL(labelLink_clicked(QString)), SIGNAL(notifyBar_link_clicked(QString)));

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
    m_notifyBar->showPermissionNotify(nReason);
    m_editTitle->setReadOnly(bReadOnly);
    m_editBtn->setEnabled(!bReadOnly);

    if (nReason == NotifyBar::Deleted)
    {
        m_tagBtn->setEnabled(false);
        m_historyBtn->setEnabled(false);
        m_commentsBtn->setEnabled(false);
        m_shareBtn->setEnabled(false);
        m_emailBtn->setEnabled(false);
    }
    else
    {
        CWizOEMSettings oemSettings(m_app.databaseManager().db().GetAccountPath());
        m_tagBtn->setVisible(bIsGroup ? false : true);
        m_tagBtn->setEnabled(bIsGroup ? false : true);
        m_shareBtn->setVisible(bIsGroup ? false : !oemSettings.isHideShare());
        m_shareBtn->setEnabled(bIsGroup ? false : true);
        m_historyBtn->setEnabled(true);
        m_commentsBtn->setEnabled(true);
        m_emailBtn->setVisible(!oemSettings.isHideShareByEmail());
        m_emailBtn->setEnabled(!oemSettings.isHideShareByEmail());
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
    m_tagBarSpacer->setVisible(visible);
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
    m_attachBtn->setState(doc.nAttachmentCount > 0 ? CellButton::Badge : CellButton::Normal);
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

void TitleBar::startEditButtonAnimation()
{
    if (!m_editButtonAnimation)
    {
        m_editButtonAnimation = new CWizAnimateAction(m_app, this);
        m_editButtonAnimation->setToolButton(m_editBtn);
        m_editButtonAnimation->setTogetherIcon("editButtonProcessing");
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

void TitleBar::onEmailButtonClicked()
{
    m_editor->shareNoteByEmail();

    WizGetAnalyzer().LogAction("shareByEmail");
}

void TitleBar::onShareButtonClicked()
{
    m_editor->shareNoteByLink();

    WizGetAnalyzer().LogAction("shareByLink");
}

void TitleBar::onAttachButtonClicked()
{
    if (!m_attachments) {
        m_attachments = new CWizAttachmentListWidget(topLevelWidget());
    }


    if (m_attachments->setDocument(noteView()->note()))
    {
        QRect rc = m_attachBtn->rect();
        QPoint pt = m_attachBtn->mapToGlobal(QPoint(rc.width()/2, rc.height()));
        m_attachments->showAtPoint(pt);
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
    }

    m_info->setDocument(noteView()->note());

    QRect rc = m_infoBtn->rect();
    QPoint pt = m_infoBtn->mapToGlobal(QPoint(rc.width()/2, rc.height()));
    m_info->showAtPoint(pt);

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
    CWizLocalProgressWebView* commentWidget = noteView()->commentWidget();
#endif
    if (commentWidget->isVisible()) {
        commentWidget->hide();

        WizGetAnalyzer().LogAction("hideComments");

        return;
    }

    WizGetAnalyzer().LogAction("showComments");

    if (isNetworkAccessible()) {
        if (!m_commentsUrl.isEmpty()) {
            commentWidget->showLocalProgress();
            commentWidget->web()->load(m_commentsUrl);
            QSplitter* splitter = qobject_cast<QSplitter*>(commentWidget->parentWidget());
            Q_ASSERT(splitter);
            QList<int> li = splitter->sizes();
            Q_ASSERT(li.size() == 2);
            QList<int> lin;
            lin.push_back(li.value(0) - COMMENT_FRAME_WIDTH);
            lin.push_back(li.value(1) + COMMENT_FRAME_WIDTH);
            splitter->setSizes(lin);
            commentWidget->show();
        } else {
            loadErrorPage();
            commentWidget->show();
        }

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
    Q_UNUSED(note);

    if (!bOk)
        return;    

    if (view != noteView()) {
        return;
    }    

    m_commentsUrl.clear();
    connect(WizService::Token::instance(), SIGNAL(tokenAcquired(QString)),
            SLOT(onTokenAcquired(QString)), Qt::QueuedConnection);
    WizService::Token::requestToken();
}

void TitleBar::onTokenAcquired(const QString& strToken)
{
    WizService::Token::instance()->disconnect(this);

#ifdef USEWEBENGINE
    QWebEngineView* comments = noteView()->commentView();
#else
    CWizLocalProgressWebView* commentWidget = noteView()->commentWidget();
#endif
    if (strToken.isEmpty())
    {
        qDebug() << "Can not get token, hide the comment widget";
        commentWidget->hide();
        return;
    }

    commentWidget->showLocalProgress();
    //
    QtConcurrent::run([this, strToken, commentWidget](){
        QString strKbGUID = noteView()->note().strKbGUID;
        QString strGUID = noteView()->note().strGUID;
        m_commentsUrl =  WizService::CommonApiEntry::commentUrl(strToken, strKbGUID, strGUID);
        if (m_commentsUrl.isEmpty())
        {
            qDebug() << "Can not get comment url by token : " << strToken;
            QMetaObject::invokeMethod(this, "loadErrorPage", Qt::QueuedConnection);
            return;
        }

        if (commentWidget->isVisible())
        {
            emit loadComment_request(m_commentsUrl);
        }

        QString kUrl = WizService::CommonApiEntry::kUrlFromGuid(strToken, strKbGUID);
        QString strCountUrl = WizService::CommonApiEntry::commentCountUrl(kUrl, strToken, strKbGUID, strGUID);

        WizService::AsyncApi* api = new WizService::AsyncApi(nullptr);
        connect(api, SIGNAL(getCommentsCountFinished(int)), SLOT(onGetCommentsCountFinished(int)));
        api->getCommentsCount(strCountUrl);
    });
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

void TitleBar::showMessageTips(Qt::TextFormat format, const QString& strInfo)
{
    m_notifyBar->showMessageTips(format, strInfo);
}

void TitleBar::hideMessageTips(bool useAnimation)
{
    m_notifyBar->hideMessageTips(useAnimation);
}
