#include "wizFolderView.h"

#include <QHeaderView>


#include "widgets/wizScrollBar.h"

#include "wizdef.h"
#include "share/wizsettings.h"
#include "share/wizuihelper.h"
#include "wiznotestyle.h"

CWizFolderView::CWizFolderView(CWizExplorerApp& app, QWidget *parent, bool showReadOnlyGroup)
    : QTreeWidget(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_showReadOnlyGroup(showReadOnlyGroup)
{
    header()->hide();
    setAnimated(true);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    //setStyle(::WizGetStyle(m_app.userSettings().skin()));

    setVerticalScrollMode(QAbstractItemView::ScrollPerItem);

#ifdef WIZNOTE_CUSTOM_SCROLLBAR
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_vScroll = new CWizScrollBar(this);
    m_vScroll->syncWith(verticalScrollBar());
#endif

    //initFolders();
}

void CWizFolderView::resizeEvent(QResizeEvent* event)
{
#ifdef WIZNOTE_CUSTOM_SCROLLBAR
    // reset scrollbar
    m_vScroll->resize(m_vScroll->sizeHint().width(), event->size().height());
    m_vScroll->move(event->size().width() - m_vScroll->sizeHint().width(), 0);
#endif

    QTreeWidget::resizeEvent(event);
}

void CWizFolderView::showEvent(QShowEvent *event)
{
    QTreeWidget::showEvent(event);
    clear();
    initFolders();
    initGroups();
    sortItems(0, Qt::AscendingOrder);
}

void CWizFolderView::initFolders()
{
    CWizCategoryViewAllFoldersItem* pAllFoldersItem = new CWizCategoryViewAllFoldersItem(m_app, tr("Note Folders"), m_dbMgr.db().kbGUID());
    addTopLevelItem(pAllFoldersItem);

    CWizStdStringArray arrayAllLocation;
    m_dbMgr.db().GetAllLocations(arrayAllLocation);

    // folder cache
    CWizStdStringArray arrayExtLocation;
    m_dbMgr.db().GetExtraFolder(arrayExtLocation);

    if (!arrayExtLocation.empty()) {
        for (CWizStdStringArray::const_iterator it = arrayExtLocation.begin();
             it != arrayExtLocation.end();
             it++) {
            if (-1 == ::WizFindInArray(arrayAllLocation, *it)) {
                arrayAllLocation.push_back(*it);
            }
        }
    }

    if (arrayAllLocation.empty()) {
        arrayAllLocation.push_back(m_dbMgr.db().GetDefaultNoteLocation());
    }

    initFolders(pAllFoldersItem, "", arrayAllLocation);

    pAllFoldersItem->setExpanded(true);
    pAllFoldersItem->sortChildren(0, Qt::AscendingOrder);
}

void CWizFolderView::initFolders(QTreeWidgetItem* pParent,
                                 const QString& strParentLocation,
                                 const CWizStdStringArray& arrayAllLocation)
{
    CWizStdStringArray arrayLocation;
    CWizDatabase::GetChildLocations(arrayAllLocation, strParentLocation, arrayLocation);

    CWizStdStringArray::const_iterator it;
    for (it = arrayLocation.begin(); it != arrayLocation.end(); it++) {
        QString strLocation = *it;

        if (m_dbMgr.db().IsInDeletedItems(strLocation))
            continue;

        CWizCategoryViewFolderItem* pFolderItem = new CWizCategoryViewFolderItem(m_app, strLocation, m_dbMgr.db().kbGUID());
        pParent->addChild(pFolderItem);

        initFolders(pFolderItem, strLocation, arrayAllLocation);
    }
}

void CWizFolderView::initGroups()
{
    CWizGroupDataArray arrayGroup;
    m_dbMgr.db().GetUserGroupInfo(arrayGroup);

    //
    CWizBizDataArray arrayBiz;
    m_dbMgr.db().GetUserBizInfo(false, arrayGroup, arrayBiz);
    //
    std::vector<CWizCategoryViewItemBase*> arrayGroupsItem;
    //
    for (CWizBizDataArray::const_iterator it = arrayBiz.begin();
         it != arrayBiz.end();
         it++)
    {
        const WIZBIZDATA& biz = *it;
        CWizCategoryViewBizGroupRootItem* pBizGroupItem = new CWizCategoryViewBizGroupRootItem(m_app, biz);

        addTopLevelItem(pBizGroupItem);
        pBizGroupItem->setExpanded(true);
        arrayGroupsItem.push_back(pBizGroupItem);
    }
    //
    CWizGroupDataArray arrayOwnGroup;
    CWizDatabase::GetOwnGroups(arrayGroup, arrayOwnGroup);
    if (!arrayOwnGroup.empty())
    {
        CWizCategoryViewOwnGroupRootItem* pOwnGroupItem = new CWizCategoryViewOwnGroupRootItem(m_app);
        addTopLevelItem(pOwnGroupItem);
        pOwnGroupItem->setExpanded(true);
        arrayGroupsItem.push_back(pOwnGroupItem);
    }
    //
    CWizGroupDataArray arrayJionedGroup;
    CWizDatabase::GetJionedGroups(arrayGroup, arrayJionedGroup);
    if (!arrayJionedGroup.empty())
    {
        CWizCategoryViewJionedGroupRootItem* pJionedGroupItem = new CWizCategoryViewJionedGroupRootItem(m_app);
        addTopLevelItem(pJionedGroupItem);
        pJionedGroupItem->setExpanded(true);
        arrayGroupsItem.push_back(pJionedGroupItem);
    }

    int nTotal = m_dbMgr.count();
    for (int i = 0; i < nTotal; i++) {
//        if (!m_showReadOnlyGroup && !m_dbMgr.at(i).IsGroupAuthor())
//            continue;

        initGroup(m_dbMgr.at(i));
    }
    //
    for (std::vector<CWizCategoryViewItemBase*>::const_iterator it = arrayGroupsItem.begin();
         it != arrayGroupsItem.end();
         it++)
    {
        CWizCategoryViewItemBase* pItem = *it;
        pItem->sortChildren(0, Qt::AscendingOrder);
    }
}

void CWizFolderView::initGroup(CWizDatabase& db)
{
    bool itemCreeated = false;
    if (findGroup(db.kbGUID()))
        return;
    //
    WIZGROUPDATA group;
    m_dbMgr.db().GetGroupData(db.kbGUID(), group);
    //
    //
    QTreeWidgetItem* pRoot = findGroupsRootItem(group);
    if (!pRoot) {
        return;
    }

    itemCreeated = true;
    //
    CWizCategoryViewGroupRootItem* pGroupItem = new CWizCategoryViewGroupRootItem(m_app, group);
    pRoot->addChild(pGroupItem);

    //
    initGroup(db, pGroupItem, "");

    CWizCategoryViewGroupNoTagItem* pGroupNoTagItem = new CWizCategoryViewGroupNoTagItem(m_app, db.kbGUID());
    pGroupItem->addChild(pGroupNoTagItem);
    pGroupItem->sortChildren(0, Qt::AscendingOrder);
}

void CWizFolderView::initGroup(CWizDatabase& db, QTreeWidgetItem* pParent, const QString& strParentTagGUID)
{
    CWizTagDataArray arrayTag;
    db.GetChildTags(strParentTagGUID, arrayTag);

    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        CWizCategoryViewGroupItem* pTagItem = new CWizCategoryViewGroupItem(m_app, *it, db.kbGUID());
        pParent->addChild(pTagItem);

        initGroup(db, pTagItem, it->strGUID);
    }
    //
    pParent->sortChildren(0, Qt::AscendingOrder);
}

CWizCategoryViewFolderItem* CWizFolderView::addFolder(const QString& strLocation, bool sort)
{
    return findFolder(strLocation, true, sort);
}

CWizCategoryViewFolderItem* CWizFolderView::findFolder(const QString& strLocation, bool create, bool sort)
{
    CWizCategoryViewAllFoldersItem* pAllFolders = findAllFolders();
    if (!pAllFolders)
        return NULL;

    if (m_dbMgr.db().IsInDeletedItems(strLocation)) {
        return findTrash(m_dbMgr.db().kbGUID());
    }

    QString strCurrentLocation = "/";
    QTreeWidgetItem* parent = pAllFolders;

    CString strTempLocation = strLocation;
    strTempLocation.Trim('/');
    QStringList sl = strTempLocation.split("/");

    QStringList::const_iterator it;
    for (it = sl.begin(); it != sl.end(); it++) {
        QString strLocationName = *it;
        Q_ASSERT(!strLocationName.isEmpty());
        strCurrentLocation = strCurrentLocation + strLocationName + "/";

        bool found = false;
        int nCount = parent->childCount();
        for (int i = 0; i < nCount; i++)
        {
            CWizCategoryViewFolderItem* pFolder = dynamic_cast<CWizCategoryViewFolderItem*>(parent->child(i));
            if (pFolder
                && pFolder->name() == strLocationName)
            {
                found = true;
                parent = pFolder;
                continue;
            }
        }

        if (found)
            continue;

        if (!create)
            return NULL;

        CWizCategoryViewFolderItem* pFolderItem = new CWizCategoryViewFolderItem(m_app, strCurrentLocation, m_dbMgr.db().kbGUID());
        parent->addChild(pFolderItem);
        parent->setExpanded(true);
        if (sort)
        {
            parent->sortChildren(0, Qt::AscendingOrder);
        }

        parent = pFolderItem;
    }

    return dynamic_cast<CWizCategoryViewFolderItem *>(parent);
}

CWizCategoryViewAllFoldersItem* CWizFolderView::findAllFolders()
{
    int nCount = topLevelItemCount();
    for (int i = 0; i < nCount; i++)
    {
        if (CWizCategoryViewAllFoldersItem* pItem = dynamic_cast<CWizCategoryViewAllFoldersItem*>(topLevelItem(i)))
        {
            return pItem;
        }
    }

    Q_ASSERT(false);
    return NULL;
}

CWizCategoryViewTrashItem* CWizFolderView::findTrash(const QString& strKbGUID)
{
    Q_UNUSED(strKbGUID);

    int nCount = topLevelItemCount();
    for (int i = 0; i < nCount; i++) {
        if (CWizCategoryViewTrashItem* pItem = dynamic_cast<CWizCategoryViewTrashItem*>(topLevelItem(i))) {
            return pItem;
        }
    }

    return NULL;
}

CWizCategoryViewGroupRootItem*CWizFolderView::findGroup(const QString& strKbGUID)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        if (topLevelItem(i)->type() < Category_GroupsRootItem || topLevelItem(i)->type() > Category_JoinedGroupRootItem)
            return nullptr;

        CWizCategoryViewGroupsRootItem* p = dynamic_cast<CWizCategoryViewGroupsRootItem *>(topLevelItem(i));

        // only search under all groups root and biz group root
        if (!p)
            continue;

        for (int j = 0; j < p->childCount(); j++) {
            CWizCategoryViewGroupRootItem* pGroup = dynamic_cast<CWizCategoryViewGroupRootItem *>(p->child(j));
            if (pGroup && (pGroup->kbGUID() == strKbGUID)) {
                return pGroup;
            }
        }
    }

    return NULL;
}

CWizCategoryViewItemBase*CWizFolderView::findGroupsRootItem(const WIZGROUPDATA& group, bool bCreate)
{
    if (group.IsBiz())
    {
        WIZBIZDATA biz;
        if (!m_dbMgr.db().GetBizData(group.bizGUID, biz))
            return NULL;
        //
        return findBizGroupsRootItem(biz);
    }
    else
    {
        if (group.IsOwn())
            return findOwnGroupsRootItem(bCreate);
        else
            return findJionedGroupsRootItem(bCreate);
    }
}

CWizCategoryViewItemBase* CWizFolderView::findBizGroupsRootItem(const WIZBIZDATA& biz, bool bCreate /*= true*/)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        CWizCategoryViewBizGroupRootItem* pItem = dynamic_cast<CWizCategoryViewBizGroupRootItem*>(topLevelItem(i));
        if (!pItem)
            continue;
        if (pItem->biz().bizGUID == biz.bizGUID)
            return pItem;
    }

    if (!bCreate)
        return NULL;
    //
    CWizCategoryViewBizGroupRootItem* pItem = new CWizCategoryViewBizGroupRootItem(m_app, biz);
    addTopLevelItem(pItem);
    //
    sortItems(0, Qt::AscendingOrder);
    //
    return pItem;
}

CWizCategoryViewItemBase* CWizFolderView::findOwnGroupsRootItem(bool bCreate /*= true*/)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        CWizCategoryViewOwnGroupRootItem* pItem = dynamic_cast<CWizCategoryViewOwnGroupRootItem*>(topLevelItem(i));
        if (!pItem)
            continue;
        //
        return pItem;
    }
    //
    if (!bCreate)
        return NULL;
    //
    CWizCategoryViewOwnGroupRootItem* pItem = new CWizCategoryViewOwnGroupRootItem(m_app);;
    addTopLevelItem(pItem);
    //
    sortItems(0, Qt::AscendingOrder);
    //
    return pItem;
}

CWizCategoryViewItemBase* CWizFolderView::findJionedGroupsRootItem(bool bCreate /*= true*/)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        CWizCategoryViewJionedGroupRootItem* pItem = dynamic_cast<CWizCategoryViewJionedGroupRootItem*>(topLevelItem(i));
        if (!pItem)
            continue;
        //
        return pItem;
    }
    //
    if (!bCreate)
        return NULL;
    //
    CWizCategoryViewJionedGroupRootItem* pItem = new CWizCategoryViewJionedGroupRootItem(m_app);;
    addTopLevelItem(pItem);
    //
    sortItems(0, Qt::AscendingOrder);
    //
    return pItem;
}
