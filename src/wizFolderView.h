#ifndef WIZFOLDERVIEW_H
#define WIZFOLDERVIEW_H

#include <QTreeWidget>

#include "share/wizDatabaseManager.h"
#include "wizCategoryViewItem.h"

class CWizExplorerApp;
class CWizScrollBar;

#ifdef Q_OS_LINUX
#define WIZNOTE_CUSTOM_SCROLLBAR
#else
#if QT_VERSION < 0x050000
#define WIZNOTE_CUSTOM_SCROLLBAR
#endif
#endif


class CWizFolderView : public QTreeWidget
{
    Q_OBJECT

public:
    explicit CWizFolderView(CWizExplorerApp& app, QWidget *parent = 0, bool showReadOnlyGroup = false);

protected:
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;

#ifdef WIZNOTE_CUSTOM_SCROLLBAR
    CWizScrollBar* m_vScroll;
#endif

    virtual void resizeEvent(QResizeEvent* event);
    virtual void showEvent(QShowEvent *event);

private:
    void initFolders();
    void initFolders(QTreeWidgetItem* pParent,
                     const QString& strParentLocation,
                     const CWizStdStringArray& arrayAllLocation);    
    //
    void initGroups();
    void initGroup(CWizDatabase& db);
    void initGroup(CWizDatabase& db, QTreeWidgetItem* pParent,
                   const QString& strParentTagGUID);


    CWizCategoryViewFolderItem* addFolder(const QString& strLocation, bool sort);
    CWizCategoryViewFolderItem* findFolder(const QString& strLocation, bool create, bool sort);
    CWizCategoryViewAllFoldersItem* findAllFolders();
    CWizCategoryViewTrashItem* findTrash(const QString& strKbGUID);

    CWizCategoryViewGroupRootItem* findGroup(const QString& strKbGUID);
    CWizCategoryViewItemBase* findGroupsRootItem(const WIZGROUPDATA& group, bool bCreate = true);
    CWizCategoryViewItemBase* findBizGroupsRootItem(const WIZBIZDATA& biz, bool bCreate = true);
    CWizCategoryViewItemBase* findOwnGroupsRootItem(bool bCreate = true);
    CWizCategoryViewItemBase* findJionedGroupsRootItem(bool bCreate = true);

private:
    bool m_showReadOnlyGroup;
};

#endif // WIZFOLDERVIEW_H
