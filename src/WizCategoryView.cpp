#include "WizCategoryView.h"

#include <QHeaderView>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QApplication>
#include <QTimer>
#include <QFileDialog>
#include <QtXml>

#include "WizDef.h"
#include "utils/WizStyleHelper.h"
#include "utils/WizMisc.h"
#include "utils/WizLogger.h"
#include "share/WizDrawTextHelper.h"
#include "share/WizSettings.h"
#include "share/WizDatabaseManager.h"
#include "share/WizSearch.h"
#include "share/WizAnalyzer.h"
#include "share/WizObjectOperator.h"
#include "share/WizMessageBox.h"
#include "share/WizThreads.h"
#include "share/WizGlobal.h"
#include "share/WizDatabase.h"
#include "sync/WizKMSync.h"
#include "sync/WizKMServer.h"
#include "sync/WizApiEntry.h"
#include "sync/WizToken.h"
#include "widgets/WizScrollBar.h"
#include "widgets/WizAdvancedSearchDialog.h"
#include "widgets/WizExecutingActionDialog.h"
#include "mac/WizMacHelper.h"
#include "core/WizNoteManager.h"
#include "WizMainWindow.h"
#include "WizProgressDialog.h"
#include "WizNoteStyle.h"
#include "WizFolderSelector.h"
#include "WizLineInputDialog.h"
#include "WizWebSettingsDialog.h"
#include "WizFileImporter.h"
#include "WizOEMSettings.h"
#include "share/jsoncpp/json/json.h"


#define CATEGORY_GENERAL    QObject::tr("General")
#define CATEGORY_TEAM_GROUPS QObject::tr("Team Notes")
#define CATEGORY_SHORTCUTS  QObject::tr("Shortcuts")
#define CATEGORY_SEARCH     QObject::tr("Quick Search")
#define CATEGORY_MYSHARES   QObject::tr("Shared Notes")
#define CATEGORY_FOLDERS    QObject::tr("Folders")
#define CATEGORY_TAGS       QObject::tr("Tags")
#define CATEGORY_STYLES     QObject::tr("Styles")

#define CATEGORY_SEARCH_BYCUSTOM  QObject::tr("Search by Custom Param")

// for context menu text
#define CATEGORY_ACTION_DOCUMENT_NEW    QObject::tr("New Note")
#define CATEGORY_ACTION_DOCUMENT_LOAD   QObject::tr("Load Note")
#define CATEGORY_ACTION_IMPORT_FILE   QObject::tr("Import File...")
#define CATEGORY_ACTION_FOLDER_NEW      QObject::tr("New Folder...")
#define CATEGORY_ACTION_FOLDER_COPY     QObject::tr("Copy to...")
#define CATEGORY_ACTION_FOLDER_MOVE     QObject::tr("Move to...")
#define CATEGORY_ACTION_FOLDER_RENAME   QObject::tr("Rename...")
#define CATEGORY_ACTION_FOLDER_DELETE   QObject::tr("Delete")
#define CATEGORY_ACTION_TAG_NEW         QObject::tr("New Tag...")
#define CATEGORY_ACTION_TAG_RENAME      QObject::tr("Rename...")
#define CATEGORY_ACTION_TAG_DELETE      QObject::tr("Delete")
#define CATEGORY_ACTION_GROUP_ATTRIBUTE QObject::tr("View Group Info...")
#define CATEGORY_ACTION_BIZ_GROUP_ATTRIBUTE QObject::tr("View Team Info...")
#define CATEGORY_ACTION_GROUP_MARK_READ QObject::tr("Mark All as Readed")
#define CATEGORY_ACTION_EMPTY_TRASH     QObject::tr("Empty Deleted Items")
#define CATEGORY_ACTION_MANAGE_GROUP     QObject::tr("Manage Group...")
#define CATEGORY_ACTION_MANAGE_BIZ     QObject::tr("Manage Team...")
#define CATEGORY_ACTION_QUIT_GROUP     QObject::tr("Quit Troup")
#define CATEGORY_ACTION_REMOVE_SHORTCUT     QObject::tr("Remove from Shortcuts")
#define CATEGORY_ACTION_RECOVERY_DELETED_NOTES    QObject::tr("Recovery Deleted Notes...")

#define LINK_COMMAND_ID_CREATE_GROUP        100

#define TREEVIEW_STATE "TreeState"
#define TREEVIEW_SELECTED_ITEM "SelectedItemID"
#define SHORTCUT_STATE "ShortcutState"

#define CATEGORY_META   "CategoryMeta"

#define QUICK_SEARCH_META   "CUSTOM_QUICK_SEARCH"


#define SHORTCUT_TYPE_FOLDER            "/Type=folder"
#define SHORTCUT_TYPE_DOCUMENT      "/Type=document"
#define SHORTCUT_TYPE_TAG                   "/Type=tag"
#define SHORTCUT_PARAM_LOCATION     "/Location="
#define SHORTCUT_PARAM_KBGUID          "/KbGUID="
#define SHORTCUT_PARAM_TAGGUID        "/TagGUID="
#define SHORTCUT_PARAM_DOCUMENTGUID     "/DocumentGUID="


/* ------------------------------ CWizCategoryBaseView ------------------------------ */

WizCategoryBaseView::WizCategoryBaseView(WizExplorerApp& app, QWidget* parent)
    : QTreeWidget(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_selectedItem(NULL)
    , m_bDragHovered(false)
    , m_cursorEntered(false)
    , m_dragUrls(false)
    , m_dragHoveredTimer(new QTimer())
    , m_dragItem(NULL)
    , m_dragHoveredItem(0)
{
    // basic features
    header()->hide();
    setAnimated(true);
    setFrameStyle(QFrame::NoFrame);
    viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    setAttribute(Qt::WA_MacShowFocusRect, false);   
    setTextElideMode(Qt::ElideMiddle);
    setIndentation(22);
    setCursor(Qt::ArrowCursor);
    //
    setMouseTracking(true);

    // scrollbar        ScrollPerPixel could cause drag and drop problem    
//    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
#ifdef Q_OS_MAC
    verticalScrollBar()->setSingleStep(10);
#else
    verticalScrollBar()->setSingleStep(30);
#endif

#ifdef WIZNOTE_CUSTOM_SCROLLBAR
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_vScroll = new WizScrollBar(this);
    m_vScroll->syncWith(verticalScrollBar());
#endif

    // style
    setStyle(::WizGetStyle(m_app.userSettings().skin()));
    QColor colorBg = Utils::WizStyleHelper::treeViewBackground();
    QPalette pal = palette();
    colorBg.setAlpha(200);
    pal.setBrush(QPalette::Base, colorBg);
    setPalette(pal);
    setStyleSheet("background-color: transparent;");
    setAutoFillBackground(true);

    // signals from database
    connect(&m_dbMgr, SIGNAL(documentCreated(const WIZDOCUMENTDATA&)),
            SLOT(on_document_created(const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(documentModified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)),
            SLOT(on_document_modified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(documentDeleted(const WIZDOCUMENTDATA&)),
            SLOT(on_document_deleted(const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(documentTagModified(const WIZDOCUMENTDATA&)),
            SLOT(on_document_tag_modified(const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(groupDocumentUnreadCountModified(QString)),
            SLOT(on_groupDocuments_unreadCount_modified(QString)));

    connect(&m_dbMgr, SIGNAL(folderCreated(const QString&)),
            SLOT(on_folder_created(const QString&)));

    connect(&m_dbMgr, SIGNAL(folderDeleted(const QString&)),
            SLOT(on_folder_deleted(const QString&)));

    connect(&m_dbMgr, SIGNAL(folderPositionChanged()),
            SLOT(on_folder_positionChanged()));

    connect(&m_dbMgr, SIGNAL(tagsPositionChanged(const QString&)),
            SLOT(on_tags_positionChanged(const QString&)));

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

    connect(m_dragHoveredTimer, SIGNAL(timeout()), SLOT(on_dragHovered_timeOut()));

}

WizCategoryBaseView::~WizCategoryBaseView()
{
    disconnect();
    //
    if (!m_dragHoveredTimer) {
        m_dragHoveredTimer->stop();
        delete m_dragHoveredTimer;
        m_dragHoveredTimer = 0;
    }
}

void WizCategoryBaseView::mousePressEvent(QMouseEvent* event)
{
    // saved for child item which need test hit position.
    m_hitPos = event->pos();

    QTreeWidget::mousePressEvent(event);

    if (WizCategoryViewItemBase* item = itemAt(event->pos()))
    {
       if (item->acceptMousePressedInfo())
       {
           item->mousePressed(event->pos());
           update();
       }
    }
}

void WizCategoryBaseView::mouseReleaseEvent(QMouseEvent* event)
{
    QTreeWidget::mouseReleaseEvent(event);

    if (WizCategoryViewItemBase* item = itemAt(event->pos()))
    {
       if (item->acceptMousePressedInfo())
       {
           item->mouseReleased(event->pos());
           update();
       }
    }

}

void WizCategoryBaseView::mouseMoveEvent(QMouseEvent* event)
{    
    QPoint msPos = event->pos();
    WizCategoryViewItemBase* pItem =  itemAt(msPos);
    if (!pItem)
        return;

    QRect rcExtra = pItem->getExtraButtonRect(visualItemRect(pItem));
    if (rcExtra.contains(msPos))
    {
        if (cursor().shape() != Qt::PointingHandCursor)
        {
            setCursor(Qt::PointingHandCursor);
            setToolTip(pItem->getExtraButtonToolTip());
        }
    }
    else
    {
        if (cursor().shape() != Qt::ArrowCursor)
        {
            setCursor(Qt::ArrowCursor);
            setToolTip("");
        }
    }

    QTreeWidget::mouseMoveEvent(event);
}

void WizCategoryBaseView::resizeEvent(QResizeEvent* event)
{
#ifdef WIZNOTE_CUSTOM_SCROLLBAR
    // reset scrollbar
    m_vScroll->resize(m_vScroll->sizeHint().width(), event->size().height());
    m_vScroll->move(event->size().width() - m_vScroll->sizeHint().width(), 0);
#endif

    QTreeWidget::resizeEvent(event);
}

void WizCategoryBaseView::contextMenuEvent(QContextMenuEvent * e)
{
    m_selectedItem = nullptr;
    WizCategoryViewItemBase* pItem = dynamic_cast<WizCategoryViewItemBase*>(itemAt(e->pos()));
    if (pItem) {
        pItem->showContextMenu(this, mapToGlobal(e->pos()));
    }
}

void WizCategoryBaseView::startDrag(Qt::DropActions supportedActions)
{
    if (m_app.userSettings().isManualSortingEnabled())
    {
        m_dragItem = currentCategoryItem<WizCategoryViewItemBase>();
        Q_ASSERT(m_dragItem);
        if (!m_dragItem->dragAble() || !m_dbMgr.db(m_dragItem->kbGUID()).isGroupSuper())
        {
            m_dragItem = nullptr;
            return;
        }

        resetRootItemsDropEnabled(m_dragItem);
        QTreeWidget::startDrag(supportedActions);
        if (m_dragItem != nullptr)
        {
//            blockSignals(true);
            //setCurrentItem(m_dragItem);
//            blockSignals(false);
            m_dragItem = nullptr;
        }

        ::WizGetAnalyzer().logAction("categoryDragItem");
        //
        viewport()->repaint();
    }

}

void WizCategoryBaseView::dragEnterEvent(QDragEnterEvent *event)
{
    m_bDragHovered = true;
    repaint();

    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS))
    {
        event->acceptProposedAction();
        event->accept();
    }
    else if (event->mimeData()->hasUrls())
    {
        m_dragUrls = true;
        event->acceptProposedAction();
        event->accept();
    }
    else
    {
        if (m_dragItem)
            event->acceptProposedAction();

        QTreeWidget::dragEnterEvent(event);
    }
}

void WizCategoryBaseView::dragMoveEvent(QDragMoveEvent *event)
{
    m_dragHoveredPos = event->pos();

#ifdef Q_OS_MAC
    //osx10.11系统上自动滚动存在问题，通过判断进行强制滚动
    static bool isEiCapitan = (getSystemMinorVersion() >= 11);
    if (isEiCapitan)
    {
        QTreeWidgetItem* hoverItem = itemAt(event->pos());
        QRect rcVisual = viewport()->rect();
        QTreeWidgetItem* aboveItem = itemAbove(hoverItem);
        QTreeWidgetItem* belowItem = itemBelow(hoverItem);
        if (aboveItem && visualItemRect(aboveItem).top() < rcVisual.top())
        {
            scrollToItem(aboveItem);
        }
        if (belowItem && visualItemRect(belowItem).bottom() > rcVisual.bottom())
        {
            scrollToItem(belowItem);
        }
    }
#endif

    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS) )
    {
        WizCategoryViewItemBase* pItem = itemAt(event->pos());
        if (!pItem)
            return;

        if (m_dragHoveredItem != pItem) {
            m_dragHoveredTimer->stop();
            m_dragHoveredItem = pItem;
            m_dragHoveredTimer->start(1000);
        }

        m_dragDocArray.clear();
        WizMime2Note(event->mimeData()->data(WIZNOTE_MIMEFORMAT_DOCUMENTS), m_dbMgr, m_dragDocArray);

        if (!m_dragDocArray.size())
            return;

        int nAccept = 0;
        for (CWizDocumentDataArray::const_iterator it = m_dragDocArray.begin();
             it != m_dragDocArray.end();
             it++)
        {
            if (pItem->acceptDrop(*it)) {
                nAccept++;
            }
        }

        if (nAccept == m_dragDocArray.size()) {
            event->acceptProposedAction();
        }
        else
            event->ignore();

    }
    else if(event->mimeData()->hasUrls())
    {
        m_dragUrls = true;
        event->acceptProposedAction();
    }
    else if (m_dragItem)
    {
        if (WizCategoryViewItemBase* pItem = itemAt(event->pos()))
        {
            if (pItem->acceptDrop(m_dragItem))
            {
                pItem->setFlags(pItem->flags() | Qt::ItemIsDropEnabled);
                m_dragItem->setFlags(m_dragItem->flags() | Qt::ItemIsDropEnabled);                
            }
            else
            {
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsDropEnabled);
                m_dragItem->setFlags(m_dragItem->flags() & ~Qt::ItemIsDropEnabled);
            }

            event->acceptProposedAction();
            QTreeWidget::dragMoveEvent(event);
        }
    }
    repaint();
    viewport()->repaint();    
}

void WizCategoryBaseView::dragLeaveEvent(QDragLeaveEvent* event)
{       
    m_bDragHovered = false;
    m_dragHoveredPos = QPoint();
    m_dragHoveredTimer->stop();
    m_dragHoveredItem = 0;
    m_dragUrls = false;

    m_dragDocArray.clear();
    QTreeWidget::dragLeaveEvent(event);
    viewport()->repaint();    
}




QAbstractItemView::DropIndicatorPosition WizCategoryBaseView::position(const QPoint &pos, const QRect &rect, const QModelIndex &index) const
{
    QAbstractItemView::DropIndicatorPosition r = QAbstractItemView::OnViewport;
    const int margin = 4;
    if (pos.y() - rect.top() < margin) {
        r = QAbstractItemView::AboveItem;
    } else if (rect.bottom() - pos.y() < margin) {
        r = QAbstractItemView::BelowItem;
    } else if (rect.contains(pos, true)) {
        r = QAbstractItemView::OnItem;
    }

    if (r == QAbstractItemView::OnItem && (!(model()->flags(index) & Qt::ItemIsDropEnabled)))
        r = pos.y() < rect.center().y() ? QAbstractItemView::AboveItem : QAbstractItemView::BelowItem;

    return r;
}

bool WizCategoryBaseView::droppingOnItself(QDropEvent *event, const QModelIndex &index)
{
    Qt::DropAction dropAction = event->dropAction();
    if (dragDropMode() == QAbstractItemView::InternalMove)
        dropAction = Qt::MoveAction;
    if (event->source() == this
        && event->possibleActions() & Qt::MoveAction
        && dropAction == Qt::MoveAction) {
        QModelIndexList selected = selectedIndexes();
        QModelIndex child = index;
        while (child.isValid() && child != rootIndex()) {
            if (selected.contains(child))
                return true;
            child = child.parent();
        }
    }
    return false;
}

bool WizCategoryBaseView::dropOn(QDropEvent *event, int *dropRow, int *dropCol, QModelIndex *dropIndex)
{
    if (event->isAccepted())
        return false;

    QModelIndex index;
    // rootIndex() (i.e. the viewport) might be a valid index
    if (viewport()->rect().contains(event->pos())) {
        index = indexAt(event->pos());
        if (!index.isValid())
        {
            index = rootIndex();
        }
        //don't reset index
        /*
        else {
            QRect rc = visualRect(index);
            QPoint pt = event->pos();
            if (!rc.contains(pt)) {
                index = rootIndex();
            }
        }
        */
    }
    else
    {
        return false;
    }

    QAbstractItemView::DropIndicatorPosition dropIndicatorPosition;
    int row = -1;
    int col = -1;
    if (index != rootIndex()) {
        dropIndicatorPosition = position(event->pos(), visualRect(index), index);
        switch (dropIndicatorPosition) {
        case QAbstractItemView::AboveItem:
            row = index.row();
            col = index.column();
            index = index.parent();
            break;
        case QAbstractItemView::BelowItem:
            row = index.row() + 1;
            col = index.column();
            index = index.parent();
            break;
        case QAbstractItemView::OnItem:
        case QAbstractItemView::OnViewport:
            break;
        }
    } else {
        dropIndicatorPosition = QAbstractItemView::OnViewport;
    }
    *dropIndex = index;
    *dropRow = row;
    *dropCol = col;
    //
    if (!droppingOnItself(event, index))
        return true;
    //
    return false;
}




QTreeWidgetItem* WizCategoryBaseView::dropEventCore(QDropEvent *event)
{
    if (event->source() == this && (event->dropAction() == Qt::MoveAction ||
                                    dragDropMode() == QAbstractItemView::InternalMove)) {
        QModelIndex topIndex;
        int col = -1;
        int row = -1;
        if (dropOn(event, &row, &col, &topIndex)) {
            const QList<QModelIndex> idxs = selectedIndexes();
            QList<QPersistentModelIndex> indexes;
            const int indexesCount = idxs.count();
            indexes.reserve(indexesCount);
            for (const auto &idx : idxs)
                indexes.append(idx);

            if (indexes.contains(topIndex))
                return nullptr;

            // When removing items the drop location could shift
            QPersistentModelIndex dropRow = model()->index(row, col, topIndex);

            QTreeWidgetItem* oldParent = nullptr;
            // Remove the items
            QList<QTreeWidgetItem *> taken;
            for (const auto &index : indexes) {
                QTreeWidgetItem *item = itemFromIndex(index);
                if (item && item->parent()) {
                    QTreeWidgetItem *itemTaken = item->parent()->child(index.row());
                    if (WizCategoryViewItemBase* base = dynamic_cast<WizCategoryViewItemBase*>(itemTaken)) {
                        base->setWillBeDeleted(true);
                    }
                    oldParent = itemTaken->parent();
                    qDebug() << item->text(0) << "==" << itemTaken->text(0);
                    QTreeWidgetItem* copied = itemTaken->clone();
                    copied->addChildren(itemTaken->takeChildren());
                    taken.append(copied);
                }
            }


            QTreeWidgetItem* result = nullptr;
            // insert them back in at their new positions
            for (int i = 0; i < indexes.count(); ++i) {
                // Either at a specific point or appended
                QTreeWidgetItem *itemInsert = taken.takeFirst();
                result = itemInsert;
                qDebug() << itemInsert->text(0);
                //
                if (row == -1) {
                    //
                    //直接移动作为子目录，不需要排序
                    if (topIndex.isValid()) {
                        //
                        QTreeWidgetItem *parent = itemFromIndex(topIndex);
                        if (parent == oldParent) {
                            return nullptr;
                        }
                        //
                        qDebug() << parent->text(0);
                        parent->insertChild(parent->childCount(), itemInsert);
                    }
                } else {
                    //
                    //移动并且排序
                    int r = dropRow.row() >= 0 ? dropRow.row() : row;
                    if (topIndex.isValid()) {
                        QTreeWidgetItem *parent = itemFromIndex(topIndex);
                        qDebug() << parent->text(0);
                        parent->insertChild(qMin(r, parent->childCount()), itemInsert);
                    }
                }
            }
            //
            event->accept();
            return result;
        }
    }

    QTreeView::dropEvent(event);
    return nullptr;
}

void WizCategoryBaseView::dropEvent(QDropEvent * event)
{
    m_bDragHovered = false;
    m_dragHoveredPos = QPoint();
    m_dragUrls = false;
    m_dragDocArray.clear();

    WizCategoryViewItemBase* pItem = itemAt(event->pos());
    if (!pItem)
        return;

    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS)) {
        ::WizGetAnalyzer().logAction("categoryDropDocument");
        CWizDocumentDataArray arrayDocument;
        WizMime2Note(event->mimeData()->data(WIZNOTE_MIMEFORMAT_DOCUMENTS), m_dbMgr, arrayDocument);

        if (!arrayDocument.size())
            return;


        Qt::KeyboardModifiers keyMod = QApplication::keyboardModifiers();
        bool forceCopy = keyMod.testFlag(Qt::AltModifier);

        pItem->drop(arrayDocument, forceCopy);

    } else if (event->mimeData()->hasUrls()) {
        ::WizGetAnalyzer().logAction("categoryDropFiles");
        if (!pItem->acceptDrop(""))
            return;

        setCurrentItem(pItem);
        QList<QUrl> urls = event->mimeData()->urls();
        QStringList strFileList;
        foreach (QUrl url, urls) {
            strFileList.append(url.path());
        }
        //
        importFiles(strFileList);
    }
    else
    {
        if (WizKMSyncThread::isBusy())
        {
            QString title = QObject::tr("Syncing");
            QString message = QObject::tr("WizNote is synchronizing notes, please wait for the synchronization to complete before the operation.");  \
            QMessageBox::information(this, title, message);
            event->ignore();
            return;
        }
        //
        if (m_dragItem && !(m_dragItem->flags() & Qt::ItemIsDropEnabled))
        {
            qDebug() << "[DragDrop]Can not drop item at invalid position";
            return;
        }

        ::WizGetAnalyzer().logAction("categoryDropItem");

        QModelIndex droppedIndex = indexAt(event->pos());
        if( !droppedIndex.isValid() )
          return;
        //
        WizCategoryViewItemBase* hoverItem = itemAt(event->pos());
        if (!hoverItem)
        {
            qDebug() << "null item";
        }
        else
        {
            qDebug() << "hover item: " << hoverItem->text(0);
        }

        if (pItem->type() == Category_ShortcutRootItem || pItem->type() == Category_TagItem)
        {
            pItem->drop(m_dragItem);
            setCurrentItem(m_dragItem);
            event->ignore();
        }
        else
        {
            QTreeWidgetItem* newItem = dropEventCore(event);
            if (newItem && event->isAccepted())
            {
                on_itemPosition_changed(dynamic_cast<WizCategoryViewItemBase*>(newItem));
            }
        }
        viewport()->repaint();
        return;
    }

    viewport()->repaint();
    event->accept();
}

void WizCategoryBaseView::enterEvent(QEvent* event)
{
    m_cursorEntered = true;
    QTreeWidget::enterEvent(event);

    update();
}

void WizCategoryBaseView::leaveEvent(QEvent* event)
{
    m_cursorEntered = false;
    QTreeWidget::leaveEvent(event);

    update();
}

void WizCategoryBaseView::importFiles(QStringList &/*strFileList*/)
{
    Q_ASSERT(0);
}

QString WizCategoryBaseView::selectedItemKbGUID()
{
    QList<QTreeWidgetItem*> items = selectedItems();
    if (!items.isEmpty()) {
        WizCategoryViewItemBase* pItem = dynamic_cast<WizCategoryViewItemBase*>(items.first());
        if (!pItem)
            return QString();

        return pItem->kbGUID();
    }

    return QString();
}

QString WizCategoryBaseView::storedSelectedItemKbGuid()
{
    if (!m_selectedItem)
        return selectedItemKbGUID();
    //
    WizCategoryViewItemBase* pItem = dynamic_cast<WizCategoryViewItemBase*>(m_selectedItem);
    if (!pItem)
        return QString();

    return pItem->kbGUID();
}


void WizCategoryBaseView::getDocuments(CWizDocumentDataArray& arrayDocument)
{
    QList<QTreeWidgetItem*> items = selectedItems();
    if (items.empty())
        return;

    WizCategoryViewItemBase* pItem = dynamic_cast<WizCategoryViewItemBase*>(items.first());
    if (!pItem)
        return;

    pItem->getDocuments(m_dbMgr.db(pItem->kbGUID()), arrayDocument);
}

bool WizCategoryBaseView::acceptDocument(const WIZDOCUMENTDATA& document)
{
    QList<QTreeWidgetItem*> items = selectedItems();
    if (items.empty())
        return false;

    WizCategoryViewItemBase* pItem = dynamic_cast<WizCategoryViewItemBase*>(items.first());
    if (!pItem)
        return false;

    return pItem->accept(m_dbMgr.db(document.strKbGUID), document);
}

bool WizCategoryView::setCurrentIndex(const WIZDOCUMENTDATA& document)
{
    if (m_dbMgr.db().kbGUID() == document.strKbGUID)
    {
        addAndSelectFolder(document.strLocation);
        return true;
    }
    else
    {
        QTreeWidgetItem* pItem = itemFromKbGUID(document.strKbGUID);
        if (pItem)
        {
            WizDatabase& db = m_dbMgr.db(document.strKbGUID);
            CWizTagDataArray arrayTag;
            if (!db.getDocumentTags(document.strGUID, arrayTag)) {
                return false;
            } else {
                if (arrayTag.size() > 1) {
                    TOLOG1("Group document should only have one tag: %1", document.strTitle);
                }

                QString tagText;
                if (arrayTag.size()) {
                    tagText = db.getTagTreeText(arrayTag[0].strGUID);
                }

                CString strTempLocation = tagText;
                strTempLocation.trim('/');
                QStringList sl = strTempLocation.split("/");
                for (int i = 0; i < sl.count(); i ++)
                {
                    for (int j = 0; j < pItem->childCount(); j ++)
                    {
                        if (pItem->child(j)->text(0) == sl.at(i))
                        {
                            pItem = pItem->child(j);
                            break;
                        }
                    }
                }
            }

            setCurrentItem(pItem);
            return true;
        }
    }

    return false;
}

void WizCategoryBaseView::saveSelection()
{
    QTreeWidgetItem* item = currentItem();
    if (item)
    {
        m_selectedItem = item;
    }
    //
    clearSelection();
}

void WizCategoryBaseView::restoreSelection()
{
    if (!m_selectedItem) {
        return;
    }

    setCurrentItem(m_selectedItem);
    m_selectedItem = NULL;

    //Q_EMIT itemSelectionChanged();
}

WizCategoryViewItemBase* WizCategoryBaseView::itemAt(const QPoint& p) const
{
    return dynamic_cast<WizCategoryViewItemBase*>(QTreeWidget::itemAt(p));
}

bool findItemByKbGUID(QTreeWidgetItem *item, const QString& strKbGUID, WizCategoryViewItemBase*& target)
{
    for( int i = 0; i < item->childCount(); ++i)
    {
        WizCategoryViewItemBase * pItem = dynamic_cast<WizCategoryViewItemBase *>(item->child(i));
        if (pItem->kbGUID() == strKbGUID)
        {
            target = pItem;
            return true;
        }

        if (findItemByKbGUID(item->child(i), strKbGUID, target))
        {
            return true;
        }
    }

    return false;
}

WizCategoryViewItemBase* WizCategoryBaseView::itemFromKbGUID(const QString &strKbGUID) const
{
    if (strKbGUID.isEmpty())
        return 0;

    WizCategoryViewItemBase * item = 0;
    for (int i = 0; i < topLevelItemCount(); i ++)
    {
        findItemByKbGUID(topLevelItem(i), strKbGUID, item);
    }

    return item;
}


WizCategoryViewItemBase* WizCategoryBaseView::categoryItemFromIndex(const QModelIndex &index) const
{
    return dynamic_cast<WizCategoryViewItemBase*>(itemFromIndex(index));
}

QModelIndex WizCategoryBaseView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    QModelIndex index = QTreeWidget::moveCursor(cursorAction, modifiers);
    if (!index.isValid())
        return index;

    WizCategoryViewItemBase* pItem = categoryItemFromIndex(index);
    if (WizCategoryViewSectionItem* pSeparatorItem = dynamic_cast<WizCategoryViewSectionItem*>(pItem))
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
            //Q_ASSERT(false);
            break;
        }
    }

    return index;
}

void WizCategoryBaseView::resetRootItemsDropEnabled(WizCategoryViewItemBase* pItem)
{
    int topCount = topLevelItemCount();
    for (int i = 0; i < topCount; i++)
    {
        WizCategoryViewItemBase* pi = dynamic_cast<WizCategoryViewItemBase*>(pItem);
        if (pi->acceptDrop(pItem))
            pi->setFlags(pi->flags() | Qt::ItemIsDropEnabled);
        else
            pi->setFlags(pi->flags() & ~Qt::ItemIsDropEnabled);
    }
    update();
}

QString WizCategoryView::getUseableItemName(QTreeWidgetItem* parent, \
                                                  QTreeWidgetItem* item)
{
    QString name = item->text(0);
    int nRepeat = 0;
    while (true)
    {
        if (nRepeat > 0)
        {
            name = item->text(0) + "_" + QString::number(nRepeat);
        }
        bool continueLoop = false;
        for (int i = 0; i < parent->childCount(); i++)
        {
            QTreeWidgetItem* brother = parent->child(i);
            if (brother != item && brother->text(0) == name)
            {
                nRepeat ++;
                continueLoop = true;
                break;
            }
        }
        if (!continueLoop)
            break;
    }
    return name;
}


bool WizCategoryView::renameFolder(WizCategoryViewFolderItem* item, const QString& strFolderName)
{
    if (strFolderName.isEmpty() || item == nullptr)
        return false;

    CString validName = strFolderName;
    WizMakeValidFileNameNoPath(validName);

    QString strLocation;
    QString strOldLocation = item->location();
    int n = strOldLocation.lastIndexOf("/", -2);
    strLocation = strOldLocation.left(n + 1) + validName + "/";

    qDebug() << "rename folder from : " << strOldLocation << "  to : " << strLocation;

    if (strLocation == strOldLocation)
        return true;

    CWizStdStringArray arrayLocation;
    m_dbMgr.db().getAllLocationsWithExtra(arrayLocation);
    if (std::find(arrayLocation.begin(), arrayLocation.end(), strLocation) != arrayLocation.end())
    {
        if (WizMessageBox::question(0, tr("Info"), tr("Folder '%1' already exists, combine these folders?").arg(validName)) == QMessageBox::No)
        {
            validName = getUseableItemName(item->parent(), item);
            item->setText(0, validName);
            strLocation = strOldLocation.left(n + 1) + validName + "/";
            if (strLocation == strOldLocation)
                return true;
        }
    }

    // move all documents to new folder
    WizFolder folder(m_dbMgr.db(), strOldLocation);
    connect(&folder, SIGNAL(moveDocument(int, int, const QString&, const QString&, const WIZDOCUMENTDATA&)),
            SLOT(on_action_user_renameFolder_confirmed_progress(int, int, const QString&, const QString&, const WIZDOCUMENTDATA&)));

    folder.moveToLocation(strLocation);
    return true;
}

bool WizCategoryView::renameGroupFolder(WizCategoryViewGroupItem* pGroup, const QString& strFolderName)
{
    WIZTAGDATA tag = pGroup->tag();

    QTreeWidgetItem* sameNameBrother = findSameNameBrother(pGroup->parent(), pGroup, strFolderName);

    WizDatabase& db = m_dbMgr.db(tag.strKbGUID);
    tag.strName = strFolderName;
    if (sameNameBrother != nullptr)
    {
        if (WizMessageBox::question(0, tr("Info"), tr("Folder '%1' already exists, combine these folders?").arg(strFolderName)) == QMessageBox::Yes)
        {
            // move documents to brother folder  and move child folders to brother folder
            WIZTAGDATA targetTag;
            if (sameNameBrother->type() == Category_GroupNoTagItem)
            {
                targetTag.strKbGUID = pGroup->kbGUID();
            }
            else
            {
                WizCategoryViewGroupItem* targetItem = dynamic_cast<WizCategoryViewGroupItem*>(sameNameBrother);
                targetTag = targetItem->tag();
            }
            //
            CWizDocumentDataArray arrayDocument;
            if (db.getDocumentsByTag(pGroup->tag(), arrayDocument))
            {
                moveDocumentsToGroupFolder(arrayDocument, targetTag);
            }
            CWizTagDataArray arrayTag;
            if (db.getChildTags(pGroup->tag().strGUID, arrayTag))
            {
                for (WIZTAGDATA tag : arrayTag)
                {
                   tag.strParentGUID = targetTag.strGUID;
                   db.modifyTag(tag);
                }
            }
            db.deleteTag(pGroup->tag(), true);

            return true;
        }
        else
        {
            QString useableName = getUseableItemName(pGroup->parent(), pGroup);
            tag.strName = useableName;
        }
    }
    db.modifyTag(tag);
    pGroup->reload(db);
    return true;
}

void WizCategoryView::updateShortcut(int type, const QString& keyValue, const QString& name)
{
    WizCategoryViewItemBase* shortcutRoot = findAllShortcutItem();
    if (shortcutRoot)
    {
        WizCategoryViewShortcutItem::ShortcutType shortcutType = (WizCategoryViewShortcutItem::ShortcutType)type;
        for (int i = 0; i < shortcutRoot->childCount(); i++)
        {
            WizCategoryViewShortcutItem* item = dynamic_cast<WizCategoryViewShortcutItem*>(shortcutRoot->child(i));
            if (item && item->shortcutType() == shortcutType)
            {
                if (shortcutType == WizCategoryViewShortcutItem::PersonalFolder)
                {
                    if (item->location() == keyValue)
                    {
                        item->setText(0, name);
                        break;
                    }
                }
                else
                {
                    if (item->guid() == keyValue)
                    {
                        item->setText(0, name);
                        break;
                    }
                }
            }
        }
    }
}

void WizCategoryView::removeShortcut(int type, const QString& keyValue)
{
    WizCategoryViewItemBase* shortcutRoot = findAllShortcutItem();
    if (shortcutRoot)
    {
        WizCategoryViewShortcutItem::ShortcutType shortcutType = (WizCategoryViewShortcutItem::ShortcutType)type;
        for (int i = 0; i < shortcutRoot->childCount(); i++)
        {
            WizCategoryViewShortcutItem* item = dynamic_cast<WizCategoryViewShortcutItem*>(shortcutRoot->child(i));
            if (item && item->shortcutType() == shortcutType)
            {
                if (shortcutType == WizCategoryViewShortcutItem::PersonalFolder)
                {
                    if (item->location() == keyValue)
                    {
                        removeShortcut(item);
                        break;
                    }
                }
                else
                {
                    if (item->guid() == keyValue)
                    {
                        removeShortcut(item);
                        break;
                    }
                }
            }
        }
    }
}

void WizCategoryView::removeShortcut(WizCategoryViewItemBase* shortcut)
{
    WizCategoryViewShortcutRootItem *pRoot = dynamic_cast<WizCategoryViewShortcutRootItem *>(shortcut->parent());
    if (shortcut && pRoot)
    {
        pRoot->removeChild(shortcut);
        if (pRoot->childCount() == 0)
        {
            pRoot->addPlaceHoldItem();
        }
    }
    saveShortcutState();
}

QTreeWidgetItem*WizCategoryView::findSameNameBrother(QTreeWidgetItem* parent, QTreeWidgetItem* exceptItem, const QString& name)
{
    if (parent)
    {
        for (int i = 0; i < parent->childCount(); i++)
        {
            if (parent->child(i) != exceptItem && parent->child(i)->text(0) == name)
            {
                return parent->child(i);
            }
        }
    }
    return nullptr;
}

bool WizCategoryView::isCombineSameNameFolder(const WIZTAGDATA& parentTag,
                                               const QString& folderName, bool& isCombine, QTreeWidgetItem* exceptBrother)
{
    WizCategoryViewItemBase* targetItem = nullptr;
    parentTag.strGUID.isEmpty() ? (targetItem = findGroup(parentTag.strKbGUID)) :  (targetItem = findGroupFolder(parentTag, false, false));
    QTreeWidgetItem* sameNameBrother = findSameNameBrother(targetItem, exceptBrother, folderName);
    if (sameNameBrother)
    {
        QMessageBox::StandardButton stb = WizMessageBox::question(m_app.mainWindow(), tr("Info"), tr("Folder '%1' already exists, combine these folders?").arg(folderName),
                                                                   (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel));
        if (stb == QMessageBox::Yes)
        {
            isCombine = true;
            return true;
        }
        else if (stb == QMessageBox::No)
        {
            isCombine = false;
            return true;
        }
        else
            return false;
    }
    return true;
}

bool WizCategoryView::isCombineSameNameFolder(const QString& parentFolder,
                                               const QString& folderName, bool& isCombine, QTreeWidgetItem* exceptBrother)
{
    WizCategoryViewItemBase* targetItem = findFolder(parentFolder, false, false);
    QTreeWidgetItem* sameNameBrother = findSameNameBrother(targetItem, exceptBrother, folderName);
    if (sameNameBrother)
    {
        QMessageBox::StandardButton stb = WizMessageBox::question(m_app.mainWindow(),
                                                                   tr("Info"), tr("Folder '%1' already exists, combine these folders?").arg(folderName),
                                                                   (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel));
        if (stb == QMessageBox::Yes)
        {
            isCombine = true;
            return true;
        }
        else if (stb == QMessageBox::No)
        {
            isCombine = false;
            return true;
        }
        else
            return false;
    }
    return true;
}

bool WizCategoryView::combineGroupFolder(WizCategoryViewGroupItem* sourceItem, WizCategoryViewGroupItem* targetItem)
{
    qDebug() << "merge group folder : " << sourceItem->text(0);
    WizDatabase& db = m_dbMgr.db(sourceItem->kbGUID());
    CWizDocumentDataArray arrayDocument;
    if (db.getDocumentsByTag(sourceItem->tag(), arrayDocument))
    {
        moveDocumentsToGroupFolder(arrayDocument, targetItem->tag());
    }
    CWizTagDataArray arrayTag;
    if (db.getChildTags(sourceItem->tag().strGUID, arrayTag))
    {
        for (WIZTAGDATA tag : arrayTag)
        {
           tag.strParentGUID = targetItem->tag().strGUID;
           db.modifyTag(tag);
        }
    }
    return db.deleteTag(sourceItem->tag(), true);
}

void WizCategoryBaseView::on_dragHovered_timeOut()
{
    if (m_dragHoveredItem) {
        m_dragHoveredTimer->stop();
        expandItem(m_dragHoveredItem);
        viewport()->repaint();
    }
}

bool WizCategoryBaseView::validateDropDestination(const QPoint& p) const
{
    if (p.isNull())
        return false;

    if (m_dragUrls)
    {
        if (WizCategoryViewItemBase* itemBase = itemAt(p))
        {
            return itemBase->acceptDrop("");
        }
    }

    if (m_dragDocArray.empty())
        return false;

    if (WizCategoryViewItemBase* itemBase = itemAt(p))
    {
        WIZDOCUMENTDATAEX data = *m_dragDocArray.begin();
        return (itemBase && itemBase->acceptDrop(data));
    }

    return false;
}

Qt::ItemFlags WizCategoryBaseView::dragItemFlags() const
{
    if (m_dragItem)
        return m_dragItem->flags();

    return Qt::NoItemFlags;
}

/* ------------------------------ CWizCategoryView ------------------------------ */
WizCategoryView::WizCategoryView(WizExplorerApp& app, QWidget* parent)
    : WizCategoryBaseView(app, parent)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(false);
    viewport()->setAcceptDrops(false);
    invisibleRootItem()->setFlags(invisibleRootItem()->flags() & ~Qt::ItemIsDropEnabled);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setContentsMargins(0, 8, 0, 0);
    initMenus();

    connect(this, SIGNAL(itemClicked(QTreeWidgetItem*, int)), SLOT(on_itemClicked(QTreeWidgetItem *, int)));
    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(on_itemChanged(QTreeWidgetItem*,int)));
    connect(this, SIGNAL(itemSelectionChanged()), SLOT(on_itemSelectionChanged()));
}

WizCategoryView::~WizCategoryView()
{
}

void WizCategoryView::initMenus()
{
    QAction* actionNewDoc = new QAction("ActionNewDocument", this);
    actionNewDoc->setShortcutContext(Qt::WidgetShortcut);
    actionNewDoc->setShortcut(QKeySequence::New);
    actionNewDoc->setData(ActionNewDocument);
    addAction(actionNewDoc);
    connect(actionNewDoc, SIGNAL(triggered()), SLOT(on_action_newDocument()));

//    QAction* actionLoadDoc = new QAction("ActionLoadDocument",this);
//    actionLoadDoc->setShortcutContext(Qt::WidgetShortcut);
//    actionLoadDoc->setShortcut(QKeySequence("Ctrl+Shift+L"));
//    actionLoadDoc->setData(ActionLoadDocument);
//    addAction(actionLoadDoc);
//    connect(actionLoadDoc,SIGNAL(triggered()),SLOT(on_action_loadDocument()));

    QAction* actionImportFile = new QAction("ActionImportFile",this);
    actionImportFile->setShortcutContext(Qt::WidgetShortcut);
    actionImportFile->setShortcut(QKeySequence("Ctrl+Shift+I"));
    actionImportFile->setData(ActionImportFile);
    addAction(actionImportFile);
    connect(actionImportFile,SIGNAL(triggered()),SLOT(on_action_importFile()));

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

    QAction* actionCopyItem = new QAction("ActionCopyItem", this);
    actionCopyItem->setShortcutContext(Qt::WidgetShortcut);
    actionCopyItem->setShortcut(QKeySequence("Ctrl+Shift+C"));
    actionCopyItem->setData(ActionCopyItem);
    addAction(actionCopyItem);
    connect(actionCopyItem, SIGNAL(triggered()), SLOT(on_action_copyItem()));

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
    actionTrash->setShortcut(QKeySequence("Ctrl+Shift+Delete"));
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

    QAction* actionAddToShortcuts = new QAction(tr("Add to Shortcuts"), this);
    actionAddToShortcuts->setData(ActionAddToShortcuts);
    addAction(actionAddToShortcuts);
    connect(actionAddToShortcuts, SIGNAL(triggered()), SLOT(on_action_addToShortcuts()));

    QAction* actionRemoveShortcut = new QAction("RemoveShortcut", this);
    actionRemoveShortcut->setText(CATEGORY_ACTION_REMOVE_SHORTCUT);
    actionRemoveShortcut->setData(ActionRemoveShortcutItem);
    actionRemoveShortcut->setShortcutContext(Qt::WidgetShortcut);
    addAction(actionRemoveShortcut);
    connect(actionRemoveShortcut, SIGNAL(triggered()), SLOT(on_action_removeShortcut()));

    QAction* actionAddCustomSearch = new QAction(tr("Add custom search"), this);
    actionAddCustomSearch->setData(ActionAddCustomSearch);
    addAction(actionAddCustomSearch);
    connect(actionAddCustomSearch, SIGNAL(triggered()), SLOT(on_action_addCustomSearch()));

    QAction* actionEditCustomSearch = new QAction(tr("Edit custom search"), this);
    actionEditCustomSearch->setData(ActionEditCustomSearch);
    addAction(actionEditCustomSearch);
    connect(actionEditCustomSearch, SIGNAL(triggered()), SLOT(on_action_editCustomSearch()));

    QAction* actionRemoveCustomSearch = new QAction(tr("Remove custom search"), this);
    actionRemoveCustomSearch->setData(ActionRemoveCustomSearch);
    addAction(actionRemoveCustomSearch);
    connect(actionRemoveCustomSearch, SIGNAL(triggered()), SLOT(on_action_removeCustomSearch()));


    // shortcut menu
    m_menuShortcut = std::make_shared<QMenu>();
    m_menuShortcut->addAction(actionRemoveShortcut);

    // custom search menu
    m_menuCustomSearch = std::make_shared<QMenu>();
    m_menuCustomSearch->addAction(actionAddCustomSearch);
    m_menuCustomSearch->addAction(actionEditCustomSearch);
    m_menuCustomSearch->addAction(actionRemoveCustomSearch);

    // trash menu
    m_menuTrash = std::make_shared<QMenu>();
    m_menuTrash->addAction(actionTrash);

    // folder root menu
    m_menuFolderRoot = std::make_shared<QMenu>();
    m_menuFolderRoot->addAction(actionNewItem);

    // folder menu
    m_menuFolder = std::make_shared<QMenu>();
    m_menuFolder->addAction(actionNewDoc);
    m_menuFolder->addAction(actionImportFile);
    m_menuFolder->addAction(actionNewItem);
    m_menuFolder->addSeparator();
    m_menuFolder->addAction(actionRenameItem);
    m_menuFolder->addAction(actionCopyItem);
    m_menuFolder->addAction(actionMoveItem);
    m_menuFolder->addAction(actionAddToShortcuts);
    m_menuFolder->addSeparator();
    m_menuFolder->addAction(actionDeleteItem);

    // tag root menu
    m_menuTagRoot = std::make_shared<QMenu>();
    m_menuTagRoot->addAction(actionNewItem);

    // tag menu
    m_menuTag = std::make_shared<QMenu>();
    m_menuTag->addAction(actionNewItem);
    m_menuTag->addAction(actionRenameItem);
    m_menuTag->addAction(actionAddToShortcuts);
    m_menuTag->addSeparator();
    m_menuTag->addAction(actionDeleteItem);

    // group root menu normal
    m_menuNormalGroupRoot = std::make_shared<QMenu>();
    m_menuNormalGroupRoot->addAction(actionNewDoc);
    m_menuNormalGroupRoot->addAction(actionImportFile);
    m_menuNormalGroupRoot->addAction(actionNewItem);
    m_menuNormalGroupRoot->addSeparator();
    m_menuNormalGroupRoot->addAction(actionItemAttr);
//    m_menuNormalGroupRoot->addAction(actionQuitGroup);

    // group root menu admin
    m_menuAdminGroupRoot = std::make_shared<QMenu>();
    m_menuAdminGroupRoot->addAction(actionNewDoc);
    m_menuAdminGroupRoot->addAction(actionImportFile);
    m_menuAdminGroupRoot->addAction(actionNewItem);
    m_menuAdminGroupRoot->addSeparator();
    m_menuAdminGroupRoot->addAction(actionManageGroup);
//    m_menuAdminGroupRoot->addAction(actionQuitGroup);

    // group root menu normal
    m_menuOwnerGroupRoot = std::make_shared<QMenu>();
    m_menuOwnerGroupRoot->addAction(actionNewDoc);
    m_menuOwnerGroupRoot->addAction(actionImportFile);
    m_menuOwnerGroupRoot->addAction(actionNewItem);
    m_menuOwnerGroupRoot->addSeparator();
    m_menuOwnerGroupRoot->addAction(actionManageGroup);

    //biz group root menu normal
    m_menuNormalBizGroupRoot = std::make_shared<QMenu>();
    m_menuNormalBizGroupRoot->addAction(actionItemAttr);

    //biz group root menu admin
    m_menuAdminBizGroupRoot = std::make_shared<QMenu>();
    m_menuAdminBizGroupRoot->addAction(actionManageBiz);


    // group menu
    m_menuGroup = std::make_shared<QMenu>();
    m_menuGroup->addAction(actionNewDoc);
    m_menuGroup->addAction(actionImportFile);
    m_menuGroup->addAction(actionNewItem);
    m_menuGroup->addAction(actionRenameItem);
    m_menuGroup->addAction(actionCopyItem);
    m_menuGroup->addAction(actionMoveItem);
    m_menuGroup->addAction(actionAddToShortcuts);
    m_menuGroup->addSeparator();
    m_menuGroup->addAction(actionDeleteItem);
}

void WizCategoryView::setActionsEnabled(bool enable)
{
    QList<QAction*> acts = actions();

    for (int i = 0; i < acts.size(); i++) {
        QAction* act = acts.at(i);
        act->setEnabled(enable);
    }
}

void WizCategoryView::resetMenu(CategoryMenuType type)
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
        case ActionLoadDocument:
            if(type==FolderItem || type == GroupRootItem || type == GroupItem) {
                act->setText(CATEGORY_ACTION_DOCUMENT_LOAD);
            }
            break;
        case ActionImportFile:
            if(type==FolderItem || type == GroupRootItem || type == GroupItem) {
                act->setText(CATEGORY_ACTION_IMPORT_FILE);
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
        case ActionCopyItem:
            if (type == FolderItem|| type == GroupItem) {
                act->setText(CATEGORY_ACTION_FOLDER_COPY);
            }
            break;
        case ActionMoveItem:
            if (type == FolderItem|| type == GroupItem) {
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
        case ActionRemoveCustomSearch:
        case ActionEditCustomSearch:
            if (type == AddCustomSearchItem) {
                act->setVisible(false);
            } else if (type == EditCustomSearchItem) {
                act->setVisible(true);
            }
            break;
        default:
            continue;
        }
    }
}

void WizCategoryView::showTrashContextMenu(QPoint pos)
{
    resetMenu(TrashItem);
    m_menuTrash->popup(pos);
}

void WizCategoryView::showShortcutContextMenu(QPoint pos)
{
    resetMenu(ShortcutItem);
    m_menuShortcut->popup(pos);
}

void WizCategoryView::showCustomSearchContextMenu(QPoint pos, bool removable)
{
    if (removable)
    {
        resetMenu(EditCustomSearchItem);
    }
    else
    {
        resetMenu(AddCustomSearchItem);
    }
    m_menuCustomSearch->popup(pos);
}

QString sectionToText(CategorySection section)
{
    QString strKey = "";
    switch (section) {
    case Section_MessageCenter:
        strKey = "MessageCenter";
        break;
    case Section_Shortcuts:
        strKey = "Shortcuts";
        break;
    case Section_QuickSearch:
        strKey = "QuickSearch";
        break;
    case Section_Folders:
        strKey = "Folders";
        break;
    case Section_Tags:
        strKey = "Tags";
        break;
    case Section_BizGroups:
        strKey = "BizGroups";
        break;
    case Section_PersonalGroups:
        strKey = "PersonalGroup";
        break;
    }

    return strKey;
}

bool WizCategoryView::setSectionVisible(CategorySection section, bool visible)
{
    QString strKey = sectionToText(section);
    if (strKey.isEmpty())
        return false;

    QString optionXML = m_dbMgr.db().meta("CategoryViewOption", "SectionVisible");
    QDomDocument doc;
    doc.setContent(optionXML);

    QDomNodeList nodeList = doc.elementsByTagName(strKey);    
    if (nodeList.count() > 0)
    {
        nodeList.at(0).toElement().firstChild().setNodeValue(visible ? "true" : "false");
    }
    else
    {
        QDomElement root = doc.firstChild().toElement();
        if (!root.isElement() || root.tagName() != "root")
        {
            root = doc.createElement("root");
            doc.appendChild(root);
        }
        QDomElement elem = doc.createElement(strKey);
        root.appendChild(elem);
        QDomText t = doc.createTextNode(visible ? "true" : "false");
        elem.appendChild(t);
    }

    optionXML = doc.toString();
    m_dbMgr.db().setMeta("CategoryViewOption", "SectionVisible", optionXML);

    loadSectionStatus();
    return true;
}

bool WizCategoryView::isSectionVisible(CategorySection section) const
{
    QString strKey = sectionToText(section);
    if (strKey.isEmpty())
        return false;

    QString optionXML = m_dbMgr.db().meta("CategoryViewOption", "SectionVisible");
    QDomDocument doc;
    doc.setContent(optionXML);

    QString nodeValue;
    QDomNodeList nodeList = doc.elementsByTagName(strKey);
    if (nodeList.count() > 0)
    {
        nodeValue = nodeList.at(0).toElement().text();
    }
    else
    {
        return true;
    }

    return nodeValue == "true";
}

void setItemVisible(const QString& strXML, CategorySection section, QTreeWidget* treeWidget,
                    QTreeWidgetItem* item)
{
    if (!treeWidget || !item)
        return;

    QDomDocument doc;
    doc.setContent(strXML);
    bool sectionVisible;
    QString strKey = sectionToText(section);
    QDomNodeList nodeList = doc.elementsByTagName(strKey);

    if (nodeList.count() > 0)
    {
        sectionVisible = nodeList.at(0).toElement().firstChild().nodeValue() == "true";
    }
    else
    {
        sectionVisible = true;
    }
    treeWidget->setItemHidden(item, !sectionVisible);
}

void hideSectionItem(QTreeWidget* treewidget)
{
    if (!treewidget)
        return;

    QTreeWidgetItem* lastSectionItem = nullptr;
    bool sectionAreaHiden = true;
    for (int i = 0; i < treewidget->topLevelItemCount(); i++)
    {
        if (treewidget->topLevelItem(i)->type() == Category_SectionItem)
        {
            if (lastSectionItem != nullptr)
            {
                lastSectionItem->setHidden(sectionAreaHiden);
            }
            sectionAreaHiden = true;
            lastSectionItem = treewidget->topLevelItem(i);
        }
        else
        {
            if (!treewidget->topLevelItem(i)->isHidden())
            {
                sectionAreaHiden = false;
            }
        }
    }

    if (lastSectionItem != nullptr)
    {
        lastSectionItem->setHidden(sectionAreaHiden);
    }
}

void WizCategoryView::loadSectionStatus()
{
    QString optionXML = m_dbMgr.db().meta("CategoryViewOption", "SectionVisible");
    if (optionXML.isEmpty())
        return;

    QTreeWidgetItem* item = findAllMessagesItem();
    setItemVisible(optionXML, Section_MessageCenter, this, item);

    //
    item = findAllShortcutItem();
    setItemVisible(optionXML, Section_Shortcuts, this, item);

    //
    item = findAllSearchItem();
    setItemVisible(optionXML, Section_QuickSearch, this, item);    

    //
    item = findAllFolderItem();
    setItemVisible(optionXML, Section_Folders, this, item);

    //
    item = findAllTagsItem();
    setItemVisible(optionXML, Section_Tags, this, item);

    CWizGroupDataArray arrayGroup;
    m_dbMgr.db().getAllGroupInfo(arrayGroup);

    CWizBizDataArray arrayBiz;
    m_dbMgr.db().getAllBizInfo(arrayBiz);

    for (WIZBIZDATA biz : arrayBiz)
    {
        item = findBizGroupsRootItem(biz, false);
        setItemVisible(optionXML, Section_BizGroups, this, item);
    }

    for (WIZGROUPDATA group : arrayGroup)
    {
        if (!group.isBiz())
        {
            item = findGroupsRootItem(group, false);
            setItemVisible(optionXML, Section_PersonalGroups, this, item);
        }
    }

    hideSectionItem(this);
}

WizCategoryViewItemBase*WizCategoryView::findFolder(const WIZDOCUMENTDATA& doc)
{
    WizDatabase& db = m_dbMgr.db(doc.strKbGUID);
    if (db.isGroup())
    {
        CWizTagDataArray arrayTag;
        db.getDocumentTags(doc.strGUID,arrayTag);

        if (arrayTag.size() == 0)
        {
            return findGroup(db.kbGUID());
        }
        else
        {
            WIZTAGDATA tag = *arrayTag.begin();
            return findGroupFolder(tag, false, false);
        }
    }
    else
    {
        return findFolder(doc.strLocation, false, false);
    }
    return nullptr;
}

void WizCategoryView::showFolderRootContextMenu(QPoint pos)
{
    resetMenu(FolderRootItem);
    m_menuFolderRoot->popup(pos);
}

void WizCategoryView::showFolderContextMenu(QPoint pos)
{
    resetMenu(FolderItem);
    m_menuFolder->popup(pos);
}

void WizCategoryView::showTagRootContextMenu(QPoint pos)
{
    resetMenu(TagRootItem);
    m_menuTagRoot->popup(pos);
}

void WizCategoryView::showTagContextMenu(QPoint pos)
{
    resetMenu(TagItem);
    m_menuTag->popup(pos);
}

void WizCategoryView::showNormalGroupRootContextMenu(QPoint pos)
{
    resetMenu(GroupRootItem);
    m_menuNormalGroupRoot->popup(pos);
}

void WizCategoryView::showAdminGroupRootContextMenu(QPoint pos)
{
    resetMenu(GroupRootItem);
    m_menuAdminGroupRoot->popup(pos);
}

void WizCategoryView::showOwnerGroupRootContextMenu(QPoint pos)
{
    resetMenu(GroupRootItem);
    m_menuOwnerGroupRoot->popup(pos);
}

void WizCategoryView::showNormalBizGroupRootContextMenu(QPoint pos)
{
    resetMenu(BizGroupRootItem);
    m_menuNormalBizGroupRoot->popup(pos);
}

void WizCategoryView::showAdminBizGroupRootContextMenu(QPoint pos, bool usable)
{
    resetMenu(BizGroupRootItem);

    const QList<QAction*>& acts = m_menuAdminBizGroupRoot->actions();
    foreach (QAction* act,  acts) {
        act->setEnabled(usable);
    }

    m_menuAdminBizGroupRoot->popup(pos);
}

void WizCategoryView::showGroupContextMenu(QPoint pos)
{
    resetMenu(GroupItem);
    m_menuGroup->popup(pos);
}

bool WizCategoryView::createDocument(WIZDOCUMENTDATA& data, const QString& strHtml, const QString& strTitle)
{
    QString strKbGUID = m_dbMgr.db().kbGUID();
    QString strLocation = m_dbMgr.db().getDefaultNoteLocation();
    WIZTAGDATA tag;

    if (getAvailableNewNoteTagAndLocation(strKbGUID, tag, strLocation))
    {
        if (!m_dbMgr.db(strKbGUID).createDocumentAndInit(strHtml, "", 0, strTitle, "newnote", strLocation, "", data))
        {
            TOLOG("Failed to new document!");
            return false;
        }

        if (!tag.strGUID.isEmpty()) {
            WizDocument doc(m_dbMgr.db(strKbGUID), data);
            doc.addTag(tag);
        }
    }

    quickSyncNewDocument(data.strKbGUID);
    //
    return true;
}

bool WizCategoryView::createDocumentByAttachments(WIZDOCUMENTDATA& data, const QStringList& attachList)
{
    if (attachList.isEmpty())
        return false;

    QString strTitle =Utils::WizMisc::extractFileName(attachList.first());
    if (!createDocument(data, "<div><br/></div>", strTitle))
        return false;

    WizDatabase& db = m_dbMgr.db(data.strKbGUID);
    foreach (QString strFileName, attachList)
    {
        WIZDOCUMENTATTACHMENTDATA attach;
        if (!db.addAttachment(data, strFileName, attach))
        {
            qWarning() << "add attachment failed , " << strFileName;
        }
    }

    return true;
}

void WizCategoryView::on_action_newDocument()
{
    ::WizGetAnalyzer().logAction("categoryMenuNewDocument");
    if (currentCategoryItem<WizCategoryViewFolderItem>()
            || currentCategoryItem<WizCategoryViewGroupRootItem>()
            || currentCategoryItem<WizCategoryViewGroupItem>())
    {
        // delegate create action to mainwindow
        Q_EMIT newDocument();
    }
}

void WizCategoryView::on_action_loadDocument()
{
    ::WizGetAnalyzer().logAction("categoryMenuLoadDocument");
    //TODO:
}

void WizCategoryView::on_action_importFile()
{
    ::WizGetAnalyzer().logAction("categoryMenuImportFile");
    QStringList files = QFileDialog::getOpenFileNames(
    this,
    tr("Select one or more files to open"),
    QDir::homePath(),
#ifdef Q_OS_LINUX
    "All files(*.*);;Text files(*.txt *.md *.html *.htm *.cpp *.h *.c *.hpp *.cpp);;Images (*.png *.xpm *.jpg *.jpeg *.svg)");
#else
    "All files(*.*);;Text files(*.txt *.md *.html *.htm *.cpp *.h *.rtf *.doc *.docx *.pages);;Images (*.png *.xpm *.jpg *.jpeg *.svg);;Webarchive (*.webarchive)");
#endif
    importFiles(files);
}

void WizCategoryView::on_action_newItem()
{    
    if (currentCategoryItem<WizCategoryViewAllFoldersItem>()
            || currentCategoryItem<WizCategoryViewFolderItem>())
    {
        on_action_user_newFolder();
    }
    else if (currentCategoryItem<WizCategoryViewAllTagsItem>()
             || currentCategoryItem<WizCategoryViewTagItem>())
    {
        on_action_user_newTag();
    }
    else if (currentCategoryItem<WizCategoryViewGroupRootItem>()
             || currentCategoryItem<WizCategoryViewGroupItem>())
    {
        on_action_group_newFolder();
    }
}

void WizCategoryView::on_action_user_newFolder()
{
    ::WizGetAnalyzer().logAction("categoryMenuNewFolder");
    WizLineInputDialog* dialog = new WizLineInputDialog(tr("New folder"),
                                                          tr("Please input folder name: "),
                                                          "", m_app.mainWindow());      //use mainWindow as parent

    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_user_newFolder_confirmed(int)));
    connect(dialog, SIGNAL(textChanged(QString)), SLOT(on_newFolder_inputText_changed(QString)));

    dialog->exec();

}

void WizCategoryView::on_newFolder_inputText_changed(const QString& text)
{
    if (WizLineInputDialog* dialog = qobject_cast<WizLineInputDialog*>(sender()))
    {
        if (!WizIsValidFileNameNoPath(text))
        {
            dialog->setErrorMessage(tr("Invalid folder name"));
            dialog->setOKButtonEnable(false);
            return;
        }

        QString strLocation;
        if (currentCategoryItem<WizCategoryViewAllFoldersItem>())
        {
            strLocation = "/" + text + "/";
        }
        else if (WizCategoryViewFolderItem* p = currentCategoryItem<WizCategoryViewFolderItem>())
        {
            strLocation = p->location() + text + "/";
        }

        if (m_dbMgr.db().isFolderExists(strLocation))
        {
            dialog->setOKButtonEnable(false);
            dialog->setErrorMessage(tr("Folder has already exists"));
            return;
        }

        dialog->setOKButtonEnable(true);
        dialog->setErrorMessage(tr(""));

    }
}

void WizCategoryView::on_action_user_newFolder_confirmed(int result)
{
    WizLineInputDialog* dialog = qobject_cast<WizLineInputDialog*>(sender());
    CString strFolderName = dialog->input();
    dialog->deleteLater();

    if (result != QDialog::Accepted) {
        return;
    }

    if (strFolderName.isEmpty())
        return;

    WizMakeValidFileNameNoPath(strFolderName);

    QString strLocation;

    if (currentCategoryItem<WizCategoryViewAllFoldersItem>()) {
        strLocation = "/" + strFolderName + "/";
        //moveFolderPostionBeforeTrash(strLocation);
    } else if (WizCategoryViewFolderItem* p = currentCategoryItem<WizCategoryViewFolderItem>()) {
        strLocation = p->location() + strFolderName + "/";
    }

    addAndSelectFolder(strLocation);
    sortFolders();
    m_dbMgr.db().addExtraFolder(strLocation);
    m_dbMgr.db().setLocalValueVersion("folders", -1);
}

void WizCategoryView::on_action_user_newTag()
{
    ::WizGetAnalyzer().logAction("categoryMenuNewTag");
    WizLineInputDialog* dialog = new WizLineInputDialog(tr("New tag"),
                                                          tr("Please input tag name: "),
                                                          "", window());
    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_user_newTag_confirmed(int)));
    connect(dialog, SIGNAL(textChanged(QString)), SLOT(on_newTag_inputText_changed(QString)));

    dialog->exec();
}

void WizCategoryView::on_action_user_newTag_confirmed(int result)
{
    WizLineInputDialog* dialog = qobject_cast<WizLineInputDialog*>(sender());
    QString strTagNames = dialog->input();
    dialog->deleteLater();

    if (result != QDialog::Accepted) {
        return;
    }

    if (strTagNames.isEmpty())
        return;

    WIZTAGDATA parentTag;

    if (WizCategoryViewTagItem* p = currentCategoryItem<WizCategoryViewTagItem>()) {
        parentTag = p->tag();
    }

    QStringList sl = strTagNames.split(';');
    QStringList::const_iterator it;
    for (it = sl.begin(); it != sl.end(); it++) {
        CString strTagName = *it;

        CWizTagDataArray arrayTag;
        bool nextName = false;
        if (m_dbMgr.db().tagByName(strTagName, arrayTag)) {
            for (WIZTAGDATA tagNew : arrayTag) {
                if (tagNew.strParentGUID == parentTag.strGUID) {
                    TOLOG1("Tag name already exist: %1", strTagName);
                    nextName = true;
                    break;
                }
            }
        }

        if (nextName)
            continue;

        WIZTAGDATA tagNew;        
        m_dbMgr.db().createTag(parentTag.strGUID, strTagName, "", tagNew);
    }
}


void WizCategoryView::on_newTag_inputText_changed(const QString& text)
{
    WIZTAGDATA parentTag;

    if (WizCategoryViewTagItem* p = currentCategoryItem<WizCategoryViewTagItem>()) {
        parentTag = p->tag();
    }

    if (WizLineInputDialog* dialog = qobject_cast<WizLineInputDialog*>(sender()))
    {
        QString strTagNames = text;
        QStringList sl = strTagNames.split(';');
        QStringList::const_iterator it;
        for (it = sl.begin(); it != sl.end(); it++) {
            CString strTagName = *it;

            CWizTagDataArray arrayTag;
            if (m_dbMgr.db().tagByName(strTagName, arrayTag)) {
                for (WIZTAGDATA tagNew : arrayTag) {
                    if (tagNew.strParentGUID == parentTag.strGUID) {
                        dialog->setErrorMessage(tr("Tag has already exists"));
                        dialog->setOKButtonEnable(false);
                        return;

                    }
                }
            }
        }

        dialog->setOKButtonEnable(true);
        dialog->setErrorMessage(tr(""));
    }
}

void WizCategoryView::on_action_group_newFolder()
{
    ::WizGetAnalyzer().logAction("categoryMenuNewGroupFolder");
    WizLineInputDialog* dialog = new WizLineInputDialog(tr("New group folder"),
                                                          tr("Please input folder name: "),
                                                          "", window());

    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_group_newFolder_confirmed(int)));
    connect(dialog, SIGNAL(textChanged(QString)), SLOT(on_group_newFolder_inputText_changed(QString)));

    dialog->exec();
}

void WizCategoryView::on_action_group_newFolder_confirmed(int result)
{
    WizLineInputDialog* dialog = qobject_cast<WizLineInputDialog*>(sender());
    QString strTagNames = dialog->input();
    dialog->deleteLater();

    if (result != QDialog::Accepted) {
        return;
    }

    if (strTagNames.isEmpty())
        return;

    WIZTAGDATA parentTag;
    QString strKbGUID;

    if (WizCategoryViewGroupRootItem* pRoot = currentCategoryItem<WizCategoryViewGroupRootItem>()) {
        strKbGUID = pRoot->kbGUID();
    }

    if (WizCategoryViewGroupItem* p = currentCategoryItem<WizCategoryViewGroupItem>()) {
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
        m_dbMgr.db(strKbGUID).createTag(parentTag.strGUID, strTagName, "", tagNew);
    }
}


void WizCategoryView::on_group_newFolder_inputText_changed(const QString& text)
{
    WIZTAGDATA parentTag;
    QString strKbGUID;

    if (WizCategoryViewGroupRootItem* pRoot = currentCategoryItem<WizCategoryViewGroupRootItem>()) {
        strKbGUID = pRoot->kbGUID();
    }

    if (WizCategoryViewGroupItem* p = currentCategoryItem<WizCategoryViewGroupItem>()) {
        strKbGUID = p->kbGUID();
        parentTag = p->tag();
    }

    if (strKbGUID.isEmpty()) {
        Q_ASSERT(0);
        return;
    }

    if (WizLineInputDialog* dialog = qobject_cast<WizLineInputDialog*>(sender()))
    {
        QString strTagNames = text;
        QStringList sl = strTagNames.split(';');
        QStringList::const_iterator it;
        for (it = sl.begin(); it != sl.end(); it++) {
            CString strTagName = *it;

            CWizTagDataArray arrayTag;
            if (m_dbMgr.db(strKbGUID).tagByName(strTagName, arrayTag)) {
                for (WIZTAGDATA tagNew : arrayTag) {
                    if (tagNew.strParentGUID == parentTag.strGUID) {
                        dialog->setErrorMessage(tr("Folder has already exists"));
                        dialog->setOKButtonEnable(false);
                        return;

                    }
                }
            }
        }

        dialog->setOKButtonEnable(true);
        dialog->setErrorMessage(tr(""));
    }
}


void WizCategoryView::on_action_moveItem()
{
//    if (currentCategoryItem<CWizCategoryViewFolderItem>()) {
    if (currentItem()->type() == Category_FolderItem || currentItem()->type() == Category_GroupItem)
    {
        on_action_user_moveFolder();
    }
}

void WizCategoryView::on_action_user_moveFolder()
{
    WIZKM_CHECK_SYNCING(this);
    //
    ::WizGetAnalyzer().logAction("categoryMenuMoveFolder");
    WizFolderSelector* selector = new WizFolderSelector(tr("Move folder"), m_app, WIZ_USERGROUP_SUPER, window());
    selector->setAcceptRoot(true);

    connect(selector, SIGNAL(finished(int)), SLOT(on_action_user_moveFolder_confirmed(int)));
    //
    QTimer::singleShot(0, [=]() {
        selector->exec();
    });
}

void WizCategoryView::on_action_user_moveFolder_confirmed(int result)
{
    WizFolderSelector* selector = qobject_cast<WizFolderSelector*>(sender());
    WizScopeGuard guard([&]{
       selector->deleteLater();
    });

    if (result != QDialog::Accepted)
        return;

    WizCategoryViewItemBase* curItem = currentCategoryItem<WizCategoryViewItemBase>();
    Q_ASSERT(curItem != nullptr);

    WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
    WizDatabase& currentDB = m_dbMgr.db(curItem->kbGUID());
    //  move group folder
    if (currentDB.isGroup())
    {
        WizCategoryViewGroupItem* groupItem = currentCategoryItem<WizCategoryViewGroupItem>();
        Q_ASSERT(groupItem != nullptr);

        moveGroupFolder(groupItem->tag(), selector);
        //
        mainWindow->quickSyncKb(groupItem->kbGUID());
    }
    else            //move personal folder
    {
        WizCategoryViewFolderItem* folderItem = currentCategoryItem<WizCategoryViewFolderItem>();
        Q_ASSERT(folderItem != nullptr);

        if (!movePersonalFolder(folderItem->location(), selector))
            return;
        //
        folderItem->parent()->removeChild(folderItem);
        //
        mainWindow->quickSyncKb("");
    }

    //
    if (selector->isSelectPersonalFolder())
    {
        mainWindow->quickSyncKb("");
    }
    else
    {
        mainWindow->quickSyncKb(selector->selectedGroupFolder().strKbGUID);
    }
}

void WizCategoryView::on_action_user_moveFolder_confirmed_progress(int nMax, int nValue,
                                                                    const QString& strOldLocation,
                                                                    const QString& strNewLocation,
                                                                    const WIZDOCUMENTDATA& data)
{
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
    WizProgressDialog* progress = mainWindow->progressDialog();
    progress->setVisible(true);

    progress->setActionString(tr("Moving note %1 ...").arg(data.strTitle));
    progress->setWindowTitle(tr("Move notes from %1 to %2").arg(strOldLocation).arg(strNewLocation));
    progress->setProgress(nMax, nValue);
    if (nMax == nValue + 1) {
        progress->setVisible(false);
    }
}

void WizCategoryView::on_action_copyItem()
{
    if (currentItem()->type() == Category_FolderItem || currentItem()->type() == Category_GroupItem)
    {
        on_action_user_copyFolder();
    }
}

void WizCategoryView::on_action_user_copyFolder()
{
    WIZKM_CHECK_SYNCING(this);
    //
    ::WizGetAnalyzer().logAction("categoryMenuCopyFolder");
    WizFolderSelector* selector = new WizFolderSelector(tr("Copy folder"), m_app, WIZ_USERGROUP_SUPER, window());
    selector->setAcceptRoot(true);
    bool isGroup = currentItem()->type() == Category_GroupItem;
    selector->setCopyStyle(!isGroup);

    connect(selector, SIGNAL(finished(int)), SLOT(on_action_user_copyFolder_confirmed(int)));
    //
    QTimer::singleShot(0, [=]() {
        selector->exec();
    });
}

void WizCategoryView::on_action_user_copyFolder_confirmed(int result)
{
    WizFolderSelector* selector = qobject_cast<WizFolderSelector*>(sender());
    WizScopeGuard guard([&]{
       selector->deleteLater();
    });

    if (result != QDialog::Accepted)
        return;

    WizCategoryViewItemBase* curItem = currentCategoryItem<WizCategoryViewItemBase>();
    Q_ASSERT(curItem != nullptr);

    WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
    WizDatabase& currentDB = m_dbMgr.db(curItem->kbGUID());
    //  move group folder
    if (currentDB.isGroup())
    {
        WizCategoryViewGroupItem* groupItem = currentCategoryItem<WizCategoryViewGroupItem>();
        Q_ASSERT(groupItem != nullptr);

        copyGroupFolder(groupItem->tag(), selector);
    }
    else            //copy  private folder to ...
    {
        WizCategoryViewFolderItem* folderItem = currentCategoryItem<WizCategoryViewFolderItem>();
        Q_ASSERT(folderItem != nullptr);

        copyPersonalFolder(folderItem->location(), selector);
    }

    //
    if (selector->isSelectPersonalFolder())
    {
        mainWindow->quickSyncKb("");
    }
    else
    {
        mainWindow->quickSyncKb(selector->selectedGroupFolder().strKbGUID);
    }
}

//

void WizApplyDarkModeStyles_lineEditor(QObject* parent)
{
    if (isDarkMode()) {
        for (QObject* child : parent->children()) {

            if (QWidget* childWidget = dynamic_cast<QWidget*>(child)) {
                //
                QString className = child->metaObject()->className();
                //
                qDebug() << className << childWidget->geometry();
                //
                if (QWidget* widget = dynamic_cast<QLineEdit*>(child)) {
                    widget->setStyleSheet("color:#a6a6a6;background-color:#333333");
                }
            }
            //
            WizApplyDarkModeStyles_lineEditor(child);
        }
    }
}

void WizCategoryView::on_action_renameItem()
{
    QTreeWidgetItem* p = currentItem();
    if (p)
    {
        if (p->type() == Category_FolderItem)
        {
            if (WizCategoryViewFolderItem* pFolder = currentCategoryItem<WizCategoryViewFolderItem>())
            {
                //user can not rename predefined folders name
                if (WizIsPredefinedLocation(pFolder->location()))
                {
                    QMessageBox::information(0, tr("Info"), tr("The default folder can not be renamed."));

                    return;
                }
            }
        }
        p->setFlags(p->flags() | Qt::ItemIsEditable);
        editItem(p, 0);
        //
        if (isDarkMode()) {
            //QTimer::singleShot(1000, [this] {
                WizApplyDarkModeStyles_lineEditor(this);
            //});
        }

    }
}

void WizCategoryView::on_action_user_renameFolder()
{
    ::WizGetAnalyzer().logAction("categoryMenuRenameFolder");
    WizCategoryViewFolderItem* p = currentCategoryItem<WizCategoryViewFolderItem>();
    Q_ASSERT(p);

    WizLineInputDialog* dialog = new WizLineInputDialog(tr("Rename folder"),
                                                          tr("Please input new folder name: "),
                                                          p->name(),window());

    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_user_renameFolder_confirmed(int)));

    dialog->exec();
}

void WizCategoryView::on_action_user_renameFolder_confirmed(int result)
{
    WizLineInputDialog* dialog = qobject_cast<WizLineInputDialog*>(sender());
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
    if (WizCategoryViewFolderItem* p = currentCategoryItem<WizCategoryViewFolderItem>()) {
        QString strOldLocation = p->location();
        int n = strOldLocation.lastIndexOf("/", -2);
        strLocation = strOldLocation.left(n + 1) + strFolderName + "/";

        qDebug() << "rename folder from : " << strOldLocation << "  to : " << strLocation;

        if (strLocation == strOldLocation)
            return;

        // move all documents to new folder
        WizFolder folder(m_dbMgr.db(), strOldLocation);
        connect(&folder, SIGNAL(moveDocument(int, int, const QString&, const QString&, const WIZDOCUMENTDATA&)),
                SLOT(on_action_user_renameFolder_confirmed_progress(int, int, const QString&, const QString&, const WIZDOCUMENTDATA&)));

        folder.moveToLocation(strLocation);
    }
}

void WizCategoryView::on_action_user_renameFolder_confirmed_progress(int nMax, int nValue,
                                                                      const QString& strOldLocation,
                                                                      const QString& strNewLocation,
                                                                      const WIZDOCUMENTDATA& data)
{
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
    WizProgressDialog* progress = mainWindow->progressDialog();
    progress->setVisible(true);

    progress->setActionString(tr("Moving note %1 ...").arg(data.strTitle));
    progress->setWindowTitle(tr("Move notes from %1 to %2").arg(strOldLocation).arg(strNewLocation));
    progress->setProgress(nMax, nValue);
    if (nMax <= nValue + 1) {
        progress->setVisible(false);
    }
}

void WizCategoryView::on_action_user_renameTag()
{
    ::WizGetAnalyzer().logAction("categoryMenuRenameTag");
    WizCategoryViewItemBase* p = currentCategoryItem<WizCategoryViewItemBase>();
    WizLineInputDialog* dialog = new WizLineInputDialog(tr("Rename tag"),
                                                          tr("Please input tag name: "),
                                                          p->name(), window());

    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_user_renameTag_confirmed(int)));

    dialog->exec();
}

void WizCategoryView::on_action_user_renameTag_confirmed(int result)
{
    WizLineInputDialog* dialog = qobject_cast<WizLineInputDialog*>(sender());
    QString strTagName = dialog->input();
    dialog->deleteLater();

    if (result != QDialog::Accepted) {
        return;
    }

    if (strTagName.isEmpty())
        return;

    if (WizCategoryViewTagItem* p = currentCategoryItem<WizCategoryViewTagItem>()) {
        WIZTAGDATA tag = p->tag();
        tag.strName = strTagName;
        m_dbMgr.db().modifyTag(tag);
        p->reload(m_dbMgr.db());
    }
}

void WizCategoryView::on_action_group_renameFolder()
{
    WIZKM_CHECK_SYNCING(this);
    //
    ::WizGetAnalyzer().logAction("categoryMenuRenameGroupFolder");
    WizCategoryViewItemBase* p = currentCategoryItem<WizCategoryViewItemBase>();

    WizLineInputDialog* dialog = new WizLineInputDialog(tr("Rename group folder"),
                                                          tr("Please input folder name: "),
                                                          p->name(), window());

    connect(dialog, SIGNAL(finished(int)), SLOT(on_action_group_renameFolder_confirmed(int)));

    dialog->exec();
}

void WizCategoryView::on_action_group_renameFolder_confirmed(int result)
{
    WizLineInputDialog* dialog = qobject_cast<WizLineInputDialog*>(sender());
    QString strTagName = dialog->input();
    dialog->deleteLater();

    if (result != QDialog::Accepted) {
        return;
    }

    if (strTagName.isEmpty()) {
        return;
    }

    if (WizCategoryViewGroupItem* p = currentCategoryItem<WizCategoryViewGroupItem>()) {
        WIZTAGDATA tag = p->tag();
        tag.strName = strTagName;
        m_dbMgr.db(tag.strKbGUID).modifyTag(tag);
        p->reload(m_dbMgr.db(tag.strKbGUID));
    }
}

void WizCategoryView::on_action_deleteItem()
{
    if (WizCategoryViewFolderItem* p = currentCategoryItem<WizCategoryViewFolderItem>())
    {
        //if (!::WizIsPredefinedLocation(p->location())) {
            on_action_user_deleteFolder();
        //}
    }
    else if (currentCategoryItem<WizCategoryViewTagItem>())
    {
        on_action_user_deleteTag();
    }
    else if (currentCategoryItem<WizCategoryViewGroupItem>())
    {
        on_action_group_deleteFolder();
    }
}

void WizCategoryView::on_action_user_deleteFolder()
{
    ::WizGetAnalyzer().logAction("categoryMenuDeleteFolder");
    WizCategoryViewFolderItem* p = currentCategoryItem<WizCategoryViewFolderItem>();
    if (!p)
        return;

    // FIXME:
    // 1. only show warning message if folder is not empty
    // 2. show progress windows when delete

    // setup warning messagebox
    QMessageBox* msgBox = new QMessageBox(window());
    msgBox->setWindowTitle(tr("Delete Folder"));
    msgBox->setIcon(QMessageBox::Information);
    msgBox->addButton(tr("Cancel"), QMessageBox::NoRole);
    QPushButton* btnOK = msgBox->addButton(tr("OK"), QMessageBox::YesRole);
    msgBox->setDefaultButton(btnOK);

    QString strWarning = tr("Do you really want to delete all notes inside folder: %1 ? (All notes will deleted in local and removed to trash from cloud server)").arg(p->location());
    msgBox->setText(strWarning);
    msgBox->exec();

    int result = QDialog::Rejected;
    if (msgBox->clickedButton() == btnOK)
    {
        result = QDialog::Accepted;
    }
    on_action_user_deleteFolder_confirmed(result);
}

void WizCategoryView::on_action_user_deleteFolder_confirmed(int result)
{    
    WizCategoryViewFolderItem* p = currentCategoryItem<WizCategoryViewFolderItem>();
    if (!p)
        return;

    if (result == QMessageBox::Accepted) {
        WizFolder folder(m_dbMgr.db(), p->location());
        folder.Delete();
    }
}

void WizCategoryView::on_action_user_deleteTag()
{
    ::WizGetAnalyzer().logAction("categoryMenuDeleteTag");
    WizCategoryViewTagItem* p = currentCategoryItem<WizCategoryViewTagItem>();
    if (!p)
        return;

    // FIXME : as above

    QMessageBox* msgBox = new QMessageBox(window());
    msgBox->setWindowTitle(tr("Delete tag"));
    msgBox->setIcon(QMessageBox::Information);
    msgBox->addButton(tr("Cancel"), QMessageBox::NoRole);
    QPushButton* btnOK = msgBox->addButton(tr("OK"), QMessageBox::YesRole);
    msgBox->setDefaultButton(btnOK);

    QString strWarning = tr("Do you really want to delete tag: %1 ? (include child tags if any)").arg(p->tag().strName);
    msgBox->setText(strWarning);
    msgBox->exec();

    int result = QDialog::Rejected;
    if (msgBox->clickedButton() == btnOK)
    {
        result = QDialog::Accepted;
    }
    on_action_user_deleteTag_confirmed(result);
}

void WizCategoryView::on_action_user_deleteTag_confirmed(int result)
{    
    WizCategoryViewTagItem* p = currentCategoryItem<WizCategoryViewTagItem>();
    if (!p)
        return;

    if (result == QMessageBox::Accepted) {
        WIZTAGDATA tag = p->tag();
        m_dbMgr.db().deleteTagWithChildren(tag, TRUE);
    }
}

void WizCategoryView::on_action_group_deleteFolder()
{
    ::WizGetAnalyzer().logAction("categoryMenuDeleteGroupFolder");
    WizCategoryViewGroupItem* p = currentCategoryItem<WizCategoryViewGroupItem>();
    if (!p)
        return;

    // FIXME: as above

    QMessageBox* msgBox = new QMessageBox(window());
    msgBox->setWindowTitle(tr("Delete group folder"));
    msgBox->setIcon(QMessageBox::Information);
    msgBox->addButton(tr("Cancel"), QMessageBox::NoRole);
    QPushButton* btnOK = msgBox->addButton(tr("OK"), QMessageBox::YesRole);
    msgBox->setDefaultButton(btnOK);

    QString strWarning = tr("Do you really want to delete folder: %1? (All notes will deleted in local and removed to trash from cloud server)").arg(p->tag().strName);
    msgBox->setText(strWarning);
    msgBox->exec();    

    int result = QDialog::Rejected;
    if (msgBox->clickedButton() == btnOK)
    {
        result = QDialog::Accepted;
    }
    on_action_group_deleteFolder_confirmed(result);
}

void WizCategoryView::on_action_group_deleteFolder_confirmed(int result)
{    
    WizCategoryViewGroupItem* p = currentCategoryItem<WizCategoryViewGroupItem>();
    if (!p)
        return;

    if (result == QMessageBox::Accepted) {
        WIZTAGDATA tag = p->tag();
        m_dbMgr.db(p->kbGUID()).deleteGroupFolder(tag, true);
    }
}



void WizCategoryView::on_action_itemAttribute()
{
    if (currentCategoryItem<WizCategoryViewGroupRootItem>())
    {
        on_action_groupAttribute();
    }
    else if(currentCategoryItem<WizCategoryViewBizGroupRootItem>())
    {
        on_action_bizgAttribute();
    }
}

void WizCategoryView::on_action_groupAttribute()
{
    ::WizGetAnalyzer().logAction("categoryMenuGroupAttribute");
    WizCategoryViewGroupRootItem* p = currentCategoryItem<WizCategoryViewGroupRootItem>();
    if (p && !p->kbGUID().isEmpty()) {
        if (p->isBizGroup()) {
            viewBizGroupInfo(p->kbGUID(), p->bizGUID());
        } else {
            viewPersonalGroupInfo(p->kbGUID());
        }
    }
}

void WizCategoryView::on_action_manageGroup()
{
    ::WizGetAnalyzer().logAction("categoryMenuManageGroup");
    WizCategoryViewGroupRootItem* p = currentCategoryItem<WizCategoryViewGroupRootItem>();
    if (p && !p->kbGUID().isEmpty()) {
        if (p->isBizGroup()) {
            manageBizGroup(p->kbGUID(), p->bizGUID());
        } else {
            managePersonalGroup(p->kbGUID());
        }
    }
}

void WizCategoryView::on_action_bizgAttribute()
{
    ::WizGetAnalyzer().logAction("categoryMenuNewBizAttribute");
    WizCategoryViewItemBase* p = currentCategoryItem<WizCategoryViewItemBase>();
    if (p && !p->kbGUID().isEmpty()) {

        viewBizInfo(p->kbGUID());
    }
}

void WizCategoryView::on_action_itemManage()
{
    if (currentCategoryItem<WizCategoryViewGroupRootItem>())
    {
        on_action_manageGroup();
    }
    else if(currentCategoryItem<WizCategoryViewBizGroupRootItem>())
    {
        on_action_manageBiz();
    }
}

void WizCategoryView::on_action_manageBiz()
{
    ::WizGetAnalyzer().logAction("categoryMenuManageBiz");
    WizCategoryViewBizGroupRootItem* p = currentCategoryItem<WizCategoryViewBizGroupRootItem>();
    if (p && !p->biz().bizGUID.isEmpty()) {
        manageBiz(p->biz().bizGUID, false);
    }
}

void WizCategoryView::on_action_removeShortcut()
{
    ::WizGetAnalyzer().logAction("categoryMenuRemoveShortcut");
    WizCategoryViewShortcutItem* p = currentCategoryItem<WizCategoryViewShortcutItem>();
    removeShortcut(p);
}

void WizCategoryView::on_action_addToShortcuts()
{
    ::WizGetAnalyzer().logAction("categoryMenuAddToShortcut");
    WizCategoryViewItemBase* p = currentCategoryItem<WizCategoryViewItemBase>();
    WizCategoryViewShortcutRootItem* shortcutRoot = dynamic_cast<WizCategoryViewShortcutRootItem*>(findAllShortcutItem());
    if (p && shortcutRoot)
    {
        shortcutRoot->addItemToShortcuts(p);
        QTimer::singleShot(200, this, SLOT(saveShortcutState()));
    }
}


void WizCategoryView::on_action_addCustomSearch()
{
    ::WizGetAnalyzer().logAction("categoryMenuAddCustomSearch");
    bool bSearchOnly = false;
    WizAdvancedSearchDialog dlg(bSearchOnly);
    if (dlg.exec() == QDialog::Accepted)
    {
        QString strParam = dlg.getParams();
        advancedSearchByCustomParam(strParam);

        // create item by param
        QString strSQLWhere, name , keyword;
        int scope;
        WizAdvancedSearchDialog::paramToSQL(strParam, strSQLWhere, keyword, name, scope);

        WizCategoryViewItemBase* rootItem = findAllSearchItem();
        if (!rootItem)
            return;

        QTreeWidgetItem* parentItem = 0;
        for (int i = 0; i < rootItem->childCount(); i++)
        {
            if (rootItem->child(i)->text(0) == CATEGORY_SEARCH_BYCUSTOM)
            {
                parentItem = dynamic_cast<WizCategoryViewSearchItem*>(rootItem->child(i));
                break;
            }
        }
        if (parentItem == 0)
        {
            parentItem = new WizCategoryViewSearchItem(m_app, CATEGORY_SEARCH_BYCUSTOM);
        }
        rootItem->addChild(parentItem);


        QString strGuid = ::WizGenGUIDLowerCaseLetterOnly();
        WizCategoryViewCustomSearchItem* item = new WizCategoryViewCustomSearchItem(
                    m_app, name, strParam, strSQLWhere, strGuid, keyword, scope);
        parentItem->addChild(item);
        sortItems(0, Qt::AscendingOrder);
        saveCustomAdvancedSearchParamToDB(strGuid, strParam);
    }
}

void WizCategoryView::on_action_editCustomSearch()
{
    ::WizGetAnalyzer().logAction("categoryMenuEditCustomSearch");
    if (currentItem()->type() == Category_QuickSearchCustomItem)
    {
        WizCategoryViewCustomSearchItem* item = dynamic_cast<WizCategoryViewCustomSearchItem*>(currentItem());
        if (item)
        {
            WizAdvancedSearchDialog dlg(false);
            dlg.setParams(item->getSelectParam());
            if (dlg.exec() == QDialog::Accepted)
            {
                QString strParam = dlg.getParams();
                QString strSQLWhere, name, keyword;
                int scope;
                WizAdvancedSearchDialog::paramToSQL(strParam, strSQLWhere, keyword, name, scope);
                item->setText(0, name);
                item->setKeyword(keyword);
                item->setSelectParam(strParam);
                item->setSQLWhere(strSQLWhere);
                item->setSearchScope(scope);
                saveCustomAdvancedSearchParamToDB(item->kbGUID(), strParam);
                update();
            }
        }
    }
}

void WizCategoryView::on_action_removeCustomSearch()
{
    ::WizGetAnalyzer().logAction("categoryMenuRemoveCustomSearch");
    if (currentItem()->type() == Category_QuickSearchCustomItem)
    {
        WizCategoryViewCustomSearchItem* item = dynamic_cast<WizCategoryViewCustomSearchItem*>(currentItem());
        if (item)
        {
            QString strGuid = item->kbGUID();
            deleteCustomAdvancedSearchParamFromDB(strGuid);
            item->parent()->removeChild(item);
        }
    }
}

void WizCategoryView::on_action_emptyTrash()
{
    ::WizGetAnalyzer().logAction("categoryMenuEmptyTrash");
    // FIXME: show progress
    if (WizCategoryViewTrashItem* pTrashItem = currentCategoryItem<WizCategoryViewTrashItem>()) {
        QString strKbGUID = pTrashItem->kbGUID();
        Q_ASSERT(!strKbGUID.isEmpty());

        CWizDocumentDataArray arrayDocument;
        pTrashItem->getDocuments(m_dbMgr.db(strKbGUID), arrayDocument);

        foreach (const WIZDOCUMENTDATA& data, arrayDocument)
        {
            WizDocument doc(m_dbMgr.db(strKbGUID), data);
            doc.Delete();
        }
    }
}

void WizCategoryView::on_itemSelectionChanged()
{
    if (currentItem() == nullptr)
        return;

    WizCategoryViewGroupRootItem* pRoot = currentCategoryItem<WizCategoryViewGroupRootItem>();
    WizCategoryViewGroupItem* pItem = currentCategoryItem<WizCategoryViewGroupItem>();
    if (pRoot || pItem) {
        WizCategoryViewItemBase* p = currentCategoryItem<WizCategoryViewItemBase>();
        on_group_permissionChanged(p->kbGUID());
    } else {
        setActionsEnabled(true); // enable all actions if selected is not group
    }

    // notify of selection
    if (currentCategoryItem<WizCategoryViewMessageItem>()) {
        Q_EMIT documentsHint(tr("Recent meesages"));
    } else if (currentCategoryItem<WizCategoryViewAllFoldersItem>()) {
        Q_EMIT documentsHint(tr("All Notes"));
    } else if (currentCategoryItem<WizCategoryViewAllTagsItem>()) {
        Q_EMIT documentsHint(tr("No tag notes"));
    } else {
        Q_EMIT documentsHint(currentItem()->text(0));
    }
}

void WizCategoryView::on_itemChanged(QTreeWidgetItem* item, int column)
{
    //ignore item changed signal that caused by drag-drop
    if (m_dragItem)
        return;

    qDebug() << "item changed : " << item->text(0) << item->type();
    if (item->type() == Category_FolderItem)
    {
        WizCategoryViewFolderItem* pFolder = dynamic_cast<WizCategoryViewFolderItem*>(item);
        if (pFolder == nullptr || pFolder->text(0) == pFolder->name())
            return;
        qDebug() << "folder changed text " << pFolder->text(0) << "  name : " << pFolder->name();

       renameFolder(pFolder, pFolder->text(0));
    }
    else if (item->type() == Category_TagItem)
    {
        WizCategoryViewTagItem* pTag = dynamic_cast<WizCategoryViewTagItem*>(item);
        if (pTag == nullptr || pTag->text(0).isEmpty() || pTag->text(0) == pTag->tag().strName)
            return;

        qDebug() << "tag changed text " << pTag->text(0) << "  name : " << pTag->tag().strName;
        WIZTAGDATA tag = pTag->tag();
        tag.strName = pTag->text(0);
        m_dbMgr.db().modifyTag(tag);
        pTag->reload(m_dbMgr.db());
    }
    else if (item->type() == Category_GroupItem)
    {
        WizCategoryViewGroupItem* pGroup = dynamic_cast<WizCategoryViewGroupItem*>(item);
        if (pGroup == nullptr || pGroup->text(0).isEmpty() || pGroup->text(0) == pGroup->tag().strName)
            return;

        qDebug() << "group changed text " << pGroup->text(0) << "  name : " << pGroup->tag().strName;
        renameGroupFolder(pGroup, pGroup->text(0));
    }
}

void WizCategoryView::on_itemClicked(QTreeWidgetItem *item, int column)
{
    if (WizCategoryViewLinkItem* pLink = dynamic_cast<WizCategoryViewLinkItem*>(item))
    {
        if (LINK_COMMAND_ID_CREATE_GROUP == pLink->commandId())
        {
            createGroup();
        }
    }
    else if (WizCategoryViewSectionItem* pItem = dynamic_cast<WizCategoryViewSectionItem*>(item))
    {
        if(CATEGORY_TEAM_GROUPS == pItem->name() && pItem->extraButtonClickTest())
        {
            createGroup();
        }
    }
    else if (WizCategoryViewMessageItem* pItem = dynamic_cast<WizCategoryViewMessageItem*>(item))
    {
        emit itemSelectionChanged();
    }
    else if (WizCategoryViewBizGroupRootItem* pItem = dynamic_cast<WizCategoryViewBizGroupRootItem*>(item))
    {
        bool bUseCount = pItem->isUnreadButtonUseable() && pItem->isSelected();
        if (bUseCount)
        {
            emit itemSelectionChanged();
            if (pItem->hitTestUnread())
            {
                emit unreadButtonClicked();
            }
        }
        else if (pItem->isExtraButtonUseable() && pItem->extraButtonClickTest())
        {
            if (pItem->isHr())
            {
                manageBiz(pItem->biz().bizGUID, true);
            }
            else
            {
                QMessageBox::information(0, tr("Info"), tr("Your enterprise services has expired, could not manage members. "
                                                           "Please purchase services or apply for an extension."));
            }
        }
    }    
    else if (WizCategoryViewGroupRootItem* pItem = dynamic_cast<WizCategoryViewGroupRootItem*>(item))
    {
        if (pItem->extraButtonClickTest())
        {
            promptGroupLimitMessage(pItem->kbGUID(), pItem->bizGUID());
        }
        else if (pItem->hitTestUnread())
        {
            emit itemSelectionChanged();
            emit unreadButtonClicked();
        }
    }
    else if (item->type() == Category_ShortcutItem)
    {
        /*
        CWizCategoryViewShortcutItem* shortcut = dynamic_cast<CWizCategoryViewShortcutItem*>(item);
        if (shortcut && shortcut->shortcutType() == CWizCategoryViewShortcutItem::Document)
        {
            emit itemSelectionChanged();
        }
        */
    }
}

void WizCategoryView::updateGroupsData()
{
    CWizGroupDataArray arrayGroup;
    m_dbMgr.db().getAllGroupInfo(arrayGroup);
    //
    CWizBizDataArray arrayBiz;
    m_dbMgr.db().getAllBizInfo(arrayBiz);
    //
    for (CWizBizDataArray::const_iterator it = arrayBiz.begin(); it != arrayBiz.end(); it++)
    {
        const WIZBIZDATA& biz = *it;
        WizCategoryViewItemBase* pBizGroupItem = findBizGroupsRootItem(biz, false);
        if (!pBizGroupItem)
        {
            //
            initBiz(biz);
        }
        setBizRootItemExtraButton(pBizGroupItem, biz);
    }

    for (CWizGroupDataArray::const_iterator it = arrayGroup.begin(); it != arrayGroup.end(); it++)
    {
        const WIZGROUPDATA& group = *it;
        WizCategoryViewGroupRootItem* pGroupItem = findGroup(group.strGroupGUID);
        setGroupRootItemExtraButton(pGroupItem, group);
    }

    resetSections();

    //
//    CWizGroupDataArray arrayOwnGroup;
//    CWizDatabase::GetOwnGroups(arrayGroup, arrayOwnGroup);
//    if (!arrayOwnGroup.empty())
//    {
//        for (CWizGroupDataArray::const_iterator it = arrayOwnGroup.begin(); it != arrayOwnGroup.end(); it++)
//        {
//            const WIZGROUPDATA& group = *it;
//            CWizCategoryViewGroupRootItem* pOwnGroupItem = findGroup(group.strGroupGUID);
//            setGroupRootItemExtraButton(pOwnGroupItem, group);
//        }
//    }
//    //
//    CWizGroupDataArray arrayJionedGroup;
//    CWizDatabase::GetJionedGroups(arrayGroup, arrayJionedGroup);
//    if (!arrayJionedGroup.empty())
//    {
//        for (CWizGroupDataArray::const_iterator it = arrayJionedGroup.begin(); it != arrayJionedGroup.end(); it++)
//        {
//            const WIZGROUPDATA& group = *it;
//            CWizCategoryViewGroupRootItem* pOwnGroupItem = findGroup(group.strGroupGUID);
//            setGroupRootItemExtraButton(pOwnGroupItem, group);
//        }
    //    }
}

void WizCategoryView::on_shortcutDataChanged(const QString& shortcut)
{
    WizCategoryViewItemBase* shortcutRoot = findAllShortcutItem();
    if (shortcutRoot)
    {
        QList<QTreeWidgetItem *> itemList = shortcutRoot->takeChildren();
        for (QTreeWidgetItem* child : itemList)
        {
            delete child;
        }
    }

    //
    initShortcut(shortcut);
}

void WizCategoryView::addDocumentToShortcuts(const WIZDOCUMENTDATA& doc)
{
    WizCategoryViewShortcutRootItem* shortcutRoot = dynamic_cast<WizCategoryViewShortcutRootItem*>(findAllShortcutItem());
    if (shortcutRoot)
    {
        shortcutRoot->addDocumentToShortcuts(doc);
        QTimer::singleShot(200, this, SLOT(saveShortcutState()));
    }
}

void WizCategoryView::createGroup()
{
    QString strExtInfo = WizCommonApiEntry::appstoreParam(false);
    QString strUrl = WizCommonApiEntry::makeUpUrlFromCommand("create_group", WIZ_TOKEN_IN_URL_REPLACE_PART, strExtInfo);
    WizShowWebDialogWithToken(tr("Create Team for Free"), strUrl, window());
    //
    WizMainWindow::instance()->setNeedResetGroups();
}

void WizCategoryView::viewPersonalGroupInfo(const QString& groupGUID)
{
    QString extInfo = "kb=" + groupGUID + WizCommonApiEntry::appstoreParam();
    QString strUrl = WizCommonApiEntry::makeUpUrlFromCommand("view_personal_group", WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    WizShowWebDialogWithToken(tr("View group info"), strUrl, window());
    //
    WizMainWindow::instance()->setNeedResetGroups();
}

void WizCategoryView::viewBizGroupInfo(const QString& groupGUID, const QString& bizGUID)
{
    QString extInfo = "kb=" + groupGUID + "&biz=" + bizGUID + WizCommonApiEntry::appstoreParam();
    QString strUrl = WizCommonApiEntry::makeUpUrlFromCommand("view_biz_group", WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    WizShowWebDialogWithToken(tr("View group info"), strUrl, window());
    //
    WizMainWindow::instance()->setNeedResetGroups();
}

void WizCategoryView::managePersonalGroup(const QString& groupGUID)
{
    QString extInfo = "kb=" + groupGUID + WizCommonApiEntry::appstoreParam();
    QString strUrl = WizCommonApiEntry::makeUpUrlFromCommand("manage_personal_group", WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    WizShowWebDialogWithToken(tr("Manage group"), strUrl, window());
    //
    WizMainWindow::instance()->setNeedResetGroups();
}

void WizCategoryView::manageBizGroup(const QString& groupGUID, const QString& bizGUID)
{
    QString extInfo = "kb=" + groupGUID + "&biz=" + bizGUID + WizCommonApiEntry::appstoreParam();
    QString strUrl = WizCommonApiEntry::makeUpUrlFromCommand("manage_biz_group", WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    WizShowWebDialogWithToken(tr("Manage group"), strUrl, window());
    //
    WizMainWindow::instance()->setNeedResetGroups();
}

void WizCategoryView::promptGroupLimitMessage(const QString &groupGUID, const QString &/*bizGUID*/)
{
    WizDatabase& db = m_dbMgr.db(groupGUID);
    QString strErrorMsg;
    if (db.getStorageLimitMessage(strErrorMsg))
    {
        QMessageBox::warning(this, tr("Storage Limit Info"), strErrorMsg);
    }
    else if (db.getTrafficLimitMessage(strErrorMsg))
    {
        QMessageBox::warning(this, tr("Traffic Limit Info"), strErrorMsg);
    }
    else if (db.getNoteCountLimit(strErrorMsg))
    {
        QMessageBox::warning(this, tr("Note Count Limit Info"), tr("Group notes count limit exceeded!"));
    }
}

void WizCategoryView::initTopLevelItems()
{
    WizCategoryViewMessageItem* pMsg = new WizCategoryViewMessageItem(m_app, CATEGORY_MESSAGES_ALL, WizCategoryViewMessageItem::All);
    addTopLevelItem(pMsg);
    pMsg->setUnreadCount(m_dbMgr.db().getUnreadMessageCount());

    WizCategoryViewShortcutRootItem* pShortcutRoot = new WizCategoryViewShortcutRootItem(m_app, CATEGORY_SHORTCUTS);
    addTopLevelItem(pShortcutRoot);

    WizCategoryViewSearchRootItem* pSearchRoot = new WizCategoryViewSearchRootItem(m_app, CATEGORY_SEARCH);
    addTopLevelItem(pSearchRoot);
    //
    WizCategoryViewMySharesItem* pShareItem = new WizCategoryViewMySharesItem(m_app, CATEGORY_MYSHARES);
    addTopLevelItem(pShareItem);

    WizCategoryViewAllFoldersItem* pAllFoldersItem = new WizCategoryViewAllFoldersItem(m_app, CATEGORY_FOLDERS, m_dbMgr.db().kbGUID());
    addTopLevelItem(pAllFoldersItem);

    WizCategoryViewAllTagsItem* pAllTagsItem = new WizCategoryViewAllTagsItem(m_app, CATEGORY_TAGS, m_dbMgr.db().kbGUID());
    addTopLevelItem(pAllTagsItem);

    CWizGroupDataArray arrayGroup;
    m_dbMgr.db().getAllGroupInfo(arrayGroup);

    //
    CWizBizDataArray arrayBiz;
    m_dbMgr.db().getAllBizInfo(arrayBiz);
    //
    std::vector<WizCategoryViewItemBase*> arrayGroupsItem;
    //
    for (CWizBizDataArray::const_iterator it = arrayBiz.begin();
         it != arrayBiz.end();
         it++)
    {
        const WIZBIZDATA& biz = *it;
        WizCategoryViewBizGroupRootItem* pBizGroupItem = new WizCategoryViewBizGroupRootItem(m_app, biz);
        setBizRootItemExtraButton(pBizGroupItem, biz);

        addTopLevelItem(pBizGroupItem);
        pBizGroupItem->setExpanded(true);
        arrayGroupsItem.push_back(pBizGroupItem);
    }
    //
    CWizGroupDataArray arrayOwnGroup;
    WizDatabase::getOwnGroups(arrayGroup, arrayOwnGroup);
    if (!arrayOwnGroup.empty())
    {
        WizCategoryViewOwnGroupRootItem* pOwnGroupItem = new WizCategoryViewOwnGroupRootItem(m_app);
        addTopLevelItem(pOwnGroupItem);
        pOwnGroupItem->setExpanded(true);
        arrayGroupsItem.push_back(pOwnGroupItem);
    }
    //
    CWizGroupDataArray arrayJionedGroup;
    WizDatabase::getJionedGroups(arrayGroup, arrayJionedGroup);
    if (!arrayJionedGroup.empty())
    {
        WizCategoryViewJionedGroupRootItem* pJionedGroupItem = new WizCategoryViewJionedGroupRootItem(m_app);
        addTopLevelItem(pJionedGroupItem);
        pJionedGroupItem->setExpanded(true);
        arrayGroupsItem.push_back(pJionedGroupItem);
    }

    resetCreateGroupLink();
}

void WizCategoryView::viewBizInfo(const QString& bizGUID)
{
    QString extInfo = "biz=" + bizGUID + WizCommonApiEntry::appstoreParam();
    QString strUrl = WizCommonApiEntry::makeUpUrlFromCommand("view_biz", WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    WizShowWebDialogWithToken(tr("View team info"), strUrl, window());
    //
    WizMainWindow::instance()->setNeedResetGroups();
}

void WizCategoryView::manageBiz(const QString& bizGUID, bool bUpgrade)
{
    QString extInfo = "biz=" + bizGUID;
    if (bUpgrade)
    {
        extInfo += "&p=payment";
    }
    extInfo += WizCommonApiEntry::appstoreParam();
    QString strUrl = WizCommonApiEntry::makeUpUrlFromCommand("manage_biz", WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    WizShowWebDialogWithToken(tr("Manage team"), strUrl, window());
    //
    WizMainWindow::instance()->setNeedResetGroups();
}


void WizCategoryView::init()
{    
    initTopLevelItems();
    initGeneral();
    initFolders();
    initTags();
    initStyles();
    resetSections();
    sortFolders();
    sortPersonalTags();

    initGroups();

    for (int i = 0; i < topLevelItemCount(); ++i)
    {
        if (WizCategoryViewBizGroupRootItem* pItem = dynamic_cast<WizCategoryViewBizGroupRootItem*>(topLevelItem(i)))
        {
            int childCount = pItem->childCount();
            qDebug() << childCount;
            pItem->sortChildren(0, Qt::AscendingOrder);
        }
    }
    //
    for (int i = 0 ; i < m_dbMgr.count(); ++i)
    {
        sortGroupTags(m_dbMgr.at(i).kbGUID());
    }
}

void WizCategoryView::resetSections()
{
    sortItems(0, Qt::AscendingOrder);
    //
    //
    //remove extra section
    for (int i = topLevelItemCount() - 1; i >= 1; i--)
    {
        if (NULL != dynamic_cast<WizCategoryViewSectionItem *>(topLevelItem(i)))
        {
            if (NULL != dynamic_cast<WizCategoryViewSectionItem *>(topLevelItem(i - 1)))
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
            if (NULL != dynamic_cast<WizCategoryViewSectionItem *>(topLevelItem(i)))
            {
                WizCategoryViewItemBase* pPrevItem = dynamic_cast<WizCategoryViewItemBase *>(topLevelItem(i - 1));
                WizCategoryViewItemBase* pNextItem = dynamic_cast<WizCategoryViewItemBase *>(topLevelItem(i + 1));
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

    sortItems(0, Qt::AscendingOrder);

    QString lastSectionName;
    //
    for (int i = 0; i < topLevelItemCount(); i++)
    {
        WizCategoryViewItemBase* pItem = dynamic_cast<WizCategoryViewItemBase *>(topLevelItem(i));
        if (!pItem)
            continue;
        //
        if (NULL != dynamic_cast<WizCategoryViewSectionItem *>(pItem))
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
                WizCategoryViewSectionItem* pExistingSection = NULL;
                if (i > 0)
                {
                    pExistingSection = dynamic_cast<WizCategoryViewSectionItem *>(topLevelItem(i - 1));
                }
                //
                if (pExistingSection)
                {
                    pExistingSection->reset(sectionName, pItem->getSortOrder() - 1);
                }
                else
                {
                    pExistingSection = new WizCategoryViewSectionItem(m_app, sectionName, pItem->getSortOrder() - 1);
                    WizOEMSettings oemSettings(m_dbMgr.db().getAccountPath());
                    if(CATEGORY_TEAM_GROUPS == sectionName && !oemSettings.isForbidCreateBiz())
                    {
                        pExistingSection->setExtraButtonIcon("category_create_group");
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

void WizCategoryView::updatePersonalFolderDocumentCount()
{
    if (!m_timerUpdateFolderCount) {
        m_timerUpdateFolderCount = new QTimer(this);
        m_timerUpdateFolderCount->setSingleShot(true);
        connect(m_timerUpdateFolderCount, SIGNAL(timeout()), SLOT(on_updatePersonalFolderDocumentCount_timeout()));
    }

    if (m_timerUpdateFolderCount->isActive()) {
        return;
    }

    m_timerUpdateFolderCount->start(2000);
}

void WizCategoryView::on_updatePersonalFolderDocumentCount_timeout()
{
    sender()->deleteLater();
    updatePersonalFolderDocumentCount_impl();
}

void WizCategoryView::updatePersonalFolderDocumentCount_impl()
{
    std::map<CString, int> mapDocumentCount;
    if (!m_dbMgr.db().getAllLocationsDocumentCount(mapDocumentCount)) {
        TOLOG("[ERROR]: Failed to get all locations count map");
        return;
    }

    WizCategoryViewItemBase* pFolderRoot = findAllFolderItem();
    if (!pFolderRoot)
        return;

    // folder items
    int nTotal = 0;
    updateChildFolderDocumentCount(pFolderRoot, mapDocumentCount, nTotal);
    pFolderRoot->setDocumentsCount(-1, nTotal);

    // trash item
    for (int i = pFolderRoot->childCount() - 1; i >= 0; i--) {
        if (WizCategoryViewTrashItem* pTrash = dynamic_cast<WizCategoryViewTrashItem*>(pFolderRoot->child(i))) {
            //pTrash->setDocumentsCount(-1, m_dbMgr.db().getTrashDocumentCount());
        }
    }

    update();
}

void WizCategoryView::updateChildFolderDocumentCount(WizCategoryViewItemBase* pItem,
                                                     const std::map<CString, int>& mapDocumentCount,
                                                     int& allCount)
{
    for (int i = 0; i < pItem->childCount(); i++) {
        if (WizCategoryViewItemBase* pItemChild = dynamic_cast<WizCategoryViewItemBase*>(pItem->child(i))) {
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

            if (WizCategoryViewFolderItem* pFolder = dynamic_cast<WizCategoryViewFolderItem*>(pItemChild))
            {
                if (!pFolder->location().startsWith(LOCATION_DELETED_ITEMS))
                {
                    allCount += nTotalChild;
                }
            }
        }
    }
}

void WizCategoryView::updateGroupFolderDocumentCount(const QString& strKbGUID)
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

void WizCategoryView::updateGroupFolderDocumentCount_impl(const QString &strKbGUID)
{
    // NOTE: groupItem have been handled as tag in other palce.Here use tagFunction to calc groupItem.
    std::map<CString, int> mapDocumentCount;
    if (!m_dbMgr.db(strKbGUID).getAllTagsDocumentCount(mapDocumentCount)) {
        TOLOG("[ERROR]: Failed to get all tags count map");
        return;
    }

    WizCategoryViewGroupRootItem* pGroupRoot = NULL;
    pGroupRoot = findGroup(strKbGUID);

    if (!pGroupRoot)
        return;

    int nCurrent = 0;
    if (!m_dbMgr.db(strKbGUID).getDocumentsNoTagCount(nCurrent)) {
        qDebug() << "Failed to get no tag documents count, kb_guid: " << strKbGUID;
        return;
    }

    int nTotalChild = 0;
    updateChildTagDocumentCount(pGroupRoot, mapDocumentCount, nTotalChild);

    // trash item
    for (int i = pGroupRoot->childCount() - 1; i >= 0; i--) {
        if (WizCategoryViewTrashItem* pTrash = dynamic_cast<WizCategoryViewTrashItem*>(pGroupRoot->child(i))) {
            pTrash->setDocumentsCount(-1, m_dbMgr.db(strKbGUID).getTrashDocumentCount());
        }

        if (WizCategoryViewGroupNoTagItem* pItem = dynamic_cast<WizCategoryViewGroupNoTagItem*>(pGroupRoot->child(i))) {
            int nCount = 0;
            if (m_dbMgr.db(strKbGUID).getDocumentsNoTagCount(nCount)) {
                pItem->setDocumentsCount(-1, nCount);
            }
        }
    }

    //unread documents
    pGroupRoot->setUnreadCount(m_dbMgr.db(strKbGUID).getGroupUnreadDocumentCount());

    if (pGroupRoot->isBizGroup())
    {
        WizCategoryViewBizGroupRootItem *bizRootItem = dynamic_cast<WizCategoryViewBizGroupRootItem *>(pGroupRoot->parent());
        if (bizRootItem)
        {
            bizRootItem->updateUnreadCount();
        }
    }


    update();
}

void WizCategoryView::updatePersonalTagDocumentCount()
{
    if (!m_timerUpdateTagCount) {
        m_timerUpdateTagCount = new QTimer(this);
        m_timerUpdateTagCount->setSingleShot(true);
        connect(m_timerUpdateTagCount, SIGNAL(timeout()), SLOT(on_updatePersonalTagDocumentCount_timeout()));
    }

    if (m_timerUpdateTagCount->isActive()) {
        return;
    }

    m_timerUpdateTagCount->start(2000);
}

void WizCategoryView::updateGroupTagDocumentCount(const QString& strKbGUID)
{
    Q_UNUSED (strKbGUID)
}

void WizCategoryView::importFiles(QStringList& strFileList)
{
    WizFileImporter *fileReader = new WizFileImporter(m_dbMgr);

    WizProgressDialog progressDialog;
    progressDialog.setProgress(100,0);
    progressDialog.setActionString(tr("loading..."));
    progressDialog.setWindowTitle(tr("%1 files to import.").arg(strFileList.count()));
    connect(fileReader, SIGNAL(importProgress(int,int)), &progressDialog, SLOT(setProgress(int,int)));
    connect(fileReader,SIGNAL(importFinished(bool,QString,QString)), &progressDialog,SLOT(close()));
    connect(fileReader, SIGNAL(importFinished(bool,QString,QString)), SLOT(on_importFile_finished(bool,QString,QString)));

    QString strKbGUID;
    WIZTAGDATA tag;
    QString strLocation;
    if (!getAvailableNewNoteTagAndLocation(strKbGUID, tag, strLocation))
        return;

    WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
        fileReader->importFiles(strFileList, strKbGUID, strLocation, tag);
        //
        fileReader->deleteLater();
    });
    //
    progressDialog.exec();
}

bool WizCategoryView::createDocument(WIZDOCUMENTDATA& data)
{
    return createDocument(data, "<!DOCTYPE html><html><head></head><body><div><br/></div></body></html>", tr("Untitled"));
}

void WizCategoryView::on_updatePersonalTagDocumentCount_timeout()
{
    sender()->deleteLater();
    updatePersonalTagDocumentCount_impl();
}

void WizCategoryView::on_updateGroupFolderDocumentCount_mapped_timeout(const QString& strKbGUID)
{
    sender()->deleteLater();
    m_mapTimerUpdateGroupCount.remove(strKbGUID);

    updateGroupFolderDocumentCount_impl(strKbGUID);
}

void WizCategoryView::on_initGroupFolder_finished()
{
    static int count = 0;
    ++count;
//    qDebug() << count << " group init finished, total " << m_dbMgr.count();
    if (count >= m_dbMgr.count())
    {
        loadExpandState();
        loadSectionStatus();
    }
}

void WizCategoryView::updatePersonalTagDocumentCount_impl(const QString& strKbGUID)
{
    std::map<CString, int> mapDocumentCount;
    if (!m_dbMgr.db(strKbGUID).getAllTagsDocumentCount(mapDocumentCount)) {
        TOLOG("[ERROR]: Failed to get all tags count map");
        return;
    }

    WizCategoryViewItemBase* pTagRoot = NULL;
    if (strKbGUID.isEmpty()) {
        pTagRoot = findAllTagsItem();
    } else {
        pTagRoot = findGroup(strKbGUID);
    }

    if (!pTagRoot)
        return;

    int nCurrent = 0;
    if (!m_dbMgr.db(strKbGUID).getDocumentsNoTagCount(nCurrent)) {
        qDebug() << "Failed to get no tag documents count, kb_guid: " << strKbGUID;
        return;
    }

    int nTotalChild = 0;
    updateChildTagDocumentCount(pTagRoot, mapDocumentCount, nTotalChild);

    // trash item
    for (int i = pTagRoot->childCount() - 1; i >= 0; i--) {
        if (WizCategoryViewTrashItem* pTrash = dynamic_cast<WizCategoryViewTrashItem*>(pTagRoot->child(i))) {
            pTrash->setDocumentsCount(-1, m_dbMgr.db(strKbGUID).getTrashDocumentCount());
        }

        if (WizCategoryViewGroupNoTagItem* pItem = dynamic_cast<WizCategoryViewGroupNoTagItem*>(pTagRoot->child(i))) {
            int nCount = 0;
            if (m_dbMgr.db(strKbGUID).getDocumentsNoTagCount(nCount)) {
                pItem->setDocumentsCount(-1, nCount);
            }
        }
    }

    update();
}

void WizCategoryView::updateChildTagDocumentCount(WizCategoryViewItemBase* pItem,
                                                  const std::map<CString, int>& mapDocumentCount, int &allCount)
{
    for (int i = 0; i < pItem->childCount(); i++) {
        QString strGUID;
        if (WizCategoryViewTagItem* pItemChild = dynamic_cast<WizCategoryViewTagItem*>(pItem->child(i)))
        {
            strGUID = pItemChild->tag().strGUID;
        }
        else if (WizCategoryViewGroupItem* pItemChild = dynamic_cast<WizCategoryViewGroupItem*>(pItem->child(i)))
        {
            strGUID = pItemChild->tag().strGUID;
        }
        else if (WizCategoryViewGroupNoTagItem* pItemChild = dynamic_cast<WizCategoryViewGroupNoTagItem*>(pItem->child(i)))
        {
            Q_UNUSED(pItemChild);
            continue;
        }
        else if (WizCategoryViewTrashItem* pTrash = dynamic_cast<WizCategoryViewTrashItem*>(pItem->child(i)))
        {
            Q_UNUSED(pTrash);
            continue;
        }

        Q_ASSERT(!strGUID.isEmpty());

        if (WizCategoryViewItemBase* pItemChild = dynamic_cast<WizCategoryViewItemBase*>(pItem->child(i))) {
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

void WizCategoryView::setBizRootItemExtraButton(WizCategoryViewItemBase* pItem, const WIZBIZDATA& bizData)
{
    if (pItem)
    {
        WizDatabase& db = m_dbMgr.db();
        if (bizData.bizIsDue || db.isBizServiceExpr(bizData.bizGUID))
        {
            pItem->setExtraButtonIcon("bizDue");
        }
        else
        {
            pItem->setExtraButtonIcon("");
        }
    }
}

void WizCategoryView::setGroupRootItemExtraButton(WizCategoryViewItemBase* pItem, const WIZGROUPDATA& gData)
{
    if (pItem)
    {
        WizDatabase& db = m_dbMgr.db(gData.strGroupGUID);
        if (db.isStorageLimit() || db.isTrafficLimit() || db.isNoteCountLimit())
        {
            pItem->setExtraButtonIcon("groupLimit");
        }
        else
        {
            pItem->setExtraButtonIcon("");
        }
    }
}

void WizCategoryView::moveFolderPostionBeforeTrash(const QString& strLocation)
{
    const QString strFolderPostion = "FolderPosition/";
    QSettings* setting = WizGlobal::settings();
    int nValue = setting->value(strFolderPostion + "/Deleted Items/").toInt();
    setting->setValue(strFolderPostion + strLocation, nValue);
    //
    QStringList strFolderList = setting->allKeys();
    foreach (QString strFolder, strFolderList)
    {
        if (strFolder.startsWith(strFolderPostion + "Deleted Items"))
        {
            nValue = setting->value(strFolder).toInt();
            setting->setValue(strFolder, nValue + 1);
        }
    }
}

bool WizCategoryView::getAvailableNewNoteTagAndLocation(QString& strKbGUID, WIZTAGDATA& tag, QString& strLocation)
{
    if (m_selectedItem) {
        this->restoreSelection();
    }
    //
    QTreeWidgetItem* item = currentItem();
    if (!item)
    {
        strLocation = LOCATION_DEFAULT;
        return true;
    }

    bool bFallback = true;

    switch (item->type()) {
    case Category_FolderItem:
        if (WizCategoryViewTrashItem* pItem = currentCategoryItem<WizCategoryViewTrashItem>())
        {
            // only handle group trash
            if (pItem->kbGUID() != m_dbMgr.db().kbGUID())
            {
                WizCategoryViewGroupRootItem* pRItem =
                        dynamic_cast<WizCategoryViewGroupRootItem*>(pItem->parent());
                Q_ASSERT(pRItem);

                strKbGUID = pRItem->kbGUID();
                bFallback = false;

                //set noTag item as current item.
                selectedItems().clear();
                for (int i = 0; i < pRItem->childCount(); i++)
                {
                    if (pRItem->child(i)->type() != Category_GroupNoTagItem)
                        continue;

                    WizCategoryViewGroupNoTagItem* pNoTag =
                            dynamic_cast<WizCategoryViewGroupNoTagItem*>(pRItem->child(i));
                    if (0 != pNoTag)
                    {
                        setCurrentItem(pNoTag);
                        break;
                    }
                }
            }
        }
        else if (WizCategoryViewFolderItem* pItem = currentCategoryItem<WizCategoryViewFolderItem>())
        {
            // only handle individual folders except trash
            if (!WizDatabase::isInDeletedItems(pItem->location())) {
                strLocation = pItem->location();
                bFallback = false;
            }
        }
        break;
    case Category_TagItem:
        if (WizCategoryViewTagItem* pItem = currentCategoryItem<WizCategoryViewTagItem>())
        {
            tag = pItem->tag();
            bFallback = false;
        }
        break;
    case Category_GroupRootItem:
        if (WizCategoryViewGroupRootItem* pItem = currentCategoryItem<WizCategoryViewGroupRootItem>())
        {
            strKbGUID = pItem->kbGUID();
            strLocation = m_dbMgr.db(strKbGUID).getDefaultNoteLocation();
            bFallback = false;
        }
        break;
    case Category_GroupNoTagItem:
        if (WizCategoryViewGroupNoTagItem* pItem = currentCategoryItem<WizCategoryViewGroupNoTagItem>())
        {
            strKbGUID = pItem->kbGUID();
            strLocation = m_dbMgr.db(strKbGUID).getDefaultNoteLocation();
            bFallback = false;
        }
        break;
    case Category_GroupItem:
        if (WizCategoryViewGroupItem* pItem = currentCategoryItem<WizCategoryViewGroupItem>())
        {
            strKbGUID = pItem->kbGUID();
            tag = pItem->tag();
            strLocation = m_dbMgr.db(strKbGUID).getDefaultNoteLocation();
            bFallback = false;
        }
        break;
    case Category_ShortcutItem:
        if (WizCategoryViewShortcutItem* pItem = currentCategoryItem<WizCategoryViewShortcutItem>())
        {
            switch (pItem->shortcutType())
            {
            case WizCategoryViewShortcutItem::PersonalFolder:
                strLocation = pItem->location();
                break;
            case WizCategoryViewShortcutItem::GroupTag:
            {
                strKbGUID = pItem->kbGUID();
                WizDatabase& db = m_dbMgr.db(strKbGUID);
                db.tagFromGuid(pItem->guid(), tag);
                strLocation = db.getDefaultNoteLocation();
            }
                break;
            case WizCategoryViewShortcutItem::PersonalTag:
                m_dbMgr.db().tagFromGuid(pItem->guid(), tag);
                strLocation = LOCATION_DEFAULT;
                break;
            case WizCategoryViewShortcutItem::Document:
            {
                strKbGUID = pItem->kbGUID();
                WizDatabase& db = m_dbMgr.db(strKbGUID);
                //如果是群组笔记，则在该笔记目录下创建新笔记
                if (db.isGroup())
                {
                    CWizTagDataArray arrayTag;
                    db.getDocumentTags(pItem->guid(), arrayTag);
                    if (arrayTag.size() == 1)
                    {
                        tag = arrayTag.front();
                    }
                    strLocation = db.getDefaultNoteLocation();
                }
                else
                {
                    WIZDOCUMENTDATA doc;
                    db.documentFromGuid(pItem->guid(), doc);
                    strLocation = doc.strLocation;
                }
            }
                break;
            }
            bFallback = false;
        }
        break;
    default:
        break;
    }

    if (bFallback) {
        addAndSelectFolder(strLocation);
    }
    //
    WizDatabase& db = m_dbMgr.db(strKbGUID);
    if (db.isGroup()
            && !db.isGroupAuthor())
    {
        return false;
    }

    return true;
}

void WizCategoryView::quickSyncNewDocument(const QString& strKbGUID)
{
    /*NOTE:
     *创建笔记后快速同步笔记到服务器,防止用户新建笔记后使用评论功能时因服务器无该篇笔记导致问题.*/
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
    mainWindow->quickSyncKb(strKbGUID);
}

void WizCategoryView::updateGroupFolderPosition(WizDatabase& db, WizCategoryViewItemBase* pItem)
{
    saveGroupTagsPosition(db.kbGUID());
    db.setGroupTagsPosModified();

    if (pItem->type() == Category_GroupItem)
    {
        WizCategoryViewGroupItem* pGroup = dynamic_cast<WizCategoryViewGroupItem*>(pItem);
        WIZTAGDATA tag = pGroup->tag();

        QTreeWidgetItem* sameNameBrother = findSameNameBrother(pItem->parent(), pItem, pItem->text(0));
        if (sameNameBrother) {

            WizCategoryViewGroupItem* brotherTagItem = dynamic_cast<WizCategoryViewGroupItem*>(sameNameBrother);
            if (brotherTagItem && brotherTagItem->tag().strGUID != tag.strGUID) {

                WizDatabase& db = m_dbMgr.db(tag.strKbGUID);
                bool combineFolder = false;
                if (WizMessageBox::question(0, tr("Info"), tr("Folder '%1' already exists, combine these folders?").arg(tag.strName)) == QMessageBox::Yes)
                {
                    // move documents to brother folder  and move child folders to brother folder
                    WizCategoryViewGroupItem* targetItem = dynamic_cast<WizCategoryViewGroupItem*>(sameNameBrother);
                    combineGroupFolder(pGroup, targetItem);

                    setCurrentItem(targetItem);
                    if (m_dragItem == pItem)
                    {
                        m_dragItem = nullptr;
                    }
                    combineFolder = true;
                }

                if (!combineFolder)
                {
                    QString useableName = getUseableItemName(pGroup->parent(), pGroup);
                    tag.strName = useableName;
                    tag.strParentGUID = "";
                    if (pItem->parent()->type() == Category_GroupItem)
                    {
                        WizCategoryViewGroupItem* parentItem = dynamic_cast<WizCategoryViewGroupItem*>(pGroup->parent());
                        Q_ASSERT(parentItem);
                        tag.strParentGUID = parentItem->tag().strGUID;
                    }
                    tag.nVersion = -1;
                    db.updateTag(tag);
                    pGroup->reload(db);
                }
            }
        }
    }
    //
    emit categoryItemPositionChanged(db.kbGUID());
}

void WizCategoryView::updatePersonalFolderLocation(WizDatabase& db, \
                                                    const QString& strOldLocation, const QString& strNewLocation)
{
    WizCategoryViewAllFoldersItem* pItem = dynamic_cast<WizCategoryViewAllFoldersItem* >(findAllFolderItem());
    if (!pItem)
        return;

    // the last item of folder root should be trash
    WizCategoryViewTrashItem* trashItem = findTrash(db.kbGUID());
    if (trashItem && pItem->indexOfChild(trashItem) != pItem->childCount() - 1)
    {
        pItem->takeChild(pItem->indexOfChild(trashItem));
        pItem->insertChild(pItem->childCount(), trashItem);
    }

    if (strOldLocation != strNewLocation)
    {
        db.updateLocation(strOldLocation, strNewLocation);
        CWizStdStringArray childLocations;
        db.getAllChildLocations(strNewLocation, childLocations);
        for (CString childLocation : childLocations)
        {
            findFolder(childLocation, true, true);
        }
    }

    // 文件夹移动后触发folder loacation changed，需要更新顺序
    QString str = getAllFoldersPosition();
    db.setFoldersPos(str, -1);
    db.setFoldersPosModified();

    emit categoryItemPositionChanged(db.kbGUID());
}

void WizCategoryView::updatePersonalTagPosition()
{
    // not support for customing personal tag position
    return;

    WizDatabase& db = m_dbMgr.db();
    savePersonalTagsPosition();
    db.setGroupTagsPosModified();

    emit categoryItemPositionChanged(db.kbGUID());
}

void WizCategoryView::initGeneral()
{
    loadShortcutState();

    initQuickSearches();
}

void WizCategoryView::sortFolders()
{
    WizCategoryViewAllFoldersItem* pFolderRoot = dynamic_cast<WizCategoryViewAllFoldersItem *>(findAllFolderItem());
    if (!pFolderRoot)
        return;

    pFolderRoot->sortChildren(0, Qt::AscendingOrder);

    for (int i = 0; i < pFolderRoot->childCount(); i++)
    {
        WizCategoryViewFolderItem* pFolder = dynamic_cast<WizCategoryViewFolderItem*>(pFolderRoot->child(i));
        if (!pFolder)
            return;

        sortFolders(pFolder);
    }
}

void WizCategoryView::sortFolders(WizCategoryViewFolderItem* pItem)
{
    if (!pItem)
        return;

    pItem->sortChildren(0, Qt::AscendingOrder);

    for (int i = 1; i < pItem->childCount(); i++)
    {
        WizCategoryViewFolderItem* pFolder = dynamic_cast<WizCategoryViewFolderItem*>(pItem->child(i));
        if (!pFolder)
            return;

        sortFolders(pFolder);
    }
}

void WizCategoryView::sortPersonalTags()
{
    if (WizCategoryViewItemBase* allTagsItem = findAllTagsItem())
    {
        sortPersonalTags(allTagsItem);
    }
}

void WizCategoryView::sortPersonalTags(QTreeWidgetItem* pItem)
{
    if (pItem)
    {
        pItem->sortChildren(0, Qt::AscendingOrder);
        for (int i = 0; i < pItem->childCount(); ++i)
        {
            sortPersonalTags(pItem->child(i));
        }
    }
}

void WizCategoryView::sortGroupTags(const QString& strKbGUID, bool bReloadData)
{
    WizCategoryViewGroupRootItem* groupRoot =
            dynamic_cast<WizCategoryViewGroupRootItem*>(itemFromKbGUID(strKbGUID));
    if (!groupRoot)
        return;

    for (int i = 0; i < groupRoot->childCount(); i++)
    {
        WizCategoryViewGroupItem* pItem = dynamic_cast<WizCategoryViewGroupItem*>(groupRoot->child(i));
        sortGroupTags(pItem, bReloadData);
    }

    groupRoot->sortChildren(0, Qt::AscendingOrder);
}

void WizCategoryView::sortGroupTags(WizCategoryViewGroupItem* pItem, bool bReloadData)
{
    if (!pItem)
        return;

    if (bReloadData)
    {
        WizDatabase& db = m_dbMgr.db(pItem->kbGUID());
        pItem->reload(db);
    }

    for (int i = 0; i < pItem->childCount(); i++)
    {
        WizCategoryViewGroupItem* childItem = dynamic_cast<WizCategoryViewGroupItem*>(pItem->child(i));
        sortGroupTags(childItem, bReloadData);
    }

    pItem->sortChildren(0, Qt::AscendingOrder);
}

void WizCategoryView::savePersonalTagsPosition()
{
    WizCategoryViewAllTagsItem* rootItem =
            dynamic_cast<WizCategoryViewAllTagsItem*>(findAllTagsItem());
    if (!rootItem)
        return;

    WizDatabase& db = m_dbMgr.db();
    db.blockSignals(true);
    for (int i = 0; i < rootItem->childCount(); i++)
    {
        WizCategoryViewTagItem* childItem = dynamic_cast<WizCategoryViewTagItem* >(rootItem->child(i));
        if (childItem)
        {
            childItem->setTagPosition(rootItem->indexOfChild(childItem) + 1);
            savePersonalTagsPosition(db, childItem);
        }
    }

    db.blockSignals(false);
}

void WizCategoryView::savePersonalTagsPosition(WizDatabase& db, WizCategoryViewTagItem* pItem)
{
    if (!pItem)
        return;

    WIZTAGDATA tag = pItem->tag();
    db.modifyTag(tag);

    for (int i = 0; i < pItem->childCount(); i++)
    {
        WizCategoryViewTagItem* childItem = dynamic_cast<WizCategoryViewTagItem* >(pItem->child(i));
        if (childItem)
        {
            childItem->setTagPosition(i + 1);
            savePersonalTagsPosition(db, childItem);
        }
    }
}

/*
 *
 *父子结构也可能变化了，需要更新父子结构
 *
 */
void WizCategoryView::saveGroupTagsPosition(const QString& strKbGUID)
{
    WizCategoryViewGroupRootItem* rootItem =
            dynamic_cast<WizCategoryViewGroupRootItem*>(itemFromKbGUID(strKbGUID));
    if (!rootItem)
        return;

    WizDatabase& db = m_dbMgr.db(strKbGUID);
    db.blockSignals(true);
    for (int i = 0; i < rootItem->childCount(); i++)
    {
        WizCategoryViewGroupItem* childItem = dynamic_cast<WizCategoryViewGroupItem* >(rootItem->child(i));
        if (childItem)
        {
            childItem->setTagPosition(rootItem->indexOfChild(childItem) + 1);
            saveGroupTagsPosition(db, childItem);
        }
    }

    db.blockSignals(false);
}

void WizCategoryView::saveGroupTagsPosition(WizDatabase& db, WizCategoryViewGroupItem* pItem)
{
    if (!pItem)
        return;
    if (pItem->willBeDeleted())
        return;

    WIZTAGDATA tag = pItem->tag();
    db.modifyTagPosition(tag);
    //
    QTreeWidgetItem* parent = pItem->parent();
    if (WizCategoryViewGroupItem* parentTag = dynamic_cast<WizCategoryViewGroupItem*>(parent)) {
        //
        if (pItem->tag().strParentGUID != parentTag->tag().strGUID) {
            tag.strParentGUID = parentTag->tag().strGUID;
            db.modifyTag(tag);
        }

    } else if (WizCategoryViewGroupRootItem* group = dynamic_cast<WizCategoryViewGroupRootItem*>(parent)) {
        //
        if (!pItem->tag().strParentGUID.isEmpty()) {
            tag.strParentGUID = "";
            db.modifyTag(tag);
        }
    }

    for (int i = 0; i < pItem->childCount(); i++)
    {
        WizCategoryViewGroupItem* childItem = dynamic_cast<WizCategoryViewGroupItem* >(pItem->child(i));
        if (childItem)
        {
            childItem->setTagPosition(i + 1);
            saveGroupTagsPosition(db, childItem);
        }
    }
}

QString WizCategoryView::getAllFoldersPosition()
{
    WizCategoryViewAllFoldersItem* pItem =
            dynamic_cast<WizCategoryViewAllFoldersItem*>(findAllFolderItem());
    if (!pItem)
        return QString();

    Json::Value jValue;

    int nStartPos = 1;
    for (int i = 0; i < pItem->childCount(); i++)
    {
        WizCategoryViewFolderItem* childItem = dynamic_cast<WizCategoryViewFolderItem* >(pItem->child(i));
        Q_ASSERT(childItem);
        getAllFoldersPosition(childItem, nStartPos, jValue);
    }
    return QString::fromStdString(jValue.toStyledString());
}

void WizCategoryView::getAllFoldersPosition(WizCategoryViewFolderItem* pItem, int& nStartPos, Json::Value& jValue)
{
    if (!pItem)
        return;
    //
    if (pItem->willBeDeleted()) {
        return;
    }

    jValue[pItem->location().toStdString()] = nStartPos;
    nStartPos ++;

    for (int i = 0; i < pItem->childCount(); i++)
    {
        WizCategoryViewFolderItem* childItem = dynamic_cast<WizCategoryViewFolderItem* >(pItem->child(i));
        Q_ASSERT(childItem);
        getAllFoldersPosition(childItem, nStartPos, jValue);
    }
}

void WizCategoryView::initFolders()
{       
    WizCategoryViewItemBase* pAllFoldersItem = findAllFolderItem();
    if (!pAllFoldersItem)
    {
#ifdef QT_DEBUG
        Q_ASSERT(0);
#endif
        return;
    }

    CWizStdStringArray arrayAllLocation;
    WizDatabase& newDB = m_dbMgr.db();
    newDB.getAllLocationsWithExtra(arrayAllLocation);
    if (arrayAllLocation.empty()) {
        arrayAllLocation.push_back(m_dbMgr.db().getDefaultNoteLocation());
    }

    doLocationSanityCheck(arrayAllLocation);

    initFolders(pAllFoldersItem, QString(), arrayAllLocation);
    WizCategoryViewTrashItem* pTrash = new WizCategoryViewTrashItem(m_app, m_dbMgr.db().kbGUID());
    pAllFoldersItem->addChild(pTrash);

    // push back folders cache
    for (CWizStdStringArray::const_iterator it = arrayAllLocation.begin();
         it != arrayAllLocation.end();
         it++) {
        newDB.addExtraFolder(*it);
    }

    WizExecuteOnThread(WIZ_THREAD_MAIN, [=](){
        pAllFoldersItem->setExpanded(true);
        updatePersonalFolderDocumentCount();
        // load expand state
        if (m_dbMgr.count() == 0)
        {
            on_initGroupFolder_finished();
        }
    });
}

void WizCategoryView::initFolders(QTreeWidgetItem* pParent, \
                                   const QString& strParentLocation, \
                                   const CWizStdStringArray& arrayAllLocation)
{
    CWizStdStringArray arrayLocation;
    WizDatabase::getChildLocations(arrayAllLocation, strParentLocation, arrayLocation);

    CWizStdStringArray::const_iterator it;
    for (it = arrayLocation.begin(); it != arrayLocation.end(); it++) {
        CString strLocation = *it;

        if (strLocation.isEmpty())
            continue;

        if (m_dbMgr.db().isInDeletedItems(strLocation))
            continue;

        WizCategoryViewFolderItem* pFolderItem = new WizCategoryViewFolderItem(m_app, strLocation, m_dbMgr.db().kbGUID());
        pParent->addChild(pFolderItem);

        initFolders(pFolderItem, strLocation, arrayAllLocation);
    }
}

void WizCategoryView::doLocationSanityCheck(CWizStdStringArray& arrayLocation)
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
                if (str.isEmpty()) {
                    qDebug() << "[doLocationSanityCheck]Invalid folder name: " << strLocation;
                    // remove from array
//                    arrayLocation.erase(arrayLocation.begin() + i);
                    break;
                }

                //
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


void WizCategoryView::loadShortcutState()
{   
    QString strData = m_dbMgr.db().getFavorites();
    initShortcut(strData);

    WizCategoryViewShortcutRootItem* pShortcutRoot = dynamic_cast<WizCategoryViewShortcutRootItem*>(findAllShortcutItem());
    if (pShortcutRoot->childCount() == 0)
    {
        pShortcutRoot->addPlaceHoldItem();
    }    
}

void WizCategoryView::saveShortcutState()
{
    WizCategoryViewItemBase *pShortcutRoot = findAllShortcutItem();
    QString strShortcutData = "";

    if (pShortcutRoot && pShortcutRoot->childCount() > 0)
    {
        for (int i = 0; i < pShortcutRoot->childCount(); i++)
        {
            WizCategoryViewShortcutItem *pItem = dynamic_cast<WizCategoryViewShortcutItem*>(pShortcutRoot->child(i));
            if (!pItem)
                return;
            //
            switch (pItem->shortcutType())
            {
            case WizCategoryViewShortcutItem::Document:
            {
                WizDatabase& db = m_dbMgr.db(pItem->kbGUID());
                ///Type=document /KbGUID= /DocumentGUID=10813667-7c9e-46cc-9896-a278462793cc
                strShortcutData = strShortcutData +  "*" + SHORTCUT_TYPE_DOCUMENT + " " + SHORTCUT_PARAM_KBGUID + (db.isGroup() ? pItem->kbGUID() : QString())  + " "
                        + SHORTCUT_PARAM_DOCUMENTGUID + pItem->guid();
            }
                break;
            case WizCategoryViewShortcutItem::PersonalTag:
            {
                // */Type=tag /KbGUID= /TagGUID=8188524c-767f-4d79-8a06-806e5787f49a
                 strShortcutData = strShortcutData + "*" + SHORTCUT_TYPE_TAG + " " + SHORTCUT_PARAM_KBGUID + " "
                         + SHORTCUT_PARAM_TAGGUID + pItem->guid();
            }
                break;
            case WizCategoryViewShortcutItem::GroupTag:
            {
                // */Type=tag /KbGUID= /TagGUID=8188524c-767f-4d79-8a06-806e5787f49a
                 strShortcutData = strShortcutData + "*" + SHORTCUT_TYPE_TAG + " " + SHORTCUT_PARAM_KBGUID + pItem->kbGUID() + " "
                         + SHORTCUT_PARAM_TAGGUID + pItem->guid();
            }
                break;
            case WizCategoryViewShortcutItem::PersonalFolder:
            {
                //  /Type=folder /Location=/abcd/gerger/
                strShortcutData = strShortcutData + "*" + SHORTCUT_TYPE_FOLDER + " " + SHORTCUT_PARAM_LOCATION + pItem->location();
            }
                break;
            }
        }
        //
        if (strShortcutData.length() > 0)
            strShortcutData.remove(0, 1);
    }
    qDebug() << "short cut data : " << strShortcutData;
    m_dbMgr.db().setFavorites(strShortcutData, -1);
}

void WizCategoryView::loadExpandState()
{
    QSettings* settings = WizGlobal::settings();
    settings->beginGroup(TREEVIEW_STATE);
    m_strSelectedId = selectedId(settings);

    for (int i = 0 ; i < topLevelItemCount(); i++) {
        loadChildState(topLevelItem(i), settings);
    }
    settings->endGroup();
}

void WizCategoryView::initTags()
{        
    WizCategoryViewItemBase* pAllTagsItem = findAllTagsItem();

    if (!pAllTagsItem)
    {
#ifdef QT_DEBUG
        Q_ASSERT(0);
#endif
        return;
    }

    initTags(pAllTagsItem, "");

    WizExecuteOnThread(WIZ_THREAD_MAIN, [=](){
        pAllTagsItem->setExpanded(true);
        updatePersonalTagDocumentCount();
    });
}

void WizCategoryView::initTags(QTreeWidgetItem* pParent, const QString& strParentTagGUID)
{
    CWizTagDataArray arrayTag;
    m_dbMgr.db().getChildTags(strParentTagGUID, arrayTag);

    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        WizCategoryViewTagItem* pTagItem = new WizCategoryViewTagItem(m_app, *it, m_dbMgr.db().kbGUID());
        pParent->addChild(pTagItem);

        initTags(pTagItem, it->strGUID);
    }
}

void WizCategoryView::initStyles()
{
    //CWizCategoryViewStyleRootItem* pStyleRoot = new CWizCategoryViewStyleRootItem(m_app, CATEGORY_STYLES);
    //addTopLevelItem(pStyleRoot);
    //pStyleRoot->setHidden(true);
}

void WizCategoryView::initGroups()
{   
    int nTotal = m_dbMgr.count();
    for (int i = 0; i < nTotal; i++) {
        initGroup(m_dbMgr.at(i));        
    }
    //
//    for (std::vector<CWizCategoryViewItemBase*>::const_iterator it = arrayGroupsItem.begin();
//         it != arrayGroupsItem.end();
//         it++)
//    {
//        CWizCategoryViewItemBase* pItem = *it;
//        pItem->sortChildren(0, Qt::AscendingOrder);
//    }
    //
}

void WizCategoryView::initBiz(const WIZBIZDATA& biz)
{
    WizCategoryViewBizGroupRootItem* pBizGroupItem = new WizCategoryViewBizGroupRootItem(m_app, biz);
    setBizRootItemExtraButton(pBizGroupItem, biz);
    addTopLevelItem(pBizGroupItem);

    CWizGroupDataArray arrayGroup;
    m_dbMgr.db().getAllGroupInfo(arrayGroup);

    //
    int nTotal = arrayGroup.size();
    for (int i = 0; i < nTotal; i++)
    {
        if (arrayGroup.at(i).bizGUID == biz.bizGUID)
        {
            initGroup(m_dbMgr.db(arrayGroup.at(i).strGroupGUID));
            updateGroupFolderDocumentCount(arrayGroup.at(i).strGroupGUID);
        }
    }
    //
    pBizGroupItem->setExpanded(true);
    pBizGroupItem->sortChildren(0, Qt::AscendingOrder);
    //
    resetCreateGroupLink();
}

void WizCategoryView::resetCreateGroupLink()
{
    bool hasGroup = false;
    int createLinkIndex = -1;
    for (int i = 0; i < topLevelItemCount(); i++)
    {
        if (NULL != dynamic_cast<WizCategoryViewCreateGroupLinkItem *>(topLevelItem(i)))
        {
            createLinkIndex = i;
        }
        else if (NULL != dynamic_cast<WizCategoryViewGroupsRootItem *>(topLevelItem(i)))
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
            WizCategoryViewCreateGroupLinkItem* pItem = new WizCategoryViewCreateGroupLinkItem(m_app, tr("Create Team for Free..."), LINK_COMMAND_ID_CREATE_GROUP);
            addTopLevelItem(pItem);
        }
    }
}

void WizCategoryView::initGroup(WizDatabase& db)
{
    bool itemCreeated = false;
    initGroup(db, itemCreeated);
}

typedef std::multimap<CString, WIZTAGDATA>::const_iterator mapTagIterator;

void WizCategoryView::initGroup(WizDatabase& db, bool& itemCreeated)
{
    itemCreeated = false;
    if (findGroup(db.kbGUID()))
        return;
    //
    WIZGROUPDATA group;
    m_dbMgr.db().getGroupData(db.kbGUID(), group);
    //
    //
    QTreeWidgetItem* pRoot = findGroupsRootItem(group);
    if (!pRoot) {
        return;
    }

    itemCreeated = true;
    //
    WizCategoryViewGroupRootItem* pGroupItem = new WizCategoryViewGroupRootItem(m_app, group);
    pRoot->addChild(pGroupItem);

    //
    setGroupRootItemExtraButton(pGroupItem, group);

    //
    QString strKbGUID = db.kbGUID();
    std::multimap<CString, WIZTAGDATA> mapTag;
    WizDatabase& newDb = m_dbMgr.db(strKbGUID);
    newDb.getAllTags(mapTag);
    initGroup(newDb, pGroupItem, "", mapTag);

    WizExecuteOnThread(WIZ_THREAD_MAIN, [=](){
        on_initGroupFolder_finished();

        updateGroupFolderDocumentCount(strKbGUID);
    });


    WizCategoryViewGroupNoTagItem* pGroupNoTagItem = new WizCategoryViewGroupNoTagItem(m_app, newDb.kbGUID());
    pGroupItem->addChild(pGroupNoTagItem);

    WizCategoryViewTrashItem* pTrashItem = new WizCategoryViewTrashItem(m_app, newDb.kbGUID());
    pGroupItem->addChild(pTrashItem);

    WizExecuteOnThread(WIZ_THREAD_MAIN, [=](){
        // only show trash if permission is enough
        WizDatabase& newDb = m_dbMgr.db(strKbGUID);
        if (newDb.permission() > WIZ_USERGROUP_SUPER)
        {
            pTrashItem->setHidden(true);
        }
    });
}

void WizCategoryView::initGroup(WizDatabase& db, QTreeWidgetItem* pParent, const QString& strParentTagGUID,
                                 const std::multimap<CString, WIZTAGDATA>& mapTag)
{
    std::pair<mapTagIterator, mapTagIterator> itPair = mapTag.equal_range(strParentTagGUID);
    mapTagIterator it;
    for (it = itPair.first; it != itPair.second; ++it)
    {
        WizCategoryViewGroupItem* pTagItem = new WizCategoryViewGroupItem(m_app, (*it).second, db.kbGUID());
        pParent->addChild(pTagItem);
        initGroup(db, pTagItem, (*it).second.strGUID, mapTag);
    }
}

void WizCategoryView::initQuickSearches()
{   
    WizCategoryViewItemBase* pSearchRoot = findAllSearchItem();
    if (!pSearchRoot)
    {
#ifdef QT_DEBUG
        Q_ASSERT(0);
#endif
        return;
    }

//    CWizCategoryViewSearchItem* pSearchByAttributes = new CWizCategoryViewSearchItem(m_app, tr("Search by Attributes(Personal Notes)"));
    WizCategoryViewSearchItem* pSearchByDTCreated = new WizCategoryViewSearchItem(m_app, tr("Search by Date Created"));
    WizCategoryViewSearchItem* pSearchByDTModified = new WizCategoryViewSearchItem(m_app, tr("Search by Date Modified"));
    WizCategoryViewSearchItem* pSearchByDTAccessed = new WizCategoryViewSearchItem(m_app, tr("Search by Date Accessed"));

//    pSearchRoot->addChild(pSearchByAttributes);
    pSearchRoot->addChild(pSearchByDTCreated);
    pSearchRoot->addChild(pSearchByDTModified);
    pSearchRoot->addChild(pSearchByDTAccessed);

    WizCategoryViewTimeSearchItem* pCreatedToday = new WizCategoryViewTimeSearchItem(m_app, tr("Created since Today"), "DT_CREATED > %1", DateInterval_Today);
    WizCategoryViewTimeSearchItem* pCreatedYesterday = new WizCategoryViewTimeSearchItem(m_app, tr("Created since Yesterday"), "DT_CREATED > %1", DateInterval_Yesterday);
    WizCategoryViewTimeSearchItem* pCreatedDayBeforeYesterday = new WizCategoryViewTimeSearchItem(m_app, tr("Created since the day before yesterday"), "DT_CREATED > %1", DateInterval_TheDayBeforeYesterday);
    WizCategoryViewTimeSearchItem* pCreatedOneWeek = new WizCategoryViewTimeSearchItem(m_app, tr("Created since one week"), "DT_CREATED > %1", DateInterval_LastWeek);
    WizCategoryViewTimeSearchItem* pCreatedOneMonth = new WizCategoryViewTimeSearchItem(m_app, tr("Created since one month"), "DT_CREATED > %1", DateInterval_LastMonth);
    pSearchByDTCreated->addChild(pCreatedToday);
    pSearchByDTCreated->addChild(pCreatedYesterday);
    pSearchByDTCreated->addChild(pCreatedDayBeforeYesterday);
    pSearchByDTCreated->addChild(pCreatedOneWeek);
    pSearchByDTCreated->addChild(pCreatedOneMonth);

    WizCategoryViewTimeSearchItem* pModifiedToday = new WizCategoryViewTimeSearchItem(m_app, tr("Modified since Today"), "DT_MODIFIED > %1", DateInterval_Today);
    WizCategoryViewTimeSearchItem* pModifiedYesterday = new WizCategoryViewTimeSearchItem(m_app, tr("Modified since Yesterday"), "DT_MODIFIED > %1", DateInterval_Yesterday);
    WizCategoryViewTimeSearchItem* pModifiedDayBeforeYesterday = new WizCategoryViewTimeSearchItem(m_app, tr("Modified since the day before yesterday"), "DT_MODIFIED > %1", DateInterval_TheDayBeforeYesterday);
    WizCategoryViewTimeSearchItem* pModifiedOneWeek = new WizCategoryViewTimeSearchItem(m_app, tr("Modified since one week"), "DT_MODIFIED > %1", DateInterval_LastWeek);
    WizCategoryViewTimeSearchItem* pModifiedOneMonth = new WizCategoryViewTimeSearchItem(m_app, tr("Modified since one month"), "DT_MODIFIED > %1", DateInterval_LastMonth);
    pSearchByDTModified->addChild(pModifiedToday);
    pSearchByDTModified->addChild(pModifiedYesterday);
    pSearchByDTModified->addChild(pModifiedDayBeforeYesterday);
    pSearchByDTModified->addChild(pModifiedOneWeek);
    pSearchByDTModified->addChild(pModifiedOneMonth);

    WizCategoryViewTimeSearchItem* pAccessedToday = new WizCategoryViewTimeSearchItem(m_app, tr("Accessed since Today"), "DT_ACCESSED > %1", DateInterval_Today);
    WizCategoryViewTimeSearchItem* pAccessedYesterday = new WizCategoryViewTimeSearchItem(m_app, tr("Accessed since Yesterday"), "DT_ACCESSED > %1", DateInterval_Yesterday);
    WizCategoryViewTimeSearchItem* pAccessedDayBeforeYesterday = new WizCategoryViewTimeSearchItem(m_app, tr("Accessed since the day before yesterday"), "DT_ACCESSED > %1", DateInterval_TheDayBeforeYesterday);
    WizCategoryViewTimeSearchItem* pAccessedOneWeek = new WizCategoryViewTimeSearchItem(m_app, tr("Accessed since one week"), "DT_ACCESSED > %1", DateInterval_LastWeek);
    WizCategoryViewTimeSearchItem* pAccessedOneMonth = new WizCategoryViewTimeSearchItem(m_app, tr("Accessed since one month"), "DT_ACCESSED > %1", DateInterval_LastMonth);
    pSearchByDTAccessed->addChild(pAccessedToday);
    pSearchByDTAccessed->addChild(pAccessedYesterday);
    pSearchByDTAccessed->addChild(pAccessedDayBeforeYesterday);
    pSearchByDTAccessed->addChild(pAccessedOneWeek);
    pSearchByDTAccessed->addChild(pAccessedOneMonth);

    QMap<QString, QString> customMap;
    loadCustomAdvancedSearchParamFromDB(customMap);
    if (customMap.count() > 0)
    {
        WizCategoryViewSearchItem* pSearchByCustomSQL = new WizCategoryViewSearchItem(m_app, CATEGORY_SEARCH_BYCUSTOM);
        pSearchRoot->addChild(pSearchByCustomSQL);
        QMap<QString, QString>::Iterator it;
        for (it = customMap.begin(); it != customMap.end(); it++)
        {
            QString where, name, keyword;
            int scope;
            WizAdvancedSearchDialog::paramToSQL(it.value(), where, keyword, name, scope);
            WizCategoryViewCustomSearchItem* pCustomSearch = new WizCategoryViewCustomSearchItem(m_app,
                                                                                                   name, it.value(), where, it.key(), keyword, scope);
            pSearchByCustomSQL->addChild(pCustomSearch);
        }
    }
}

void WizCategoryView::initShortcut(const QString& shortcut)
{
    WizCategoryViewItemBase* pShortcutRoot = findAllShortcutItem();
    if (!pShortcutRoot)
    {
#ifdef QT_DEBUG
        Q_ASSERT(0);
#endif
        return;
    }

    //
    QStringList shortcutList = shortcut.split("*", QString::SkipEmptyParts);
    for (QString param : shortcutList)
    {
        QStringList paramList = param.split(" ", QString::SkipEmptyParts);
        if (paramList.count() < 2)
        {
            qDebug() << "Invalid shortcut data : " << paramList;
            continue;
        }

        //
        QString type = paramList.first();
        if (type == SHORTCUT_TYPE_FOLDER)
        {
            QString location = param;
            location = location.remove(SHORTCUT_TYPE_FOLDER + QString(" ") + SHORTCUT_PARAM_LOCATION);
            QString name = WizDatabase::getLocationName(location);
            WizCategoryViewShortcutItem* item = new WizCategoryViewShortcutItem(m_app,
                                                                                  name, WizCategoryViewShortcutItem::PersonalFolder,
                                                                                  "", "", location);
            pShortcutRoot->addChild(item);
        }
        else if (type == SHORTCUT_TYPE_TAG)
        {
            QString kbGuid = paramList.at(1);
            QString guid = paramList.last();
            Q_ASSERT(kbGuid.startsWith(SHORTCUT_PARAM_KBGUID) && guid.startsWith(SHORTCUT_PARAM_TAGGUID));
            kbGuid = kbGuid.remove(SHORTCUT_PARAM_KBGUID);
            guid = guid.remove(SHORTCUT_PARAM_TAGGUID);
            WIZTAGDATA tag;
            if (m_dbMgr.db(kbGuid).tagFromGuid(guid, tag))
            {
                WizCategoryViewShortcutItem* item = new WizCategoryViewShortcutItem(m_app,
                                                                                      tag.strName, m_dbMgr.db(kbGuid).isGroup() ? WizCategoryViewShortcutItem::GroupTag : WizCategoryViewShortcutItem::PersonalTag,
                                                                                      kbGuid, guid, "");
                pShortcutRoot->addChild(item);
            }
        }
        else if (type == SHORTCUT_TYPE_DOCUMENT)
        {
            QString kbGuid = paramList.at(1);
            QString guid = paramList.last();
            Q_ASSERT(kbGuid.startsWith(SHORTCUT_PARAM_KBGUID) && guid.startsWith(SHORTCUT_PARAM_DOCUMENTGUID));
            kbGuid = kbGuid.remove(SHORTCUT_PARAM_KBGUID);
            guid = guid.remove(SHORTCUT_PARAM_DOCUMENTGUID);
            WIZDOCUMENTDATA doc;
            if (m_dbMgr.db(kbGuid).documentFromGuid(guid, doc))
            {
                bool isEncrypted = doc.nProtected == 1;
                WizCategoryViewShortcutItem* item = new WizCategoryViewShortcutItem(m_app,
                                                                                      doc.strTitle, WizCategoryViewShortcutItem::Document,
                                                                                      kbGuid, guid, doc.strLocation, isEncrypted);
                pShortcutRoot->addChild(item);
            }
        }
        else
        {
            qDebug() << "Invalid shortcut type : " << type;
        }
    }    
}

WizCategoryViewItemBase* WizCategoryView::findGroupsRootItem(const WIZGROUPDATA& group, bool bCreate /* = true*/)
{
    qDebug() << "find group root: " << group.strGroupName;
    //
    if (group.isBiz())
    {
        WIZBIZDATA biz;
        if (!m_dbMgr.db().getBizData(group.bizGUID, biz))
            return NULL;
        //
        return findBizGroupsRootItem(biz);
    }
    else
    {
        if (group.isOwn())
            return findOwnGroupsRootItem(bCreate);
        else
            return findJionedGroupsRootItem(bCreate);
    }
}

WizCategoryViewItemBase* WizCategoryView::findBizGroupsRootItem(const WIZBIZDATA& biz, bool bCreate /*= true*/)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        WizCategoryViewBizGroupRootItem* pItem = dynamic_cast<WizCategoryViewBizGroupRootItem*>(topLevelItem(i));
        if (!pItem)
            continue;
        if (pItem->biz().bizGUID == biz.bizGUID)
            return pItem;
    }

    if (!bCreate)
        return NULL;
    //
    WizCategoryViewBizGroupRootItem* pItem = new WizCategoryViewBizGroupRootItem(m_app, biz);;
    addTopLevelItem(pItem);
    //
    resetSections();
    //
    return pItem;
}

WizCategoryViewItemBase* WizCategoryView::findOwnGroupsRootItem(bool bCreate /*= true*/)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        WizCategoryViewOwnGroupRootItem* pItem = dynamic_cast<WizCategoryViewOwnGroupRootItem*>(topLevelItem(i));
        if (!pItem)
            continue;
        //
        return pItem;
    }
    //
    if (!bCreate)
        return NULL;
    //
    WizCategoryViewOwnGroupRootItem* pItem = new WizCategoryViewOwnGroupRootItem(m_app);;
    addTopLevelItem(pItem);
    //
    resetSections();
    return pItem;
}

WizCategoryViewItemBase* WizCategoryView::findJionedGroupsRootItem(bool bCreate /*= true*/)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        WizCategoryViewJionedGroupRootItem* pItem = dynamic_cast<WizCategoryViewJionedGroupRootItem*>(topLevelItem(i));
        if (!pItem)
            continue;
        //
        return pItem;
    }
    //
    if (!bCreate)
        return NULL;
    //
    WizCategoryViewJionedGroupRootItem* pItem = new WizCategoryViewJionedGroupRootItem(m_app);;
    addTopLevelItem(pItem);
    //
    resetSections();
    return pItem;
}

WizCategoryViewItemBase* WizCategoryView::findAllFolderItem()
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        WizCategoryViewAllFoldersItem* pItem = dynamic_cast<WizCategoryViewAllFoldersItem*>(topLevelItem(i));
        if (pItem) {
            return pItem;
        }
    }
    //
    return NULL;
}
WizCategoryViewItemBase* WizCategoryView::findAllTagsItem()
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        WizCategoryViewAllTagsItem* pItem = dynamic_cast<WizCategoryViewAllTagsItem*>(topLevelItem(i));
        if (pItem) {
            return pItem;
        }
    }
    //
    return NULL;
}

WizCategoryViewItemBase*WizCategoryView::findAllSearchItem()
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        if (topLevelItem(i)->type() != Category_QuickSearchRootItem)
            continue;

        WizCategoryViewSearchRootItem* pItem = dynamic_cast<WizCategoryViewSearchRootItem*>(topLevelItem(i));
        if (pItem) {
            return pItem;
        }
    }
    //
    return NULL;
}

WizCategoryViewItemBase* WizCategoryView::findAllMessagesItem()
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        if (topLevelItem(i)->type() != Category_MessageItem)
            continue;

        WizCategoryViewMessageItem* pItem = dynamic_cast<WizCategoryViewMessageItem*>(topLevelItem(i));
        if (pItem) {
            return pItem;
        }
    }
    //
    return NULL;
}

WizCategoryViewGroupRootItem* WizCategoryView::findGroup(const QString& strKbGUID)
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        WizCategoryViewGroupsRootItem* p = dynamic_cast<WizCategoryViewGroupsRootItem *>(topLevelItem(i));

        // only search under all groups root and biz group root
        if (!p)
            continue;

        for (int j = 0; j < p->childCount(); j++) {
            WizCategoryViewGroupRootItem* pGroup = dynamic_cast<WizCategoryViewGroupRootItem *>(p->child(j));
            if (pGroup && (pGroup->kbGUID() == strKbGUID)) {
                return pGroup;
            }
        }
    }

    return NULL;
}

WizCategoryViewTrashItem* WizCategoryView::findTrash(const QString& strKbGUID /* = NULL */)
{
    // personal notes' trash, should exist
    if(strKbGUID.isEmpty() || strKbGUID == m_dbMgr.db().kbGUID()) {
        WizCategoryViewItemBase* pItem = findAllFolderItem();
        if (!pItem) {
            Q_ASSERT(0);
            return NULL;
        }

        for (int i = 0; i < pItem->childCount(); i++) {
            if (WizCategoryViewTrashItem* pTrash = dynamic_cast<WizCategoryViewTrashItem*>(pItem->child(i))) {
                return pTrash;
            }
        }

        return NULL;
    }

    // group's trash, also should exist
    WizCategoryViewGroupRootItem* pGroupItem = findGroup(strKbGUID);
    if (!pGroupItem) {
        return NULL;
    }

    for (int i = 0; i < pGroupItem->childCount(); i++) {
        WizCategoryViewTrashItem * pTrashItem = dynamic_cast<WizCategoryViewTrashItem *>(pGroupItem->child(i));
        if (pTrashItem) {
            return pTrashItem;
        }
    }

    return NULL;
}

void WizCategoryView::addAndSelectFolder(const CString& strLocation)
{
    if (QTreeWidgetItem* pItem = addFolder(strLocation, true)) {
        setCurrentItem(pItem);
    }
}

WizCategoryViewFolderItem* WizCategoryView::findFolder(const QString& strLocation, bool create, bool sort)
{
    WizCategoryViewAllFoldersItem* pAllFolders = dynamic_cast<WizCategoryViewAllFoldersItem *>(findAllFolderItem());
    if (!pAllFolders)
        return NULL;

    if (m_dbMgr.db().isInDeletedItems(strLocation)) {
        return findTrash(m_dbMgr.db().kbGUID());
    }

    QString strCurrentLocation = "/";
    QTreeWidgetItem* parent = pAllFolders;

    CString strTempLocation = strLocation;
    strTempLocation.trim('/');
    QStringList sl = strTempLocation.split("/");

    QStringList::const_iterator it;
    for (it = sl.begin(); it != sl.end(); it++) {
        QString strLocationName = *it;
        if (strLocationName.isEmpty())
            return NULL;
        Q_ASSERT(!strLocationName.isEmpty());
        strCurrentLocation = strCurrentLocation + strLocationName + "/";

        bool found = false;
        int nCount = parent->childCount();
        for (int i = 0; i < nCount; i++)
        {
            WizCategoryViewFolderItem* pFolder = dynamic_cast<WizCategoryViewFolderItem*>(parent->child(i));
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

        WizCategoryViewFolderItem* pFolderItem = createFolderItem(parent, strCurrentLocation);
        //
        CWizStdStringArray arrayAllLocation;
        m_dbMgr.db().getAllLocations(arrayAllLocation);
        initFolders(pFolderItem, pFolderItem->location(), arrayAllLocation);
        if (sort) {
            parent->sortChildren(0, Qt::AscendingOrder);
        }
        parent = pFolderItem;
    }

    return dynamic_cast<WizCategoryViewFolderItem *>(parent);
}

WizCategoryViewFolderItem* WizCategoryView::addFolder(const QString& strLocation, bool sort)
{
    return findFolder(strLocation, true, sort);
}

WizCategoryViewTagItem* WizCategoryView::findTagInTree(const WIZTAGDATA& tag)
{
    WizCategoryViewAllTagsItem* pAllTags = dynamic_cast<WizCategoryViewAllTagsItem*>(findAllTagsItem());
    if (!pAllTags)
        return NULL;

    return findTagInTree(tag, pAllTags);
}

WizCategoryViewTagItem* WizCategoryView::findTagInTree(const WIZTAGDATA& tag,
                                                         QTreeWidgetItem* itemParent)
{
    for (int i = 0; i < itemParent->childCount(); i++) {
        QTreeWidgetItem* it = itemParent->child(i);

        if (WizCategoryViewTagItem* item = dynamic_cast<WizCategoryViewTagItem*>(it)) {
            if (item->tag().strGUID == tag.strGUID)
                return item;
        }

        if (WizCategoryViewTagItem* childItem = findTagInTree(tag, it)) {
            return childItem;
        }
    }

    return NULL;
}


WizCategoryViewTagItem* WizCategoryView::findTag(const WIZTAGDATA& tag, bool create, bool sort)
{
    Q_ASSERT(tag.strKbGUID == m_dbMgr.db().kbGUID());

    CWizStdStringArray arrayGUID;
    if (!m_dbMgr.db().getAllParentsTagGuid(tag.strGUID, arrayGUID))
        return NULL;

    arrayGUID.insert(arrayGUID.begin(), tag.strGUID);   //insert self

    WizCategoryViewAllTagsItem* pAllTags = dynamic_cast<WizCategoryViewAllTagsItem*>(findAllTagsItem());
    if (!pAllTags)
        return NULL;

    QTreeWidgetItem* parent = pAllTags;

    size_t nCount = arrayGUID.size();
    for (intptr_t i = nCount - 1; i >= 0; i--)
    {
        CString strParentTagGUID = arrayGUID[i];

        WIZTAGDATA tagParent;
        if (!m_dbMgr.db().tagFromGuid(strParentTagGUID, tagParent))
            return NULL;

        bool found = false;
        int nCount = parent->childCount();
        for (int i = 0; i < nCount; i++)
        {
            WizCategoryViewTagItem* pTag = dynamic_cast<WizCategoryViewTagItem*>(parent->child(i));
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

        WizCategoryViewTagItem* pTagItem = new WizCategoryViewTagItem(m_app, tagParent, m_dbMgr.db().kbGUID());
        parent->addChild(pTagItem);
        if (sort) {
            parent->sortChildren(0, Qt::AscendingOrder);
        }

        parent = pTagItem;
    }

    return dynamic_cast<WizCategoryViewTagItem *>(parent);
}



WizCategoryViewTagItem* WizCategoryView::addTagWithChildren(const WIZTAGDATA& tag)
{
    WizCategoryViewTagItem* pItem = findTag(tag, true, true);
    if (!pItem)
        return NULL;

    CWizTagDataArray arrayTag;
    m_dbMgr.db().getChildTags(tag.strGUID, arrayTag);

    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        addTagWithChildren(*it);
    }

    return pItem;
}

WizCategoryViewTagItem* WizCategoryView::addTag(const WIZTAGDATA& tag, bool sort)
{
    return findTag(tag, true, sort);
}

WizCategoryViewTagItem* WizCategoryView::addAndSelectTag(const WIZTAGDATA& tag)
{
    if (WizCategoryViewTagItem* pItem = addTag(tag, true)) {
        setCurrentItem(pItem);
        return pItem;
    }

    Q_ASSERT(0);
    return NULL;
}

void WizCategoryView::removeTag(const WIZTAGDATA& tag)
{
    WizCategoryViewTagItem* pItem = findTagInTree(tag);
    if (pItem) {
        QTreeWidgetItem* parent = pItem->parent();
        if (parent) {
            parent->removeChild(pItem);
        }
    }
}

WizCategoryViewGroupItem* WizCategoryView::findGroupFolderInTree(const WIZTAGDATA& tag)
{
    WizCategoryViewGroupRootItem* pItem = findGroup(tag.strKbGUID);
    if (!pItem)
        return NULL;

    return findGroupFolderInTree(tag, pItem);
}

WizCategoryViewGroupItem* WizCategoryView::findGroupFolderInTree(const WIZTAGDATA& tag,
                                                                         QTreeWidgetItem* itemParent)
{
    for (int i = 0; i < itemParent->childCount(); i++) {
        QTreeWidgetItem* it = itemParent->child(i);

        if (WizCategoryViewGroupItem* item = dynamic_cast<WizCategoryViewGroupItem*>(it)) {
            if (item && item->tag().strGUID == tag.strGUID
                    && item->tag().strKbGUID == tag.strKbGUID)
                return item;
        }

        if (WizCategoryViewGroupItem* childItem = findGroupFolderInTree(tag, it)) {
            return childItem;
        }
    }

    return NULL;
}


WizCategoryViewGroupItem* WizCategoryView::findGroupFolder(const WIZTAGDATA& tag,
                                                                   bool create,
                                                                   bool sort)
{
    CWizStdStringArray arrayGUID;
    if (!m_dbMgr.db(tag.strKbGUID).getAllParentsTagGuid(tag.strGUID, arrayGUID))
        return NULL;

    arrayGUID.insert(arrayGUID.begin(), tag.strGUID);   //insert self

    WizCategoryViewGroupRootItem* pItem = findGroup(tag.strKbGUID);
    if (!pItem)
        return NULL;

    QTreeWidgetItem* parent = pItem;

    size_t nCount = arrayGUID.size();
    for (intptr_t i = nCount - 1; i >= 0; i--) {
        CString strParentTagGUID = arrayGUID[i];

        WIZTAGDATA tagParent;
        if (!m_dbMgr.db(tag.strKbGUID).tagFromGuid(strParentTagGUID, tagParent))
            return NULL;

        bool found = false;
        int nCount = parent->childCount();
        for (int i = 0; i < nCount; i++) {
            WizCategoryViewGroupItem* pTag = dynamic_cast<WizCategoryViewGroupItem*>(parent->child(i));
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

        WizCategoryViewGroupItem* pTagItem = new WizCategoryViewGroupItem(m_app, tagParent, tag.strKbGUID);
        parent->addChild(pTagItem);
        if (sort) {
            parent->sortChildren(0, Qt::AscendingOrder);
        }

        parent = pTagItem;
    }

    return dynamic_cast<WizCategoryViewGroupItem *>(parent);
}


WizCategoryViewGroupItem* WizCategoryView::addGroupFolderWithChildren(const WIZTAGDATA& tag)
{
    WizCategoryViewGroupItem* pItem = findGroupFolder(tag, true, true);
    if (!pItem)
        return NULL;

    CWizTagDataArray arrayTag;
    m_dbMgr.db(tag.strKbGUID).getChildTags(tag.strGUID, arrayTag);

    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        addGroupFolderWithChildren(*it);
    }

    return pItem;
}

void WizCategoryView::removeGroupFolder(const WIZTAGDATA& tag)
{
    WizCategoryViewGroupItem* pItem = findGroupFolderInTree(tag);
    if (pItem) {
        QTreeWidgetItem* parent = pItem->parent();
        if (parent) {
            parent->removeChild(pItem);
        }
    }
}

void WizCategoryView::on_document_created(const WIZDOCUMENTDATA& doc)
{
    if (doc.strKbGUID == m_dbMgr.db().kbGUID() || doc.strKbGUID.isEmpty()) {
        // for backward compatibility
        if (!m_dbMgr.db().isInDeletedItems(doc.strLocation)) {
            addFolder(doc.strLocation, true);
        }
        updatePersonalFolderDocumentCount();
        updatePersonalTagDocumentCount();
    }
    else {
        updateGroupFolderDocumentCount(doc.strKbGUID);
    }
}

void WizCategoryView::on_document_modified(const WIZDOCUMENTDATA& docOld, const WIZDOCUMENTDATA& docNew)
{
    Q_UNUSED(docOld);

    if (docNew.strKbGUID == m_dbMgr.db().kbGUID() || docNew.strKbGUID.isEmpty()) {
        // for backward compatibility
        if (!m_dbMgr.db().isInDeletedItems(docNew.strLocation)) {
            addFolder(docNew.strLocation, true);
        }

        updatePersonalFolderDocumentCount();
        updatePersonalTagDocumentCount();
    } else {
        updateGroupFolderDocumentCount(docNew.strKbGUID);
    }

    //
    updateShortcut(WizCategoryViewShortcutItem::Document, docNew.strGUID, docNew.strTitle);
}

void WizCategoryView::on_document_deleted(const WIZDOCUMENTDATA& doc)
{
    Q_UNUSED(doc);

    if (doc.strKbGUID == m_dbMgr.db().kbGUID() || doc.strKbGUID.isEmpty()) {
        updatePersonalFolderDocumentCount();
        updatePersonalTagDocumentCount();
    } else {
        updateGroupFolderDocumentCount(doc.strKbGUID);
    }

    //
    removeShortcut(WizCategoryViewShortcutItem::Document, doc.strGUID);
}

void WizCategoryView::on_document_tag_modified(const WIZDOCUMENTDATA& doc)
{
//    updateTagDocumentCount(doc.strKbGUID);
    Q_UNUSED (doc)
    updatePersonalTagDocumentCount();
}

void WizCategoryView::on_folder_created(const QString& strLocation)
{
    Q_ASSERT(!strLocation.isEmpty());

    addFolder(strLocation, true);
}

void WizCategoryView::on_folder_deleted(const QString& strLocation)
{
    Q_ASSERT(!strLocation.isEmpty());

    bool bDeleted = false;

    if (WizCategoryViewFolderItem* pFolder = findFolder(strLocation, false, false))
    {
        if (QTreeWidgetItem* parent = pFolder->parent())
        {
            parent->removeChild(pFolder);
            bDeleted = true;
        }
    }

    if (!bDeleted)
    {
        qCritical() << "------------***-------------------Folder deleted in db, but can not deleted in category------------***-------------------" << strLocation;
    }

    //
    removeShortcut(WizCategoryViewShortcutItem::PersonalFolder, strLocation);
}

void WizCategoryView::on_folder_positionChanged()
{
    sortFolders();
}

void WizCategoryView::on_tag_created(const WIZTAGDATA& tag)
{
    if (tag.strKbGUID == m_dbMgr.db().kbGUID()) {
        addTagWithChildren(tag);
        updatePersonalTagDocumentCount();
    } else {
        WizCategoryViewGroupItem* pTagItem = addGroupFolderWithChildren(tag);
        if (pTagItem) {
            pTagItem->parent()->sortChildren(0, Qt::AscendingOrder);
            updateGroupFolderDocumentCount(tag.strKbGUID);
        }
    }
}

void WizCategoryView::on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew)
{
    if (tagNew.strKbGUID == m_dbMgr.db().kbGUID()) {
        if (tagOld.strParentGUID != tagNew.strParentGUID) {
            removeTag(tagOld);
        }

        WizCategoryViewTagItem* pTagItem = addTagWithChildren(tagNew);
        if (pTagItem) {
            pTagItem->reload(m_dbMgr.db());
        }

        updatePersonalTagDocumentCount();
        //
        updateShortcut(WizCategoryViewShortcutItem::PersonalTag, tagNew.strGUID, tagNew.strName);
    } else {
        if (tagOld.strParentGUID != tagNew.strParentGUID) {
            removeGroupFolder(tagOld);
        }

        WizCategoryViewGroupItem* pTagItem = addGroupFolderWithChildren(tagNew);
        if (pTagItem) {
            pTagItem->reload(m_dbMgr.db(tagNew.strKbGUID));
            pTagItem->parent()->sortChildren(0, Qt::AscendingOrder);
        }
        updateGroupFolderDocumentCount(tagNew.strKbGUID);
        //
        updateShortcut(WizCategoryViewShortcutItem::GroupTag, tagNew.strGUID, tagNew.strName);
    }
}

void WizCategoryView::on_tag_deleted(const WIZTAGDATA& tag)
{
    if (tag.strKbGUID == m_dbMgr.db().kbGUID()) {
        removeTag(tag);
        updatePersonalTagDocumentCount();
        //
        removeShortcut(WizCategoryViewShortcutItem::PersonalTag, tag.strGUID);
    } else {
        removeGroupFolder(tag);
        updateGroupFolderDocumentCount(tag.strKbGUID);
        //
        removeShortcut(WizCategoryViewShortcutItem::GroupTag, tag.strGUID);
    }
}

void WizCategoryView::on_tags_positionChanged(const QString& strKbGUID)
{
    bool reloadData = true;
    sortGroupTags(strKbGUID, reloadData);
}

void WizCategoryView::on_group_opened(const QString& strKbGUID)
{
    Q_ASSERT(!strKbGUID.isEmpty());

    bool itemCreated = false;
    initGroup(m_dbMgr.db(strKbGUID), itemCreated);
    //
    if (itemCreated)
    {
        WizCategoryViewGroupRootItem* pItem = findGroup(strKbGUID);
        if (pItem) {
            QTreeWidgetItem* parent = pItem->parent();
            if (parent) {
                if (parent->childCount() >= 2) {
                    parent->sortChildren(0, Qt::AscendingOrder);
                }
            }

            resetCreateGroupLink();
        }
    }
}

void WizCategoryView::on_group_closed(const QString& strKbGUID)
{
    Q_ASSERT(!strKbGUID.isEmpty());

    WizCategoryViewGroupRootItem* pItem = findGroup(strKbGUID);
    if (pItem) {
        QTreeWidgetItem* parent = pItem->parent();
        if (parent) {
            parent->removeChild(pItem);
        }
    }
}

void WizCategoryView::on_group_renamed(const QString& strKbGUID)
{
    Q_ASSERT(!strKbGUID.isEmpty());

    WizCategoryViewGroupRootItem* pItem = findGroup(strKbGUID);
    if (pItem) {
        pItem->reload(m_dbMgr.db(strKbGUID));
    }
}

QAction* WizCategoryView::findAction(CategoryActions type)
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

WizCategoryViewItemBase*WizCategoryView::findAllShortcutItem()
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        if (topLevelItem(i)->type() != Category_ShortcutRootItem)
            continue;

        WizCategoryViewShortcutRootItem* pItem = dynamic_cast<WizCategoryViewShortcutRootItem*>(topLevelItem(i));
        if (pItem) {
            return pItem;
        }
    }
    //
    return nullptr;
}

void WizCategoryView::on_group_permissionChanged(const QString& strKbGUID)
{
    // only reset action if item is selected
    WizCategoryViewItemBase* pItem = currentCategoryItem<WizCategoryViewItemBase>();
    if (!pItem)
        return;

    if (pItem->kbGUID() != strKbGUID)
        return;

    int nPerm = m_dbMgr.db(strKbGUID).permission();

    // only Admin and Super user see trash folder and operate with tag (group folder)
    if (nPerm > WIZ_USERGROUP_SUPER) {
        WizCategoryViewTrashItem* pItem =  findTrash(strKbGUID);
        if (pItem) pItem->setHidden(true);

        findAction(ActionNewItem)->setEnabled(false);
        findAction(ActionRenameItem)->setEnabled(false);
        findAction(ActionDeleteItem)->setEnabled(false);

        findAction(ActionCopyItem)->setEnabled(false);
        findAction(ActionMoveItem)->setEnabled(false);
    } else {
        WizCategoryViewTrashItem* pItem = findTrash(strKbGUID);
        if (pItem) pItem->setHidden(false);

        findAction(ActionNewItem)->setEnabled(true);
        findAction(ActionRenameItem)->setEnabled(true);
        findAction(ActionDeleteItem)->setEnabled(true);

        findAction(ActionCopyItem)->setEnabled(true);
        findAction(ActionMoveItem)->setEnabled(true);
    }

    // permission greater than author can create new document
    if (nPerm >= WIZ_USERGROUP_READER) {
        findAction(ActionNewDocument)->setEnabled(false);
    } else {
        findAction(ActionNewDocument)->setEnabled(true);
    }

    if (nPerm >= WIZ_USERGROUP_READER) {
        findAction(ActionImportFile)->setEnabled(false);
    } else {
        findAction(ActionImportFile)->setEnabled(true);
    }
}

void WizCategoryView::on_group_bizChanged(const QString& strKbGUID)
{
    //TODO:
}

void WizCategoryView::on_groupDocuments_unreadCount_modified(const QString& strKbGUID)
{
    updateGroupFolderDocumentCount_impl(strKbGUID);
}

void WizCategoryView::on_itemPosition_changed(WizCategoryViewItemBase* pItem)
{
    if (!pItem) {
        return;
    }
    //
    QTreeWidgetItem* parentItem = pItem->parent();
    if (!parentItem)
    {
        return;
    }
    //
    qDebug() << "category item position changed, try to update item position data, item text : " << pItem->text(0);
    WizDatabase& db = m_dbMgr.db(pItem->kbGUID());
    if (db.isGroup())
    {
        updateGroupFolderPosition(db, pItem);
    }
    else
    {
        if (pItem->type() == Category_FolderItem)
        {
            WizCategoryViewFolderItem* item = dynamic_cast<WizCategoryViewFolderItem*>(pItem);
            Q_ASSERT(item);
            //
            resetFolderLocation(item);
            sortFolders();
        }
        else if (pItem->type() == Category_TagItem)
        {
            updatePersonalTagPosition();
            WizCategoryViewItemBase* allTags = findAllTagsItem();
            if (allTags)
            {
                allTags->sortChildren(0, Qt::AscendingOrder);
            }
        }

    }
}

void WizCategoryView::on_importFile_finished(bool ok, QString text, QString kbGuid)
{
    if (ok)
    {
        quickSyncNewDocument(kbGuid);
    }
    else
    {
        WizMessageBox::information(nullptr, tr("Info"), text);
    }
}

WizFolder* WizCategoryView::SelectedFolder()
{
    QList<QTreeWidgetItem*> items = selectedItems();
    if (items.empty())
        return NULL;

    WizCategoryViewFolderItem* pItem = dynamic_cast<WizCategoryViewFolderItem*>(items.first());
    if (!pItem)
        return NULL;

    return new WizFolder(m_dbMgr.db(), pItem->location());
}

void WizCategoryView::loadChildState(QTreeWidgetItem* pItem, QSettings* settings)
{
    loadItemState(pItem, settings);

    if (!m_strSelectedId.isEmpty())
    {
        WizCategoryViewItemBase* pi = dynamic_cast<WizCategoryViewItemBase*>(pItem);
        if (!pi)
            return;

        if (pi->id() == m_strSelectedId)
        {
            setCurrentItem(pItem);
        }
    }
    else
    {
        WizCategoryViewItemBase* pi = findAllFolderItem();
        setCurrentItem(pi);
    }

    for (int i = 0; i < pItem->childCount(); i++)
    {
        loadChildState(pItem->child(i), settings);
    }
}

void WizCategoryView::loadItemState(QTreeWidgetItem* pi, QSettings* settings)
{
    if (!pi || !settings)
        return;

    WizCategoryViewItemBase* pItem = dynamic_cast<WizCategoryViewItemBase*>(pi);
    if (!pItem)
        return;

    QString strId = pItem->id();

    bool bExpand = settings->value(strId).toBool();

    if (bExpand)
        expandItem(pItem);
    else
        collapseItem(pItem);
}

void WizCategoryView::saveExpandState()
{
    QSettings* settings = WizGlobal::settings();
    settings->beginGroup(TREEVIEW_STATE);
    settings->remove("");
    for (int i = 0 ; i < topLevelItemCount(); i++) {
        saveChildState(topLevelItem(i), settings);
    }

    saveSelected(settings);
    settings->endGroup();

    settings->sync();
}

QString WizCategoryView::selectedId(QSettings* settings)
{
    QString strItem = settings->value(TREEVIEW_SELECTED_ITEM).toString();

    return strItem;
}

void WizCategoryView::saveChildState(QTreeWidgetItem* pItem, QSettings* settings)
{
    saveItemState(pItem, settings);

    for (int i = 0; i < pItem->childCount(); i++) {
        saveChildState(pItem->child(i), settings);
    }
}

void WizCategoryView::saveItemState(QTreeWidgetItem* pi, QSettings *settings)
{
   if (!pi || !settings)
       return;

   WizCategoryViewItemBase* pItem = dynamic_cast<WizCategoryViewItemBase*>(pi);
   Q_ASSERT(pItem);

   QString strId = pItem->id();
   bool bExpand = pItem->isExpanded() ? true : false;

   settings->setValue(strId, bExpand);
}

void WizCategoryView::advancedSearchByCustomParam(const QString& strParam)
{
    QString strSql, strName, strKeyword;
    int scope;
    WizAdvancedSearchDialog::paramToSQL(strParam, strSql, strKeyword, strName, scope);

    WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
    WizSearcher* searcher = mainWindow->searcher();
    if (searcher)
    {
        if (strSql.isEmpty())
        {
            searcher->search(strKeyword, 500, (SearchScope)scope);
        }
        else if (strKeyword.isEmpty())
        {
            searcher->searchBySQLWhere(strSql, 500, (SearchScope)scope);
        }
        else
        {
            searcher->searchByKeywordAndWhere(strKeyword, strSql, 500, (SearchScope)scope);
        }
    }
}

void WizCategoryView::saveCustomAdvancedSearchParamToDB(const QString& strGuid, const QString& strParam)
{
    m_dbMgr.db().setMeta(QUICK_SEARCH_META, strGuid, strParam);
}

void WizCategoryView::loadCustomAdvancedSearchParamFromDB(QMap<QString, QString>& paramMap)
{
    CWizMetaDataArray arrayMeta;
     m_dbMgr.db().getMetasByName(QUICK_SEARCH_META, arrayMeta);
     CWizMetaDataArray::iterator it;
     for (it = arrayMeta.begin(); it != arrayMeta.end(); it++)
     {
         paramMap.insert(it->strKey, it->strValue);
     }
}

void WizCategoryView::deleteCustomAdvancedSearchParamFromDB(const QString& strGuid)
{
    m_dbMgr.db().deleteMetaByKey(QUICK_SEARCH_META, strGuid);
}

WizCategoryViewFolderItem* WizCategoryView::createFolderItem(QTreeWidgetItem* parent, const QString& strLocation)
{
    WizCategoryViewFolderItem* pFolderItem = new WizCategoryViewFolderItem(m_app, strLocation, m_dbMgr.db().kbGUID());
    parent->addChild(pFolderItem);
    m_dbMgr.db().addExtraFolder(strLocation);
    return pFolderItem;
}

void WizCategoryView::moveGroupFolder(const WIZTAGDATA& sourceFolder, WizFolderSelector* selector)
{
    // move group folder to private folder
    if (selector->isSelectPersonalFolder())
    {
        QString strSelectedFolder = selector->selectedFolder();
        if (strSelectedFolder.isEmpty())
            return;
        //        
        bool combineSameNameFolder = false;
        if (!isCombineSameNameFolder(strSelectedFolder, sourceFolder.strName, combineSameNameFolder))
            return;

        qDebug() << "move group folder to private folder " << strSelectedFolder;
        moveGroupFolderToPersonalFolder(sourceFolder, strSelectedFolder, combineSameNameFolder);
    }
    else if (selector->isSelectGroupFolder())
    {
        WIZTAGDATA tag = selector->selectedGroupFolder();
        if (tag.strKbGUID.isEmpty() || (!tag.strGUID.isEmpty() && tag.strGUID == sourceFolder.strParentGUID)
                || tag.strGUID == sourceFolder.strGUID)
            return;

        //        
        bool combineSameNameFolder = false;
        if (!isCombineSameNameFolder(tag, sourceFolder.strName, combineSameNameFolder))
            return;


        qDebug() << "move group folder to group folder " << tag.strName;
        //
        moveGroupFolderToGroupFolder(sourceFolder, tag, combineSameNameFolder);
    }
}

void WizCategoryView::moveGroupFolderToPersonalFolder(const WIZTAGDATA& groupFolder, const QString& targetParentFolder, bool combineFolder)
{
    qDebug() << "move group folder to personal folder, group folder : " << groupFolder.strName << " private folder ; " << targetParentFolder;
    WizDocumentOperator documentOperator(m_dbMgr);
    documentOperator.moveGroupFolderToPersonalDB(groupFolder, targetParentFolder, combineFolder, true);
}

void WizCategoryView::moveGroupFolderToGroupFolder(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder, bool combineFolder)
{
    qDebug() << "move group folder to group folder , source : " << sourceFolder.strName << "  kb : " << sourceFolder.strKbGUID
             << "target : " << targetFolder.strName << " kb ; " << targetFolder.strKbGUID;

    WizDocumentOperator documentOperator(m_dbMgr);
    documentOperator.moveGroupFolderToGroupDB(sourceFolder, targetFolder, combineFolder, true);
}

bool WizCategoryView::movePersonalFolder(const QString& sourceFolder, WizFolderSelector* selector)
{
    // mvoe  personal folder to personal folder
    if (selector->isSelectPersonalFolder())
    {
        QString strSelectedFolder = selector->selectedFolder();
        if (strSelectedFolder.isEmpty() || strSelectedFolder == sourceFolder ||
                (sourceFolder.startsWith(strSelectedFolder) && sourceFolder.indexOf('/', strSelectedFolder.length()) == sourceFolder.length() -1))
            return false;

        //combine same name folder
        WizCategoryViewItemBase* sourceItem = findFolder(sourceFolder, false, false);
        bool combineSameNameFolder = false;
        if (!isCombineSameNameFolder(strSelectedFolder, WizDatabase::getLocationName(sourceFolder), combineSameNameFolder, sourceItem))
            return false;

        qDebug() << "move personal folder to personal folder  ; " << strSelectedFolder;
        //
        movePersonalFolderToPersonalFolder(sourceFolder, strSelectedFolder, combineSameNameFolder);
        return true;
    }
    else if (selector->isSelectGroupFolder())
    {
        WIZTAGDATA tag = selector->selectedGroupFolder();
        if (tag.strKbGUID.isEmpty())
            return false;
        //        
        bool combineSameNameFolders = false;
        if (!isCombineSameNameFolder(tag, WizDatabase::getLocationName(sourceFolder), combineSameNameFolders))
            return false;
        qDebug() << "move personal folder to group folder " << tag.strName;
        //
        movePersonalFolderToGroupFolder(sourceFolder, tag, combineSameNameFolders);
        return true;
    }
    //
    return false;
}

void WizCategoryView::movePersonalFolderToPersonalFolder(const QString& sourceFolder, const QString& targetParentFolder, bool combineFolder)
{
    QString name = WizDatabase::getLocationName(sourceFolder);
    QString newLocation = targetParentFolder + name + "/";
    //
    moveFolder(sourceFolder, newLocation);
}

void WizCategoryView::movePersonalFolderToGroupFolder(const QString& sourceFolder, const WIZTAGDATA& targetFolder, bool combineFolder)
{
    WizDatabase& db = m_dbMgr.db();
    if (!WizAskUserCipherToOperateEncryptedNotes(sourceFolder, db))
        return;

    qDebug() << "move personal folder : " << sourceFolder << "  to gorup folder ; " << targetFolder.strName;
    WizDocumentOperator documentOperator(m_dbMgr);
    documentOperator.movePersonalFolderToGroupDB(sourceFolder, targetFolder, combineFolder, true);

    WizClearUserCipher(db, m_app.userSettings());
}

void WizCategoryView::copyGroupFolder(const WIZTAGDATA& sourceFolder, WizFolderSelector* selector)
{
    // copy group folder to private folder
    if (selector->isSelectPersonalFolder())
    {
        QString strSelectedFolder = selector->selectedFolder();
        if (strSelectedFolder.isEmpty())
            return;

        //        
        bool combineSameNameFolder = false;
        if (!isCombineSameNameFolder(strSelectedFolder, sourceFolder.strName, combineSameNameFolder))
            return;

        qDebug() << "copy group folder to private folder " << strSelectedFolder;
        copyGroupFolderToPersonalFolder(sourceFolder, strSelectedFolder, selector->isKeepTime(), combineSameNameFolder);
    }
    else if (selector->isSelectGroupFolder())
    {
        WIZTAGDATA tag = selector->selectedGroupFolder();
        if (tag.strKbGUID.isEmpty() || tag.strGUID == sourceFolder.strGUID ||
                (!tag.strGUID.isEmpty() && tag.strGUID == sourceFolder.strParentGUID))
            return;
        qDebug() << "copy group folder to group folder " << tag.strName;
        //        
        bool combineSameNameFolder = false;
        if (!isCombineSameNameFolder(tag, sourceFolder.strName, combineSameNameFolder))
            return;

        copyGroupFolderToGroupFolder(sourceFolder, tag, selector->isKeepTime(), combineSameNameFolder);
    }
}

void WizCategoryView::copyGroupFolderToPersonalFolder(const WIZTAGDATA& groupFolder,
                                                       const QString& targetParentFolder, bool keepDocTime, bool combineFolder)
{
    WizDocumentOperator documentOperator(m_dbMgr);
    documentOperator.copyGroupFolderToPersonalDB(groupFolder, targetParentFolder, keepDocTime, combineFolder, true);
}

void WizCategoryView::copyGroupFolderToGroupFolder(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder,
                                                    bool keepDocTime, bool combineFolder)
{
    WizDocumentOperator documentOperator(m_dbMgr);
    documentOperator.copyGroupFolderToGroupDB(sourceFolder, targetFolder, keepDocTime, combineFolder, true);
}

void WizCategoryView::copyPersonalFolder(const QString& sourceFolder, WizFolderSelector* selector)
{
    // copy  personal folder to personal folder
    if (selector->isSelectPersonalFolder())
    {
        QString strSelectedFolder = selector->selectedFolder();
        if (strSelectedFolder.isEmpty() || strSelectedFolder == sourceFolder)
            return;

        qDebug() << "copy personal folder to personal folder  ; " << strSelectedFolder;
        //combine same name folder
        WizCategoryViewItemBase* sourceItem = findFolder(sourceFolder, false, false);
        bool combineSameNameFolder = false;
        if (!isCombineSameNameFolder(strSelectedFolder, WizDatabase::getLocationName(sourceFolder),
                                     combineSameNameFolder, sourceItem))
            return;

        copyPersonalFolderToPersonalFolder(sourceFolder, strSelectedFolder, selector->isKeepTime(),
                                            selector->isKeepTag(), combineSameNameFolder);
    }
    else if (selector->isSelectGroupFolder())
    {
        WIZTAGDATA tag = selector->selectedGroupFolder();
        if (tag.strKbGUID.isEmpty())
            return;
        qDebug() << "copy personal folder to group folder " << tag.strName;        
        bool combineSameNameFolders = false;
        if (!isCombineSameNameFolder(tag, WizDatabase::getLocationName(sourceFolder), combineSameNameFolders))
            return;
        //
        copyPersonalFolderToGroupFolder(sourceFolder, tag, selector->isKeepTime(), combineSameNameFolders);
    }
}

void WizCategoryView::copyPersonalFolderToPersonalFolder(const QString& sourceFolder, const QString& targetParentFolder,
                                                          bool keepDocTime, bool keepTag, bool combineFolder)
{
    WizDatabase& db = m_dbMgr.db();
    if (!WizAskUserCipherToOperateEncryptedNotes(sourceFolder, db))
        return;

    WizDocumentOperator documentOperator(m_dbMgr);
    documentOperator.copyPersonalFolderToPersonalDB(sourceFolder, targetParentFolder,
                                                     keepDocTime, keepTag, combineFolder, true);

    WizClearUserCipher(db, m_app.userSettings());
}

void WizCategoryView::copyPersonalFolderToGroupFolder(const QString& sourceFolder,
                                                       const WIZTAGDATA& targetFolder, bool keepDocTime, bool combineFolder)
{
    WizDatabase& db = m_dbMgr.db();
    if (!WizAskUserCipherToOperateEncryptedNotes(sourceFolder, db))
        return;

    WizDocumentOperator documentOperator(m_dbMgr);
    documentOperator.copyPersonalFolderToGroupDB(sourceFolder, targetFolder, keepDocTime, combineFolder);

    WizClearUserCipher(db, m_app.userSettings());
}

void WizCategoryView::moveDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument, const WIZTAGDATA& targetTag)
{
    WizDocumentOperator documentOperator(m_dbMgr);
    // move, show progress if size > 3
    if (arrayDocument.size() <= 3)
    {
        documentOperator.moveDocumentsToGroupFolder(arrayDocument, targetTag, false);
    }
    else
    {

        documentOperator.moveDocumentsToGroupFolder(arrayDocument, targetTag, true);
    }
}


void WizCategoryView::dropItemAsBrother(WizCategoryViewItemBase* targetItem,
                                         WizCategoryViewItemBase* dragedItem, bool dropAtTop, bool deleteDragSource)
{
//    if (targetItem->type() == Category_FolderItem)
//    {
//        CWizCategoryViewFolderItem* folderItem = dynamic_cast<CWizCategoryViewFolderItem*>(targetItem);
//        folderItem->location();
//    }
//    else if (targetItem->type() == Category_GroupItem)
//    {

//    }
}

void WizCategoryView::dropItemAsChild(WizCategoryViewItemBase* targetItem, WizCategoryViewItemBase* dragedItem, bool deleteDragSource)
{
    if (targetItem->kbGUID() == dragedItem->kbGUID())
    {
        qDebug() << "Try to dropItemAsChild, but the targetItem and dragedItem are in same db, it should be processed in drop event, not here";
        return;
    }

    if (targetItem->type() == Category_AllFoldersItem || targetItem->type() == Category_FolderItem)
    {
        QString targetParentFolder = "/";
        if (targetItem->type() == Category_FolderItem)
        {
            WizCategoryViewFolderItem* targetFolder = dynamic_cast<WizCategoryViewFolderItem*>(targetItem);
            targetParentFolder = targetFolder->location();
        }
        if (dragedItem->type() == Category_GroupItem)
        {
            WizCategoryViewGroupItem* groupItem = dynamic_cast<WizCategoryViewGroupItem*>(dragedItem);
            if (deleteDragSource)
            {
                moveGroupFolderToPersonalFolder(groupItem->tag(), targetParentFolder, true);
            }
            else
            {
                copyGroupFolderToPersonalFolder(groupItem->tag(), targetParentFolder, true, true);
            }
        }
    }
    else if (targetItem->type() == Category_GroupItem)
    {
        WizCategoryViewGroupItem* targetFolder = dynamic_cast<WizCategoryViewGroupItem*>(targetItem);
        if (dragedItem->type() == Category_FolderItem)
        {
            WizCategoryViewFolderItem* sourceFolder = dynamic_cast<WizCategoryViewFolderItem*>(dragedItem);
            if (deleteDragSource)
            {
                movePersonalFolderToGroupFolder(sourceFolder->location(), targetFolder->tag(), false);
            }
            else
            {
                copyPersonalFolderToGroupFolder(sourceFolder->location(), targetFolder->tag(), true, true);
            }
        }
        else if (dragedItem->type() == Category_GroupItem)
        {
            WizCategoryViewGroupItem* dragedFolder = dynamic_cast<WizCategoryViewGroupItem*>(dragedItem);
            if (deleteDragSource)
            {
                moveGroupFolderToGroupFolder(dragedFolder->tag(), targetFolder->tag(), false);
            }
            else
            {
                copyGroupFolderToGroupFolder(dragedFolder->tag(), targetFolder->tag(), true, true);
            }
        }
    }
}

void WizCategoryView::resetFolderLocation(WizCategoryViewFolderItem* item)
{
    QString oldLocation = item->location();
    QString newLocation;
    //
    QString strName = item->name();
    //
    if (WizCategoryViewFolderItem* parentFolderItem = dynamic_cast<WizCategoryViewFolderItem*>(item->parent()))
    {
        newLocation = parentFolderItem->location() + strName + "/";
    }
    else
    {
        newLocation = "/" + strName + "/";
    }
    //change item location
    resetFolderLocation(item, newLocation);
    //
    //save folder pos
    WizDatabase& db = m_dbMgr.db();
    updatePersonalFolderLocation(db, oldLocation, newLocation);
    //
    //start move folder
    moveFolder(oldLocation, newLocation);
}

void WizCategoryView::resetFolderLocation(WizCategoryViewFolderItem* item, const QString& strNewLocation)
{
    item->setLocation(strNewLocation);
    for (int i = 0; i < item->childCount(); i++)
    {
        WizCategoryViewFolderItem* child = dynamic_cast<WizCategoryViewFolderItem*>(item->child(i));
        if (child)
        {
            QString strChildLocation = strNewLocation + child->text(0) + "/";
            resetFolderLocation(child, strChildLocation);
        }
    }
}

void WizCategoryView::moveFolder(QString oldLocation, QString newLocation)
{
    if (oldLocation == newLocation) {
        return;
    }
    //
    ::WizExecutingActionDialog::executeAction(tr("Moving folder..."), WIZ_THREAD_DEFAULT, [=]{
        //
        //wait for sync
        WIZKM_WAIT_AND_PAUSE_SYNC();
        //
        WizDatabase& db = m_dbMgr.db();
        db.blockSignals(true);
        WizFolder folder(db, oldLocation);
        folder.blockSignals(true);
        folder.moveToLocation(newLocation);
        folder.blockSignals(false);
        db.blockSignals(false);
    });
    //
    WizCategoryViewFolderItem* newItem = findFolder(newLocation, true, true);
    if (newItem)
    {
        //setCurrentItem(newItem);
    }
    //
    WizMainWindow::instance()->quickSyncKb("");
    //
    updatePersonalFolderDocumentCount();
}

void WizCategoryView::saveSelected(QSettings* settings)
{
    if (!settings)
        return;
    //
    QTreeWidgetItem *curr = currentItem();
    if (!curr)
        return;

    WizCategoryViewItemBase* pItem = dynamic_cast<WizCategoryViewItemBase*>(curr);
    if (!pItem)
        return;

    settings->setValue(TREEVIEW_SELECTED_ITEM, pItem->id());
}
