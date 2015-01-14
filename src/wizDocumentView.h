#ifndef CORE_WIZDOCUMENTVIEW_H
#define CORE_WIZDOCUMENTVIEW_H

#include <coreplugin/inoteview.h>

#include "share/wizobject.h"
#include <QSharedPointer>

class QWebView;
class QScrollArea;
class QLineEdit;
class QLabel;

struct WIZDOCUMENTDATA;
struct WIZDOCUMENTATTACHMENTDATA;
class CWizExplorerApp;
class CWizDatabaseManager;
class CWizUserSettings;
class CWizScrollBar;
class CWizDocumentWebView;
class CWizDatabase;
class CWizSplitter;
class CWizUserCipherForm;
class CWizObjectDataDownloaderHost;
class QStackedWidget;
class QWebFrame;
class CWizDocumentEditStatusSyncThread;
class CWizDocumentStatusCheckThread;
class CWizDocumentStatusChecker;

namespace Core {
namespace Internal {
class TitleBar;
class EditorToolBar;
} // namespace Internal

class CWizDocumentView : public INoteView
{
    Q_OBJECT

public:
    CWizDocumentView(CWizExplorerApp& app, QWidget* parent = 0);
    ~CWizDocumentView();
    virtual QSize sizeHint() const { return QSize(200, 1); }

    QWidget* client() const;
    CWizDocumentWebView* web() const { return m_web; }
    QWebView* commentView() const { return m_comments; }
    //
    void waitForDone();

protected:
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;
    CWizUserSettings& m_userSettings;
    CWizObjectDataDownloaderHost* m_downloaderHost;

    QStackedWidget* m_tab;
    QWidget* m_msgWidget;
    QLabel* m_msgLabel;

    QWidget* m_docView;
    CWizDocumentWebView* m_web;
    QWebView* m_comments;
    CWizSplitter* m_splitter;
    Core::Internal::TitleBar* m_title;

    CWizUserCipherForm* m_passwordView;
    CWizDocumentEditStatusSyncThread* m_editStatusSyncThread;
//    CWizDocumentStatusCheckThread* m_editStatusCheckThread;
    CWizDocumentStatusChecker* m_editStatusChecker;

    virtual void showEvent(QShowEvent *event);

private:
    WIZDOCUMENTDATA m_note;
    bool m_bLocked; // note is force locked as readonly status
    bool m_bEditingMode; // true: editing mode, false: reading mode
    int m_viewMode; // user defined editing mode
    bool m_noteLoaded;
    //
    int m_status;  // document edit or version status

public:
    const WIZDOCUMENTDATA& note() const { return m_note; }
    bool isLocked() const { return m_bLocked; }
    bool isEditing() const { return m_bEditingMode; }
    bool defaultEditingMode();
    bool reload();
    void reloadNote();
    void setEditorFocus();
    bool noteLoaded() const { return m_noteLoaded; }

    void initStat(const WIZDOCUMENTDATA& data, bool bEditing);
    void viewNote(const WIZDOCUMENTDATA& data, bool forceEdit);
    void reviewCurrentNote();
    void showClient(bool visible);
    void setEditNote(bool bEdit);
    void setViewMode(int mode);
    void setModified(bool modified);
    void settingsChanged();
    void sendDocumentSavedSignal(const QString& strGUID, const QString& strKbGUID);
    void resetTitle(const QString& strTitle);
    void promptMessage(const QString& strMsg);
    bool checkListClickable();
    void setStatusToEditingByCheckList();

    QWebFrame* noteFrame();

signals:
    void documentSaved(const QString& strGUID, CWizDocumentView* viewer);
    void checkDocumentEditStatusRequest(const QString& strKbGUID, const QString& strGUID);
    void stopCheckDocumentEditStatusRequest(const QString& strKbGUID, const QString& strGUID);

public Q_SLOTS:
    void onViewNoteRequested(Core::INoteView* view, const WIZDOCUMENTDATA& doc);
    void onViewNoteLoaded(Core::INoteView*,const WIZDOCUMENTDATA&,bool);
    void onCloseNoteRequested(Core::INoteView* view);

    void onCipherCheckRequest();

    void on_download_finished(const WIZOBJECTDATA& data, bool bSucceed);

    void on_document_modified(const WIZDOCUMENTDATA& documentOld,
                              const WIZDOCUMENTDATA& documentNew);
    void on_document_data_modified(const WIZDOCUMENTDATA& data);
    void on_document_data_saved(const QString& strGUID, CWizDocumentView* viewer);

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

private:
    void loadNote(const WIZDOCUMENTDATA &doc);
    void downloadDocumentFromServer();
    void sendDocumentEditingStatus();
    void stopDocumentEditingStatus();
    void startCheckDocumentEditStatus();
    void stopCheckDocumentEditStatus();
    bool checkDocumentEditable();
};

class WizFloatDocumentViewer : public QWidget
{
    Q_OBJECT
public:
    WizFloatDocumentViewer(CWizExplorerApp& app, QWidget* parent = 0);

    CWizDocumentView* docView()
    {
        return m_docView;
    }

    ~WizFloatDocumentViewer();

private:
    CWizDocumentView* m_docView;
};

} // namespace Core

#endif // CORE_WIZDOCUMENTVIEW_H
