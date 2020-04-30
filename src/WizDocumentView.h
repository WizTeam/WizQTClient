#ifndef CORE_WIZDOCUMENTVIEW_H
#define CORE_WIZDOCUMENTVIEW_H

#include "share/WizObject.h"
#include <QSharedPointer>
#include <QWidget>

class QScrollArea;
class QLineEdit;
class QLabel;


struct WIZDOCUMENTDATA;
struct WIZDOCUMENTATTACHMENTDATA;
class WizExplorerApp;
class WizDatabaseManager;
class WizUserSettings;
class WizScrollBar;
class WizDocumentWebView;
class WizDatabase;
class WizSplitter;
class WizUserCipherForm;
class WizObjectDownloaderHost;
class QStackedWidget;
class QWebFrame;
class QWebEnginePage;
class WizWebEngineView;
class WizDocumentEditStatusSyncThread;
class WizDocumentStatusChecker;
class WizLocalProgressWebView;
class WizDocumentTransitionView;

class WizTitleBar;
class WizEditorToolBar;
class WizTagBar;

class WizDocumentView : public QWidget
{
    Q_OBJECT

public:
    WizDocumentView(WizExplorerApp& app, QWidget* parent = 0);
    ~WizDocumentView();
    virtual QSize sizeHint() const;
    void setSizeHint(QSize size);

    QWidget* client() const;
    WizDocumentWebView* web() const { return m_web; }
    WizWebEngineView* commentView() const;
    WizLocalProgressWebView* commentWidget() const;
    //
    WizDocumentTransitionView* transitionView();
    //
    WizTitleBar* titleBar();
    //
    void waitForDone();

protected:
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;
    WizUserSettings& m_userSettings;
    WizObjectDownloaderHost* m_downloaderHost;
    WizDocumentTransitionView* m_transitionView;

    QStackedWidget* m_tab;
    QWidget* m_msgWidget;
    QLabel* m_msgLabel;

    QWidget* m_docView;
    WizDocumentWebView* m_web;
    WizWebEngineView* m_comments;
    WizLocalProgressWebView* m_commentWidget;
    WizSplitter* m_splitter;
    WizTitleBar* m_title;
    QWidget* m_blankView;

    WizUserCipherForm* m_passwordView;
    WizDocumentEditStatusSyncThread* m_editStatusSyncThread;
//    CWizDocumentStatusCheckThread* m_editStatusCheckThread;
    WizDocumentStatusChecker* m_editStatusChecker;

    virtual void showEvent(QShowEvent *event);
    virtual void resizeEvent(QResizeEvent* ev);

private:
    WIZDOCUMENTDATAEX m_note;
    bool m_bLocked; // note is force locked as readonly status
    WizEditorMode m_editorMode;
    WizDocumentViewMode m_defaultViewMode; // user defined editing mode
    bool m_noteLoaded;
    //
    int m_editStatus;  // document edit or version status
    QSize m_sizeHint;

public:
    const WIZDOCUMENTDATAEX& note() const { return m_note; }
    bool isLocked() const { return m_bLocked; }
    bool isEditing() const { return m_editorMode == modeEditor; }
    WizEditorMode editorMode() const { return m_editorMode; }
    bool reload();
    void reloadNote();
    void setEditorFocus();
    bool noteLoaded() const { return m_noteLoaded; }
    void changeType(QString type) { m_note.strType = type; }

    void initStat(const WIZDOCUMENTDATA& data, bool forceEdit);
    void viewNote(const WIZDOCUMENTDATAEX& data, bool forceEdit);
    void reviewCurrentNote();
    void setEditorMode(WizEditorMode editorMode);
    void setDefaultViewMode(WizDocumentViewMode mode);
    void setModified(bool modified);
    void settingsChanged();
    void sendDocumentSavedSignal(const QString& strGUID, const QString& strKbGUID);
    void resetTitle(const QString& strTitle);
    void promptMessage(const QString& strMsg);
    bool checkListClickable();
    void setStatusToEditingByCheckList();
    //
    void showCoachingTips();
    //
    void wordCount(std::function<void(const QString&)> callback);

signals:
    void documentSaved(const QString& strGUID, WizDocumentView* viewer);
    void checkDocumentEditStatusRequest(const QString& strKbGUID, const QString& strGUID);
    void stopCheckDocumentEditStatusRequest(const QString& strKbGUID, const QString& strGUID);

public Q_SLOTS:
    void onViewNoteRequested(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool forceEditing);
    void onViewNoteLoaded(WizDocumentView*,const WIZDOCUMENTDATAEX&,bool);
    void onCloseNoteRequested(WizDocumentView* view);

    void onCipherCheckRequest();

    void on_download_finished(const WIZOBJECTDATA& data, bool bSucceed);

    void on_document_modified(const WIZDOCUMENTDATA& documentOld,
                              const WIZDOCUMENTDATA& documentNew);
    void on_document_data_modified(const WIZDOCUMENTDATA& data);
    void on_document_data_changed(const QString& strGUID, WizDocumentView* viewer);

    void on_attachment_created(const WIZDOCUMENTATTACHMENTDATA& attachment);
    void on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA& attachment);

    //
    void on_checkEditStatus_finished(const QString& strGUID, bool editable);
    void on_checkEditStatus_timeout(const QString& strGUID);
    void on_documentEditingByOthers(QString strGUID, QStringList editors);
    void on_checkDocumentChanged_finished(const QString& strGUID, bool changed);
    void on_syncDatabase_request(const QString& strKbGUID);
    void on_webView_focus_changed();

    void on_notifyBar_link_clicked(const QString& link);

    void on_loadComment_request(const QString& url);

    void on_commentWidget_statusChanged();

private:
    void loadNote(const WIZDOCUMENTDATAEX &doc);
    void downloadNoteFromServer(const WIZDOCUMENTDATA& note);
    void sendDocumentEditingStatus();
    void stopDocumentEditingStatus();
    void startCheckDocumentEditStatus();
    void stopCheckDocumentEditStatus();
    bool checkDocumentEditable(bool checklist);
    //
    void stopCheckDocumentAnimations();    
};


#endif // CORE_WIZDOCUMENTVIEW_H
