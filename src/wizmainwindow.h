#ifndef WIZMAINWINDOW_H
#define WIZMAINWINDOW_H

#include <QtGlobal>
#include <QMainWindow>
#include <QPushButton>

#include "wizdef.h"
#include "share/wizuihelper.h"
#include "share/wizsettings.h"
#include "wizUpgrade.h"
#include "wizconsoledialog.h"
#include "wizCategoryView.h"
#include "wizDocumentListView.h"
#include "wizcertmanager.h"
#include "wizusercipherform.h"
//#include "wizdownloadobjectdatadialog.h"
#include "wizDocumentView.h"
#ifndef Q_OS_MAC
#include "share/wizshadowwindow.h"
#endif

class QToolBar;
class QLabel;
class QSystemTrayIcon;

class CWizProgressDialog;
class CWizDocumentListView;
class CWizDocumentSelectionView;
class CWizDocumentTransitionView;
class CWizActions;
class CWizDocumentViewHistory;
class CWizFixedSpacer;
class CWizSplitter;
class CWizAnimateAction;
class CWizOptionsWidget;

class CWizSearchWidget;
class CWizSearcher;
class CWizSearchIndexer;

class QtSegmentControl;
class CWizObjectDataDownloaderHost;
class CWizUserAvatarDownloaderHost;
class CWizKMSyncThread;
class CWizUserVerifyDialog;

class CWizMacToolBar;

class CWizDocumentWebView;

namespace WizService {
namespace Internal {
class MessageListView;
}
}

namespace Core {
class ICore;
class CWizDocumentView;

namespace Internal {

class MainWindow
#ifdef Q_OS_MAC
    : public QMainWindow
#else
    : public CWizShadowWindow<QMainWindow>
#endif
    , public CWizExplorerApp
{
    Q_OBJECT

#ifdef Q_OS_MAC
    typedef QMainWindow  _baseClass;
#else
    typedef CWizShadowWindow<QMainWindow> _baseClass;
#endif

public:
    explicit MainWindow(CWizDatabaseManager& dbMgr, QWidget *parent = 0);
    virtual void init();

    void saveStatus();
    void restoreStatus();
    ~MainWindow();

    void cleanOnQuit();

    void setRestart(bool b) { m_bRestart = b; }
    bool isRestart() const { return m_bRestart; }
    Q_PROPERTY(bool restart READ isRestart WRITE setRestart)

    bool isLogout() const { return m_bLogoutRestart; }

    QString searchKeywords() const { return m_strSearchKeywords; }

    static MainWindow* instance();

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event);
    virtual void resizeEvent(QResizeEvent* event);
    virtual void closeEvent(QCloseEvent* event);

private:
    ICore* m_core;
    CWizDatabaseManager& m_dbMgr;
    CWizProgressDialog* m_progress;
    CWizUserSettings* m_settings;
    QPointer<CWizKMSyncThread> m_sync;
    QPointer<CWizUserVerifyDialog> m_userVerifyDialog;
    QPointer<CWizConsoleDialog> m_console;
    QPointer<CWizUpgrade> m_upgrade;

    CWizObjectDataDownloaderHost* m_objectDownloaderHost;
    //CWizUserAvatarDownloaderHost* m_avatarDownloaderHost;
    //
    QSystemTrayIcon* m_tray;

#ifdef Q_OS_MAC
    QToolBar* m_toolBar;
    QMenuBar* m_menuBar;
#else
    QToolBar* m_toolBar;
    QMenu* m_menu;
    QToolButton* m_menuButton;
#endif


#ifndef Q_OS_MAC
    QLabel* m_labelNotice;
    QAction* m_optionsAction;
#endif

    CWizActions* m_actions;
    QPointer<CWizCategoryView> m_category;
    CWizDocumentListView* m_documents;
    WizService::Internal::MessageListView* m_msgList;
    QWidget* m_noteList;
    CWizDocumentSelectionView* m_documentSelection;
    CWizDocumentView* m_doc;
    CWizDocumentTransitionView* m_transitionView;
    QPointer<CWizSplitter> m_splitter;
    QPointer<CWizOptionsWidget> m_options;

    QLabel* m_labelDocumentsHint;
    QLabel* m_labelDocumentsCount;

    CWizDocumentViewHistory* m_history;
    QPointer<CWizAnimateAction> m_animateSync;

    QPointer<CWizSearcher> m_searcher;
    QString m_strSearchKeywords;

    CWizSearchIndexer* m_searchIndexer;
    QPointer<CWizSearchWidget> m_search;
#ifdef Q_OS_LINUX
    CWizFixedSpacer* m_spacerBeforeSearch;
#else
    CWizSpacer* m_spacerBeforeSearch;
#endif

    bool m_bRestart;
    bool m_bLogoutRestart;
    bool m_bUpdatingSelection;

    WIZDOCUMENTDATA m_documentForEditing;

private:
    void initActions();

    void initToolBar();
    void initClient();
    //
#ifndef Q_OS_MAC
    virtual void layoutTitleBar();
    void initMenuList();
#else
    void initMenuBar();
#endif

    QWidget* createListView();


public:
    // CWizDocument passthrough methods
    QSize clientSize() const { return m_splitter->widget(2)->size(); }
    QWidget* client() const { return m_doc->client(); }
    CWizDocumentWebView* web() const { return m_doc->web(); }
    void showClient(bool visible) const { return m_doc->showClient(visible); }

    CWizActions* actions() const { return m_actions; }
    //CWizDownloadObjectDataDialog* objectDownloadDialog() const { return m_objectDownloadDialog; }
    CWizObjectDataDownloaderHost* downloaderHost() const { return m_objectDownloaderHost; }
    //CWizUserAvatarDownloaderHost* avatarHost() const { return m_avatarDownloaderHost; }
    CWizProgressDialog* progressDialog() const { return m_progress; }
    CWizDocumentTransitionView* transitionView() const { return m_transitionView; }

    void resetPermission(const QString& strKbGUID, const QString& strDocumentOwner);
    void viewDocument(const WIZDOCUMENTDATA& data, bool addToHistory);
    void locateDocument(const WIZDOCUMENTDATA& data);
    //
    void viewDocumentInFloatWidget(const WIZDOCUMENTDATA& data);
    //
    static void quickSyncKb(const QString& kbGuid);

    void checkWizUpdate();
    void setSystemTrayIconVisible(bool bVisible);
    //
    void viewDocumentByWizKMURL(const QString& strKMURL);
    //
    void createNoteWithAttachments(const QStringList& strAttachList);
    void createNoteWithText(const QString& strText);
signals:
    void documentSaved(const QString& strGUID, CWizDocumentView* viewer);

public Q_SLOTS:
    void on_actionExit_triggered();
    void on_actionConsole_triggered();
    void on_actionAutoSync_triggered();
    void on_actionSync_triggered();
    void on_actionNewNote_triggered();
    void on_actionLogout_triggered();
    void on_actionAbout_triggered();
    void on_actionPreference_triggered();
    void on_actionRebuildFTS_triggered();
    void on_actionFeedback_triggered();
    void on_actionSupport_triggered();
    void on_actionSearch_triggered();
    void on_actionResetSearch_triggered();
    void on_actionSearchReplace_triggered();
    void on_actionSaveAsPDF_triggered();
    void on_actionPrint_triggered();

    // menu editing
    void on_actionEditingUndo_triggered();
    void on_actionEditingRedo_triggered();

    void on_actionEditingCut_triggered();
    void on_actionEditingCopy_triggered();
    void on_actionEditingPaste_triggered();
    void on_actionEditingPastePlain_triggered();
    void on_actionEditingDelete_triggered();
    void on_actionEditingSelectAll_triggered();

    // menu view
    void on_actionViewToggleCategory_triggered();
    void on_actionViewToggleFullscreen_triggered();

    // menu format
    void on_actionFormatJustifyLeft_triggered();
    void on_actionFormatJustifyRight_triggered();
    void on_actionFormatJustifyCenter_triggered();
    void on_actionFormatJustifyJustify_triggered();
    void on_actionFormatIndent_triggered();
    void on_actionFormatOutdent_triggered();
    void on_actionFormatInsertOrderedList_triggered();
    void on_actionFormatInsertUnorderedList_triggered();
    void on_actionFormatInsertTable_triggered();
    void on_actionFormatInsertLink_triggered();
    void on_actionFormatForeColor_triggered();
    void on_actionFormatBold_triggered();
    void on_actionFormatItalic_triggered();
    void on_actionFormatUnderLine_triggered();
    void on_actionFormatStrikeThrough_triggered();
    void on_actionFormatInsertHorizontal_triggered();
    void on_actionFormatInsertDate_triggered();
    void on_actionFormatInsertTime_triggered();
    void on_actionFormatRemoveFormat_triggered();
    void on_actionFormatPlainText_triggered();
    void on_actionEditorViewSource_triggered();
    void on_actionFormatInsertCheckList_triggered();
    void on_actionFormatInsertCode_triggered();
    void on_actionFormatInsertImage_triggered();

    void on_searchProcess(const QString &strKeywords, const CWizDocumentDataArray& arrayDocument, bool bEnd);

    void on_actionGoBack_triggered();
    void on_actionGoForward_triggered();

    void on_category_itemSelectionChanged();
    void on_documents_itemSelectionChanged();
    void on_documents_itemDoubleClicked(QListWidgetItem * item);
    void on_message_itemSelectionChanged();
    void on_documents_documentCountChanged();
    void on_documents_lastDocumentDeleted();
    void on_documents_hintChanged(const QString& strHint);
    void on_documents_viewTypeChanged(int type);
    void on_documents_sortingTypeChanged(int type);
    //void on_document_contentChanged();

    void on_search_doSearch(const QString& keywords);
    void on_options_settingsChanged(WizOptionsType type);

    void on_syncLogined();
    void on_syncStarted(bool syncAll);
    void on_syncDone(int nErrorcode, const QString& strErrorMsg);
    void on_syncDone_userVerified();

    void on_syncProcessLog(const QString& strMsg);

    void on_options_restartForSettings();
    void locateDocument(const QString& strKbGuid, const QString& strGuid);

    void on_editor_statusChanged();

    void createDocumentByTemplate(const QString& strFile);

    //js environment func
    QString getSkinResourcePath() const;
    QString getUserAvatarFilePath(int size) const;
    QString getUserAlias() const;
    QString getFormatedDateTime() const;
    bool isPersonalDocument() const;
    QString getCurrentNoteHtml() const;
    void saveHtmlToCurrentNote(const QString& strHtml, const QString& strResource);
    bool hasEditPermissionOnCurrentNote() const;
    void setCurrentDocumentType(const QString& strType);
    void OpenURLInDefaultBrowser(const QString& strURL);
    void SetDialogResult(int nResult);

#ifndef Q_OS_MAC
    void on_actionPopupMainMenu_triggered();
    void on_client_splitterMoved(int pos, int index);
    void on_menuButtonClicked();
    void adjustToolBarLayout();
#endif

    void on_application_aboutToQuit();

    void on_checkUpgrade_finished(bool bUpgradeAvaliable);

#ifdef WIZ_OBOSOLETE
    void on_upgradeThread_finished();
#endif

    void on_trayIcon_newDocument_clicked();
    void on_hideTrayIcon_clicked();
    //
    void shiftVisableStatus();

public:
    // WizExplorerApp pointer
    virtual QWidget* mainWindow() { return this; }
    virtual QObject* object() { return this; }
    virtual CWizDatabaseManager& databaseManager() { return m_dbMgr; }
    virtual CWizCategoryBaseView& category() { return *m_category; }
    virtual CWizUserSettings& userSettings() { return *m_settings; }

    //WizExplorerApp API:
    QObject* Window() { return this; }
    Q_PROPERTY(QObject* Window READ Window)

    QObject* CategoryCtrl() { return m_category; }
    Q_PROPERTY(QObject* CategoryCtrl READ CategoryCtrl)

    QObject* DocumentsCtrl();
    Q_PROPERTY(QObject* DocumentsCtrl READ DocumentsCtrl)

    QObject* DatabaseManager();
    Q_PROPERTY(QObject* DatabaseManager READ DatabaseManager)

    Q_INVOKABLE QObject* CreateWizObject(const QString& strObjectID);
    Q_INVOKABLE void SetSavingDocument(bool saving);
    Q_INVOKABLE void ProcessClipboardBeforePaste(const QVariantMap& data);

private:
    void syncAllData();

    //FIXME：新建笔记时,为了将光标移到编辑器中,需要将Editor的模式设置为disable,此处需要将actions设置为可用
    void setActionsEnableForNewNote();
    void setFocusForNewNote(WIZDOCUMENTDATA doc);
    //
    void initTrayIcon(QSystemTrayIcon* trayIcon);
    //
    void startSearchStatus();
    void cancleSearchStatus();

    //
    void initVariableBeforCreateNote();
};

} // namespace Internal
} // namespace Core

#endif // WIZMAINWINDOW_H
