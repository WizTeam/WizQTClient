#ifndef WIZMAINWINDOW_H
#define WIZMAINWINDOW_H

#include <QtGui>

#include "wizdef.h"

#include "share/wizsettings.h"
#include "share/wizsyncthread.h"

#include "wizupdater.h"
#include "wizconsoledialog.h"

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


class MainWindow
    : public QMainWindow
    , public CWizExplorerApp
{
    Q_OBJECT

public:
    explicit MainWindow(CWizDatabase& db, QWidget *parent = 0);
    virtual void init();

    void center();
    ~MainWindow();

    void setRestart(bool b) { m_bRestart = b; }
    bool isRestart() const { return m_bRestart; }
    Q_PROPERTY(bool restart READ isRestart WRITE setRestart)

    bool isLogout() const { return m_bLogoutRestart; }

protected:
    virtual void closeEvent(QCloseEvent* event);

public:
    virtual QWidget* mainWindow() { return this; }
    virtual QObject* object() { return this; }
    virtual CWizDatabase& database() { return m_db; }
    virtual CWizCategoryView& category() { return *m_category; }
    virtual CWizUserSettings& userSettings() { return *m_settings; }

    //WizExplorerApp API:
    QObject* Window() { return this; }
    Q_PROPERTY(QObject* Window READ Window)

    QObject* CategoryCtrl();
    Q_PROPERTY(QObject* CategoryCtrl READ CategoryCtrl)

    QObject* DocumentsCtrl();
    Q_PROPERTY(QObject* DocumentsCtrl READ DocumentsCtrl)

    QObject* Database() { return &m_db; }
    Q_PROPERTY(QObject* Database READ Database)

    Q_INVOKABLE QObject* CreateWizObject(const QString& strObjectID);
    Q_INVOKABLE void SetDocumentModified(bool modified);
    Q_INVOKABLE void SetSavingDocument(bool saving);

private:
    CWizDatabase& m_db;
    CWizUserSettings* m_settings;
    QPointer<CWizUpgradeThread> m_upgrade;
    QPointer<CWizSyncThread> m_sync;
    CWizConsoleDialog* m_console;

#ifndef Q_OS_MAC
    QToolBar* m_toolBar;
    QLabel* m_labelNotice;
    QAction* m_optionsAction;
#endif

    QMenuBar* m_menuBar;
    CWizStatusBar* m_statusBar;

    CWizActions* m_actions;
    CWizCategoryView* m_category;
    CWizDocumentListView* m_documents;
    CWizDocumentView* m_doc;
    CWizSplitter* m_splitter;
    CWizOptionsWidget* m_options;

    CWizDocumentViewHistory* m_history;
    CWizAnimateAction* m_animateSync;

    bool m_bRestart;
    bool m_bLogoutRestart;
    bool m_bUpdatingSelection;

    WIZDOCUMENTDATA m_documentForEditing;

    QTimer m_timerReadyQuit;
    QMessageBox* m_msgQuit;

private:
    void initActions();
    void initMenuBar();
    void initToolBar();
    void initClient();
    void initStatusBar();

public:
    void viewDocument(const WIZDOCUMENTDATA& data, bool addToHistory);
    void locateDocument(const WIZDOCUMENTDATA& data);

#ifndef Q_OS_MAC
    CWizFixedSpacer* findFixedSpacer(int index);
    void adjustToolBarSpacerToPos(int index, int pos);
#endif

private:
    QObject* CategoryCtrlObject();
    QObject* DocumentsCtrlObject();

public Q_SLOTS:
    void on_actionExit_triggered();
    void on_actionConsole_triggered();
    void on_actionSync_triggered();
    void on_actionNewNote_triggered();
    void on_actionDeleteCurrentNote_triggered();
    void on_actionLogout_triggered();
    void on_actionAbout_triggered();
    void on_actionPreference_triggered();

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

#ifndef Q_OS_MAC
    void on_actionPopupMainMenu_triggered();
    void on_client_splitterMoved(int pos, int index);
    void on_options_restartForSettings();
#endif

    void on_readyQuit_timeout();

    void on_upgradeThread_finished();
};

#endif // WIZMAINWINDOW_H
