#ifndef WIZCATEGORYCTRL_H
#define WIZCATEGORYCTRL_H

#include <QPointer>
#include <memory>
#include <QTreeView>
#include "WizCategoryViewItem.h"

class WizFolder;
class WizScrollBar;
class WizDatabaseManager;
class WizExplorerApp;
class QSettings;
class WizProgressDialog;
class WizObjectDownloaderHost;
class WizFolderSelector;

#define CATEGORY_MESSAGES_ALL               QObject::tr("Message Center")
#define CATEGORY_MESSAGES_SEND_TO_ME        QObject::tr("Send to me")
#define CATEGORY_MESSAGES_MODIFY            QObject::tr("Note modified")
#define CATEGORY_MESSAGES_COMMENTS          QObject::tr("Comments")
#define CATEGORY_MESSAGES_SEND_FROM_ME      QObject::tr("Send from me")


#define WIZNOTE_CUSTOM_SCROLLBAR

class WizCategoryBaseView : public QTreeWidget
{
    Q_OBJECT

public:
    WizCategoryBaseView(WizExplorerApp& app, QWidget *parent = 0);
    ~WizCategoryBaseView();

    QString selectedItemKbGUID();
    QString storedSelectedItemKbGuid();
    void getDocuments(CWizDocumentDataArray& arrayDocument);
    bool acceptDocument(const WIZDOCUMENTDATA& document);
    void updateItem(QTreeWidgetItem* pItem) { update(indexFromItem(pItem, 0)); }

    virtual void importFiles(QStringList& strFileList);

    void saveSelection();
    void restoreSelection();

    WizCategoryViewItemBase* itemAt(const QPoint& p) const;
    WizCategoryViewItemBase* itemFromKbGUID(const QString& strKbGUID) const;

    template <class T> T* currentCategoryItem() const
    {
        return dynamic_cast<T*>(currentItem());
    }

    WizCategoryViewItemBase* categoryItemFromIndex(const QModelIndex &index) const;

    bool isDragHovered() const { return m_bDragHovered; }
    QPoint dragHoveredPos() const { return m_dragHoveredPos; }
    bool validateDropDestination(const QPoint& p) const;
    Qt::ItemFlags dragItemFlags() const;

    bool isCursorEntered() const { return m_cursorEntered; }

    QPoint hitPoint() const { return m_hitPos; }

protected:
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void startDrag(Qt::DropActions supportedActions);
    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dragMoveEvent(QDragMoveEvent* event);
    virtual void dragLeaveEvent(QDragLeaveEvent* event);
    virtual void dropEvent(QDropEvent* event);
    //
    bool dropOn(QDropEvent *event, int *dropRow, int *dropCol, QModelIndex *dropIndex);
    void dropEventCore(QDropEvent *event);
    bool droppingOnItself(QDropEvent *event, const QModelIndex &index);
    QAbstractItemView::DropIndicatorPosition position(const QPoint &pos, const QRect &rect, const QModelIndex &index) const;



    virtual void enterEvent(QEvent * event);
    virtual void leaveEvent(QEvent * event);

    virtual void resizeEvent(QResizeEvent* event);
    virtual void contextMenuEvent(QContextMenuEvent* e);

    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

    virtual void resetRootItemsDropEnabled(WizCategoryViewItemBase* pItem);    

protected:
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;
    QTreeWidgetItem* m_selectedItem;


protected Q_SLOTS:
    virtual void on_document_created(const WIZDOCUMENTDATA& doc) { Q_UNUSED(doc); }
    virtual void on_document_modified(const WIZDOCUMENTDATA& documentOld,
                                      const WIZDOCUMENTDATA& documentNew)
    { Q_UNUSED(documentOld); Q_UNUSED(documentNew); }
    virtual void on_document_deleted(const WIZDOCUMENTDATA& doc) { Q_UNUSED(doc); }
    virtual void on_document_tag_modified(const WIZDOCUMENTDATA& doc) { Q_UNUSED(doc); }

    virtual void on_groupDocuments_unreadCount_modified(const QString& strKbGUID) {Q_UNUSED(strKbGUID);}

    virtual void on_folder_created(const QString& strLocation) { Q_UNUSED(strLocation); }
    virtual void on_folder_deleted(const QString& strLocation) { Q_UNUSED(strLocation); }
    virtual void on_folder_positionChanged() {}

    virtual void on_tag_created(const WIZTAGDATA& tag) { Q_UNUSED(tag); }
    virtual void on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew) { Q_UNUSED(tagOld); Q_UNUSED(tagNew); }
    virtual void on_tag_deleted(const WIZTAGDATA& tag) { Q_UNUSED(tag); }
    virtual void on_tags_positionChanged(const QString& strKbGUID) { Q_UNUSED(strKbGUID); }

    virtual void on_group_opened(const QString& strKbGUID) { Q_UNUSED(strKbGUID); }
    virtual void on_group_closed(const QString& strKbGUID) { Q_UNUSED(strKbGUID); }
    virtual void on_group_renamed(const QString& strKbGUID) { Q_UNUSED(strKbGUID); }
    virtual void on_group_permissionChanged(const QString& strKbGUID) { Q_UNUSED(strKbGUID); }
    virtual void on_group_bizChanged(const QString& strKbGUID) { Q_UNUSED(strKbGUID); }

    virtual void on_itemPosition_changed(WizCategoryViewItemBase* pItem) { Q_UNUSED(pItem); }

    void on_dragHovered_timeOut();

protected:
    QPoint m_hitPos;
    bool m_bDragHovered;
    bool m_cursorEntered;
    QPoint m_dragHoveredPos;
    CWizDocumentDataArray m_dragDocArray;
    bool m_dragUrls;
    QTimer* m_dragHoveredTimer;
    WizCategoryViewItemBase* m_dragItem;
    WizCategoryViewItemBase* m_dragHoveredItem;

#ifdef WIZNOTE_CUSTOM_SCROLLBAR
    WizScrollBar* m_vScroll;
#endif
};

enum CategorySection
{
    Section_MessageCenter,
    Section_Shortcuts,
    Section_QuickSearch,
    Section_Folders,
    Section_Tags,
    Section_BizGroups,
    Section_PersonalGroups
};

class WizCategoryView : public WizCategoryBaseView
{
    Q_OBJECT

public:
    WizCategoryView(WizExplorerApp& app, QWidget *parent = 0);
    virtual ~WizCategoryView();
    Q_INVOKABLE void init();

    void loadShortcutState();
    Q_INVOKABLE void saveShortcutState();
    void loadExpandState();
    void saveExpandState();


    // action user data
    enum CategoryActions
    {
        ActionNewDocument,
        ActionLoadDocument,
        ActionImportFile,
        ActionNewItem,
        ActionMoveItem,
        ActionCopyItem,
        ActionRenameItem,
        ActionDeleteItem,
        ActionItemAttribute,
        ActionEmptyTrash,
        ActionQuitGroup,
        ActionItemManage,
        ActionRemoveShortcutItem,
        ActionAddToShortcuts,
        ActionAdvancedSearch,
        ActionAddCustomSearch,
        ActionEditCustomSearch,
        ActionRemoveCustomSearch
    };

    enum CategoryMenuType
    {
        FolderRootItem,
        FolderItem,
        TagRootItem,
        TagItem,
        GroupRootItem,
        GroupItem,
        TrashItem,
        BizGroupRootItem,
        OwnGroupRootItem,
        ShortcutItem,
        AddCustomSearchItem,
        EditCustomSearchItem,
    };

    void initMenus();

    // becase Qt can't invoke two action with one shortcut keys, we can only use
    // one QAction and for different type of usages, and delegate task, so reset
    // menu text is necessary.
    void resetMenu(CategoryMenuType type);
    void setActionsEnabled(bool enable);

    void showFolderRootContextMenu(QPoint pos);
    void showFolderContextMenu(QPoint pos);
    void showTagRootContextMenu(QPoint pos);
    void showTagContextMenu(QPoint pos);
    void showNormalGroupRootContextMenu(QPoint pos);
    void showAdminGroupRootContextMenu(QPoint pos);
    void showOwnerGroupRootContextMenu(QPoint pos);
    void showNormalBizGroupRootContextMenu(QPoint pos);
    void showAdminBizGroupRootContextMenu(QPoint pos, bool usable = true);
    void showGroupContextMenu(QPoint pos);
    void showTrashContextMenu(QPoint pos);
    void showShortcutContextMenu(QPoint pos);
    void showCustomSearchContextMenu(QPoint pos, bool removable = false);


    bool setSectionVisible(CategorySection section, bool visible);
    bool isSectionVisible(CategorySection section) const;
    void loadSectionStatus();

public:
    WizCategoryViewItemBase* findFolder(const WIZDOCUMENTDATA& doc);

    // folders
    WizCategoryViewFolderItem* findFolder(const QString& strLocation, bool create, bool sort);
    WizCategoryViewFolderItem* addFolder(const QString& strLocation, bool sort);
    void addAndSelectFolder(const CString& strLocation);
    bool setCurrentIndex(const WIZDOCUMENTDATA& document);

    void sortFolders();
    void sortFolders(WizCategoryViewFolderItem* pItem);

    void sortPersonalTags();
    void sortPersonalTags(QTreeWidgetItem* pItem);

    void sortGroupTags(const QString& strKbGUID, bool bReloadData = false);
    void sortGroupTags(WizCategoryViewGroupItem* pItem, bool bReloadData);

    void savePersonalTagsPosition();
    void savePersonalTagsPosition(WizDatabase& db, WizCategoryViewTagItem* pItem);

    void saveGroupTagsPosition(const QString& strKbGUID);
    void saveGroupTagsPosition(WizDatabase& db, WizCategoryViewGroupItem* pItem);

    QString getAllFoldersPosition();
    QString getAllFoldersPosition(WizCategoryViewFolderItem* pItem, int& nStartPos);

    // tags
    WizCategoryViewTagItem* findTag(const WIZTAGDATA& tag, bool create, bool sort);
    WizCategoryViewTagItem* addTagWithChildren(const WIZTAGDATA& tag);
    WizCategoryViewTagItem* addTag(const WIZTAGDATA& tag, bool sort);
    WizCategoryViewTagItem* addAndSelectTag(const WIZTAGDATA& tag);
    WizCategoryViewTagItem* findTagInTree(const WIZTAGDATA& tag);
    WizCategoryViewTagItem* findTagInTree(const WIZTAGDATA& tag,
                                           QTreeWidgetItem* itemParent);
    void removeTag(const WIZTAGDATA& tag);

    // groups
    WizCategoryViewGroupItem* findGroupFolder(const WIZTAGDATA& tag, bool create, bool sort);
    WizCategoryViewGroupItem* addGroupFolderWithChildren(const WIZTAGDATA& tag);
    WizCategoryViewGroupRootItem* findGroup(const QString& strKbGUID);
    WizCategoryViewGroupItem* findGroupFolderInTree(const WIZTAGDATA& tag);
    WizCategoryViewGroupItem* findGroupFolderInTree(const WIZTAGDATA& tag,
                                                     QTreeWidgetItem* itemParent);
    void removeGroupFolder(const WIZTAGDATA& tag);

    // helper
    QAction* findAction(CategoryActions type);

    WizCategoryViewItemBase* findBizGroupsRootItem(const WIZBIZDATA& biz, bool bCreate = true);
    WizCategoryViewItemBase* findOwnGroupsRootItem(bool bCreate = true);
    WizCategoryViewItemBase* findJionedGroupsRootItem(bool bCreate = true);
    WizCategoryViewItemBase* findGroupsRootItem(const WIZGROUPDATA& group, bool bCreate = true);
    WizCategoryViewItemBase* findAllFolderItem();
    WizCategoryViewItemBase* findAllTagsItem();
    WizCategoryViewItemBase* findAllSearchItem();
    WizCategoryViewItemBase* findAllMessagesItem();
    WizCategoryViewItemBase* findAllShortcutItem();
    WizCategoryViewTrashItem* findTrash(const QString& strKbGUID = NULL);

    // document count update
    void updatePersonalFolderDocumentCount();
    void updatePersonalFolderDocumentCount_impl();

    void updateGroupFolderDocumentCount(const QString& strKbGUID);
    void updateGroupFolderDocumentCount_impl(const QString& strKbGUID);

    void updatePersonalTagDocumentCount();
    void updatePersonalTagDocumentCount_impl(const QString& strKbGUID = NULL);

    void updateGroupTagDocumentCount(const QString &strKbGUID);

    virtual void importFiles(QStringList& strFileList);

    //TODO: 全部移动到notemanager去实现
    bool createDocument(WIZDOCUMENTDATA& data);
    bool createDocument(WIZDOCUMENTDATA& data, const QString& strHtml, const QString& strTitle);
    bool createDocumentByAttachments(WIZDOCUMENTDATA& data, const QStringList& attachList);

    //
    void createGroup();
    void viewPersonalGroupInfo(const QString& groupGUID);
    void viewBizGroupInfo(const QString& groupGUID, const QString& bizGUID);
    void managePersonalGroup(const QString& groupGUID);
    void manageBizGroup(const QString& groupGUID, const QString& bizGUID);
    void viewBizInfo(const QString& bizGUID);
    void manageBiz(const QString& bizGUID, bool bUpgrade);

    //
    bool getAvailableNewNoteTagAndLocation(QString& strKbGUID,WIZTAGDATA& strTag,
                                           QString& strLocation);
    //

signals:
    void newDocument();
    void documentsHint(const QString& strHint);

    void unreadButtonClicked();

    void categoryItemPositionChanged(const QString& strKbGUID);


protected Q_SLOTS:
    virtual void on_document_created(const WIZDOCUMENTDATA& doc);
    virtual void on_document_modified(const WIZDOCUMENTDATA& docOld,
                                      const WIZDOCUMENTDATA& docNew);
    virtual void on_document_deleted(const WIZDOCUMENTDATA& doc);
    virtual void on_document_tag_modified(const WIZDOCUMENTDATA& doc);

    virtual void on_folder_created(const QString& strLocation);
    virtual void on_folder_deleted(const QString& strLocation);
    virtual void on_folder_positionChanged();

    virtual void on_tag_created(const WIZTAGDATA& tag);
    virtual void on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew);
    virtual void on_tag_deleted(const WIZTAGDATA& tag);
    virtual void on_tags_positionChanged(const QString& strKbGUID);

    virtual void on_group_opened(const QString& strKbGUID);
    virtual void on_group_closed(const QString& strKbGUID);
    virtual void on_group_renamed(const QString& strKbGUID);
    virtual void on_group_permissionChanged(const QString& strKbGUID);
    virtual void on_group_bizChanged(const QString& strKbGUID);
    virtual void on_groupDocuments_unreadCount_modified(const QString& strKbGUID);

    virtual void on_itemPosition_changed(WizCategoryViewItemBase* pItem);


    void on_importFile_finished(bool ok, QString text, QString kbGuid);

public Q_SLOTS:
    void on_action_newDocument();
    void on_action_loadDocument();
    void on_action_importFile();

    void on_action_newItem();
    void on_action_user_newFolder();
    void on_newFolder_inputText_changed(const QString& text);
    void on_action_user_newFolder_confirmed(int result);
    void on_action_user_newTag();
    void on_action_user_newTag_confirmed(int result);
    void on_newTag_inputText_changed(const QString& text);
    void on_action_group_newFolder();
    void on_action_group_newFolder_confirmed(int result);
    void on_group_newFolder_inputText_changed(const QString& text);

    void on_action_moveItem();
    void on_action_user_moveFolder();
    void on_action_user_moveFolder_confirmed(int result);
    void on_action_user_moveFolder_confirmed_progress(int nMax, int nValue,
                                                      const QString& strOldLocation,
                                                      const QString& strNewLocation,
                                                      const WIZDOCUMENTDATA& data);

    void on_action_copyItem();
    void on_action_user_copyFolder();
    void on_action_user_copyFolder_confirmed(int result);


    void on_action_renameItem();
    void on_action_user_renameFolder();
    void on_action_user_renameFolder_confirmed(int result);
    void on_action_user_renameFolder_confirmed_progress(int nMax, int nValue,
                                                        const QString& strOldLocation,
                                                        const QString& strNewLocation,
                                                        const WIZDOCUMENTDATA& data);
    void on_action_user_renameTag();
    void on_action_user_renameTag_confirmed(int result);
    void on_action_group_renameFolder();
    void on_action_group_renameFolder_confirmed(int result);

    void on_action_deleteItem();
    void on_action_user_deleteFolder();
    void on_action_user_deleteFolder_confirmed(int result);
    void on_action_user_deleteTag();
    void on_action_user_deleteTag_confirmed(int result);
    void on_action_group_deleteFolder();
    void on_action_group_deleteFolder_confirmed(int result);

    void on_action_itemAttribute();
    void on_action_groupAttribute();
    void on_action_bizgAttribute();

    void on_action_itemManage();
    void on_action_manageGroup();
    void on_action_manageBiz();

    void on_action_removeShortcut();
    void on_action_addToShortcuts();

    void on_action_addCustomSearch();
    void on_action_editCustomSearch();
    void on_action_removeCustomSearch();

    void on_action_emptyTrash();

    void on_itemSelectionChanged();
    void on_itemChanged(QTreeWidgetItem * item, int column);
    void on_itemClicked(QTreeWidgetItem *item, int column);

    void updateGroupsData();

    void on_shortcutDataChanged(const QString& shortcut);

    //
    void addDocumentToShortcuts(const WIZDOCUMENTDATA& doc);

public:
    // Public API:
    Q_INVOKABLE WizFolder* SelectedFolder();

private Q_SLOTS:
    void on_updatePersonalFolderDocumentCount_timeout();
    void on_updatePersonalTagDocumentCount_timeout();
    void on_updateGroupFolderDocumentCount_mapped_timeout(const QString& strKbGUID);

    //
    void on_initGroupFolder_finished();

private:
    void updateChildFolderDocumentCount(WizCategoryViewItemBase* pItem,
                                       const std::map<CString, int>& mapDocumentCount, int& allCount);

    void updateChildTagDocumentCount(WizCategoryViewItemBase* pItem,
                                    const std::map<CString, int>& mapDocumentCount, int& allCount);

    void setBizRootItemExtraButton(WizCategoryViewItemBase* pItem, \
                                     const WIZBIZDATA& bizData);
    void setGroupRootItemExtraButton(WizCategoryViewItemBase* pItem, \
                                     const WIZGROUPDATA& gData);

    void moveFolderPostionBeforeTrash(const QString& strLocation);    

    void quickSyncNewDocument(const QString& strKbGUID);

    void updatePersonalFolderLocation(WizDatabase& db,const QString& strOldLocation,\
                                      const QString& strNewLocation);
    void updatePersonalTagPosition();
    void updateGroupFolderPosition(WizDatabase& db, WizCategoryViewItemBase* pItem);

    //
    void promptGroupLimitMessage(const QString& groupGUID, const QString& bizGUID);

private:
    void initTopLevelItems();
    void initGeneral();
    void initFolders();
    void initFolders(QTreeWidgetItem* pParent, const QString& strParentLocation, \
                     const CWizStdStringArray& arrayAllLocation);//, const QMap<QString, int> &mfpos);
    void initTags();
    void initTags(QTreeWidgetItem* pParent, const QString& strParentTagGUID);
    void initStyles();
    void initGroups();
    void initBiz(const WIZBIZDATA& biz);
    void initGroup(WizDatabase& db);
    void initGroup(WizDatabase& db, bool& itemCreeated);
    void initGroup(WizDatabase& db, QTreeWidgetItem* pParent,
                   const QString& strParentTagGUID, const std::multimap<CString, WIZTAGDATA>& mapTag);
    void initQuickSearches();
    void initShortcut(const QString& shortcut);
    //
    void resetCreateGroupLink();

    //
    void resetSections();

    void doLocationSanityCheck(CWizStdStringArray& arrayLocation);

    QString selectedId(QSettings* settings);
    void saveSelected(QSettings* settings);

    void loadChildState(QTreeWidgetItem* pi, QSettings* settings);
    void loadItemState(QTreeWidgetItem* pi, QSettings* settings);
    void saveChildState(QTreeWidgetItem* pi, QSettings* settings);
    void saveItemState(QTreeWidgetItem* pi, QSettings* settings);

    void advancedSearchByCustomParam(const QString& strParam);
    void saveCustomAdvancedSearchParamToDB(const QString& strGuid, const QString& strParam);
    void loadCustomAdvancedSearchParamFromDB(QMap<QString, QString>& paramMap);
    void deleteCustomAdvancedSearchParamFromDB(const QString& strGuid);

    WizCategoryViewFolderItem* createFolderItem(QTreeWidgetItem* parent, const QString& strLocation);


    //
    void moveGroupFolder(const WIZTAGDATA& sourceFolder, WizFolderSelector* selector);

    void moveGroupFolderToPersonalFolder(const WIZTAGDATA& groupFolder, const QString& targetParentFolder, bool combineFolder);

    void moveGroupFolderToGroupFolder(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder, bool combineFolder);
    //
    bool movePersonalFolder(const QString& sourceFolder, WizFolderSelector* selector);

    void movePersonalFolderToPersonalFolder(const QString& sourceFolder, const QString& targetParentFolder, bool combineFolder);

    void movePersonalFolderToGroupFolder(const QString& sourceFolder, const WIZTAGDATA& targetFolder, bool combineFolder);

    //
    void copyGroupFolder(const WIZTAGDATA& sourceFolder, WizFolderSelector* selector);

    void copyGroupFolderToPersonalFolder(const WIZTAGDATA& groupFolder, const QString& targetParentFolder,
                                         bool keepDocTime, bool combineFolder);

    void copyGroupFolderToGroupFolder(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder,
                                      bool keepDocTime, bool combineFolder);
    //
    void copyPersonalFolder(const QString& sourceFolder, WizFolderSelector* selector);

    void copyPersonalFolderToPersonalFolder(const QString& sourceFolder, const QString& targetParentFolder,
                                            bool keepDocTime, bool keepTag, bool combineFolder);

    void copyPersonalFolderToGroupFolder(const QString& sourceFolder, const WIZTAGDATA& targetFolder,
                                         bool keepDocTime, bool combineFolder);
    //
    void moveDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument, const WIZTAGDATA& targetTag);    

    //
    virtual void dropItemAsBrother(WizCategoryViewItemBase* targetItem, WizCategoryViewItemBase* dragedItem,
                                   bool dropAtTop, bool deleteDragSource);
    virtual void dropItemAsChild(WizCategoryViewItemBase* targetItem, WizCategoryViewItemBase* dragedItem,
                                 bool deleteDragSource);

    //
    QString getUseableItemName(QTreeWidgetItem* parent, \
                                QTreeWidgetItem* item);
    void moveFolder(QString oldLocation, QString newLocation);
    void resetFolderLocation(WizCategoryViewFolderItem* item);
    void resetFolderLocation(WizCategoryViewFolderItem* item, const QString& strNewLocation);
    bool renameFolder(WizCategoryViewFolderItem* item, const QString& strFolderName);
    bool renameGroupFolder(WizCategoryViewGroupItem* pGroup, const QString& strFolderName);
    //
    void updateShortcut(int type, const QString& keyValue, const QString& name);
    void removeShortcut(int type, const QString& keyValue);
    void removeShortcut(WizCategoryViewItemBase* shortcut);

    //
    QTreeWidgetItem* findSameNameBrother(QTreeWidgetItem* parent, QTreeWidgetItem* exceptItem, const QString& name);
    bool isCombineSameNameFolder(const WIZTAGDATA& parentTag, const QString& folderName,
                                 bool& isCombine, QTreeWidgetItem* exceptBrother = nullptr);
    bool isCombineSameNameFolder(const QString& parentFolder, const QString& folderName,
                                 bool& isCombine, QTreeWidgetItem* exceptBrother = nullptr);

    bool combineGroupFolder(WizCategoryViewGroupItem* sourceItem, WizCategoryViewGroupItem* targetItem);

private:
    std::shared_ptr<QMenu> m_menuShortcut;
    std::shared_ptr<QMenu> m_menuFolderRoot;
    std::shared_ptr<QMenu> m_menuFolder;
    std::shared_ptr<QMenu> m_menuTagRoot;
    std::shared_ptr<QMenu> m_menuTag;
    std::shared_ptr<QMenu> m_menuNormalGroupRoot;
    std::shared_ptr<QMenu> m_menuAdminGroupRoot;
    std::shared_ptr<QMenu> m_menuOwnerGroupRoot;
    std::shared_ptr<QMenu> m_menuNormalBizGroupRoot;
    std::shared_ptr<QMenu> m_menuAdminBizGroupRoot;
    std::shared_ptr<QMenu> m_menuGroup;
    std::shared_ptr<QMenu> m_menuTrash;
    std::shared_ptr<QMenu> m_menuCustomSearch;
    QPointer<QTimer> m_timerUpdateFolderCount;
    QPointer<QTimer> m_timerUpdateTagCount;
    QMap<QString, QTimer*> m_mapTimerUpdateGroupCount;

    QString m_strRequestedGroupKbGUID;

    QString m_strSelectedId;

};


#endif // WIZCATEGORYCTRL_H
