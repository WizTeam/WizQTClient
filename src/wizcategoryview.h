#ifndef WIZCATEGORYCTRL_H
#define WIZCATEGORYCTRL_H

#include <QTreeWidget>
#include "wizdef.h"

class CWizCategoryViewItem;
class CWizCategoryViewAllFoldersItem;
class CWizCategoryViewFolderItem;
class CWizCategoryViewAllTagsItem;
class CWizCategoryViewTagItem;
class CWizCategoryViewSearchItem;
class CWizCategoryViewTrashItem;

class CWizCategoryView : public QTreeWidget
{
    Q_OBJECT
public:
    CWizCategoryView(CWizExplorerApp& app, QWidget *parent = 0);
protected:
    CWizDatabase& m_db;
    //
    void initFolders();
    void initFolders(QTreeWidgetItem* pParent, const CString& strParentLocation, const CWizStdStringArray& arrayAllLocation);
    void initTags();
    void initTags(QTreeWidgetItem* pParent, const CString& strParentTagGUID);
    void initTrash();
    //
    void addSeparator();
private:
    CWizExplorerApp& m_app;
    QMenu* m_menuAllFolders;
    QMenu* m_menuAllTags;
    QMenu* m_menuFolder;
    QMenu* m_menuTag;
    QMenu* m_menuTrash;
public:
    void showAllFoldersContextMenu(QPoint pos);
    void showFolderContextMenu(QPoint pos);
    void showAllTagsContextMenu(QPoint pos);
    void showTagContextMenu(QPoint pos);
    void showTrashContextMenu(QPoint pos);
public:
    void init();
    //
    void getDocuments(CWizDocumentDataArray& arrayDocument);
    //
    virtual void contextMenuEvent(QContextMenuEvent *);
    virtual void mousePressEvent(QMouseEvent* event );
    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
    virtual QSize sizeHint() const { return QSize(180, 1); }
    //drag
    virtual void startDrag(Qt::DropActions supportedActions);
    //drop
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent * event);
public:
    CWizCategoryViewItem* categoryItemFromIndex(const QModelIndex &index) const;
    bool isSeparatorItemByIndex(const QModelIndex &index) const;
    bool isSeparatorItemByPosition(const QPoint& pt) const;
    //
    CWizCategoryViewAllFoldersItem* findAllFolders();
    CWizCategoryViewFolderItem* findFolder(const CString& strLocation, bool create, bool sort);
    CWizCategoryViewFolderItem* addFolder(const CString& strLocation, bool sort);

    CWizCategoryViewAllTagsItem* findAllTags();
    CWizCategoryViewTagItem* findTag(const WIZTAGDATA& tag, bool create, bool sort);
    CWizCategoryViewTagItem* findTagInTree(const WIZTAGDATA& tag);
    CWizCategoryViewTagItem* findTagInTree(const WIZTAGDATA& tag, QTreeWidgetItem* itemParent);
    CWizCategoryViewTagItem* addTag(const WIZTAGDATA& tag, bool sort);
    CWizCategoryViewTagItem* addTagWithChildren(const WIZTAGDATA& tag);
    void removeTag(const WIZTAGDATA& tag);
    //
    CWizCategoryViewSearchItem* findSearch();
    //
    CWizCategoryViewTrashItem* findTrash();
    //
    bool acceptDocument(const WIZDOCUMENTDATA& document);
    //
    void addAndSelectFolder(const CString& strLocation);
    //
    void search(const CString& str);
    //
    template <class T> T* currentCategoryItem() const;
public slots:
    void on_tag_created(const WIZTAGDATA& tag);
    void on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew);
    void on_tag_deleted(const WIZTAGDATA& tag);
    void on_document_created(const WIZDOCUMENTDATA& document);
    void on_document_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew);
    void on_folder_created(const CString& strLocation);
    void on_folder_deleted(const CString& strLocation);
    void on_action_newFolder();
    void on_action_deleteFolder();
    void on_action_newTag();
    void on_action_deleteTag();
    void on_action_emptyTrash();
public:
    CWizFolder* SelectedFolder();
    void setSelectedFolder(QObject* pFolder);
public:
    Q_PROPERTY(QObject* SelectedFolder READ SelectedFolder WRITE setSelectedFolder)
};

template <class T>
        inline  T* CWizCategoryView::currentCategoryItem() const
{
    return dynamic_cast<T*>(currentItem());
}


#endif // WIZCATEGORYCTRL_H
