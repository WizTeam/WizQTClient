#include "wizCategoryView.h"

#include <QHeaderView>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QApplication>
#include <QTimer>
#include <QFileDialog>

#include <extensionsystem/pluginmanager.h>

#include "wizdef.h"
#include "widgets/wizScrollBar.h"
#include "wizmainwindow.h"
#include "wizProgressDialog.h"
#include "wiznotestyle.h"
#include "share/wizdrawtexthelper.h"
#include "share/wizsettings.h"
#include "share/wizDatabaseManager.h"
#include "share/wizSearchIndexer.h"
#include "share/wizAnalyzer.h"
#include "wizFolderSelector.h"
#include "wizLineInputDialog.h"
#include "wizWebSettingsDialog.h"
#include "sync/wizkmxmlrpc.h"
#include "sync/apientry.h"
#include "sync/token.h"
#include "utils/stylehelper.h"
#include "utils/misc.h"
#include "wizFileReader.h"
#include "widgets/wizAdvancedSearchDialog.h"
#include "wizOEMSettings.h"

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

#define CATEGORY_SEARCH_BYCUSTOM  QObject::tr("Search by custom param")

// for context menu text
#define CATEGORY_ACTION_DOCUMENT_NEW    QObject::tr("New note")
#define CATEGORY_ACTION_DOCUMENT_LOAD   QObject::tr("Load note")
#define CATEGORY_ACTION_IMPORT_FILE   QObject::tr("Import file...")
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
#define CATEGORY_ACTION_REMOVE_SHORTCUT     QObject::tr("Remove from shortcuts")
#define CATEGORY_ACTION_RECOVERY_DELETED_NOTES    QObject::tr("Recovery deleted notes...")

#define LINK_COMMAND_ID_CREATE_GROUP        100

#define TREEVIEW_STATE "TreeState"
#define TREEVIEW_SELECTED_ITEM "SelectedItemID"
#define SHORTCUT_STATE "ShortcutState"

#define CATEGORY_SHORTCUT   "CategoryShortcut"
#define CATEGORY_META   "CategoryMeta"

#define QUICK_SEARCH_META   "CUSTOM_QUICK_SEARCH"


/* ------------------------------ CWizCategoryBaseView ------------------------------ */

CWizCategoryBaseView::CWizCategoryBaseView(CWizExplorerApp& app, QWidget* parent)
    : ITreeView(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_bDragHovered(false)
    , m_selectedItem(NULL)
    , m_dragHoveredTimer(new QTimer())
    , m_dragHoveredItem(0)
    , m_dragItem(NULL)
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
    m_vScroll = new CWizScrollBar(this);
    m_vScroll->syncWith(verticalScrollBar());
#endif

    // style
    setStyle(::WizGetStyle(m_app.userSettings().skin()));
    QColor colorBg = Utils::StyleHelper::treeViewBackground();
    QPalette pal = palette();
    pal.setBrush(QPalette::Base, colorBg);
    setPalette(pal);
    //
    setCursor(QCursor(Qt::ArrowCursor));
    setMouseTracking(true);

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

CWizCategoryBaseView::~CWizCategoryBaseView()
{
    if (!m_dragHoveredTimer) {
        m_dragHoveredTimer->stop();
        delete m_dragHoveredTimer;
        m_dragHoveredTimer = 0;
    }
}

void CWizCategoryBaseView::mousePressEvent(QMouseEvent* event)
{
    // saved for child item which need test hit position.
    m_hitPos = event->pos();

    QTreeWidget::mousePressEvent(event);
}

void CWizCategoryBaseView::mouseMoveEvent(QMouseEvent* event)
{    
    QPoint msPos = event->pos();
    CWizCategoryViewItemBase* pItem =  itemAt(msPos);
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

    if (m_app.userSettings().isManualSortingEnabled())
    {
        m_dragItem = currentCategoryItem<CWizCategoryViewItemBase>();
        Q_ASSERT(m_dragItem);
        if (!m_dragItem->dragAble() || !m_dbMgr.db(m_dragItem->kbGUID()).IsGroupSuper())
        {
            m_dragItem = 0;
            return;
        }

        resetRootItemsDropEnabled(m_dragItem);
        QTreeWidget::startDrag(supportedActions);
        setCurrentItem(m_dragItem);
        m_dragItem = 0;

        ::WizGetAnalyzer().LogAction("categoryDragItem");
    }

}

void mime2Note(const QByteArray& bMime, CWizDocumentDataArray& arrayDocument)
{
    QString strMime(QString::fromUtf8(bMime));
    QStringList lsNotes = strMime.split(";");
    for (int i = 0; i < lsNotes.size(); i++) {
        QStringList lsMeta = lsNotes[i].split(":");
        //qDebug()<<lsMeta;
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
    } else if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else{
        QTreeWidget::dragEnterEvent(event);
    }

}

void CWizCategoryBaseView::dragMoveEvent(QDragMoveEvent *event)
{
    m_dragHoveredPos = event->pos();

    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS) )
    {
        CWizCategoryViewItemBase* pItem = itemAt(event->pos());
        if (!pItem)
            return;

        if (m_dragHoveredItem != pItem) {
            m_dragHoveredTimer->stop();
            m_dragHoveredItem = pItem;
            m_dragHoveredTimer->start(1000);
        }

        m_dragDocArray.clear();
        mime2Note(event->mimeData()->data(WIZNOTE_MIMEFORMAT_DOCUMENTS), m_dragDocArray);

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

    }else if(event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
    else if (m_dragItem)
    {
        if (CWizCategoryViewItemBase* pItem = itemAt(event->pos()))
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

    viewport()->repaint();
}

void CWizCategoryBaseView::dragLeaveEvent(QDragLeaveEvent* event)
{
    Q_UNUSED(event);

    m_bDragHovered = false;
    m_dragHoveredPos = QPoint();
    m_dragHoveredTimer->stop();
    m_dragHoveredItem = 0;

    m_dragDocArray.clear();
    viewport()->repaint();
}

void CWizCategoryBaseView::dropEvent(QDropEvent * event)
{
    m_bDragHovered = false;
    m_dragHoveredPos = QPoint();

    m_dragDocArray.clear();

    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS)) {
        ::WizGetAnalyzer().LogAction("categoryDropDocument");
        CWizDocumentDataArray arrayDocument;
        mime2Note(event->mimeData()->data(WIZNOTE_MIMEFORMAT_DOCUMENTS), arrayDocument);

        if (!arrayDocument.size())
            return;

        CWizCategoryViewItemBase* pItem = itemAt(event->pos());
        if (!pItem)
            return;

        bool forceCopy = (QApplication::keyboardModifiers() == Qt::ControlModifier);

        for (CWizDocumentDataArray::const_iterator it = arrayDocument.begin();
             it != arrayDocument.end();
             it++)
        {
            pItem->drop(*it, forceCopy);
        }
    } else if (event->mimeData()->hasUrls()) {
        ::WizGetAnalyzer().LogAction("categoryDropFiles");
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
        if (m_dragItem && !(m_dragItem->flags() & Qt::ItemIsDropEnabled))
        {
            qDebug() << "[DragDrop]Can not drop item at invalid position";
            return;
        }

        ::WizGetAnalyzer().LogAction("categoryDropItem");

        QModelIndex droppedIndex = indexAt(event->pos());
        if( !droppedIndex.isValid() )
          return;

        QTreeWidget::dropEvent(event);
        if (m_dragItem)
        {
            on_itemPosition_changed(m_dragItem);
        }
        viewport()->repaint();
        return;
    }

    viewport()->repaint();
    event->accept();
}

void CWizCategoryBaseView::importFiles(QStringList &strFileList)
{
    CWizFileReader *fileReader = new CWizFileReader();
    connect(fileReader, SIGNAL(fileLoaded(QString, QString)),
            SLOT(createDocumentByHtml(QString, QString)));
    connect(fileReader, SIGNAL(htmlFileloaded(QString, QString, QString)),
            SLOT(createDocumentByHtml(QString, QString, QString)));
    connect(fileReader, SIGNAL(fileLoadFailed(QString)),
            SLOT(createDocumentWithAttachment(QString)));
    connect(fileReader, SIGNAL(richTextFileLoaded(QString,QString,QString)),
            SLOT(createDocumentByHtmlWithAttachment(QString,QString,QString)));
    MainWindow *mainWindow = dynamic_cast<MainWindow*>(m_app.mainWindow());
    CWizProgressDialog *progressDialog  = mainWindow->progressDialog();
    progressDialog->setProgress(100,0);
    progressDialog->setActionString(tr("%1 files to load.").arg(strFileList.count()));
    progressDialog->setNotifyString(tr("loading..."));
    connect(fileReader, SIGNAL(loadProgress(int,int)), progressDialog, SLOT(setProgress(int,int)));
    connect(fileReader,SIGNAL(loadFinished()),progressDialog,SLOT(close()));
    fileReader->loadFiles(strFileList);
    progressDialog->exec();
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

bool CWizCategoryView::setCurrentIndex(const WIZDOCUMENTDATA& document)
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
            CWizDatabase& db = m_dbMgr.db(document.strKbGUID);
            CWizTagDataArray arrayTag;
            if (!db.GetDocumentTags(document.strGUID, arrayTag)) {
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
                strTempLocation.Trim('/');
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

bool findItemByKbGUID(QTreeWidgetItem *item, const QString& strKbGUID, CWizCategoryViewItemBase*& target)
{
    for( int i = 0; i < item->childCount(); ++i)
    {
        CWizCategoryViewItemBase * pItem = dynamic_cast<CWizCategoryViewItemBase *>(item->child(i));
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

CWizCategoryViewItemBase* CWizCategoryBaseView::itemFromKbGUID(const QString &strKbGUID) const
{
    if (strKbGUID.isEmpty())
        return 0;

    CWizCategoryViewItemBase * item = 0;
    for (int i = 0; i < topLevelItemCount(); i ++)
    {
        findItemByKbGUID(topLevelItem(i), strKbGUID, item);
    }

    return item;
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
            //Q_ASSERT(false);
            break;
        }
    }

    return index;
}

void CWizCategoryBaseView::resetRootItemsDropEnabled(CWizCategoryViewItemBase* pItem)
{
    int topCount = topLevelItemCount();
    for (int i = 0; i < topCount; i++)
    {
        CWizCategoryViewItemBase* pi = dynamic_cast<CWizCategoryViewItemBase*>(pItem);
        if (pi->acceptDrop(pItem))
            pi->setFlags(pi->flags() | Qt::ItemIsDropEnabled);
        else
            pi->setFlags(pi->flags() & ~Qt::ItemIsDropEnabled);
    }
    update();
}

QString CWizCategoryBaseView::getUseableItemName(QTreeWidgetItem* parent, \
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

void CWizCategoryBaseView::resetFolderLocation(CWizCategoryViewFolderItem* item, const QString& strNewLocation)
{
    item->setLocation(strNewLocation);
    for (int i = 0; i < item->childCount(); i++)
    {
        CWizCategoryViewFolderItem* child = dynamic_cast<CWizCategoryViewFolderItem*>(item->child(i));
        if (child)
        {
            QString strChildLocation = strNewLocation + child->text(0) + "/";
            resetFolderLocation(child, strChildLocation);
        }
    }
}

void CWizCategoryBaseView::on_dragHovered_timeOut()
{
    if (m_dragHoveredItem) {
        m_dragHoveredTimer->stop();
        expandItem(m_dragHoveredItem);
        viewport()->repaint();
    }
}

bool CWizCategoryBaseView::validateDropDestination(const QPoint& p) const
{
    if (p.isNull())
        return false;

    if (m_dragDocArray.empty())
        return false;

    CWizCategoryViewItemBase* itemBase = itemAt(p);
    WIZDOCUMENTDATAEX data = *m_dragDocArray.begin();
    return (itemBase && itemBase->acceptDrop(data));

}

Qt::ItemFlags CWizCategoryBaseView::dragItemFlags() const
{
    if (m_dragItem)
        return m_dragItem->flags();

    return Qt::NoItemFlags;
}

void CWizCategoryBaseView::drawItem(QPainter* p, const QStyleOptionViewItemV4 *vopt) const
{
    CWizCategoryViewItemBase* pItem = categoryItemFromIndex(vopt->index);
    Q_ASSERT(pItem);

    if (pItem)
        pItem->draw(p, vopt);
}

void CWizCategoryBaseView::createDocumentByHtml(const QString& /*strHtml*/, const QString& /*strTitle*/)
{
    // do nothing
    // create document in CWizCategoryView

}

void CWizCategoryBaseView::createDocumentByHtml(const QString &/*strFileName*/,
                                                const QString& /*strHtml*/, const QString& /*strTitle*/)
{
}

bool CWizCategoryBaseView::createDocumentWithAttachment(const QString& /*strFileName*/)
{
    return true;
}

bool CWizCategoryBaseView::createDocumentByHtmlWithAttachment(const QString& /*strHtml*/,
                                                              const QString& /*strTitle*/, const QString& /*strAttachFile*/)
{
    return true;
}


/* ------------------------------ CWizCategoryView ------------------------------ */
CWizCategoryView::CWizCategoryView(CWizExplorerApp& app, QWidget* parent)
    : CWizCategoryBaseView(app, parent)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(false);
    viewport()->setAcceptDrops(false);
    invisibleRootItem()->setFlags(invisibleRootItem()->flags() & ~Qt::ItemIsDropEnabled);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);


    initMenus();

    connect(this, SIGNAL(itemClicked(QTreeWidgetItem*, int)), SLOT(on_itemClicked(QTreeWidgetItem *, int)));
    connect(this, SIGNAL(itemSelectionChanged()), SLOT(on_itemSelectionChanged()));

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

    QAction* actionRecovery = new QAction("ActionRecovery", this);
    actionRecovery->setData(ActionRecovery);
    actionRecovery->setText(CATEGORY_ACTION_RECOVERY_DELETED_NOTES);
    addAction(actionRecovery);
    connect(actionRecovery, SIGNAL(triggered()), SLOT(on_action_deleted_recovery()));
    //


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

    QAction* actionRemoveShortcut = new QAction("RemoveShortcut", this);
    actionRemoveShortcut->setText(CATEGORY_ACTION_REMOVE_SHORTCUT);
    actionRemoveShortcut->setShortcutContext(Qt::WidgetShortcut);
    addAction(actionRemoveShortcut);
    connect(actionRemoveShortcut, SIGNAL(triggered()), SLOT(on_action_removeShortcut()));

    QAction* actionAdvancedSearch = new QAction(tr("Advanced search"), this);
    actionAdvancedSearch->setData(ActionAdvancedSearch);
    addAction(actionAdvancedSearch);
    connect(actionAdvancedSearch, SIGNAL(triggered()), SLOT(on_action_advancedSearch()));

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
    m_menuShortcut = new QMenu(this);
    m_menuShortcut->addAction(actionRemoveShortcut);

    // custom search menu
    m_menuCustomSearch = new QMenu(this);
    m_menuCustomSearch->addAction(actionAdvancedSearch);
    m_menuCustomSearch->addSeparator();
    m_menuCustomSearch->addAction(actionAddCustomSearch);
    m_menuCustomSearch->addAction(actionEditCustomSearch);
    m_menuCustomSearch->addAction(actionRemoveCustomSearch);

    // trash menu
    m_menuTrash = new QMenu(this);
    m_menuTrash->addAction(actionTrash);
    m_menuTrash->addAction(actionRecovery);

    // folder root menu
    m_menuFolderRoot = new QMenu(this);
    m_menuFolderRoot->addAction(actionNewItem);

    // folder menu
    m_menuFolder = new QMenu(this);
    m_menuFolder->addAction(actionNewDoc);
    m_menuFolder->addAction(actionImportFile);
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

void CWizCategoryView::showTrashContextMenu(QPoint pos)
{
    resetMenu(TrashItem);
    m_menuTrash->popup(pos);
}

void CWizCategoryView::showShortcutContextMenu(QPoint pos)
{
    resetMenu(ShortcutItem);
    m_menuShortcut->popup(pos);
}

void CWizCategoryView::showCustomSearchContextMenu(QPoint pos, bool removable)
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

bool CWizCategoryView::createDocument(WIZDOCUMENTDATA& data, const QString& strHtml, const QString& strTitle)
{
    QString strKbGUID = m_dbMgr.db().kbGUID();
    QString strLocation = m_dbMgr.db().GetDefaultNoteLocation();
    WIZTAGDATA tag;

    if (getAvailableNewNoteTagAndLocation(strKbGUID, tag, strLocation))
    {
        QString strBody = WizGetHtmlBodyContent(strHtml);
        if (!m_dbMgr.db(strKbGUID).CreateDocumentAndInit(strBody, "", 0, strTitle, "newnote", strLocation, "", data))
        {
            TOLOG("Failed to new document!");
            return false;
        }

        if (!tag.strGUID.IsEmpty()) {
            CWizDocument doc(m_dbMgr.db(strKbGUID), data);
            doc.AddTag(tag);
        }
    }

    quickSyncNewDocument(data.strKbGUID);
    //
    return true;
}

bool CWizCategoryView::createDocumentWithAttachment(const QString& strFileName)
{
    if (!QFile::exists(strFileName))
        return false;

    QStringList fileList(strFileName);
    WIZDOCUMENTDATA data;
    return createDocumentByAttachments(data, fileList);
}

bool CWizCategoryView::createDocumentByHtmlWithAttachment(const QString& strHtml, const QString& strTitle, const QString& strAttachFile)
{
    if (!QFile::exists(strAttachFile))
        return false;

     WIZDOCUMENTDATA data;
    if (!createDocument(data, strHtml, strTitle))
        return false;

    CWizDatabase& db = m_dbMgr.db(data.strKbGUID);
    WIZDOCUMENTATTACHMENTDATA attach;
    if (!db.AddAttachment(data, strAttachFile, attach))
    {
        TOLOG1("[Service] add attch failed :  1%", strAttachFile);
        return false;
    }
    return true;
}

bool CWizCategoryView::createDocumentByAttachments(WIZDOCUMENTDATA& data, const QStringList& attachList)
{
    if (attachList.isEmpty())
        return false;

    QString strTitle =Utils::Misc::extractFileName(attachList.first());
    if (!createDocument(data, "<p><br/></p>", strTitle))
        return false;

    CWizDatabase& db = m_dbMgr.db(data.strKbGUID);
    foreach (QString strFileName, attachList) {
        WIZDOCUMENTATTACHMENTDATA attach;
        if (!db.AddAttachment(data, strFileName, attach))
        {
            TOLOG1("[Service] add attch failed :  1%", strFileName);
        }
    }

    return true;
}

bool CWizCategoryView::createDocumentByTemplate(WIZDOCUMENTDATA& data, const QString& strZiw)
{
    QString strKbGUID = m_dbMgr.db().kbGUID();
    QString strLocation = m_dbMgr.db().GetDefaultNoteLocation();
    WIZTAGDATA tag;

    if (getAvailableNewNoteTagAndLocation(strKbGUID, tag, strLocation))
    {
        if (!m_dbMgr.db(strKbGUID).CreateDocumentByTemplate(strZiw, strLocation, tag, data))
        {
            TOLOG("Failed to new document!");
            return false;
        }

        if (!tag.strGUID.IsEmpty()) {
            CWizDocument doc(m_dbMgr.db(strKbGUID), data);
            doc.AddTag(tag);
        }
    }

    quickSyncNewDocument(data.strKbGUID);
    //
    return true;
}
QString CWizCategoryView::WizGetHtmlBodyContent(QString strHtml)
{
    QRegExp regex("<body.*>([\\s\\S]*)</body>", Qt::CaseInsensitive);
    QString strBody;
    if (regex.indexIn(strHtml) != -1) {
        strBody = regex.cap(1);
    } else {
        strBody = strHtml;
    }
    return strBody;
}

void CWizCategoryView::on_action_newDocument()
{
    ::WizGetAnalyzer().LogAction("categoryMenuNewDocument");
    if (currentCategoryItem<CWizCategoryViewFolderItem>()
            || currentCategoryItem<CWizCategoryViewGroupRootItem>()
            || currentCategoryItem<CWizCategoryViewGroupItem>())
    {
        // delegate create action to mainwindow
        Q_EMIT newDocument();
    }
}

void CWizCategoryView::on_action_loadDocument()
{
    ::WizGetAnalyzer().LogAction("categoryMenuLoadDocument");
    //TODO:
}

void CWizCategoryView::on_action_importFile()
{
    ::WizGetAnalyzer().LogAction("categoryMenuImportFile");
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
    ::WizGetAnalyzer().LogAction("categoryMenuNewFolder");
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
        //moveFolderPostionBeforeTrash(strLocation);
    } else if (CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>()) {
        strLocation = p->location() + strFolderName + "/";
    }

    addAndSelectFolder(strLocation);
    sortFolders();
    m_dbMgr.db().AddExtraFolder(strLocation);
    m_dbMgr.db().SetLocalValueVersion("folders", -1);
}

void CWizCategoryView::on_action_user_newTag()
{
    ::WizGetAnalyzer().LogAction("categoryMenuNewTag");
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
    ::WizGetAnalyzer().LogAction("categoryMenuNewGroupFolder");
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
    ::WizGetAnalyzer().LogAction("categoryMenuMoveFolder");
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
    ::WizGetAnalyzer().LogAction("categoryMenuRenameFolder");
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
    if (nMax <= nValue + 1) {
        progress->setVisible(false);
    }
}

void CWizCategoryView::on_action_user_renameTag()
{
    ::WizGetAnalyzer().LogAction("categoryMenuRenameTag");
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
    ::WizGetAnalyzer().LogAction("categoryMenuRenameGroupFolder");
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
    ::WizGetAnalyzer().LogAction("categoryMenuDeleteFolder");
    CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>();
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

    QString strWarning = tr("Do you really want to delete all notes inside folder: %1 ? (All notes will move to trash folder and remove from cloud server)").arg(p->location());
    msgBox->setText(strWarning);
    msgBox->exec();

    int result = QDialog::Rejected;
    if (msgBox->clickedButton() == btnOK)
    {
        result = QDialog::Accepted;
    }
    on_action_user_deleteFolder_confirmed(result);
}

void CWizCategoryView::on_action_user_deleteFolder_confirmed(int result)
{    
    CWizCategoryViewFolderItem* p = currentCategoryItem<CWizCategoryViewFolderItem>();
    if (!p)
        return;

    if (result == QMessageBox::Accepted) {
        CWizFolder folder(m_dbMgr.db(), p->location());
        folder.Delete();
    }
}

void CWizCategoryView::on_action_user_deleteTag()
{
    ::WizGetAnalyzer().LogAction("categoryMenuDeleteTag");
    CWizCategoryViewTagItem* p = currentCategoryItem<CWizCategoryViewTagItem>();
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

void CWizCategoryView::on_action_user_deleteTag_confirmed(int result)
{    
    CWizCategoryViewTagItem* p = currentCategoryItem<CWizCategoryViewTagItem>();
    if (!p)
        return;

    if (result == QMessageBox::Accepted) {
        WIZTAGDATA tag = p->tag();
        m_dbMgr.db().DeleteTagWithChildren(tag, TRUE);
    }
}

void CWizCategoryView::on_action_group_deleteFolder()
{
    ::WizGetAnalyzer().LogAction("categoryMenuDeleteGroupFolder");
    CWizCategoryViewGroupItem* p = currentCategoryItem<CWizCategoryViewGroupItem>();
    if (!p)
        return;

    // FIXME: as above

    QMessageBox* msgBox = new QMessageBox(window());
    msgBox->setWindowTitle(tr("Delete group folder"));
    msgBox->setIcon(QMessageBox::Information);
    msgBox->addButton(tr("Cancel"), QMessageBox::NoRole);
    QPushButton* btnOK = msgBox->addButton(tr("OK"), QMessageBox::YesRole);
    msgBox->setDefaultButton(btnOK);

    QString strWarning = tr("Do you really want to delete folder: %1? (All notes will move to unclassified folder, It's safe.)").arg(p->tag().strName);
    msgBox->setText(strWarning);
    msgBox->exec();    

    int result = QDialog::Rejected;
    if (msgBox->clickedButton() == btnOK)
    {
        result = QDialog::Accepted;
    }
    on_action_group_deleteFolder_confirmed(result);
}

void CWizCategoryView::on_action_group_deleteFolder_confirmed(int result)
{    
    CWizCategoryViewGroupItem* p = currentCategoryItem<CWizCategoryViewGroupItem>();
    if (!p)
        return;

    if (result == QMessageBox::Accepted) {
        WIZTAGDATA tag = p->tag();
        m_dbMgr.db(p->kbGUID()).DeleteTagWithChildren(tag, true);
    }
}

void CWizCategoryView::on_action_deleted_recovery()
{
    ::WizGetAnalyzer().LogAction("categoryMenuRecovery");
    CWizCategoryViewTrashItem* trashItem = currentCategoryItem<CWizCategoryViewTrashItem>();
    if (trashItem)
    {
        QString strToken = WizService::Token::token();
        QString strUrl = WizService::ApiEntry::standardCommandUrl("deleted_recovery", strToken, "&kb_guid=" + trashItem->kbGUID(), false);
        WizShowWebDialogWithToken(tr("Recovery notes"), strUrl, 0, QSize(800, 480), true);
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
    ::WizGetAnalyzer().LogAction("categoryMenuGroupAttribute");
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
    ::WizGetAnalyzer().LogAction("categoryMenuManageGroup");
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
    ::WizGetAnalyzer().LogAction("categoryMenuNewBizAttribute");
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
    ::WizGetAnalyzer().LogAction("categoryMenuManageBiz");
    CWizCategoryViewBizGroupRootItem* p = currentCategoryItem<CWizCategoryViewBizGroupRootItem>();
    if (p && !p->biz().bizGUID.isEmpty()) {
        manageBiz(p->biz().bizGUID, false);
    }
}

void CWizCategoryView::on_action_removeShortcut()
{
    ::WizGetAnalyzer().LogAction("categoryMenuRemoveShortcut");
    CWizCategoryViewShortcutItem* p = currentCategoryItem<CWizCategoryViewShortcutItem>();
    CWizCategoryViewShortcutRootItem *pRoot = dynamic_cast<CWizCategoryViewShortcutRootItem *>(p->parent());
    if (p && pRoot)
    {
        pRoot->removeChild(p);
        if (pRoot->childCount() == 0)
        {
            pRoot->addPlaceHoldItem();
        }
    }
    saveShortcutState();
}

void CWizCategoryView::on_action_advancedSearch()
{
    ::WizGetAnalyzer().LogAction("categoryMenuAdvancedSearch");
    bool bSearchOnly = true;
    CWizAdvancedSearchDialog dlg(bSearchOnly);
    if (dlg.exec() == QDialog::Accepted)
    {
        QString strParam = dlg.getParams();
        advancedSearchByCustomParam(strParam);
    }
}

void CWizCategoryView::on_action_addCustomSearch()
{
    ::WizGetAnalyzer().LogAction("categoryMenuAddCustomSearch");
    bool bSearchOnly = false;
    CWizAdvancedSearchDialog dlg(bSearchOnly);
    if (dlg.exec() == QDialog::Accepted)
    {
        QString strParam = dlg.getParams();
        advancedSearchByCustomParam(strParam);

        // create item by param
        QString strSQLWhere, name , keyword;
        int scope;
        CWizAdvancedSearchDialog::paramToSQL(strParam, strSQLWhere, keyword, name, scope);

        CWizCategoryViewItemBase* rootItem = findAllSearchItem();
        if (!rootItem)
            return;

        QTreeWidgetItem* parentItem = 0;
        for (int i = 0; i < rootItem->childCount(); i++)
        {
            if (rootItem->child(i)->text(0) == CATEGORY_SEARCH_BYCUSTOM)
            {
                parentItem = dynamic_cast<CWizCategoryViewSearchItem*>(rootItem->child(i));
                break;
            }
        }
        if (parentItem == 0)
        {
            parentItem = new CWizCategoryViewSearchItem(m_app, CATEGORY_SEARCH_BYCUSTOM);
        }
        rootItem->addChild(parentItem);


        QString strGuid = ::WizGenGUIDLowerCaseLetterOnly();
        CWizCategoryViewCustomSearchItem* item = new CWizCategoryViewCustomSearchItem(
                    m_app, name, strParam, strSQLWhere, strGuid, keyword, scope);
        parentItem->addChild(item);
        sortItems(0, Qt::AscendingOrder);
        saveCustomAdvancedSearchParamToDB(strGuid, strParam);
    }
}

void CWizCategoryView::on_action_editCustomSearch()
{
    ::WizGetAnalyzer().LogAction("categoryMenuEditCustomSearch");
    if (currentItem()->type() == ItemType_QuickSearchCustomItem)
    {
        CWizCategoryViewCustomSearchItem* item = dynamic_cast<CWizCategoryViewCustomSearchItem*>(currentItem());
        if (item)
        {
            CWizAdvancedSearchDialog dlg(false);
            dlg.setParams(item->getSelectParam());
            if (dlg.exec() == QDialog::Accepted)
            {
                QString strParam = dlg.getParams();
                QString strSQLWhere, name, keyword;
                int scope;
                CWizAdvancedSearchDialog::paramToSQL(strParam, strSQLWhere, keyword, name, scope);
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

void CWizCategoryView::on_action_removeCustomSearch()
{
    ::WizGetAnalyzer().LogAction("categoryMenuRemoveCustomSearch");
    if (currentItem()->type() == ItemType_QuickSearchCustomItem)
    {
        CWizCategoryViewCustomSearchItem* item = dynamic_cast<CWizCategoryViewCustomSearchItem*>(currentItem());
        if (item)
        {
            QString strGuid = item->kbGUID();
            deleteCustomAdvancedSearchParamFromDB(strGuid);
            item->parent()->removeChild(item);
        }
    }
}

void CWizCategoryView::on_action_emptyTrash()
{
    ::WizGetAnalyzer().LogAction("categoryMenuEmptyTrash");
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
    else if (CWizCategoryViewSectionItem* pItem = dynamic_cast<CWizCategoryViewSectionItem*>(item))
    {
        if(CATEGORY_TEAM_GROUPS == pItem->name() && pItem->extraButtonClickTest())
        {
            createGroup();
        }
    }
    else if (CWizCategoryViewMessageItem* pItem = dynamic_cast<CWizCategoryViewMessageItem*>(item))
    {
        emit itemSelectionChanged();
    }
    else if (CWizCategoryViewBizGroupRootItem* pItem = dynamic_cast<CWizCategoryViewBizGroupRootItem*>(item))
    {
        bool bUseCount = pItem->isUnreadButtonUseable() && pItem->isSelected();
        if (bUseCount)
        {
            emit itemSelectionChanged();
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
    else if (CWizCategoryViewGroupRootItem* pItem = dynamic_cast<CWizCategoryViewGroupRootItem*>(item))
    {
        if (pItem->extraButtonClickTest())
        {
            promptGroupLimitMessage(pItem->kbGUID(), pItem->bizGUID());
        }

    }
}

void CWizCategoryView::updateGroupsData()
{
    CWizGroupDataArray arrayGroup;
    m_dbMgr.db().GetUserGroupInfo(arrayGroup);
    //
    CWizBizDataArray arrayBiz;
    m_dbMgr.db().GetUserBizInfo(false, arrayGroup, arrayBiz);
    //
    for (CWizBizDataArray::const_iterator it = arrayBiz.begin(); it != arrayBiz.end(); it++)
    {
        const WIZBIZDATA& biz = *it;
        CWizCategoryViewItemBase* pBizGroupItem = findBizGroupsRootItem(biz, false);
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
        CWizCategoryViewGroupRootItem* pGroupItem = findGroup(group.strGroupGUID);
        setGroupRootItemExtraButton(pGroupItem, group);
    }

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

void CWizCategoryView::createGroup()
{
    QString strExtInfo = WizService::ApiEntry::appstoreParam(false);
    QString strUrl = WizService::ApiEntry::standardCommandUrl("create_group", WIZ_TOKEN_IN_URL_REPLACE_PART, strExtInfo);
    WizShowWebDialogWithToken(tr("Create new group"), strUrl, window());
}

void CWizCategoryView::viewPersonalGroupInfo(const QString& groupGUID)
{
    QString extInfo = "kb=" + groupGUID + WizService::ApiEntry::appstoreParam();
    QString strUrl = WizService::ApiEntry::standardCommandUrl("view_personal_group", WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    WizShowWebDialogWithToken(tr("View group info"), strUrl, window());
}

void CWizCategoryView::viewBizGroupInfo(const QString& groupGUID, const QString& bizGUID)
{
    QString extInfo = "kb=" + groupGUID + "&biz=" + bizGUID + WizService::ApiEntry::appstoreParam();
    QString strUrl = WizService::ApiEntry::standardCommandUrl("view_biz_group", WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    WizShowWebDialogWithToken(tr("View group info"), strUrl, window());
}

void CWizCategoryView::managePersonalGroup(const QString& groupGUID)
{
    QString extInfo = "kb=" + groupGUID + WizService::ApiEntry::appstoreParam();
    QString strUrl = WizService::ApiEntry::standardCommandUrl("manage_personal_group", WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    WizShowWebDialogWithToken(tr("Manage group"), strUrl, window());
}

void CWizCategoryView::manageBizGroup(const QString& groupGUID, const QString& bizGUID)
{
    QString extInfo = "kb=" + groupGUID + "&biz=" + bizGUID + WizService::ApiEntry::appstoreParam();
    QString strUrl = WizService::ApiEntry::standardCommandUrl("manage_biz_group", WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    WizShowWebDialogWithToken(tr("Manage group"), strUrl, window());
}

void CWizCategoryView::promptGroupLimitMessage(const QString &groupGUID, const QString &/*bizGUID*/)
{
    CWizDatabase& db = m_dbMgr.db(groupGUID);
    QString strErrorMsg;
    if (db.GetStorageLimitMessage(strErrorMsg))
    {
        QMessageBox::warning(this, tr("Storage Limit Info"), strErrorMsg);
    }
    else if (db.GetTrafficLimitMessage(strErrorMsg))
    {
        QMessageBox::warning(this, tr("Traffic Limit Info"), strErrorMsg);
    }
    else if (db.GetNoteCountLimit(strErrorMsg))
    {
        QMessageBox::warning(this, tr("Note Count Limit Info"), tr("Group notes count limit exceeded!"));
    }
}

void CWizCategoryView::viewBizInfo(const QString& bizGUID)
{
    QString extInfo = "biz=" + bizGUID + WizService::ApiEntry::appstoreParam();
    QString strUrl = WizService::ApiEntry::standardCommandUrl("view_biz", WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    WizShowWebDialogWithToken(tr("View team info"), strUrl, window());
}

void CWizCategoryView::manageBiz(const QString& bizGUID, bool bUpgrade)
{
    QString extInfo = "biz=" + bizGUID;
    if (bUpgrade)
    {
        extInfo += _T("&p=payment");
    }
    extInfo += WizService::ApiEntry::appstoreParam();
    QString strUrl = WizService::ApiEntry::standardCommandUrl("manage_biz", WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    WizShowWebDialogWithToken(tr("Manage team"), strUrl, window());

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

    loadExpandState();

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
                    if(CATEGORY_TEAM_GROUPS == sectionName && !CWizOEMSettings::isForbidCreateBiz())
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

    CWizCategoryViewGroupRootItem* pGroupRoot = NULL;
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

    //unread documents
    pGroupRoot->setUnreadCount(m_dbMgr.db(strKbGUID).getGroupUnreadDocumentCount());

    if (pGroupRoot->isBizGroup())
    {
        CWizCategoryViewBizGroupRootItem *bizRootItem = dynamic_cast<CWizCategoryViewBizGroupRootItem *>(pGroupRoot->parent());
        if (bizRootItem)
        {
            bizRootItem->updateUnreadCount();
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

bool CWizCategoryView::createDocument(WIZDOCUMENTDATA& data)
{
    return createDocument(data, "<p><br/></p>", tr("New note"));
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

void CWizCategoryView::setBizRootItemExtraButton(CWizCategoryViewItemBase* pItem, const WIZBIZDATA& bizData)
{
    if (pItem)
    {
        CWizDatabase& db = m_dbMgr.db();
        if (bizData.bizIsDue || db.IsBizServiceExpr(bizData.bizGUID))
        {
            QString strIconPath = ::WizGetSkinResourcePath(m_app.userSettings().skin()) + "bizDue.png";
            pItem->setExtraButtonIcon(strIconPath);
        }
        else
        {
            pItem->setExtraButtonIcon("");
        }
    }
}

void CWizCategoryView::setGroupRootItemExtraButton(CWizCategoryViewItemBase* pItem, const WIZGROUPDATA& gData)
{
    if (pItem)
    {
        CWizDatabase& db = m_dbMgr.db(gData.strGroupGUID);
        if (db.IsStorageLimit() || db.IsTrafficLimit() || db.IsNoteCountLimit())
        {
            QString strIconPath = ::WizGetSkinResourcePath(m_app.userSettings().skin()) + "groupLimit.png";
            pItem->setExtraButtonIcon(strIconPath);
        }
        else
        {
            pItem->setExtraButtonIcon("");
        }
    }
}

void CWizCategoryView::moveFolderPostionBeforeTrash(const QString& strLocation)
{
    const QString strFolderPostion = "FolderPosition/";
    QSettings* setting = ExtensionSystem::PluginManager::settings();
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

bool CWizCategoryView::getAvailableNewNoteTagAndLocation(QString& strKbGUID, WIZTAGDATA& tag, QString& strLocation)
{
    bool bFallback = true;
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
        strLocation = m_dbMgr.db(strKbGUID).GetDefaultNoteLocation();
        bFallback = false;
    }
    else if (CWizCategoryViewGroupNoTagItem* pItem = currentCategoryItem<CWizCategoryViewGroupNoTagItem>())
    {
        strKbGUID = pItem->kbGUID();
        strLocation = m_dbMgr.db(strKbGUID).GetDefaultNoteLocation();
        bFallback = false;
    }
    else if (CWizCategoryViewGroupItem* pItem = currentCategoryItem<CWizCategoryViewGroupItem>())
    {
        strKbGUID = pItem->kbGUID();
        tag = pItem->tag();
        strLocation = m_dbMgr.db(strKbGUID).GetDefaultNoteLocation();
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

    return true;
}

void CWizCategoryView::quickSyncNewDocument(const QString& strKbGUID)
{
    /*FIXME:
     *创建笔记后快速同步笔记到服务器,防止用户新建笔记后使用评论功能时因服务器无该篇笔记导致问题.*/
    MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());
    mainWindow->quickSyncKb(strKbGUID);
}

void CWizCategoryView::updateGroupFolderPosition(CWizDatabase& db)
{
    saveGroupTagsPosition(db.kbGUID());
    db.SetGroupTagsPosModified();

    emit categoryItemPositionChanged(db.kbGUID());
}

void CWizCategoryView::updatePrivateFolderLocation(CWizDatabase& db, \
                                                    const QString& strOldLocation, const QString& strNewLocation)
{
    CWizCategoryViewAllFoldersItem* pItem = dynamic_cast<CWizCategoryViewAllFoldersItem* >(findAllFolderItem());
    if (!pItem)
        return;

    // the last item of folder root should be trash
    CWizCategoryViewTrashItem* trashItem = findTrash(db.kbGUID());
    if (trashItem && pItem->indexOfChild(trashItem) != pItem->childCount() - 1)
    {
        pItem->takeChild(pItem->indexOfChild(trashItem));
        pItem->insertChild(pItem->childCount(), trashItem);
    }

    if (strOldLocation != strNewLocation)
    {
        db.UpdateLocation(strOldLocation, strNewLocation);

    }

    QString str = getAllFoldersPosition();
    db.SetFoldersPos(str, -1);
    db.SetFoldersPosModified();

    emit categoryItemPositionChanged(db.kbGUID());
}

void CWizCategoryView::updatePrivateTagPosition(CWizDatabase& db)
{
    Q_UNUSED(db);
}

void CWizCategoryView::initGeneral()
{
    //CWizCategoryViewCategoryItem* pCategoryItem = new CWizCategoryViewCategoryItem(m_app, CATEGORY_GENERAL);
    //addTopLevelItem(pCategoryItem);

    CWizCategoryViewMessageItem* pMsg = new CWizCategoryViewMessageItem(m_app, CATEGORY_MESSAGES_ALL, CWizCategoryViewMessageItem::All);
    addTopLevelItem(pMsg);

    pMsg->setUnreadCount(m_dbMgr.db().getUnreadMessageCount());

    //QList<QTreeWidgetItem*> pList;
    //pList.append(new CWizCategoryViewMessageItem(m_app, CATEGORY_MESSAGES_SEND_TO_ME, CWizCategoryViewMessageItem::SendToMe));
    //pList.append(new CWizCategoryViewMessageItem(m_app, CATEGORY_MESSAGES_MODIFY, CWizCategoryViewMessageItem::ModifyNote));
    //pList.append(new CWizCategoryViewMessageItem(m_app, CATEGORY_MESSAGES_COMMENTS, CWizCategoryViewMessageItem::Comment));
    //pList.append(new CWizCategoryViewMessageItem(m_app, CATEGORY_MESSAGES_SEND_FROM_ME, CWizCategoryViewMessageItem::SendFromMe));
    //pMsg->addChildren(pList);

    loadShortcutState();

    initQuickSearches();
}

void CWizCategoryView::sortFolders()
{
    CWizCategoryViewAllFoldersItem* pFolderRoot = dynamic_cast<CWizCategoryViewAllFoldersItem *>(findAllFolderItem());
    if (!pFolderRoot)
        return;

    pFolderRoot->sortChildren(0, Qt::AscendingOrder);

    for (int i = 0; i < pFolderRoot->childCount(); i++)
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

void CWizCategoryView::sortGroupTags(const QString& strKbGUID, bool bReloadData)
{
    CWizCategoryViewGroupRootItem* groupRoot =
            dynamic_cast<CWizCategoryViewGroupRootItem*>(itemFromKbGUID(strKbGUID));
    if (!groupRoot)
        return;

    for (int i = 0; i < groupRoot->childCount(); i++)
    {
        CWizCategoryViewGroupItem* pItem = dynamic_cast<CWizCategoryViewGroupItem*>(groupRoot->child(i));
        sortGroupTags(pItem, bReloadData);
    }

    groupRoot->sortChildren(0, Qt::AscendingOrder);
}

void CWizCategoryView::sortGroupTags(CWizCategoryViewGroupItem* pItem, bool bReloadData)
{
    if (!pItem)
        return;

    if (bReloadData)
    {
        CWizDatabase& db = m_dbMgr.db(pItem->kbGUID());
        pItem->reload(db);
    }

    for (int i = 0; i < pItem->childCount(); i++)
    {
        CWizCategoryViewGroupItem* childItem = dynamic_cast<CWizCategoryViewGroupItem*>(pItem->child(i));
        sortGroupTags(childItem, bReloadData);
    }

    pItem->sortChildren(0, Qt::AscendingOrder);
}

void CWizCategoryView::saveGroupTagsPosition(const QString& strKbGUID)
{
    CWizCategoryViewGroupRootItem* rootItem =
            dynamic_cast<CWizCategoryViewGroupRootItem*>(itemFromKbGUID(strKbGUID));
    if (!rootItem)
        return;

    CWizDatabase& db = m_dbMgr.db(strKbGUID);
    db.blockSignals(true);
    for (int i = 0; i < rootItem->childCount(); i++)
    {
        CWizCategoryViewGroupItem* childItem = dynamic_cast<CWizCategoryViewGroupItem* >(rootItem->child(i));
        if (childItem)
        {
            childItem->setTagPosition(rootItem->indexOfChild(childItem));
            saveGroupTagsPosition(db, childItem);
        }
    }

    db.blockSignals(false);
}

void CWizCategoryView::saveGroupTagsPosition(CWizDatabase& db, CWizCategoryViewGroupItem* pItem)
{
    if (!pItem)
        return;

    WIZTAGDATA tag = pItem->tag();
    db.ModifyTag(tag);

    for (int i = 0; i < pItem->childCount(); i++)
    {
        CWizCategoryViewGroupItem* childItem = dynamic_cast<CWizCategoryViewGroupItem* >(pItem->child(i));
        if (childItem)
        {
            childItem->setTagPosition(i);
            saveGroupTagsPosition(db, childItem);
        }
    }
}

QString CWizCategoryView::getAllFoldersPosition()
{
    CWizCategoryViewAllFoldersItem* pItem =
            dynamic_cast<CWizCategoryViewAllFoldersItem*>(findAllFolderItem());
    if (!pItem)
        return QString();

    QString str = "{";
    int nStartPos = 1;
    for (int i = 0; i < pItem->childCount(); i++)
    {
        CWizCategoryViewFolderItem* childItem = dynamic_cast<CWizCategoryViewFolderItem* >(pItem->child(i));
        Q_ASSERT(childItem);
        str += getAllFoldersPosition(childItem, nStartPos);

        if (i < pItem->childCount() - 1)
            str += ",\n";
    }
    str += "}";
    return str;
}

QString CWizCategoryView::getAllFoldersPosition(CWizCategoryViewFolderItem* pItem, int& nStartPos)
{
    if (!pItem)
        return QString();

    QString str ="\"" + pItem->location() + "\": " + QString::number(nStartPos);
    nStartPos ++;

    for (int i = 0; i < pItem->childCount(); i++)
    {
        CWizCategoryViewFolderItem* childItem = dynamic_cast<CWizCategoryViewFolderItem* >(pItem->child(i));
        Q_ASSERT(childItem);
        str += ", \n";
        str += getAllFoldersPosition(childItem, nStartPos);
    }

    return str;
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
    m_dbMgr.db().GetAllLocationsWithExtra(arrayAllLocation);

    /*
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
    */

    if (arrayAllLocation.empty()) {
        arrayAllLocation.push_back(m_dbMgr.db().GetDefaultNoteLocation());
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

        if (strLocation.isEmpty())
            continue;

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


void CWizCategoryView::loadShortcutState()
{
    CWizCategoryViewShortcutRootItem* pShortcutRoot = 0;
    for (int i = 0 ; i < topLevelItemCount(); i++)
    {
        pShortcutRoot = dynamic_cast<CWizCategoryViewShortcutRootItem*>(topLevelItem(i));
        if (pShortcutRoot)
            break;
    }

    if (!pShortcutRoot)
    {
        pShortcutRoot = new CWizCategoryViewShortcutRootItem(m_app, CATEGORY_SHORTCUTS);
        addTopLevelItem(pShortcutRoot);
    }

    //
    QString strData = m_dbMgr.db().meta(CATEGORY_META, CATEGORY_SHORTCUT);
    if (!strData.isEmpty())
    {
        QStringList shortCutList = strData.split(';');
        foreach (QString shortCut, shortCutList) {
            QStringList shortDataList = shortCut.split(',');
            if (shortDataList.count() != 2)
                continue;

            QString strKbGuid = shortDataList.first();
            QString strGuid = shortDataList.last();
            CWizDatabase &db = m_dbMgr.db(strKbGuid);
            WIZDOCUMENTDATA doc;
            if (db.DocumentFromGUID(strGuid, doc))
            {
                bool isEncrypted = doc.nProtected == 1;
                CWizCategoryViewShortcutItem *pShortcutItem =
                        new CWizCategoryViewShortcutItem(m_app, doc.strTitle, doc.strKbGUID, doc.strGUID, isEncrypted);
                pShortcutRoot->addChild(pShortcutItem);
            }

        }
    }

    if (pShortcutRoot->childCount() == 0)
    {
        pShortcutRoot->addPlaceHoldItem();
    }

    pShortcutRoot->sortChildren(0, Qt::AscendingOrder);
}

void CWizCategoryView::saveShortcutState()
{

    CWizCategoryViewShortcutRootItem *pShortcutRoot = 0;
    for (int i = 0 ; i < topLevelItemCount(); i++)
    {
        pShortcutRoot = dynamic_cast<CWizCategoryViewShortcutRootItem*>(topLevelItem(i));
        if (pShortcutRoot)
            break;
    }

    QString strShortcutData = "";

    if (pShortcutRoot && pShortcutRoot->childCount() > 0)
    {
        for (int i = 0; i < pShortcutRoot->childCount(); i++)
        {
            CWizCategoryViewShortcutItem *pItem = dynamic_cast<CWizCategoryViewShortcutItem*>(pShortcutRoot->child(i));
            if (pItem)
            {
                strShortcutData += pItem->kbGUID() + "," + pItem->guid() + ";";
            }
        }
    }
    m_dbMgr.db().setMeta(CATEGORY_META, CATEGORY_SHORTCUT, strShortcutData);
}

void CWizCategoryView::loadExpandState()
{
    QSettings* settings = ExtensionSystem::PluginManager::settings();
    settings->beginGroup(TREEVIEW_STATE);
    m_strSelectedId = selectedId(settings);

    for (int i = 0 ; i < topLevelItemCount(); i++) {
        loadChildState(topLevelItem(i), settings);
    }
    settings->endGroup();
}

void CWizCategoryView::initTags()
{
    CWizCategoryViewAllTagsItem* pAllTagsItem = new CWizCategoryViewAllTagsItem(m_app, CATEGORY_TAGS, m_dbMgr.db().kbGUID());
    addTopLevelItem(pAllTagsItem);

    initTags(pAllTagsItem, "");

    pAllTagsItem->setExpanded(true);
    pAllTagsItem->sortChildren(0, Qt::AscendingOrder);

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
        setBizRootItemExtraButton(pBizGroupItem, biz);

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

void CWizCategoryView::initBiz(const WIZBIZDATA& biz)
{
    CWizCategoryViewBizGroupRootItem* pBizGroupItem = new CWizCategoryViewBizGroupRootItem(m_app, biz);
    setBizRootItemExtraButton(pBizGroupItem, biz);
    addTopLevelItem(pBizGroupItem);

    CWizGroupDataArray arrayGroup;
    m_dbMgr.db().GetUserGroupInfo(arrayGroup);

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

    //
    setGroupRootItemExtraButton(pGroupItem, group);

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

void CWizCategoryView::initQuickSearches()
{
    CWizCategoryViewSearchRootItem* pSearchRoot = new CWizCategoryViewSearchRootItem(m_app, CATEGORY_SEARCH);
    addTopLevelItem(pSearchRoot);

//    CWizCategoryViewSearchItem* pSearchByAttributes = new CWizCategoryViewSearchItem(m_app, tr("Search by Attributes(Personal Notes)"));
    CWizCategoryViewSearchItem* pSearchByDTCreated = new CWizCategoryViewSearchItem(m_app, tr("Search by Date Created"));
    CWizCategoryViewSearchItem* pSearchByDTModified = new CWizCategoryViewSearchItem(m_app, tr("Search by Date Modified"));
    CWizCategoryViewSearchItem* pSearchByDTAccessed = new CWizCategoryViewSearchItem(m_app, tr("Search by Date Accessed"));

//    pSearchRoot->addChild(pSearchByAttributes);
    pSearchRoot->addChild(pSearchByDTCreated);
    pSearchRoot->addChild(pSearchByDTModified);
    pSearchRoot->addChild(pSearchByDTAccessed);

    CWizCategoryViewTimeSearchItem* pCreatedToday = new CWizCategoryViewTimeSearchItem(m_app, tr("Created since Today"), "DT_CREATED > %1", DateInterval_Today);
    CWizCategoryViewTimeSearchItem* pCreatedYestoday = new CWizCategoryViewTimeSearchItem(m_app, tr("Created since Yestoday"), "DT_CREATED > %1", DateInterval_Yestoday);
    CWizCategoryViewTimeSearchItem* pCreatedDayBeforeYestoday = new CWizCategoryViewTimeSearchItem(m_app, tr("Created since the day before yestoday"), "DT_CREATED > %1", DateInterval_TheDayBeforeYestoday);
    CWizCategoryViewTimeSearchItem* pCreatedOneWeek = new CWizCategoryViewTimeSearchItem(m_app, tr("Created since one week"), "DT_CREATED > %1", DateInterval_LastWeek);
    CWizCategoryViewTimeSearchItem* pCreatedOneMonth = new CWizCategoryViewTimeSearchItem(m_app, tr("Created since one month"), "DT_CREATED > %1", DateInterval_LastMonth);
    pSearchByDTCreated->addChild(pCreatedToday);
    pSearchByDTCreated->addChild(pCreatedYestoday);
    pSearchByDTCreated->addChild(pCreatedDayBeforeYestoday);
    pSearchByDTCreated->addChild(pCreatedOneWeek);
    pSearchByDTCreated->addChild(pCreatedOneMonth);

    CWizCategoryViewTimeSearchItem* pModifiedToday = new CWizCategoryViewTimeSearchItem(m_app, tr("Modified since Today"), "DT_MODIFIED > %1", DateInterval_Today);
    CWizCategoryViewTimeSearchItem* pModifiedYestoday = new CWizCategoryViewTimeSearchItem(m_app, tr("Modified since Yestoday"), "DT_MODIFIED > %1", DateInterval_Yestoday);
    CWizCategoryViewTimeSearchItem* pModifiedDayBeforeYestoday = new CWizCategoryViewTimeSearchItem(m_app, tr("Modified since the day before yestoday"), "DT_MODIFIED > %1", DateInterval_TheDayBeforeYestoday);
    CWizCategoryViewTimeSearchItem* pModifiedOneWeek = new CWizCategoryViewTimeSearchItem(m_app, tr("Modified since one week"), "DT_MODIFIED > %1", DateInterval_LastWeek);
    CWizCategoryViewTimeSearchItem* pModifiedOneMonth = new CWizCategoryViewTimeSearchItem(m_app, tr("Modified since one month"), "DT_MODIFIED > %1", DateInterval_LastMonth);
    pSearchByDTModified->addChild(pModifiedToday);
    pSearchByDTModified->addChild(pModifiedYestoday);
    pSearchByDTModified->addChild(pModifiedDayBeforeYestoday);
    pSearchByDTModified->addChild(pModifiedOneWeek);
    pSearchByDTModified->addChild(pModifiedOneMonth);

    CWizCategoryViewTimeSearchItem* pAccessedToday = new CWizCategoryViewTimeSearchItem(m_app, tr("Accessed since Today"), "DT_ACCESSED > %1", DateInterval_Today);
    CWizCategoryViewTimeSearchItem* pAccessedYestoday = new CWizCategoryViewTimeSearchItem(m_app, tr("Accessed since Yestoday"), "DT_ACCESSED > %1", DateInterval_Yestoday);
    CWizCategoryViewTimeSearchItem* pAccessedDayBeforeYestoday = new CWizCategoryViewTimeSearchItem(m_app, tr("Accessed since the day before yestoday"), "DT_ACCESSED > %1", DateInterval_TheDayBeforeYestoday);
    CWizCategoryViewTimeSearchItem* pAccessedOneWeek = new CWizCategoryViewTimeSearchItem(m_app, tr("Accessed since one week"), "DT_ACCESSED > %1", DateInterval_LastWeek);
    CWizCategoryViewTimeSearchItem* pAccessedOneMonth = new CWizCategoryViewTimeSearchItem(m_app, tr("Accessed since one month"), "DT_ACCESSED > %1", DateInterval_LastMonth);
    pSearchByDTAccessed->addChild(pAccessedToday);
    pSearchByDTAccessed->addChild(pAccessedYestoday);
    pSearchByDTAccessed->addChild(pAccessedDayBeforeYestoday);
    pSearchByDTAccessed->addChild(pAccessedOneWeek);
    pSearchByDTAccessed->addChild(pAccessedOneMonth);

    QMap<QString, QString> customMap;
    loadCustomAdvancedSearchParamFromDB(customMap);
    if (customMap.count() > 0)
    {
        CWizCategoryViewSearchItem* pSearchByCustomSQL = new CWizCategoryViewSearchItem(m_app, CATEGORY_SEARCH_BYCUSTOM);
        pSearchRoot->addChild(pSearchByCustomSQL);
        QMap<QString, QString>::Iterator it;
        for (it = customMap.begin(); it != customMap.end(); it++)
        {
            QString where, name, keyword;
            int scope;
            CWizAdvancedSearchDialog::paramToSQL(it.value(), where, keyword, name, scope);
            CWizCategoryViewCustomSearchItem* pCustomSearch = new CWizCategoryViewCustomSearchItem(m_app,
                                                                                                   name, it.value(), where, it.key(), keyword, scope);
            pSearchByCustomSQL->addChild(pCustomSearch);
        }
    }
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

CWizCategoryViewItemBase*CWizCategoryView::findAllSearchItem()
{
    for (int i = 0; i < topLevelItemCount(); i++) {
        if (topLevelItem(i)->type() != ItemType_QuickSearchRootItem)
            continue;

        CWizCategoryViewSearchRootItem* pItem = dynamic_cast<CWizCategoryViewSearchRootItem*>(topLevelItem(i));
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
        if (strLocationName.isEmpty())
            return NULL;
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

        CWizCategoryViewFolderItem* pFolderItem = createFolderItem(parent, strCurrentLocation);
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

void CWizCategoryView::on_tags_positionChanged(const QString& strKbGUID)
{
    bool reloadData = true;
    sortGroupTags(strKbGUID, reloadData);
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
        findAction(ActionRecovery)->setEnabled(false);
    } else {
        CWizCategoryViewTrashItem* pItem = findTrash(strKbGUID);
        if (pItem) pItem->setHidden(false);

        findAction(ActionNewItem)->setEnabled(true);
        findAction(ActionRenameItem)->setEnabled(true);
        findAction(ActionDeleteItem)->setEnabled(true);
        findAction(ActionRecovery)->setEnabled(true);
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

void CWizCategoryView::on_group_bizChanged(const QString& strKbGUID)
{
    //TODO:
}

void CWizCategoryView::on_groupDocuments_unreadCount_modified(const QString& strKbGUID)
{
    updateGroupFolderDocumentCount(strKbGUID);
}

void CWizCategoryView::on_itemPosition_changed(CWizCategoryViewItemBase* pItem)
{
    CWizDatabase& db = m_dbMgr.db(pItem->kbGUID());
    if (db.IsGroup())
    {
        updateGroupFolderPosition(db);
    }
    else
    {
        if (CWizCategoryViewFolderItem* item = dynamic_cast<CWizCategoryViewFolderItem*>(pItem))
        {
            CWizCategoryViewItemBase* folderRoot = findAllFolderItem();
            QTreeWidgetItem* parentItem = item->parent();
            if (parentItem == 0)
            {
//                parentItem = folderRoot;
                //拖拽存在bug，会导致普通元素变成顶级元素，此处进行特殊处理。
//                int index = indexOfTopLevelItem(item);
//                takeTopLevelItem(index);
//                folderRoot->insertChild(0, item);
                parentItem = folderRoot;
            }

            QString strName = getUseableItemName(parentItem, item);
//            qDebug() << "get useable item name : " << strName;
            QString strNewLocation = "/" + strName + "/";
            item->setText(0, strName);


            if (parentItem != folderRoot)
            {
                CWizCategoryViewItemBase* parentBase = dynamic_cast<CWizCategoryViewItemBase*>(parentItem);
                if (!parentBase)
                    return;

//                qDebug() << "parent is not root , parent name : " << parentBase->name();
                strNewLocation = parentBase->name() + strNewLocation.remove(0, 1);
                parentItem = parentBase->parent();
            }

            QString strOldLocation = item->location();

//            qDebug() << "item position changed , " << strOldLocation << " ,  new location : " << strNewLocation;
            resetFolderLocation(item, strNewLocation);
            updatePrivateFolderLocation(db, strOldLocation, strNewLocation);
        }

        sortFolders();
    }
}


void CWizCategoryView::createDocumentByHtml(const QString& strHtml, const QString& strTitle)
{
    WIZDOCUMENTDATA data;
    createDocument(data, strHtml, strTitle);
}

void CWizCategoryView::createDocumentByHtml(const QString& strFileName,
                                            const QString& strHtml, const QString& strTitle)
{
    //为了提取和file路径相关联的图片，不直接使用strHtml创建笔记，而是在创建之后更新笔记内容
    WIZDOCUMENTDATA data;
    createDocument(data, "<p><br/></p>", strTitle);
    CWizDatabase& db = m_dbMgr.db(data.strKbGUID);
    db.UpdateDocumentData(data, strHtml, strFileName, 0);
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

void CWizCategoryView::loadChildState(QTreeWidgetItem* pItem, QSettings* settings)
{
    loadItemState(pItem, settings);

    if (!m_strSelectedId.isEmpty()) {
        CWizCategoryViewItemBase* pi = dynamic_cast<CWizCategoryViewItemBase*>(pItem);
        if (!pi)
            return;

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
    if (!pItem)
        return;

    QString strId = pItem->id();

    bool bExpand = settings->value(strId).toBool();

    if (bExpand)
        expandItem(pItem);
    else
        collapseItem(pItem);
}

void CWizCategoryView::saveExpandState()
{
    QSettings* settings = ExtensionSystem::PluginManager::settings();
    settings->beginGroup(TREEVIEW_STATE);
    settings->remove("");
    for (int i = 0 ; i < topLevelItemCount(); i++) {
        saveChildState(topLevelItem(i), settings);
    }

    saveSelected(settings);
    settings->endGroup();

    settings->sync();
}

QString CWizCategoryView::selectedId(QSettings* settings)
{
    QString strItem = settings->value(TREEVIEW_SELECTED_ITEM).toString();

    return strItem;
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

   settings->setValue(strId, bExpand);
}

void CWizCategoryView::advancedSearchByCustomParam(const QString& strParam)
{
    QString strSql, strName, strKeyword;
    int scope;
    CWizAdvancedSearchDialog::paramToSQL(strParam, strSql, strKeyword, strName, scope);

    MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());
    CWizSearcher* searcher = mainWindow->searcher();
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

void CWizCategoryView::saveCustomAdvancedSearchParamToDB(const QString& strGuid, const QString& strParam)
{
    m_dbMgr.db().SetMeta(QUICK_SEARCH_META, strGuid, strParam);
}

void CWizCategoryView::loadCustomAdvancedSearchParamFromDB(QMap<QString, QString>& paramMap)
{
    CWizMetaDataArray arrayMeta;
     m_dbMgr.db().GetMetasByName(QUICK_SEARCH_META, arrayMeta);
     CWizMetaDataArray::iterator it;
     for (it = arrayMeta.begin(); it != arrayMeta.end(); it++)
     {
         paramMap.insert(it->strKey, it->strValue);
     }
}

void CWizCategoryView::deleteCustomAdvancedSearchParamFromDB(const QString& strGuid)
{
    m_dbMgr.db().deleteMetaByKey(QUICK_SEARCH_META, strGuid);
}

CWizCategoryViewFolderItem* CWizCategoryView::createFolderItem(QTreeWidgetItem* parent, const QString& strLocation)
{
    CWizCategoryViewFolderItem* pFolderItem = new CWizCategoryViewFolderItem(m_app, strLocation, m_dbMgr.db().kbGUID());
    parent->addChild(pFolderItem);
    m_dbMgr.db().AddExtraFolder(strLocation);
    return pFolderItem;
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

    settings->setValue(TREEVIEW_SELECTED_ITEM, pItem->id());
}
