#ifndef WIZCATEGORYCTRL_H
#define WIZCATEGORYCTRL_H

#include <QTreeWidget>
#include <QMessageBox>

#include "wizdef.h"
#include "wizCategoryViewItem.h"
#include "wizGroupAttributeForm.h"
#include "wizNewDialog.h"


class CWizCategoryBaseView : public QTreeWidget
{
    Q_OBJECT

public:
    CWizCategoryBaseView(CWizExplorerApp& app, QWidget *parent = 0);
    //virtual QSize sizeHint() const { return QSize(150, 1); }
    virtual void init() = 0;

    virtual CWizCategoryViewTrashItem* findTrash(const QString& strKbGUID) { Q_UNUSED(strKbGUID); return NULL; }
    void showTrashContextMenu(QPoint pos, const QString& strKbGUID);

    QString selectedItemKbGUID();
    void getDocuments(CWizDocumentDataArray& arrayDocument);
    bool acceptDocument(const WIZDOCUMENTDATA& document);
    void addSeparator();

    void saveSelection(const QString& str);
    void restoreSelection();

    template <class T> T* currentCategoryItem() const;
    CWizCategoryViewItemBase* categoryItemFromIndex(const QModelIndex &index) const;
    bool isSeparatorItemByIndex(const QModelIndex &index) const;
    bool isSeparatorItemByPosition(const QPoint& pt) const;

    virtual void contextMenuEvent(QContextMenuEvent * e);
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
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;

    QTreeWidgetItem* m_selectedItem;

private:
    bool m_bDragHovered;
    QPoint m_dragHoveredPos;

    QPointer<QMenu> m_menuTrash;
    QString m_strTrashKbGUID;

public Q_SLOTS:
    void on_action_emptyTrash();
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

private:
    QPointer<QMenu> m_menuAllFolders;
    QPointer<QMenu> m_menuFolder;
    QPointer<CWizNewDialog> m_MsgNewFolder;
    QPointer<QMessageBox> m_MsgWarning;

public:
    CWizCategoryViewAllFoldersItem* findAllFolders();
    CWizCategoryViewFolderItem* findFolder(const CString& strLocation, bool create, bool sort);
    CWizCategoryViewFolderItem* addFolder(const CString& strLocation, bool sort);


    void addAndSelectFolder(const CString& strLocation);

public Q_SLOTS:
    void on_document_created(const WIZDOCUMENTDATA& document);
    void on_document_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew);
    void on_folder_created(const CString& strLocation);
    void on_folder_deleted(const CString& strLocation);
    void on_action_newDocument();
    void on_action_newFolder();
    void on_action_newFolder_confirmed(int result);
    void on_action_deleteFolder();
    void on_action_deleteFolder_confirmed();

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

private:
    QMenu* m_menuAllTags;
    QMenu* m_menuTag;
    QPointer<CWizNewDialog> m_MsgNewTag;

    void initTags();
    void initTags(QTreeWidgetItem* pParent, const QString& strParentTagGUID);

public Q_SLOTS:
    void on_tag_created(const WIZTAGDATA& tag);
    void on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew);
    void on_tag_deleted(const WIZTAGDATA& tag);
    void on_action_newTag();
    void on_action_newTag_confirmed(int result);
    void on_action_deleteTag();
};

class CWizCategoryGroupsView : public CWizCategoryBaseView
{
    Q_OBJECT

public:
    CWizCategoryGroupsView(CWizExplorerApp& app, QWidget* parent);
    virtual void init();
    virtual CWizCategoryViewTrashItem* findTrash(const QString& strKbGUID);
    virtual QAction* findAction(const QString& strName);

    void showGroupRootContextMenu(const QString& strKbGUID, QPoint pos);
    void showGroupContextMenu(const QString& strKbGUID, QPoint pos);

    CWizCategoryViewGroupRootItem* findGroup(const QString& strKbGUID);

    CWizCategoryViewGroupItem* findTagInTree(const WIZTAGDATA& tag);
    CWizCategoryViewGroupItem* findTagInTree(const WIZTAGDATA& tag, QTreeWidgetItem* itemParent);
    CWizCategoryViewGroupItem* findTag(const WIZTAGDATA& tag, bool create, bool sort);
    CWizCategoryViewGroupItem* addTagWithChildren(const WIZTAGDATA& tag);
    void removeTag(const WIZTAGDATA& tag);

private:
    QString m_strKbGUID;
    QPointer<QMenu> m_menuGroupRoot;
    QPointer<QMenu> m_menuGroup;
    QPointer<CWizNewDialog> m_MsgNewFolder;
    void initGroup(CWizDatabase& db);
    void initGroup(CWizDatabase& db, QTreeWidgetItem* pParent, const QString& strParentTagGUID);

public Q_SLOTS:
    void on_group_opened(const QString& strKbGUID);
    void on_group_closed(const QString& strKbGUID);
    void on_group_rename(const QString& strKbGUID);
    void on_group_permissionChanged(const QString& strKbGUID);
    void on_tag_created(const WIZTAGDATA& tag);
    void on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew);
    void on_tag_deleted(const WIZTAGDATA& tag);

    void on_action_markRead();
    void on_action_newDocument();
    void on_action_newTag();
    void on_action_newTag_confirmed(int result);
    void on_action_modifyTag();
    void on_action_deleteTag();
    void on_action_openGroupAttribute();
};


#endif // WIZCATEGORYCTRL_H
