#include "wizFolderView.h"

#include <QtWidgets>

#include "widgets/wizScrollBar.h"

#include "wizdef.h"
#include "share/wizsettings.h"
#include "share/wizuihelper.h"
#include "wiznotestyle.h"

CWizFolderView::CWizFolderView(CWizExplorerApp& app, QWidget *parent)
    : QTreeWidget(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
{
    header()->hide();
    setAnimated(true);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setStyle(::WizGetStyle(m_app.userSettings().skin()));

    // use custom scrollbar
    setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_vScroll = new CWizScrollBar(this);
    m_vScroll->syncWith(verticalScrollBar());

    initFolders();
}

void CWizFolderView::resizeEvent(QResizeEvent* event)
{
    // reset scrollbar
    m_vScroll->resize(m_vScroll->sizeHint().width(), event->size().height());
    m_vScroll->move(event->size().width() - m_vScroll->sizeHint().width(), 0);

    QTreeWidget::resizeEvent(event);
}

void CWizFolderView::initFolders()
{
    CWizCategoryViewAllFoldersItem* pAllFoldersItem = new CWizCategoryViewAllFoldersItem(m_app, tr("Note Folders"), m_dbMgr.db().kbGUID());
    addTopLevelItem(pAllFoldersItem);

    CWizStdStringArray arrayAllLocation;
    m_dbMgr.db().GetAllLocations(arrayAllLocation);

    initFolders(pAllFoldersItem, "", arrayAllLocation);

    if (arrayAllLocation.empty()) {
        const QString strNotes("/My Notes/");
        m_dbMgr.db().AddExtraFolder(strNotes);
        m_dbMgr.db().SetLocalValueVersion("folders", -1);
        arrayAllLocation.push_back(strNotes);
    }

    //init extra folders
    CWizStdStringArray arrayExtLocation;
    m_dbMgr.db().GetExtraFolder(arrayExtLocation);

    CWizStdStringArray::const_iterator it;
    for (it = arrayExtLocation.begin(); it != arrayExtLocation.end(); it++) {
        QString strLocation = *it;

        if (strLocation.isEmpty())
            continue;

        if (m_dbMgr.db().IsInDeletedItems(strLocation))
            continue;

        addFolder(strLocation, true);
    }

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
