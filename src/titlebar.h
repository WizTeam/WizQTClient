#ifndef CORE_TITLEBAR_H
#define CORE_TITLEBAR_H

#include <QWidget>
#include <QIcon>

class QString;
class QMenu;

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
class CWizCommentManager;

namespace Internal {
class TitleEdit;
class InfoBar;
class NotifyBar;
class EditorToolBar;
class CellButton;
class RoundCellButton;
class CWizTagBar;

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(CWizExplorerApp& app, QWidget *parent);
    CWizDocumentView* noteView();
    EditorToolBar* editorToolBar();
    void setLocked(bool bReadOnly, int nReason, bool bIsGroup);
    void showMessageTips(Qt::TextFormat format, const QString& strInfo);
    void hideMessageTips(bool useAnimation);
#ifdef USEWEBENGINE
    void setEditor(CWizDocumentWebEngine* editor);
#else
    void setEditor(CWizDocumentWebView* editor);
#endif

    void setBackgroundColor(QColor color);

    void setNote(const WIZDOCUMENTDATA& data, bool editing, bool locked);
    void updateInfo(const WIZDOCUMENTDATA& doc);
    void setEditingDocument(bool editing);
    void setEditButtonState(bool enable, bool editing);
    void updateEditButton(bool bEditing);
    void resetTitle(const QString& strTitle);
    void moveTitileTextToPlaceHolder();
    void clearPlaceHolderText();
    void showCoachingTips();

    void startEditButtonAnimation();
    void stopEditButtonAnimation();

    void applyButtonStateForSeparateWindow(bool inSeparateWindow);

public Q_SLOTS:
    void onEditButtonClicked();
    void onSeparateButtonClicked();
    void onTagButtonClicked();
    void onShareButtonClicked();
    void onAttachButtonClicked();
    void onHistoryButtonClicked();
    void onInfoButtonClicked();

    void onEmailActionClicked();
    void onShareActionClicked();

    void onCommentsButtonClicked();
    void onCommentPageLoaded(bool ok);
    void onViewNoteLoaded(Core::INoteView* view, const WIZDOCUMENTDATA& note, bool bOk);

    void on_commentUrlAcquired(QString GUID, QString url);
    void on_commentCountAcquired(QString GUID, int count);

    void onEditorChanged();
    void onEditorFocusIn();
    void onEditorFocusOut();

    //
    void updateTagButtonStatus();
    void updateAttachButtonStatus();
    void updateInfoButtonStatus();
    void updateCommentsButtonStatus();

    //
    void onTitleEditFinished();

    void loadErrorPage();
signals:
    void notifyBar_link_clicked(const QString& link);
    void loadComment_request(const QString& url);
    void viewNoteInSeparateWindow_request();
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
    InfoBar* m_infoBar;
    NotifyBar* m_notifyBar;
    EditorToolBar* m_editorBar;

    RoundCellButton* m_editBtn;
    CellButton* m_separateBtn;
    CellButton* m_tagBtn;    
//    CellButton* m_emailBtn;
    CellButton* m_shareBtn;
    CellButton* m_attachBtn;
//    CellButton* m_historyBtn;
    CellButton* m_infoBtn;    

    QMenu* m_shareMenu;

    CellButton* m_commentsBtn;

    CWizCommentManager* m_commentManager;

    CWizTagListWidget* m_tags;
    CWizAttachmentListWidget* m_attachments;
    CWizNoteInfoForm* m_info;

    QString m_strWebchannelUrl;
    CWizAnimateAction* m_editButtonAnimation;
};

} //namesapce Internal
} // namespace Core

#endif // CORE_TITLEBAR_H
