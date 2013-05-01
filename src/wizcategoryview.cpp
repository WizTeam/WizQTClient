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
#include <QUrl>

#include "wizmainwindow.h"

#include "share/wizdrawtexthelper.h"
#include "wiznotestyle.h"

#include "newtagdialog.h"
#include "share/wizsettings.h"
#include "share/wizDatabaseManager.h"
#include "widgets/qscrollareakineticscroller.h"

/* ------------------------------ CWizCategoryBaseView ------------------------------ */

CWizCategoryBaseView::CWizCategoryBaseView(CWizExplorerApp& app, QWidget* parent)
    : QTreeWidget(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_bDragHovered(false)
    , m_selectedItem(NULL)
{
    header()->hide();
    setFrameStyle(QFrame::NoFrame);
    viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setAutoFillBackground(true);
    setTextElideMode(Qt::ElideMiddle);
    setIndentation(12);

    //m_kineticScroller = new QScrollAreaKineticScroller();
    //m_kineticScroller->setWidget(this);

    // use custom scrollbar
    setAnimated(true);
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

void CWizCategoryBaseView::saveSelection(const QString& str)
{
    Q_UNUSED(str);

    m_selectedItem = currentItem();
    clearSelection();
}

void CWizCategoryBaseView::restoreSelection()
{
    if (m_selectedItem) {
        setCurrentItem(m_selectedItem);
        Q_EMIT itemSelectionChanged();
    }
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



/* ------------------------------ CWizCategoryPrivateView ------------------------------ */

#define WIZACTION_PRIVATE_NEW_DOCUMENT    QObject::tr("Create new document")
#define WIZACTION_PRIVATE_NEW_FOLDER      QObject::tr("Create new folder")
#define WIZACTION_PRIVATE_RENAME_FOLDER   QObject::tr("Rename current folder")
#define WIZACTION_PRIVATE_DELETE_FOLDER   QObject::tr("Delete current folder")

CWizCategoryView::CWizCategoryView(CWizExplorerApp& app, QWidget* parent)
    : CWizCategoryBaseView(app, parent)
    , m_menuAllFolders(NULL)
    , m_menuFolder(NULL)
{
    qRegisterMetaType<WIZDOCUMENTDATA>("WIZDOCUMENTDATA");

    connect(&m_dbMgr.db(), SIGNAL(documentCreated(const WIZDOCUMENTDATA&)), \
            SLOT(on_document_created(const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr.db(), SIGNAL(documentModified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)), \
            SLOT(on_document_modified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr.db(), SIGNAL(folderCreated(const CString&)), \
            SLOT(on_folder_created(const CString&)));

    connect(&m_dbMgr.db(), SIGNAL(folderDeleted(const CString&)), \
            SLOT(on_folder_deleted(const CString&)));

    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);

    // setup context menu
    m_menuAllFolders = new QMenu(this);
    m_menuAllFolders->addAction(WIZACTION_PRIVATE_NEW_FOLDER, this, SLOT(on_action_newFolder()));

    m_menuFolder = new QMenu(this);
    m_menuFolder->addAction(WIZACTION_PRIVATE_NEW_DOCUMENT, this, SLOT(on_action_newDocument()));
    m_menuFolder->addAction(WIZACTION_PRIVATE_NEW_FOLDER, this, SLOT(on_action_newFolder()));
    m_menuFolder->addSeparator();
    m_menuFolder->addAction(WIZACTION_PRIVATE_RENAME_FOLDER, this, SLOT(on_action_renameFolder()));
    m_menuFolder->addAction(WIZACTION_PRIVATE_DELETE_FOLDER, this, SLOT(on_action_deleteFolder()));


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
    CWizCategoryViewAllFoldersItem* pAllFoldersItem = new CWizCategoryViewAllFoldersItem(m_app, tr("Note Folders"), m_dbMgr.db().kbGUID());
    addTopLevelItem(pAllFoldersItem);

    CWizStdStringArray arrayAllLocation;
    m_dbMgr.db().GetAllLocations(arrayAllLocation);

    doLocationSanityCheck(arrayAllLocation);

    initFolders(pAllFoldersItem, QString(), arrayAllLocation);

    if (arrayAllLocation.empty()) {
        const QString strNotes("/My Notes/");
        m_dbMgr.db().AddExtraFolder(strNotes);
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

    if (m_dbMgr.db().IsInDeletedItems(strLocation)) {
        return findTrash(m_dbMgr.db().kbGUID());
    }

    CString strCurrentLocation = "/";
    QTreeWidgetItem* parent = pAllFolders;
    //
    CString strTempLocation = strLocation;
    strTempLocation.Trim('/');
    QStringList sl = strTempLocation.split("/");

    QStringList::const_iterator it;
    for (it = sl.begin(); it != sl.end(); it++) {
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
        CWizCategoryViewFolderItem* pFolderItem = new CWizCategoryViewFolderItem(m_app, strCurrentLocation, m_dbMgr.db().kbGUID());
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
    if (m_dbMgr.db().IsInDeletedItems(document.strLocation))
        return;

    addFolder(document.strLocation, true);
}

void CWizCategoryView::on_document_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew)
{
    Q_UNUSED(documentOld);

    if (m_dbMgr.db().IsInDeletedItems(documentNew.strLocation))
        return;

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

void CWizCategoryView::on_action_newDocument()
{
    Q_EMIT newDocument();
}

void CWizCategoryView::on_action_newFolder()
{
    if (!m_MsgNewFolder) {
        m_MsgNewFolder = new CWizNewDialog(tr("Please input folder name: "), this);
        connect(m_MsgNewFolder, SIGNAL(finished(int)), SLOT(on_action_newFolder_confirmed(int)));
    }

    m_MsgNewFolder->clear();
    m_MsgNewFolder->open();
}

void CWizCategoryView::on_action_newFolder_confirmed(int result)
{
    if (result != QDialog::Accepted) {
        return;
    }

    CString strFolderName = m_MsgNewFolder->input();
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
}

void CWizCategoryView::on_action_renameFolder()
{
    // not allowed change predefined location name
    //if (CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>()) {
    //    if (::WizIsPredefinedLocation(p->location())) {
    //        return;
    //    }
    //}

    if (!m_MsgRenameFolder) {
        m_MsgRenameFolder = new CWizNewDialog(tr("Please input new folder name: "), this);
        connect(m_MsgRenameFolder, SIGNAL(finished(int)), SLOT(on_action_renameFolder_confirmed(int)));
    }

    m_MsgRenameFolder->clear();
    m_MsgRenameFolder->open();
}

void CWizCategoryView::on_action_renameFolder_confirmed(int result)
{
    if (result != QDialog::Accepted) {
        return;
    }

    CString strFolderName = m_MsgRenameFolder->input();
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
        folder.MoveToLocation(strLocation);
        addAndSelectFolder(strLocation);
        on_folder_deleted(strOldLocation);
    }
}

void CWizCategoryView::on_action_deleteFolder()
{
    CWaitCursor wait;
    Q_UNUSED(wait);

    CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>();
    if (!p)
        return;

    // setup warning messagebox
    if (!m_MsgWarning) {
        m_MsgWarning = new QMessageBox(this);
        m_MsgWarning->setWindowTitle(tr("Delete Folder"));
        m_MsgWarning->addButton(QMessageBox::Ok);
        m_MsgWarning->addButton(QMessageBox::Cancel);
        m_MsgWarning->setWindowModality(Qt::ApplicationModal);
    }

    QString strWarning = tr("Do you really want to delete all documents inside folder: ") + p->location();
    m_MsgWarning->setText(strWarning);
    m_MsgWarning->open(this, SLOT(on_action_deleteFolder_confirmed()));
}

void CWizCategoryView::on_action_deleteFolder_confirmed()
{
    CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>();
    if (!p)
        return;

    if (m_MsgWarning->result() == QMessageBox::Ok) {
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

CWizCategoryTagsView::CWizCategoryTagsView(CWizExplorerApp& app, QWidget *parent)
    : CWizCategoryBaseView(app, parent)
    , m_menuAllTags(NULL)
    , m_menuTag(NULL)
{
    //setDragDropMode(QAbstractItemView::DragDrop);

    qRegisterMetaType<WIZTAGDATA>("WIZTAGDATA");

    connect(&m_dbMgr, SIGNAL(tagCreated(const WIZTAGDATA&)), \
            SLOT(on_tag_created(const WIZTAGDATA&)));

    connect(&m_dbMgr, SIGNAL(tagModified(const WIZTAGDATA&, const WIZTAGDATA&)), \
            SLOT(on_tag_modified(const WIZTAGDATA&, const WIZTAGDATA&)));

    connect(&m_dbMgr, SIGNAL(tagDeleted(const WIZTAGDATA&)), \
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

void CWizCategoryTagsView::on_tag_created(const WIZTAGDATA& tag)
{
    addTagWithChildren(tag);
}

void CWizCategoryTagsView::on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew)
{
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

void CWizCategoryTagsView::on_tag_deleted(const WIZTAGDATA& tag)
{
    removeTag(tag);
}

void CWizCategoryTagsView::on_action_newTag()
{
    if (!m_MsgNewTag) {
        m_MsgNewTag = new CWizNewDialog(tr("Please input tag name: "), this);
        connect(m_MsgNewTag, SIGNAL(finished(int)), SLOT(on_action_newTag_confirmed(int)));
    }

    m_MsgNewTag->clear();
    m_MsgNewTag->open();
}

void CWizCategoryTagsView::on_action_newTag_confirmed(int result)
{
    if (result != QDialog::Accepted) {
        return;
    }

    CString strTagNames = m_MsgNewTag->input();
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

        // only create tag for unique name
        if (m_dbMgr.db().TagByName(strTagName, tagNew)) {
            TOLOG1("Tag name already exist: %1", strTagName);
            continue;
        }

        m_dbMgr.db().CreateTag(parentTag.strGUID, strTagName, "", tagNew);
    }
}

void CWizCategoryTagsView::on_action_deleteTag()
{
    CWaitCursor wait;
    Q_UNUSED(wait);

    if (CWizCategoryViewTagItem* p = currentCategoryItem<CWizCategoryViewTagItem>()) {
        WIZTAGDATA tag = p->tag();
        m_dbMgr.db().DeleteTagWithChildren(tag, TRUE);
    }
}


/* ------------------------------ CWizCategoryGroupsView ------------------------------ */

#define WIZACTION_GROUP_MARK_READ       QObject::tr("Mark all documents read")
#define WIZACTION_GROUP_NEW_DOCUMENT    QObject::tr("Create new document")
#define WIZACTION_GROUP_NEW_FOLDER      QObject::tr("Create new folder")
#define WIZACTION_GROUP_OPEN_ATTRIBUTE  QObject::tr("Open group attribute")
#define WIZACTION_GROUP_RENAME_FOLDER   QObject::tr("Change current folder name")
#define WIZACTION_GROUP_DELETE_FOLDER   QObject::tr("Delete current folder")

CWizCategoryGroupsView::CWizCategoryGroupsView(CWizExplorerApp& app, QWidget* parent)
    : CWizCategoryBaseView(app, parent)
{
    qRegisterMetaType<WIZTAGDATA>("WIZTAGDATA");

    connect(&m_dbMgr, SIGNAL(databaseOpened(const QString&)), \
            SLOT(on_group_opened(const QString&)));

    connect(&m_dbMgr, SIGNAL(databaseClosed(const QString&)), \
            SLOT(on_group_closed(const QString&)));

    connect(&m_dbMgr, SIGNAL(databaseRename(const QString&)), \
            SLOT(on_group_rename(const QString&)));

    connect(&m_dbMgr, SIGNAL(databasePermissionChanged(const QString&)), \
            SLOT(on_group_permissionChanged(const QString&)));

    connect(&m_dbMgr, SIGNAL(tagCreated(const WIZTAGDATA&)), \
            SLOT(on_tag_created(const WIZTAGDATA&)));

    connect(&m_dbMgr, SIGNAL(tagModified(const WIZTAGDATA&, const WIZTAGDATA&)), \
            SLOT(on_tag_modified(const WIZTAGDATA&, const WIZTAGDATA&)));

    connect(&m_dbMgr, SIGNAL(tagDeleted(const WIZTAGDATA&)), \
            SLOT(on_tag_deleted(const WIZTAGDATA&)));

    m_menuGroupRoot = new QMenu(this);
    //QAction* actionMarkRead = m_menuGroupRoot->addAction(WIZACTION_GROUP_MARK_READ, this, SLOT(on_action_markRead()));
    m_menuGroupRoot->addSeparator();
    QAction* actionNewDoc = m_menuGroupRoot->addAction(WIZACTION_GROUP_NEW_DOCUMENT, this, SLOT(on_action_newDocument()));
    QAction* actionNewFolder = m_menuGroupRoot->addAction(WIZACTION_GROUP_NEW_FOLDER, this, SLOT(on_action_newTag()));
    m_menuGroupRoot->addSeparator();
    m_menuGroupRoot->addAction(WIZACTION_GROUP_OPEN_ATTRIBUTE, this, SLOT(on_action_openGroupAttribute()));

    m_menuGroup = new QMenu(this);
    //m_menuGroup->addAction(actionMarkRead);
    m_menuGroup->addSeparator();
    m_menuGroup->addAction(actionNewDoc);
    m_menuGroup->addAction(actionNewFolder);
    m_menuGroup->addSeparator();
    //m_menuGroup->addAction(WIZACTION_GROUP_RENAME_FOLDER, this, SLOT(on_action_modifyTag()));
    m_menuGroup->addAction(WIZACTION_GROUP_DELETE_FOLDER, this, SLOT(on_action_deleteTag()));
}

void CWizCategoryGroupsView::showGroupRootContextMenu(const QString& strKbGUID, QPoint pos)
{
    m_strKbGUID = strKbGUID;
    on_group_permissionChanged(strKbGUID);
    m_menuGroupRoot->popup(pos);
}

void CWizCategoryGroupsView::showGroupContextMenu(const QString& strKbGUID, QPoint pos)
{
    m_strKbGUID = strKbGUID;
    on_group_permissionChanged(strKbGUID);
    m_menuGroup->popup(pos);
}

void CWizCategoryGroupsView::init()
{
    CWizCategoryViewAllGroupsRootItem* pAllGroupsItem = new CWizCategoryViewAllGroupsRootItem(m_app, tr("Groups"), "");
    addTopLevelItem(pAllGroupsItem);
    pAllGroupsItem->setExpanded(true);

    int nTotal = m_dbMgr.count();
    for (int i = 0; i < nTotal; i++) {
        initGroup(m_dbMgr.at(i));
    }
}

void CWizCategoryGroupsView::initGroup(CWizDatabase& db)
{
    QTreeWidgetItem* pAllGroupsItem = topLevelItem(0);

    CWizCategoryViewGroupRootItem* pGroupItem = new CWizCategoryViewGroupRootItem(m_app, db.name(), db.kbGUID());

    pAllGroupsItem->addChild(pGroupItem);
    initGroup(db, pGroupItem, "");

    pAllGroupsItem->sortChildren(0, Qt::AscendingOrder);

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

CWizCategoryViewGroupRootItem* CWizCategoryGroupsView::findGroup(const QString& strKbGUID)
{
    QTreeWidgetItem* pTop = topLevelItem(0);
    int nCount = pTop->childCount();
    for (int i = 0; i < nCount; i++) {
        CWizCategoryViewGroupRootItem* pItem = dynamic_cast<CWizCategoryViewGroupRootItem *>(pTop->child(i));
        if (pItem && (pItem->kbGUID() == strKbGUID)) {
            return pItem;
        }
    }

    return NULL;
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

        CWizCategoryViewGroupItem* pTagItem = new CWizCategoryViewGroupItem(m_app, tagParent, tag.strKbGUID);
        parent->addChild(pTagItem);
        parent->setExpanded(true);
        parent = pTagItem;

        if (sort) {
            parent->sortChildren(0, Qt::AscendingOrder);
        }
    }

    return dynamic_cast<CWizCategoryViewGroupItem *>(parent);
}

void CWizCategoryGroupsView::on_group_opened(const QString& strKbGUID)
{
    initGroup(m_dbMgr.db(strKbGUID));
}

void CWizCategoryGroupsView::on_group_closed(const QString& strKbGUID)
{
    CWizCategoryViewGroupRootItem* pItem = findGroup(strKbGUID);
    if (pItem) {
        QTreeWidgetItem* parent = pItem->parent();
        if (parent) {
            parent->removeChild(pItem);
        }
    }
}

void CWizCategoryGroupsView::on_group_rename(const QString& strKbGUID)
{
    CWizCategoryViewGroupRootItem* pItem = findGroup(strKbGUID);
    if (pItem) {
        pItem->reload(m_dbMgr.db(strKbGUID));
    }
}

void CWizCategoryGroupsView::on_group_permissionChanged(const QString& strKbGUID)
{
    int nPerm = m_dbMgr.db(strKbGUID).permission();

    // only Admin and Super user see trash folder and operate with tag (folder)
    if (nPerm > WIZ_USERGROUP_SUPER) {
        findTrash(strKbGUID)->setHidden(true);
        findAction(WIZACTION_GROUP_NEW_FOLDER)->setEnabled(false);
        //findAction(WIZACTION_GROUP_RENAME_FOLDER)->setEnabled(false);
        findAction(WIZACTION_GROUP_DELETE_FOLDER)->setEnabled(false);
    } else {
        findTrash(strKbGUID)->setHidden(false);
        findAction(WIZACTION_GROUP_NEW_FOLDER)->setEnabled(true);
        //findAction(WIZACTION_GROUP_RENAME_FOLDER)->setEnabled(true);
        findAction(WIZACTION_GROUP_DELETE_FOLDER)->setEnabled(true);
    }

    // permission greater than author can create new document
    if (nPerm >= WIZ_USERGROUP_READER) {
        findAction(WIZACTION_GROUP_NEW_DOCUMENT)->setEnabled(false);
    } else {
        findAction(WIZACTION_GROUP_NEW_DOCUMENT)->setEnabled(true);
    }
}

void CWizCategoryGroupsView::on_tag_created(const WIZTAGDATA& tag)
{
    // ignore signal from private database
    if (tag.strKbGUID == m_dbMgr.db().kbGUID())
        return;

    addTagWithChildren(tag);
}

void CWizCategoryGroupsView::on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew)
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

void CWizCategoryGroupsView::on_tag_deleted(const WIZTAGDATA& tag)
{
    // ignore signal from private database
    if (tag.strKbGUID == m_dbMgr.db().kbGUID())
        return;

    removeTag(tag);
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
    if (!m_MsgNewFolder) {
        m_MsgNewFolder = new CWizNewDialog(tr("Please input folder name: "), this);
        connect(m_MsgNewFolder, SIGNAL(finished(int)), SLOT(on_action_newTag_confirmed(int)));
    }

    m_MsgNewFolder->clear();
    m_MsgNewFolder->open();
}

void CWizCategoryGroupsView::on_action_newTag_confirmed(int result)
{
    if (result != QDialog::Accepted) {
        return;
    }

    QString strTagNames = m_MsgNewFolder->input();
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

void CWizCategoryGroupsView::on_action_modifyTag()
{

}

void CWizCategoryGroupsView::on_action_deleteTag()
{
    CWaitCursor wait;
    Q_UNUSED(wait);

    if (CWizCategoryViewGroupItem* p = currentCategoryItem<CWizCategoryViewGroupItem>()) {
        WIZTAGDATA tag = p->tag();
        m_dbMgr.db(p->kbGUID()).DeleteTagWithChildren(tag, true);
    }
}
