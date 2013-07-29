#ifndef WIZCATEGORYCTRL_H
#define WIZCATEGORYCTRL_H

#include <QTreeWidget>
#include <QMessageBox>

#include "wizdef.h"
#include "wizCategoryViewItem.h"
#include "wizGroupAttributeForm.h"
#include "wizLineInputDialog.h"
#include "share/wizuihelper.h"

class CWizScrollBar;
class QScrollAreaKineticScroller;

class CWizCategoryBaseView : public QTreeWidget
{
    Q_OBJECT

public:
    CWizCategoryBaseView(CWizExplorerApp& app, QWidget *parent = 0);
    void baseInit();

    virtual CWizCategoryViewTrashItem* findTrash(const QString& strKbGUID) { Q_UNUSED(strKbGUID); return NULL; }
    void showTrashContextMenu(QPoint pos, const QString& strKbGUID);

    QString selectedItemKbGUID();
    void getDocuments(CWizDocumentDataArray& arrayDocument);
    bool acceptDocument(const WIZDOCUMENTDATA& document);
    void addSeparator();
    CWizCategoryViewSpacerItem* addSpacer();

    void saveSelection();
    void restoreSelection();

    template <class T> T* currentCategoryItem() const;
    CWizCategoryViewItemBase* categoryItemFromIndex(const QModelIndex &index) const;
    bool isSeparatorItemByIndex(const QModelIndex &index) const;
    bool isSeparatorItemByPosition(const QPoint& pt) const;

    virtual void resizeEvent(QResizeEvent* event);
    virtual void contextMenuEvent(QContextMenuEvent* e);
    virtual void mousePressEvent(QMouseEvent* event);

    bool isDragHovered() const { return m_bDragHovered; }
    QPoint dragHoveredPos() const { return m_dragHoveredPos; }
    bool validateDropDestination(const QPoint& p) const;

    virtual void startDrag(Qt::DropActions supportedActions);
    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dragMoveEvent(QDragMoveEvent* event);
    virtual void dragLeaveEvent(QDragLeaveEvent* event);
    virtual void dropEvent(QDropEvent* event);

    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

protected:
    // subclass delegate
    virtual void onDocument_created(const WIZDOCUMENTDATA& document) { Q_UNUSED(document); }
    virtual void onDocument_modified(const WIZDOCUMENTDATA& documentOld,
                                     const WIZDOCUMENTDATA& documentNew)
    { Q_UNUSED(documentOld); Q_UNUSED(documentNew); }
    virtual void onDocument_deleted(const WIZDOCUMENTDATA& document) { Q_UNUSED(document); }
    virtual void onFolder_created(const QString& strLocation) { Q_UNUSED(strLocation); }
    virtual void onFolder_deleted(const QString& strLocation) { Q_UNUSED(strLocation); }
    virtual void onTag_created(const WIZTAGDATA& tag) { Q_UNUSED(tag); }
    virtual void onTag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew) { Q_UNUSED(tagOld); Q_UNUSED(tagNew); }
    virtual void onTag_deleted(const WIZTAGDATA& tag) { Q_UNUSED(tag); }
    virtual void onGroup_opened(const QString& strKbGUID) { Q_UNUSED(strKbGUID); }
    virtual void onGroup_closed(const QString& strKbGUID) { Q_UNUSED(strKbGUID); }
    virtual void onGroup_rename(const QString& strKbGUID) { Q_UNUSED(strKbGUID); }
    virtual void onGroup_permissionChanged(const QString& strKbGUID) { Q_UNUSED(strKbGUID); }

    virtual void updateDocumentCount(const QString& strKbGUID) = 0;

protected:
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;
    QTreeWidgetItem* m_selectedItem;

    virtual void init() = 0;

private:
    bool m_bDragHovered;
    QPoint m_dragHoveredPos;

    QPointer<QMenu> m_menuTrash;
    QString m_strTrashKbGUID;

    CWizScrollBar* m_vScroll;
    QPointer<QScrollAreaKineticScroller> m_kineticScroller;

public Q_SLOTS:
    void on_action_emptyTrash();

    void on_document_created(const WIZDOCUMENTDATA& document);
    void on_document_modified(const WIZDOCUMENTDATA& documentOld,
                              const WIZDOCUMENTDATA& documentNew);
    void on_document_deleted(const WIZDOCUMENTDATA& document);
    void on_document_tag_modified(const WIZDOCUMENTDATA& document);
    void on_folder_created(const QString& strLocation);
    void on_folder_deleted(const QString& strLocation);
    void on_tag_created(const WIZTAGDATA& tag);
    void on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew);
    void on_tag_deleted(const WIZTAGDATA& tag);
    void on_group_opened(const QString& strKbGUID);
    void on_group_closed(const QString& strKbGUID);
    void on_group_rename(const QString& strKbGUID);
    void on_group_permissionChanged(const QString& strKbGUID);
};


class CWizCategoryView : public CWizCategoryBaseView
{
    Q_OBJECT

public:
    CWizCategoryView(CWizExplorerApp& app, QWidget *parent = 0);
    virtual void init();
    virtual CWizCategoryViewTrashItem* findTrash(const QString& strKbGUID);

    void showAllFoldersContextMenu(QPoint pos);
    void showFolderContextMenu(QPoint pos);

private:
    void initFolders();
    void initFolders(QTreeWidgetItem* pParent, const QString& strParentLocation, \
                     const CWizStdStringArray& arrayAllLocation);
    void initTrash();

    void initDocumentCount();
    int initDocumentCount(CWizCategoryViewItemBase* item,
                          const std::map<CString, int>& mapDocumentCount);

    virtual void updateDocumentCount(const QString& strKbGUID);

private:
    QPointer<QMenu> m_menuAllFolders;
    QPointer<QMenu> m_menuFolder;

public:
    CWizCategoryViewAllFoldersItem* findAllFolders();
    CWizCategoryViewFolderItem* findFolder(const QString& strLocation, bool create, bool sort);
    CWizCategoryViewFolderItem* addFolder(const QString& strLocation, bool sort);

    void addAndSelectFolder(const CString& strLocation);

private:
    void doLocationSanityCheck(CWizStdStringArray& arrayLocation);

protected:
    virtual void onDocument_created(const WIZDOCUMENTDATA& document);
    virtual void onDocument_modified(const WIZDOCUMENTDATA& documentOld,
                                     const WIZDOCUMENTDATA& documentNew);
    virtual void onFolder_created(const QString& strLocation);
    virtual void onFolder_deleted(const QString& strLocation);

public Q_SLOTS:
    void on_action_newDocument();
    void on_action_newFolder();
    void on_action_newFolder_confirmed(int result);
    void on_action_moveFolder();
    void on_action_moveFolder_confirmed(int result);
    void on_action_moveFolder_confirmed_progress(int nMax, int nValue,
                                                 const QString& strOldLocation,
                                                 const QString& strNewLocation,
                                                 const WIZDOCUMENTDATA& data);
    void on_action_renameFolder();
    void on_action_renameFolder_confirmed(int result);
    void on_action_renameFolder_confirmed_progress(int nMax, int nValue,
                                                   const QString& strOldLocation,
                                                   const QString& strNewLocation,
                                                   const WIZDOCUMENTDATA& data);
    void on_action_deleteFolder();
    void on_action_deleteFolder_confirmed(int result);

Q_SIGNALS:
    void newDocument();

public:
    // Public API:
    Q_INVOKABLE CWizFolder* SelectedFolder();
};


class CWizCategoryTagsView : public CWizCategoryBaseView
{
    Q_OBJECT

public:
    CWizCategoryTagsView(CWizExplorerApp& app, QWidget *parent);
    virtual void init();

    void showAllTagsContextMenu(QPoint pos);
    void showTagContextMenu(QPoint pos);

    CWizCategoryViewAllTagsItem* findAllTags();
    CWizCategoryViewTagItem* findTag(const WIZTAGDATA& tag, bool create, bool sort);
    CWizCategoryViewTagItem* findTagInTree(const WIZTAGDATA& tag);
    CWizCategoryViewTagItem* findTagInTree(const WIZTAGDATA& tag, QTreeWidgetItem* itemParent);
    CWizCategoryViewTagItem* addTag(const WIZTAGDATA& tag, bool sort);
    CWizCategoryViewTagItem* addTagWithChildren(const WIZTAGDATA& tag);
    void removeTag(const WIZTAGDATA& tag);

    void addAndSelectTag(const WIZTAGDATA& tag);

private:
    QMenu* m_menuAllTags;
    QMenu* m_menuTag;

    void initTags();
    void initTags(QTreeWidgetItem* pParent, const QString& strParentTagGUID);

    void initDocumentCount();
    int initDocumentCount(CWizCategoryViewTagItem* item,
                          const std::map<CString, int>& mapDocumentCount);

    virtual void updateDocumentCount(const QString& strKbGUID);

protected:
    virtual void onTag_created(const WIZTAGDATA& tag);
    virtual void onTag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew);
    virtual void onTag_deleted(const WIZTAGDATA& tag);

public Q_SLOTS:
    void on_action_newTag();
    void on_action_newTag_confirmed(int result);
    void on_action_renameTag();
    void on_action_renameTag_confirmed(int result);
    void on_action_deleteTag();
    void on_action_deleteTag_confirmed(int result);
};

class CWizCategoryGroupsView : public CWizCategoryBaseView
{
    Q_OBJECT

public:
    CWizCategoryGroupsView(CWizExplorerApp& app, QWidget* parent);
    virtual void init();

    virtual QAction* findAction(const QString& strName);

    void showGroupRootContextMenu(const QString& strKbGUID, QPoint pos);
    void showGroupContextMenu(const QString& strKbGUID, QPoint pos);

    CWizCategoryViewGroupRootItem* findGroup(const QString& strKbGUID);
    CWizCategoryViewAllGroupsRootItem* findGroupSet(const QString& strName, bool bCreate);
    virtual CWizCategoryViewTrashItem* findTrash(const QString& strKbGUID);

    CWizCategoryViewGroupItem* findTagInTree(const WIZTAGDATA& tag);
    CWizCategoryViewGroupItem* findTagInTree(const WIZTAGDATA& tag, QTreeWidgetItem* itemParent);
    CWizCategoryViewGroupItem* findTag(const WIZTAGDATA& tag, bool create, bool sort);
    CWizCategoryViewGroupItem* addTagWithChildren(const WIZTAGDATA& tag);
    void removeTag(const WIZTAGDATA& tag);

private:
    QString m_strKbGUID;
    QPointer<QMenu> m_menuGroupRoot;
    QPointer<QMenu> m_menuGroup;

    void initGroup(CWizDatabase& db);
    void initGroup(CWizDatabase& db, QTreeWidgetItem* pParent, const QString& strParentTagGUID);

    void initDocumentCount();
    void initDocumentCount(CWizCategoryViewGroupRootItem* item,
                           CWizDatabase& db);
    int initDocumentCount(CWizCategoryViewGroupItem* item,
                          const std::map<CString, int>& mapDocumentCount);

    virtual void updateDocumentCount(const QString& strKbGUID);

protected:
    virtual void onTag_created(const WIZTAGDATA& tag);
    virtual void onTag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew);
    virtual void onTag_deleted(const WIZTAGDATA& tag);
    virtual void onGroup_opened(const QString& strKbGUID);
    virtual void onGroup_closed(const QString& strKbGUID);
    virtual void onGroup_rename(const QString& strKbGUID);
    virtual void onGroup_permissionChanged(const QString& strKbGUID);

public Q_SLOTS:
    void on_itemSelectionChanged();

    void on_action_markRead();
    void on_action_newDocument();
    void on_action_newTag();
    void on_action_newTag_confirmed(int result);
    void on_action_renameTag();
    void on_action_renameTag_confirmed(int result);
    void on_action_deleteTag();
    void on_action_deleteTag_confirmed(int result);
    void on_action_openGroupAttribute();
};


#endif // WIZCATEGORYCTRL_H
