#include "WizFolderView.h"

#include <QHeaderView>


#include "widgets/WizScrollBar.h"

#include "WizDef.h"
#include "share/WizSettings.h"
#include "share/WizUIHelper.h"
#include "WizNoteStyle.h"

WizFolderView::WizFolderView(WizExplorerApp& app, QWidget *parent, bool showReadOnlyGroup)
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
    m_vScroll = new WizScrollBar(this);
    m_vScroll->syncWith(verticalScrollBar());
#endif

    //initFolders();
    //
    if (isDarkMode()) {
        QString darkStyleSheet = QString("background-color:%1").arg(WizColorLineEditorBackground.name());
        setStyleSheet(darkStyleSheet);
    }
}

void WizFolderView::resizeEvent(QResizeEvent* event)
{
#ifdef WIZNOTE_CUSTOM_SCROLLBAR
    // reset scrollbar
    m_vScroll->resize(m_vScroll->sizeHint().width(), event->size().height());
    m_vScroll->move(event->size().width() - m_vScroll->sizeHint().width(), 0);
#endif

    QTreeWidget::resizeEvent(event);
}

void WizFolderView::showEvent(QShowEvent *event)
{
    QTreeWidget::showEvent(event);
    clear();
    initFolders();
    initGroups();
    sortItems(0, Qt::AscendingOrder);
}

void WizFolderView::initFolders()
{
    WizCategoryViewAllFoldersItem* pAllFoldersItem = new WizCategoryViewAllFoldersItem(m_app, tr("Personal Notes"), m_dbMgr.db().kbGUID());
    addTopLevelItem(pAllFoldersItem);

    CWizStdStringArray arrayAllLocation;
    m_dbMgr.db().getAllLocations(arrayAllLocation);

    // folder cache
    CWizStdStringArray arrayExtLocation;
    m_dbMgr.db().getExtraFolder(arrayExtLocation);

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
        arrayAllLocation.push_back(m_dbMgr.db().getDefaultNoteLocation());
    }

    initFolders(pAllFoldersItem, "", arrayAllLocation);

    pAllFoldersItem->setExpanded(true);
    pAllFoldersItem->sortChildren(0, Qt::AscendingOrder);
}

void WizFolderView::initFolders(QTreeWidgetItem* pParent,
                                 const QString& strParentLocation,
                                 const CWizStdStringArray& arrayAllLocation)
{
    CWizStdStringArray arrayLocation;
    WizDatabase::getChildLocations(arrayAllLocation, strParentLocation, arrayLocation);

    CWizStdStringArray::const_iterator it;
    for (it = arrayLocation.begin(); it != arrayLocation.end(); it++) {
        QString strLocation = *it;

        if (m_dbMgr.db().isInDeletedItems(strLocation))
            continue;

        WizCategoryViewFolderItem* pFolderItem = new WizCategoryViewFolderItem(m_app, strLocation, m_dbMgr.db().kbGUID());
        pParent->addChild(pFolderItem);

        initFolders(pFolderItem, strLocation, arrayAllLocation);
    }
}

void WizFolderView::initGroups()
{
    CWizGroupDataArray arrayGroup;
    m_dbMgr.db().getAllGroupInfo(arrayGroup);

    //
    CWizBizDataArray arrayBiz;
    m_dbMgr.db().getAllBizInfo(arrayBiz);
    //
    std::vector<WizCategoryViewItemBase*> arrayGroupsItem;
    //
    for (CWizBizDataArray::const_iterator it = arrayBiz.begin();
         it != arrayBiz.end();
         it++)
    {
        const WIZBIZDATA& biz = *it;
        WizCategoryViewBizGroupRootItem* pBizGroupItem = new WizCategoryViewBizGroupRootItem(m_app, biz);

        addTopLevelItem(pBizGroupItem);
        pBizGroupItem->setExpanded(true);
        arrayGroupsItem.push_back(pBizGroupItem);
    }
    //
    CWizGroupDataArray arrayOwnGroup;
    WizDatabase::getOwnGroups(arrayGroup, arrayOwnGroup);
    if (!arrayOwnGroup.empty())
    {
        WizCategoryViewOwnGroupRootItem* pOwnGroupItem = new WizCategoryViewOwnGroupRootItem(m_app);
        addTopLevelItem(pOwnGroupItem);
        pOwnGroupItem->setExpanded(true);
        arrayGroupsItem.push_back(pOwnGroupItem);
    }
    //
    CWizGroupDataArray arrayJionedGroup;
    WizDatabase::getJionedGroups(arrayGroup, arrayJionedGroup);
    if (!arrayJionedGroup.empty())
    {
        WizCategoryViewJionedGroupRootItem* pJionedGroupItem = new WizCategoryViewJionedGroupRootItem(m_app);
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
    for (std::vector<WizCategoryViewItemBase*>::const_iterator it = arrayGroupsItem.begin();
         it != arrayGroupsItem.end();
         it++)
    {
        WizCategoryViewItemBase* pItem = *it;
        pItem->sortChildren(0, Qt::AscendingOrder);
    }
}

void WizFolderView::initGroup(WizDatabase& db)
{
    bool itemCreeated = false;
    if (findGroup(db.kbGUID()))
        return;
    //
    WIZGROUPDATA group;
    m_dbMgr.db().getGroupData(db.kbGUID(), group);
    //
    //
    QTreeWidgetItem* pRoot = findGroupsRootItem(group);
    if (!pRoot) {
        return;
    }

    itemCreeated = true;
    //
    WizCategoryViewGroupRootItem* pGroupItem = new WizCategoryViewGroupRootItem(m_app, group);
    pRoot->addChild(pGroupItem);

    //
    initGroup(db, pGroupItem, "");

    WizCategoryViewGroupNoTagItem* pGroupNoTagItem = new WizCategoryViewGroupNoTagItem(m_app, db.kbGUID());
    pGroupItem->addChild(pGroupNoTagItem);
    pGroupItem->sortChildren(0, Qt::AscendingOrder);
}

void WizFolderView::initGroup(WizDatabase& db, QTreeWidgetItem* pParent, const QString& strParentTagGUID)
{
    CWizTagDataArray arrayTag;
    db.getChildTags(strParentTagGUID, arrayTag);

    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        WizCategoryViewGroupItem* pTagItem = new WizCategoryViewGroupItem(m_app, *it, db.kbGUID());
        pParent->addChild(pTagItem);

        initGroup(db, pTagItem, it->strGUID);
    }
    //
    pParent->sortChildren(0, Qt::AscendingOrder);
}

WizCategoryViewFolderItem* WizFolderView::addFolder(const QString& strLocation, bool sort)
{
    return findFolder(strLocation, true, sort);
}

WizCategoryViewFolderItem* WizFolderView::findFolder(const QString& strLocation, bool create, bool sort)
{
    WizCategoryViewAllFoldersItem* pAllFolders = findAllFolders();
    if (!pAllFolders)
        return NULL;

    if (m_dbMgr.db().isInDeletedItems(strLocation)) {
        return findTrash(m_dbMgr.db().kbGUID());
    }

    QString strCurrentLocation = "/";
    QTreeWidgetItem* parent = pAllFolders;

    CString strTempLocation = strLocation;
    strTempLocation.trim('/');
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
            WizCategoryViewFolderItem* pFolder = dynamic_cast<WizCategoryViewFolderItem*>(parent->child(i));
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

        WizCategoryViewFolderItem* pFolderItem = new WizCategoryViewFolderItem(m_app, strCurrentLocation, m_dbMgr.db().kbGUID());
        parent->addChild(pFolderItem);
        parent->setExpanded(true);
        if (sort)
        {
            parent->sortChildren(0, Qt::AscendingOrder);
        }

        parent = pFolderItem;
    }

    return dynamic_cast<WizCategoryViewFolderItem *>(parent);
}

WizCategoryViewAllFoldersItem* WizFolderView::findAllFolders()
{
    int nCount = topLevelItemCount();
    for (int i = 0; i < nCount; i++)
    {
        if (WizCategoryViewAllFoldersItem* pItem = dynamic_cast<WizCategoryViewAllFoldersItem*>(topLevelItem(i)))
        {
            return pItem;
        }
    }

    Q_ASSERT(false);
    return NULL;
}

WizCategoryViewTrashItem* WizFolderView::findTrash(const QString& strKbGUID)
{
    Q_UNUSED(strKbGUID);

    int nCount = topLevelItemCount();
    for (int i = 0; i < nCount; i++) {
        if (WizCategoryViewTrashItem* pItem = dynamic_cast<WizCategoryViewTrashItem*>(topLevelItem(i))) {
            return pItem;
        }
    }

    return NULL;
}

WizCategoryViewGroupRootItem*WizFolderView::findGroup(const QString& strKbGUID)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        if (topLevelItem(i)->type() < Category_GroupsRootItem || topLevelItem(i)->type() > Category_JoinedGroupRootItem)
            return nullptr;

        WizCategoryViewGroupsRootItem* p = dynamic_cast<WizCategoryViewGroupsRootItem *>(topLevelItem(i));

        // only search under all groups root and biz group root
        if (!p)
            continue;

        for (int j = 0; j < p->childCount(); j++) {
            WizCategoryViewGroupRootItem* pGroup = dynamic_cast<WizCategoryViewGroupRootItem *>(p->child(j));
            if (pGroup && (pGroup->kbGUID() == strKbGUID)) {
                return pGroup;
            }
        }
    }

    return NULL;
}

WizCategoryViewItemBase*WizFolderView::findGroupsRootItem(const WIZGROUPDATA& group, bool bCreate)
{
    if (group.isBiz())
    {
        WIZBIZDATA biz;
        if (!m_dbMgr.db().getBizData(group.bizGUID, biz))
            return NULL;
        //
        return findBizGroupsRootItem(biz);
    }
    else
    {
        if (group.isOwn())
            return findOwnGroupsRootItem(bCreate);
        else
            return findJionedGroupsRootItem(bCreate);
    }
}

WizCategoryViewItemBase* WizFolderView::findBizGroupsRootItem(const WIZBIZDATA& biz, bool bCreate /*= true*/)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        WizCategoryViewBizGroupRootItem* pItem = dynamic_cast<WizCategoryViewBizGroupRootItem*>(topLevelItem(i));
        if (!pItem)
            continue;
        if (pItem->biz().bizGUID == biz.bizGUID)
            return pItem;
    }

    if (!bCreate)
        return NULL;
    //
    WizCategoryViewBizGroupRootItem* pItem = new WizCategoryViewBizGroupRootItem(m_app, biz);
    addTopLevelItem(pItem);
    //
    sortItems(0, Qt::AscendingOrder);
    //
    return pItem;
}

WizCategoryViewItemBase* WizFolderView::findOwnGroupsRootItem(bool bCreate /*= true*/)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        WizCategoryViewOwnGroupRootItem* pItem = dynamic_cast<WizCategoryViewOwnGroupRootItem*>(topLevelItem(i));
        if (!pItem)
            continue;
        //
        return pItem;
    }
    //
    if (!bCreate)
        return NULL;
    //
    WizCategoryViewOwnGroupRootItem* pItem = new WizCategoryViewOwnGroupRootItem(m_app);;
    addTopLevelItem(pItem);
    //
    sortItems(0, Qt::AscendingOrder);
    //
    return pItem;
}

WizCategoryViewItemBase* WizFolderView::findJionedGroupsRootItem(bool bCreate /*= true*/)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        WizCategoryViewJionedGroupRootItem* pItem = dynamic_cast<WizCategoryViewJionedGroupRootItem*>(topLevelItem(i));
        if (!pItem)
            continue;
        //
        return pItem;
    }
    //
    if (!bCreate)
        return NULL;
    //
    WizCategoryViewJionedGroupRootItem* pItem = new WizCategoryViewJionedGroupRootItem(m_app);;
    addTopLevelItem(pItem);
    //
    sortItems(0, Qt::AscendingOrder);
    //
    return pItem;
}
