#ifndef WIZMAINWINDOW_H
#define WIZMAINWINDOW_H

#include <QtGui>

#ifdef BUILD_WITH_QT5
#include <QtWidgets>
#endif

#include "wizdef.h"
#include "share/wizuihelper.h"
#include "share/wizsettings.h"
#include "share/wizsyncthread.h"
#include "wizUpgrade.h"
//#include "wizupdater.h"
#include "wizconsoledialog.h"
#include "wizdocumentview.h"
#include "wizcertmanager.h"
#include "wizusercipherform.h"
#include "wizdownloadobjectdatadialog.h"
#include "wizcategoryview.h"
#include "wizdocumentlistview.h"

class CWizDocumentListView;
class CWizDocumentView;
class CWizActions;
class CWizDocumentViewHistory;
class CWizFixedSpacer;
class CWizSplitter;
class CWizAnimateAction;
class CWizOptionsWidget;
class CWizStatusBar;

class CWizSearchBox;
class CWizSearchIndexerThread;


class MainWindow
    : public QMainWindow
    , public CWizExplorerApp
{
    Q_OBJECT

public:
    explicit MainWindow(CWizDatabaseManager& dbMgr, QWidget *parent = 0);
    virtual void init();

    void saveStatus();
    void restoreStatus();
    ~MainWindow();
    bool requestThreadsQuit();

    void setRestart(bool b) { m_bRestart = b; }
    bool isRestart() const { return m_bRestart; }
    Q_PROPERTY(bool restart READ isRestart WRITE setRestart)

    bool isLogout() const { return m_bLogoutRestart; }

protected:
    virtual void showEvent(QShowEvent* event);
    virtual void closeEvent(QCloseEvent* event);

private:
    CWizDatabaseManager& m_dbMgr;
    CWizUserSettings* m_settings;
    QPointer<CWizSyncThread> m_sync;
    QPointer<QTimer> m_syncTimer;
    CWizConsoleDialog* m_console;
    QPointer<CWizUpgrade> m_upgrade;
    //QPointer<CWizUpgradeThread> m_upgrade;
    QPointer<CWizCertManager> m_certManager;
    QPointer<CWizUserCipherForm> m_cipherForm;
    QPointer<CWizGroupAttributeForm> m_groupAttribute;
    QPointer<CWizDownloadObjectDataDialog> m_objectDownloadDialog;
    QToolBar* m_toolBar;
    QMenuBar* m_menuBar;
    CWizStatusBar* m_statusBar;
    QSystemTrayIcon *systemTray;

#ifndef Q_OS_MAC
    QLabel* m_labelNotice;
    QAction* m_optionsAction;
#endif

    CWizActions* m_actions;
    QPointer<CWizCategoryBaseView> m_category;
    QPointer<CWizCategoryView> m_categoryPrivate;
    QPointer<CWizCategoryTagsView> m_categoryTags;
    QPointer<CWizCategoryGroupsView> m_categoryGroups;
    QWidget* m_categoryLayer;
    CWizDocumentListView* m_documents;
    CWizDocumentView* m_doc;
    CWizSplitter* m_splitter;
    CWizOptionsWidget* m_options;

    CWizDocumentViewHistory* m_history;
    CWizAnimateAction* m_animateSync;

    QPointer<CWizSearchIndexerThread> m_searchIndexer;
    QPointer<CWizSearchBox> m_searchBox;

    bool m_bRestart;
    bool m_bLogoutRestart;
    bool m_bUpdatingSelection;

    WIZDOCUMENTDATA m_documentForEditing;

    QTimer m_timerQuit;
    bool m_bReadyQuit;
    // indicate close event is fired internal or external
    bool m_bRequestQuit;

private:
    void initActions();
    void initMenuBar();
    void initToolBar();
    void initClient();
    void initStatusBar();

public:
    // CWizDocument passthrough methods
    QWidget* client() const { return m_doc->client(); }
    CWizDocumentWebView* web() const { return m_doc->web(); }
    void showClient(bool visible) const { return m_doc->showClient(visible); }

    CWizUserCipherForm* cipherForm() const { return m_cipherForm; }
    CWizGroupAttributeForm* groupAttributeForm() { return m_groupAttribute; }
    CWizDownloadObjectDataDialog* objectDownloadDialog() const { return m_objectDownloadDialog; }

    void resetPermission(const QString& strKbGUID, const QString& strDocumentOwner);
    void viewDocument(const WIZDOCUMENTDATA& data, bool addToHistory);
    void locateDocument(const WIZDOCUMENTDATA& data);

#ifndef Q_OS_MAC
    CWizFixedSpacer* findFixedSpacer(int index);
    void adjustToolBarSpacerToPos(int index, int pos);
#endif

public Q_SLOTS:
    void on_actionExit_triggered();
    void on_actionConsole_triggered();
    void on_actionSync_triggered();
    void on_actionNewNote_triggered();
    //void on_actionDeleteCurrentNote_triggered();
    void on_actionLogout_triggered();
    void on_actionAbout_triggered();
    void on_actionPreference_triggered();
    void on_actionRebuildFTS_triggered();
    void on_actionSearch_triggered();
    void on_actionResetSearch_triggered();
    void systemTrayActivated(QSystemTrayIcon::ActivationReason);

    // menu editing
    void on_actionEditingUndo_triggered();
    void on_actionEditingRedo_triggered();

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
    void on_actionFormatBold_triggered();
    void on_actionFormatItalic_triggered();
    void on_actionFormatUnderLine_triggered();
    void on_actionFormatStrikeThrough_triggered();
    void on_actionFormatInsertHorizontal_triggered();
    void on_actionFormatInsertDate_triggered();
    void on_actionFormatInsertTime_triggered();
    void on_actionFormatRemoveFormat_triggered();

    void on_searchIndexerStarted();
    void on_searchDocumentFind(const CWizDocumentDataArray& arrayDocument);

    void on_actionGoBack_triggered();
    void on_actionGoForward_triggered();

    void on_actionCategorySwitchPrivate_triggered();
    void on_actionCategorySwitchTags_triggered();
    void on_actionCategorySwitchGroups_triggered();
    void on_actionCategorySwitchPrivate_triggered2(bool toggled);
    void on_actionCategorySwitchTags_triggered2(bool toggled);
    void on_actionCategorySwitchGroups_triggered2(bool toggled);
    void categorySwitchTo(CWizCategoryBaseView* sourceCategory, CWizCategoryBaseView* destCategory);
    void onAnimationCategorySwitchStateChanged(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);

    void on_category_itemSelectionChanged();
    void on_documents_itemSelectionChanged();

    void on_search_doSearch(const QString& keywords);
    void on_options_settingsChanged(WizOptionsType type);

    void on_syncStarted();
    void on_syncLogined();
    void on_syncDone(bool error);
    void on_syncProcessLog(const QString& strMsg);
    void on_syncProcessDebugLog(const QString& strMsg);
    void on_syncProcessErrorLog(const QString& strMsg);

    void on_options_restartForSettings();

#ifndef Q_OS_MAC
    void on_actionPopupMainMenu_triggered();
    void on_client_splitterMoved(int pos, int index);
#endif

    void on_quitTimeout();

    void on_checkUpgrade_finished(bool bUpgradeAvaliable);

#ifdef WIZ_OBOSOLETE
    void on_upgradeThread_finished();
#endif

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

    QObject* DocumentsCtrl() { return m_documents; }
    Q_PROPERTY(QObject* DocumentsCtrl READ DocumentsCtrl)

    QObject* DatabaseManager() { return &m_dbMgr; }
    Q_PROPERTY(QObject* DatabaseManager READ DatabaseManager)

    Q_INVOKABLE QObject* CreateWizObject(const QString& strObjectID);
    Q_INVOKABLE void SetDocumentModified(bool modified);
    Q_INVOKABLE void SetSavingDocument(bool saving);

};

#endif // WIZMAINWINDOW_H
