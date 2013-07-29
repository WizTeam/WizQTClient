#include "wizCategoryView.h"

#ifdef BUILD_WITH_QT5
#include <QtWidgets>
#endif

#include <QtGui>

#include "widgets/wizScrollBar.h"

#include "wizmainwindow.h"
#include "wizProgressDialog.h"
#include "share/wizdrawtexthelper.h"
#include "wiznotestyle.h"
#include "share/wizsettings.h"
#include "share/wizDatabaseManager.h"
#include "wizFolderSelector.h"

/* ------------------------------ CWizCategoryBaseView ------------------------------ */

CWizCategoryBaseView::CWizCategoryBaseView(CWizExplorerApp& app, QWidget* parent)
    : QTreeWidget(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_bDragHovered(false)
    , m_selectedItem(NULL)
{
    header()->hide();
    setAnimated(true);
    setFrameStyle(QFrame::NoFrame);
    viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setAutoFillBackground(true);
    setTextElideMode(Qt::ElideMiddle);
    setIndentation(12);

    //m_kineticScroller = new QScrollAreaKineticScroller();
    //m_kineticScroller->setWidget(this);

    // use custom scrollbar
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_vScroll = new CWizScrollBar(this);
    m_vScroll->syncWith(verticalScrollBar());

    // setup background
    QPixmap pixmapBg;
    pixmapBg.load(::WizGetResourcesPath() + "skins/leftview_bg.png");
    QBrush brushBg(pixmapBg);
    QPalette pal = palette();
    pal.setBrush(QPalette::Base, brushBg);
    setPalette(pal);

    setStyle(::WizGetStyle(m_app.userSettings().skin()));

    connect(&m_dbMgr, SIGNAL(documentCreated(const WIZDOCUMENTDATA&)),
            SLOT(on_document_created(const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(documentModified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)),
            SLOT(on_document_modified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(documentDeleted(const WIZDOCUMENTDATA&)),
            SLOT(on_document_deleted(const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(documentTagModified(const WIZDOCUMENTDATA&)),
            SLOT(on_document_tag_modified(const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(folderCreated(const QString&)),
            SLOT(on_folder_created(const QString&)));

    connect(&m_dbMgr, SIGNAL(folderDeleted(const QString&)),
            SLOT(on_folder_deleted(const QString&)));

    connect(&m_dbMgr, SIGNAL(tagCreated(const WIZTAGDATA&)),
            SLOT(on_tag_created(const WIZTAGDATA&)));

    connect(&m_dbMgr, SIGNAL(tagModified(const WIZTAGDATA&, const WIZTAGDATA&)),
            SLOT(on_tag_modified(const WIZTAGDATA&, const WIZTAGDATA&)));

    connect(&m_dbMgr, SIGNAL(tagDeleted(const WIZTAGDATA&)),
            SLOT(on_tag_deleted(const WIZTAGDATA&)));

    connect(&m_dbMgr, SIGNAL(databaseOpened(const QString&)),
            SLOT(on_group_opened(const QString&)));

    connect(&m_dbMgr, SIGNAL(databaseClosed(const QString&)),
            SLOT(on_group_closed(const QString&)));

    connect(&m_dbMgr, SIGNAL(databaseRename(const QString&)),
            SLOT(on_group_rename(const QString&)));

    connect(&m_dbMgr, SIGNAL(databasePermissionChanged(const QString&)),
            SLOT(on_group_permissionChanged(const QString&)));
}

void CWizCategoryBaseView::baseInit()
{
    init();
}

void CWizCategoryBaseView::resizeEvent(QResizeEvent* event)
{
    // reset scrollbar
    m_vScroll->resize(m_vScroll->sizeHint().width(), event->size().height());
    m_vScroll->move(event->size().width() - m_vScroll->sizeHint().width(), 0);

    QTreeWidget::resizeEvent(event);
}

void CWizCategoryBaseView::addSeparator()
{
    addTopLevelItem(new CWizCategoryViewSeparatorItem(m_app));
}

CWizCategoryViewSpacerItem* CWizCategoryBaseView::addSpacer()
{
    CWizCategoryViewSpacerItem* spacer = new CWizCategoryViewSpacerItem(m_app);
    addTopLevelItem(spacer);
    spacer->setHidden(true);
    return spacer;
}

QString CWizCategoryBaseView::selectedItemKbGUID()
{
    QList<QTreeWidgetItem*> items = selectedItems();
    if (!items.isEmpty()) {
        CWizCategoryViewItemBase* pItem = dynamic_cast<CWizCategoryViewItemBase*>(items.first());
        if (!pItem)
            return NULL;

        return pItem->kbGUID();
    }

    return NULL;
}

void CWizCategoryBaseView::getDocuments(CWizDocumentDataArray& arrayDocument)
{
    QList<QTreeWidgetItem*> items = selectedItems();
    if (items.empty())
        return;

    CWizCategoryViewItemBase* pItem = dynamic_cast<CWizCategoryViewItemBase*>(items.first());
    if (!pItem)
        return;

//    if (pItem->kbGUID().isEmpty()) {
//        return;
//    }

    pItem->getDocuments(m_dbMgr.db(pItem->kbGUID()), arrayDocument);
}

bool CWizCategoryBaseView::acceptDocument(const WIZDOCUMENTDATA& document)
{
    QList<QTreeWidgetItem*> items = selectedItems();
    if (items.empty())
        return false;

    CWizCategoryViewItemBase* pItem = dynamic_cast<CWizCategoryViewItemBase*>(items.first());
    if (!pItem)
        return false;

    return pItem->accept(m_dbMgr.db(document.strKbGUID), document);
}

void CWizCategoryBaseView::saveSelection()
{
    m_selectedItem = currentItem();
    clearSelection();
}

void CWizCategoryBaseView::restoreSelection()
{
    if (!m_selectedItem) {
        return;
    }

    setCurrentItem(m_selectedItem);
    m_selectedItem = NULL;

    //Q_EMIT itemSelectionChanged();
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
    if (NULL != dynamic_cast<const CWizCategoryViewSeparatorItem*>(pItem)) {
        return true;
    } else if (NULL != dynamic_cast<const CWizCategoryViewSpacerItem*>(pItem)) {
        return true;
    }
    return false;
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
                if (m_dbMgr.db().DocumentFromGUID(strDocumentGUID, dataDocument))
                {
                    CWizDocument doc(m_dbMgr.db(), dataDocument);
                    doc.AddTag(item->tag());
                }
            }
        }
        else if (CWizCategoryViewFolderItem* item = dynamic_cast<CWizCategoryViewFolderItem*>(itemAt(event->pos())))
        {
            CWizFolder folder(m_dbMgr.db(), item->location());

            foreach (const CString& strDocumentGUID, arrayDocumentGUID)
            {
                WIZDOCUMENTDATA dataDocument;
                if (m_dbMgr.db().DocumentFromGUID(strDocumentGUID, dataDocument))
                {
                    CWizDocument doc(m_dbMgr.db(), dataDocument);
                    doc.MoveDocument(&folder);
                }
            }
        }

        event->acceptProposedAction();
    }
}

void CWizCategoryBaseView::showTrashContextMenu(QPoint pos, const QString& strKbGUID)
{
    if (!m_menuTrash) {
        m_menuTrash = new QMenu(this);
        m_menuTrash->addAction(tr("Empty the Trash"), this, SLOT(on_action_emptyTrash()));
    }

    m_strTrashKbGUID = strKbGUID;
    m_menuTrash->popup(pos);
}

void CWizCategoryBaseView::on_action_emptyTrash()
{
    CWaitCursor wait;
    Q_UNUSED(wait);

    CWizCategoryViewTrashItem* pTrashItem = findTrash(m_strTrashKbGUID);
    if (!pTrashItem) {
        return;
    }

    CWizDocumentDataArray arrayDocument;
    pTrashItem->getDocuments(m_dbMgr.db(m_strTrashKbGUID), arrayDocument);

    foreach (const WIZDOCUMENTDATA& data, arrayDocument)
    {
        CWizDocument doc(m_dbMgr.db(m_strTrashKbGUID), data);
        doc.Delete();
    }
}

void CWizCategoryBaseView::on_document_created(const WIZDOCUMENTDATA& document)
{
    onDocument_created(document);
    updateDocumentCount(document.strKbGUID);
}

void CWizCategoryBaseView::on_document_modified(const WIZDOCUMENTDATA& documentOld,
                                                const WIZDOCUMENTDATA& documentNew)
{
    onDocument_modified(documentOld, documentNew);
    updateDocumentCount(documentNew.strKbGUID);
}

void CWizCategoryBaseView::on_document_deleted(const WIZDOCUMENTDATA& document)
{
    onDocument_deleted(document);
    updateDocumentCount(document.strKbGUID);
}

void CWizCategoryBaseView::on_document_tag_modified(const WIZDOCUMENTDATA& document)
{
    updateDocumentCount(document.strKbGUID);
}

void CWizCategoryBaseView::on_folder_created(const QString& strLocation)
{
    onFolder_created(strLocation);
}

void CWizCategoryBaseView::on_folder_deleted(const QString& strLocation)
{
    onFolder_deleted(strLocation);
}

void CWizCategoryBaseView::on_tag_created(const WIZTAGDATA& tag)
{
    onTag_created(tag);
}

void CWizCategoryBaseView::on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew)
{
    onTag_modified(tagOld, tagNew);
}

void CWizCategoryBaseView::on_tag_deleted(const WIZTAGDATA& tag)
{
    onTag_deleted(tag);
}

void CWizCategoryBaseView::on_group_opened(const QString& strKbGUID)
{
    onGroup_opened(strKbGUID);
}

void CWizCategoryBaseView::on_group_closed(const QString& strKbGUID)
{
    onGroup_closed(strKbGUID);
}

void CWizCategoryBaseView::on_group_rename(const QString& strKbGUID)
{
    onGroup_rename(strKbGUID);
}

void CWizCategoryBaseView::on_group_permissionChanged(const QString& strKbGUID)
{
    onGroup_permissionChanged(strKbGUID);
}


/* ------------------------------ CWizCategoryPrivateView ------------------------------ */

#define WIZACTION_PRIVATE_NEW_DOCUMENT    QObject::tr("Create new document")
#define WIZACTION_PRIVATE_NEW_FOLDER      QObject::tr("Create new folder")
#define WIZACTION_PRIVATE_MOVE_FOLDER     QObject::tr("Move current folder")
#define WIZACTION_PRIVATE_RENAME_FOLDER   QObject::tr("Rename current folder")
#define WIZACTION_PRIVATE_DELETE_FOLDER   QObject::tr("Delete current folder")

CWizCategoryView::CWizCategoryView(CWizExplorerApp& app, QWidget* parent)
    : CWizCategoryBaseView(app, parent)
{
    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);

    // setup context menu
    m_menuAllFolders = new QMenu(this);
    QAction* actionNewFolder = m_menuAllFolders->addAction(WIZACTION_PRIVATE_NEW_FOLDER, this, SLOT(on_action_newFolder()), QKeySequence("Ctrl+Shift+N"));

    m_menuFolder = new QMenu(this);
    QAction* actionNewDoc = m_menuFolder->addAction(WIZACTION_PRIVATE_NEW_DOCUMENT, this, SLOT(on_action_newDocument()), QKeySequence::New);
    m_menuFolder->addAction(actionNewFolder);
    m_menuFolder->addSeparator();
    QAction* actionMoveFolder = m_menuFolder->addAction(WIZACTION_PRIVATE_MOVE_FOLDER, this, SLOT(on_action_moveFolder()), QKeySequence("Ctrl+Shift+M"));
    QAction* actionRenameFolder = m_menuFolder->addAction(WIZACTION_PRIVATE_RENAME_FOLDER, this, SLOT(on_action_renameFolder()), QKeySequence(Qt::Key_F2));
    QAction* actionDeleteFolder = m_menuFolder->addAction(WIZACTION_PRIVATE_DELETE_FOLDER, this, SLOT(on_action_deleteFolder()), QKeySequence::Delete);

    // add to widget's actions list
    addAction(actionNewDoc);
    addAction(actionNewFolder);
    addAction(actionMoveFolder);
    addAction(actionRenameFolder);
    addAction(actionDeleteFolder);

    actionNewDoc->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionNewFolder->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionMoveFolder->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionRenameFolder->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionDeleteFolder->setShortcutContext(Qt::WidgetWithChildrenShortcut);
}

void CWizCategoryView::init()
{
    initFolders();
    addSeparator();
    initTrash();
    initDocumentCount();

    setCurrentItem(findAllFolders());
}

void CWizCategoryView::initDocumentCount()
{
    std::map<CString, int> mapDocumentCount;
    if (!m_dbMgr.db().GetAllLocationsDocumentCount(mapDocumentCount)) {
        TOLOG("[ERROR]: Failed to get all locations count map");
        return;
    }

    // folder items
    if (CWizCategoryViewItemBase* item = dynamic_cast<CWizCategoryViewItemBase*>(topLevelItem(0))) {
        int nTotal = initDocumentCount(item, mapDocumentCount);
        item->setDocumentsCount(nTotal, false);
    }

    // trash item
    if (CWizCategoryViewTrashItem* itemTrash = dynamic_cast<CWizCategoryViewTrashItem*>(topLevelItem(2))) {
        itemTrash->setDocumentsCount(-1, true);
        itemTrash->setDocumentsCount(0, false);

        std::map<CString, int>::const_iterator it;
        for (it = mapDocumentCount.begin(); it != mapDocumentCount.end(); it++) {
            if (m_dbMgr.db().IsInDeletedItems(it->first)) {
                int nCurrent = itemTrash->getDocumentsCount(false);
                itemTrash->setDocumentsCount(nCurrent + it->second, false);
            }
        }
    }

    update();
}

int CWizCategoryView::initDocumentCount(CWizCategoryViewItemBase* item,
                                        const std::map<CString, int>& mapDocumentCount)
{
    // reset
    item->setDocumentsCount(-1, true);

    int nCurrent = 0;
    std::map<CString, int>::const_iterator itCurrent = mapDocumentCount.find(item->name());
    if (itCurrent != mapDocumentCount.end()) {
        nCurrent = itCurrent->second;
    }

    item->setDocumentsCount(nCurrent, false);

    int nCountAll = nCurrent;
    int nChild = item->childCount();
    for (int i = 0; i < nChild; i++) {
        if (CWizCategoryViewItemBase* itemChild = dynamic_cast<CWizCategoryViewItemBase*>(item->child(i))) {
            int nCountChild = initDocumentCount(itemChild, mapDocumentCount);
            if (itemChild->childCount() && nCountChild) {
                itemChild->setDocumentsCount(nCountChild, true);
            }

            nCountAll += nCountChild;
        }
    }

    return nCountAll;
}

void CWizCategoryView::updateDocumentCount(const QString& strKbGUID)
{
    if (strKbGUID != m_dbMgr.db().kbGUID()) {
        return;
    }

    initDocumentCount();
}

void CWizCategoryView::initFolders()
{
    CWizCategoryViewAllFoldersItem* pAllFoldersItem = new CWizCategoryViewAllFoldersItem(m_app, tr("Note Folders"), m_dbMgr.db().kbGUID());
    addTopLevelItem(pAllFoldersItem);

    CWizStdStringArray arrayAllLocation;
    m_dbMgr.db().GetAllLocations(arrayAllLocation);

    doLocationSanityCheck(arrayAllLocation);

    initFolders(pAllFoldersItem, QString(), arrayAllLocation);

    if (arrayAllLocation.empty()) {
        const QString strNotes("/My Notes/");
        m_dbMgr.db().AddExtraFolder(strNotes);
        m_dbMgr.db().SetObjectVersion("folder", 0);
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

void CWizCategoryView::initFolders(QTreeWidgetItem* pParent, \
                                   const QString& strParentLocation, \
                                   const CWizStdStringArray& arrayAllLocation)
{
    CWizStdStringArray arrayLocation;
    CWizDatabase::GetChildLocations(arrayAllLocation, strParentLocation, arrayLocation);

    CWizStdStringArray::const_iterator it;
    for (it = arrayLocation.begin(); it != arrayLocation.end(); it++) {
        CString strLocation = *it;

        if (m_dbMgr.db().IsInDeletedItems(strLocation))
            continue;

        CWizCategoryViewFolderItem* pFolderItem = new CWizCategoryViewFolderItem(m_app, strLocation, m_dbMgr.db().kbGUID());
        pParent->addChild(pFolderItem);

        initFolders(pFolderItem, strLocation, arrayAllLocation);
    }
}

void CWizCategoryView::doLocationSanityCheck(CWizStdStringArray& arrayLocation)
{
    int nCount = arrayLocation.size();
    for (intptr_t i = nCount - 1; i >= 0; i--) {
        QString strLocation = arrayLocation.at(i);

        if (strLocation.left(1) != "/" && strLocation.right(1) != "/") {
            qDebug() << "[doLocationSanityCheck]: find folder name have not respect location naming spec: " << strLocation;

            CWizDocumentDataArray arrayDocument;
            CWizDocumentDataArray::iterator itDoc;
            m_dbMgr.db().GetDocumentsByLocation(strLocation, arrayDocument);

            if (!strLocation.isEmpty()) {
                strLocation = "/" + strLocation + "/";
            } else {
                strLocation = "/My Notes/";
            }

            qDebug() << "[doLocationSanityCheck]: try to amend name to : " << strLocation << ", Total: " << arrayDocument.size();
            for (itDoc = arrayDocument.begin(); itDoc != arrayDocument.end(); itDoc++) {
                WIZDOCUMENTDATAEX& doc = *itDoc;

                qDebug() << "[doLocationSanityCheck]: title: [" + doc.strTitle + "] Location: [" + doc.strLocation + "]";
                doc.strLocation = strLocation;
                doc.nObjectPart = WIZKM_XMLRPC_OBJECT_PART_INFO;
                m_dbMgr.db().UpdateDocument(doc);
            }

            arrayLocation.erase(arrayLocation.begin() + i);
        }
    }
}


void CWizCategoryView::initTrash()
{
    addTopLevelItem(new CWizCategoryViewTrashItem(m_app, tr("Trash"), m_dbMgr.db().kbGUID()));
}

void CWizCategoryView::showAllFoldersContextMenu(QPoint pos)
{
    m_menuAllFolders->popup(pos);
}

void CWizCategoryView::showFolderContextMenu(QPoint pos)
{
    m_menuFolder->popup(pos);
}

//CWizCategoryViewSearchItem* CWizCategoryView::findSearch()
//{
//    int nCount = topLevelItemCount();
//    for (int i = 0; i < nCount; i++)
//    {
//        if (CWizCategoryViewSearchItem* pItem = dynamic_cast<CWizCategoryViewSearchItem*>(topLevelItem(i)))
//        {
//            return pItem;
//        }
//    }
//
//    addSeparator();
//
//    CWizCategoryViewSearchItem* pItem = new CWizCategoryViewSearchItem(m_app, "");
//    addTopLevelItem(pItem);
//
//    return pItem;
//}

CWizCategoryViewTrashItem* CWizCategoryView::findTrash(const QString& strKbGUID)
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

void CWizCategoryView::addAndSelectFolder(const CString& strLocation)
{
    if (QTreeWidgetItem* pItem = addFolder(strLocation, true)) {
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

CWizCategoryViewFolderItem* CWizCategoryView::findFolder(const QString& strLocation, bool create, bool sort)
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

CWizCategoryViewFolderItem* CWizCategoryView::addFolder(const QString& strLocation, bool sort)
{
    return findFolder(strLocation, true, sort);
}

void CWizCategoryView::onDocument_created(const WIZDOCUMENTDATA& document)
{
    // ignore signal from group database
    if (document.strKbGUID != m_dbMgr.db().kbGUID())
        return;

    if (m_dbMgr.db().IsInDeletedItems(document.strLocation))
        return;

    addFolder(document.strLocation, true);
}

void CWizCategoryView::onDocument_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew)
{
    Q_UNUSED(documentOld);

    // ignore signal from group database
    if (documentNew.strKbGUID != m_dbMgr.db().kbGUID())
        return;

    if (m_dbMgr.db().IsInDeletedItems(documentNew.strLocation))
        return;

    addFolder(documentNew.strLocation, true);
}

void CWizCategoryView::onFolder_created(const QString& strLocation)
{
    addFolder(strLocation, true);
}

void CWizCategoryView::onFolder_deleted(const QString& strLocation)
{
    if (CWizCategoryViewFolderItem* pFolder = findFolder(strLocation, false, false))
    {
        if (QTreeWidgetItem* parent = pFolder->parent())
        {
            parent->removeChild(pFolder);
        }
    }
}

void CWizCategoryView::on_action_newDocument()
{
    Q_EMIT newDocument();
}

void CWizCategoryView::on_action_newFolder()
{
    CWizLineInputDialog* dialog = new CWizLineInputDialog(tr("New folder"),
                                                          tr("Please input folder name: "),
                                                          "", this);
    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_newFolder_confirmed(int)));

    dialog->open();
}

void CWizCategoryView::on_action_newFolder_confirmed(int result)
{
    CWizLineInputDialog* dialog = qobject_cast<CWizLineInputDialog*>(sender());
    CString strFolderName = dialog->input();
    dialog->deleteLater();

    if (result != QDialog::Accepted) {
        return;
    }

    if (strFolderName.isEmpty())
        return;

    WizMakeValidFileNameNoPath(strFolderName);

    QString strLocation;

    if (CWizCategoryViewAllFoldersItem* p = currentCategoryItem<CWizCategoryViewAllFoldersItem>()) {
        Q_UNUSED(p);
        strLocation = "/" + strFolderName + "/";
    } else if (CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>()) {
        strLocation = p->location() + strFolderName + "/";
    }

    addAndSelectFolder(strLocation);
    m_dbMgr.db().AddExtraFolder(strLocation);
    m_dbMgr.db().SetObjectVersion("folder", 0);
}

void CWizCategoryView::on_action_moveFolder()
{
    CWizFolderSelector* selector = new CWizFolderSelector(tr("Move folder"), m_app, this);
    selector->setAcceptRoot(true);

    connect(selector, SIGNAL(finished(int)), SLOT(on_action_moveFolder_confirmed(int)));
    selector->open();
}

void CWizCategoryView::on_action_moveFolder_confirmed(int result)
{
    CWizFolderSelector* selector = qobject_cast<CWizFolderSelector*>(sender());
    QString strSelectedFolder = selector->selectedFolder();
    sender()->deleteLater();

    if (result != QDialog::Accepted) {
        return;
    }

    if (strSelectedFolder.isEmpty()) {
        return;
    }

    QString strLocation;
    if (CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>()) {
        QString strOldLocation = p->location();
        int n = strOldLocation.lastIndexOf("/", -2);
        strLocation = strSelectedFolder + strOldLocation.right(strOldLocation.length() - n - 1);

        // move all documents to new folder
        CWizFolder folder(m_dbMgr.db(), strOldLocation);
        connect(&folder, SIGNAL(moveDocument(int, int, const QString&, const QString&, const WIZDOCUMENTDATA&)),
                SLOT(on_action_moveFolder_confirmed_progress(int, int, const QString&, const QString&, const WIZDOCUMENTDATA&)));

        if (!CWizFolder::CanMove(strOldLocation, strLocation)) {
            return;
        }

        folder.MoveToLocation(strLocation);

        // hide progress dialog
        MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());
        mainWindow->progressDialog()->hide();

        addAndSelectFolder(strLocation);
        on_folder_deleted(strOldLocation);
    }
}

void CWizCategoryView::on_action_moveFolder_confirmed_progress(int nMax, int nValue,
                                                               const QString& strOldLocation,
                                                               const QString& strNewLocation,
                                                               const WIZDOCUMENTDATA& data)
{
    MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());
    CWizProgressDialog* progress = mainWindow->progressDialog();

    progress->setActionString(tr("Move Document: %1 to %2").arg(strOldLocation).arg(strNewLocation));
    progress->setNotifyString(data.strTitle);
    progress->setProgress(nMax, nValue);
    progress->open();
}

void CWizCategoryView::on_action_renameFolder()
{
    CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>();
    if (!p) {
        return;
    }

    CWizLineInputDialog* dialog = new CWizLineInputDialog(tr("Rename folder"),
                                                          tr("Please input new folder name: "),
                                                          p->name(), this);
    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_renameFolder_confirmed(int)));

    dialog->open();
}

void CWizCategoryView::on_action_renameFolder_confirmed(int result)
{
    CWizLineInputDialog* dialog = qobject_cast<CWizLineInputDialog*>(sender());
    CString strFolderName = dialog->input();
    dialog->deleteLater();

    if (result != QDialog::Accepted) {
        return;
    }

    if (strFolderName.isEmpty()) {
        return;
    }

    WizMakeValidFileNameNoPath(strFolderName);

    QString strLocation;
    if (CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>()) {
        QString strOldLocation = p->location();
        int n = strOldLocation.lastIndexOf("/", -2);
        strLocation = strOldLocation.left(n + 1) + strFolderName + "/";

        // move all documents to new folder
        CWizFolder folder(m_dbMgr.db(), strOldLocation);
        connect(&folder, SIGNAL(moveDocument(int, int, const QString&, const QString&, const WIZDOCUMENTDATA&)),
                SLOT(on_action_renameFolder_confirmed_progress(int, int, const QString&, const QString&, const WIZDOCUMENTDATA&)));
        folder.MoveToLocation(strLocation);

        // hide progress dialog
        MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());
        mainWindow->progressDialog()->hide();

        addAndSelectFolder(strLocation);
        on_folder_deleted(strOldLocation);
    }
}

void CWizCategoryView::on_action_renameFolder_confirmed_progress(int nMax, int nValue,
                                                                 const QString& strOldLocation,
                                                                 const QString& strNewLocation,
                                                                 const WIZDOCUMENTDATA& data)
{
    MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());
    CWizProgressDialog* progress = mainWindow->progressDialog();

    progress->setActionString(tr("Move Document: %1 to %2").arg(strOldLocation).arg(strNewLocation));
    progress->setNotifyString(data.strTitle);
    progress->setProgress(nMax, nValue);
    progress->open();
}

void CWizCategoryView::on_action_deleteFolder()
{
    CWaitCursor wait;
    Q_UNUSED(wait);

    CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>();
    if (!p)
        return;

    // setup warning messagebox
    QMessageBox* msgBox = new QMessageBox(this);
    msgBox->setWindowTitle(tr("Delete Folder"));
    msgBox->addButton(QMessageBox::Ok);
    msgBox->addButton(QMessageBox::Cancel);
    //msgBox->setWindowModality(Qt::ApplicationModal);

    QString strWarning = tr("Do you really want to delete all documents inside folder: %1 ? (All documents will move to trash folder and remove from cloud server)").arg(p->location());
    msgBox->setText(strWarning);
    msgBox->open(this, SLOT(on_action_deleteFolder_confirmed(int)));
}

void CWizCategoryView::on_action_deleteFolder_confirmed(int result)
{
    sender()->deleteLater();

    CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>();
    if (!p)
        return;

    if (result == QMessageBox::Ok) {
        CWizFolder folder(m_dbMgr.db(), p->location());
        folder.Delete();
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

    return new CWizFolder(m_dbMgr.db(), pItem->location());
}



/* ------------------------------ CWizCategoryTagsView ------------------------------ */

#define WIZACTION_CATEGORY_NEW_TAG      QObject::tr("New tag")
#define WIZACTION_CATEGORY_RENAME_TAG   QObject::tr("Rename current tag")
#define WIZACTION_CATEGORY_DELETE_TAG   QObject::tr("Delete current tag")

CWizCategoryTagsView::CWizCategoryTagsView(CWizExplorerApp& app, QWidget *parent)
    : CWizCategoryBaseView(app, parent)
    , m_menuAllTags(NULL)
    , m_menuTag(NULL)
{
    m_menuAllTags = new QMenu(this);
    QAction* actionNewTag = m_menuAllTags->addAction(WIZACTION_CATEGORY_NEW_TAG, this, SLOT(on_action_newTag()), QKeySequence("Ctrl+Shift+N"));

    m_menuTag = new QMenu(this);
    m_menuTag->addAction(actionNewTag);
    QAction* actionRenameTag = m_menuTag->addAction(WIZACTION_CATEGORY_RENAME_TAG, this, SLOT(on_action_renameTag()), QKeySequence(Qt::Key_F2));
    m_menuTag->addSeparator();
    QAction* actionDeleteTag = m_menuTag->addAction(WIZACTION_CATEGORY_DELETE_TAG, this, SLOT(on_action_deleteTag()), QKeySequence::Delete);

    // add to widget's actions list
    addAction(actionNewTag);
    addAction(actionRenameTag);
    addAction(actionDeleteTag);

    actionNewTag->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionRenameTag->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionDeleteTag->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    //setDragDropMode(QAbstractItemView::DragDrop);
}

void CWizCategoryTagsView::showAllTagsContextMenu(QPoint pos)
{
    m_menuAllTags->popup(pos);
}

void CWizCategoryTagsView::showTagContextMenu(QPoint pos)
{
    m_menuTag->popup(pos);
}

void CWizCategoryTagsView::init()
{
    initTags();
    initDocumentCount();
}

void CWizCategoryTagsView::initTags()
{
    CWizCategoryViewAllTagsItem* pAllTagsItem = new CWizCategoryViewAllTagsItem(m_app, tr("Tags"), m_dbMgr.db().kbGUID());
    addTopLevelItem(pAllTagsItem);
    pAllTagsItem->setExpanded(true);

    initTags(pAllTagsItem, "");
}

void CWizCategoryTagsView::initTags(QTreeWidgetItem* pParent, const QString& strParentTagGUID)
{
    CWizTagDataArray arrayTag;
    m_dbMgr.db().GetChildTags(strParentTagGUID, arrayTag);

    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        CWizCategoryViewTagItem* pTagItem = new CWizCategoryViewTagItem(m_app, *it, m_dbMgr.db().kbGUID());
        pParent->addChild(pTagItem);

        initTags(pTagItem, it->strGUID);
    }
}

void CWizCategoryTagsView::initDocumentCount()
{
    std::map<CString, int> mapDocumentCount;
    if (!m_dbMgr.db().GetAllTagsDocumentCount(mapDocumentCount)) {
        TOLOG("[ERROR]: Failed to get all tags count map");
        return;
    }

    // tags items
    if (CWizCategoryViewAllTagsItem* item = dynamic_cast<CWizCategoryViewAllTagsItem*>(topLevelItem(0))) {
        int nChild = item->childCount();
        for (int i = 0; i < nChild; i++) {
            if (CWizCategoryViewTagItem* itemChild = dynamic_cast<CWizCategoryViewTagItem*>(item->child(i))) {
                int nCountChild = initDocumentCount(itemChild, mapDocumentCount);
                if (itemChild->childCount() && nCountChild) {
                    itemChild->setDocumentsCount(nCountChild, true);
                } else {
                    itemChild->setDocumentsCount(-1, true);
                }
            }
        }
    }

    update();
}

int CWizCategoryTagsView::initDocumentCount(CWizCategoryViewTagItem* item,
                                            const std::map<CString, int>& mapDocumentCount)
{
    // reset
    item->setDocumentsCount(-1, true);

    int nCurrent = 0;
    std::map<CString, int>::const_iterator itCurrent = mapDocumentCount.find(item->tag().strGUID);
    if (itCurrent != mapDocumentCount.end()) {
        nCurrent = itCurrent->second;
    }

    item->setDocumentsCount(nCurrent, false);

    int nCountAll = nCurrent;
    int nChild = item->childCount();
    for (int i = 0; i < nChild; i++) {
        if (CWizCategoryViewTagItem* itemChild = dynamic_cast<CWizCategoryViewTagItem*>(item->child(i))) {
            int nCountChild = initDocumentCount(itemChild, mapDocumentCount);
            if (itemChild->childCount() && nCountChild) {
                itemChild->setDocumentsCount(nCountChild, true);
            }

            nCountAll += nCountChild;
        }
    }

    return nCountAll;
}

void CWizCategoryTagsView::updateDocumentCount(const QString& strKbGUID)
{
    if (strKbGUID != m_dbMgr.db().kbGUID()) {
        return;
    }

    initDocumentCount();
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
    if (!m_dbMgr.db().GetAllParentsTagGUID(tag.strGUID, arrayGUID))
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
        if (!m_dbMgr.db().TagFromGUID(strParentTagGUID, tagParent))
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

        CWizCategoryViewTagItem* pTagItem = new CWizCategoryViewTagItem(m_app, tagParent, m_dbMgr.db().kbGUID());
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
    m_dbMgr.db().GetChildTags(tag.strGUID, arrayTag);

    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        addTagWithChildren(*it);
    }

    return pItem;
}

void CWizCategoryTagsView::addAndSelectTag(const WIZTAGDATA& tag)
{
    if (QTreeWidgetItem* pItem = addTag(tag, true)) {
        setCurrentItem(pItem);
    }
}

void CWizCategoryTagsView::removeTag(const WIZTAGDATA& tag)
{
    CWizCategoryViewTagItem* pItem = findTagInTree(tag);
    if (pItem) {
        QTreeWidgetItem* parent = pItem->parent();
        if (parent) {
            parent->removeChild(pItem);
        }
    }
}

void CWizCategoryTagsView::onTag_created(const WIZTAGDATA& tag)
{
    // ignore signal from group datebase
    if (tag.strKbGUID != m_dbMgr.db().kbGUID())
        return;

    addTagWithChildren(tag);
}

void CWizCategoryTagsView::onTag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew)
{
    // ignore signal from group database
    if (tagNew.strKbGUID != m_dbMgr.db().kbGUID())
        return;

    if (tagOld.strParentGUID != tagNew.strParentGUID) {
        removeTag(tagOld);
        addTagWithChildren(tagNew);
    } else {
        CWizCategoryViewTagItem* pTagItem = addTagWithChildren(tagNew);
        if (pTagItem) {
            pTagItem->reload(m_dbMgr.db());
        }
    }
}

void CWizCategoryTagsView::onTag_deleted(const WIZTAGDATA& tag)
{
    // ignore signal from group database
    if (tag.strKbGUID != m_dbMgr.db().kbGUID())
        return;

    removeTag(tag);
}

void CWizCategoryTagsView::on_action_newTag()
{
    CWizLineInputDialog* dialog = new CWizLineInputDialog(tr("New tag"),
                                                          tr("Please input tag name: "),
                                                          "", this);
    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_newTag_confirmed(int)));

    dialog->open();
}

void CWizCategoryTagsView::on_action_newTag_confirmed(int result)
{
    CWizLineInputDialog* dialog = qobject_cast<CWizLineInputDialog*>(sender());
    QString strTagNames = dialog->input();
    dialog->deleteLater();

    if (result != QDialog::Accepted) {
        return;
    }

    if (strTagNames.isEmpty())
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

        // only create tag for unique name
        if (m_dbMgr.db().TagByName(strTagName, tagNew)) {
            TOLOG1("Tag name already exist: %1", strTagName);
            continue;
        }

        m_dbMgr.db().CreateTag(parentTag.strGUID, strTagName, "", tagNew);
    }
}

void CWizCategoryTagsView::on_action_renameTag()
{
    CWizCategoryViewTagItem* p = currentCategoryItem<CWizCategoryViewTagItem>();
    if (!p)
        return;

    CWizLineInputDialog* dialog = new CWizLineInputDialog(tr("Rename tag"),
                                                          tr("Please input tag name: "),
                                                          p->name(), this);
    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_renameTag_confirmed(int)));

    dialog->open();
}

void CWizCategoryTagsView::on_action_renameTag_confirmed(int result)
{
    CWizLineInputDialog* dialog = qobject_cast<CWizLineInputDialog*>(sender());
    QString strTagName = dialog->input();
    dialog->deleteLater();

    if (result != QDialog::Accepted) {
        return;
    }

    if (strTagName.isEmpty())
        return;

    if (CWizCategoryViewTagItem* p = currentCategoryItem<CWizCategoryViewTagItem>()) {
        WIZTAGDATA tag = p->tag();
        tag.strName = strTagName;
        m_dbMgr.db().ModifyTag(tag);
        p->reload(m_dbMgr.db());
    }
}

void CWizCategoryTagsView::on_action_deleteTag()
{
    CWaitCursor wait;
    Q_UNUSED(wait);

    CWizCategoryViewTagItem* p = currentCategoryItem<CWizCategoryViewTagItem>();
    if (!p)
        return;

    QMessageBox* msgBox = new QMessageBox(this);
    msgBox->setWindowTitle(tr("Delete tag"));
    msgBox->addButton(QMessageBox::Ok);
    msgBox->addButton(QMessageBox::Cancel);
    //msgBox->setWindowModality(Qt::ApplicationModal);

    QString strWarning = tr("Do you really want to delete tag: %1 ? (include child tags if any)").arg(p->tag().strName);
    msgBox->setText(strWarning);
    msgBox->open(this, SLOT(on_action_deleteTag_confirmed(int)));
}

void CWizCategoryTagsView::on_action_deleteTag_confirmed(int result)
{
    sender()->deleteLater();

    CWizCategoryViewTagItem* p = currentCategoryItem<CWizCategoryViewTagItem>();
    if (!p)
        return;

    if (result == QMessageBox::Ok) {
        WIZTAGDATA tag = p->tag();
        m_dbMgr.db().DeleteTagWithChildren(tag, TRUE);
    }
}


/* ------------------------------ CWizCategoryGroupsView ------------------------------ */

#define WIZ_PERSONAL_GROUP_ROOT_NAME QObject::tr("Personal groups")

#define WIZACTION_GROUP_MARK_READ       QObject::tr("Mark all documents read")
#define WIZACTION_GROUP_NEW_DOCUMENT    QObject::tr("Create new document")
#define WIZACTION_GROUP_NEW_FOLDER      QObject::tr("Create new folder")
#define WIZACTION_GROUP_OPEN_ATTRIBUTE  QObject::tr("Open group attribute")
#define WIZACTION_GROUP_RENAME_FOLDER   QObject::tr("Change current folder name")
#define WIZACTION_GROUP_DELETE_FOLDER   QObject::tr("Delete current folder")

CWizCategoryGroupsView::CWizCategoryGroupsView(CWizExplorerApp& app, QWidget* parent)
    : CWizCategoryBaseView(app, parent)
{
    connect(this, SIGNAL(itemSelectionChanged()), SLOT(on_itemSelectionChanged()));

    m_menuGroupRoot = new QMenu(this);
    //QAction* actionMarkRead = m_menuGroupRoot->addAction(WIZACTION_GROUP_MARK_READ, this, SLOT(on_action_markRead()));
    QAction* actionNewDoc = m_menuGroupRoot->addAction(WIZACTION_GROUP_NEW_DOCUMENT, this, SLOT(on_action_newDocument()), QKeySequence::New);
    QAction* actionNewFolder = m_menuGroupRoot->addAction(WIZACTION_GROUP_NEW_FOLDER, this, SLOT(on_action_newTag()), QKeySequence("Ctrl+Shift+N"));
    m_menuGroupRoot->addSeparator();
    m_menuGroupRoot->addAction(WIZACTION_GROUP_OPEN_ATTRIBUTE, this, SLOT(on_action_openGroupAttribute()));

    m_menuGroup = new QMenu(this);
    //m_menuGroup->addAction(actionMarkRead);
    m_menuGroup->addSeparator();
    m_menuGroup->addAction(actionNewDoc);
    m_menuGroup->addAction(actionNewFolder);
    QAction* actionRenameFolder = m_menuGroup->addAction(WIZACTION_GROUP_RENAME_FOLDER, this, SLOT(on_action_renameTag()), QKeySequence(Qt::Key_F2));
    m_menuGroup->addSeparator();
    QAction* actionDeleteFolder = m_menuGroup->addAction(WIZACTION_GROUP_DELETE_FOLDER, this, SLOT(on_action_deleteTag()), QKeySequence::Delete);


    // add to widget's actions list
    addAction(actionNewDoc);
    addAction(actionNewFolder);
    addAction(actionRenameFolder);
    addAction(actionDeleteFolder);

    actionNewDoc->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionNewFolder->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionRenameFolder->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionDeleteFolder->setShortcutContext(Qt::WidgetWithChildrenShortcut);
}

void CWizCategoryGroupsView::showGroupRootContextMenu(const QString& strKbGUID, QPoint pos)
{
    m_strKbGUID = strKbGUID;
    //onGroup_permissionChanged(strKbGUID);
    m_menuGroupRoot->popup(pos);
}

void CWizCategoryGroupsView::showGroupContextMenu(const QString& strKbGUID, QPoint pos)
{
    m_strKbGUID = strKbGUID;
    //onGroup_permissionChanged(strKbGUID);
    m_menuGroup->popup(pos);
}

void CWizCategoryGroupsView::init()
{
    // init biz groups
    QMap<QString, QString> bizInfo;
    m_dbMgr.db().getBizGroupInfo(bizInfo);

    QMap<QString, QString>::const_iterator it;
    for (it = bizInfo.begin(); it != bizInfo.end(); it++) {
        CWizCategoryViewBizGroupRootItem* pBizGroupItem = new CWizCategoryViewBizGroupRootItem(m_app, it.value(), "");
        addTopLevelItem(pBizGroupItem);
        pBizGroupItem->setExpanded(true);
    }

    // draw seperator only if biz group exist
    if (bizInfo.size()) {
        addSeparator();
    }

    // init personal groups
    CWizCategoryViewAllGroupsRootItem* pAllGroupsItem = new CWizCategoryViewAllGroupsRootItem(m_app, WIZ_PERSONAL_GROUP_ROOT_NAME, "");
    addTopLevelItem(pAllGroupsItem);
    pAllGroupsItem->setExpanded(true);

    int nTotal = m_dbMgr.count();
    for (int i = 0; i < nTotal; i++) {
        initGroup(m_dbMgr.at(i));
    }

    initDocumentCount();
}

void CWizCategoryGroupsView::initGroup(CWizDatabase& db)
{
    QTreeWidgetItem* pRoot = NULL;

    // if biz info exist, add group to it instead of All groups root
    if (!db.info().bizGUID.isEmpty() && !db.info().bizName.isEmpty()) {
        pRoot = findGroupSet(db.info().bizName, true);
    } else {
        pRoot = findGroupSet(WIZ_PERSONAL_GROUP_ROOT_NAME, true);
    }

    Q_ASSERT(pRoot);

    CWizCategoryViewGroupRootItem* pGroupItem = new CWizCategoryViewGroupRootItem(m_app, db.name(), db.kbGUID());
    pRoot->addChild(pGroupItem);
    initGroup(db, pGroupItem, "");

    pRoot->sortChildren(0, Qt::AscendingOrder);

    CWizCategoryViewTrashItem* pTrashItem = new CWizCategoryViewTrashItem(m_app, tr("Trash"), db.kbGUID());
    pGroupItem->addChild(pTrashItem);

    // only show trash if permission is enough
    if (db.permission() > WIZ_USERGROUP_SUPER) {
        pTrashItem->setHidden(true);
    }
}

void CWizCategoryGroupsView::initGroup(CWizDatabase& db, QTreeWidgetItem* pParent, const QString& strParentTagGUID)
{
    CWizTagDataArray arrayTag;
    db.GetChildTags(strParentTagGUID, arrayTag);

    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        CWizCategoryViewGroupItem* pTagItem = new CWizCategoryViewGroupItem(m_app, *it, db.kbGUID());
        pParent->addChild(pTagItem);

        initGroup(db, pTagItem, it->strGUID);
    }
}

void CWizCategoryGroupsView::initDocumentCount()
{
    int nCount = topLevelItemCount();
    for (int i = 0; i < nCount; i++) {
        QTreeWidgetItem* pRoot = topLevelItem(i);
        int nGroup = pRoot->childCount();

        for (int j = 0; j < nGroup; j++) {
            CWizCategoryViewGroupRootItem* p = dynamic_cast<CWizCategoryViewGroupRootItem*>(pRoot->child(j));
            if (p) {
                initDocumentCount(p, m_dbMgr.db(p->kbGUID()));
            }
        }
    }
}

void CWizCategoryGroupsView::initDocumentCount(CWizCategoryViewGroupRootItem* pGroupRoot,
                                               CWizDatabase& db)
{
    std::map<CString, int> mapDocumentCount;
    if (!db.GetAllTagsDocumentCount(mapDocumentCount)) {
        TOLOG("[ERROR]: Failed to get document count map: " + db.name());
        return;
    }

    int nGroupCount = 0;
    for (int j = 0; j < pGroupRoot->childCount(); j++) {
        int nCurrentGroupCount = 0;
        CWizCategoryViewGroupItem* pGroupItem = dynamic_cast<CWizCategoryViewGroupItem*>(pGroupRoot->child(j));
        if (pGroupItem) {
            nCurrentGroupCount = initDocumentCount(pGroupItem, mapDocumentCount);
            if (pGroupItem->childCount() && nCurrentGroupCount) {
                pGroupItem->setDocumentsCount(nCurrentGroupCount, true);
            }
        }

        CWizCategoryViewTrashItem* pTrashItem = dynamic_cast<CWizCategoryViewTrashItem*>(pGroupRoot->child(j));
        if (pTrashItem) {
            int nSize = 0;
            db.GetDocumentsSizeByLocation("/Deleted Items/", nSize, true);
            pTrashItem->setDocumentsCount(nSize, false);
        }

        nGroupCount += nCurrentGroupCount;
    }

    int nCount = 0;
    db.getDocumentsSizeNoTag(nCount);
    pGroupRoot->setDocumentsCount(nCount, false);
    pGroupRoot->setDocumentsCount(nGroupCount + nCount, true);

    update();
}

int CWizCategoryGroupsView::initDocumentCount(CWizCategoryViewGroupItem* item,
                                              const std::map<CString, int>& mapDocumentCount)
{
    // reset
    item->setDocumentsCount(-1, true);

    int nCurrent = 0;
    std::map<CString, int>::const_iterator itCurrent = mapDocumentCount.find(item->tag().strGUID);
    if (itCurrent != mapDocumentCount.end()) {
        nCurrent = itCurrent->second;
    }

    item->setDocumentsCount(nCurrent, false);

    int nCountAll = nCurrent;
    int nChild = item->childCount();
    for (int i = 0; i < nChild; i++) {
        if (CWizCategoryViewGroupItem* itemChild = dynamic_cast<CWizCategoryViewGroupItem*>(item->child(i))) {
            int nCountChild = initDocumentCount(itemChild, mapDocumentCount);
            if (itemChild->childCount() && nCountChild) {
                itemChild->setDocumentsCount(nCountChild, true);
            }

            nCountAll += nCountChild;
        }
    }

    return nCountAll;
}

void CWizCategoryGroupsView::updateDocumentCount(const QString& strKbGUID)
{
    if (strKbGUID == m_dbMgr.db().kbGUID()) {
        return;
    }

    CWizCategoryViewGroupRootItem* pGroupItem = findGroup(strKbGUID);
    if (!pGroupItem)
        return;

    initDocumentCount(pGroupItem, m_dbMgr.db(strKbGUID));
}

CWizCategoryViewGroupRootItem* CWizCategoryGroupsView::findGroup(const QString& strKbGUID)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        CWizCategoryViewAllGroupsRootItem* p = dynamic_cast<CWizCategoryViewAllGroupsRootItem *>(topLevelItem(i));

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

CWizCategoryViewAllGroupsRootItem* CWizCategoryGroupsView::findGroupSet(const QString& strName, bool bCreate)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        CWizCategoryViewAllGroupsRootItem* pItem = dynamic_cast<CWizCategoryViewAllGroupsRootItem *>(topLevelItem(i));
        if (pItem && (pItem->name() == strName)) {
            return pItem;
        }
    }

    CWizCategoryViewAllGroupsRootItem* p = NULL;
    if (bCreate) {
        if (strName == WIZ_PERSONAL_GROUP_ROOT_NAME) {
            p = new CWizCategoryViewAllGroupsRootItem(m_app, strName, "");
            addTopLevelItem(p);
        } else {
            p = new CWizCategoryViewBizGroupRootItem(m_app, strName, "");
            insertTopLevelItem(0, p);
        }

        p->setExpanded(true);
    }

    return p;
}

CWizCategoryViewTrashItem* CWizCategoryGroupsView::findTrash(const QString& strKbGUID)
{
    CWizCategoryViewGroupRootItem* pGroupItem = findGroup(strKbGUID);
    if (!pGroupItem) {
        return NULL;
    }

    int nCount = pGroupItem->childCount();
    for (int i = 0; i < nCount; i++) {
        CWizCategoryViewTrashItem * pTrashItem = dynamic_cast<CWizCategoryViewTrashItem *>(pGroupItem->child(i));
        if (pTrashItem) {
            return pTrashItem;
        }
    }

    Q_ASSERT(0);
    return NULL;
}

QAction* CWizCategoryGroupsView::findAction(const QString& strName)
{
    QList<QAction *> actionList;
    actionList.append(m_menuGroupRoot->actions());
    actionList.append(m_menuGroup->actions());

    QList<QAction *>::const_iterator it;
    for (it = actionList.begin(); it != actionList.end(); it++) {
        QAction* action = *it;
        if (action->text() == strName) {
            return action;
        }
    }

    Q_ASSERT(0);
    return NULL;
}

CWizCategoryViewGroupItem* CWizCategoryGroupsView::findTagInTree(const WIZTAGDATA& tag)
{
    CWizCategoryViewGroupRootItem* pItem = findGroup(tag.strKbGUID);
    if (!pItem)
        return NULL;

    return findTagInTree(tag, pItem);
}

CWizCategoryViewGroupItem* CWizCategoryGroupsView::findTagInTree(const WIZTAGDATA& tag, QTreeWidgetItem* itemParent)
{
    for (int i = 0; i < itemParent->childCount(); i++) {
        QTreeWidgetItem* it = itemParent->child(i);

        if (CWizCategoryViewGroupItem* item = dynamic_cast<CWizCategoryViewGroupItem*>(it)) {
            if (item && item->tag().strGUID == tag.strGUID
                    && item->tag().strKbGUID == tag.strKbGUID)
                return item;
        }

        if (CWizCategoryViewGroupItem* childItem = findTagInTree(tag, it)) {
            return childItem;
        }
    }

    return NULL;
}

void CWizCategoryGroupsView::removeTag(const WIZTAGDATA& tag)
{
    CWizCategoryViewGroupItem* pItem = findTagInTree(tag);
    if (pItem) {
        QTreeWidgetItem* parent = pItem->parent();
        if (parent) {
            parent->removeChild(pItem);
        }
    }
}

CWizCategoryViewGroupItem* CWizCategoryGroupsView::addTagWithChildren(const WIZTAGDATA& tag)
{
    CWizCategoryViewGroupItem* pItem = findTag(tag, true, true);
    if (!pItem)
        return NULL;

    CWizTagDataArray arrayTag;
    m_dbMgr.db(tag.strKbGUID).GetChildTags(tag.strGUID, arrayTag);

    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        addTagWithChildren(*it);
    }

    return pItem;
}

CWizCategoryViewGroupItem* CWizCategoryGroupsView::findTag(const WIZTAGDATA& tag, bool create, bool sort)
{
    CWizStdStringArray arrayGUID;
    if (!m_dbMgr.db(tag.strKbGUID).GetAllParentsTagGUID(tag.strGUID, arrayGUID))
        return NULL;

    arrayGUID.insert(arrayGUID.begin(), tag.strGUID);   //insert self

    CWizCategoryViewGroupRootItem* pItem = findGroup(tag.strKbGUID);
    if (!pItem)
        return NULL;

    QTreeWidgetItem* parent = pItem;

    size_t nCount = arrayGUID.size();
    for (intptr_t i = nCount - 1; i >= 0; i--) {
        CString strParentTagGUID = arrayGUID[i];

        WIZTAGDATA tagParent;
        if (!m_dbMgr.db(tag.strKbGUID).TagFromGUID(strParentTagGUID, tagParent))
            return NULL;

        bool found = false;
        int nCount = parent->childCount();
        for (int i = 0; i < nCount; i++) {
            CWizCategoryViewGroupItem* pTag = dynamic_cast<CWizCategoryViewGroupItem*>(parent->child(i));
            if (pTag && pTag->tag().strGUID == tagParent.strGUID
                    && pTag->tag().strKbGUID == tagParent.strKbGUID)
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

        // always add trash item to the end of group
        CWizCategoryViewGroupItem* pTagItem = new CWizCategoryViewGroupItem(m_app, tagParent, tag.strKbGUID);
        CWizCategoryViewTrashItem* pTrash = findTrash(tag.strKbGUID);
        if (pTrash)
            parent->takeChild(parent->indexOfChild(pTrash));

        parent->addChild(pTagItem);
        if (sort) {
            parent->sortChildren(0, Qt::AscendingOrder);
        }

        if (pTrash)
            parent->addChild(pTrash);

        parent->setExpanded(true);
        parent = pTagItem;
    }

    return dynamic_cast<CWizCategoryViewGroupItem *>(parent);
}

void CWizCategoryGroupsView::onGroup_opened(const QString& strKbGUID)
{
    initGroup(m_dbMgr.db(strKbGUID));
}

void CWizCategoryGroupsView::onGroup_closed(const QString& strKbGUID)
{
    CWizCategoryViewGroupRootItem* pItem = findGroup(strKbGUID);
    if (pItem) {
        QTreeWidgetItem* parent = pItem->parent();
        if (parent) {
            parent->removeChild(pItem);
        }
    }
}

void CWizCategoryGroupsView::onGroup_rename(const QString& strKbGUID)
{
    CWizCategoryViewGroupRootItem* pItem = findGroup(strKbGUID);
    if (pItem) {
        pItem->reload(m_dbMgr.db(strKbGUID));
    }
}

void CWizCategoryGroupsView::onGroup_permissionChanged(const QString& strKbGUID)
{
    int nPerm = m_dbMgr.db(strKbGUID).permission();

    // only Admin and Super user see trash folder and operate with tag (folder)
    if (nPerm > WIZ_USERGROUP_SUPER) {
        findTrash(strKbGUID)->setHidden(true);
        findAction(WIZACTION_GROUP_NEW_FOLDER)->setEnabled(false);
        findAction(WIZACTION_GROUP_RENAME_FOLDER)->setEnabled(false);
        findAction(WIZACTION_GROUP_DELETE_FOLDER)->setEnabled(false);
    } else {
        findTrash(strKbGUID)->setHidden(false);
        findAction(WIZACTION_GROUP_NEW_FOLDER)->setEnabled(true);
        findAction(WIZACTION_GROUP_RENAME_FOLDER)->setEnabled(true);
        findAction(WIZACTION_GROUP_DELETE_FOLDER)->setEnabled(true);
    }

    // permission greater than author can create new document
    if (nPerm >= WIZ_USERGROUP_READER) {
        findAction(WIZACTION_GROUP_NEW_DOCUMENT)->setEnabled(false);
    } else {
        findAction(WIZACTION_GROUP_NEW_DOCUMENT)->setEnabled(true);
    }
}

void CWizCategoryGroupsView::onTag_created(const WIZTAGDATA& tag)
{
    // ignore signal from private database
    if (tag.strKbGUID == m_dbMgr.db().kbGUID())
        return;

    addTagWithChildren(tag);
}

void CWizCategoryGroupsView::onTag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew)
{
    // ignore signal from private database
    if (tagNew.strKbGUID == m_dbMgr.db().kbGUID())
        return;

    if (tagOld.strParentGUID != tagNew.strParentGUID) {
        removeTag(tagOld);
        addTagWithChildren(tagNew);
    } else {
        CWizCategoryViewGroupItem* pTagItem = addTagWithChildren(tagNew);
        if (pTagItem) {
            pTagItem->reload(m_dbMgr.db(tagNew.strKbGUID));
        }
    }
}

void CWizCategoryGroupsView::onTag_deleted(const WIZTAGDATA& tag)
{
    // ignore signal from private database
    if (tag.strKbGUID == m_dbMgr.db().kbGUID())
        return;

    removeTag(tag);
}

void CWizCategoryGroupsView::on_itemSelectionChanged()
{
    CWizCategoryViewGroupRootItem* pRoot = currentCategoryItem<CWizCategoryViewGroupRootItem>();
    CWizCategoryViewGroupItem* pItem = currentCategoryItem<CWizCategoryViewGroupItem>();
    if (pRoot || pItem) {
        CWizCategoryViewItemBase* p = currentCategoryItem<CWizCategoryViewItemBase>();
        on_group_permissionChanged(p->kbGUID());
    }
}

void CWizCategoryGroupsView::on_action_markRead()
{

}

void CWizCategoryGroupsView::on_action_newDocument()
{

}

void CWizCategoryGroupsView::on_action_openGroupAttribute()
{
    Q_ASSERT(!m_strKbGUID.isEmpty());

    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    mainWindow->groupAttributeForm()->sheetShow(m_strKbGUID);
}

void CWizCategoryGroupsView::on_action_newTag()
{
    // should not all groups root item
    CWizCategoryViewAllGroupsRootItem* pRoot = currentCategoryItem<CWizCategoryViewAllGroupsRootItem>();
    if (pRoot)
        return;

    CWizLineInputDialog* dialog = new CWizLineInputDialog(tr("New group folder"),
                                                          tr("Please input folder name: "),
                                                          "", this);
    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_newTag_confirmed(int)));

    dialog->open();
}

void CWizCategoryGroupsView::on_action_newTag_confirmed(int result)
{
    CWizLineInputDialog* dialog = qobject_cast<CWizLineInputDialog*>(sender());
    QString strTagNames = dialog->input();
    dialog->deleteLater();

    if (result != QDialog::Accepted) {
        return;
    }

    if (strTagNames.isEmpty())
        return;

    WIZTAGDATA parentTag;
    QString strKbGUID;

    if (CWizCategoryViewGroupRootItem* pRoot = currentCategoryItem<CWizCategoryViewGroupRootItem>()) {
        strKbGUID = pRoot->kbGUID();
    }

    if (CWizCategoryViewGroupItem* p = currentCategoryItem<CWizCategoryViewGroupItem>()) {
        strKbGUID = p->kbGUID();
        parentTag = p->tag();
    }

    if (strKbGUID.isEmpty()) {
        Q_ASSERT(0);
        return;
    }

    QStringList sl = strTagNames.split(';');
    QStringList::const_iterator it;
    for (it = sl.begin(); it != sl.end(); it++) {
        CString strTagName = *it;

        WIZTAGDATA tagNew;
        m_dbMgr.db(strKbGUID).CreateTag(parentTag.strGUID, strTagName, "", tagNew);
    }
}

void CWizCategoryGroupsView::on_action_renameTag()
{
    CWizCategoryViewGroupItem* p = currentCategoryItem<CWizCategoryViewGroupItem>();
    if (!p)
        return;

    CWizLineInputDialog* dialog = new CWizLineInputDialog(tr("Rename group folder"),
                                                          tr("Please input folder name: "),
                                                          p->name(), this);
    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_renameTag_confirmed(int)));

    dialog->open();
}

void CWizCategoryGroupsView::on_action_renameTag_confirmed(int result)
{
    CWizLineInputDialog* dialog = qobject_cast<CWizLineInputDialog*>(sender());
    QString strTagName = dialog->input();
    dialog->deleteLater();

    if (result != QDialog::Accepted) {
        return;
    }

    if (strTagName.isEmpty()) {
        return;
    }

    if (CWizCategoryViewGroupItem* p = currentCategoryItem<CWizCategoryViewGroupItem>()) {
        WIZTAGDATA tag = p->tag();
        tag.strName = strTagName;
        m_dbMgr.db(tag.strKbGUID).ModifyTag(tag);
        p->reload(m_dbMgr.db(tag.strKbGUID));
    }
}

void CWizCategoryGroupsView::on_action_deleteTag()
{
    CWaitCursor wait;
    Q_UNUSED(wait);

    CWizCategoryViewGroupItem* p = currentCategoryItem<CWizCategoryViewGroupItem>();
    if (!p)
        return;

    QMessageBox* msgBox = new QMessageBox(this);
    msgBox->setWindowTitle(tr("Delete group folder"));
    msgBox->addButton(QMessageBox::Ok);
    msgBox->addButton(QMessageBox::Cancel);
    //msgBox->setWindowModality(Qt::ApplicationModal);

    QString strWarning = tr("Do you really want to delete folder: %1 ? (All documents will move to root folder, It's safe.)").arg(p->tag().strName);
    msgBox->setText(strWarning);
    msgBox->open(this, SLOT(on_action_deleteTag_confirmed(int)));
}

void CWizCategoryGroupsView::on_action_deleteTag_confirmed(int result)
{
    sender()->deleteLater();

    CWizCategoryViewGroupItem* p = currentCategoryItem<CWizCategoryViewGroupItem>();
    if (!p)
        return;

    if (result == QMessageBox::Ok) {
        WIZTAGDATA tag = p->tag();
        m_dbMgr.db(p->kbGUID()).DeleteTagWithChildren(tag, true);
    }
}
