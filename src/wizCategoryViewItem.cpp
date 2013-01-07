#include "wizCategoryViewItem.h"

#include "wizcategoryview.h"

CWizCategoryViewItemBase::CWizCategoryViewItemBase(CWizExplorerApp& app, const QString& strName)
    : QTreeWidgetItem()
    , m_app(app)
    , m_strName(strName)
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


CWizCategoryViewAllFoldersItem::CWizCategoryViewAllFoldersItem(CWizExplorerApp& app, const QString& str)
    : CWizCategoryViewItemBase(app, str)
{
    setText(0, str);
    setIcon(0, ::WizLoadSkinIcon(app.userSettings().skin(), "folders"));
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
    return t.addDays(60) >= WizGetCurrentTime();
}

void CWizCategoryViewAllFoldersItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
        view->showAllFoldersContextMenu(pos);
    }
}


CWizCategoryViewFolderItem::CWizCategoryViewFolderItem(CWizExplorerApp& app, const QString& strLocation)
    : CWizCategoryViewItemBase(app, strLocation)
{
    setText(0, CWizDatabase::GetLocationDisplayName(strLocation));
    setIcon(0, ::WizLoadSkinIcon(m_app.userSettings().skin(), "folder"));
}

QTreeWidgetItem* CWizCategoryViewFolderItem::clone() const
{
    return new CWizCategoryViewFolderItem(m_app, m_strName);
}

void CWizCategoryViewFolderItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.GetDocumentsByLocation(m_strName, arrayDocument);
}

bool CWizCategoryViewFolderItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    Q_UNUSED(db);
    return m_strName == data.strLocation;
}

void CWizCategoryViewFolderItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
        view->showFolderContextMenu(pos);
    }
}


CWizCategoryViewAllTagsItem::CWizCategoryViewAllTagsItem(CWizExplorerApp& app, const QString& str)
    : CWizCategoryViewItemBase(app, str)
{
    setText(0, str);
    setIcon(0, ::WizLoadSkinIcon(app.userSettings().skin(), "tags"));
}

void CWizCategoryViewAllTagsItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryTagsView* view = dynamic_cast<CWizCategoryTagsView *>(pCtrl)) {
        view->showAllTagsContextMenu(pos);
    }
}


CWizCategoryViewTagItem::CWizCategoryViewTagItem(CWizExplorerApp& app, const WIZTAGDATA& tag)
    : CWizCategoryViewItemBase(app)
    , m_tag(tag)
{
    setText(0, CWizDatabase::TagNameToDisplayName(tag.strName));
    setIcon(0, ::WizLoadSkinIcon(app.userSettings().skin(), "tag"));
}

QTreeWidgetItem* CWizCategoryViewTagItem::clone() const
{
    return new CWizCategoryViewTagItem(m_app, m_tag);
}

void CWizCategoryViewTagItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.GetDocumentsByTag(m_tag, arrayDocument);
}

bool CWizCategoryViewTagItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    CString strTagGUIDs = db.GetDocumentTagGUIDsString(data.strGUID);
    return -1 != strTagGUIDs.Find(m_tag.strGUID);
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

const WIZTAGDATA& CWizCategoryViewTagItem::tag() const
{
    return m_tag;
}


CWizCategoryViewTrashItem::CWizCategoryViewTrashItem(CWizExplorerApp& app, const QString& str)
    : CWizCategoryViewFolderItem(app, "/Deleted Items/")
{
    setText(0, str);
    setIcon(0, ::WizLoadSkinIcon(app.userSettings().skin(), "trash"));
}

void CWizCategoryViewTrashItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
        view->showTrashContextMenu(pos);
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


CWizCategoryViewSearchItem::CWizCategoryViewSearchItem(CWizExplorerApp& app, const QString& keywords)
    : CWizCategoryViewItemBase(app, keywords)
{
    setKeywords(keywords);
    setIcon(0, WizLoadSkinIcon(app.userSettings().skin(), "search"));
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
