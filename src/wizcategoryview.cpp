#include "wizcategoryview.h"

#ifdef BUILD_WITH_QT5
#include <QtWidgets>
#endif

#include <QHeaderView>
#include <QPalette>
#include <QContextMenuEvent>
#include <QMenu>
#include <QCursor>
#include <QPainter>

#include "share/wizdrawtexthelper.h"
#include "wiznotestyle.h"

#include "newfolderdialog.h"
#include "newtagdialog.h"


CWizCategoryBaseView::CWizCategoryBaseView(CWizExplorerApp& app, QWidget* parent)
    : QTreeWidget(parent)
    , m_app(app)
    , m_db(app.database())
    , m_bDragHovered(false)
{
    header()->hide();
    setFrameStyle(QFrame::NoFrame);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setAutoFillBackground(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTextElideMode(Qt::ElideMiddle);
    setIndentation(12);

    QPalette pal = palette();
    pal.setColor(QPalette::Base, WizGetCategoryBackroundColor(m_app.userSettings().skin()));
    setPalette(pal);

    setStyle(::WizGetStyle(m_app.userSettings().skin()));
}

void CWizCategoryBaseView::addSeparator()
{
    addTopLevelItem(new CWizCategoryViewSeparatorItem(m_app));
}

void CWizCategoryBaseView::getDocuments(CWizDocumentDataArray& arrayDocument)
{
    QList<QTreeWidgetItem*> items = selectedItems();
    if (items.empty())
        return;

    CWizCategoryViewItemBase* pItem = dynamic_cast<CWizCategoryViewItemBase*>(items.first());
    if (!pItem)
        return;

    pItem->getDocuments(m_db, arrayDocument);
}

bool CWizCategoryBaseView::acceptDocument(const WIZDOCUMENTDATA& document)
{
    QList<QTreeWidgetItem*> items = selectedItems();
    if (items.empty())
        return false;

    CWizCategoryViewItemBase* pItem = dynamic_cast<CWizCategoryViewItemBase*>(items.first());
    if (!pItem)
        return false;

    return pItem->accept(m_db, document);
}

template <class T> inline  T* CWizCategoryBaseView::currentCategoryItem() const
{
    return dynamic_cast<T*>(currentItem());
}

CWizCategoryViewItemBase* CWizCategoryBaseView::categoryItemFromIndex(const QModelIndex &index) const
{
    return dynamic_cast<CWizCategoryViewItemBase*>(itemFromIndex(index));
}

bool CWizCategoryBaseView::isSeparatorItemByIndex(const QModelIndex &index) const
{
    CWizCategoryViewItemBase* pItem = categoryItemFromIndex(index);
    return NULL != dynamic_cast<const CWizCategoryViewSeparatorItem*>(pItem);
}

bool CWizCategoryBaseView::isSeparatorItemByPosition(const QPoint& pt) const
{
    const QTreeWidgetItem* pItem = itemAt(pt);
    return NULL != dynamic_cast<const CWizCategoryViewSeparatorItem*>(pItem);
}

void CWizCategoryBaseView::contextMenuEvent(QContextMenuEvent * e)
{
    CWizCategoryViewItemBase* pItem = dynamic_cast<CWizCategoryViewItemBase*>(itemAt(e->pos()));
    if (!pItem)
        return;

    pItem->showContextMenu(this, mapToGlobal(e->pos()));
}

void CWizCategoryBaseView::mousePressEvent(QMouseEvent* event)
{
    if (isSeparatorItemByPosition(event->pos()))
        return;

    QTreeWidget::mousePressEvent(event);
}

QModelIndex CWizCategoryBaseView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    QModelIndex index = QTreeWidget::moveCursor(cursorAction, modifiers);
    if (!index.isValid())
        return index;

    CWizCategoryViewItemBase* pItem = categoryItemFromIndex(index);
    if (CWizCategoryViewSeparatorItem* pSeparatorItem = dynamic_cast<CWizCategoryViewSeparatorItem*>(pItem))
    {
        switch (cursorAction)
        {
        case MoveUp:
        case MoveLeft:
            {
                QTreeWidgetItem* pAbove = itemAbove(pSeparatorItem);
                Q_ASSERT(pAbove);
                return indexFromItem(pAbove);
            }
        case MoveDown:
        case MoveRight:
            {
                QTreeWidgetItem* pBelow = itemBelow(pSeparatorItem);
                Q_ASSERT(pBelow);
                return indexFromItem(pBelow);
            }
        default:
            Q_ASSERT(false);
            break;
        }
    }

    return index;
}

bool CWizCategoryBaseView::validateDropDestination(const QPoint& p) const
{
    if (p.isNull())
        return false;

    CWizCategoryViewFolderItem* itemFolder = dynamic_cast<CWizCategoryViewFolderItem*>(itemAt(p));
    if (itemFolder) {
        return true;
    }

    CWizCategoryViewTagItem* itemTag = dynamic_cast<CWizCategoryViewTagItem*>(itemAt(p));
    if (itemTag) {
        return true;
    }

    return false;
}

void CWizCategoryBaseView::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions);

    QPoint pt = mapFromGlobal(QCursor::pos());

    if (CWizCategoryViewTagItem* item = dynamic_cast<CWizCategoryViewTagItem*>(itemAt(pt)))
    {
        QDrag* drag = new QDrag(this);
        QMimeData* mimeData = new QMimeData();
        mimeData->setData(WIZNOTE_MIMEFORMAT_TAGS, item->tag().strGUID.toUtf8());
        drag->setMimeData(mimeData);
        drag->exec(Qt::CopyAction);
    }
}

void CWizCategoryBaseView::dragEnterEvent(QDragEnterEvent *event)
{
    m_bDragHovered = true;
    repaint();

    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS)) {
        event->acceptProposedAction();
    }
}

void CWizCategoryBaseView::dragMoveEvent(QDragMoveEvent *event)
{
    m_dragHoveredPos = event->pos();
    repaint();

    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS)) {
        event->acceptProposedAction();
    }
}

void CWizCategoryBaseView::dragLeaveEvent(QDragLeaveEvent* event)
{
    Q_UNUSED(event);

    m_bDragHovered = false;
    m_dragHoveredPos = QPoint();
    repaint();
}

void CWizCategoryBaseView::dropEvent(QDropEvent * event)
{
    m_bDragHovered = false;
    m_dragHoveredPos = QPoint();
    repaint();

    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS))
    {
        QByteArray data = event->mimeData()->data(WIZNOTE_MIMEFORMAT_DOCUMENTS);
        QString strDocumentGUIDs = QString::fromUtf8(data, data.length());
        CWizStdStringArray arrayDocumentGUID;
        ::WizSplitTextToArray(strDocumentGUIDs, ';', arrayDocumentGUID);

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

        event->acceptProposedAction();
    }
}



CWizCategoryView::CWizCategoryView(CWizExplorerApp& app, QWidget* parent)
    : CWizCategoryBaseView(app, parent)
    , m_menuAllFolders(NULL)
    , m_menuFolder(NULL)
    , m_menuTrash(NULL)
{
    qRegisterMetaType<WIZDOCUMENTDATA>("WIZDOCUMENTDATA");

    connect(&m_db, SIGNAL(documentCreated(const WIZDOCUMENTDATA&)), \
            SLOT(on_document_created(const WIZDOCUMENTDATA&)));

    connect(&m_db, SIGNAL(documentModified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)), \
            SLOT(on_document_modified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)));

    connect(&m_db, SIGNAL(folderCreated(const CString&)), \
            SLOT(on_folder_created(const CString&)));

    connect(&m_db, SIGNAL(folderDeleted(const CString&)), \
            SLOT(on_folder_deleted(const CString&)));

    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);
}

void CWizCategoryView::init()
{
    initFolders();
    addSeparator();
    initTrash();

    setCurrentItem(findAllFolders());
}

void CWizCategoryView::initFolders()
{
    CWizCategoryViewAllFoldersItem* pAllFoldersItem = new CWizCategoryViewAllFoldersItem(m_app, tr("Note Folders"));
    addTopLevelItem(pAllFoldersItem);

    CWizStdStringArray arrayAllLocation;
    m_db.GetAllLocations(arrayAllLocation);

    initFolders(pAllFoldersItem, QString(), arrayAllLocation);

    if (arrayAllLocation.empty()) {
        const QString strNotes("/My Notes/");
        m_db.AddExtraFolder(strNotes);
        arrayAllLocation.push_back(strNotes);
    }

    //init extra folders
    CWizStdStringArray arrayExtLocation;
    m_db.GetExtraFolder(arrayExtLocation);

    CWizStdStringArray::const_iterator it;
    for (it = arrayExtLocation.begin(); it != arrayExtLocation.end(); it++) {
        QString strLocation = *it;

        if (strLocation.isEmpty())
            continue;

        if (m_db.IsInDeletedItems(strLocation))
            continue;

        addFolder(strLocation, true);
    }

    pAllFoldersItem->setExpanded(true);
    pAllFoldersItem->sortChildren(0, Qt::AscendingOrder);
}

void CWizCategoryView::initFolders(QTreeWidgetItem* pParent, \
                                   const QString& strParentLocation, \
                                   const CWizStdStringArray& arrayAllLocation)
{
    CWizStdStringArray arrayLocation;
    CWizDatabase::GetChildLocations(arrayAllLocation, strParentLocation, arrayLocation);

    CWizStdStringArray::const_iterator it;
    for (it = arrayLocation.begin(); it != arrayLocation.end(); it++) {
        CString strLocation = *it;

        if (m_db.IsInDeletedItems(strLocation))
            continue;

        CWizCategoryViewFolderItem* pFolderItem = new CWizCategoryViewFolderItem(m_app, strLocation);
        pParent->addChild(pFolderItem);

        initFolders(pFolderItem, strLocation, arrayAllLocation);
    }
}

void CWizCategoryView::initTrash()
{
    addTopLevelItem(new CWizCategoryViewTrashItem(m_app, tr("Trash")));
}

void CWizCategoryView::showAllFoldersContextMenu(QPoint pos)
{
    if (!m_menuAllFolders) {
        m_menuAllFolders = new QMenu(this);
        m_menuAllFolders->addAction(tr("New Folder"), this, SLOT(on_action_newFolder()));
    }

    m_menuAllFolders->popup(pos);
}

void CWizCategoryView::showFolderContextMenu(QPoint pos)
{
    if (!m_menuFolder) {
        m_menuFolder = new QMenu(this);
        m_menuFolder->addAction(tr("New Folder"), this, SLOT(on_action_newFolder()));
        m_menuFolder->addAction(tr("Delete Folder"), this, SLOT(on_action_deleteFolder()));
    }

    m_menuFolder->popup(pos);
}

void CWizCategoryView::showTrashContextMenu(QPoint pos)
{
    if (!m_menuTrash) {
        m_menuTrash = new QMenu(this);
        m_menuTrash->addAction(tr("Empty the Trash"), this, SLOT(on_action_emptyTrash()));
    }

    m_menuTrash->popup(pos);
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
    CWizCategoryViewSearchItem* pItem = new CWizCategoryViewSearchItem(m_app, "");
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

void CWizCategoryView::search(const QString& str)
{
    Q_UNUSED(str);

    m_selectedItem = currentItem();
    clearSelection();

    //CWizCategoryViewSearchItem* pItem = findSearch();
    //pItem->setKeywords(str);
//
    //if (currentItem() == pItem) {
    //    emit itemSelectionChanged();
    //} else {
    //    setCurrentItem(pItem, 0);
    //}
}

void CWizCategoryView::restoreSelection()
{
    setCurrentItem(m_selectedItem);
    emit itemSelectionChanged();
}

void CWizCategoryView::addAndSelectFolder(const CString& strLocation)
{
    if (QTreeWidgetItem* pItem = addFolder(strLocation, true))
    {
        setCurrentItem(pItem);
    }
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

    Q_ASSERT(false);
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
        CWizCategoryViewFolderItem* pFolderItem = new CWizCategoryViewFolderItem(m_app, strCurrentLocation);
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

void CWizCategoryView::on_document_created(const WIZDOCUMENTDATA& document)
{
    if (m_db.IsInDeletedItems(document.strLocation))
        return;

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

    CString strFolderName = dlg.folderName();
    if (strFolderName.IsEmpty())
        return;

    WizMakeValidFileNameNoPath(strFolderName);

    CString strLocation;

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

CWizFolder* CWizCategoryView::SelectedFolder()
{
    QList<QTreeWidgetItem*> items = selectedItems();
    if (items.empty())
        return NULL;

    CWizCategoryViewFolderItem* pItem = dynamic_cast<CWizCategoryViewFolderItem*>(items.first());
    if (!pItem)
        return NULL;

    return new CWizFolder(m_db, pItem->location());
}



CWizCategoryTagsView::CWizCategoryTagsView(CWizExplorerApp& app, QWidget *parent)
    : CWizCategoryBaseView(app, parent)
    , m_menuAllTags(NULL)
    , m_menuTag(NULL)
{
    qRegisterMetaType<WIZTAGDATA>("WIZTAGDATA");

    connect(&m_db, SIGNAL(tagCreated(const WIZTAGDATA&)), \
            SLOT(on_tag_created(const WIZTAGDATA&)));

    connect(&m_db, SIGNAL(tagModified(const WIZTAGDATA&, const WIZTAGDATA&)), \
            SLOT(on_tag_modified(const WIZTAGDATA&, const WIZTAGDATA&)));

    connect(&m_db, SIGNAL(tagDeleted(const WIZTAGDATA&)), \
            SLOT(on_tag_deleted(const WIZTAGDATA&)));
}

void CWizCategoryTagsView::showAllTagsContextMenu(QPoint pos)
{
    if (!m_menuAllTags) {
        m_menuAllTags = new QMenu(this);
        m_menuAllTags->addAction(tr("New Tag"), this, SLOT(on_action_newTag()));
    }

    m_menuAllTags->popup(pos);
}

void CWizCategoryTagsView::showTagContextMenu(QPoint pos)
{
    if (!m_menuTag) {
        m_menuTag = new QMenu(this);
        m_menuTag->addAction(tr("New Tag"), this, SLOT(on_action_newTag()));
        m_menuTag->addAction(tr("Delete Tag"), this, SLOT(on_action_deleteTag()));
    }

    m_menuTag->popup(pos);
}

void CWizCategoryTagsView::init()
{
    initTags();
}

void CWizCategoryTagsView::initTags()
{
    CWizCategoryViewAllTagsItem* pAllTagsItem = new CWizCategoryViewAllTagsItem(m_app, tr("Tags"));
    addTopLevelItem(pAllTagsItem);
    initTags(pAllTagsItem, "");
}

void CWizCategoryTagsView::initTags(QTreeWidgetItem* pParent, const QString& strParentTagGUID)
{
    CWizTagDataArray arrayTag;
    m_db.GetChildTags(strParentTagGUID, arrayTag);

    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        CWizCategoryViewTagItem* pTagItem = new CWizCategoryViewTagItem(m_app, *it);
        pParent->addChild(pTagItem);

        initTags(pTagItem, it->strGUID);
    }
}

CWizCategoryViewAllTagsItem* CWizCategoryTagsView::findAllTags()
{
    int nCount = topLevelItemCount();
    for (int i = 0; i < nCount; i++) {
        if (CWizCategoryViewAllTagsItem* pItem = dynamic_cast<CWizCategoryViewAllTagsItem*>(topLevelItem(i))) {
            return pItem;
        }
    }

    Q_ASSERT(false);

    return NULL;
}

CWizCategoryViewTagItem* CWizCategoryTagsView::findTag(const WIZTAGDATA& tag, bool create, bool sort)
{
    CWizStdStringArray arrayGUID;
    if (!m_db.GetAllParentsTagGUID(tag.strGUID, arrayGUID))
        return NULL;

    arrayGUID.insert(arrayGUID.begin(), tag.strGUID);   //insert self

    CWizCategoryViewAllTagsItem* pAllTags = findAllTags();
    if (!pAllTags)
        return NULL;

    QTreeWidgetItem* parent = pAllTags;

    size_t nCount = arrayGUID.size();
    for (intptr_t i = nCount - 1; i >= 0; i--)
    {
        CString strParentTagGUID = arrayGUID[i];

        WIZTAGDATA tagParent;
        if (!m_db.TagFromGUID(strParentTagGUID, tagParent))
            return NULL;

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

        if (found)
            continue;

        if (!create)
            return NULL;

        CWizCategoryViewTagItem* pTagItem = new CWizCategoryViewTagItem(m_app, tagParent);
        parent->addChild(pTagItem);
        parent->setExpanded(true);
        parent = pTagItem;

        if (sort) {
            parent->sortChildren(0, Qt::AscendingOrder);
        }
    }

    return dynamic_cast<CWizCategoryViewTagItem *>(parent);
}

CWizCategoryViewTagItem* CWizCategoryTagsView::findTagInTree(const WIZTAGDATA& tag)
{
    CWizCategoryViewAllTagsItem* pAllTags = findAllTags();
    if (!pAllTags)
        return NULL;

    return findTagInTree(tag, pAllTags);
}

CWizCategoryViewTagItem* CWizCategoryTagsView::findTagInTree(const WIZTAGDATA& tag, QTreeWidgetItem* itemParent)
{
    for (int i = 0; i < itemParent->childCount(); i++) {
        QTreeWidgetItem* it = itemParent->child(i);

        if (CWizCategoryViewTagItem* item = dynamic_cast<CWizCategoryViewTagItem*>(it)) {
            if (item->tag().strGUID == tag.strGUID)
                return item;
        }

        if (CWizCategoryViewTagItem* childItem = findTagInTree(tag, it)) {
            return childItem;
        }
    }

    return NULL;
}

CWizCategoryViewTagItem* CWizCategoryTagsView::addTag(const WIZTAGDATA& tag, bool sort)
{
    return findTag(tag, true, sort);
}

CWizCategoryViewTagItem* CWizCategoryTagsView::addTagWithChildren(const WIZTAGDATA& tag)
{
    CWizCategoryViewTagItem* pItem = findTag(tag, true, true);
    if (!pItem)
        return NULL;

    CWizTagDataArray arrayTag;
    m_db.GetChildTags(tag.strGUID, arrayTag);

    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        addTagWithChildren(*it);
    }

    return pItem;
}

void CWizCategoryTagsView::removeTag(const WIZTAGDATA& tag)
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

void CWizCategoryTagsView::on_tag_created(const WIZTAGDATA& tag)
{
    addTagWithChildren(tag);
}

void CWizCategoryTagsView::on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew)
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

void CWizCategoryTagsView::on_tag_deleted(const WIZTAGDATA& tag)
{
    removeTag(tag);
}

void CWizCategoryTagsView::on_action_newTag()
{
    NewTagDialog dlg;
    if (QDialog::Accepted != dlg.exec())
        return;

    CString strTagNames = dlg.tagName();
    if (strTagNames.IsEmpty())
        return;

    WIZTAGDATA parentTag;

    if (CWizCategoryViewTagItem* p = currentCategoryItem<CWizCategoryViewTagItem>()) {
        parentTag = p->tag();
    }

    QStringList sl = strTagNames.split(';');
    QStringList::const_iterator it;
    for (it = sl.begin(); it != sl.end(); it++) {
        CString strTagName = *it;

        WIZTAGDATA tagNew;
        m_db.CreateTag(parentTag.strGUID, strTagName, "", tagNew);
    }
}

void CWizCategoryTagsView::on_action_deleteTag()
{
    CWaitCursor wait;
    Q_UNUSED(wait);

    if (CWizCategoryViewTagItem* p = currentCategoryItem<CWizCategoryViewTagItem>()) {
        WIZTAGDATA tag = p->tag();
        m_db.DeleteTagWithChildren(tag, TRUE);
    }
}
