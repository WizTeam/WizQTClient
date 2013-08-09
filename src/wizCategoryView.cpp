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

#define CATEGORY_GENERAL    QObject::tr("General")
#define CATEGORY_PERSONAL   QObject::tr("Personal notes")
#define CATEGORY_ENTERPRISE QObject::tr("Enterprise groups")
#define CATEGORY_INDIVIDUAL QObject::tr("Individual groups")
#define CATEGORY_MESSAGES   QObject::tr("Message Center")
#define CATEGORY_SHORTCUTS  QObject::tr("Shortcuts")
#define CATEGORY_SEARCH     QObject::tr("Quick search")
#define CATEGORY_STYLES     QObject::tr("Styles")
#define CATEGORY_GROUP      QObject::tr("My groups")

// for context menu text
#define CATEGORY_ACTION_DOCUMENT_NEW    QObject::tr("Create new document")
#define CATEGORY_ACTION_FOLDER_NEW      QObject::tr("Create new folder")
#define CATEGORY_ACTION_FOLDER_MOVE     QObject::tr("Move current folder")
#define CATEGORY_ACTION_FOLDER_RENAME   QObject::tr("Change current folder name")
#define CATEGORY_ACTION_FOLDER_DELETE   QObject::tr("Delete current folder")
#define CATEGORY_ACTION_TAG_NEW         QObject::tr("Create new tag")
#define CATEGORY_ACTION_TAG_RENAME      QObject::tr("Change current tag name")
#define CATEGORY_ACTION_TAG_DELETE      QObject::tr("Delete current tag")
#define CATEGORY_ACTION_GROUP_ATTRIBUTE QObject::tr("Open group attribute")
#define CATEGORY_ACTION_GROUP_MARK_READ QObject::tr("Mark all documents read")

// action user data
enum CategoryActions
{
    ActionNewDocument,
    ActionNewItem,
    ActionMoveItem,
    ActionRenameItem,
    ActionDeleteItem,
    ActionItemAttribute,
    ActionEmptyTrash
};


/* ------------------------------ CWizCategoryBaseView ------------------------------ */

CWizCategoryBaseView::CWizCategoryBaseView(CWizExplorerApp& app, QWidget* parent)
    : QTreeWidget(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_bDragHovered(false)
    , m_selectedItem(NULL)
{
    // basic features
    header()->hide();
    setAnimated(true);
    setFrameStyle(QFrame::NoFrame);
    viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setAutoFillBackground(true);
    setTextElideMode(Qt::ElideMiddle);
    setIndentation(12);

    // use custom scrollbar
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_vScroll = new CWizScrollBar(this);
    m_vScroll->syncWith(verticalScrollBar());

    // style
    setStyle(::WizGetStyle(m_app.userSettings().skin()));
    QColor colorBg = WizGetCategoryBackroundColor(m_app.userSettings().skin());
    QPalette pal = palette();
    pal.setBrush(QPalette::Base, colorBg);
    setPalette(pal);

    // signals from database
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

void CWizCategoryBaseView::resizeEvent(QResizeEvent* event)
{
    // reset scrollbar
    m_vScroll->resize(m_vScroll->sizeHint().width(), event->size().height());
    m_vScroll->move(event->size().width() - m_vScroll->sizeHint().width(), 0);

    QTreeWidget::resizeEvent(event);
}

void CWizCategoryBaseView::contextMenuEvent(QContextMenuEvent * e)
{
    CWizCategoryViewItemBase* pItem = dynamic_cast<CWizCategoryViewItemBase*>(itemAt(e->pos()));
    if (pItem) {
        pItem->showContextMenu(this, mapToGlobal(e->pos()));
    }
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

int CWizCategoryBaseView::findCategoryIndex(CategoryType type)
{
    QString strTypeName;
    switch (type)
    {
    case General:
        strTypeName = CATEGORY_GENERAL;
        break;
    case PersonalNotes:
        strTypeName = CATEGORY_PERSONAL;
        break;
    case EnterpriseGroups:
        strTypeName = CATEGORY_ENTERPRISE;
        break;
    case IndividualGroups:
        strTypeName = CATEGORY_INDIVIDUAL;
        break;
    default:
        Q_ASSERT(0);
    };

    for (int i = 0; i < topLevelItemCount(); i++) {
        CWizCategoryViewCategoryItem* pItem = dynamic_cast<CWizCategoryViewCategoryItem *>(topLevelItem(i));
        if (pItem && pItem->name() == strTypeName) {
          return i;
        }
    }

    Q_ASSERT(0);
    return 0;
}

void CWizCategoryBaseView::baseInit()
{
    init();
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

bool CWizCategoryBaseView::isHelperItemByIndex(const QModelIndex &index) const
{
    CWizCategoryViewItemBase* pItem = categoryItemFromIndex(index);
    if (NULL != dynamic_cast<const CWizCategoryViewSeparatorItem*>(pItem)) {
        return true;
    } else if (NULL != dynamic_cast<const CWizCategoryViewSpacerItem*>(pItem)) {
        return true;
    } else if (NULL != dynamic_cast<const CWizCategoryViewCategoryItem*>(pItem)) {
        return true;
    }
    return false;
}

bool CWizCategoryBaseView::isSeparatorItemByPosition(const QPoint& pt) const
{
    const QTreeWidgetItem* pItem = itemAt(pt);
    return NULL != dynamic_cast<const CWizCategoryViewSeparatorItem*>(pItem);
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

void CWizCategoryBaseView::on_document_created(const WIZDOCUMENTDATA& document)
{
    onDocument_created(document);
    //updateDocumentCount(document.strKbGUID);
}

void CWizCategoryBaseView::on_document_modified(const WIZDOCUMENTDATA& documentOld,
                                                const WIZDOCUMENTDATA& documentNew)
{
    onDocument_modified(documentOld, documentNew);
    //updateDocumentCount(documentNew.strKbGUID);
}

void CWizCategoryBaseView::on_document_deleted(const WIZDOCUMENTDATA& document)
{
    onDocument_deleted(document);
    //updateDocumentCount(document.strKbGUID);
}

void CWizCategoryBaseView::on_document_tag_modified(const WIZDOCUMENTDATA& document)
{
    //updateDocumentCount(document.strKbGUID);
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
CWizCategoryView::CWizCategoryView(CWizExplorerApp& app, QWidget* parent)
    : CWizCategoryBaseView(app, parent)
{
    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);

    initMenus();

    connect(this, SIGNAL(itemSelectionChanged()), SLOT(on_itemSelectionChanged()));
}

void CWizCategoryView::initMenus()
{
    QAction* actionNewDoc = new QAction("ActionNewDocument", this);
    actionNewDoc->setShortcutContext(Qt::WidgetShortcut);
    actionNewDoc->setShortcut(QKeySequence::New);
    actionNewDoc->setData(ActionNewDocument);
    addAction(actionNewDoc);
    connect(actionNewDoc, SIGNAL(triggered()), SLOT(on_action_newDocument()));

    QAction* actionNewItem = new QAction("ActionNewItem", this);
    actionNewItem->setShortcutContext(Qt::WidgetShortcut);
    actionNewItem->setShortcut(QKeySequence("Ctrl+Shift+N"));
    actionNewItem->setData(ActionNewItem);
    addAction(actionNewItem);
    connect(actionNewItem, SIGNAL(triggered()), SLOT(on_action_newItem()));

    QAction* actionMoveItem = new QAction("ActionMoveItem", this);
    actionMoveItem->setShortcutContext(Qt::WidgetShortcut);
    actionMoveItem->setShortcut(QKeySequence("Ctrl+Shift+M"));
    actionMoveItem->setData(ActionMoveItem);
    addAction(actionMoveItem);
    connect(actionMoveItem, SIGNAL(triggered()), SLOT(on_action_moveItem()));

    QAction* actionRenameItem = new QAction("ActionRenameItem", this);
    actionRenameItem->setShortcutContext(Qt::WidgetShortcut);
    actionRenameItem->setShortcut(QKeySequence(Qt::Key_F2));
    actionRenameItem->setData(ActionRenameItem);
    addAction(actionRenameItem);
    connect(actionRenameItem, SIGNAL(triggered()), SLOT(on_action_renameItem()));

    QAction* actionDeleteItem = new QAction("ActionDeleteItem", this);
    actionDeleteItem->setShortcutContext(Qt::WidgetShortcut);
    actionDeleteItem->setShortcut(QKeySequence::Delete);
    actionDeleteItem->setData(ActionDeleteItem);
    addAction(actionDeleteItem);
    connect(actionDeleteItem, SIGNAL(triggered()), SLOT(on_action_deleteItem()));

    QAction* actionItemAttr = new QAction("ActionItemAttribute", this);
    actionItemAttr->setShortcutContext(Qt::WidgetShortcut);
    actionItemAttr->setData(ActionItemAttribute);
    addAction(actionItemAttr);
    connect(actionItemAttr, SIGNAL(triggered()), SLOT(on_action_itemAttribute()));

    QAction* actionTrash = new QAction("ActionEmptyTrash", this);
    actionTrash->setShortcutContext(Qt::WidgetShortcut);
    actionDeleteItem->setShortcut(QKeySequence("Ctrl+Shift+Delete"));
    actionTrash->setData(ActionEmptyTrash);
    addAction(actionTrash);
    connect(actionTrash, SIGNAL(triggered()), SLOT(on_action_emptyTrash()));

    // trash menu
    m_menuTrash = new QMenu(this);
    m_menuTrash->addAction(actionTrash);

    // folder root menu
    m_menuFolderRoot = new QMenu(this);
    m_menuFolderRoot->addAction(actionNewItem);

    // folder menu
    m_menuFolder = new QMenu(this);
    m_menuFolder->addAction(actionNewDoc);
    m_menuFolder->addAction(actionNewItem);
    m_menuFolder->addSeparator();
    m_menuFolder->addAction(actionMoveItem);
    m_menuFolder->addAction(actionRenameItem);
    m_menuFolder->addAction(actionDeleteItem);

    // tag root menu
    m_menuTagRoot = new QMenu(this);
    m_menuTagRoot->addAction(actionNewItem);

    // tag menu
    m_menuTag = new QMenu(this);
    m_menuTag->addAction(actionNewItem);
    m_menuTag->addAction(actionRenameItem);
    m_menuTag->addSeparator();
    m_menuTag->addAction(actionDeleteItem);

    // group root menu
    m_menuGroupRoot = new QMenu(this);
    m_menuGroupRoot->addAction(actionNewDoc);
    m_menuGroupRoot->addAction(actionNewItem);
    m_menuGroupRoot->addSeparator();
    m_menuGroupRoot->addAction(actionItemAttr);

    // group menu
    m_menuGroup = new QMenu(this);
    m_menuGroup->addAction(actionNewDoc);
    m_menuGroup->addAction(actionNewItem);
    m_menuGroup->addAction(actionRenameItem);
    m_menuGroup->addSeparator();
    m_menuGroup->addAction(actionDeleteItem);
}

void CWizCategoryView::resetMenu(CategoryMenuType type)
{
    QList<QAction*> acts = actions();

    for (int i = 0; i < acts.size(); i++) {
        QAction* act = acts.at(i);
        switch (act->data().toInt()) {
        case ActionNewDocument:
            if (type == FolderItem || type == GroupRootItem || type == GroupItem) {
                act->setText(CATEGORY_ACTION_DOCUMENT_NEW);
            }
            break;
        case ActionNewItem:
            if (type == FolderRootItem || type == FolderItem
                    || type == GroupRootItem || type == GroupItem) {
                act->setText(CATEGORY_ACTION_FOLDER_NEW);
            } else if (type == TagRootItem || type == TagItem) {
                    act->setText(CATEGORY_ACTION_TAG_NEW);
            }
            break;
        case ActionMoveItem:
            if (type == FolderItem) {
                act->setText(CATEGORY_ACTION_FOLDER_MOVE);
            }
            break;
        case ActionRenameItem:
            if (type == FolderItem || type == GroupItem) {
                act->setText(CATEGORY_ACTION_FOLDER_RENAME);
            } else if (type == TagItem) {
                act->setText(CATEGORY_ACTION_TAG_RENAME);
            }
            break;
        case ActionDeleteItem:
            if (type == FolderItem || type == GroupItem) {
                act->setText(CATEGORY_ACTION_FOLDER_DELETE);
            } else if (type == TagItem) {
                act->setText(CATEGORY_ACTION_TAG_DELETE);
            }
            break;
        case ActionItemAttribute:
            if (type == GroupRootItem) {
                act->setText(CATEGORY_ACTION_GROUP_ATTRIBUTE);
            }
            break;
        default:
            continue;
        }
    }
}

void CWizCategoryView::showTrashContextMenu(QPoint pos)
{
    m_menuTrash->popup(pos);
}

void CWizCategoryView::showFolderRootContextMenu(QPoint pos)
{
    resetMenu(FolderRootItem);
    m_menuFolderRoot->popup(pos);
}

void CWizCategoryView::showFolderContextMenu(QPoint pos)
{
    resetMenu(FolderItem);
    m_menuFolder->popup(pos);
}

void CWizCategoryView::showTagRootContextMenu(QPoint pos)
{
    resetMenu(TagRootItem);
    m_menuTagRoot->popup(pos);
}

void CWizCategoryView::showTagContextMenu(QPoint pos)
{
    resetMenu(TagItem);
    m_menuTag->popup(pos);
}

void CWizCategoryView::showGroupRootContextMenu(QPoint pos)
{
    resetMenu(GroupRootItem);
    m_menuGroupRoot->popup(pos);
}

void CWizCategoryView::showGroupContextMenu(QPoint pos)
{
    resetMenu(GroupItem);
    m_menuGroup->popup(pos);
}

void CWizCategoryView::on_action_newDocument()
{
    if (currentCategoryItem<CWizCategoryViewFolderItem>()
            || currentCategoryItem<CWizCategoryViewGroupRootItem>()
            || currentCategoryItem<CWizCategoryViewGroupItem>())
    {
        // delegate create action to mainwindow
        Q_EMIT newDocument();
    }
}

void CWizCategoryView::on_action_newItem()
{
    if (currentCategoryItem<CWizCategoryViewAllFoldersItem>()
            || currentCategoryItem<CWizCategoryViewFolderItem>())
    {
        on_action_user_newFolder();
    }
    else if (currentCategoryItem<CWizCategoryViewAllTagsItem>()
             || currentCategoryItem<CWizCategoryViewTagItem>())
    {
        on_action_user_newTag();
    }
    else if (currentCategoryItem<CWizCategoryViewGroupRootItem>()
             || currentCategoryItem<CWizCategoryViewGroupItem>())
    {
        on_action_group_newFolder();
    }
}

void CWizCategoryView::on_action_user_newFolder()
{
    CWizLineInputDialog* dialog = new CWizLineInputDialog(tr("New folder"),
                                                          tr("Please input folder name: "),
                                                          "", this);

    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_user_newFolder_confirmed(int)));

    dialog->open();
}

void CWizCategoryView::on_action_user_newFolder_confirmed(int result)
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

    if (currentCategoryItem<CWizCategoryViewAllFoldersItem>()) {
        strLocation = "/" + strFolderName + "/";
    } else if (CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>()) {
        strLocation = p->location() + strFolderName + "/";
    }

    addAndSelectFolder(strLocation);
    m_dbMgr.db().AddExtraFolder(strLocation);
    m_dbMgr.db().SetObjectVersion("folder", 0);
}

void CWizCategoryView::on_action_user_newTag()
{
    CWizLineInputDialog* dialog = new CWizLineInputDialog(tr("New tag"),
                                                          tr("Please input tag name: "),
                                                          "", this);
    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_user_newTag_confirmed(int)));

    dialog->open();
}

void CWizCategoryView::on_action_user_newTag_confirmed(int result)
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

void CWizCategoryView::on_action_group_newFolder()
{
    CWizLineInputDialog* dialog = new CWizLineInputDialog(tr("New group folder"),
                                                          tr("Please input folder name: "),
                                                          "", this);

    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_group_newFolder_confirmed(int)));

    dialog->open();
}

void CWizCategoryView::on_action_group_newFolder_confirmed(int result)
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


void CWizCategoryView::on_action_moveItem()
{
    if (currentCategoryItem<CWizCategoryViewFolderItem>()) {
        on_action_user_moveFolder();
    }
}

void CWizCategoryView::on_action_user_moveFolder()
{
    CWizFolderSelector* selector = new CWizFolderSelector(tr("Move folder"), m_app, this);
    selector->setAcceptRoot(true);

    connect(selector, SIGNAL(finished(int)), SLOT(on_action_user_moveFolder_confirmed(int)));
    selector->open();
}

void CWizCategoryView::on_action_user_moveFolder_confirmed(int result)
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

        if (!CWizFolder::CanMove(strOldLocation, strLocation)) {
            return;
        }

        // move all documents to new folder
        CWizFolder folder(m_dbMgr.db(), strOldLocation);
        connect(&folder, SIGNAL(moveDocument(int, int, const QString&, const QString&, const WIZDOCUMENTDATA&)),
                SLOT(on_action_user_moveFolder_confirmed_progress(int, int, const QString&, const QString&, const WIZDOCUMENTDATA&)));

        folder.MoveToLocation(strLocation);

        // hide progress dialog
        MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());
        mainWindow->progressDialog()->hide();

        addAndSelectFolder(strLocation);
        on_folder_deleted(strOldLocation);
    }
}

void CWizCategoryView::on_action_user_moveFolder_confirmed_progress(int nMax, int nValue,
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

void CWizCategoryView::on_action_renameItem()
{
    if (CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>())
    {
        // user can not rename predefined folders name
        if (!::WizIsPredefinedLocation(p->location())) {
            on_action_user_renameFolder();
        }
    }
    else if (currentCategoryItem<CWizCategoryViewTagItem>())
    {
        on_action_user_renameTag();
    }
    else if (currentCategoryItem<CWizCategoryViewGroupItem>())
    {
        on_action_group_renameFolder();
    }
}

void CWizCategoryView::on_action_user_renameFolder()
{
    CWizCategoryViewItemBase* p = currentCategoryItem<CWizCategoryViewItemBase>();

    CWizLineInputDialog* dialog = new CWizLineInputDialog(tr("Rename folder"),
                                                          tr("Please input new folder name: "),
                                                          p->name(), this);
    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_user_renameFolder_confirmed(int)));

    dialog->open();
}

void CWizCategoryView::on_action_user_renameFolder_confirmed(int result)
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
                SLOT(on_action_user_renameFolder_confirmed_progress(int, int, const QString&, const QString&, const WIZDOCUMENTDATA&)));
        folder.MoveToLocation(strLocation);

        // hide progress dialog
        MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());
        mainWindow->progressDialog()->hide();

        addAndSelectFolder(strLocation);
        on_folder_deleted(strOldLocation);
    }
}

void CWizCategoryView::on_action_user_renameFolder_confirmed_progress(int nMax, int nValue,
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

void CWizCategoryView::on_action_user_renameTag()
{
    CWizCategoryViewItemBase* p = currentCategoryItem<CWizCategoryViewItemBase>();

    CWizLineInputDialog* dialog = new CWizLineInputDialog(tr("Rename tag"),
                                                          tr("Please input tag name: "),
                                                          p->name(), this);

    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_user_renameTag_confirmed(int)));

    dialog->open();
}

void CWizCategoryView::on_action_user_renameTag_confirmed(int result)
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

void CWizCategoryView::on_action_group_renameFolder()
{
    CWizCategoryViewItemBase* p = currentCategoryItem<CWizCategoryViewItemBase>();

    CWizLineInputDialog* dialog = new CWizLineInputDialog(tr("Rename group folder"),
                                                          tr("Please input folder name: "),
                                                          p->name(), this);

    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_group_renameFolder_confirmed(int)));

    dialog->open();
}

void CWizCategoryView::on_action_group_renameFolder_confirmed(int result)
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

void CWizCategoryView::on_action_deleteItem()
{
    if (CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>())
    {
        if (!::WizIsPredefinedLocation(p->location())) {
            on_action_user_deleteFolder();
        }
    }
    else if (currentCategoryItem<CWizCategoryViewTagItem>())
    {
        on_action_user_deleteTag();
    }
    else if (currentCategoryItem<CWizCategoryViewGroupItem>())
    {
        on_action_group_deleteFolder();
    }
}

void CWizCategoryView::on_action_user_deleteFolder()
{
    CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>();
    if (!p)
        return;

    // FIXME:
    // 1. only show warning message if folder is not empty
    // 2. show progress windows when delete

    // setup warning messagebox
    QMessageBox* msgBox = new QMessageBox(this);
    msgBox->setWindowTitle(tr("Delete Folder"));
    msgBox->addButton(QMessageBox::Ok);
    msgBox->addButton(QMessageBox::Cancel);

    QString strWarning = tr("Do you really want to delete all documents inside folder: %1 ? (All documents will move to trash folder and remove from cloud server)").arg(p->location());
    msgBox->setText(strWarning);
    msgBox->open(this, SLOT(on_action_user_deleteFolder_confirmed(int)));
}

void CWizCategoryView::on_action_user_deleteFolder_confirmed(int result)
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

void CWizCategoryView::on_action_user_deleteTag()
{
    CWizCategoryViewTagItem* p = currentCategoryItem<CWizCategoryViewTagItem>();
    if (!p)
        return;

    // FIXME : as above

    QMessageBox* msgBox = new QMessageBox(this);
    msgBox->setWindowTitle(tr("Delete tag"));
    msgBox->addButton(QMessageBox::Ok);
    msgBox->addButton(QMessageBox::Cancel);

    QString strWarning = tr("Do you really want to delete tag: %1 ? (include child tags if any)").arg(p->tag().strName);
    msgBox->setText(strWarning);
    msgBox->open(this, SLOT(on_action_user_deleteTag_confirmed(int)));
}

void CWizCategoryView::on_action_user_deleteTag_confirmed(int result)
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

void CWizCategoryView::on_action_group_deleteFolder()
{
    CWizCategoryViewGroupItem* p = currentCategoryItem<CWizCategoryViewGroupItem>();
    if (!p)
        return;

    // FIXME: as above

    QMessageBox* msgBox = new QMessageBox(this);
    msgBox->setWindowTitle(tr("Delete group folder"));
    msgBox->addButton(QMessageBox::Ok);
    msgBox->addButton(QMessageBox::Cancel);

    QString strWarning = tr("Do you really want to delete folder: %1 ? (All documents will move to root folder, It's safe.)").arg(p->tag().strName);
    msgBox->setText(strWarning);
    msgBox->open(this, SLOT(on_action_group_deleteFolder_confirmed(int)));
}

void CWizCategoryView::on_action_group_deleteFolder_confirmed(int result)
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

void CWizCategoryView::on_action_itemAttribute()
{
    if (currentCategoryItem<CWizCategoryViewGroupRootItem>()
            || currentCategoryItem<CWizCategoryViewGroupItem>())
    {
        on_action_group_attribute();
    }
}

void CWizCategoryView::on_action_group_attribute()
{
    CWizCategoryViewItemBase* p = currentCategoryItem<CWizCategoryViewItemBase>();
    if (p && !p->kbGUID().isEmpty()) {
        MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
        mainWindow->groupAttributeForm()->sheetShow(p->kbGUID());
    }
}

void CWizCategoryView::on_action_emptyTrash()
{
    // FIXME: show progress

    if (CWizCategoryViewTrashItem* pTrashItem = currentCategoryItem<CWizCategoryViewTrashItem>()) {
        QString strKbGUID = pTrashItem->kbGUID();
        Q_ASSERT(!strKbGUID.isEmpty());

        CWizDocumentDataArray arrayDocument;
        pTrashItem->getDocuments(m_dbMgr.db(strKbGUID), arrayDocument);

        foreach (const WIZDOCUMENTDATA& data, arrayDocument)
        {
            CWizDocument doc(m_dbMgr.db(strKbGUID), data);
            doc.Delete();
        }
    }
}

void CWizCategoryView::on_itemSelectionChanged()
{
    CWizCategoryViewGroupRootItem* pRoot = currentCategoryItem<CWizCategoryViewGroupRootItem>();
    CWizCategoryViewGroupItem* pItem = currentCategoryItem<CWizCategoryViewGroupItem>();
    if (pRoot || pItem) {
        CWizCategoryViewItemBase* p = currentCategoryItem<CWizCategoryViewItemBase>();
        on_group_permissionChanged(p->kbGUID());
    }
}

void CWizCategoryView::init()
{
    initGeneral();
    initFolders();
    initTags();
    initStyles();
    initGroups();
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

void CWizCategoryView::initGeneral()
{
    CWizCategoryViewCategoryItem* pCategoryItem = new CWizCategoryViewCategoryItem(m_app, CATEGORY_GENERAL);
    addTopLevelItem(pCategoryItem);

    CWizCategoryViewMessageRootItem* pMsgRoot = new CWizCategoryViewMessageRootItem(m_app, CATEGORY_MESSAGES);
    addTopLevelItem(pMsgRoot);

    CWizCategoryViewShortcutRootItem* pShortcutRoot = new CWizCategoryViewShortcutRootItem(m_app, CATEGORY_SHORTCUTS);
    addTopLevelItem(pShortcutRoot);

    CWizCategoryViewSearchRootItem* pSearchRoot = new CWizCategoryViewSearchRootItem(m_app, CATEGORY_SEARCH);
    addTopLevelItem(pSearchRoot);
}

void CWizCategoryView::initFolders()
{
    CWizCategoryViewSpacerItem* pSpacer = new CWizCategoryViewSpacerItem(m_app);
    addTopLevelItem(pSpacer);

    CWizCategoryViewCategoryItem* pCategoryItem = new CWizCategoryViewCategoryItem(m_app, CATEGORY_PERSONAL);
    addTopLevelItem(pCategoryItem);

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

    CWizCategoryViewTrashItem* pTrash = new CWizCategoryViewTrashItem(m_app, tr("Trash"), m_dbMgr.db().kbGUID());
    pAllFoldersItem->addChild(pTrash);
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

void CWizCategoryView::initTags()
{
    CWizCategoryViewAllTagsItem* pAllTagsItem = new CWizCategoryViewAllTagsItem(m_app, tr("Tags"), m_dbMgr.db().kbGUID());
    addTopLevelItem(pAllTagsItem);
    pAllTagsItem->setExpanded(true);

    initTags(pAllTagsItem, "");
}

void CWizCategoryView::initTags(QTreeWidgetItem* pParent, const QString& strParentTagGUID)
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

void CWizCategoryView::initStyles()
{
    CWizCategoryViewStyleRootItem* pStyleRoot = new CWizCategoryViewStyleRootItem(m_app, CATEGORY_STYLES);
    addTopLevelItem(pStyleRoot);
}

void CWizCategoryView::initGroups()
{
    QMap<QString, QString> bizInfo;
    m_dbMgr.db().GetBizGroupInfo(bizInfo);

    if (!bizInfo.isEmpty()) {
        CWizCategoryViewSpacerItem* pSpacer = new CWizCategoryViewSpacerItem(m_app);
        addTopLevelItem(pSpacer);

        CWizCategoryViewCategoryItem* pCategoryItem = new CWizCategoryViewCategoryItem(m_app, CATEGORY_ENTERPRISE);
        addTopLevelItem(pCategoryItem);

        QMap<QString, QString>::const_iterator it;
        for (it = bizInfo.begin(); it != bizInfo.end(); it++) {
            CWizCategoryViewBizGroupRootItem* pBizGroupItem = new CWizCategoryViewBizGroupRootItem(m_app, it.value(), "");
            addTopLevelItem(pBizGroupItem);
            pBizGroupItem->setExpanded(true);
        }
    }

    CWizCategoryViewSpacerItem* pSpacer = new CWizCategoryViewSpacerItem(m_app);
    addTopLevelItem(pSpacer);

    CWizCategoryViewCategoryItem* pCategoryItem = new CWizCategoryViewCategoryItem(m_app, CATEGORY_INDIVIDUAL);
    addTopLevelItem(pCategoryItem);

    CWizCategoryViewAllGroupsRootItem* pAllGroupsItem = new CWizCategoryViewAllGroupsRootItem(m_app, CATEGORY_GROUP, "");
    addTopLevelItem(pAllGroupsItem);
    pAllGroupsItem->setExpanded(true);

    int nTotal = m_dbMgr.count();
    for (int i = 0; i < nTotal; i++) {
        initGroup(m_dbMgr.at(i));
    }
}

void CWizCategoryView::initGroup(CWizDatabase& db)
{
    QTreeWidgetItem* pRoot = NULL;

    // if biz info exist, add group to it instead of All groups root
    if (!db.info().bizGUID.isEmpty() && !db.info().bizName.isEmpty()) {
        pRoot = findGroupSet(db.info().bizName, true);
    } else {
        pRoot = findGroupSet(CATEGORY_GROUP, true);
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

void CWizCategoryView::initGroup(CWizDatabase& db, QTreeWidgetItem* pParent, const QString& strParentTagGUID)
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

CWizCategoryViewAllGroupsRootItem* CWizCategoryView::findGroupSet(const QString& strName, bool bCreate)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        CWizCategoryViewAllGroupsRootItem* pItem = dynamic_cast<CWizCategoryViewAllGroupsRootItem *>(topLevelItem(i));
        if (pItem && (pItem->name() == strName)) {
            return pItem;
        }
    }

    CWizCategoryViewAllGroupsRootItem* p = NULL;
    if (bCreate) {
        if (strName == CATEGORY_GROUP) {
            p = new CWizCategoryViewAllGroupsRootItem(m_app, strName, "");
            addTopLevelItem(p);
        } else {
            p = new CWizCategoryViewBizGroupRootItem(m_app, strName, "");
            insertTopLevelItem(findCategoryIndex(EnterpriseGroups)+1, p);
        }

        p->setExpanded(true);
    }

    return p;
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
{
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


/* ------------------------------ CWizCategoryGroupsView ------------------------------ */
CWizCategoryGroupsView::CWizCategoryGroupsView(CWizExplorerApp& app, QWidget* parent)
    : CWizCategoryBaseView(app, parent)
{
}

//void CWizCategoryGroupsView::init()
//{
//    // init biz groups
//    QMap<QString, QString> bizInfo;
//    m_dbMgr.db().getBizGroupInfo(bizInfo);

//    if (!bizInfo.isEmpty()) {
//        CWizCategoryViewCategoryItem* pCategoryItem = new CWizCategoryViewCategoryItem(m_app, CATEGORY_ENTERPRISE);
//        addTopLevelItem(pCategoryItem);

//        QMap<QString, QString>::const_iterator it;
//        for (it = bizInfo.begin(); it != bizInfo.end(); it++) {
//            CWizCategoryViewBizGroupRootItem* pBizGroupItem = new CWizCategoryViewBizGroupRootItem(m_app, it.value(), "");
//            addTopLevelItem(pBizGroupItem);
//            pBizGroupItem->setExpanded(true);
//        }
//    }

//    // draw seperator only if biz group exist
//    if (bizInfo.size()) {
//        addSeparator();
//    }

//    // init personal groups
//    CWizCategoryViewCategoryItem* pCategoryItem = new CWizCategoryViewCategoryItem(m_app, CATEGORY_INDIVIDUAL);
//    addTopLevelItem(pCategoryItem);

//    CWizCategoryViewAllGroupsRootItem* pAllGroupsItem = new CWizCategoryViewAllGroupsRootItem(m_app, WIZ_PERSONAL_GROUP_ROOT_NAME, "");
//    addTopLevelItem(pAllGroupsItem);
//    pAllGroupsItem->setExpanded(true);

//    int nTotal = m_dbMgr.count();
//    for (int i = 0; i < nTotal; i++) {
//        initGroup(m_dbMgr.at(i));
//    }

//    initDocumentCount();
//}

//void CWizCategoryGroupsView::initGroup(CWizDatabase& db)
//{
//    QTreeWidgetItem* pRoot = NULL;

//    // if biz info exist, add group to it instead of All groups root
//    if (!db.info().bizGUID.isEmpty() && !db.info().bizName.isEmpty()) {
//        pRoot = findGroupSet(db.info().bizName, true);
//    } else {
//        pRoot = findGroupSet(WIZ_PERSONAL_GROUP_ROOT_NAME, true);
//    }

//    Q_ASSERT(pRoot);

//    CWizCategoryViewGroupRootItem* pGroupItem = new CWizCategoryViewGroupRootItem(m_app, db.name(), db.kbGUID());
//    pRoot->addChild(pGroupItem);
//    initGroup(db, pGroupItem, "");

//    pRoot->sortChildren(0, Qt::AscendingOrder);

//    CWizCategoryViewTrashItem* pTrashItem = new CWizCategoryViewTrashItem(m_app, tr("Trash"), db.kbGUID());
//    pGroupItem->addChild(pTrashItem);

//    // only show trash if permission is enough
//    if (db.permission() > WIZ_USERGROUP_SUPER) {
//        pTrashItem->setHidden(true);
//    }
//}

//void CWizCategoryGroupsView::initGroup(CWizDatabase& db, QTreeWidgetItem* pParent, const QString& strParentTagGUID)
//{
//    CWizTagDataArray arrayTag;
//    db.GetChildTags(strParentTagGUID, arrayTag);

//    CWizTagDataArray::const_iterator it;
//    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
//        CWizCategoryViewGroupItem* pTagItem = new CWizCategoryViewGroupItem(m_app, *it, db.kbGUID());
//        pParent->addChild(pTagItem);

//        initGroup(db, pTagItem, it->strGUID);
//    }
//}

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
    //initGroup(m_dbMgr.db(strKbGUID));
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
        findAction(CATEGORY_ACTION_FOLDER_NEW)->setEnabled(false);
        findAction(CATEGORY_ACTION_FOLDER_RENAME)->setEnabled(false);
        findAction(CATEGORY_ACTION_FOLDER_DELETE)->setEnabled(false);
    } else {
        findTrash(strKbGUID)->setHidden(false);
        findAction(CATEGORY_ACTION_FOLDER_NEW)->setEnabled(true);
        findAction(CATEGORY_ACTION_FOLDER_RENAME)->setEnabled(true);
        findAction(CATEGORY_ACTION_FOLDER_DELETE)->setEnabled(true);
    }

    // permission greater than author can create new document
    if (nPerm >= WIZ_USERGROUP_READER) {
        findAction(CATEGORY_ACTION_DOCUMENT_NEW)->setEnabled(false);
    } else {
        findAction(CATEGORY_ACTION_DOCUMENT_NEW)->setEnabled(true);
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
