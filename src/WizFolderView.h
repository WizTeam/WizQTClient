#ifndef WIZFOLDERVIEW_H
#define WIZFOLDERVIEW_H

#include <QTreeWidget>

#include "share/WizDatabaseManager.h"
#include "WizCategoryViewItem.h"

class WizExplorerApp;
class WizScrollBar;

#define WIZNOTE_CUSTOM_SCROLLBAR


class WizFolderView : public QTreeWidget
{
    Q_OBJECT

public:
    explicit WizFolderView(WizExplorerApp& app, QWidget *parent = 0, bool showReadOnlyGroup = false);

protected:
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;

#ifdef WIZNOTE_CUSTOM_SCROLLBAR
    WizScrollBar* m_vScroll;
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
    void initGroup(WizDatabase& db);
    void initGroup(WizDatabase& db, QTreeWidgetItem* pParent,
                   const QString& strParentTagGUID);


    WizCategoryViewFolderItem* addFolder(const QString& strLocation, bool sort);
    WizCategoryViewFolderItem* findFolder(const QString& strLocation, bool create, bool sort);
    WizCategoryViewAllFoldersItem* findAllFolders();
    WizCategoryViewTrashItem* findTrash(const QString& strKbGUID);

    WizCategoryViewGroupRootItem* findGroup(const QString& strKbGUID);
    WizCategoryViewItemBase* findGroupsRootItem(const WIZGROUPDATA& group, bool bCreate = true);
    WizCategoryViewItemBase* findBizGroupsRootItem(const WIZBIZDATA& biz, bool bCreate = true);
    WizCategoryViewItemBase* findOwnGroupsRootItem(bool bCreate = true);
    WizCategoryViewItemBase* findJionedGroupsRootItem(bool bCreate = true);

private:
    bool m_showReadOnlyGroup;
};

#endif // WIZFOLDERVIEW_H
