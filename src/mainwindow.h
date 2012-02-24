#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "wizdef.h"

#include "share/wizsync.h"


#ifndef Q_OS_MAC
class QToolBar;
#endif
class QStatusBar;
class QProgressBar;
class QLabel;
class QMenuBar;

class CWizCategoryView;
class CWizDocumentListView;
class CWizDocumentView;
class CWizActions;
class CWizDocumentViewHistory;
class CWizFixedSpacer;
class CWizSplitter;
class CWizAnimateAction;


class MainWindow
    : public QMainWindow
    , public CWizExplorerApp
    , public CWizSyncEvents
{
    Q_OBJECT

public:
    explicit MainWindow(CWizDatabase& db, QWidget *parent = 0);
    ~MainWindow();

public:
    virtual QWidget* mainWindow();
    virtual QObject* object();
    virtual CWizDatabase& database();
    virtual CWizCategoryView& category() { return *m_category; }
    //
    virtual void syncStarted();
    virtual void syncDone(bool error);
    virtual void addLog(const CString& strMsg);
    virtual void addDebugLog(const CString& strMsg);
    virtual void addErrorLog(const CString& strMsg);
    virtual void changeProgress(int pos);
private:
    CWizDatabase& m_db;
    CWizSync m_sync;
    //
    QMenuBar* m_menuBar;
#ifndef Q_OS_MAC
    QToolBar* m_toolBar;
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
    //
    CWizDocumentViewHistory* m_history;
    //
    CWizAnimateAction* m_animateSync;
    //
    bool m_bRestart;
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
public:
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

};

#endif // MAINWINDOW_H
