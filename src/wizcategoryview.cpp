#include "wizcategoryview.h"

#include "share/wizdrawtexthelper.h"
#include "wiznotestyle.h"

#include "newfolderdialog.h"
#include "newtagdialog.h"


#include <QHeaderView>
#include <QPalette>
#include <QContextMenuEvent>
#include <QMenu>
#include <QCursor>

class CWizCategoryViewItem : public QTreeWidgetItem
{
public:
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument) { Q_UNUSED(db); Q_UNUSED(arrayDocument); }
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data) { Q_UNUSED(db); Q_UNUSED(data); return false; }
    virtual void showContextMenu(CWizCategoryView* pCtrl, QPoint pos) { Q_UNUSED(pCtrl); Q_UNUSED(pos); }

    virtual QVariant data(int column, int role) const
    {
        if (role == Qt::SizeHintRole)
        {
            int fontHeight = treeWidget()->fontMetrics().height();
            int defHeight = fontHeight + 6;
            int height = getItemHeight(defHeight);
            QSize sz(-1, height);
            return QVariant(sz);
        }
        else
        {
            return QTreeWidgetItem::data(column, role);
        }
    }

    virtual int getItemHeight(int hintHeight) const
    {
        return hintHeight;
    }
    virtual bool operator<(const QTreeWidgetItem &other) const
    {
        return text(0).compare(other.text(0), Qt::CaseInsensitive) < 0;
    }
};

class CWizCategoryViewSeparatorItem : public CWizCategoryViewItem
{
public:
    CWizCategoryViewSeparatorItem()
    {
        setText(0, "-");
    }
    virtual int getItemHeight(int hintHeight) const
    {
        Q_UNUSED(hintHeight);
        return 8;
    }
};

class CWizCategoryViewAllFoldersItem : public CWizCategoryViewItem
{
public:
    CWizCategoryViewAllFoldersItem(const CString& str)
    {
        setText(0, str);
        setIcon(0, WizLoadSkinIcon("folders"));
    }
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
    {
        COleDateTime t = ::WizGetCurrentTime();
        t = t.addDays(-60);
        //
        db.GetRecentDocumentsByCreatedTime(t, arrayDocument);
    }
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
    {
        if (db.IsInDeletedItems(data.strLocation))
        {
            return false;
        }
        //
        COleDateTime t = data.tCreated;
        return t.addDays(60) >= WizGetCurrentTime();
    }
    virtual void showContextMenu(CWizCategoryView* pCtrl, QPoint pos)
    {
        pCtrl->showAllFoldersContextMenu(pos);
    }
};

class CWizCategoryViewFolderItem : public CWizCategoryViewItem
{
    CString m_strLocation;
public:
    CWizCategoryViewFolderItem(const CString& strLocation)
        : m_strLocation(strLocation)
    {
        setText(0, CWizDatabase::GetLocationDisplayName(strLocation));
        setIcon(0, WizLoadSkinIcon("folder"));
    }
    virtual QTreeWidgetItem *clone() const
    {
        return new CWizCategoryViewFolderItem(m_strLocation);
    }
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
    {
        db.GetDocumentsByLocation(m_strLocation, arrayDocument);
    }
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
    {
        Q_UNUSED(db);
        return m_strLocation == data.strLocation;
    }
    virtual void showContextMenu(CWizCategoryView* pCtrl, QPoint pos)
    {
        pCtrl->showFolderContextMenu(pos);
    }


    CString location() const { return m_strLocation; }
    CString name() const { return CWizDatabase::GetLocationName(m_strLocation); }
};



class CWizCategoryViewAllTagsItem : public CWizCategoryViewItem
{
public:
    CWizCategoryViewAllTagsItem(const CString& str)
    {
        setText(0, str);
        setIcon(0, WizLoadSkinIcon("tags"));
    }
    virtual void showContextMenu(CWizCategoryView* pCtrl, QPoint pos)
    {
        pCtrl->showAllTagsContextMenu(pos);
    }
};

class CWizCategoryViewTagItem : public CWizCategoryViewItem
{
    WIZTAGDATA m_tag;
public:
    CWizCategoryViewTagItem(const WIZTAGDATA& tag)
        : m_tag(tag)
    {
        setText(0, tag.strName);
        setIcon(0, WizLoadSkinIcon("tag"));
    }
    virtual QTreeWidgetItem *clone() const
    {
        return new CWizCategoryViewTagItem(m_tag);
    }
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
    {
        db.GetDocumentsByTag(m_tag, arrayDocument);
    }
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
    {
        CString strTagGUIDs = db.GetDocumentTagGUIDsString(data.strGUID);
        return -1 != strTagGUIDs.Find(m_tag.strGUID);
    }
    virtual void showContextMenu(CWizCategoryView* pCtrl, QPoint pos)
    {
        pCtrl->showTagContextMenu(pos);
    }
    void reload(CWizDatabase& db)
    {
        db.TagFromGUID(m_tag.strGUID, m_tag);
        setText(0, m_tag.strName);
    }
    const WIZTAGDATA& tag() const
    {
        return m_tag;
    }
};


class CWizCategoryViewTrashItem : public CWizCategoryViewFolderItem
{
public:
    CWizCategoryViewTrashItem(const CString& str)
        : CWizCategoryViewFolderItem("/Deleted Items/")
    {
        setText(0, str);
        setIcon(0, WizLoadSkinIcon("trash"));
    }
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
    {
        db.GetDocumentsByLocationIncludeSubFolders(db.GetDeletedItemsLocation(), arrayDocument);
    }
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
    {
        return db.IsInDeletedItems(data.strLocation);
    }
    virtual void showContextMenu(CWizCategoryView* pCtrl, QPoint pos)
    {
        pCtrl->showTrashContextMenu(pos);
    }
};



class CWizCategoryViewSearchItem : public CWizCategoryViewItem
{
    CString m_strKeywords;
public:
    CWizCategoryViewSearchItem(const CString& keywords)
    {
        setKeywords(keywords);
        setIcon(0, WizLoadSkinIcon("search"));
    }
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
    {
        db.Search(m_strKeywords, "", true, arrayDocument);
    }
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
    {
        Q_UNUSED(db);
        //
        if (m_strKeywords.IsEmpty())
            return false;
        //
        return -1 != ::WizStrStrI_Pos(data.strTitle, m_strKeywords);
    }
    //
    void setKeywords(const CString& keywords)
    {
        m_strKeywords = keywords;
        //
        CString strText = QObject::tr("Search for %1").arg(keywords);

        setText(0, strText);
    }
};


CWizCategoryView::CWizCategoryView(CWizExplorerApp& app, QWidget *parent)
    : QTreeWidget(parent)
    , m_db(app.database())
    , m_menuAllFolders(NULL)
    , m_menuAllTags(NULL)
    , m_menuFolder(NULL)
    , m_menuTag(NULL)
    , m_menuTrash(NULL)
{
    setFrameStyle(QFrame::NoFrame);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setAutoFillBackground(true);
    //
    QPalette pal = palette();
    pal.setColor(QPalette::Base, WizGetCategoryBackroundColor());
    setPalette(pal);
    //
    setStyle(::WizGetStyle());
    //
    header()->hide();
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTextElideMode(Qt::ElideMiddle);
    //
    connect(&m_db, SIGNAL(tagCreated(const WIZTAGDATA&)), this, SLOT(on_tag_created(const WIZTAGDATA&)));
    connect(&m_db, SIGNAL(tagModified(const WIZTAGDATA&, const WIZTAGDATA&)), this, SLOT(on_tag_modified(const WIZTAGDATA&, const WIZTAGDATA&)));
    connect(&m_db, SIGNAL(tagDeleted(WIZTAGDATA)), this, SLOT(on_tag_deleted(const WIZTAGDATA&)));
    connect(&m_db, SIGNAL(documentCreated(const WIZDOCUMENTDATA&)), this, SLOT(on_document_created(const WIZDOCUMENTDATA&)));
    connect(&m_db, SIGNAL(documentModified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)), this, SLOT(on_document_modified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)));
    connect(&m_db, SIGNAL(folderCreated(const CString&)), this, SLOT(on_folder_created(const CString&)));
    connect(&m_db, SIGNAL(folderDeleted(const CString&)), this, SLOT(on_folder_deleted(const CString&)));
    //
    setIndentation(12);
    //
    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);
}



void CWizCategoryView::init()
{
    initFolders();
    addSeparator();
    initTags();
    addSeparator();
    initTrash();
    //
    setCurrentItem(findAllFolders());
}

void CWizCategoryView::initFolders()
{
    CWizCategoryViewAllFoldersItem* pAllFoldersItem = new CWizCategoryViewAllFoldersItem(tr("Note Folders"));
    addTopLevelItem(pAllFoldersItem);
    //
    CWizStdStringArray arrayAllLocation;
    m_db.GetAllLocations(arrayAllLocation);
    //
    initFolders(pAllFoldersItem, "", arrayAllLocation);
    //
    if (arrayAllLocation.empty())
    {
        const CString strNotes("/My Notes/");
        const CString strDrafts("/My Drafts/");
        m_db.AddExtraFolder(strNotes);
        m_db.AddExtraFolder(strDrafts);
        arrayAllLocation.push_back(strNotes);
        arrayAllLocation.push_back(strDrafts);
    }
    //
    //init extra folders
    CWizStdStringArray arrayExtLocation;
    m_db.GetExtraFolder(arrayExtLocation);
    for (CWizStdStringArray::const_iterator it = arrayExtLocation.begin();
    it != arrayExtLocation.end();
    it++)
    {
        CString strLocation = *it;
        if (strLocation.IsEmpty())
            continue;
        //
        if (m_db.IsInDeletedItems(strLocation))
            continue;
        //
        addFolder(strLocation, true);
    }
    //
    pAllFoldersItem->setExpanded(true);
    //
    pAllFoldersItem->sortChildren(0, Qt::AscendingOrder);
}

void CWizCategoryView::initFolders(QTreeWidgetItem* pParent, const CString& strParentLocation, const CWizStdStringArray& arrayAllLocation)
{
    CWizStdStringArray arrayLocation;
    CWizDatabase::GetChildLocations(arrayAllLocation, strParentLocation, arrayLocation);
    //
    for (CWizStdStringArray::const_iterator it = arrayLocation.begin();
    it != arrayLocation.end();
    it++)
    {
        CString strLocation = *it;
        //
        if (m_db.IsInDeletedItems(strLocation))
            continue;
        //
        CWizCategoryViewFolderItem* pFolderItem = new CWizCategoryViewFolderItem(strLocation);
        pParent->addChild(pFolderItem);
        //
        initFolders(pFolderItem, strLocation, arrayAllLocation);
    }
}

void CWizCategoryView::initTags()
{
    CWizCategoryViewAllTagsItem* pAllTagsItem = new CWizCategoryViewAllTagsItem(tr("Tags"));
    addTopLevelItem(pAllTagsItem);
    //
    initTags(pAllTagsItem, "");
}

void CWizCategoryView::initTags(QTreeWidgetItem* pParent, const CString& strParentTagGUID)
{
    CWizTagDataArray arrayTag;
    m_db.GetChildTags(strParentTagGUID, arrayTag);
    //
    for (CWizTagDataArray::const_iterator it = arrayTag.begin();
    it != arrayTag.end();
    it++)
    {
        CWizCategoryViewTagItem* pTagItem = new CWizCategoryViewTagItem(*it);
        pParent->addChild(pTagItem);
        //
        initTags(pTagItem, it->strGUID);
    }
}

void CWizCategoryView::initTrash()
{
    addTopLevelItem(new CWizCategoryViewTrashItem(tr("Trash")));
}

void CWizCategoryView::addSeparator()
{
    addTopLevelItem(new CWizCategoryViewSeparatorItem());
}


void CWizCategoryView::getDocuments(CWizDocumentDataArray& arrayDocument)
{
    QList<QTreeWidgetItem*> items = selectedItems();
    if (items.empty())
        return;
    //
    CWizCategoryViewItem* pItem = dynamic_cast<CWizCategoryViewItem*>(items.first());
    if (!pItem)
        return;
    //
    pItem->getDocuments(m_db, arrayDocument);
    //
}

void CWizCategoryView::showAllFoldersContextMenu(QPoint pos)
{
    if (!m_menuAllFolders)
    {
        m_menuAllFolders = new QMenu(this);
        m_menuAllFolders->addAction(tr("New Folder"), this, SLOT(on_action_newFolder()));
    }
    //
    m_menuAllFolders->popup(pos);
}

void CWizCategoryView::showFolderContextMenu(QPoint pos)
{
    if (!m_menuFolder)
    {
        m_menuFolder = new QMenu(this);
        m_menuFolder->addAction(tr("New Folder"), this, SLOT(on_action_newFolder()));
        m_menuFolder->addAction(tr("Delete Folder"), this, SLOT(on_action_deleteFolder()));
    }
    //
    m_menuFolder->popup(pos);
}

void CWizCategoryView::showAllTagsContextMenu(QPoint pos)
{
    if (!m_menuAllTags)
    {
        m_menuAllTags = new QMenu(this);
        m_menuAllTags->addAction(tr("New Tag"), this, SLOT(on_action_newTag()));
    }
    //
    m_menuAllTags->popup(pos);
}

void CWizCategoryView::showTagContextMenu(QPoint pos)
{
    if (!m_menuTag)
    {
        m_menuTag = new QMenu(this);
        m_menuTag->addAction(tr("New Tag"), this, SLOT(on_action_newTag()));
        m_menuTag->addAction(tr("Delete Tag"), this, SLOT(on_action_deleteTag()));
    }
    //
    m_menuTag->popup(pos);
}

void CWizCategoryView::showTrashContextMenu(QPoint pos)
{
    if (!m_menuTrash)
    {
        m_menuTrash = new QMenu(this);
        m_menuTrash->addAction(tr("Empty the Trash"), this, SLOT(on_action_emptyTrash()));
    }
    //
    m_menuTrash->popup(pos);
}

void CWizCategoryView::contextMenuEvent(QContextMenuEvent * e)
{
    CWizCategoryViewItem* pItem = dynamic_cast<CWizCategoryViewItem*>(itemAt(e->pos()));
    if (!pItem)
        return;
    //
    pItem->showContextMenu(this, mapToGlobal(e->pos()));
}

void CWizCategoryView::mousePressEvent(QMouseEvent* event )
{
    if (isSeparatorItemByPosition(event->pos()))
        return;
    //
    QTreeWidget::mousePressEvent(event);
}

QModelIndex CWizCategoryView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    QModelIndex index = QTreeWidget::moveCursor(cursorAction, modifiers);
    if (!index.isValid())
        return index;
    //
    CWizCategoryViewItem* pItem = categoryItemFromIndex(index);
    if (CWizCategoryViewSeparatorItem* pSeparatorItem = dynamic_cast<CWizCategoryViewSeparatorItem*>(pItem))
    {
        switch (cursorAction)
        {
        case MoveUp:
        case MoveLeft:
            {
                QTreeWidgetItem* pAbove = itemAbove(pSeparatorItem);
                ATLASSERT(pAbove);
                return indexFromItem(pAbove);
            }
        case MoveDown:
        case MoveRight:
            {
                QTreeWidgetItem* pBelow = itemBelow(pSeparatorItem);
                ATLASSERT(pBelow);
                return indexFromItem(pBelow);
            }
        default:
            ATLASSERT(FALSE);
            break;
        }
    }
    //
    return index;
}

void CWizCategoryView::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions);
    //
    QPoint pt = mapFromGlobal(QCursor::pos());
    //
    if (CWizCategoryViewTagItem* item = dynamic_cast<CWizCategoryViewTagItem*>(itemAt(pt)))
    {
        QDrag* drag = new QDrag(this);
        QMimeData* mimeData = new QMimeData();
        mimeData->setData(WIZNOTE_MIMEFORMAT_TAGS, item->tag().strGUID.toUtf8());
        drag->setMimeData(mimeData);
        drag->exec(Qt::CopyAction);
    }
}

void CWizCategoryView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS))
    {
        event->accept();
        return;
    }
    //
    QTreeWidget::dragEnterEvent(event);
}

void CWizCategoryView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS))
    {
        event->accept();
        return;
    }
    //
    QTreeWidget::dragMoveEvent(event);
}

void CWizCategoryView::dropEvent(QDropEvent * event)
{
    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS))
    {
        QByteArray data = event->mimeData()->data(WIZNOTE_MIMEFORMAT_DOCUMENTS);
        QString strDocumentGUIDs = QString::fromUtf8(data, data.length());
        CWizStdStringArray arrayDocumentGUID;
        ::WizSplitTextToArray(strDocumentGUIDs, ';', arrayDocumentGUID);
        //
        if (CWizCategoryViewTagItem* item = dynamic_cast<CWizCategoryViewTagItem*>(itemAt(event->pos())))
        {
            foreach (const CString& strDocumentGUID, arrayDocumentGUID)
            {
                WIZDOCUMENTDATA dataDocument;
                if (m_db.DocumentFromGUID(strDocumentGUID, dataDocument))
                {
                    CWizDocument doc(m_db, dataDocument);
                    doc.AddTag(item->tag());
                }
            }
        }
        else if (CWizCategoryViewFolderItem* item = dynamic_cast<CWizCategoryViewFolderItem*>(itemAt(event->pos())))
        {
            CWizFolder folder(m_db, item->location());
            //
            foreach (const CString& strDocumentGUID, arrayDocumentGUID)
            {
                WIZDOCUMENTDATA dataDocument;
                if (m_db.DocumentFromGUID(strDocumentGUID, dataDocument))
                {
                    CWizDocument doc(m_db, dataDocument);
                    doc.MoveDocument(&folder);
                }
            }
        }

        event->accept();
        return;
    }
}

CWizCategoryViewAllTagsItem* CWizCategoryView::findAllTags()
{
    int nCount = topLevelItemCount();
    for (int i = 0; i < nCount; i++)
    {
        if (CWizCategoryViewAllTagsItem* pItem = dynamic_cast<CWizCategoryViewAllTagsItem*>(topLevelItem(i)))
        {
            return pItem;
        }
    }
    //
    ATLASSERT(FALSE);
    //
    return NULL;
}

CWizCategoryViewTagItem* CWizCategoryView::findTag(const WIZTAGDATA& tag, bool create, bool sort)
{
    CWizStdStringArray arrayGUID;
    if (!m_db.GetAllParentsTagGUID(tag.strGUID, arrayGUID))
        return NULL;
    //
    arrayGUID.insert(arrayGUID.begin(), tag.strGUID);   //insert self
    //
    CWizCategoryViewAllTagsItem* pAllTags = findAllTags();
    if (!pAllTags)
        return NULL;
    //
    QTreeWidgetItem* parent = pAllTags;
    //
    size_t nCount = arrayGUID.size();
    for (intptr_t i = nCount - 1; i >= 0; i--)
    {
        CString strParentTagGUID = arrayGUID[i];
        //
        WIZTAGDATA tagParent;
        if (!m_db.TagFromGUID(strParentTagGUID, tagParent))
            return NULL;
        //
        bool found = false;
        int nCount = parent->childCount();
        for (int i = 0; i < nCount; i++)
        {
            CWizCategoryViewTagItem* pTag = dynamic_cast<CWizCategoryViewTagItem*>(parent->child(i));
            if (pTag
                && pTag->tag().strGUID == tagParent.strGUID)
            {
                found = true;
                parent = pTag;
                continue;
            }
        }
        //
        if (found)
            continue;
        //
        if (!create)
            return NULL;
        //
        CWizCategoryViewTagItem* pTagItem = new CWizCategoryViewTagItem(tagParent);
        parent->addChild(pTagItem);
        parent->setExpanded(true);
        parent = pTagItem;
        //
        if (sort)
        {
            parent->sortChildren(0, Qt::AscendingOrder);
        }
    }
    //
    //
    return dynamic_cast<CWizCategoryViewTagItem *>(parent);
}

CWizCategoryViewTagItem* CWizCategoryView::findTagInTree(const WIZTAGDATA& tag)
{
    CWizCategoryViewAllTagsItem* pAllTags = findAllTags();
    if (!pAllTags)
        return NULL;
    //
    return findTagInTree(tag, pAllTags);
}
CWizCategoryViewTagItem* CWizCategoryView::findTagInTree(const WIZTAGDATA& tag, QTreeWidgetItem* itemParent)
{
    for (int i = 0; i < itemParent->childCount(); i++)
    {
        QTreeWidgetItem* it = itemParent->child(i);
        //
        if (CWizCategoryViewTagItem* item = dynamic_cast<CWizCategoryViewTagItem*>(it))
        {
            if (item->tag().strGUID == tag.strGUID)
                return item;
        }
        //
        if (CWizCategoryViewTagItem* childItem = findTagInTree(tag, it))
        {
            return childItem;
        }
    }
    //
    return NULL;
}


CWizCategoryViewTagItem* CWizCategoryView::addTag(const WIZTAGDATA& tag, bool sort)
{
    return findTag(tag, true, sort);
}
CWizCategoryViewTagItem* CWizCategoryView::addTagWithChildren(const WIZTAGDATA& tag)
{
    CWizCategoryViewTagItem* pItem = findTag(tag, true, true);
    if (!pItem)
        return NULL;
    //
    CWizTagDataArray arrayTag;
    m_db.GetChildTags(tag.strGUID, arrayTag);
    for (CWizTagDataArray::const_iterator it = arrayTag.begin();
    it != arrayTag.end();
    it++)
    {
        addTagWithChildren(*it);
    }
    //
    return pItem;
}

void CWizCategoryView::removeTag(const WIZTAGDATA& tag)
{
    CWizCategoryViewTagItem* pItem = findTagInTree(tag);
    if (pItem)
    {
        QTreeWidgetItem* parent = pItem->parent();
        if (parent)
        {
            parent->removeChild(pItem);
        }
    }
}

CWizCategoryViewSearchItem* CWizCategoryView::findSearch()
{
    int nCount = topLevelItemCount();
    for (int i = 0; i < nCount; i++)
    {
        if (CWizCategoryViewSearchItem* pItem = dynamic_cast<CWizCategoryViewSearchItem*>(topLevelItem(i)))
        {
            return pItem;
        }
    }
    //
    addSeparator();
    //
    CWizCategoryViewSearchItem* pItem = new CWizCategoryViewSearchItem("");
    addTopLevelItem(pItem);
    //
    return pItem;
}
CWizCategoryViewTrashItem* CWizCategoryView::findTrash()
{
    int nCount = topLevelItemCount();
    for (int i = 0; i < nCount; i++)
    {
        if (CWizCategoryViewTrashItem* pItem = dynamic_cast<CWizCategoryViewTrashItem*>(topLevelItem(i)))
        {
            return pItem;
        }
    }
    //
    return NULL;
}


void CWizCategoryView::search(const CString& str)
{
    if (str.isEmpty())
        return;
    //
    CWizCategoryViewSearchItem* pItem = findSearch();
    pItem->setKeywords(str);
    //
    if (currentItem() == pItem)
    {
        emit itemSelectionChanged();
    }
    else
    {
        setCurrentItem(pItem, 0);
    }
}


bool CWizCategoryView::acceptDocument(const WIZDOCUMENTDATA& document)
{
    QList<QTreeWidgetItem*> items = selectedItems();
    if (items.empty())
        return false;
    //
    CWizCategoryViewItem* pItem = dynamic_cast<CWizCategoryViewItem*>(items.first());
    if (!pItem)
        return false;
    //
    return pItem->accept(m_db, document);
}

void CWizCategoryView::addAndSelectFolder(const CString& strLocation)
{
    if (QTreeWidgetItem* pItem = addFolder(strLocation, true))
    {
        setCurrentItem(pItem);
    }
}


CWizCategoryViewItem* CWizCategoryView::categoryItemFromIndex(const QModelIndex &index) const
{
    return dynamic_cast<CWizCategoryViewItem*>(itemFromIndex(index));
}
bool CWizCategoryView::isSeparatorItemByIndex(const QModelIndex &index) const
{
    const CWizCategoryViewItem* pItem = categoryItemFromIndex(index);
    return NULL != dynamic_cast<const CWizCategoryViewSeparatorItem*>(pItem);
}
bool CWizCategoryView::isSeparatorItemByPosition(const QPoint& pt) const
{
    const QTreeWidgetItem* pItem = itemAt(pt);
    return NULL != dynamic_cast<const CWizCategoryViewSeparatorItem*>(pItem);
}



CWizCategoryViewAllFoldersItem* CWizCategoryView::findAllFolders()
{
    int nCount = topLevelItemCount();
    for (int i = 0; i < nCount; i++)
    {
        if (CWizCategoryViewAllFoldersItem* pItem = dynamic_cast<CWizCategoryViewAllFoldersItem*>(topLevelItem(i)))
        {
            return pItem;
        }
    }
    //
    ATLASSERT(FALSE);
    //
    return NULL;
}

CWizCategoryViewFolderItem* CWizCategoryView::findFolder(const CString& strLocation, bool create, bool sort)
{
    CWizCategoryViewAllFoldersItem* pAllFolders = findAllFolders();
    if (!pAllFolders)
        return NULL;
    //
    if (m_db.IsInDeletedItems(strLocation))
    {
        return findTrash();
    }
    //
    CString strCurrentLocation = "/";
    QTreeWidgetItem* parent = pAllFolders;
    //
    CString strTempLocation = strLocation;
    strTempLocation.Trim('/');
    QStringList sl = strTempLocation.split("/");
    for (QStringList::const_iterator it = sl.begin();
    it != sl.end();
    it++)
    {
        CString strLocationName = *it;
        ATLASSERT(!strLocationName.IsEmpty());
        strCurrentLocation = strCurrentLocation + strLocationName + "/";
        //
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
        //
        if (found)
            continue;
        //
        if (!create)
            return NULL;
        //
        CWizCategoryViewFolderItem* pFolderItem = new CWizCategoryViewFolderItem(strCurrentLocation);
        parent->addChild(pFolderItem);
        parent->setExpanded(true);
        if (sort)
        {
            parent->sortChildren(0, Qt::AscendingOrder);
        }
        //
        parent = pFolderItem;
    }
    //
    return dynamic_cast<CWizCategoryViewFolderItem *>(parent);
}

CWizCategoryViewFolderItem* CWizCategoryView::addFolder(const CString& strLocation, bool sort)
{
    return findFolder(strLocation, true, sort);
}

void CWizCategoryView::on_tag_created(const WIZTAGDATA& tag)
{
    addTagWithChildren(tag);
}
void CWizCategoryView::on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew)
{
    if (tagOld.strParentGUID != tagNew.strParentGUID)
    {
        removeTag(tagOld);
        addTagWithChildren(tagNew);
    }
    else
    {
        CWizCategoryViewTagItem* pTagItem = addTagWithChildren(tagNew);
        if (pTagItem)
        {
            pTagItem->reload(m_db);
        }
    }
}
void CWizCategoryView::on_tag_deleted(const WIZTAGDATA& tag)
{
    removeTag(tag);
}


void CWizCategoryView::on_document_created(const WIZDOCUMENTDATA& document)
{
    if (m_db.IsInDeletedItems(document.strLocation))
        return;
    //
    addFolder(document.strLocation, true);
}

void CWizCategoryView::on_document_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew)
{
    Q_UNUSED(documentOld);
    //
    if (m_db.IsInDeletedItems(documentNew.strLocation))
        return;
    //
    addFolder(documentNew.strLocation, true);
}
void CWizCategoryView::on_folder_created(const CString& strLocation)
{
    addFolder(strLocation, true);
}

void CWizCategoryView::on_folder_deleted(const CString& strLocation)
{
    if (CWizCategoryViewFolderItem* pFolder = findFolder(strLocation, false, false))
    {
        if (QTreeWidgetItem* parent = pFolder->parent())
        {
            parent->removeChild(pFolder);
        }
    }
}

void CWizCategoryView::on_action_newFolder()
{
    NewFolderDialog dlg;
    if (QDialog::Accepted != dlg.exec())
        return;
    //
    CString strFolderName = dlg.folderName();
    if (strFolderName.IsEmpty())
        return;
    //
    WizMakeValidFileNameNoPath(strFolderName);
    //
    CString strLocation;
    //
    if (CWizCategoryViewAllFoldersItem* p = currentCategoryItem<CWizCategoryViewAllFoldersItem>())
    {
        Q_UNUSED(p);
        strLocation = "/" + strFolderName + "/";
    }
    else if (CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>())
    {
        strLocation = p->location() + strFolderName + "/";
    }
    addAndSelectFolder(strLocation);
    m_db.AddExtraFolder(strLocation);
}

void CWizCategoryView::on_action_deleteFolder()
{
    CWaitCursor wait;
    Q_UNUSED(wait);
    //
    if (CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>())
    {
        CWizFolder folder(m_db, p->location());
        folder.Delete();
    }
}

void CWizCategoryView::on_action_newTag()
{
    NewTagDialog dlg;
    if (QDialog::Accepted != dlg.exec())
        return;
    //
    CString strTagNames = dlg.tagName();
    if (strTagNames.IsEmpty())
        return;
    //
    WIZTAGDATA parentTag;
    //
    if (CWizCategoryViewTagItem* p = currentCategoryItem<CWizCategoryViewTagItem>())
    {
        parentTag = p->tag();
    }

    QStringList sl = strTagNames.split(';');
    for (QStringList::const_iterator it = sl.begin();
    it != sl.end();
    it++)
    {
        CString strTagName = *it;
        //
        WIZTAGDATA tagNew;
        m_db.CreateTag(parentTag.strGUID, strTagName, "", tagNew);
    }
}

void CWizCategoryView::on_action_deleteTag()
{
    CWaitCursor wait;
    Q_UNUSED(wait);
    //
    if (CWizCategoryViewTagItem* p = currentCategoryItem<CWizCategoryViewTagItem>())
    {
        WIZTAGDATA tag = p->tag();
        m_db.DeleteTagWithChildren(tag, TRUE);
    }
}

void CWizCategoryView::on_action_emptyTrash()
{
    CWaitCursor wait;
    Q_UNUSED(wait);
    //
    if (CWizCategoryViewTrashItem* trashItem = CWizCategoryView::findTrash())
    {
        CWizDocumentDataArray arrayDocument;
        trashItem->getDocuments(m_db, arrayDocument);
        //
        foreach (const WIZDOCUMENTDATA& data, arrayDocument)
        {
            CWizDocument doc(m_db, data);
            doc.Delete();
        }
    }
}


//////////////////////////////////////////////////////
CWizFolder* CWizCategoryView::SelectedFolder()
{
    QList<QTreeWidgetItem*> items = selectedItems();
    if (items.empty())
        return NULL;
    //
    CWizCategoryViewFolderItem* pItem = dynamic_cast<CWizCategoryViewFolderItem*>(items.first());
    if (!pItem)
        return NULL;
    //
    return new CWizFolder(m_db, pItem->location());
}

void CWizCategoryView::setSelectedFolder(QObject* pFolder)
{
    Q_UNUSED(pFolder);
}
