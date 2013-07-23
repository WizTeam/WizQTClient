#include "wizCategoryViewItem.h"

#include "wizCategoryView.h"
#include "share/wizsettings.h"
#include "wiznotestyle.h"
#include "share/wizDatabaseManager.h"

/* ------------------------------ CWizCategoryViewItemBase ------------------------------ */

CWizCategoryViewItemBase::CWizCategoryViewItemBase(CWizExplorerApp& app,
                                                   const QString& strName,
                                                   const QString& strKbGUID)
    : QTreeWidgetItem()
    , m_app(app)
    , m_strName(strName)
    , m_strKbGUID(strKbGUID)
    , m_nDocuments(-1)
    , m_nDocumentsIncludeSub(-1)
{
}

QVariant CWizCategoryViewItemBase::data(int column, int role) const
{
    if (role == Qt::SizeHintRole) {
        int fontHeight = treeWidget()->fontMetrics().height();
        int defHeight = fontHeight + 8;
        int height = getItemHeight(defHeight);
        QSize sz(-1, height);
        return QVariant(sz);
    } else {
        return QTreeWidgetItem::data(column, role);
    }
}

int CWizCategoryViewItemBase::getItemHeight(int hintHeight) const
{
    return hintHeight;
}

bool CWizCategoryViewItemBase::operator < (const QTreeWidgetItem &other) const
{
    return text(0).compare(other.text(0), Qt::CaseInsensitive) < 0;
}

int CWizCategoryViewItemBase::getDocumentsCount(bool bIncludeSub)
{
    if (bIncludeSub) {
        return m_nDocumentsIncludeSub;
    } else {
        return m_nDocuments;
    }
}

void CWizCategoryViewItemBase::setDocumentsCount(int nCount, bool bIncludeSub)
{
    if (bIncludeSub) {
        m_nDocumentsIncludeSub = nCount;
    } else {
        m_nDocuments = nCount;
    }

    if (m_nDocumentsIncludeSub == -1) {
        countString = QString("(%1)").arg(m_nDocuments);
    } else {
        countString = QString("(%1/%2)").arg(m_nDocuments).arg(m_nDocumentsIncludeSub);
    }
}

/* ------------------------------ CWizCategoryViewSpacerItem ------------------------------ */

CWizCategoryViewSpacerItem::CWizCategoryViewSpacerItem(CWizExplorerApp& app)
    : CWizCategoryViewItemBase(app)
{
    setText(0, "=");
}

int CWizCategoryViewSpacerItem::getItemHeight(int hintHeight) const
{
    Q_UNUSED(hintHeight);
    return 12;
}

/* ------------------------------ CWizCategoryViewSeparatorItem ------------------------------ */

CWizCategoryViewSeparatorItem::CWizCategoryViewSeparatorItem(CWizExplorerApp& app)
    : CWizCategoryViewItemBase(app)
{
    setText(0, "-");
}

int CWizCategoryViewSeparatorItem::getItemHeight(int hintHeight) const
{
    Q_UNUSED(hintHeight);
    return 12;
}


/* ------------------------------ CWizCategoryViewAllFoldersItem ------------------------------ */

CWizCategoryViewAllFoldersItem::CWizCategoryViewAllFoldersItem(CWizExplorerApp& app,
                                                               const QString& strName,
                                                               const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, strName, strKbGUID)
{
    QIcon icon;
    QColor color = ::WizGetCategoryBackroundColor(app.userSettings().skin());
    ::WizLoadSkinIcon3(icon, app.userSettings().skin(), "folder", QIcon::Normal, QIcon::On, color);
    ::WizLoadSkinIcon3(icon, app.userSettings().skin(), "folder", QIcon::Selected, QIcon::On, color);
    setIcon(0, icon);

    setText(0, strName);

    //updateDocumentsCount();
}

void CWizCategoryViewAllFoldersItem::updateDocumentsCount()
{
    int nCount = 0;
    CWizDatabase& db = m_app.databaseManager().db(m_strKbGUID);
    db.GetAllDocumentsSize(nCount);
    setDocumentsCount(nCount, false);
}

void CWizCategoryViewAllFoldersItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    COleDateTime t = ::WizGetCurrentTime();
    t = t.addDays(-60);

    db.GetRecentDocumentsByCreatedTime(t, arrayDocument);
}

bool CWizCategoryViewAllFoldersItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (db.IsInDeletedItems(data.strLocation)) {
        return false;
    }

    COleDateTime t = data.tCreated;
    if (t.addDays(60) >= WizGetCurrentTime() && data.strKbGUID == kbGUID()) {
        return true;
    }

    return false;
}

void CWizCategoryViewAllFoldersItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
        view->showAllFoldersContextMenu(pos);
    }
}


/* ------------------------------ CWizCategoryViewFolderItem ------------------------------ */

CWizCategoryViewFolderItem::CWizCategoryViewFolderItem(CWizExplorerApp& app,
                                                       const QString& strLocation,
                                                       const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, strLocation, strKbGUID)
{
    QIcon icon;
    QColor color = ::WizGetCategoryBackroundColor(app.userSettings().skin());
    ::WizLoadSkinIcon3(icon, app.userSettings().skin(), "folder", QIcon::Normal, QIcon::On, color);
    ::WizLoadSkinIcon3(icon, app.userSettings().skin(), "folder", QIcon::Selected, QIcon::On, color);
    setIcon(0, icon);
    setText(0, CWizDatabase::GetLocationDisplayName(strLocation));

    //updateDocumentsCount();
}

void CWizCategoryViewFolderItem::updateDocumentsCount()
{
    int nCount = 0, nCount2 = 0;
    CWizDatabase& db = m_app.databaseManager().db(m_strKbGUID);
    db.GetDocumentsSizeByLocation(m_strName, nCount, false);
    setDocumentsCount(nCount, false);

    CWizStdStringArray arrayLocation;
    db.GetAllChildLocations(m_strName, arrayLocation);
    if (arrayLocation.size()) {
        db.GetDocumentsSizeByLocation(m_strName, nCount2, true);
        setDocumentsCount(nCount2, true);
    }
}

QTreeWidgetItem* CWizCategoryViewFolderItem::clone() const
{
    return new CWizCategoryViewFolderItem(m_app, m_strName, m_strKbGUID);
}

void CWizCategoryViewFolderItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.GetDocumentsByLocation(m_strName, arrayDocument);
}

bool CWizCategoryViewFolderItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    Q_UNUSED(db);

    if (m_strName == data.strLocation && data.strKbGUID == kbGUID()) {
        return true;
    }

    return false;
}

void CWizCategoryViewFolderItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
        view->showFolderContextMenu(pos);
    }
}

QString CWizCategoryViewFolderItem::name() const
{
    return CWizDatabase::GetLocationName(m_strName);
}


/* ------------------------------ CWizCategoryViewAllTagsItem ------------------------------ */

CWizCategoryViewAllTagsItem::CWizCategoryViewAllTagsItem(CWizExplorerApp& app,
                                                         const QString& strName,
                                                         const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, strName, strKbGUID)
{
    QIcon icon;
    QColor color = ::WizGetCategoryBackroundColor(app.userSettings().skin());
    ::WizLoadSkinIcon3(icon, app.userSettings().skin(), "tag", QIcon::Normal, QIcon::On, color);
    ::WizLoadSkinIcon3(icon, app.userSettings().skin(), "tag", QIcon::Selected, QIcon::On, color);
    setIcon(0, icon);
    setText(0, strName);

    //updateDocumentsCount();
}

void CWizCategoryViewAllTagsItem::updateDocumentsCount()
{
    int nCount;
    CWizDatabase& db = m_app.databaseManager().db(m_strKbGUID);
    db.getDocumentsSizeNoTag(nCount);
    setDocumentsCount(nCount, false);
}

void CWizCategoryViewAllTagsItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryTagsView* view = dynamic_cast<CWizCategoryTagsView *>(pCtrl)) {
        view->showAllTagsContextMenu(pos);
    }
}

void CWizCategoryViewAllTagsItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    // no deleted
    db.getDocumentsNoTag(arrayDocument);
}

bool CWizCategoryViewAllTagsItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    CString strTagGUIDs = db.GetDocumentTagGUIDsString(data.strGUID);
    if (strTagGUIDs.isEmpty() && data.strKbGUID == kbGUID()) {
        return true;
    }

    return false;
}


/* ------------------------------ CWizCategoryViewTagItem ------------------------------ */

CWizCategoryViewTagItem::CWizCategoryViewTagItem(CWizExplorerApp& app,
                                                 const WIZTAGDATA& tag,
                                                 const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, tag.strName, strKbGUID)
    , m_tag(tag)
{
    QIcon icon;
    QColor color = ::WizGetCategoryBackroundColor(app.userSettings().skin());
    ::WizLoadSkinIcon3(icon, app.userSettings().skin(), "tag", QIcon::Normal, QIcon::On, color);
    ::WizLoadSkinIcon3(icon, app.userSettings().skin(), "tag", QIcon::Selected, QIcon::On, color);
    setIcon(0, icon);
    setText(0, CWizDatabase::TagNameToDisplayName(tag.strName));

    //updateDocumentsCount();
}

void CWizCategoryViewTagItem::updateDocumentsCount()
{
    int nCount = 0, nCount2 = 0;
    CWizDatabase& db = m_app.databaseManager().db(m_strKbGUID);
    db.GetDocumentsSizeByTag(m_tag, nCount);
    setDocumentsCount(nCount, false);

    int nSizeTags = 0;
    db.GetAllChildTagsSize(m_tag.strGUID, nSizeTags);
    if (nSizeTags) {
        db.GetAllDocumentsSizeByTag(m_tag, nCount2);
        setDocumentsCount(nCount2, true);
    }
}

QTreeWidgetItem* CWizCategoryViewTagItem::clone() const
{
    return new CWizCategoryViewTagItem(m_app, m_tag, m_strKbGUID);
}

void CWizCategoryViewTagItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.GetDocumentsByTag(m_tag, arrayDocument);
}

bool CWizCategoryViewTagItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    CString strTagGUIDs = db.GetDocumentTagGUIDsString(data.strGUID);

    if (strTagGUIDs.Find(m_tag.strGUID) != -1 && data.strKbGUID == kbGUID()) {
        return true;
    }

    return false;
}

void CWizCategoryViewTagItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryTagsView* view = dynamic_cast<CWizCategoryTagsView *>(pCtrl)) {
        view->showTagContextMenu(pos);
    }
}

void CWizCategoryViewTagItem::reload(CWizDatabase& db)
{
    db.TagFromGUID(m_tag.strGUID, m_tag);
    setText(0, m_tag.strName);
}

/* ---------------------------- CWizCategoryViewAllGroupsRootItem ---------------------------- */

CWizCategoryViewAllGroupsRootItem::CWizCategoryViewAllGroupsRootItem(CWizExplorerApp& app,
                                                                     const QString& strName,
                                                                     const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, strName, strKbGUID)
{
    QIcon icon;
    QColor color = ::WizGetCategoryBackroundColor(app.userSettings().skin());
    ::WizLoadSkinIcon3(icon, app.userSettings().skin(), "groups", QIcon::Normal, QIcon::On, color);
    ::WizLoadSkinIcon3(icon, app.userSettings().skin(), "groups", QIcon::Selected, QIcon::On, color);
    setIcon(0, icon);

    setText(0, strName);
}

void CWizCategoryViewAllGroupsRootItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    Q_UNUSED(pCtrl);
    Q_UNUSED(pos);
}

void CWizCategoryViewAllGroupsRootItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    Q_UNUSED(db);
    Q_UNUSED(arrayDocument);
}

/* ------------------------------ CWizCategoryViewGroupRootItem ------------------------------ */
CWizCategoryViewBizGroupRootItem::CWizCategoryViewBizGroupRootItem(CWizExplorerApp& app,
                                                                   const QString& strName,
                                                                   const QString& strKbGUID)
    : CWizCategoryViewAllGroupsRootItem(app, strName, strKbGUID)
{
//    QIcon icon;
//    QColor color = ::WizGetCategoryBackroundColor(app.userSettings().skin());
//    ::WizLoadSkinIcon3(icon, app.userSettings().skin(), "groups", QIcon::Normal, QIcon::On, color);
//    ::WizLoadSkinIcon3(icon, app.userSettings().skin(), "groups", QIcon::Selected, QIcon::On, color);
//    setIcon(0, icon);
//    setText(0, strName);
}

void CWizCategoryViewBizGroupRootItem::updateDocumentsCount()
{
//    int nChild = childCount();
//    for (int i = 0; i > nChild; i++) {
//        CWizCategoryViewGroupRootItem* pItem = dynamic_cast<CWizCategoryViewGroupRootItem *>(child(i));
//        if (pItem) {
//            pItem->updateDocumentsCount();
//        }
//    }
//    int nCount = 0, nCount2 = 0;
//    CWizDatabase& db = m_app.databaseManager().db(m_strKbGUID);
//    db.getDocumentsSizeNoTag(nCount);
//    setDocumentsCount(nCount, false);
//    db.GetAllDocumentsSize(nCount2);
//    setDocumentsCount(nCount2, true);
}


/* ------------------------------ CWizCategoryViewGroupRootItem ------------------------------ */

CWizCategoryViewGroupRootItem::CWizCategoryViewGroupRootItem(CWizExplorerApp& app,
                                                             const QString& strName,
                                                             const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, strName, strKbGUID)
{
    QIcon icon;
    QColor color = ::WizGetCategoryBackroundColor(app.userSettings().skin());
    ::WizLoadSkinIcon3(icon, app.userSettings().skin(), "groups", QIcon::Normal, QIcon::On, color);
    ::WizLoadSkinIcon3(icon, app.userSettings().skin(), "groups", QIcon::Selected, QIcon::On, color);
    setIcon(0, icon);
    setText(0, strName);

    //updateDocumentsCount();
}

void CWizCategoryViewGroupRootItem::updateDocumentsCount()
{
    int nCount = 0, nCount2 = 0;
    CWizDatabase& db = m_app.databaseManager().db(m_strKbGUID);
    db.getDocumentsSizeNoTag(nCount);
    setDocumentsCount(nCount, false);
    db.GetAllDocumentsSize(nCount2);
    setDocumentsCount(nCount2, true);
}

void CWizCategoryViewGroupRootItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryGroupsView* view = dynamic_cast<CWizCategoryGroupsView *>(pCtrl)) {
        if (!m_strKbGUID.isEmpty()) {
            view->showGroupRootContextMenu(m_strKbGUID, pos);
        }
    }
}

void CWizCategoryViewGroupRootItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.getDocumentsNoTag(arrayDocument);
}

bool CWizCategoryViewGroupRootItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (db.IsInDeletedItems(data.strLocation)) {
        return false;
    }

    QString strTagGUIDs = db.GetDocumentTagGUIDsString(data.strGUID);
    if (strTagGUIDs.isEmpty() && data.strKbGUID == kbGUID()) {
        return true;
    }

    return false;
}

void CWizCategoryViewGroupRootItem::reload(CWizDatabase& db)
{
    setText(0, db.name());
}


/* ------------------------------ CWizCategoryViewGroupItem ------------------------------ */

CWizCategoryViewGroupItem::CWizCategoryViewGroupItem(CWizExplorerApp& app,
                                                     const WIZTAGDATA& tag,
                                                     const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, tag.strName, strKbGUID)
    , m_tag(tag)
{
    QIcon icon;
    QColor color = ::WizGetCategoryBackroundColor(app.userSettings().skin());
    ::WizLoadSkinIcon3(icon, app.userSettings().skin(), "groups", QIcon::Normal, QIcon::On, color);
    ::WizLoadSkinIcon3(icon, app.userSettings().skin(), "groups", QIcon::Selected, QIcon::On, color);
    setIcon(0, icon);
    setText(0, CWizDatabase::TagNameToDisplayName(tag.strName));

    //updateDocumentsCount();
}

void CWizCategoryViewGroupItem::updateDocumentsCount()
{
    int nCount = 0, nCount2 = 0;
    CWizDatabase& db = m_app.databaseManager().db(m_strKbGUID);
    db.GetDocumentsSizeByTag(m_tag, nCount);
    setDocumentsCount(nCount, false);

    int nSizeTags = 0;
    db.GetAllChildTagsSize(m_tag.strGUID, nSizeTags);
    if (nSizeTags) {
        db.GetAllDocumentsSizeByTag(m_tag, nCount2);
        setDocumentsCount(nCount2, true);
    }
}

void CWizCategoryViewGroupItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryGroupsView* view = dynamic_cast<CWizCategoryGroupsView *>(pCtrl)) {
        view->showGroupContextMenu(m_strKbGUID, pos);
    }
}

void CWizCategoryViewGroupItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.GetDocumentsByTag(m_tag, arrayDocument);
}

bool CWizCategoryViewGroupItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (db.IsInDeletedItems(data.strLocation)) {
        return false;
    }

    QString strTagGUIDs = db.GetDocumentTagGUIDsString(data.strGUID);
    if (strTagGUIDs.indexOf(m_tag.strGUID) != -1 && data.strKbGUID == kbGUID()) {
        return true;
    }

    return false;
}

void CWizCategoryViewGroupItem::reload(CWizDatabase& db)
{
    db.TagFromGUID(m_tag.strGUID, m_tag);
    setText(0, m_tag.strName);
}


/* ------------------------------ CWizCategoryViewTrashItem ------------------------------ */

CWizCategoryViewTrashItem::CWizCategoryViewTrashItem(CWizExplorerApp& app,
                                                     const QString& strName,
                                                     const QString& strKbGUID)
    : CWizCategoryViewFolderItem(app, "/Deleted Items/", strKbGUID)
{
    QIcon icon;
    QColor color = ::WizGetCategoryBackroundColor(app.userSettings().skin());
    ::WizLoadSkinIcon3(icon, app.userSettings().skin(), "trash", QIcon::Normal, QIcon::On, color);
    ::WizLoadSkinIcon3(icon, app.userSettings().skin(), "trash", QIcon::Selected, QIcon::On, color);
    setIcon(0, icon);

    setText(0, strName);
}

void CWizCategoryViewTrashItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryBaseView* view = dynamic_cast<CWizCategoryBaseView *>(pCtrl)) {
        view->showTrashContextMenu(pos, m_strKbGUID);
    }
}

void CWizCategoryViewTrashItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.GetDocumentsByLocation(db.GetDeletedItemsLocation(), arrayDocument, true);
}

bool CWizCategoryViewTrashItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    return db.IsInDeletedItems(data.strLocation);
}


/* ------------------------------ CWizCategoryViewSearchItem ------------------------------ */

//CWizCategoryViewSearchItem::CWizCategoryViewSearchItem(CWizExplorerApp& app, const QString& keywords)
//    : CWizCategoryViewItemBase(app, keywords)
//{
//    setKeywords(keywords);
//    setIcon(0, WizLoadSkinIcon(app.userSettings().skin(), QColor(0xff, 0xff, 0xff), "search"));
//}

//bool CWizCategoryViewSearchItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
//{
//    Q_UNUSED(db);

//    if (m_strName.isEmpty())
//        return false;

//    return -1 != ::WizStrStrI_Pos(data.strTitle, m_strName);
//}

//void CWizCategoryViewSearchItem::setKeywords(const QString& keywords)
//{
//    m_strName = keywords;

//    QString strText = QObject::tr("Search for %1").arg(m_strName);

//    setText(0, strText);
//}
