#ifndef CORE_TITLEBAR_H
#define CORE_TITLEBAR_H

#include <QWidget>
#include <QIcon>
#include "share/wizobject.h"

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

class CWizDocumentView;
class INoteView;
class CWizCommentManager;

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
    void setEditor(CWizDocumentWebView* editor);

    void setBackgroundColor(QColor color);

    void setNote(const WIZDOCUMENTDATA& data, WizEditorMode editorMode, bool locked);
    void updateInfo(const WIZDOCUMENTDATA& doc);
    void setEditorMode(WizEditorMode editorMode);
    void setEditButtonEnabled(bool enable);
    void updateEditButton(WizEditorMode editorMode);
    void resetTitle(const QString& strTitle);
    void moveTitileTextToPlaceHolder();
    void clearPlaceHolderText();

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
    void onViewNoteLoaded(CWizDocumentView* view, const WIZDOCUMENTDATA& note, bool bOk);

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

    void showCoachingTips();
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
    //
    CWizDocumentWebView* m_editor;
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


#endif // CORE_TITLEBAR_H
