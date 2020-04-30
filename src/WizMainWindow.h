#ifndef WIZMAINWINDOW_H
#define WIZMAINWINDOW_H

#include <QtGlobal>
#include <QMainWindow>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <memory>

#include "WizDef.h"
#include "share/WizUIHelper.h"
#include "share/WizSettings.h"
#ifndef Q_OS_MAC
#include "share/WizShadowWindow.h"
#endif


#define WIZ_SINGLE_APPLICATION "WIZ_SINGLE_APPLICATION"

class QToolBar;
class QLabel;
class QSystemTrayIcon;
class QComboBox;
class QActionGroup;
class QStackedWidget;
struct TemplateData;

class WizProgressDialog;
class WizDocumentListView;
class WizDocumentSelectionView;
class WizDocumentTransitionView;
class WizActions;
class WizDocumentViewHistory;
class WizFixedSpacer;
class WizMacFixedSpacer;
class WizSplitter;
class WizAnimateAction;
class WizOptionsWidget;
class WizIAPDialog;
class WizTemplatePurchaseDialog;
class WizWebEngineView;

class WizSearchView;
class WizSearcher;
class WizUserInfoWidget;

class QtSegmentControl;
class WizObjectDownloaderHost;
class CWizUserAvatarDownloaderHost;
class WizKMSyncThread;
class WizUserVerifyDialog;
class WizImageButton;

class WizMacToolBar;
class QNetworkDiskCache;
class WizConsoleDialog;
class WizUpgradeChecker;
class WizCategoryView;
class QListWidgetItem;
class WizCategoryViewMessageItem;
class WizCategoryViewShortcutItem;
class WizDocumentWebView;
class WizTrayIcon;
class WizMobileFileReceiver;
class ICore;

class WizMessageListView;
class WizMessageSelector;
class WizMessageListTitleBar;

class WizDocumentView;
class WizSingleDocumentViewDelegate;
class QWebEngineView;

#ifdef Q_OS_MAC
class WizMacToolBarButtonItem;
#endif

class WizMainWindow
#ifdef Q_OS_MAC
    : public QMainWindow
#else
    : public WizShadowWindow<QMainWindow>
#endif
    , public WizExplorerApp
{
    Q_OBJECT

#ifdef Q_OS_MAC
    typedef QMainWindow  _baseClass;
#else
    typedef WizShadowWindow<QMainWindow> _baseClass;
#endif

public:
    explicit WizMainWindow(WizDatabaseManager& dbMgr, QWidget *parent = 0);
    virtual void init();

    void saveStatus();
    void restoreStatus();
    ~WizMainWindow();

    void cleanOnQuit();

    void setRestart(bool b) { m_bRestart = b; }
    bool isRestart() const { return m_bRestart; }
    Q_PROPERTY(bool restart READ isRestart WRITE setRestart)

    bool isLogout() const { return m_bLogoutRestart; }

    WizSearcher* searcher();
    QString searchKeywords() const { return m_strSearchKeywords; }

    static WizMainWindow* instance();

    QNetworkDiskCache* webViewNetworkCache();
    WizDocumentView* docView();
    //
    void trySaveCurrentNote(std::function<void(const QVariant &)> callback);

protected:
    bool eventFilter(QObject* watched, QEvent* event);
    void resizeEvent(QResizeEvent* event);
    void closeEvent(QCloseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void changeEvent(QEvent *event);
    void moveEvent(QMoveEvent* ev);
    void keyPressEvent(QKeyEvent* ev);

#ifdef USECOCOATOOLBAR
    virtual void showEvent(QShowEvent *event);
#endif

private:
    WizDatabaseManager& m_dbMgr;
    WizUserSettings* m_settings;
    WizProgressDialog* m_progress;
    WizKMSyncThread* m_syncFull;
    WizKMSyncThread* m_syncQuick;
    WizUserVerifyDialog* m_userVerifyDialog;
    WizConsoleDialog* m_console;
    WizUpgradeChecker* m_upgrade;
    WizIAPDialog* m_iapDialog;
    WizTemplatePurchaseDialog* m_templateIAPDialog;
    //
    bool m_quiting;

    //
    WizTrayIcon* m_tray;
    QMenu* m_trayMenu;

#ifdef USECOCOATOOLBAR
    WizMacToolBar* m_toolBar;
    WizMacFixedSpacer* m_spacerForToolButtonAdjust;
    WizMacToolBarButtonItem* m_newNoteButton;
#else
    QToolBar* m_toolBar;
    WizFixedSpacer* m_spacerForToolButtonAdjust;
#endif

    QMenuBar* m_menuBar;
    QMenu* m_dockMenu;
    QMenu* m_windowListMenu;
    QMenu* m_newNoteExtraMenu;
    QActionGroup* m_viewTypeActions;
    QActionGroup* m_sortTypeActions;
#ifndef Q_OS_MAC
    QMenu* m_menu;
    QToolButton* m_menuButton;    
#endif
    bool m_useSystemBasedStyle;

    QWidget* m_clienWgt;


#ifndef Q_OS_MAC
    QLabel* m_labelNotice;
    QAction* m_optionsAction;
#endif

    WizActions* m_actions;
    WizCategoryView* m_category;
    WizDocumentListView* m_documents;
    WizMessageListView* m_msgList;
    QWidget* m_noteListWidget;
    QWidget* m_msgListWidget;
    WizMessageListTitleBar* m_msgListTitleBar;

    WizDocumentSelectionView* m_documentSelection;
    WizDocumentView* m_doc;
    WizSplitter* m_splitter;
    WizSplitter* m_subSplitter;
    QStackedWidget* m_subContainer;
    WizWebEngineView* m_mainWebView;
    QWidget* m_docListContainer;
    WizSingleDocumentViewDelegate* m_singleViewDelegate;

    QLabel* m_labelDocumentsHint;
//    QLabel* m_labelDocumentsCount;
    WizImageButton* m_btnMarkDocumentsReaded;

    WizDocumentViewHistory* m_history;
    WizAnimateAction* m_animateSync;

    WizSearcher* m_searcher;
    QString m_strSearchKeywords;

    WizSearchView* m_searchWidget;
    WizUserInfoWidget* m_userInfoWidget;

    WizMobileFileReceiver *m_mobileFileReceiver;    

    bool m_bRestart;
    bool m_bLogoutRestart;
    bool m_bUpdatingSelection;

    bool m_bQuickDownloadMessageEnable;
    //
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
    //
    void promptServiceExpr(bool free, WIZGROUPDATA group);

public:
    // CWizDocument passthrough methods
    QSize clientSize() const { return m_splitter->widget(2)->size(); }
    QWidget* client() const;
    WizDocumentView* documentView() const;
    WizDocumentListView* documentList() const { return m_documents; }
    WizKMSyncThread* fullSync() const { return m_syncFull; }
    WizKMSyncThread* quickSync() const { return m_syncQuick; }
    void quickSyncKb(const QString& kbGuid);
    void setNeedResetGroups();

    WizActions* actions() const { return m_actions; }

    //FIXME: why provide download host and dialog by mainwidnow ???
    WizObjectDownloaderHost* downloaderHost() const;
    WizProgressDialog* progressDialog() const { return m_progress; }
    WizIAPDialog* iapDialog();

    void resetPermission(const QString& strKbGUID, const QString& strDocumentOwner);
    void viewDocument(const WIZDOCUMENTDATAEX& data, bool addToHistory);
    //
    void checkWizUpdate();
    void setSystemTrayIconVisible(bool bVisible);
    void setMobileFileReceiverEnable(bool bEnable);
    //
    void viewDocumentByWizKMURL(const QString& strKMURL);
    void viewAttachmentByWizKMURL(const QString& strKbGUID, const QString& strKMURL);
    //
    void createNoteWithAttachments(const QStringList& strAttachList);
    void createNoteWithText(const QString& strText);

    void createNoteByTemplateCore(const TemplateData& tmplData);
    //
    void refreshAd();
    //
    void showTrash();
    void showSharedNotes(); 
    //
    void processCategoryItemChanged();
    //
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
    void on_actionPreference_triggered();
    void on_actionFeedback_triggered();
    void on_actionSupport_triggered();
    void on_actionManual_triggered();
    void on_actionSearch_triggered();
    void on_actionResetSearch_triggered();
    void on_actionFindReplace_triggered();
    void on_actionSaveAsPDF_triggered();
    void on_actionSaveAsHtml_triggered();
    void on_actionSaveAsMarkdown_triggered();
    void on_actionImportFile_triggered();
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

    // menu view
    void on_actionViewToggleCategory_triggered();
    void on_actionViewShowSubFolderDocuments_triggered();
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
    void on_actionSearchResultView_triggered();
    void on_actionTwoLineView_triggered();
    void on_actionOneLineView_triggered();
    void on_actionSortByCreatedTime_triggered();
    void on_actionSortByUpdatedTime_triggered();
    void on_actionSortByAccessTime_triggered();
    void on_actionSortByTitle_triggered();
    void on_actionSortByFolder_triggered();
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
    void on_actionMenuFormatInsertLink_triggered();
    void on_actionMenuFormatBold_triggered();
    void on_actionMenuFormatItalic_triggered();
    void on_actionMenuFormatUnderLine_triggered();
    void on_actionMenuFormatStrikeThrough_triggered();
    void on_actionMenuFormatInsertHorizontal_triggered();
    void on_actionMenuFormatInsertDate_triggered();
    void on_actionMenuFormatInsertTime_triggered();
    void on_actionMenuFormatRemoveFormat_triggered();
    void on_actionMenuFormatInsertCheckList_triggered();
    void on_actionMenuFormatInsertCode_triggered();
    void on_actionMenuFormatInsertImage_triggered();
    void on_actionMenuFormatScreenShot_triggered();
    void on_actionMenuFormatInsertTable(int row, int col);

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
    void on_syncDone(int nErrorcode, bool isNetworkError, const QString& strErrorMsg, bool isBackground);
    void on_syncDone_userVerified();

    void on_syncProcessLog(const QString& strMsg);
    void on_promptMessage_request(int nType, const QString& strTitle, const QString& strMsg);
    void on_promptFreeServiceExpr(WIZGROUPDATA group);
    void on_promptVipServiceExpr(WIZGROUPDATA group);
    void on_bubbleNotification_request(const QVariant& param);

    void on_TokenAcquired(const QString& strToken);

    void on_quickSync_request(const QString& strKbGUID);

    void on_options_restartForSettings();

    void on_editor_statusChanged(const QString& currentStyle);

    void createNoteByTemplate(const TemplateData& tmplData);

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
    void on_viewMessage_requestNormal(QVariant messageData);
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
    void titleChanged();

    //
    void viewNoteInSeparateWindow(const WIZDOCUMENTDATA& data);
    void viewCurrentNoteInSeparateWindow();
    //
    void onAttachmentModified(QString strKbGUID, QString strGUID,QString strFileName,
                              QString strMD5, QDateTime dtLastModified);

public:
    // WizExplorerApp pointer
    virtual QWidget* mainWindow();
    virtual QObject* object();
    virtual WizDatabaseManager& databaseManager() { return m_dbMgr; }
    virtual WizCategoryBaseView& category();
    virtual WizUserSettings& userSettings();

    //WizExplorerApp API:
    QObject* Window() { return this; }
    Q_PROPERTY(QObject* Window READ Window)

    QObject* CategoryCtrl();
    Q_PROPERTY(QObject* CategoryCtrl READ CategoryCtrl)

    QObject* DocumentsCtrl();
    Q_PROPERTY(QObject* DocumentsCtrl READ DocumentsCtrl)

    QObject* DatabaseManager();
    Q_PROPERTY(QObject* DatabaseManager READ DatabaseManager)

    QObject* CurrentDocumentBrowserObject();
    Q_PROPERTY(QObject* CurrentDocumentBrowserObject READ CurrentDocumentBrowserObject)


    Q_INVOKABLE QObject* CreateWizObject(const QString& strObjectID);
    Q_INVOKABLE void SetSavingDocument(bool saving);
    Q_INVOKABLE void ProcessClipboardBeforePaste(const QVariantMap& data);


    //NOTE: these functions would called by web page, do not delete
    Q_INVOKABLE QString TranslateString(const QString& string);
    Q_INVOKABLE void OpenURLInDefaultBrowser(const QString& strUrl);
    Q_INVOKABLE void GetToken(const QString& strFunctionName);
    Q_INVOKABLE void SetDialogResult(int nResult);
    Q_INVOKABLE void AppStoreIAP();
    Q_INVOKABLE void copyLink(const QString& link);
    Q_INVOKABLE void onClickedImage(const QString& src, const QString& list);

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
    void resetSearchStatus();

    //
    void initVariableBeforCreateNote();

    //
    bool needShowNewFeatureGuide();
    //
    void resortDocListAfterViewDocument(const WIZDOCUMENTDATA& doc);

    //
    void showCommentWidget();

    //
    WizDocumentWebView* getActiveEditor();
    //
    void showDocumentList();
    void showDocumentList(WizCategoryBaseView* category);
    void showMessageList(WizCategoryViewMessageItem* pItem);
    void viewDocumentByShortcut(WizCategoryViewShortcutItem *pShortcut);
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
    //
    void prepareNewNoteMenu();

private slots:
    void windowActived();
    //
    void resetDockMenu();
    void resetWindowMenu();
    void removeWindowsMenuItem(QString guid);

    void showVipUpgradePage();

#ifdef Q_OS_MAC
    void on_newNoteButton_extraMenuRequest();
#endif
    void on_newNoteByExtraMenu_request();

private:
    void showTemplateIAPDlg(const TemplateData& tmpl);
};


#endif // WIZMAINWINDOW_H
