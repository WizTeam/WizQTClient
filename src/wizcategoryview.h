#ifndef WIZCATEGORYCTRL_H
#define WIZCATEGORYCTRL_H

#include <QTreeWidget>

#include "wizdef.h"
#include "wizCategoryViewItem.h"


class CWizCategoryBaseView : public QTreeWidget
{
    Q_OBJECT

public:
    CWizCategoryBaseView(CWizExplorerApp& app, QWidget *parent = 0);
    virtual QSize sizeHint() const { return QSize(150, 1); }
    virtual void init() = 0;

    void getDocuments(CWizDocumentDataArray& arrayDocument);
    bool acceptDocument(const WIZDOCUMENTDATA& document);
    void addSeparator();

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
    CWizDatabase& m_db;

private:
    bool m_bDragHovered;
    QPoint m_dragHoveredPos;
};


class CWizCategoryView : public CWizCategoryBaseView
{
    Q_OBJECT

public:
    CWizCategoryView(CWizExplorerApp& app, QWidget *parent = 0);
    virtual void init();

    void showAllFoldersContextMenu(QPoint pos);
    void showFolderContextMenu(QPoint pos);
    void showTrashContextMenu(QPoint pos);

private:
    void initFolders();
    void initFolders(QTreeWidgetItem* pParent, const QString& strParentLocation, \
                     const CWizStdStringArray& arrayAllLocation);
    void initTrash();

private:
    QMenu* m_menuAllFolders;
    QMenu* m_menuFolder;
    QMenu* m_menuTrash;

    QTreeWidgetItem* m_selectedItem;

public:
    CWizCategoryViewAllFoldersItem* findAllFolders();
    CWizCategoryViewFolderItem* findFolder(const CString& strLocation, bool create, bool sort);
    CWizCategoryViewFolderItem* addFolder(const CString& strLocation, bool sort);

    CWizCategoryViewSearchItem* findSearch();
    CWizCategoryViewTrashItem* findTrash();

    void addAndSelectFolder(const CString& strLocation);

    void search(const QString& str);
    void restoreSelection();


public Q_SLOTS:
    void on_document_created(const WIZDOCUMENTDATA& document);
    void on_document_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew);
    void on_folder_created(const CString& strLocation);
    void on_folder_deleted(const CString& strLocation);
    void on_action_newFolder();
    void on_action_deleteFolder();
    void on_action_emptyTrash();

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

    void initTags();
    void initTags(QTreeWidgetItem* pParent, const QString& strParentTagGUID);

public Q_SLOTS:
    void on_tag_created(const WIZTAGDATA& tag);
    void on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew);
    void on_tag_deleted(const WIZTAGDATA& tag);
    void on_action_newTag();
    void on_action_deleteTag();
};


#endif // WIZCATEGORYCTRL_H
