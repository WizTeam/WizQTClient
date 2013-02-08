#include "wizCategoryViewItem.h"

#include "wizcategoryview.h"
#include "share/wizsettings.h"
#include "wiznotestyle.h"

/* ------------------------------ CWizCategoryViewItemBase ------------------------------ */

CWizCategoryViewItemBase::CWizCategoryViewItemBase(CWizExplorerApp& app,
                                                   const QString& strName,
                                                   const QString& strKbGUID)
    : QTreeWidgetItem()
    , m_app(app)
    , m_strName(strName)
    , m_strKbGUID(strKbGUID)
{
}

QVariant CWizCategoryViewItemBase::data(int column, int role) const
{
    if (role == Qt::SizeHintRole) {
        int fontHeight = treeWidget()->fontMetrics().height();
        int defHeight = fontHeight + 6;
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


/* ------------------------------ CWizCategoryViewSeparatorItem ------------------------------ */

CWizCategoryViewSeparatorItem::CWizCategoryViewSeparatorItem(CWizExplorerApp& app)
    : CWizCategoryViewItemBase(app)
{
    setText(0, "-");
}

int CWizCategoryViewSeparatorItem::getItemHeight(int hintHeight) const
{
    Q_UNUSED(hintHeight);
    return 8;
}


/* ------------------------------ CWizCategoryViewAllFoldersItem ------------------------------ */

CWizCategoryViewAllFoldersItem::CWizCategoryViewAllFoldersItem(CWizExplorerApp& app,
                                                               const QString& strName,
                                                               const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, strName, strKbGUID)
{
    QColor color = ::WizGetCategoryBackroundColor(app.userSettings().skin());

    setText(0, strName);
    setIcon(0, ::WizLoadSkinIcon(app.userSettings().skin(), color, "folder"));
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
    QColor color = ::WizGetCategoryBackroundColor(app.userSettings().skin());

    setText(0, CWizDatabase::GetLocationDisplayName(strLocation));
    setIcon(0, ::WizLoadSkinIcon(m_app.userSettings().skin(), color, "folder"));
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


/* ------------------------------ CWizCategoryViewAllTagsItem ------------------------------ */

CWizCategoryViewAllTagsItem::CWizCategoryViewAllTagsItem(CWizExplorerApp& app,
                                                         const QString& strName,
                                                         const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, strName, strKbGUID)
{
    QColor color = ::WizGetCategoryBackroundColor(app.userSettings().skin());

    setText(0, strName);
    setIcon(0, ::WizLoadSkinIcon(app.userSettings().skin(), color, "tags"));
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
    db.getDocumentsNoTag(arrayDocument, false);
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
    QColor color = ::WizGetCategoryBackroundColor(app.userSettings().skin());

    setText(0, CWizDatabase::TagNameToDisplayName(tag.strName));
    setIcon(0, ::WizLoadSkinIcon(app.userSettings().skin(), color, "tag"));
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


/* ------------------------------ CWizCategoryViewGroupRootItem ------------------------------ */

CWizCategoryViewGroupRootItem::CWizCategoryViewGroupRootItem(CWizExplorerApp& app,
                                                             const QString& strName,
                                                             const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, strName, strKbGUID)
{
    QColor color = ::WizGetCategoryBackroundColor(app.userSettings().skin());

    setText(0, strName);
    setIcon(0, ::WizLoadSkinIcon(app.userSettings().skin(), color, "groups"));
}

void CWizCategoryViewGroupRootItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryGroupsView* view = dynamic_cast<CWizCategoryGroupsView *>(pCtrl)) {
        if (!m_strKbGUID.isEmpty()) {
            view->showGroupRootContextMenu(m_strKbGUID, pos);
        }
    }
}

void CWizCategoryViewGroupRootItem::getDocuments(CWizDatabase& db,
                                                 CWizDocumentDataArray& arrayDocument)
{
    db.getDocumentsNoTag(arrayDocument, false);
}

bool CWizCategoryViewGroupRootItem::accept(CWizDatabase& db,
                                           const WIZDOCUMENTDATA& data)
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
    QColor color = ::WizGetCategoryBackroundColor(app.userSettings().skin());

    setText(0, CWizDatabase::TagNameToDisplayName(tag.strName));
    setIcon(0, ::WizLoadSkinIcon(app.userSettings().skin(), color, "tag"));
}

void CWizCategoryViewGroupItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryGroupsView* view = dynamic_cast<CWizCategoryGroupsView *>(pCtrl)) {
        view->showGroupContextMenu(m_strKbGUID, pos);
    }
}

void CWizCategoryViewGroupItem::getDocuments(CWizDatabase& db,
                                           CWizDocumentDataArray& arrayDocument)
{
    db.GetDocumentsByTag(m_tag, arrayDocument);
}

bool CWizCategoryViewGroupItem::accept(CWizDatabase& db,
                                       const WIZDOCUMENTDATA& data)
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
    QColor color = ::WizGetCategoryBackroundColor(app.userSettings().skin());

    setText(0, strName);
    setIcon(0, ::WizLoadSkinIcon(app.userSettings().skin(), color, "trash"));
}

void CWizCategoryViewTrashItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryBaseView* view = dynamic_cast<CWizCategoryBaseView *>(pCtrl)) {
        view->showTrashContextMenu(pos, m_strKbGUID);
    }
}

void CWizCategoryViewTrashItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.GetDocumentsByLocationIncludeSubFolders(db.GetDeletedItemsLocation(), arrayDocument);
}

bool CWizCategoryViewTrashItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    return db.IsInDeletedItems(data.strLocation);
}


/* ------------------------------ CWizCategoryViewSearchItem ------------------------------ */

CWizCategoryViewSearchItem::CWizCategoryViewSearchItem(CWizExplorerApp& app, const QString& keywords)
    : CWizCategoryViewItemBase(app, keywords)
{
    setKeywords(keywords);
    setIcon(0, WizLoadSkinIcon(app.userSettings().skin(), QColor(0xff, 0xff, 0xff), "search"));
}

bool CWizCategoryViewSearchItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    Q_UNUSED(db);

    if (m_strName.isEmpty())
        return false;

    return -1 != ::WizStrStrI_Pos(data.strTitle, m_strName);
}

void CWizCategoryViewSearchItem::setKeywords(const QString& keywords)
{
    m_strName = keywords;

    QString strText = QObject::tr("Search for %1").arg(m_strName);

    setText(0, strText);
}
