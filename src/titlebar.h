#ifndef CORE_TITLEBAR_H
#define CORE_TITLEBAR_H

#include <QWidget>
#include <QIcon>

class QString;

struct WIZDOCUMENTDATA;
class CWizDatabase;
class CWizTagListWidget;
class CWizNoteInfoForm;
class CWizDocumentWebEngine;
class CWizDocumentWebView;
class CWizAttachmentListWidget;
class CWizAnimateAction;
class CWizExplorerApp;
class QNetworkReply;

namespace Core {
class CWizDocumentView;
class INoteView;

namespace Internal {
class TitleEdit;
class InfoBar;
class NotifyBar;
class EditorToolBar;
class CellButton;
class CWizTagBar;

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(CWizExplorerApp& app, QWidget *parent);
    CWizDocumentView* noteView();
    void setLocked(bool bReadOnly, int nReason, bool bIsGroup);
    void showMessageTips(Qt::TextFormat format, const QString& strInfo);
    void hideMessageTips(bool useAnimation);
#ifdef USEWEBENGINE
    void setEditor(CWizDocumentWebEngine* editor);
#else
    void setEditor(CWizDocumentWebView* editor);
#endif


    void setNote(const WIZDOCUMENTDATA& data, bool editing, bool locked);
    void updateInfo(const WIZDOCUMENTDATA& doc);
    void setEditingDocument(bool editing);
    void setEditButtonState(bool enable, bool editing);
    void updateEditButton(bool bEditing);
    void resetTitle(const QString& strTitle);
    void moveTitileTextToPlaceHolder();
    void clearPlaceHolderText();

    void startEditButtonAnimation();
    void stopEditButtonAnimation();


public Q_SLOTS:
    void onEditButtonClicked();
    void onTagButtonClicked();
    void onEmailButtonClicked();
    void onShareButtonClicked();
    void onAttachButtonClicked();
    void onHistoryButtonClicked();
    void onInfoButtonClicked();

    void onCommentsButtonClicked();
    void onCommentPageLoaded(bool ok);
    void onViewNoteLoaded(Core::INoteView* view, const WIZDOCUMENTDATA& note, bool bOk);
    void onTokenAcquired(const QString& strToken);
    void onGetCommentsCountFinished(int nCount);

    void onEditorChanged();
    void onEditorFocusIn();
    void onEditorFocusOut();

    //
    void onTitleEditFinished();

    void loadErrorPage();
signals:
    void notifyBar_link_clicked(const QString& link);
    void loadComment_request(const QString& url);
private:
    void showInfoBar();
    void showEditorBar();
    void setTagBarVisible(bool visible);
#ifdef USEWEBENGINE
    //
    void initWebChannel();
    void registerWebChannel();

private:
    CWizDocumentWebEngine* m_editor;
#else
    CWizDocumentWebView* m_editor;
#endif
    CWizExplorerApp& m_app;

    TitleEdit* m_editTitle;
    CWizTagBar* m_tagBar;
    QWidget* m_tagBarSpacer;
    InfoBar* m_infoBar;
    NotifyBar* m_notifyBar;
    EditorToolBar* m_editorBar;

    CellButton* m_editBtn;
    CellButton* m_tagBtn;
    CellButton* m_emailBtn;
    CellButton* m_shareBtn;
    CellButton* m_attachBtn;
    CellButton* m_historyBtn;
    CellButton* m_infoBtn;

    CellButton* m_commentsBtn;
    QString m_commentsUrl;

    CWizTagListWidget* m_tags;
    CWizAttachmentListWidget* m_attachments;
    CWizNoteInfoForm* m_info;

    QString m_strWebchannelUrl;
    CWizAnimateAction* m_editButtonAnimation;
};

} //namesapce Internal
} // namespace Core

#endif // CORE_TITLEBAR_H
