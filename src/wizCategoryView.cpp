#include "wizCategoryView.h"

#include <QHeaderView>
#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include <extensionsystem/pluginmanager.h>

#include "wizdef.h"
#include "widgets/wizScrollBar.h"
#include "wizmainwindow.h"
#include "wizProgressDialog.h"
#include "share/wizdrawtexthelper.h"
#include "wiznotestyle.h"
#include "share/wizsettings.h"
#include "share/wizDatabaseManager.h"
#include "wizFolderSelector.h"
#include "wizLineInputDialog.h"
#include "wizWebSettingsDialog.h"
#include "sync/wizkmxmlrpc.h"
#include "sync/apientry.h"

#include "sync/token.h"

using namespace WizService;

using namespace Core::Internal;


#define CATEGORY_GENERAL    QObject::tr("General")
#define CATEGORY_PERSONAL   QObject::tr("Personal notes")
#define CATEGORY_TEAM_GROUPS QObject::tr("Team & Groups")
//#define CATEGORY_ENTERPRISE QObject::tr("Enterprise groups")
#define CATEGORY_INDIVIDUAL QObject::tr("Individual groups")
#define CATEGORY_SHORTCUTS  QObject::tr("Shortcuts")
#define CATEGORY_SEARCH     QObject::tr("Quick search")
#define CATEGORY_FOLDERS    QObject::tr("Note folders")
#define CATEGORY_TAGS       QObject::tr("Tags")
#define CATEGORY_STYLES     QObject::tr("Styles")


// for context menu text
#define CATEGORY_ACTION_DOCUMENT_NEW    QObject::tr("New note")
#define CATEGORY_ACTION_FOLDER_NEW      QObject::tr("New folder...")
#define CATEGORY_ACTION_FOLDER_MOVE     QObject::tr("Move to...")
#define CATEGORY_ACTION_FOLDER_RENAME   QObject::tr("Rename...")
#define CATEGORY_ACTION_FOLDER_DELETE   QObject::tr("Delete")
#define CATEGORY_ACTION_TAG_NEW         QObject::tr("New tag...")
#define CATEGORY_ACTION_TAG_RENAME      QObject::tr("Rename...")
#define CATEGORY_ACTION_TAG_DELETE      QObject::tr("Delete")
#define CATEGORY_ACTION_GROUP_ATTRIBUTE QObject::tr("View group info...")
#define CATEGORY_ACTION_BIZ_GROUP_ATTRIBUTE QObject::tr("View team info...")
#define CATEGORY_ACTION_GROUP_MARK_READ QObject::tr("Mark all read")
#define CATEGORY_ACTION_EMPTY_TRASH     QObject::tr("Empty deleted items")
#define CATEGORY_ACTION_MANAGE_GROUP     QObject::tr("Manage group...")
#define CATEGORY_ACTION_MANAGE_BIZ     QObject::tr("Manage team...")
#define CATEGORY_ACTION_QUIT_GROUP     QObject::tr("Quit group")


#define LINK_COMMAND_ID_CREATE_GROUP        100


/* ------------------------------ CWizCategoryBaseView ------------------------------ */

CWizCategoryBaseView::CWizCategoryBaseView(CWizExplorerApp& app, QWidget* parent)
    : ITreeView(parent)
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

    // scrollbar
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
#ifdef Q_OS_MAC
    verticalScrollBar()->setSingleStep(10);
#else
    verticalScrollBar()->setSingleStep(30);
#endif

#ifdef WIZNOTE_CUSTOM_SCROLLBAR
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_vScroll = new CWizScrollBar(this);
    m_vScroll->syncWith(verticalScrollBar());
#endif

    // style
    setStyle(::WizGetStyle(m_app.userSettings().skin()));
    QColor colorBg = WizGetCategoryBackroundColor(m_app.userSettings().skin());
    QPalette pal = palette();
    pal.setBrush(QPalette::Base, colorBg);
    setPalette(pal);
    //
    setCursor(QCursor(Qt::ArrowCursor));

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

    connect(&m_dbMgr, SIGNAL(folderPositionChanged()),
            SLOT(on_folder_positionChanged()));

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
            SLOT(on_group_renamed(const QString&)));

    connect(&m_dbMgr, SIGNAL(databasePermissionChanged(const QString&)),
            SLOT(on_group_permissionChanged(const QString&)));

    connect(&m_dbMgr, SIGNAL(databaseBizchanged(const QString&)),
            SLOT(on_group_bizChanged(const QString&)));
}

void CWizCategoryBaseView::mousePressEvent(QMouseEvent* event)
{
    // saved for child item which need test hit position.
    m_hitPos = event->pos();

    QTreeWidget::mousePressEvent(event);
}

void CWizCategoryBaseView::resizeEvent(QResizeEvent* event)
{
#ifdef WIZNOTE_CUSTOM_SCROLLBAR
    // reset scrollbar
    m_vScroll->resize(m_vScroll->sizeHint().width(), event->size().height());
    m_vScroll->move(event->size().width() - m_vScroll->sizeHint().width(), 0);
#endif

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

void mime2Note(const QByteArray& bMime, CWizDocumentDataArray& arrayDocument)
{
    QString strMime(QString::fromUtf8(bMime));
    QStringList lsNotes = strMime.split(";");
    for (int i = 0; i < lsNotes.size(); i++) {
        QStringList lsMeta = lsNotes[i].split(":");
        Q_ASSERT(lsMeta.size() == 2);

        CWizDatabase& db = CWizDatabaseManager::instance()->db(lsMeta[0]);

        WIZDOCUMENTDATA data;
        if (db.DocumentFromGUID(lsMeta[1], data))
            arrayDocument.push_back(data);
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

    if (!event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS))
        return;

    CWizCategoryViewItemBase* pItem = itemAt(event->pos());
    if (!pItem)
        return;

    CWizDocumentDataArray arrayDocument;
    mime2Note(event->mimeData()->data(WIZNOTE_MIMEFORMAT_DOCUMENTS), arrayDocument);

    if (!arrayDocument.size())
        return;

    int nAccept = 0;
    for (CWizDocumentDataArray::const_iterator it = arrayDocument.begin();
         it != arrayDocument.end();
         it++)
    {
        if (pItem->acceptDrop(*it))
            nAccept++;
    }

    if (nAccept == arrayDocument.size())
        event->acceptProposedAction();
    else
        event->ignore();
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

    if (!event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS))
        return;

    CWizDocumentDataArray arrayDocument;
    mime2Note(event->mimeData()->data(WIZNOTE_MIMEFORMAT_DOCUMENTS), arrayDocument);

    if (!arrayDocument.size())
        return;

    CWizCategoryViewItemBase* pItem = itemAt(event->pos());
    if (!pItem)
        return;

    for (CWizDocumentDataArray::const_iterator it = arrayDocument.begin();
         it != arrayDocument.end();
         it++)
    {
        pItem->drop(*it);
    }

    event->accept();
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

CWizCategoryViewItemBase* CWizCategoryBaseView::itemAt(const QPoint& p) const
{
    return dynamic_cast<CWizCategoryViewItemBase*>(QTreeWidget::itemAt(p));
}


CWizCategoryViewItemBase* CWizCategoryBaseView::categoryItemFromIndex(const QModelIndex &index) const
{
    return dynamic_cast<CWizCategoryViewItemBase*>(itemFromIndex(index));
}

bool CWizCategoryBaseView::isHelperItemByIndex(const QModelIndex &index) const
{
    CWizCategoryViewItemBase* pItem = categoryItemFromIndex(index);
    if (NULL != dynamic_cast<const CWizCategoryViewSectionItem*>(pItem)) {
        return true;
    }
    else if (NULL != dynamic_cast<const CWizCategoryViewLinkItem*>(pItem)) {
        return true;
    }
    return false;
}

QModelIndex CWizCategoryBaseView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    QModelIndex index = QTreeWidget::moveCursor(cursorAction, modifiers);
    if (!index.isValid())
        return index;

    CWizCategoryViewItemBase* pItem = categoryItemFromIndex(index);
    if (CWizCategoryViewSectionItem* pSeparatorItem = dynamic_cast<CWizCategoryViewSectionItem*>(pItem))
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

void CWizCategoryBaseView::drawItem(QPainter* p, const QStyleOptionViewItemV4 *vopt) const
{
    CWizCategoryViewItemBase* pItem = categoryItemFromIndex(vopt->index);
    Q_ASSERT(pItem);

    if (pItem)
        pItem->draw(p, vopt);
}



/* ------------------------------ CWizCategoryView ------------------------------ */
CWizCategoryView::CWizCategoryView(CWizExplorerApp& app, QWidget* parent)
    : CWizCategoryBaseView(app, parent)
{
    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);

    initMenus();

    connect(this, SIGNAL(itemSelectionChanged()), SLOT(on_itemSelectionChanged()));
    connect(this, SIGNAL(itemClicked(QTreeWidgetItem*, int)), SLOT(on_itemClicked(QTreeWidgetItem *, int)));

    ExtensionSystem::PluginManager::addObject(this);
}

CWizCategoryView::~CWizCategoryView()
{
    ExtensionSystem::PluginManager::removeObject(this);
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
    actionRenameItem->setShortcut(QKeySequence::SaveAs);
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

//    QAction* actionQuitGroup = new QAction("QuitGroup", this);
//    actionQuitGroup->setShortcutContext(Qt::WidgetShortcut);
//    actionQuitGroup->setData(ActionQuitGroup);
//    addAction(actionQuitGroup);
    //connect(actionQuitGroup, SIGNAL(triggered()), SLOT(on_action_itemAttribute()));

    QAction* actionManageGroup = new QAction("ManageGroup", this);
    actionManageGroup->setShortcutContext(Qt::WidgetShortcut);
    actionManageGroup->setData(ActionItemManage);
    addAction(actionManageGroup);
    connect(actionManageGroup, SIGNAL(triggered()), SLOT(on_action_manageGroup()));

    QAction* actionManageBiz = new QAction("ManageGroup", this);
    actionManageBiz->setShortcutContext(Qt::WidgetShortcut);
    actionManageBiz->setData(ActionItemManage);
    addAction(actionManageBiz);
    connect(actionManageBiz, SIGNAL(triggered()), SLOT(on_action_manageBiz()));

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

    // group root menu normal
    m_menuNormalGroupRoot = new QMenu(this);
    m_menuNormalGroupRoot->addAction(actionNewDoc);
    m_menuNormalGroupRoot->addAction(actionNewItem);
    m_menuNormalGroupRoot->addSeparator();
    m_menuNormalGroupRoot->addAction(actionItemAttr);
//    m_menuNormalGroupRoot->addAction(actionQuitGroup);

    // group root menu admin
    m_menuAdminGroupRoot = new QMenu(this);
    m_menuAdminGroupRoot->addAction(actionNewDoc);
    m_menuAdminGroupRoot->addAction(actionNewItem);
    m_menuAdminGroupRoot->addSeparator();
    m_menuAdminGroupRoot->addAction(actionManageGroup);
//    m_menuAdminGroupRoot->addAction(actionQuitGroup);

    // group root menu normal
    m_menuOwnerGroupRoot = new QMenu(this);
    m_menuOwnerGroupRoot->addAction(actionNewDoc);
    m_menuOwnerGroupRoot->addAction(actionNewItem);
    m_menuOwnerGroupRoot->addSeparator();
    m_menuOwnerGroupRoot->addAction(actionManageGroup);

    //biz group root menu normal
    m_menuNormalBizGroupRoot = new QMenu(this);
    m_menuNormalBizGroupRoot->addAction(actionItemAttr);

    //biz group root menu admin
    m_menuAdminBizGroupRoot = new QMenu(this);
    m_menuAdminBizGroupRoot->addAction(actionManageBiz);


    // group menu
    m_menuGroup = new QMenu(this);
    m_menuGroup->addAction(actionNewDoc);
    m_menuGroup->addAction(actionNewItem);
    m_menuGroup->addAction(actionRenameItem);
    m_menuGroup->addSeparator();
    m_menuGroup->addAction(actionDeleteItem);
}

void CWizCategoryView::setActionsEnabled(bool enable)
{
    QList<QAction*> acts = actions();

    for (int i = 0; i < acts.size(); i++) {
        QAction* act = acts.at(i);
        act->setEnabled(enable);
    }
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
            } else if (type == BizGroupRootItem) {
                act->setText(CATEGORY_ACTION_BIZ_GROUP_ATTRIBUTE);
            }
            break;
        case ActionEmptyTrash:
            if (type == TrashItem) {
                act->setText(CATEGORY_ACTION_EMPTY_TRASH);
            }
            break;
        case ActionQuitGroup:
            if (type == GroupRootItem) {
                act->setText(CATEGORY_ACTION_QUIT_GROUP);
            }
            break;
        case ActionItemManage:
            if (type == GroupRootItem) {
                act->setText(CATEGORY_ACTION_MANAGE_GROUP);
            }else if (type == BizGroupRootItem) {
                act->setText(CATEGORY_ACTION_MANAGE_BIZ);
            }
            break;
        default:
            continue;
        }
    }
}

void CWizCategoryView::showTrashContextMenu(QPoint pos)
{
    resetMenu(TrashItem);
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

void CWizCategoryView::showNormalGroupRootContextMenu(QPoint pos)
{
    resetMenu(GroupRootItem);
    m_menuNormalGroupRoot->popup(pos);
}

void CWizCategoryView::showAdminGroupRootContextMenu(QPoint pos)
{
    resetMenu(GroupRootItem);
    m_menuAdminGroupRoot->popup(pos);
}

void CWizCategoryView::showOwnerGroupRootContextMenu(QPoint pos)
{
    resetMenu(GroupRootItem);
    m_menuOwnerGroupRoot->popup(pos);
}

void CWizCategoryView::showNormalBizGroupRootContextMenu(QPoint pos)
{
    resetMenu(BizGroupRootItem);
    m_menuNormalBizGroupRoot->popup(pos);
}

void CWizCategoryView::showAdminBizGroupRootContextMenu(QPoint pos, bool usable)
{
    resetMenu(BizGroupRootItem);

    const QList<QAction*>& acts = m_menuAdminBizGroupRoot->actions();
    foreach (QAction* act,  acts) {
        act->setEnabled(usable);
    }

    m_menuAdminBizGroupRoot->popup(pos);
}

void CWizCategoryView::showGroupContextMenu(QPoint pos)
{
    resetMenu(GroupItem);
    m_menuGroup->popup(pos);
}

bool CWizCategoryView::createDocument(WIZDOCUMENTDATA& data)
{
    bool bFallback = true;

    QString strKbGUID = m_dbMgr.db().kbGUID();
    QString strLocation = "/My Notes/";
    WIZTAGDATA tag;

    // trash first, because it's inherited
    if (CWizCategoryViewTrashItem* pItem = currentCategoryItem<CWizCategoryViewTrashItem>())
    {
        // only handle group trash
        if (pItem->kbGUID() != m_dbMgr.db().kbGUID()) {
            CWizCategoryViewGroupRootItem* pRItem =
                    dynamic_cast<CWizCategoryViewGroupRootItem*>(pItem->parent());

            Q_ASSERT(pRItem);

            strKbGUID = pRItem->kbGUID();
            bFallback = false;

            //set noTag item as current item.
            selectedItems().clear();
            for (int i = 0; i < pRItem->childCount(); i++) {
                CWizCategoryViewGroupNoTagItem* pNoTag =
                        dynamic_cast<CWizCategoryViewGroupNoTagItem*>(pRItem->child(i));
                if (0 != pNoTag) {
                    setCurrentItem(pNoTag);
                    break;
                }
            }
        }
    }
    else if (CWizCategoryViewFolderItem* pItem = currentCategoryItem<CWizCategoryViewFolderItem>())
    {
        // only handle individual folders except trash
        if (!CWizDatabase::IsInDeletedItems(pItem->location())) {
            strLocation = pItem->location();
            bFallback = false;
        }
    }
    else if (CWizCategoryViewTagItem* pItem = currentCategoryItem<CWizCategoryViewTagItem>())
    {
        tag = pItem->tag();
        bFallback = false;
    }
    else if (CWizCategoryViewGroupRootItem* pItem = currentCategoryItem<CWizCategoryViewGroupRootItem>())
    {
        strKbGUID = pItem->kbGUID();
        bFallback = false;
    }
    else if (CWizCategoryViewGroupNoTagItem* pItem = currentCategoryItem<CWizCategoryViewGroupNoTagItem>())
    {
        strKbGUID = pItem->kbGUID();
        bFallback = false;
    }
    else if (CWizCategoryViewGroupItem* pItem = currentCategoryItem<CWizCategoryViewGroupItem>())
    {
        strKbGUID = pItem->kbGUID();
        tag = pItem->tag();
        bFallback = false;
    }

    if (bFallback) {
        addAndSelectFolder(strLocation);
    }
    //
    CWizDatabase& db = m_dbMgr.db(strKbGUID);
    if (db.IsGroup()
            && !db.IsGroupAuthor())
    {
        return false;
    }

    bool ret = m_dbMgr.db(strKbGUID).CreateDocumentAndInit("<p><br/></p>", "", 0, tr("New note"), "newnote", strLocation, "", data);
    if (!ret) {
        TOLOG("Failed to new document!");
        return false;
    }

    if (!tag.strGUID.IsEmpty()) {
        CWizDocument doc(m_dbMgr.db(strKbGUID), data);
        doc.AddTag(tag);
    }
    //
    return true;
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
                                                          "", m_app.mainWindow());      //use mainWindow as parent

    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_user_newFolder_confirmed(int)));

    dialog->exec();

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
    m_dbMgr.db().SetLocalValueVersion("folders", -1);
}

void CWizCategoryView::on_action_user_newTag()
{
    CWizLineInputDialog* dialog = new CWizLineInputDialog(tr("New tag"),
                                                          tr("Please input tag name: "),
                                                          "", window());
    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_user_newTag_confirmed(int)));

    dialog->exec();
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
                                                          "", window());

    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_group_newFolder_confirmed(int)));

    dialog->exec();
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
    CWizFolderSelector* selector = new CWizFolderSelector(tr("Move folder"), m_app, window());
    selector->setAcceptRoot(true);

    connect(selector, SIGNAL(finished(int)), SLOT(on_action_user_moveFolder_confirmed(int)));
    selector->exec();
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

        //addAndSelectFolder(strLocation);
        //on_folder_deleted(strOldLocation);
    }
}

void CWizCategoryView::on_action_user_moveFolder_confirmed_progress(int nMax, int nValue,
                                                                    const QString& strOldLocation,
                                                                    const QString& strNewLocation,
                                                                    const WIZDOCUMENTDATA& data)
{
    MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());
    CWizProgressDialog* progress = mainWindow->progressDialog();
    progress->setVisible(true);

    progress->setActionString(tr("Move Note: %1 to %2").arg(strOldLocation).arg(strNewLocation));
    progress->setNotifyString(data.strTitle);
    progress->setProgress(nMax, nValue);
    if (nMax == nValue + 1) {
        progress->setVisible(false);
    }
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
    CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>();
    Q_ASSERT(p);

    CWizLineInputDialog* dialog = new CWizLineInputDialog(tr("Rename folder"),
                                                          tr("Please input new folder name: "),
                                                          p->name(),window());

    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_user_renameFolder_confirmed(int)));

    dialog->exec();
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

        if (strLocation == strOldLocation)
            return;

        // move all documents to new folder
        CWizFolder folder(m_dbMgr.db(), strOldLocation);
        connect(&folder, SIGNAL(moveDocument(int, int, const QString&, const QString&, const WIZDOCUMENTDATA&)),
                SLOT(on_action_user_renameFolder_confirmed_progress(int, int, const QString&, const QString&, const WIZDOCUMENTDATA&)));

        folder.MoveToLocation(strLocation);

        //addAndSelectFolder(strLocation);
        //on_folder_deleted(strOldLocation);
    }
}

void CWizCategoryView::on_action_user_renameFolder_confirmed_progress(int nMax, int nValue,
                                                                      const QString& strOldLocation,
                                                                      const QString& strNewLocation,
                                                                      const WIZDOCUMENTDATA& data)
{
    MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());
    CWizProgressDialog* progress = mainWindow->progressDialog();
    progress->setVisible(true);

    progress->setActionString(tr("Move Note: %1 to %2").arg(strOldLocation).arg(strNewLocation));
    progress->setNotifyString(data.strTitle);
    progress->setProgress(nMax, nValue);
    if (nMax == nValue + 1) {
        progress->setVisible(false);
    }
}

void CWizCategoryView::on_action_user_renameTag()
{
    CWizCategoryViewItemBase* p = currentCategoryItem<CWizCategoryViewItemBase>();

    CWizLineInputDialog* dialog = new CWizLineInputDialog(tr("Rename tag"),
                                                          tr("Please input tag name: "),
                                                          p->name(), window());

    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_user_renameTag_confirmed(int)));

    dialog->exec();
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
                                                          p->name(), window());

    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_group_renameFolder_confirmed(int)));

    dialog->exec();
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
        //if (!::WizIsPredefinedLocation(p->location())) {
            on_action_user_deleteFolder();
        //}
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
    QMessageBox* msgBox = new QMessageBox(window());
    msgBox->setWindowTitle(tr("Delete Folder"));
    msgBox->addButton(QMessageBox::Ok);
    msgBox->addButton(QMessageBox::Cancel);

    QString strWarning = tr("Do you really want to delete all notes inside folder: %1 ? (All notes will move to trash folder and remove from cloud server)").arg(p->location());
    msgBox->setText(strWarning);
    connect(msgBox,SIGNAL(finished(int)),this,SLOT(on_action_user_deleteFolder_confirmed(int)));
    msgBox->exec();
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

    QMessageBox* msgBox = new QMessageBox(window());
    msgBox->setWindowTitle(tr("Delete tag"));
    msgBox->addButton(QMessageBox::Ok);
    msgBox->addButton(QMessageBox::Cancel);

    QString strWarning = tr("Do you really want to delete tag: %1 ? (include child tags if any)").arg(p->tag().strName);
    msgBox->setText(strWarning);
    connect(msgBox,SIGNAL(finished(int)),this,SLOT(on_action_user_deleteTag_confirmed(int)));
    msgBox->exec();
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

    QMessageBox* msgBox = new QMessageBox(window());
    msgBox->setWindowTitle(tr("Delete group folder"));
    msgBox->addButton(QMessageBox::Ok);
    msgBox->addButton(QMessageBox::Cancel);

    QString strWarning = tr("Do you really want to delete folder: %1? (All notes will move to unclassified folder, It's safe.)").arg(p->tag().strName);
    msgBox->setText(strWarning);
    connect(msgBox,SIGNAL(finished(int)),this,SLOT(on_action_group_deleteFolder_confirmed(int)));
    msgBox->exec();
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
    if (currentCategoryItem<CWizCategoryViewGroupRootItem>())
    {
        on_action_groupAttribute();
    }
    else if(currentCategoryItem<CWizCategoryViewBizGroupRootItem>())
    {
        on_action_bizgAttribute();
    }
}

void CWizCategoryView::on_action_groupAttribute()
{

    CWizCategoryViewGroupRootItem* p = currentCategoryItem<CWizCategoryViewGroupRootItem>();
    if (p && !p->kbGUID().isEmpty()) {
        if (p->isBizGroup()) {
            viewBizGroupInfo(p->kbGUID(), p->bizGUID());
        } else {
            viewPersonalGroupInfo(p->kbGUID());
        }
    }
}

void CWizCategoryView::on_action_manageGroup()
{
    CWizCategoryViewGroupRootItem* p = currentCategoryItem<CWizCategoryViewGroupRootItem>();
    if (p && !p->kbGUID().isEmpty()) {
        if (p->isBizGroup()) {
            manageBizGroup(p->kbGUID(), p->bizGUID());
        } else {
            managePersonalGroup(p->kbGUID());
        }
    }
}

void CWizCategoryView::on_action_bizgAttribute()
{
    CWizCategoryViewItemBase* p = currentCategoryItem<CWizCategoryViewItemBase>();
    if (p && !p->kbGUID().isEmpty()) {

        viewBizInfo(p->kbGUID());
    }
}

void CWizCategoryView::on_action_itemManage()
{
    if (currentCategoryItem<CWizCategoryViewGroupRootItem>())
    {
        on_action_manageGroup();
    }
    else if(currentCategoryItem<CWizCategoryViewBizGroupRootItem>())
    {
        on_action_manageBiz();
    }
}

void CWizCategoryView::on_action_manageBiz()
{
    CWizCategoryViewBizGroupRootItem* p = currentCategoryItem<CWizCategoryViewBizGroupRootItem>();
    if (p && !p->biz().bizGUID.isEmpty()) {
        manageBiz(p->biz().bizGUID);
    }
}

void CWizCategoryView::showWebDialogWithToken(const QString& windowTitle, const QString& url)
{
    CWizWebSettingsDialog* pDlg = new CWizWebSettingsWithTokenDialog(url, QSize(800, 480), window());
    pDlg->setWindowTitle(windowTitle);
    pDlg->exec();
    //
    delete pDlg;
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
    } else {
        setActionsEnabled(true); // enable all actions if selected is not group
    }

    // notify of selection
    if (currentCategoryItem<CWizCategoryViewMessageItem>()) {
        Q_EMIT documentsHint(tr("Recent meesages"));
    } else if (currentCategoryItem<CWizCategoryViewAllFoldersItem>()) {
        Q_EMIT documentsHint(tr("Recent notes"));
    } else if (currentCategoryItem<CWizCategoryViewAllTagsItem>()) {
        Q_EMIT documentsHint(tr("No tag notes"));
    } else {
        Q_EMIT documentsHint(currentItem()->text(0));
    }
}
void CWizCategoryView::on_itemClicked(QTreeWidgetItem *item, int column)
{
    if (CWizCategoryViewLinkItem* pLink = dynamic_cast<CWizCategoryViewLinkItem*>(item))
    {
        if (LINK_COMMAND_ID_CREATE_GROUP == pLink->commandId())
        {
            createGroup();
        }
    }
    else if (CWizCategoryViewSectionItem* sItem = dynamic_cast<CWizCategoryViewSectionItem*>(item))
    {
        if(CATEGORY_TEAM_GROUPS == sItem->name() && sItem->extraButtonClickTest())
        {
            createGroup();
        }
    }
}

void CWizCategoryView::createGroup()
{
    QString strUrl = WizService::ApiEntry::standardCommandUrl("create_group", WIZ_TOKEN_IN_URL_REPLACE_PART);
    showWebDialogWithToken(tr("Create new group"), strUrl);
}

void CWizCategoryView::viewPersonalGroupInfo(const QString& groupGUID)
{
    QString extInfo = "kb=" + groupGUID;
    QString strUrl = WizService::ApiEntry::standardCommandUrl("view_personal_group", WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    showWebDialogWithToken(tr("View group info"), strUrl);
}

void CWizCategoryView::viewBizGroupInfo(const QString& groupGUID, const QString& bizGUID)
{
    QString extInfo = "kb=" + groupGUID + "&&biz=" + bizGUID;
    QString strUrl = WizService::ApiEntry::standardCommandUrl("view_biz_group", WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    showWebDialogWithToken(tr("View group info"), strUrl);
}

void CWizCategoryView::managePersonalGroup(const QString& groupGUID)
{
    QString extInfo = "kb=" + groupGUID;
    QString strUrl = WizService::ApiEntry::standardCommandUrl("manage_personal_group", WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    showWebDialogWithToken(tr("Manage group"), strUrl);
}

void CWizCategoryView::manageBizGroup(const QString& groupGUID, const QString& bizGUID)
{
    QString extInfo = "kb=" + groupGUID + "&biz=" + bizGUID;
    QString strUrl = WizService::ApiEntry::standardCommandUrl("manage_biz_group", WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    showWebDialogWithToken(tr("Manage group"), strUrl);
}

void CWizCategoryView::viewBizInfo(const QString& bizGUID)
{
    QString extInfo = "biz=" + bizGUID;
    QString strUrl = WizService::ApiEntry::standardCommandUrl("view_biz", WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    showWebDialogWithToken(tr("View team info"), strUrl);
}

void CWizCategoryView::manageBiz(const QString& bizGUID)
{
    QString extInfo = "biz=" + bizGUID;
    QString strUrl = WizService::ApiEntry::standardCommandUrl("manage_biz", WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    showWebDialogWithToken(tr("Manage team"), strUrl);
}


void CWizCategoryView::init()
{
    initGeneral();
    initFolders();
    initTags();
    initStyles();
    initGroups();
    //
    resetSections();

    loadState();
}

void CWizCategoryView::resetSections()
{
    sortItems(0, Qt::AscendingOrder);
    //
    //
    //remove extra section
    for (int i = topLevelItemCount() - 1; i >= 1; i--)
    {
        if (NULL != dynamic_cast<CWizCategoryViewSectionItem *>(topLevelItem(i)))
        {
            if (NULL != dynamic_cast<CWizCategoryViewSectionItem *>(topLevelItem(i - 1)))
            {
                takeTopLevelItem(i);
            }
        }
    }
    //remove invalid section
    for (int i = topLevelItemCount() - 1; i >= 0; i--)
    {
        if (i > 0 && i < topLevelItemCount() - 1)
        {
            if (NULL != dynamic_cast<CWizCategoryViewSectionItem *>(topLevelItem(i)))
            {
                CWizCategoryViewItemBase* pPrevItem = dynamic_cast<CWizCategoryViewItemBase *>(topLevelItem(i - 1));
                CWizCategoryViewItemBase* pNextItem = dynamic_cast<CWizCategoryViewItemBase *>(topLevelItem(i + 1));
                if (pPrevItem && pNextItem)
                {
                    QString prevSectionName = pPrevItem->getSectionName();
                    QString nextSectionName = pNextItem->getSectionName();
                    if (prevSectionName == nextSectionName)
                    {
                        takeTopLevelItem(i);
                    }
                }
            }
        }
    }

    QString lastSectionName;
    //
    for (int i = 0; i < topLevelItemCount(); i++)
    {
        CWizCategoryViewItemBase* pItem = dynamic_cast<CWizCategoryViewItemBase *>(topLevelItem(i));
        if (!pItem)
            continue;
        //
        if (NULL != dynamic_cast<CWizCategoryViewSectionItem *>(pItem))
        {
            lastSectionName = "";
        }
        else
        {
            QString sectionName = pItem->getSectionName();
            Q_ASSERT(!sectionName.isEmpty());
            //
            if (lastSectionName != sectionName)
            {
                CWizCategoryViewSectionItem* pExistingSection = NULL;
                if (i > 0)
                {
                    pExistingSection = dynamic_cast<CWizCategoryViewSectionItem *>(topLevelItem(i - 1));
                }
                //
                if (pExistingSection)
                {
                    pExistingSection->reset(sectionName, pItem->getSortOrder());
                }
                else
                {
                    pExistingSection = new CWizCategoryViewSectionItem(m_app, sectionName, pItem->getSortOrder() - 1);
                    if(CATEGORY_TEAM_GROUPS == sectionName)
                    {
                        QString strIconPath = ::WizGetSkinResourcePath(m_app.userSettings().skin()) + "plus.png";
                        pExistingSection->setExtraButtonIcon(strIconPath);
                    }
                    insertTopLevelItem(i, pExistingSection);
                    i++;
                }
                //
                lastSectionName = sectionName;
            }
        }
    }
}

void CWizCategoryView::updatePrivateFolderDocumentCount()
{
    if (!m_timerUpdateFolderCount) {
        m_timerUpdateFolderCount = new QTimer(this);
        m_timerUpdateFolderCount->setSingleShot(true);
        connect(m_timerUpdateFolderCount, SIGNAL(timeout()), SLOT(on_updatePrivateFolderDocumentCount_timeout()));
    }

    if (m_timerUpdateFolderCount->isActive()) {
        return;
    }

    m_timerUpdateFolderCount->start(2000);
}

void CWizCategoryView::on_updatePrivateFolderDocumentCount_timeout()
{
    sender()->deleteLater();
    updatePrivateFolderDocumentCount_impl();
}

void CWizCategoryView::updatePrivateFolderDocumentCount_impl()
{
    std::map<CString, int> mapDocumentCount;
    if (!m_dbMgr.db().GetAllLocationsDocumentCount(mapDocumentCount)) {
        TOLOG("[ERROR]: Failed to get all locations count map");
        return;
    }

    CWizCategoryViewItemBase* pFolderRoot = findAllFolderItem();
    if (!pFolderRoot)
        return;

    // folder items
    int nTotal = 0;
    updateChildFolderDocumentCount(pFolderRoot, mapDocumentCount, nTotal);
    pFolderRoot->setDocumentsCount(-1, nTotal);

    // trash item
    for (int i = pFolderRoot->childCount() - 1; i >= 0; i--) {
        if (CWizCategoryViewTrashItem* pTrash = dynamic_cast<CWizCategoryViewTrashItem*>(pFolderRoot->child(i))) {
            pTrash->setDocumentsCount(-1, m_dbMgr.db().GetTrashDocumentCount());
        }
    }

    update();
}

void CWizCategoryView::updateChildFolderDocumentCount(CWizCategoryViewItemBase* pItem,
                                                     const std::map<CString, int>& mapDocumentCount,
                                                     int& allCount)
{
    for (int i = 0; i < pItem->childCount(); i++) {
        if (CWizCategoryViewItemBase* pItemChild = dynamic_cast<CWizCategoryViewItemBase*>(pItem->child(i))) {
            int nCurrentChild = 0;
            std::map<CString, int>::const_iterator itCurrent = mapDocumentCount.find(pItemChild->name());
            if (itCurrent != mapDocumentCount.end()) {
                nCurrentChild = itCurrent->second;
            }

            int nTotalChild = 0;
            updateChildFolderDocumentCount(pItemChild, mapDocumentCount, nTotalChild);

            nTotalChild += nCurrentChild;

            if (pItemChild->childCount() && nTotalChild) { // only show total number when child folders's document count is not zero
                pItemChild->setDocumentsCount(nCurrentChild, nTotalChild);
            } else {
                pItemChild->setDocumentsCount(-1, nTotalChild);
            }

            if (CWizCategoryViewFolderItem* pFolder = dynamic_cast<CWizCategoryViewFolderItem*>(pItemChild))
            {
                if (!pFolder->location().startsWith(LOCATION_DELETED_ITEMS))
                {
                    allCount += nTotalChild;
                }
            }
        }
    }
}

void CWizCategoryView::updateGroupFolderDocumentCount(const QString& strKbGUID)
{
    Q_ASSERT(!strKbGUID.isEmpty());

    QTimer* timer = NULL;
    QMap<QString, QTimer*>::const_iterator it = m_mapTimerUpdateGroupCount.find(strKbGUID);
    if (it == m_mapTimerUpdateGroupCount.end()) {
        QSignalMapper* mapper = new QSignalMapper(this);
        timer = new QTimer(mapper);
        timer->setSingleShot(true);
        mapper->setMapping(timer, strKbGUID);
        connect(timer, SIGNAL(timeout()), mapper, SLOT(map()));
        connect(mapper, SIGNAL(mapped(const QString&)),
                SLOT(on_updateGroupFolderDocumentCount_mapped_timeout(const QString&)));

        m_mapTimerUpdateGroupCount.insert(strKbGUID, timer);
    } else {
        timer = it.value();
    }

    Q_ASSERT(timer);

    if (timer->isActive()) {
        return;
    }

    timer->start(2000);
}

void CWizCategoryView::updateGroupFolderDocumentCount_impl(const QString &strKbGUID)
{
    // NOTE: groupItem have been handled as tag in other palce.Here use tagFunction to calc groupItem.
    std::map<CString, int> mapDocumentCount;
    if (!m_dbMgr.db(strKbGUID).GetAllTagsDocumentCount(mapDocumentCount)) {
        TOLOG("[ERROR]: Failed to get all tags count map");
        return;
    }

    CWizCategoryViewItemBase* pGroupRoot = NULL;
    pGroupRoot = findGroup(strKbGUID);

    if (!pGroupRoot)
        return;

    int nCurrent = 0;
    if (!m_dbMgr.db(strKbGUID).GetDocumentsNoTagCount(nCurrent)) {
        qDebug() << "Failed to get no tag documents count, kb_guid: " << strKbGUID;
        return;
    }

    int nTotalChild = 0;
    updateChildTagDocumentCount(pGroupRoot, mapDocumentCount, nTotalChild);

    // trash item
    for (int i = pGroupRoot->childCount() - 1; i >= 0; i--) {
        if (CWizCategoryViewTrashItem* pTrash = dynamic_cast<CWizCategoryViewTrashItem*>(pGroupRoot->child(i))) {
            pTrash->setDocumentsCount(-1, m_dbMgr.db(strKbGUID).GetTrashDocumentCount());
        }

        if (CWizCategoryViewGroupNoTagItem* pItem = dynamic_cast<CWizCategoryViewGroupNoTagItem*>(pGroupRoot->child(i))) {
            int nCount = 0;
            if (m_dbMgr.db(strKbGUID).GetDocumentsNoTagCount(nCount)) {
                pItem->setDocumentsCount(-1, nCount);
            }
        }
    }

    update();
}

void CWizCategoryView::updatePrivateTagDocumentCount()
{
    if (!m_timerUpdateTagCount) {
        m_timerUpdateTagCount = new QTimer(this);
        m_timerUpdateTagCount->setSingleShot(true);
        connect(m_timerUpdateTagCount, SIGNAL(timeout()), SLOT(on_updatePrivateTagDocumentCount_timeout()));
    }

    if (m_timerUpdateTagCount->isActive()) {
        return;
    }

    m_timerUpdateTagCount->start(2000);
}

void CWizCategoryView::updateGroupTagDocumentCount(const QString& strKbGUID)
{
    Q_UNUSED (strKbGUID)
}

void CWizCategoryView::on_updatePrivateTagDocumentCount_timeout()
{
    sender()->deleteLater();
    updatePrivateTagDocumentCount_impl();
}

void CWizCategoryView::on_updateGroupFolderDocumentCount_mapped_timeout(const QString& strKbGUID)
{
    sender()->deleteLater();
    m_mapTimerUpdateGroupCount.remove(strKbGUID);

    updateGroupFolderDocumentCount_impl(strKbGUID);
}

void CWizCategoryView::updatePrivateTagDocumentCount_impl(const QString& strKbGUID)
{
    std::map<CString, int> mapDocumentCount;
    if (!m_dbMgr.db(strKbGUID).GetAllTagsDocumentCount(mapDocumentCount)) {
        TOLOG("[ERROR]: Failed to get all tags count map");
        return;
    }

    CWizCategoryViewItemBase* pTagRoot = NULL;
    if (strKbGUID.isEmpty()) {
        pTagRoot = findAllTagsItem();
    } else {
        pTagRoot = findGroup(strKbGUID);
    }

    if (!pTagRoot)
        return;

    int nCurrent = 0;
    if (!m_dbMgr.db(strKbGUID).GetDocumentsNoTagCount(nCurrent)) {
        qDebug() << "Failed to get no tag documents count, kb_guid: " << strKbGUID;
        return;
    }

    int nTotalChild = 0;
    updateChildTagDocumentCount(pTagRoot, mapDocumentCount, nTotalChild);

    // trash item
    for (int i = pTagRoot->childCount() - 1; i >= 0; i--) {
        if (CWizCategoryViewTrashItem* pTrash = dynamic_cast<CWizCategoryViewTrashItem*>(pTagRoot->child(i))) {
            pTrash->setDocumentsCount(-1, m_dbMgr.db(strKbGUID).GetTrashDocumentCount());
        }

        if (CWizCategoryViewGroupNoTagItem* pItem = dynamic_cast<CWizCategoryViewGroupNoTagItem*>(pTagRoot->child(i))) {
            int nCount = 0;
            if (m_dbMgr.db(strKbGUID).GetDocumentsNoTagCount(nCount)) {
                pItem->setDocumentsCount(-1, nCount);
            }
        }
    }

    update();
}

void CWizCategoryView::updateChildTagDocumentCount(CWizCategoryViewItemBase* pItem,
                                                  const std::map<CString, int>& mapDocumentCount, int &allCount)
{
    for (int i = 0; i < pItem->childCount(); i++) {
        QString strGUID;
        if (CWizCategoryViewTagItem* pItemChild = dynamic_cast<CWizCategoryViewTagItem*>(pItem->child(i)))
        {
            strGUID = pItemChild->tag().strGUID;
        }
        else if (CWizCategoryViewGroupItem* pItemChild = dynamic_cast<CWizCategoryViewGroupItem*>(pItem->child(i)))
        {
            strGUID = pItemChild->tag().strGUID;
        }
        else if (CWizCategoryViewGroupNoTagItem* pItemChild = dynamic_cast<CWizCategoryViewGroupNoTagItem*>(pItem->child(i)))
        {
            Q_UNUSED(pItemChild);
            continue;
        }
        else if (CWizCategoryViewTrashItem* pTrash = dynamic_cast<CWizCategoryViewTrashItem*>(pItem->child(i)))
        {
            Q_UNUSED(pTrash);
            continue;
        }

        Q_ASSERT(!strGUID.isEmpty());

        if (CWizCategoryViewItemBase* pItemChild = dynamic_cast<CWizCategoryViewItemBase*>(pItem->child(i))) {
            int nCurrentChild = 0;
            std::map<CString, int>::const_iterator itCurrent = mapDocumentCount.find(strGUID);
            if (itCurrent != mapDocumentCount.end()) {
                nCurrentChild = itCurrent->second;
            }

            int nTotalChild = 0;
            updateChildTagDocumentCount(pItemChild, mapDocumentCount, nTotalChild);

            nTotalChild += nCurrentChild;

            if (pItemChild->childCount() && nTotalChild) { // only show total number when child folders's document count is not zero
                pItemChild->setDocumentsCount(nCurrentChild, nTotalChild);
            } else {
                pItemChild->setDocumentsCount(-1, nTotalChild);
            }

            allCount += nTotalChild;
        }
    }
}

void CWizCategoryView::initGeneral()
{
    //CWizCategoryViewCategoryItem* pCategoryItem = new CWizCategoryViewCategoryItem(m_app, CATEGORY_GENERAL);
    //addTopLevelItem(pCategoryItem);

    CWizCategoryViewMessageItem* pMsg = new CWizCategoryViewMessageItem(m_app, CATEGORY_MESSAGES_ALL, CWizCategoryViewMessageItem::All);
    addTopLevelItem(pMsg);

    pMsg->setUnread(m_dbMgr.db().getUnreadMessageCount());

    //QList<QTreeWidgetItem*> pList;
    //pList.append(new CWizCategoryViewMessageItem(m_app, CATEGORY_MESSAGES_SEND_TO_ME, CWizCategoryViewMessageItem::SendToMe));
    //pList.append(new CWizCategoryViewMessageItem(m_app, CATEGORY_MESSAGES_MODIFY, CWizCategoryViewMessageItem::ModifyNote));
    //pList.append(new CWizCategoryViewMessageItem(m_app, CATEGORY_MESSAGES_COMMENTS, CWizCategoryViewMessageItem::Comment));
    //pList.append(new CWizCategoryViewMessageItem(m_app, CATEGORY_MESSAGES_SEND_FROM_ME, CWizCategoryViewMessageItem::SendFromMe));
    //pMsg->addChildren(pList);

    //CWizCategoryViewShortcutRootItem* pShortcutRoot = new CWizCategoryViewShortcutRootItem(m_app, CATEGORY_SHORTCUTS);
    //addTopLevelItem(pShortcutRoot);
    //pShortcutRoot->setHidden(true);

    //CWizCategoryViewSearchRootItem* pSearchRoot = new CWizCategoryViewSearchRootItem(m_app, CATEGORY_SEARCH);
    //addTopLevelItem(pSearchRoot);
    //pSearchRoot->setHidden(true);
}

void CWizCategoryView::sortFolders()
{
    CWizCategoryViewAllFoldersItem* pFolderRoot = dynamic_cast<CWizCategoryViewAllFoldersItem *>(findAllFolderItem());
    if (!pFolderRoot)
        return;

    pFolderRoot->sortChildren(0, Qt::AscendingOrder);

    for (int i = 1; i < pFolderRoot->childCount(); i++)
    {
        CWizCategoryViewFolderItem* pFolder = dynamic_cast<CWizCategoryViewFolderItem*>(pFolderRoot->child(i));
        if (!pFolder)
            return;

        sortFolders(pFolder);
    }
}

void CWizCategoryView::sortFolders(CWizCategoryViewFolderItem* pItem)
{
    if (!pItem)
        return;

    pItem->sortChildren(0, Qt::AscendingOrder);

    for (int i = 1; i < pItem->childCount(); i++)
    {
        CWizCategoryViewFolderItem* pFolder = dynamic_cast<CWizCategoryViewFolderItem*>(pItem->child(i));
        if (!pFolder)
            return;

        sortFolders(pFolder);
    }
}

void CWizCategoryView::initFolders()
{
    //CWizCategoryViewSpacerItem* pSpacer = new CWizCategoryViewSpacerItem(m_app);
    //addTopLevelItem(pSpacer);

    //CWizCategoryViewCategoryItem* pCategoryItem = new CWizCategoryViewCategoryItem(m_app, CATEGORY_PERSONAL);
    //addTopLevelItem(pCategoryItem);

    CWizCategoryViewAllFoldersItem* pAllFoldersItem = new CWizCategoryViewAllFoldersItem(m_app, CATEGORY_FOLDERS, m_dbMgr.db().kbGUID());
    addTopLevelItem(pAllFoldersItem);

    CWizStdStringArray arrayAllLocation;
    m_dbMgr.db().GetAllLocations(arrayAllLocation);

    // folder cache
    CWizStdStringArray arrayExtLocation;
    m_dbMgr.db().GetExtraFolder(arrayExtLocation);

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
        arrayAllLocation.push_back(LOCATION_DEFAULT);
    }

    doLocationSanityCheck(arrayAllLocation);

    initFolders(pAllFoldersItem, QString(), arrayAllLocation);
    CWizCategoryViewTrashItem* pTrash = new CWizCategoryViewTrashItem(m_app, m_dbMgr.db().kbGUID());
    pAllFoldersItem->addChild(pTrash);

    pAllFoldersItem->setExpanded(true);

    sortFolders();
    updatePrivateFolderDocumentCount();

    // push back folders cache
    for (CWizStdStringArray::const_iterator it = arrayAllLocation.begin();
         it != arrayAllLocation.end();
         it++) {
        m_dbMgr.db().AddExtraFolder(*it);
    }
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

        // sub folders must have parent
        bool bHasParent = false;
        int idx = strLocation.lastIndexOf("/", -2);

        // root: /A/
        if (idx == 0) {
                //qDebug() << "Folder :" << strLocation << ", it's root!'";
                bHasParent = true;
        } else {
            for (CWizStdStringArray::const_iterator it = arrayLocation.begin();
                 it != arrayLocation.end();
                 it++) {
                if (*it == strLocation.left(idx + 1)) {
                    //qDebug() << "Folder :" << strLocation << ", parent:" << *it;
                    bHasParent = true;
                }
            }
        }

        if (strLocation.left(1) != "/" && strLocation.right(1) != "/") {
            qDebug() << "[doLocationSanityCheck]Find folder name have not respect location naming spec: " << strLocation;

            // remove from array
            arrayLocation.erase(arrayLocation.begin() + i);

        } else if (!bHasParent) {
            qDebug() << "[doLocationSanityCheck]Find folder not have parent: " << strLocation;

            // add all of it's parents
            QString str = strLocation;
            int idx = str.lastIndexOf("/", -2);
            while (idx) {
                str = str.left(idx + 1);
                idx = str.lastIndexOf("/", -2);

                if (-1 == ::WizFindInArray(arrayLocation, str)) {
                    arrayLocation.push_back(str);
                }
            }
        }
    }

    // debug
    //qDebug() << "dump folders:";
    //for (CWizStdStringArray::const_iterator it = arrayLocation.begin();
    //     it != arrayLocation.end();
    //     it++) {
    //    qDebug() << *it;
    //}
}

void CWizCategoryView::initTags()
{
    CWizCategoryViewAllTagsItem* pAllTagsItem = new CWizCategoryViewAllTagsItem(m_app, CATEGORY_TAGS, m_dbMgr.db().kbGUID());
    addTopLevelItem(pAllTagsItem);

    initTags(pAllTagsItem, "");

    pAllTagsItem->setExpanded(true);
    pAllTagsItem->sortChildren(0, Qt::AscendingOrder);

//    updatePrivateFolderDocumentCount();
    updatePrivateTagDocumentCount();
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

   pParent->sortChildren(0, Qt::AscendingOrder);
}

void CWizCategoryView::initStyles()
{
    //CWizCategoryViewStyleRootItem* pStyleRoot = new CWizCategoryViewStyleRootItem(m_app, CATEGORY_STYLES);
    //addTopLevelItem(pStyleRoot);
    //pStyleRoot->setHidden(true);
}

void CWizCategoryView::initGroups()
{
    CWizGroupDataArray arrayGroup;
    m_dbMgr.db().GetUserGroupInfo(arrayGroup);

    //
    CWizBizDataArray arrayBiz;
    m_dbMgr.db().GetUserBizInfo(false, arrayGroup, arrayBiz);
    //
    std::vector<CWizCategoryViewItemBase*> arrayGroupsItem;
    //
    for (CWizBizDataArray::const_iterator it = arrayBiz.begin();
         it != arrayBiz.end();
         it++)
    {
        const WIZBIZDATA& biz = *it;
        CWizCategoryViewBizGroupRootItem* pBizGroupItem = new CWizCategoryViewBizGroupRootItem(m_app, biz);
        addTopLevelItem(pBizGroupItem);
        pBizGroupItem->setExpanded(true);
        arrayGroupsItem.push_back(pBizGroupItem);
    }
    //
    CWizGroupDataArray arrayOwnGroup;
    CWizDatabase::GetOwnGroups(arrayGroup, arrayOwnGroup);
    if (!arrayOwnGroup.empty())
    {
        CWizCategoryViewOwnGroupRootItem* pOwnGroupItem = new CWizCategoryViewOwnGroupRootItem(m_app);
        addTopLevelItem(pOwnGroupItem);
        pOwnGroupItem->setExpanded(true);
        arrayGroupsItem.push_back(pOwnGroupItem);
    }
    //
    CWizGroupDataArray arrayJionedGroup;
    CWizDatabase::GetJionedGroups(arrayGroup, arrayJionedGroup);
    if (!arrayJionedGroup.empty())
    {
        CWizCategoryViewJionedGroupRootItem* pJionedGroupItem = new CWizCategoryViewJionedGroupRootItem(m_app);
        addTopLevelItem(pJionedGroupItem);
        pJionedGroupItem->setExpanded(true);
        arrayGroupsItem.push_back(pJionedGroupItem);
    }

    int nTotal = m_dbMgr.count();
    for (int i = 0; i < nTotal; i++) {
        initGroup(m_dbMgr.at(i));
        updateGroupFolderDocumentCount(m_dbMgr.at(i).kbGUID());
    }
    //
    for (std::vector<CWizCategoryViewItemBase*>::const_iterator it = arrayGroupsItem.begin();
         it != arrayGroupsItem.end();
         it++)
    {
        CWizCategoryViewItemBase* pItem = *it;
        pItem->sortChildren(0, Qt::AscendingOrder);
    }
    //
    resetCreateGroupLink();
}

void CWizCategoryView::resetCreateGroupLink()
{
    bool hasGroup = false;
    int createLinkIndex = -1;
    for (int i = 0; i < topLevelItemCount(); i++)
    {
        if (NULL != dynamic_cast<CWizCategoryViewCreateGroupLinkItem *>(topLevelItem(i)))
        {
            createLinkIndex = i;
        }
        else if (NULL != dynamic_cast<CWizCategoryViewGroupsRootItem *>(topLevelItem(i)))
        {
            hasGroup = true;
        }
    }
    //
    if (hasGroup)
    {
        if (-1 != createLinkIndex)
        {
            takeTopLevelItem(createLinkIndex);
        }
    }
    else
    {
        if (-1 == createLinkIndex)
        {
            CWizCategoryViewCreateGroupLinkItem* pItem = new CWizCategoryViewCreateGroupLinkItem(m_app, tr("Create new group..."), LINK_COMMAND_ID_CREATE_GROUP);
            addTopLevelItem(pItem);
        }
    }
}

void CWizCategoryView::initGroup(CWizDatabase& db)
{
    bool itemCreeated = false;
    initGroup(db, itemCreeated);
}
void CWizCategoryView::initGroup(CWizDatabase& db, bool& itemCreeated)
{
    itemCreeated = false;
    if (findGroup(db.kbGUID()))
        return;
    //
    WIZGROUPDATA group;
    m_dbMgr.db().GetGroupData(db.kbGUID(), group);
    //
    //
    QTreeWidgetItem* pRoot = findGroupsRootItem(group);
    if (!pRoot) {
        return;
    }

    itemCreeated = true;
    //
    CWizCategoryViewGroupRootItem* pGroupItem = new CWizCategoryViewGroupRootItem(m_app, group);
    pRoot->addChild(pGroupItem);

    initGroup(db, pGroupItem, "");

    CWizCategoryViewGroupNoTagItem* pGroupNoTagItem = new CWizCategoryViewGroupNoTagItem(m_app, db.kbGUID());
    pGroupItem->addChild(pGroupNoTagItem);

    CWizCategoryViewTrashItem* pTrashItem = new CWizCategoryViewTrashItem(m_app, db.kbGUID());
    pGroupItem->addChild(pTrashItem);

    // only show trash if permission is enough
    if (db.permission() > WIZ_USERGROUP_SUPER) {
        pTrashItem->setHidden(true);
    }

    pGroupItem->sortChildren(0, Qt::AscendingOrder);
    //
    resetCreateGroupLink();
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
    //
    pParent->sortChildren(0, Qt::AscendingOrder);
}

CWizCategoryViewItemBase* CWizCategoryView::findGroupsRootItem(const WIZGROUPDATA& group, bool bCreate /* = true*/)
{
    if (group.IsBiz())
    {
        WIZBIZDATA biz;
        if (!m_dbMgr.db().GetBizData(group.bizGUID, biz))
            return NULL;
        //
        return findBizGroupsRootItem(biz);
    }
    else
    {
        if (group.IsOwn())
            return findOwnGroupsRootItem(bCreate);
        else
            return findJionedGroupsRootItem(bCreate);
    }
}

CWizCategoryViewItemBase* CWizCategoryView::findBizGroupsRootItem(const WIZBIZDATA& biz, bool bCreate /*= true*/)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        CWizCategoryViewBizGroupRootItem* pItem = dynamic_cast<CWizCategoryViewBizGroupRootItem*>(topLevelItem(i));
        if (!pItem)
            continue;
        if (pItem->biz().bizGUID == biz.bizGUID)
            return pItem;
    }
    if (!bCreate)
        return NULL;
    //
    CWizCategoryViewBizGroupRootItem* pItem = new CWizCategoryViewBizGroupRootItem(m_app, biz);;
    addTopLevelItem(pItem);
    //
    resetSections();
    //
    return pItem;
}

CWizCategoryViewItemBase* CWizCategoryView::findOwnGroupsRootItem(bool bCreate /*= true*/)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        CWizCategoryViewOwnGroupRootItem* pItem = dynamic_cast<CWizCategoryViewOwnGroupRootItem*>(topLevelItem(i));
        if (!pItem)
            continue;
        //
        return pItem;
    }
    //
    if (!bCreate)
        return NULL;
    //
    CWizCategoryViewOwnGroupRootItem* pItem = new CWizCategoryViewOwnGroupRootItem(m_app);;
    addTopLevelItem(pItem);
    //
    resetSections();
    return pItem;
}

CWizCategoryViewItemBase* CWizCategoryView::findJionedGroupsRootItem(bool bCreate /*= true*/)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        CWizCategoryViewJionedGroupRootItem* pItem = dynamic_cast<CWizCategoryViewJionedGroupRootItem*>(topLevelItem(i));
        if (!pItem)
            continue;
        //
        return pItem;
    }
    //
    if (!bCreate)
        return NULL;
    //
    CWizCategoryViewJionedGroupRootItem* pItem = new CWizCategoryViewJionedGroupRootItem(m_app);;
    addTopLevelItem(pItem);
    //
    resetSections();
    return pItem;
}

CWizCategoryViewItemBase* CWizCategoryView::findAllFolderItem()
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        CWizCategoryViewAllFoldersItem* pItem = dynamic_cast<CWizCategoryViewAllFoldersItem*>(topLevelItem(i));
        if (pItem) {
            return pItem;
        }
    }
    //
    return NULL;
}
CWizCategoryViewItemBase* CWizCategoryView::findAllTagsItem()
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        CWizCategoryViewAllTagsItem* pItem = dynamic_cast<CWizCategoryViewAllTagsItem*>(topLevelItem(i));
        if (pItem) {
            return pItem;
        }
    }
    //
    return NULL;
}
CWizCategoryViewItemBase* CWizCategoryView::findAllMessagesItem()
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        CWizCategoryViewMessageItem* pItem = dynamic_cast<CWizCategoryViewMessageItem*>(topLevelItem(i));
        if (pItem) {
            return pItem;
        }
    }
    //
    return NULL;
}

/*

CWizCategoryViewItemBase* CWizCategoryView::findCategory(const QString& strName, bool bCreate)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        CWizCategoryViewItemBase* pItem = dynamic_cast<CWizCategoryViewItemBase*>(topLevelItem(i));
        if (pItem && pItem->name() == strName) {
            return pItem;
        }
    }

    if (!bCreate)
        return NULL;

    // create group root if not found
    if (strName == CATEGORY_ENTERPRISE)
    {
        // find style root and insert after it
        CWizCategoryViewItemBase* pStyle = findCategory(CATEGORY_STYLES, false);
        if (!pStyle)
            return NULL;

        int index = indexOfTopLevelItem(pStyle);

        CWizCategoryViewSpacerItem* pSpacer = new CWizCategoryViewSpacerItem(m_app);
        insertTopLevelItem(index + 1, pSpacer);

        CWizCategoryViewCategoryItem* pItem = new CWizCategoryViewCategoryItem(m_app, CATEGORY_ENTERPRISE);
        insertTopLevelItem(index + 2, pItem);

        return pItem;
    }
    else if (strName == CATEGORY_GROUP)
    {
        // always append on the end
        if (!findCategory(CATEGORY_INDIVIDUAL, false)) {
            CWizCategoryViewSpacerItem* pSpacer = new CWizCategoryViewSpacerItem(m_app);
            addTopLevelItem(pSpacer);

            CWizCategoryViewCategoryItem* pRoot = new CWizCategoryViewCategoryItem(m_app, CATEGORY_INDIVIDUAL);
            addTopLevelItem(pRoot);
        }

        // insert individual group root
        CWizCategoryViewGroupsRootItem* pItem  = new CWizCategoryViewGroupsRootItem(m_app, CATEGORY_GROUP, "");
        addTopLevelItem(pItem);
        pItem->setExpanded(true);

        return pItem;
    }
    else
    {
        // it should be enterprise groups root
        CWizCategoryViewItemBase* pEnterprise = findCategory(CATEGORY_ENTERPRISE);
        if (!pEnterprise)
            return NULL;

        // insert enterprise group root
        CWizCategoryViewBizGroupRootItem* pItem = new CWizCategoryViewBizGroupRootItem(m_app, strName, "");
        insertTopLevelItem(indexOfTopLevelItem(pEnterprise) + 1, pItem);
        pItem->setExpanded(true);

        return pItem;
    }
    //
    return NULL;
}
*/

CWizCategoryViewGroupRootItem* CWizCategoryView::findGroup(const QString& strKbGUID)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        CWizCategoryViewGroupsRootItem* p = dynamic_cast<CWizCategoryViewGroupsRootItem *>(topLevelItem(i));

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

CWizCategoryViewTrashItem* CWizCategoryView::findTrash(const QString& strKbGUID /* = NULL */)
{
    // personal notes' trash, should exist
    if(strKbGUID.isEmpty() || strKbGUID == m_dbMgr.db().kbGUID()) {
        CWizCategoryViewItemBase* pItem = findAllFolderItem();
        if (!pItem) {
            Q_ASSERT(0);
            return NULL;
        }

        for (int i = 0; i < pItem->childCount(); i++) {
            if (CWizCategoryViewTrashItem* pTrash = dynamic_cast<CWizCategoryViewTrashItem*>(pItem->child(i))) {
                return pTrash;
            }
        }

        return NULL;
    }

    // group's trash, also should exist
    CWizCategoryViewGroupRootItem* pGroupItem = findGroup(strKbGUID);
    if (!pGroupItem) {
        return NULL;
    }

    for (int i = 0; i < pGroupItem->childCount(); i++) {
        CWizCategoryViewTrashItem * pTrashItem = dynamic_cast<CWizCategoryViewTrashItem *>(pGroupItem->child(i));
        if (pTrashItem) {
            return pTrashItem;
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

CWizCategoryViewFolderItem* CWizCategoryView::findFolder(const QString& strLocation, bool create, bool sort)
{
    CWizCategoryViewAllFoldersItem* pAllFolders = dynamic_cast<CWizCategoryViewAllFoldersItem *>(findAllFolderItem());
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
        if (sort) {
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

CWizCategoryViewTagItem* CWizCategoryView::findTagInTree(const WIZTAGDATA& tag)
{
    CWizCategoryViewAllTagsItem* pAllTags = dynamic_cast<CWizCategoryViewAllTagsItem*>(findAllTagsItem());
    if (!pAllTags)
        return NULL;

    return findTagInTree(tag, pAllTags);
}

CWizCategoryViewTagItem* CWizCategoryView::findTagInTree(const WIZTAGDATA& tag,
                                                         QTreeWidgetItem* itemParent)
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


CWizCategoryViewTagItem* CWizCategoryView::findTag(const WIZTAGDATA& tag, bool create, bool sort)
{
    Q_ASSERT(tag.strKbGUID == m_dbMgr.db().kbGUID());

    CWizStdStringArray arrayGUID;
    if (!m_dbMgr.db().GetAllParentsTagGUID(tag.strGUID, arrayGUID))
        return NULL;

    arrayGUID.insert(arrayGUID.begin(), tag.strGUID);   //insert self

    CWizCategoryViewAllTagsItem* pAllTags = dynamic_cast<CWizCategoryViewAllTagsItem*>(findAllTagsItem());
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
        if (sort) {
            parent->sortChildren(0, Qt::AscendingOrder);
        }

        parent = pTagItem;
    }

    return dynamic_cast<CWizCategoryViewTagItem *>(parent);
}



CWizCategoryViewTagItem* CWizCategoryView::addTagWithChildren(const WIZTAGDATA& tag)
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

CWizCategoryViewTagItem* CWizCategoryView::addTag(const WIZTAGDATA& tag, bool sort)
{
    return findTag(tag, true, sort);
}

CWizCategoryViewTagItem* CWizCategoryView::addAndSelectTag(const WIZTAGDATA& tag)
{
    if (CWizCategoryViewTagItem* pItem = addTag(tag, true)) {
        setCurrentItem(pItem);
        return pItem;
    }

    Q_ASSERT(0);
    return NULL;
}

void CWizCategoryView::removeTag(const WIZTAGDATA& tag)
{
    CWizCategoryViewTagItem* pItem = findTagInTree(tag);
    if (pItem) {
        QTreeWidgetItem* parent = pItem->parent();
        if (parent) {
            parent->removeChild(pItem);
        }
    }
}

CWizCategoryViewGroupItem* CWizCategoryView::findGroupFolderInTree(const WIZTAGDATA& tag)
{
    CWizCategoryViewGroupRootItem* pItem = findGroup(tag.strKbGUID);
    if (!pItem)
        return NULL;

    return findGroupFolderInTree(tag, pItem);
}

CWizCategoryViewGroupItem* CWizCategoryView::findGroupFolderInTree(const WIZTAGDATA& tag,
                                                                         QTreeWidgetItem* itemParent)
{
    for (int i = 0; i < itemParent->childCount(); i++) {
        QTreeWidgetItem* it = itemParent->child(i);

        if (CWizCategoryViewGroupItem* item = dynamic_cast<CWizCategoryViewGroupItem*>(it)) {
            if (item && item->tag().strGUID == tag.strGUID
                    && item->tag().strKbGUID == tag.strKbGUID)
                return item;
        }

        if (CWizCategoryViewGroupItem* childItem = findGroupFolderInTree(tag, it)) {
            return childItem;
        }
    }

    return NULL;
}


CWizCategoryViewGroupItem* CWizCategoryView::findGroupFolder(const WIZTAGDATA& tag,
                                                                   bool create,
                                                                   bool sort)
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
        if (sort) {
            parent->sortChildren(0, Qt::AscendingOrder);
        }

        parent = pTagItem;
    }

    return dynamic_cast<CWizCategoryViewGroupItem *>(parent);
}


CWizCategoryViewGroupItem* CWizCategoryView::addGroupFolderWithChildren(const WIZTAGDATA& tag)
{
    CWizCategoryViewGroupItem* pItem = findGroupFolder(tag, true, true);
    if (!pItem)
        return NULL;

    CWizTagDataArray arrayTag;
    m_dbMgr.db(tag.strKbGUID).GetChildTags(tag.strGUID, arrayTag);

    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        addGroupFolderWithChildren(*it);
    }

    return pItem;
}

void CWizCategoryView::removeGroupFolder(const WIZTAGDATA& tag)
{
    CWizCategoryViewGroupItem* pItem = findGroupFolderInTree(tag);
    if (pItem) {
        QTreeWidgetItem* parent = pItem->parent();
        if (parent) {
            parent->removeChild(pItem);
        }
    }
}

void CWizCategoryView::on_document_created(const WIZDOCUMENTDATA& doc)
{
    if (doc.strKbGUID == m_dbMgr.db().kbGUID() || doc.strKbGUID.isEmpty()) {
        // for backward compatibility
        if (!m_dbMgr.db().IsInDeletedItems(doc.strLocation)) {
            addFolder(doc.strLocation, true);
        }
        updatePrivateFolderDocumentCount();
        updatePrivateTagDocumentCount();
    }
    else {
        updateGroupFolderDocumentCount(doc.strKbGUID);
    }
}

void CWizCategoryView::on_document_modified(const WIZDOCUMENTDATA& docOld, const WIZDOCUMENTDATA& docNew)
{
    Q_UNUSED(docOld);

    if (docNew.strKbGUID == m_dbMgr.db().kbGUID() || docNew.strKbGUID.isEmpty()) {
        // for backward compatibility
        if (!m_dbMgr.db().IsInDeletedItems(docNew.strLocation)) {
            addFolder(docNew.strLocation, true);
        }

        updatePrivateFolderDocumentCount();
        updatePrivateTagDocumentCount();
    } else {
        updateGroupFolderDocumentCount(docNew.strKbGUID);
    }
}

void CWizCategoryView::on_document_deleted(const WIZDOCUMENTDATA& doc)
{
    Q_UNUSED(doc);

    if (doc.strKbGUID == m_dbMgr.db().kbGUID() || doc.strKbGUID.isEmpty()) {
        updatePrivateFolderDocumentCount();
        updatePrivateTagDocumentCount();
    } else {
        updateGroupFolderDocumentCount(doc.strKbGUID);
    }
}

void CWizCategoryView::on_document_tag_modified(const WIZDOCUMENTDATA& doc)
{
//    updateTagDocumentCount(doc.strKbGUID);
    Q_UNUSED (doc)
    updatePrivateTagDocumentCount();
}

void CWizCategoryView::on_folder_created(const QString& strLocation)
{
    Q_ASSERT(!strLocation.isEmpty());

    addFolder(strLocation, true);
}

void CWizCategoryView::on_folder_deleted(const QString& strLocation)
{
    Q_ASSERT(!strLocation.isEmpty());

    if (CWizCategoryViewFolderItem* pFolder = findFolder(strLocation, false, false))
    {
        if (QTreeWidgetItem* parent = pFolder->parent())
        {
            parent->removeChild(pFolder);
        }
    }
}

void CWizCategoryView::on_folder_positionChanged()
{
    sortFolders();
}

void CWizCategoryView::on_tag_created(const WIZTAGDATA& tag)
{
    if (tag.strKbGUID == m_dbMgr.db().kbGUID()) {
        addTagWithChildren(tag);
        updatePrivateTagDocumentCount();
    } else {
        CWizCategoryViewGroupItem* pTagItem = addGroupFolderWithChildren(tag);
        if (pTagItem) {
            pTagItem->parent()->sortChildren(0, Qt::AscendingOrder);
            updateGroupFolderDocumentCount(tag.strKbGUID);
        }
    }
}

void CWizCategoryView::on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew)
{
    if (tagNew.strKbGUID == m_dbMgr.db().kbGUID()) {
        if (tagOld.strParentGUID != tagNew.strParentGUID) {
            removeTag(tagOld);
        }

        CWizCategoryViewTagItem* pTagItem = addTagWithChildren(tagNew);
        if (pTagItem) {
            pTagItem->reload(m_dbMgr.db());
        }

        updatePrivateTagDocumentCount();
    } else {
        if (tagOld.strParentGUID != tagNew.strParentGUID) {
            removeGroupFolder(tagOld);
        }

        CWizCategoryViewGroupItem* pTagItem = addGroupFolderWithChildren(tagNew);
        if (pTagItem) {
            pTagItem->reload(m_dbMgr.db(tagNew.strKbGUID));
            pTagItem->parent()->sortChildren(0, Qt::AscendingOrder);
        }
        updateGroupFolderDocumentCount(tagNew.strKbGUID);
    }
}

void CWizCategoryView::on_tag_deleted(const WIZTAGDATA& tag)
{
    if (tag.strKbGUID == m_dbMgr.db().kbGUID()) {
        removeTag(tag);
        updatePrivateTagDocumentCount();
    } else {
        removeGroupFolder(tag);
        updateGroupFolderDocumentCount(tag.strKbGUID);
    }
}

void CWizCategoryView::on_group_opened(const QString& strKbGUID)
{
    Q_ASSERT(!strKbGUID.isEmpty());

    bool itemCreated = false;
    initGroup(m_dbMgr.db(strKbGUID), itemCreated);
    //
    if (itemCreated)
    {
        CWizCategoryViewGroupRootItem* pItem = findGroup(strKbGUID);
        if (pItem) {
            QTreeWidgetItem* parent = pItem->parent();
            if (parent) {
                if (parent->childCount() >= 2) {
                    parent->sortChildren(0, Qt::AscendingOrder);
                }
            }
        }
    }
}

void CWizCategoryView::on_group_closed(const QString& strKbGUID)
{
    Q_ASSERT(!strKbGUID.isEmpty());

    CWizCategoryViewGroupRootItem* pItem = findGroup(strKbGUID);
    if (pItem) {
        QTreeWidgetItem* parent = pItem->parent();
        if (parent) {
            parent->removeChild(pItem);
        }
    }
}

void CWizCategoryView::on_group_renamed(const QString& strKbGUID)
{
    Q_ASSERT(!strKbGUID.isEmpty());

    CWizCategoryViewGroupRootItem* pItem = findGroup(strKbGUID);
    if (pItem) {
        pItem->reload(m_dbMgr.db(strKbGUID));
    }
}

QAction* CWizCategoryView::findAction(CategoryActions type)
{
    QList<QAction *> actionList = actions();

    QList<QAction *>::const_iterator it;
    for (it = actionList.begin(); it != actionList.end(); it++) {
        QAction* action = *it;
        if (action->data() == type) {
            return action;
        }
    }

    Q_ASSERT(0);
    return NULL;
}

void CWizCategoryView::on_group_permissionChanged(const QString& strKbGUID)
{
    // only reset action if item is selected
    CWizCategoryViewItemBase* pItem = currentCategoryItem<CWizCategoryViewItemBase>();
    if (!pItem)
        return;

    if (pItem->kbGUID() != strKbGUID)
        return;

    int nPerm = m_dbMgr.db(strKbGUID).permission();

    // only Admin and Super user see trash folder and operate with tag (group folder)
    if (nPerm > WIZ_USERGROUP_SUPER) {
        CWizCategoryViewTrashItem* pItem =  findTrash(strKbGUID);
        if (pItem) pItem->setHidden(true);

        findAction(ActionNewItem)->setEnabled(false);
        findAction(ActionRenameItem)->setEnabled(false);
        findAction(ActionDeleteItem)->setEnabled(false);
    } else {
        CWizCategoryViewTrashItem* pItem = findTrash(strKbGUID);
        if (pItem) pItem->setHidden(false);

        findAction(ActionNewItem)->setEnabled(true);
        findAction(ActionRenameItem)->setEnabled(true);
        findAction(ActionDeleteItem)->setEnabled(true);
    }

    // permission greater than author can create new document
    if (nPerm >= WIZ_USERGROUP_READER) {
        findAction(ActionNewDocument)->setEnabled(false);
    } else {
        findAction(ActionNewDocument)->setEnabled(true);
    }
}

void CWizCategoryView::on_group_bizChanged(const QString& strKbGUID)
{
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


#define TREEVIEW_STATE "TreeState"
#define TREEVIEW_SELECTED_ITEM "SelectedItemID"

void CWizCategoryView::loadState()
{
    QSettings* settings = ExtensionSystem::PluginManager::settings();
    m_strSelectedId = selectedId(settings);

    for (int i = 0 ; i < topLevelItemCount(); i++) {
        loadChildState(topLevelItem(i), settings);
    }
}

void CWizCategoryView::loadChildState(QTreeWidgetItem* pItem, QSettings* settings)
{
    loadItemState(pItem, settings);

    if (!m_strSelectedId.isEmpty()) {
        CWizCategoryViewItemBase* pi = dynamic_cast<CWizCategoryViewItemBase*>(pItem);
        Q_ASSERT(pi);
        if (pi->id() == m_strSelectedId) {
            setCurrentItem(pItem);
        }
    }

    for (int i = 0; i < pItem->childCount(); i++) {
        loadChildState(pItem->child(i), settings);
    }
}

void CWizCategoryView::loadItemState(QTreeWidgetItem* pi, QSettings* settings)
{
    if (!pi || !settings)
        return;

    CWizCategoryViewItemBase* pItem = dynamic_cast<CWizCategoryViewItemBase*>(pi);
    Q_ASSERT(pItem);

    QString strId = pItem->id();
    settings->beginGroup(TREEVIEW_STATE);
    bool bExpand = settings->value(strId).toBool();
    settings->endGroup();

    if (bExpand)
        expandItem(pItem);
    else
        collapseItem(pItem);
}

QString CWizCategoryView::selectedId(QSettings* settings)
{
    settings->beginGroup(TREEVIEW_STATE);
    QString strItem = settings->value(TREEVIEW_SELECTED_ITEM).toString();
    settings->endGroup();

    return strItem;
}

void CWizCategoryView::saveState()
{
    QSettings* settings = ExtensionSystem::PluginManager::settings();
    for (int i = 0 ; i < topLevelItemCount(); i++) {
        saveChildState(topLevelItem(i), settings);
    }

    saveSelected(settings);

    settings->sync();
}

void CWizCategoryView::saveChildState(QTreeWidgetItem* pItem, QSettings* settings)
{
    saveItemState(pItem, settings);

    for (int i = 0; i < pItem->childCount(); i++) {
        saveChildState(pItem->child(i), settings);
    }
}

void CWizCategoryView::saveItemState(QTreeWidgetItem* pi, QSettings *settings)
{
   if (!pi || !settings)
       return;

   CWizCategoryViewItemBase* pItem = dynamic_cast<CWizCategoryViewItemBase*>(pi);
   Q_ASSERT(pItem);

   QString strId = pItem->id();
   bool bExpand = pItem->isExpanded() ? true : false;

   settings->beginGroup(TREEVIEW_STATE);
   settings->setValue(strId, bExpand);
   settings->endGroup();
}

void CWizCategoryView::saveSelected(QSettings* settings)
{
    if (!settings)
        return;
    //
    QTreeWidgetItem *curr = currentItem();
    if (!curr)
        return;

    CWizCategoryViewItemBase* pItem = dynamic_cast<CWizCategoryViewItemBase*>(curr);
    if (!pItem)
        return;

    settings->beginGroup(TREEVIEW_STATE);
    settings->setValue(TREEVIEW_SELECTED_ITEM, pItem->id());
    settings->endGroup();
}
