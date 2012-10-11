#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "wizdef.h"

#include "share/wizsettings.h"
#include "share/wizsync.h"
#include "wiznotesettings.h"


#ifndef Q_OS_MAC
class QToolBar;
#endif
class QStatusBar;
class QProgressBar;
class QLabel;
class QMenuBar;
class QTimer;

class CWizCategoryView;
class CWizDocumentListView;
class CWizDocumentView;
class CWizActions;
class CWizDocumentViewHistory;
class CWizFixedSpacer;
class CWizSplitter;
class CWizAnimateAction;
class CWizOptionsWidget;


class MainWindow
    : public QMainWindow
    , public CWizExplorerApp
    , public CWizSyncEvents
{
    Q_OBJECT

public:
    explicit MainWindow(CWizDatabase& db, QWidget *parent = 0);

    CWizUserSettings* settings() const { return m_settings; }
    void center(int width, int height);
    ~MainWindow();

public:
    virtual QWidget* mainWindow();
    virtual QObject* object();
    virtual CWizDatabase& database();
    virtual CWizCategoryView& category() { return *m_category; }
    //
    virtual void syncStarted();
    virtual void syncLogin();
    virtual void syncDone(bool error);
    virtual void addLog(const CString& strMsg);
    virtual void addDebugLog(const CString& strMsg);
    virtual void addErrorLog(const CString& strMsg);
    virtual void changeProgress(int pos);
private:
    CWizDatabase& m_db;
    CWizUserSettings* m_settings;
    CWizSync m_sync;
    //
    QMenuBar* m_menuBar;
#ifndef Q_OS_MAC
    QToolBar* m_toolBar;
    QLabel* m_labelNotice;
    QAction* m_optionsAction;
#endif
    QStatusBar* m_statusBar;
    QLabel* m_labelStatus;
    QProgressBar* m_progressSync;
    //
    CWizActions* m_actions;
    CWizCategoryView* m_category;
    CWizDocumentListView* m_documents;
    CWizDocumentView* m_doc;
    CWizSplitter* m_splitter;
    CWizOptionsWidget* m_options;
    //
    CWizDocumentViewHistory* m_history;
    //
    CWizAnimateAction* m_animateSync;
    //
    QTimer* m_syncTimer;
    //
    bool m_bRestart;
    bool m_bLogoutRestart;
    //
    bool m_bUpdatingSelection;
    //
    WIZDOCUMENTDATA m_documentForEditing;
private:
    void initActions();
    void initMenuBar();
    void initToolBar();
    void initClient();
    void initStatusBar();
    //
    void resetNotice();

public:
    virtual void init();
    virtual void closeEvent(QCloseEvent *);
    virtual QSize sizeHint() const;
public:
    CWizDatabase* Database() { return &m_db; }
    MainWindow* Window() { return this; }
    CWizCategoryView* CategoryCtrl() { return m_category; }
    CWizDocumentListView* DocumentsCtrl() { return m_documents; }
    //
    void viewDocument(const WIZDOCUMENTDATA& data, bool addToHistory);
    void locateDocument(const WIZDOCUMENTDATA& data);
    //
#ifndef Q_OS_MAC
    CWizFixedSpacer* findFixedSpacer(int index);
    void adjustToolBarSpacerToPos(int index, int pos);
#endif
public:
    bool isRestart() const { return m_bRestart; }
    bool isLogout() const { return m_bLogoutRestart; }
private:
    QObject* CategoryCtrlObject();
    QObject* DocumentsCtrlObject();
public:
    //interface WizExplorerApp
    Q_PROPERTY(QObject* Database READ Database)
    Q_PROPERTY(QObject* Window READ Window)
    //interface WizExplorerWindow
    Q_PROPERTY(QObject* CategoryCtrl READ CategoryCtrlObject)
    Q_PROPERTY(QObject* DocumentsCtrl READ DocumentsCtrlObject)
public slots:
    //interface WizExplorerApp;
    QObject* CreateWizObject(const QString& strObjectID);
    //interface WizExplorerWindow
    //
    //ext functions
    void SetDocumentModified(bool modified);
    void SetSavingDocument(bool saving);

    void on_actionExit_triggered();
    void on_actionSync_triggered();
    void on_actionNewNote_triggered();
    void on_actionDeleteCurrentNote_triggered();
    void on_actionLogout_triggered();
    void on_actionAbout_triggered();
    //void on_actionOptions_triggered();
    void on_actionPreference_triggered();
#ifndef Q_OS_MAC
    void on_actionPopupMainMenu_triggered();
    void on_client_splitterMoved(int pos, int index);
#endif
    //
    void on_actionGoBack_triggered();
    void on_actionGoForward_triggered();

    void on_category_itemSelectionChanged();
    void on_documents_itemSelectionChanged();

    void on_search_doSearch(const QString& keywords);
    //
    void on_options_settingsChanged(WizOptionsType type);
#ifndef Q_OS_MAC
    void on_options_restartForSettings();
#endif
    //
    void on_syncTimer_timeout();

};

#endif // MAINWINDOW_H
