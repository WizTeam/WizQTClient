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
#include "wizupdater.h"
#include "wizconsoledialog.h"
#include "wizdocumentview.h"
#include "wizcertmanager.h"
#include "wizusercipherform.h"
#include "wizdownloadobjectdatadialog.h"
#include "wizcategoryview.h"
#include "wizdocumentlistview.h"

class CWizCategoryView;
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
    explicit MainWindow(CWizDatabase& db, QWidget *parent = 0);
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
    CWizDatabase& m_db;
    CWizUserSettings* m_settings;
    QPointer<CWizSyncThread> m_sync;
    QPointer<QTimer> m_syncTimer;
    CWizConsoleDialog* m_console;
    QPointer<CWizUpgradeThread> m_upgrade;
    QPointer<CWizCertManager> m_certManager;
    QPointer<CWizUserCipherForm> m_cipherForm;
    QPointer<CWizDownloadObjectDataDialog> m_objectDownloadDialog;
    QToolBar* m_toolBar;
    QMenuBar* m_menuBar;
    CWizStatusBar* m_statusBar;

#ifndef Q_OS_MAC
    QLabel* m_labelNotice;
    QAction* m_optionsAction;
#endif

    CWizActions* m_actions;
    CWizCategoryView* m_category;
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
    void showClient(bool visible) const { return m_doc->showClient(visible); }

    CWizUserCipherForm* cipherForm() const { return m_cipherForm; }
    CWizDownloadObjectDataDialog* objectDownloadDialog() const { return m_objectDownloadDialog; }

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
    void on_actionDeleteCurrentNote_triggered();
    void on_actionLogout_triggered();
    void on_actionAbout_triggered();
    void on_actionPreference_triggered();
    void on_actionRebuildFTS_triggered();
    void on_actionSearch_triggered();
    void on_actionResetSearch_triggered();
    void on_searchDocumentFind(const CWizDocumentDataArray& arrayDocument);

    void on_actionGoBack_triggered();
    void on_actionGoForward_triggered();

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

    void on_upgradeThread_finished();

public:
    // WizExplorerApp pointer
    virtual QWidget* mainWindow() { return this; }
    virtual QObject* object() { return this; }
    virtual CWizDatabase& database() { return m_db; }
    virtual CWizCategoryView& category() { return *m_category; }
    virtual CWizUserSettings& userSettings() { return *m_settings; }

    //WizExplorerApp API:
    QObject* Window() { return this; }
    Q_PROPERTY(QObject* Window READ Window)

    QObject* CategoryCtrl() { return m_category; }
    Q_PROPERTY(QObject* CategoryCtrl READ CategoryCtrl)

    QObject* DocumentsCtrl() { return m_documents; }
    Q_PROPERTY(QObject* DocumentsCtrl READ DocumentsCtrl)

    QObject* Database() { return &m_db; }
    Q_PROPERTY(QObject* Database READ Database)

    Q_INVOKABLE QObject* CreateWizObject(const QString& strObjectID);
    Q_INVOKABLE void SetDocumentModified(bool modified);
    Q_INVOKABLE void SetSavingDocument(bool saving);

};

#endif // WIZMAINWINDOW_H
