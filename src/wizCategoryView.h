#ifndef WIZCATEGORYCTRL_H
#define WIZCATEGORYCTRL_H

#include <QPointer>
#include <memory>
#include <coreplugin/itreeview.h>
#include "wizCategoryViewItem.h"

class CWizFolder;
class CWizScrollBar;
class CWizDatabaseManager;
class CWizExplorerApp;
class QSettings;
class CWizProgressDialog;
class CWizObjectDataDownloaderHost;
class CWizFolderSelector;

#define CATEGORY_MESSAGES_ALL               QObject::tr("Message Center")
#define CATEGORY_MESSAGES_SEND_TO_ME        QObject::tr("Send to me")
#define CATEGORY_MESSAGES_MODIFY            QObject::tr("Note modified")
#define CATEGORY_MESSAGES_COMMENTS          QObject::tr("Comments")
#define CATEGORY_MESSAGES_SEND_FROM_ME      QObject::tr("Send from me")


#ifdef Q_OS_LINUX
#define WIZNOTE_CUSTOM_SCROLLBAR
#else
//#if QT_VERSION < 0x050000
#define WIZNOTE_CUSTOM_SCROLLBAR
//#endif
#endif

class CWizCategoryBaseView : public Core::ITreeView
{
    Q_OBJECT

public:
    CWizCategoryBaseView(CWizExplorerApp& app, QWidget *parent = 0);
    ~CWizCategoryBaseView();

    QString selectedItemKbGUID();
    void getDocuments(CWizDocumentDataArray& arrayDocument);
    bool acceptDocument(const WIZDOCUMENTDATA& document);

    void importFiles(QStringList& strFileList);

    void saveSelection();
    void restoreSelection();

    CWizCategoryViewItemBase* itemAt(const QPoint& p) const;
    CWizCategoryViewItemBase* itemFromKbGUID(const QString& strKbGUID) const;

    template <class T> T* currentCategoryItem() const
    {
        return dynamic_cast<T*>(currentItem());
    }

    CWizCategoryViewItemBase* categoryItemFromIndex(const QModelIndex &index) const;

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

    virtual void enterEvent(QEvent * event);
    virtual void leaveEvent(QEvent * event);

    virtual void resizeEvent(QResizeEvent* event);
    virtual void contextMenuEvent(QContextMenuEvent* e);

    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

    virtual void resetRootItemsDropEnabled(CWizCategoryViewItemBase* pItem);    

    //
    virtual void dropItemAsBrother(CWizCategoryViewItemBase* targetItem, CWizCategoryViewItemBase* dragedItem,
                                   bool dropAtTop, bool deleteDragSource);
    virtual void dropItemAsChild(CWizCategoryViewItemBase* targetItem, CWizCategoryViewItemBase* dragedItem,
                                 bool deleteDragSource);

protected:
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;
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

    virtual void on_itemPosition_changed(CWizCategoryViewItemBase* pItem) { Q_UNUSED(pItem); }

    virtual void createDocumentByHtml(const QString& strHtml, const QString& strTitle) = 0;
    virtual void createDocumentByHtml(const QString& strFileName, const QString& strHtml, const QString& strTitle);
    virtual bool createDocumentWithAttachment(const QString& strFileName);
    virtual bool createDocumentByHtmlWithAttachment(const QString& strHtml, const QString& strTitle,
                                                    const QString& strAttachFile);
    void on_dragHovered_timeOut();

protected:
    QPoint m_hitPos;
    bool m_bDragHovered;
    bool m_cursorEntered;
    QPoint m_dragHoveredPos;
    CWizDocumentDataArray m_dragDocArray;
    bool m_dragUrls;
    QTimer* m_dragHoveredTimer;
    CWizCategoryViewItemBase* m_dragItem;
    CWizCategoryViewItemBase* m_dragHoveredItem;

#ifdef WIZNOTE_CUSTOM_SCROLLBAR
    CWizScrollBar* m_vScroll;
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

class CWizCategoryView : public CWizCategoryBaseView
{
    Q_OBJECT

public:
    CWizCategoryView(CWizExplorerApp& app, QWidget *parent = 0);
    virtual ~CWizCategoryView();
    void init();

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
        ActionRecovery,
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
    CWizCategoryViewItemBase* findFolder(const WIZDOCUMENTDATA& doc);

    // folders
    CWizCategoryViewFolderItem* findFolder(const QString& strLocation, bool create, bool sort);
    CWizCategoryViewFolderItem* addFolder(const QString& strLocation, bool sort);
    void addAndSelectFolder(const CString& strLocation);
    bool setCurrentIndex(const WIZDOCUMENTDATA& document);

    void sortFolders();
    void sortFolders(CWizCategoryViewFolderItem* pItem);

    void sortGroupTags(const QString& strKbGUID, bool bReloadData = false);
    void sortGroupTags(CWizCategoryViewGroupItem* pItem, bool bReloadData);

    void savePersonalTagsPosition();
    void savePersonalTagsPosition(CWizDatabase& db, CWizCategoryViewTagItem* pItem);

    void saveGroupTagsPosition(const QString& strKbGUID);
    void saveGroupTagsPosition(CWizDatabase& db, CWizCategoryViewGroupItem* pItem);

    QString getAllFoldersPosition();
    QString getAllFoldersPosition(CWizCategoryViewFolderItem* pItem, int& nStartPos);

    // tags
    CWizCategoryViewTagItem* findTag(const WIZTAGDATA& tag, bool create, bool sort);
    CWizCategoryViewTagItem* addTagWithChildren(const WIZTAGDATA& tag);
    CWizCategoryViewTagItem* addTag(const WIZTAGDATA& tag, bool sort);
    CWizCategoryViewTagItem* addAndSelectTag(const WIZTAGDATA& tag);
    CWizCategoryViewTagItem* findTagInTree(const WIZTAGDATA& tag);
    CWizCategoryViewTagItem* findTagInTree(const WIZTAGDATA& tag,
                                           QTreeWidgetItem* itemParent);
    void removeTag(const WIZTAGDATA& tag);

    // groups
    CWizCategoryViewGroupItem* findGroupFolder(const WIZTAGDATA& tag, bool create, bool sort);
    CWizCategoryViewGroupItem* addGroupFolderWithChildren(const WIZTAGDATA& tag);
    CWizCategoryViewGroupRootItem* findGroup(const QString& strKbGUID);
    CWizCategoryViewGroupItem* findGroupFolderInTree(const WIZTAGDATA& tag);
    CWizCategoryViewGroupItem* findGroupFolderInTree(const WIZTAGDATA& tag,
                                                     QTreeWidgetItem* itemParent);
    void removeGroupFolder(const WIZTAGDATA& tag);

    // helper
    QAction* findAction(CategoryActions type);

    CWizCategoryViewItemBase* findBizGroupsRootItem(const WIZBIZDATA& biz, bool bCreate = true);
    CWizCategoryViewItemBase* findOwnGroupsRootItem(bool bCreate = true);
    CWizCategoryViewItemBase* findJionedGroupsRootItem(bool bCreate = true);
    CWizCategoryViewItemBase* findGroupsRootItem(const WIZGROUPDATA& group, bool bCreate = true);
    CWizCategoryViewItemBase* findAllFolderItem();
    CWizCategoryViewItemBase* findAllTagsItem();
    CWizCategoryViewItemBase* findAllSearchItem();
    CWizCategoryViewItemBase* findAllMessagesItem();
    CWizCategoryViewItemBase* findAllShortcutItem();
    CWizCategoryViewTrashItem* findTrash(const QString& strKbGUID = NULL);

    // document count update
    void updatePersonalFolderDocumentCount();
    void updatePersonalFolderDocumentCount_impl();

    void updateGroupFolderDocumentCount(const QString& strKbGUID);
    void updateGroupFolderDocumentCount_impl(const QString& strKbGUID);

    void updatePersonalTagDocumentCount();
    void updatePersonalTagDocumentCount_impl(const QString& strKbGUID = NULL);

    void updateGroupTagDocumentCount(const QString &strKbGUID);

    bool createDocument(WIZDOCUMENTDATA& data);
    bool createDocument(WIZDOCUMENTDATA& data, const QString& strHtml, const QString& strTitle);

    bool createDocumentWithAttachment(const QString& strFileName);
    bool createDocumentByHtmlWithAttachment(const QString& strHtml, const QString& strTitle,
                                                    const QString& strAttachFile);
    bool createDocumentByAttachments(WIZDOCUMENTDATA& data, const QStringList& attachList);
    bool createDocumentByTemplate(WIZDOCUMENTDATA& data, const QString& strZiw);

    //
    void createGroup();
    void viewPersonalGroupInfo(const QString& groupGUID);
    void viewBizGroupInfo(const QString& groupGUID, const QString& bizGUID);
    void managePersonalGroup(const QString& groupGUID);
    void manageBizGroup(const QString& groupGUID, const QString& bizGUID);
    void viewBizInfo(const QString& bizGUID);
    void manageBiz(const QString& bizGUID, bool bUpgrade);


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

    virtual void on_itemPosition_changed(CWizCategoryViewItemBase* pItem);

    virtual void createDocumentByHtml(const QString& strHtml, const QString& strTitle);
    virtual void createDocumentByHtml(const QString &strFileName, const QString& strHtml,
                                      const QString& strTitle);


public Q_SLOTS:
    void on_action_newDocument();
    void on_action_loadDocument();
    void on_action_importFile();

    void on_action_newItem();
    void on_action_user_newFolder();
    void on_action_user_newFolder_confirmed(int result);
    void on_action_user_newTag();
    void on_action_user_newTag_confirmed(int result);
    void on_action_group_newFolder();
    void on_action_group_newFolder_confirmed(int result);

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

    void on_action_deleted_recovery();

    void on_action_itemAttribute();
    void on_action_groupAttribute();
    void on_action_bizgAttribute();

    void on_action_itemManage();
    void on_action_manageGroup();
    void on_action_manageBiz();

    void on_action_removeShortcut();
    void on_action_addToShortcuts();

    void on_action_advancedSearch();
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
    Q_INVOKABLE CWizFolder* SelectedFolder();

private Q_SLOTS:
    void on_updatePersonalFolderDocumentCount_timeout();
    void on_updatePersonalTagDocumentCount_timeout();
    void on_updateGroupFolderDocumentCount_mapped_timeout(const QString& strKbGUID);


private:
    void updateChildFolderDocumentCount(CWizCategoryViewItemBase* pItem,
                                       const std::map<CString, int>& mapDocumentCount, int& allCount);

    void updateChildTagDocumentCount(CWizCategoryViewItemBase* pItem,
                                    const std::map<CString, int>& mapDocumentCount, int& allCount);

    void setBizRootItemExtraButton(CWizCategoryViewItemBase* pItem, \
                                     const WIZBIZDATA& bizData);
    void setGroupRootItemExtraButton(CWizCategoryViewItemBase* pItem, \
                                     const WIZGROUPDATA& gData);

    void moveFolderPostionBeforeTrash(const QString& strLocation);

    bool getAvailableNewNoteTagAndLocation(QString& strKbGUID,WIZTAGDATA& strTag,
                                           QString& strLocation);

    void quickSyncNewDocument(const QString& strKbGUID);

    void updatePersonalFolderLocation(CWizDatabase& db,const QString& strOldLocation,\
                                      const QString& strNewLocation);
    void updatePersonalTagPosition();
    void updateGroupFolderPosition(CWizDatabase& db, CWizCategoryViewItemBase* pItem);

    //
    void promptGroupLimitMessage(const QString& groupGUID, const QString& bizGUID);

private:
    void initGeneral();
    void initFolders();
    void initFolders(QTreeWidgetItem* pParent, const QString& strParentLocation, \
                     const CWizStdStringArray& arrayAllLocation);//, const QMap<QString, int> &mfpos);
    void initTags();
    void initTags(QTreeWidgetItem* pParent, const QString& strParentTagGUID);
    void initStyles();
    void initGroups();
    void initBiz(const WIZBIZDATA& biz);
    void initGroup(CWizDatabase& db);
    void initGroup(CWizDatabase& db, bool& itemCreeated);
    void initGroup(CWizDatabase& db, QTreeWidgetItem* pParent,
                   const QString& strParentTagGUID);
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

    CWizCategoryViewFolderItem* createFolderItem(QTreeWidgetItem* parent, const QString& strLocation);


    //
    void moveGroupFolder(const WIZTAGDATA& sourceFolder, CWizFolderSelector* selector,
                         CWizProgressDialog* progress, CWizObjectDataDownloaderHost* downloader);

    void moveGroupFolderToPersonalFolder(const WIZTAGDATA& groupFolder, const QString& targetParentFolder, bool combineFolder,
                                         CWizProgressDialog* progress, CWizObjectDataDownloaderHost* downloader);

    void moveGroupFolderToGroupFolder(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder, bool combineFolder,
                                      CWizProgressDialog* progress, CWizObjectDataDownloaderHost* downloader);
    //
    void movePersonalFolder(const QString& sourceFolder, CWizFolderSelector* selector,
                            CWizProgressDialog* progress, CWizObjectDataDownloaderHost* downloader);

    void movePersonalFolderToPersonalFolder(const QString& sourceFolder, const QString& targetParentFolder, bool combineFolder,
                                            CWizProgressDialog* progress);

    void movePersonalFolderToGroupFolder(const QString& sourceFolder, const WIZTAGDATA& targetFolder, bool combineFolder,
                                         CWizProgressDialog* progress, CWizObjectDataDownloaderHost* downloader);

    //
    void copyGroupFolder(const WIZTAGDATA& sourceFolder, CWizFolderSelector* selector,
                         CWizProgressDialog* progress, CWizObjectDataDownloaderHost* downloader);

    void copyGroupFolderToPersonalFolder(const WIZTAGDATA& groupFolder, const QString& targetParentFolder,
                                         bool keepDocTime, bool combineFolder, CWizProgressDialog* progress,
                                         CWizObjectDataDownloaderHost* downloader);

    void copyGroupFolderToGroupFolder(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder,
                                      bool keepDocTime, bool combineFolder, CWizProgressDialog* progress,
                                      CWizObjectDataDownloaderHost* downloader);
    //
    void copyPersonalFolder(const QString& sourceFolder, CWizFolderSelector* selector,
                            CWizProgressDialog* progress, CWizObjectDataDownloaderHost* downloader);

    void copyPersonalFolderToPersonalFolder(const QString& sourceFolder, const QString& targetParentFolder,
                                            bool keepDocTime, bool keepTag, bool combineFolder,
                                            CWizProgressDialog* progress, CWizObjectDataDownloaderHost* downloader);

    void copyPersonalFolderToGroupFolder(const QString& sourceFolder, const WIZTAGDATA& targetFolder,
                                         bool keepDocTime, bool combineFolder, CWizProgressDialog* progress, CWizObjectDataDownloaderHost* downloader);
    //
    void moveDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument, const WIZTAGDATA& targetTag);    

    //
    virtual void dropItemAsBrother(CWizCategoryViewItemBase* targetItem, CWizCategoryViewItemBase* dragedItem,
                                   bool dropAtTop, bool deleteDragSource);
    virtual void dropItemAsChild(CWizCategoryViewItemBase* targetItem, CWizCategoryViewItemBase* dragedItem,
                                 bool deleteDragSource);

    //
    QString getUseableItemName(QTreeWidgetItem* parent, \
                                QTreeWidgetItem* item);
    void resetFolderLocation(CWizCategoryViewFolderItem* item);
    void resetFolderLocation(CWizCategoryViewFolderItem* item, const QString& strNewLocation);
    bool renameFolder(CWizCategoryViewFolderItem* item, const QString& strFolderName);
    bool renameGroupFolder(CWizCategoryViewGroupItem* pGroup, const QString& strFolderName);
    //
    void updateShortcut(int type, const QString& keyValue, const QString& name);
    void removeShortcut(int type, const QString& keyValue);
    void removeShortcut(CWizCategoryViewItemBase* shortcut);

    //
    QTreeWidgetItem* findSameNameBrother(QTreeWidgetItem* parent, QTreeWidgetItem* exceptItem, const QString& name);
    bool isCombineSameNameFolder(const WIZTAGDATA& parentTag, const QString& folderName,
                                 bool& isCombine, QTreeWidgetItem* exceptBrother = nullptr);
    bool isCombineSameNameFolder(const QString& parentFolder, const QString& folderName,
                                 bool& isCombine, QTreeWidgetItem* exceptBrother = nullptr);

    bool combineGroupFolder(CWizCategoryViewGroupItem* sourceItem, CWizCategoryViewGroupItem* targetItem);

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
