#include "wizDocumentListView.h"

#include <QtGui>

#include "wizDocumentListViewItem.h"
#include "wizCategoryView.h"
#include "widgets/wizScrollBar.h"
#include "wiznotestyle.h"
#include "wiztaglistwidget.h"
#include "share/wizsettings.h"
#include "wizFolderSelector.h"
#include "wizProgressDialog.h"
#include "share/wizUserAvatar.h"
#include "wizmainwindow.h"


class CWizDocumentListViewDelegate : public QStyledItemDelegate
{
public:
    CWizDocumentListViewDelegate(QWidget* parent)
        : QStyledItemDelegate(parent)
    {
    }

    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
    {
        QSize sz = QStyledItemDelegate::sizeHint(option, index);
        sz.setHeight(sz.height() + (option.fontMetrics.height() + 2) * 3 + 2 + 16);
        return sz;
    }
};

// Document actions
#define WIZACTION_LIST_DELETE   QObject::tr("Delete")
#define WIZACTION_LIST_TAGS     QObject::tr("Tags...")
#define WIZACTION_LIST_MOVE_DOCUMENT QObject::tr("Move Document")
#define WIZACTION_LIST_COPY_DOCUMENT QObject::tr("Copy Document")

// Message actions
#define WIZACTION_LIST_MESSAGE_MARK_READ    QObject::tr("Mark as read")
#define WIZACTION_LIST_MESSAGE_DELETE       QObject::tr("Delete Message(s)")

CWizDocumentListView::CWizDocumentListView(CWizExplorerApp& app, QWidget *parent /*= 0*/)
    : QListWidget(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_tagList(NULL)
{
    setFrameStyle(QFrame::NoFrame);
    setAttribute(Qt::WA_MacShowFocusRect, false);

    connect(this, SIGNAL(itemSelectionChanged()), SLOT(on_itemSelectionChanged()));

    // use custom scrollbar
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_vScroll = new CWizScrollBar(this);
    m_vScroll->syncWith(verticalScrollBar());

    //QScrollAreaKineticScroller *newScroller = new QScrollAreaKineticScroller();
    //newScroller->setWidget(this);
    //m_kinecticScroll = new QsKineticScroller(this);
    //m_kinecticScroll->enableKineticScrollFor(this);

//#ifndef Q_OS_MAC
    // smoothly scroll
    m_vscrollCurrent = 0;
    m_vscrollOldPos = 0;
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(on_vscroll_valueChanged(int)));
    connect(verticalScrollBar(), SIGNAL(actionTriggered(int)), SLOT(on_vscroll_actionTriggered(int)));
    connect(&m_vscrollTimer, SIGNAL(timeout()), SLOT(on_vscroll_update()));
//#endif //Q_OS_MAC

    setItemDelegate(new CWizDocumentListViewDelegate(this));

    // setup style
    QString strSkinName = m_app.userSettings().skin();
    setStyle(::WizGetStyle(strSkinName));

    QPalette pal = palette();
    //pal.setColor(QPalette::Base, QColor(247,247,247));
    pal.setColor(QPalette::Base, WizGetDocumentsBackroundColor(strSkinName));
    setPalette(pal);

    connect(&m_dbMgr, SIGNAL(tagCreated(const WIZTAGDATA&)), \
            SLOT(on_tag_created(const WIZTAGDATA&)));

    connect(&m_dbMgr, SIGNAL(tagModified(const WIZTAGDATA&, const WIZTAGDATA&)), \
            SLOT(on_tag_modified(const WIZTAGDATA&, const WIZTAGDATA&)));

    connect(&m_dbMgr, SIGNAL(documentCreated(const WIZDOCUMENTDATA&)), \
            SLOT(on_document_created(const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(documentModified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)), \
            SLOT(on_document_modified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(documentDeleted(const WIZDOCUMENTDATA&)),
            SLOT(on_document_deleted(const WIZDOCUMENTDATA&)));

    // message
    connect(&m_dbMgr.db(), SIGNAL(messageModified(const WIZMESSAGEDATA&, const WIZMESSAGEDATA&)),
            SLOT(on_message_modified(const WIZMESSAGEDATA&, const WIZMESSAGEDATA&)));

    connect(&m_dbMgr.db(), SIGNAL(messageDeleted(const WIZMESSAGEDATA&)),
            SLOT(on_message_deleted(const WIZMESSAGEDATA&)));


    // thumb cache
    m_thumbCache = new CWizThumbIndexCache(app);
    connect(m_thumbCache, SIGNAL(loaded(const WIZABSTRACT&)),
            SLOT(on_document_abstractLoaded(const WIZABSTRACT&)));

    QThread *thread = new QThread();
    m_thumbCache->moveToThread(thread);
    thread->start();

    // avatar downloader
    //m_avatarDownloader = new CWizUserAvatarDownloaderHost(m_dbMgr.db().GetAvatarPath(), this);
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    m_avatarDownloader = mainWindow->avatarHost();
    connect(m_avatarDownloader, SIGNAL(downloaded(const QString&)),
            SLOT(on_userAvatar_downloaded(const QString&)));

    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);
    viewport()->setAcceptDrops(true);

    setSelectionMode(QAbstractItemView::ExtendedSelection);

    // message context menu
    m_menuMessage = new QMenu(this);
    m_menuMessage->addAction(WIZACTION_LIST_MESSAGE_MARK_READ, this,
                             SLOT(on_action_message_mark_read()));
    m_menuMessage->addAction(WIZACTION_LIST_MESSAGE_DELETE, this,
                             SLOT(on_action_message_delete()));

    // document context menu
    m_menuDocument = new QMenu(this);
    m_menuDocument->addAction(WIZACTION_LIST_TAGS, this,
                              SLOT(on_action_selectTags()));
    m_menuDocument->addSeparator();
    QAction* actionDeleteDoc = m_menuDocument->addAction(WIZACTION_LIST_DELETE,
                                                         this, SLOT(on_action_deleteDocument()), QKeySequence::Delete);
    QAction* actionMoveDoc = m_menuDocument->addAction(WIZACTION_LIST_MOVE_DOCUMENT,
                                                       this, SLOT(on_action_moveDocument()), QKeySequence("Ctrl+Shift+M"));
    QAction* actionCopyDoc = m_menuDocument->addAction(WIZACTION_LIST_COPY_DOCUMENT,
                                                       this, SLOT(on_action_copyDocument()), QKeySequence("Ctrl+Shift+C"));

    // not implement, hide currently.
    actionCopyDoc->setVisible(false);

    // Add to widget's actions list
    addAction(actionDeleteDoc);
    addAction(actionMoveDoc);
    addAction(actionCopyDoc);
    actionDeleteDoc->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionMoveDoc->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    //actionCopyDoc->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    //m_actionEncryptDocument = new QAction(tr("Encrypt Document"), m_menu);
    //connect(m_actionEncryptDocument, SIGNAL(triggered()), SLOT(on_action_encryptDocument()));
    //m_menu->addAction(m_actionEncryptDocument);
}

CWizDocumentListView::~CWizDocumentListView()
{
}

void CWizDocumentListView::resizeEvent(QResizeEvent* event)
{
    // reset scrollbar position
    m_vScroll->resize(m_vScroll->sizeHint().width(), event->size().height());
    m_vScroll->move(event->size().width() - m_vScroll->sizeHint().width(), 0);

    QListWidget::resizeEvent(event);
}

void CWizDocumentListView::setDocuments(const CWizDocumentDataArray& arrayDocument)
{
    //reset
    clear();
    verticalScrollBar()->setValue(0);

    addDocuments(arrayDocument);
}

void CWizDocumentListView::setDocuments(const CWizMessageDataArray& arrayMessage)
{
    //reset
    clear();
    verticalScrollBar()->setValue(0);

    addDocuments(arrayMessage);
}

void CWizDocumentListView::addDocuments(const CWizDocumentDataArray& arrayDocument)
{
    CWizDocumentDataArray::const_iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
        addDocument(*it, false);
    }

    sortItems();
}

void CWizDocumentListView::addDocuments(const CWizMessageDataArray& arrayMessage)
{
    CWizMessageDataArray::const_iterator it;
    for (it = arrayMessage.begin(); it != arrayMessage.end(); it++) {
        addDocument(*it, false);
    }

    sortItems();
}

int CWizDocumentListView::addDocument(const WIZDOCUMENTDATA& data, bool sort)
{
    CWizDocumentListViewItem* pItem = new CWizDocumentListViewItem(data);
    addItem(pItem);

    if (sort) {
        sortItems();
    }

    Q_EMIT documentCountChanged();
    return count();
}

int CWizDocumentListView::addDocument(const WIZMESSAGEDATA& data, bool sort)
{
    CWizDocumentListViewItem* pItem = new CWizDocumentListViewItem(data);
    addItem(pItem);

    if (sort) {
        sortItems();
    }

    Q_EMIT documentCountChanged();
    return count();
}

bool CWizDocumentListView::acceptDocument(const WIZDOCUMENTDATA& document)
{
    return m_app.category().acceptDocument(document);
}

void CWizDocumentListView::addAndSelectDocument(const WIZDOCUMENTDATA& document)
{
    Q_ASSERT(acceptDocument(document));

    int index = documentIndexFromGUID(document.strGUID);
    if (-1 == index) {
        index = addDocument(document, false);
    }

    if (-1 == index)
        return;

    setCurrentItem(item(index), QItemSelectionModel::ClearAndSelect);
    sortItems();
}

void CWizDocumentListView::getSelectedDocuments(CWizDocumentDataArray& arrayDocument)
{
    QList<QListWidgetItem*> items = selectedItems();

    QList<QListWidgetItem*>::const_iterator it;
    for (it = items.begin(); it != items.end(); it++)
    {
        QListWidgetItem* pItem = *it;

        CWizDocumentListViewItem* pDocumentItem = dynamic_cast<CWizDocumentListViewItem*>(pItem);
        if (!pDocumentItem)
            continue;

        // if document is message type
        if (pDocumentItem->type() == CWizDocumentListViewItem::MessageDocument) {
            QString strKbGUID = pDocumentItem->message().kbGUID;
            QString strGUID = pDocumentItem->message().documentGUID;

            // document must have record in database.
            WIZDOCUMENTDATA doc;
            if (!m_dbMgr.db(strKbGUID).DocumentFromGUID(strGUID, doc)) {
                qDebug() << "[getSelectedDocuments] failed to query document from guid";
                continue;
            }

            // no matter document exist or not, just push it
            arrayDocument.push_back(doc);

        } else {
            arrayDocument.push_back(pDocumentItem->document());
        }
    }
}

void CWizDocumentListView::contextMenuEvent(QContextMenuEvent * e)
{
    CWizDocumentListViewItem* pItem = dynamic_cast<CWizDocumentListViewItem*>(itemAt(e->pos()));

    if (!pItem)
        return;

    if (pItem->type() == CWizDocumentListViewItem::MessageDocument) {
        m_menuMessage->popup(mapToGlobal(e->pos()));
    } else {
        m_menuDocument->popup(mapToGlobal(e->pos()));
    }
}

void CWizDocumentListView::resetPermission()
{
    CWizDocumentDataArray arrayDocument;
    QList<QListWidgetItem*> items = selectedItems();
    foreach (QListWidgetItem* it, items) {
        if (CWizDocumentListViewItem* item = dynamic_cast<CWizDocumentListViewItem*>(it)) {
            arrayDocument.push_back(item->document());
        }
    }

    bool bGroup = isDocumentsWithGroupDocument(arrayDocument);
    bool bDeleted = isDocumentsWithDeleted(arrayDocument);
    bool bCanDelete = isDocumentsAllCanDelete(arrayDocument);

    // if group documents or deleted documents selected
    if (bGroup || bDeleted) {
        findAction(WIZACTION_LIST_TAGS)->setEnabled(false);
    } else {
        findAction(WIZACTION_LIST_TAGS)->setEnabled(true);
    }

    // deleted user private documents
    if (!bGroup) {
        findAction(WIZACTION_LIST_MOVE_DOCUMENT)->setEnabled(true);
    } else {
        findAction(WIZACTION_LIST_MOVE_DOCUMENT)->setEnabled(false);
    }

    // disable delete if permission is not enough
    if (!bCanDelete) {
        findAction(WIZACTION_LIST_DELETE)->setEnabled(false);
    } else {
        findAction(WIZACTION_LIST_DELETE)->setEnabled(true);
    }
}

QAction* CWizDocumentListView::findAction(const QString& strName)
{
    QList<QAction *> actionList;
    actionList.append(m_menuDocument->actions());

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

bool CWizDocumentListView::isDocumentsWithDeleted(const CWizDocumentDataArray& arrayDocument)
{
    foreach (const WIZDOCUMENTDATAEX& doc, arrayDocument) {
       if (doc.strLocation.startsWith(LOCATION_DELETED_ITEMS)) {
           return true;
       }
    }

    return false;
}

bool CWizDocumentListView::isDocumentsWithGroupDocument(const CWizDocumentDataArray& arrayDocument)
{
    QString strUserGUID = m_dbMgr.db().kbGUID();
    foreach (const WIZDOCUMENTDATAEX& doc, arrayDocument) {
        if (doc.strKbGUID != strUserGUID) {
            return true;
        }
    }

    return false;
}

bool CWizDocumentListView::isDocumentsAllCanDelete(const CWizDocumentDataArray& arrayDocument)
{
    foreach (const WIZDOCUMENTDATAEX& doc, arrayDocument) {
        int nPerm = m_dbMgr.db(doc.strKbGUID).permission();
        if (nPerm > WIZ_USERGROUP_EDITOR) {
                return false;
        }
    }

    return true;
}

void CWizDocumentListView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition.setX(event->pos().x());
        m_dragStartPosition.setY(event->pos().y());
    }

    QListWidget::mousePressEvent(event);
}

void CWizDocumentListView::mouseMoveEvent(QMouseEvent* event)
{
    if ((event->buttons() & Qt::LeftButton) && \
            (event->pos() - m_dragStartPosition).manhattanLength() > QApplication::startDragDistance()) {
        setState(QAbstractItemView::DraggingState);
    }

    QListWidget::mouseMoveEvent(event);
}

QPixmap WizGetDocumentDragBadget(int nCount)
{
    QString strFileName = WizGetResourcesPath() + "skins/document_drag.png";
    QPixmap pixmap(strFileName);

    if (pixmap.isNull()) {
        return NULL;
    }

    // default
    QSize szPixmap(32, 32);

    // count badget width
    QFont font;
    font.setPixelSize(10);
    QFontMetrics fm(font);
    int width = fm.width(QString::number(nCount));
    QRect rectBadget(0, 0, width + 15, 16);

    QPixmap pixmapBadget(rectBadget.size());
    pixmapBadget.fill(Qt::transparent);

    // draw badget
    QPainter p(&pixmapBadget);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen = p.pen();
    pen.setWidth(2);
    pen.setColor("white");
    p.setPen(pen);
    QBrush brush = p.brush();
    brush.setColor("red");
    brush.setStyle(Qt::SolidPattern);
    p.setBrush(brush);

    p.drawEllipse(rectBadget);
    p.drawText(rectBadget,  Qt::AlignCenter, QString::number(nCount));

    // draw badget on icon
    QPixmap pixmapDragIcon(szPixmap.width() + rectBadget.width() / 2, szPixmap.height());
    pixmapDragIcon.fill(Qt::transparent);
    QPainter painter(&pixmapDragIcon);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPixmap(0, 0, pixmap.scaled(szPixmap));
    painter.drawPixmap(pixmapDragIcon.width() -  rectBadget.width(),
                       pixmapDragIcon.height() - rectBadget.height(),
                       pixmapBadget);

    return pixmapDragIcon;
}

void CWizDocumentListView::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions);

    CWizStdStringArray arrayGUID;

    QList<QListWidgetItem*> items = selectedItems();
    foreach (QListWidgetItem* it, items) {
        if (CWizDocumentListViewItem* item = dynamic_cast<CWizDocumentListViewItem*>(it)) {
            // not support drag group document currently
            if (item->document().strKbGUID != m_dbMgr.db().kbGUID())
                return;

            arrayGUID.push_back((item->document().strGUID));
        }
    }

    if (arrayGUID.empty())
        return;

    CString strGUIDs;
    ::WizStringArrayToText(arrayGUID, strGUIDs, ";");

    QDrag* drag = new QDrag(this);

    QMimeData* mimeData = new QMimeData();
    mimeData->setData(WIZNOTE_MIMEFORMAT_DOCUMENTS, strGUIDs.toUtf8());
    drag->setMimeData(mimeData);

    drag->setPixmap(WizGetDocumentDragBadget(items.size()));
    drag->exec();
}

void CWizDocumentListView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_TAGS)) {
        event->acceptProposedAction();
    }

    //QListWidget::dragEnterEvent(event);
}

void CWizDocumentListView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_TAGS)) {
        event->acceptProposedAction();
    }

    //QListWidget::dragMoveEvent(event);
}

void CWizDocumentListView::dropEvent(QDropEvent * event)
{
    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_TAGS))
    {
        if (CWizDocumentListViewItem* item = dynamic_cast<CWizDocumentListViewItem*>(itemAt(event->pos())))
        {
            QByteArray data = event->mimeData()->data(WIZNOTE_MIMEFORMAT_TAGS);
            QString strTagGUIDs = QString::fromUtf8(data, data.length());
            CWizStdStringArray arrayTagGUID;
            ::WizSplitTextToArray(strTagGUIDs, ';', arrayTagGUID);
            foreach (const CString& strTagGUID, arrayTagGUID)
            {
                WIZTAGDATA dataTag;
                if (m_dbMgr.db().TagFromGUID(strTagGUID, dataTag))
                {
                    CWizDocument doc(m_dbMgr.db(), item->document());
                    doc.AddTag(dataTag);
                }
            }
        }

        event->acceptProposedAction();
    }
}

void CWizDocumentListView::on_itemSelectionChanged()
{
    resetPermission();
}

void CWizDocumentListView::on_tag_created(const WIZTAGDATA& tag)
{
    Q_UNUSED(tag);
}

void CWizDocumentListView::on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew)
{
    Q_UNUSED(tagOld);
    Q_UNUSED(tagNew);

}

void CWizDocumentListView::on_document_created(const WIZDOCUMENTDATA& document)
{
    if (acceptDocument(document))
    {
        if (-1 == documentIndexFromGUID(document.strGUID))
        {
            addDocument(document, true);
        }
    }
}

void CWizDocumentListView::on_document_modified(const WIZDOCUMENTDATA& documentOld,
                                                const WIZDOCUMENTDATA& documentNew)
{
    Q_UNUSED(documentOld);

    // FIXME: if user search on-going, acceptDocument will remove this document from the list.
    if (acceptDocument(documentNew))
    {
        int index = documentIndexFromGUID(documentNew.strGUID);
        if (-1 == index) {
            addDocument(documentNew, true);
        } else {
            if (CWizDocumentListViewItem* pItem = documentItemAt(index)) {
                pItem->reload(m_dbMgr.db(documentNew.strKbGUID));
            }
        }
    } else {
        int index = documentIndexFromGUID(documentNew.strGUID);
        if (-1 != index) {
            takeItem(index);
        }
    }
}

void CWizDocumentListView::on_document_deleted(const WIZDOCUMENTDATA& document)
{
    int index = documentIndexFromGUID(document.strGUID);
    if (-1 != index) {
        takeItem(index);
    }
}

void CWizDocumentListView::on_document_abstractLoaded(const WIZABSTRACT& abs)
{
    int index = documentIndexFromGUID(abs.guid);
    if (-1 == index)
        return;

    // kbGUID also should equal
    CWizDocumentListViewItem* pItem = documentItemAt(index);
    if (!pItem->document().strKbGUID.isEmpty() &&
            pItem->document().strKbGUID != abs.strKbGUID) {
        return;
    }

    if (!pItem->message().kbGUID.isEmpty() &&
            pItem->message().kbGUID != abs.strKbGUID) {
        return;
    }

    pItem->resetAbstract(abs);
    update(indexFromItem(pItem));
}

void CWizDocumentListView::on_userAvatar_downloaded(const QString& strUserGUID)
{
    CWizDocumentListViewItem* pItem = NULL;
    for (int i = 0; i < count(); i++) {
        pItem = documentItemAt(i);

        if (pItem->message().senderGUID == strUserGUID) {
            QString strFileName = m_dbMgr.db().GetAvatarPath() + strUserGUID + ".png";
            pItem->resetAvatar(strFileName);
            update(indexFromItem(pItem));
        }
    }
}

void CWizDocumentListView::on_message_created(const WIZMESSAGEDATA& data)
{

}

void CWizDocumentListView::on_message_modified(const WIZMESSAGEDATA& oldMsg,
                                               const WIZMESSAGEDATA& newMsg)
{
    Q_UNUSED(oldMsg);

    int index = documentIndexFromGUID(newMsg.documentGUID);
    if (-1 != index) {
        if (CWizDocumentListViewItem* pItem = documentItemAt(index)) {
            pItem->reload(m_dbMgr.db());
            update(indexFromItem(pItem));
        }
    }
}

void CWizDocumentListView::on_message_deleted(const WIZMESSAGEDATA& data)
{
    int index = documentIndexFromGUID(data.documentGUID);
    if (-1 != index) {
        takeItem(index);
    }
}

void CWizDocumentListView::on_action_message_mark_read()
{
    QList<QListWidgetItem*> items = selectedItems();

    CWizMessageDataArray arrayMessage;
    foreach (QListWidgetItem* it, items) {
        if (CWizDocumentListViewItem* pItem = dynamic_cast<CWizDocumentListViewItem*>(it)) {
            if (pItem->type() == CWizDocumentListViewItem::MessageDocument) {
                arrayMessage.push_back(pItem->message());
            }
        }
    }

    // 1 means read
    m_dbMgr.db().setMessageReadStatus(arrayMessage, 1);
}

void CWizDocumentListView::on_action_message_delete()
{
    QList<QListWidgetItem*> items = selectedItems();

    foreach (QListWidgetItem* it, items) {
        if (CWizDocumentListViewItem* pItem = dynamic_cast<CWizDocumentListViewItem*>(it)) {
            if (pItem->type() == CWizDocumentListViewItem::MessageDocument) {
                m_dbMgr.db().deleteMessageEx(pItem->message());
            }
        }
    }
}

void CWizDocumentListView::on_action_selectTags()
{
    QList<QListWidgetItem*> items = selectedItems();
    if (items.isEmpty())
        return;

    if (CWizDocumentListViewItem* item = dynamic_cast<CWizDocumentListViewItem*>(items.at(0)))
    {
        Q_UNUSED(item);

        if (!m_tagList)
        {
            m_tagList = new CWizTagListWidget(m_dbMgr, this);
            m_tagList->setLeftAlign(true);
        }

        m_tagList->setDocument(item->document());
        m_tagList->showAtPoint(QCursor::pos());
    }
}

void CWizDocumentListView::on_action_deleteDocument()
{
    QList<QListWidgetItem*> items = selectedItems();

    foreach (QListWidgetItem* it, items) {
        if (CWizDocumentListViewItem* item = dynamic_cast<CWizDocumentListViewItem*>(it)) {
            if (item->type() == CWizDocumentListViewItem::MessageDocument) {
                continue;
            }

            CWizDocument doc(m_dbMgr.db(item->document().strKbGUID), item->document());
            doc.Delete();
        }
    }
}

void CWizDocumentListView::on_action_moveDocument()
{
    CWizFolderSelector* selector = new CWizFolderSelector("Move documents", m_app, this);
    selector->setAcceptRoot(false);

    connect(selector, SIGNAL(finished(int)), SLOT(on_action_moveDocument_confirmed(int)));
    selector->open();
}

void CWizDocumentListView::on_action_moveDocument_confirmed(int result)
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

    // collect documents
    CWizDocumentDataArray arrayDocument;
    QList<QListWidgetItem*> items = selectedItems();
    foreach (QListWidgetItem* it, items) {
        if (CWizDocumentListViewItem* item = dynamic_cast<CWizDocumentListViewItem*>(it)) {
            arrayDocument.push_back(item->document());
        }
    }

    // only move user private documents
    if (isDocumentsWithGroupDocument(arrayDocument)) {
        TOLOG("on_action_moveDocument_confirmed: selected documents with group document!");
        return;
    }

    // move, show progress if size > 3
    CWizFolder folder(m_dbMgr.db(), strSelectedFolder);
    if (arrayDocument.size() <= 3) {
        foreach (const WIZDOCUMENTDATAEX& data, arrayDocument) {
            CWizDocument doc(m_dbMgr.db(), data);
            doc.MoveDocument(&folder);
        }
    } else {
        MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());
        CWizProgressDialog* progress = mainWindow->progressDialog();

        int i = 0;
        foreach (const WIZDOCUMENTDATAEX& data, arrayDocument) {
            CWizDocument doc(m_dbMgr.db(), data);
            doc.MoveDocument(&folder);

            progress->setActionString(tr("Move Document: %1 to %2").arg(data.strLocation).arg(strSelectedFolder));
            progress->setNotifyString(data.strTitle);
            progress->setProgress(arrayDocument.size(), i);
            progress->open();

            i++;
        }

        // hide progress dialog
        mainWindow->progressDialog()->hide();
    }
}

void CWizDocumentListView::on_action_copyDocument()
{
    CWizFolderSelector* selector = new CWizFolderSelector("Copy documents", m_app, this);
    selector->setCopyStyle();
    selector->setAcceptRoot(false);

    connect(selector, SIGNAL(finished(int)), SLOT(on_action_copyDocument_confirmed(int)));
    selector->open();
}

void CWizDocumentListView::on_action_copyDocument_confirmed(int result)
{
    CWizFolderSelector* selector = qobject_cast<CWizFolderSelector*>(sender());
    QString strSelectedFolder = selector->selectedFolder();
    sender()->deleteLater();

    if (strSelectedFolder.isEmpty()) {
        return;
    }

    if (result == QDialog::Accepted) {
        qDebug() << "user select: " << strSelectedFolder;
    }
}

void CWizDocumentListView::on_action_encryptDocument()
{
    QList<QListWidgetItem*> items = selectedItems();

    foreach (QListWidgetItem* it, items) {
        if (CWizDocumentListViewItem* item = dynamic_cast<CWizDocumentListViewItem*>(it)) {
            CWizDocument doc(m_dbMgr.db(), item->document());
            doc.encryptDocument();
        }
    }
}


int CWizDocumentListView::documentIndexFromGUID(const QString& strGUID)
{
    for (int i = 0; i < count(); i++) {
        if (CWizDocumentListViewItem *pItem = documentItemAt(i)) {
            if (!pItem->document().strGUID.isEmpty() && pItem->document().strGUID == strGUID) {
                return i;
            } else if (!pItem->message().documentGUID.isEmpty()
                       && pItem->message().documentGUID == strGUID) {
                return i;
            }
        }
    }

    return -1;
}

CWizDocumentListViewItem *CWizDocumentListView::documentItemAt(int index)
{
    return dynamic_cast<CWizDocumentListViewItem*>(item(index));
}

CWizDocumentListViewItem *CWizDocumentListView::documentItemFromIndex(const QModelIndex &index) const
{
    return dynamic_cast<CWizDocumentListViewItem*>(itemFromIndex(index));
}

const WIZMESSAGEDATA& CWizDocumentListView::messageFromIndex(const QModelIndex& index) const
{
    return documentItemFromIndex(index)->message();
}

const QImage& CWizDocumentListView::messageSenderAvatarFromIndex(const QModelIndex& index) const
{
    return documentItemFromIndex(index)->avatar(m_dbMgr.db(), *m_avatarDownloader);
}

const WIZDOCUMENTDATA& CWizDocumentListView::documentFromIndex(const QModelIndex &index) const
{
    return documentItemFromIndex(index)->document();
}

const WIZABSTRACT& CWizDocumentListView::documentAbstractFromIndex(const QModelIndex &index) const
{
    return documentItemFromIndex(index)->abstract(*m_thumbCache);
}

const QString& CWizDocumentListView::documentTagsFromIndex(const QModelIndex &index) const
{
    return documentItemFromIndex(index)->tags(m_dbMgr.db());
}


//#ifndef Q_OS_MAC
//void CWizDocumentListView::updateGeometries()
//{
//    QListWidget::updateGeometries();
//
//    // singleStep will initialized to item height(94 pixel), reset it
//    verticalScrollBar()->setSingleStep(1);
//}

void CWizDocumentListView::wheelEvent(QWheelEvent* event)
{
    //if (event->orientation() == Qt::Vertical) {
        //vscrollBeginUpdate(event->delta());
        //return;
    //}

//#ifdef Q_OS_MAC
//    QWheelEvent* wEvent = new QWheelEvent(event->pos(),
//                                          event->globalPos(),
//                                          event->delta()/2,
//                                          event->buttons(),
//                                          event->modifiers(),
//                                          event->orientation());
//#endif

    QListWidget::wheelEvent(event);

}

void CWizDocumentListView::vscrollBeginUpdate(int delta)
{
    //if (m_vscrollDelta > 0) {
    //    if (delta > 0 && delta < m_vscrollDelta) {
    //        return;
    //    }
    //}
//
    //if (m_vscrollDelta < 0) {
    //    if (delta > m_vscrollDelta && delta < 0) {
    //        return;
    //    }
    //}

    //m_vscrollDelta = delta;
    //qDebug() << "start:" << m_vscrollDelta;

    if (!m_scrollAnimation)  {
        m_scrollAnimation = new QPropertyAnimation();
        m_scrollAnimation->setDuration(300);
        m_scrollAnimation->setTargetObject(verticalScrollBar());
        m_scrollAnimation->setEasingCurve(QEasingCurve::Linear);
        connect(m_scrollAnimation, SIGNAL(valueChanged(const QVariant&)), SLOT(on_vscrollAnimation_valueChanged(const QVariant&)));
        connect(m_scrollAnimation, SIGNAL(finished()), SLOT(on_vscrollAnimation_finished()));
    }

    if (delta > 0) {
        m_scrollAnimation->setStartValue(0);
        m_scrollAnimation->setEndValue(delta);
    } else {
        m_scrollAnimation->setStartValue(delta);
        m_scrollAnimation->setEndValue(0);
    }

    m_scrollAnimation->start();
}

void CWizDocumentListView::on_vscrollAnimation_valueChanged(const QVariant& value)
{
    QPropertyAnimation* animation = qobject_cast<QPropertyAnimation *>(sender());
    QScrollBar* scrollBar = qobject_cast<QScrollBar *>(animation->targetObject());

    int delta = value.toInt();

    //if (qAbs(delta) > m_vscrollCurrent) {
    //    qDebug() << "change:" << delta;
        scrollBar->setValue(scrollBar->value() - delta/8);
    //    m_vscrollCurrent += qAbs(delta);
    //} else {
    //    animation->stop();
    //}
}

void CWizDocumentListView::on_vscrollAnimation_finished()
{
    QPropertyAnimation* animation = qobject_cast<QPropertyAnimation *>(sender());
    qDebug() << "end:" << animation->startValue() << ":" << animation->endValue();

    //reset
    //m_vscrollDelta = 0;
    //m_vscrollCurrent = 0;
}

void CWizDocumentListView::on_vscroll_update()
{
    // scroll animation stop condition
    //if (qAbs(m_vscrollDelta) > m_vscrollCurrent) {
    //    verticalScrollBar()->setValue(m_vscrollOldPos - m_vscrollDelta/2);
    //    m_vscrollCurrent += qAbs(m_vscrollDelta/2);
    //} else {
    //    m_vscrollTimer.stop();
    //}
}

void CWizDocumentListView::on_vscroll_valueChanged(int value)
{
    m_vscrollOldPos = value;
}

void CWizDocumentListView::on_vscroll_actionTriggered(int action)
{
    switch (action) {
        case QAbstractSlider::SliderSingleStepAdd:
            vscrollBeginUpdate(-120);
            break;
        case QAbstractSlider::SliderSingleStepSub:
            vscrollBeginUpdate(120);
            break;
        default:
            return;
    }
}
//#endif // Q_OS_MAC
