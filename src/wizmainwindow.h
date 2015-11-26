#ifndef WIZMAINWINDOW_H
#define WIZMAINWINDOW_H

#include <QtGlobal>
#include <QMainWindow>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <memory>

#include "wizdef.h"
#include "share/wizuihelper.h"
#include "share/wizsettings.h"
#ifndef Q_OS_MAC
#include "share/wizshadowwindow.h"
#endif


#define WIZ_SINGLE_APPLICATION "WIZ_SINGLE_APPLICATION"

class QToolBar;
class QLabel;
class QSystemTrayIcon;
class QComboBox;
class QActionGroup;

class CWizProgressDialog;
class CWizDocumentListView;
class CWizDocumentSelectionView;
class CWizDocumentTransitionView;
class CWizActions;
class CWizDocumentViewHistory;
class CWizFixedSpacer;
class CWizMacFixedSpacer;
class CWizSplitter;
class CWizAnimateAction;
class CWizOptionsWidget;
class CWizIAPDialog;

class CWizSearchWidget;
class CWizSearcher;
class CWizSearchIndexer;

class QtSegmentControl;
class CWizObjectDataDownloaderHost;
class CWizUserAvatarDownloaderHost;
class CWizKMSyncThread;
class CWizUserVerifyDialog;
class wizImageButton;

class CWizMacToolBar;
class QNetworkDiskCache;
class CWizConsoleDialog;
class CWizUpgrade;
class CWizCategoryView;
class QListWidgetItem;
class CWizCategoryViewMessageItem;
class CWizCategoryViewShortcutItem;
class CWizDocumentWebView;
class CWizTrayIcon;
class CWizMobileFileReceiver;

namespace WizService {
namespace Internal {
class MessageListView;
class WizMessageSelector;
class WizMessageListTitleBar;
}
}

namespace Core {
class ICore;
class CWizDocumentView;
class CWizSingleDocumentViewDelegate;

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

    CWizSearcher* searcher();
    QString searchKeywords() const { return m_strSearchKeywords; }
    void rebuildFTS();

    static MainWindow* instance();

    QNetworkDiskCache* webViewNetworkCache();
    CWizDocumentView* docView();

protected:
    bool eventFilter(QObject* watched, QEvent* event);
    void resizeEvent(QResizeEvent* event);
    void closeEvent(QCloseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void changeEvent(QEvent *event);
    void moveEvent(QMoveEvent* ev);

#ifdef Q_OS_MAC
    virtual void paintEvent(QPaintEvent* event);
#endif

#ifdef USECOCOATOOLBAR
    virtual void showEvent(QShowEvent *event);
#endif

private:
    ICore* m_core;
    CWizDatabaseManager& m_dbMgr;
    CWizProgressDialog* m_progress;
    CWizUserSettings* m_settings;
    CWizKMSyncThread* m_sync;
    CWizUserVerifyDialog* m_userVerifyDialog;
    CWizConsoleDialog* m_console;
    CWizUpgrade* m_upgrade;
    CWizIAPDialog* m_iapDialog;

    CWizObjectDataDownloaderHost* m_objectDownloaderHost;
    //
    CWizTrayIcon* m_tray;
    QMenu* m_trayMenu;

#ifdef USECOCOATOOLBAR
    CWizMacToolBar* m_toolBar;
    CWizMacFixedSpacer* m_spacerForToolButtonAdjust;
#else
    QToolBar* m_toolBar;
    CWizFixedSpacer* m_spacerForToolButtonAdjust;
#endif

    QMenuBar* m_menuBar;
    QMenu* m_dockMenu;
    QMenu* m_windowListMenu;
    QActionGroup* m_viewTypeActions;
    QActionGroup* m_sortTypeActions;
#ifdef Q_OS_LINUX
    QMenu* m_menu;
    QToolButton* m_menuButton;    
#endif
    bool m_useSystemBasedStyle;

    QWidget* m_clienWgt;


#ifndef Q_OS_MAC
    QLabel* m_labelNotice;
    QAction* m_optionsAction;
#endif

    CWizActions* m_actions;
    CWizCategoryView* m_category;
    CWizDocumentListView* m_documents;
    WizService::Internal::MessageListView* m_msgList;
    QWidget* m_noteListWidget;
    QWidget* m_msgListWidget;
    WizService::Internal::WizMessageListTitleBar* m_msgListTitleBar;

    CWizDocumentSelectionView* m_documentSelection;
    CWizDocumentView* m_doc;
    std::shared_ptr<CWizSplitter> m_splitter;
    QWidget* m_docListContainer;
    CWizSingleDocumentViewDelegate* m_singleViewDelegate;

    QLabel* m_labelDocumentsHint;
//    QLabel* m_labelDocumentsCount;
    wizImageButton* m_btnMarkDocumentsReaded;

    CWizDocumentViewHistory* m_history;
    CWizAnimateAction* m_animateSync;

    CWizSearcher* m_searcher;
    QString m_strSearchKeywords;

    CWizSearchIndexer* m_searchIndexer;
    CWizSearchWidget* m_searchWidget;

    CWizMobileFileReceiver *m_mobileFileReceiver;

    bool m_bRestart;
    bool m_bLogoutRestart;
    bool m_bUpdatingSelection;

    bool m_bQuickDownloadMessageEnable;

    WIZDOCUMENTDATA m_documentForEditing;

private:
    void initActions();

    void initToolBar();
    void initClient();
    //
#ifndef Q_OS_MAC
    virtual void layoutTitleBar();
    void initMenuList();
#endif
    void initMenuBar();
    void initDockMenu();

    QWidget* createNoteListView();
    QWidget* createMessageListView();

public:
    // CWizDocument passthrough methods
    QSize clientSize() const { return m_splitter->widget(2)->size(); }
    QWidget* client() const;
    void showClient(bool visible) const;
    CWizDocumentView* documentView() const;

    CWizActions* actions() const { return m_actions; }
    //CWizDownloadObjectDataDialog* objectDownloadDialog() const { return m_objectDownloadDialog; }
    CWizObjectDataDownloaderHost* downloaderHost() const { return m_objectDownloaderHost; }
    CWizProgressDialog* progressDialog() const { return m_progress; }
    CWizIAPDialog* iapDialog();

    void resetPermission(const QString& strKbGUID, const QString& strDocumentOwner);
    void viewDocument(const WIZDOCUMENTDATA& data, bool addToHistory);  
    //
    static void quickSyncKb(const QString& kbGuid);

    void checkWizUpdate();
    void setSystemTrayIconVisible(bool bVisible);
    void setMobileFileReceiverEnable(bool bEnable);
    //
    void viewDocumentByWizKMURL(const QString& strKMURL);
    void viewAttachmentByWizKMURL(const QString& strKbGUID, const QString& strKMURL);
    //
    void createNoteWithAttachments(const QStringList& strAttachList);
    void createNoteWithText(const QString& strText);
    void createNoteWithImage(const QString& strImageFile);

signals:    
    void documentsViewTypeChanged(int);
    void documentsSortTypeChanged(int);

public Q_SLOTS:
    void on_actionExit_triggered();
    void on_actionClose_triggered();
    void on_actionConsole_triggered();
    void on_actionAutoSync_triggered();
    void on_actionSync_triggered();
    void on_actionNewNote_triggered();
    void on_actionNewNoteByTemplate_triggered();
    void on_actionLogout_triggered();
    void on_actionAbout_triggered();
    void on_actionDeveloper_triggered();
    void on_actionPreference_triggered();
    void on_actionRebuildFTS_triggered();
    void on_actionFeedback_triggered();
    void on_actionSupport_triggered();
    void on_actionManual_triggered();
    void on_actionSearch_triggered();
    void on_actionResetSearch_triggered();
    void on_actionAdvancedSearch_triggered();
    void on_actionAddCustomSearch_triggered();
    void on_actionFindReplace_triggered();
    void on_actionSaveAsPDF_triggered();
    void on_actionSaveAsHtml_triggered();
    void on_actionImportFile_triggered();
    void on_actionPrint_triggered();
    void on_actionPrintMargin_triggered();

    // menu editing
    void on_actionEditingUndo_triggered();
    void on_actionEditingRedo_triggered();

    void on_actionEditingCut_triggered();
    void on_actionEditingCopy_triggered();
    void on_actionEditingPaste_triggered();
    void on_actionEditingPastePlain_triggered();
    void on_actionEditingDelete_triggered();
    void on_actionEditingSelectAll_triggered();

#ifdef USEWEBENGINE
    //move input position
    void on_actionMoveToPageStart_triggered();
    void on_actionMoveToPageEnd_triggered();
    void on_actionMoveToLineStart_triggered();
    void on_actionMoveToLineEnd_triggered();
#endif

    // menu view
    void on_actionViewToggleCategory_triggered();
    void on_actionViewToggleFullscreen_triggered();
    void on_actionViewMinimize_triggered();
    void on_actionZoom_triggered();
    void on_actionBringFront_triggered();

    void on_actionCategoryMessageCenter_triggered();
    void on_actionCategoryShortcuts_triggered();
    void on_actionCategoryQuickSearch_triggered();
    void on_actionCategoryFolders_triggered();
    void on_actionCategoryTags_triggered();
    void on_actionCategoryBizGroups_triggered();
    void on_actionCategoryPersonalGroups_triggered();
    void on_actionThumbnailView_triggered();
    void on_actionTwoLineView_triggered();
    void on_actionOneLineView_triggered();
    void on_actionSortByCreatedTime_triggered();
    void on_actionSortByUpdatedTime_triggered();
    void on_actionSortByAccessTime_triggered();
    void on_actionSortByTitle_triggered();
    void on_actionSortByFolder_triggered();
    void on_actionSortByTag_triggered();
    void on_actionSortBySize_triggered();

    void on_categoryUnreadButton_triggered();

    void on_actionMarkAllMessageRead_triggered(bool removeItems);
    void on_messageSelector_senderSelected(QString userGUID);

    // menu format
    void on_actionMenuFormatJustifyLeft_triggered();
    void on_actionMenuFormatJustifyRight_triggered();
    void on_actionMenuFormatJustifyCenter_triggered();
    void on_actionMenuFormatJustifyJustify_triggered();
    void on_actionMenuFormatIndent_triggered();
    void on_actionMenuFormatOutdent_triggered();
    void on_actionMenuFormatInsertOrderedList_triggered();
    void on_actionMenuFormatInsertUnorderedList_triggered();
    void on_actionMenuFormatInsertTable_triggered();
    void on_actionMenuFormatInsertLink_triggered();
    void on_actionMenuFormatBold_triggered();
    void on_actionMenuFormatItalic_triggered();
    void on_actionMenuFormatUnderLine_triggered();
    void on_actionMenuFormatStrikeThrough_triggered();
    void on_actionMenuFormatInsertHorizontal_triggered();
    void on_actionMenuFormatInsertDate_triggered();
    void on_actionMenuFormatInsertTime_triggered();
    void on_actionMenuFormatRemoveFormat_triggered();
    void on_actionMenuFormatPlainText_triggered();
    void on_actionMenuEditorViewSource_triggered();
    void on_actionMenuFormatInsertCheckList_triggered();
    void on_actionMenuFormatInsertCode_triggered();
    void on_actionMenuFormatInsertImage_triggered();
    void on_actionMenuFormatScreenShot_triggered();

    void on_searchProcess(const QString &strKeywords, const CWizDocumentDataArray& arrayDocument,
                          bool bStart, bool bEnd);

    void on_actionGoBack_triggered();
    void on_actionGoForward_triggered();

    void on_category_itemSelectionChanged();
    void on_documents_itemSelectionChanged();
    void on_documents_itemDoubleClicked(QListWidgetItem * item);
    void on_documents_lastDocumentDeleted();
//    void on_documents_documentCountChanged();
//    void on_documents_hintChanged(const QString& strHint);
    void on_btnMarkDocumentsRead_triggered();
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
    void on_promptMessage_request(int nType, const QString& strTitle, const QString& strMsg);
    void on_bubbleNotification_request(const QVariant& param);

    void on_TokenAcquired(const QString& strToken);

    void on_quickSync_request(const QString& strKbGUID);

    void on_options_restartForSettings();

    void on_editor_statusChanged();

    void createDocumentByTemplate(const QString& strFile);

    void on_mobileFileRecived(const QString& strFile);

    //
    void on_shareDocumentByLink_request(const QString& strKbGUID, const QString& strGUID);

#ifndef Q_OS_MAC
    void on_actionPopupMainMenu_triggered();
    void on_menuButtonClicked();
#endif
    void adjustToolBarLayout();
    void on_client_splitterMoved(int pos, int index);

    void on_application_aboutToQuit();
    void on_application_messageAvailable(const QString& strMsg);

    void on_checkUpgrade_finished(bool bUpgradeAvaliable);

#ifdef WIZ_OBOSOLETE
    void on_upgradeThread_finished();
#endif

    //
    void on_trayIcon_newDocument_clicked();
    void on_hideTrayIcon_clicked();
    void on_trayIcon_actived(QSystemTrayIcon::ActivationReason reason);
    void showBubbleNotification(const QString& strTitle, const QString& strInfo);
    void showTrayIconMenu();
    void on_viewMessage_request(qint64 messageID);
    void on_viewMessage_request(const WIZMESSAGEDATA& msg);
    //
    void on_dockMenuAction_triggered();
    //
    void shiftVisableStatus();

    //
    void showNewFeatureGuide();
    void showMobileFileReceiverUserGuide();
    void setDoNotShowMobileFileReceiverUserGuideAgain(bool bNotAgain);

    //
    void locateDocument(const WIZDOCUMENTDATA& data);
    void locateDocument(const QString& strKbGuid, const QString& strGuid);

    //
    void viewNoteInSeparateWindow(const WIZDOCUMENTDATA& data);
    void viewCurrentNoteInSeparateWindow();

public:
    // WizExplorerApp pointer
    virtual QWidget* mainWindow();
    virtual QObject* object();
    virtual CWizDatabaseManager& databaseManager() { return m_dbMgr; }
    virtual CWizCategoryBaseView& category();
    virtual CWizUserSettings& userSettings();

    //WizExplorerApp API:
    QObject* Window() { return this; }
    Q_PROPERTY(QObject* Window READ Window)

    QObject* CategoryCtrl();
    Q_PROPERTY(QObject* CategoryCtrl READ CategoryCtrl)

    QObject* DocumentsCtrl();
    Q_PROPERTY(QObject* DocumentsCtrl READ DocumentsCtrl)

    QObject* DatabaseManager();
    Q_PROPERTY(QObject* DatabaseManager READ DatabaseManager)

    Q_INVOKABLE QObject* CreateWizObject(const QString& strObjectID);
    Q_INVOKABLE void SetSavingDocument(bool saving);
    Q_INVOKABLE void ProcessClipboardBeforePaste(const QVariantMap& data);


    //NOTE: these functions would called by web page, do not delete
    Q_INVOKABLE QString TranslateString(const QString& string);
    Q_INVOKABLE void OpenURLInDefaultBrowser(const QString& strUrl);
    Q_INVOKABLE void GetToken(const QString& strFunctionName);
    Q_INVOKABLE void SetDialogResult(int nResult);
    Q_INVOKABLE void AppStoreIAP();

private:
    void syncAllData();
    void reconnectServer();

    void setFocusForNewNote(WIZDOCUMENTDATA doc);
    //
    void initTrayIcon(QSystemTrayIcon* trayIcon);

#ifdef Q_OS_LINUX
    void setWindowStyleForLinux(bool bUseSystemStyle);
#endif
    //
    void startSearchStatus();
    void quitSearchStatus();

    //
    void initVariableBeforCreateNote();

    //
    bool needShowNewFeatureGuide();
    //
    void resortDocListAfterViewDocument(const WIZDOCUMENTDATA& doc);

    //
    void showCommentWidget();

    //
    CWizDocumentWebView* getActiveEditor();
    //
    void showDocumentList();
    void showDocumentList(CWizCategoryBaseView* category);
    void showMessageList(CWizCategoryViewMessageItem* pItem);
    void viewDocumentByShortcut(CWizCategoryViewShortcutItem *pShortcut);
    void searchNotesBySQL(const QString& strSQLWhere);
    void searchNotesBySQLAndKeyword(const QString& strSQLWhere, const QString& strKeyword, int searchScope);
    //
    void updateHistoryButtonStatus();
    //
    void openAttachment(const WIZDOCUMENTATTACHMENTDATA& attachment, const QString& strFileName);
    void downloadAttachment(const WIZDOCUMENTATTACHMENTDATA& attachment);

    void openVipPageInWebBrowser();

    //
    void loadMessageByUserGuid(const QString& guid);

    void resetWindowListMenu(QMenu* menu, bool removeExists);

    void changeDocumentsSortTypeByAction(QAction* action);

    //
    bool processApplicationActiveEvent();

private slots:
    void windowActived();
    //
    void resetDockMenu();
    void resetWindowMenu();
    void removeWindowsMenuItem(QString guid);
};

} // namespace Internal
} // namespace Core

#endif // WIZMAINWINDOW_H
