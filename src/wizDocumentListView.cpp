#include "wizDocumentListView.h"

#include <QPixmapCache>
#include <QApplication>
#include <QMenu>
#include <QSet>

#include "share/wizDatabaseManager.h"
#include "wizCategoryView.h"
#include "widgets/wizScrollBar.h"
#include "wiznotestyle.h"
#include "wiztaglistwidget.h"
#include "share/wizsettings.h"
#include "wizFolderSelector.h"
#include "wizProgressDialog.h"
#include "wizmainwindow.h"
#include "utils/stylehelper.h"
#include "utils/logger.h"
#include "utils/pathresolve.h"
#include "sync/apientry.h"
#include "wizWebSettingsDialog.h"
#include "wizPopupButton.h"
#include "wizLineInputDialog.h"
#include "share/wizAnalyzer.h"
#include "share/wizObjectOperator.h"

#include "sync/avatar.h"
#include "thumbcache.h"

using namespace Core;
using namespace Core::Internal;


// Document actions
#define WIZACTION_LIST_DELETE   QObject::tr("Delete")
#define WIZACTION_LIST_TAGS     QObject::tr("Tags...")
#define WIZACTION_LIST_MOVE_DOCUMENT QObject::tr("Move to...")
#define WIZACTION_LIST_COPY_DOCUMENT QObject::tr("Copy to...")
#define WIZACTION_LIST_DOCUMENT_HISTORY QObject::tr("Version History...")
#define WIZACTION_LIST_COPY_DOCUMENT_LINK QObject::tr("Copy Note Link")
#define WIZACTION_LIST_SHARE_DOCUMENT_BY_LINK QObject::tr("Share Link...")
#define WIZACTION_LIST_ENCRYPT_DOCUMENT QObject::tr("Encrypt Note")
#define WIZACTION_LIST_CANCEL_ENCRYPTION  QObject::tr("Cancel Note Encryption")
#define WIZACTION_LIST_ALWAYS_ON_TOP  QObject::tr("Always On Top")
//#define WIZACTION_LIST_CANCEL_ON_TOP  QObject::tr("Cancel always on top")


CWizDocumentListView::CWizDocumentListView(CWizExplorerApp& app, QWidget *parent /*= 0*/)
    : QListWidget(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_tagList(NULL)
    , m_itemSelectionChanged(false)
    , m_accpetAllSearchItems(false)
{
    setFrameStyle(QFrame::NoFrame);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_nViewType = (ViewType)app.userSettings().get("VIEW_TYPE").toInt();
    m_nSortingType = app.userSettings().get("SORT_TYPE").toInt();
    if (qAbs(m_nSortingType) < CWizSortingPopupButton::SortingCreateTime ||
            qAbs(m_nSortingType) > CWizSortingPopupButton::SortingSize)
    {
        m_nSortingType = CWizSortingPopupButton::SortingCreateTime;
    }

    connect(this, SIGNAL(itemSelectionChanged()), SLOT(on_itemSelectionChanged()));

    // scroll bar
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
#ifdef Q_OS_MAC
    verticalScrollBar()->setSingleStep(15);
#else
    verticalScrollBar()->setSingleStep(30);
#endif

#ifdef WIZNOTE_CUSTOM_SCROLLBAR
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_vScroll = new CWizScrollBar(this);
    m_vScroll->syncWith(verticalScrollBar());
#endif

    // setup style
    QString strSkinName = m_app.userSettings().skin();
    setStyle(::WizGetStyle(strSkinName));

    QPalette pal = palette();
    pal.setColor(QPalette::Base, Utils::StyleHelper::listViewBackground());
    setPalette(pal);

    setCursor(QCursor(Qt::ArrowCursor));
    //
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

    connect(&m_dbMgr, SIGNAL(documentReadCountChanged(const WIZDOCUMENTDATA&)),
            SLOT(on_documentReadCount_changed(const WIZDOCUMENTDATA&)));

    // message
    //connect(&m_dbMgr.db(), SIGNAL(messageModified(const WIZMESSAGEDATA&, const WIZMESSAGEDATA&)),
    //        SLOT(on_message_modified(const WIZMESSAGEDATA&, const WIZMESSAGEDATA&)));

    //connect(&m_dbMgr.db(), SIGNAL(messageDeleted(const WIZMESSAGEDATA&)),
    //        SLOT(on_message_deleted(const WIZMESSAGEDATA&)));

    // thumb cache
    //m_thumbCache = new CWizThumbIndexCache(app);
    //connect(m_thumbCache, SIGNAL(loaded(const WIZABSTRACT&)),
    //        SLOT(on_document_abstractLoaded(const WIZABSTRACT&)));

    //QThread *thread = new QThread();
    //m_thumbCache->moveToThread(thread);
    //thread->start();

    connect(ThumbCache::instance(), SIGNAL(loaded(const QString& ,const QString&)),
            SLOT(onThumbCacheLoaded(const QString&, const QString&)));

    connect(WizService::AvatarHost::instance(), SIGNAL(loaded(const QString&)),
            SLOT(on_userAvatar_loaded(const QString&)));

    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);
    viewport()->setAcceptDrops(true);

    // message context menu
    //m_menuMessage = new QMenu(this);
    //m_menuMessage->addAction(WIZACTION_LIST_MESSAGE_MARK_READ, this,
    //                         SLOT(on_action_message_mark_read()));
    //m_menuMessage->addAction(WIZACTION_LIST_MESSAGE_DELETE, this,
    //                         SLOT(on_action_message_delete()));

    // document context menu
    m_menuDocument = new QMenu(this);
    m_menuDocument->addAction(WIZACTION_LIST_TAGS, this,
                              SLOT(on_action_selectTags()));
    m_menuDocument->addSeparator();

    m_menuDocument->addAction(tr("Open in new Window"), this,
                              SLOT(on_action_showDocumentInFloatWindow()));
    m_menuDocument->addAction(WIZACTION_LIST_COPY_DOCUMENT_LINK, this,
                              SLOT(on_action_copyDocumentLink()));
    m_menuDocument->addAction(WIZACTION_LIST_DOCUMENT_HISTORY, this,
                              SLOT(on_action_documentHistory()));

    m_menuDocument->addAction(WIZACTION_LIST_SHARE_DOCUMENT_BY_LINK, this,
                              SLOT(on_action_shareDocumentByLink()));

    m_menuDocument->addSeparator();
    QAction* actionOnTop = m_menuDocument->addAction(WIZACTION_LIST_ALWAYS_ON_TOP,
                                                         this, SLOT(on_action_alwaysOnTop()));
    actionOnTop->setCheckable(true);
    addAction(actionOnTop);

    QAction* actionAddToShortcuts = m_menuDocument->addAction(QObject::tr("Add to Shortcuts"),
                                                              this, SLOT(on_action_addToShortcuts()));
    addAction(actionAddToShortcuts);

    m_menuDocument->addSeparator();

    QAction* actionCopyDoc = m_menuDocument->addAction(WIZACTION_LIST_COPY_DOCUMENT,
                                                       this, SLOT(on_action_copyDocument()), QKeySequence("Ctrl+Shift+C"));
    QAction* actionMoveDoc = m_menuDocument->addAction(WIZACTION_LIST_MOVE_DOCUMENT,
                                                       this, SLOT(on_action_moveDocument()), QKeySequence("Ctrl+Shift+M"));
    m_menuDocument->addAction(WIZACTION_LIST_ENCRYPT_DOCUMENT, this,
                              SLOT(on_action_encryptDocument()));
    m_menuDocument->addAction(WIZACTION_LIST_CANCEL_ENCRYPTION, this,
                              SLOT(on_action_cancelEncryption()));

    m_menuDocument->addSeparator();
    QAction* actionDeleteDoc = m_menuDocument->addAction(WIZACTION_LIST_DELETE,
                                                         this, SLOT(on_action_deleteDocument()), QKeySequence::Delete);
    // not implement, hide currently.
//    actionCopyDoc->setVisible(false);

    // Add to widget's actions list
    addAction(actionMoveDoc);
    addAction(actionDeleteDoc);
    addAction(actionCopyDoc);

    actionDeleteDoc->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionMoveDoc->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionCopyDoc->setShortcutContext(Qt::WidgetWithChildrenShortcut);



    //m_actionEncryptDocument = new QAction(tr("Encrypt Document"), m_menu);
    //connect(m_actionEncryptDocument, SIGNAL(triggered()), SLOT(on_action_encryptDocument()));
    //m_menu->addAction(m_actionEncryptDocument);
    connect(m_menuDocument, SIGNAL(aboutToHide()), SLOT(on_menu_aboutToHide()));
}

CWizDocumentListView::~CWizDocumentListView()
{
}

void CWizDocumentListView::resizeEvent(QResizeEvent* event)
{
#ifdef WIZNOTE_CUSTOM_SCROLLBAR
    // reset scrollbar position
    m_vScroll->resize(m_vScroll->sizeHint().width(), event->size().height());
    m_vScroll->move(event->size().width() - m_vScroll->sizeHint().width(), 0);
#endif

    // FIXME!!!
    //QPixmapCache::clear();
    setItemsNeedUpdate();
    QListWidget::resizeEvent(event);
}

void CWizDocumentListView::setDocuments(const CWizDocumentDataArray& arrayDocument)
{
    //reset
    clear();

    verticalScrollBar()->setValue(0);

    appendDocuments(arrayDocument);
}

void CWizDocumentListView::appendDocuments(const CWizDocumentDataArray& arrayDocument)
{
    CWizDocumentDataArray::const_iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
        addDocument(*it);
    }

    sortItems();

    Q_EMIT documentCountChanged();
}

int CWizDocumentListView::addDocument(const WIZDOCUMENTDATA& doc, bool sort)
{
    addDocument(doc);

    if (sort) {
        sortItems();
    }

    Q_EMIT documentCountChanged();
    int nCount = count();
    return nCount;
}

void CWizDocumentListView::addDocument(const WIZDOCUMENTDATA& doc)
{
    WizDocumentListViewItemData data;
    data.doc = doc;

    if (doc.strKbGUID.isEmpty() || m_dbMgr.db().kbGUID() == doc.strKbGUID) {
        data.nType = CWizDocumentListViewItem::TypePrivateDocument;
    } else {
        data.nType = CWizDocumentListViewItem::TypeGroupDocument;
        data.strAuthorId = doc.strOwner;
    }


    CWizDocumentListViewItem* pItem = new CWizDocumentListViewItem(m_app, data);
    pItem->setSizeHint(QSize(sizeHint().width(), Utils::StyleHelper::listViewItemHeight(m_nViewType)));
    pItem->setSortingType(m_nSortingType);

    addItem(pItem);
}

bool CWizDocumentListView::acceptDocumentChange(const WIZDOCUMENTDATA& document)
{
    //  搜索模式下屏蔽因同步带来的笔记新增和修改
    if (m_accpetAllSearchItems)
    {
        if (documentIndexFromGUID(document.strGUID) == -1)
            return false;
    }

    return true;
}

void CWizDocumentListView::moveDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument, const QString& targetFolder)
{    
    MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());       

    CWizDocumentOperator* documentOperator = new CWizDocumentOperator(m_dbMgr);  
    CWizProgressDialog* progress = mainWindow->progressDialog();
    progress->setWindowTitle(QObject::tr("Move notes to %1").arg(targetFolder));
    documentOperator->bindSignalsToProgressDialog(progress);
    documentOperator->moveDocumentsToPersonalFolder(arrayDocument, targetFolder, mainWindow->downloaderHost());
    progress->exec();
}

void CWizDocumentListView::moveDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument, const WIZTAGDATA& targetTag)
{
    CWizDatabase& db = m_dbMgr.db();
    if (!WizAskUserCipherToOperateEncryptedNote(arrayDocument, db))
        return;

    MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());    

    CWizDocumentOperator* documentOperator = new CWizDocumentOperator(m_dbMgr);
    CWizProgressDialog* progress = mainWindow->progressDialog();
    progress->setWindowTitle(QObject::tr("Move notes to %1").arg(targetTag.strName));
    documentOperator->bindSignalsToProgressDialog(progress);
    documentOperator->moveDocumentsToGroupFolder(arrayDocument, targetTag, mainWindow->downloaderHost());
    progress->exec();

    WizClearUserCipher(db, m_app.userSettings());
}

void CWizDocumentListView::copyDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument,
                                                        const QString& targetFolder, bool keepDocTime, bool keepTag)
{
    CWizDatabase& db = m_dbMgr.db();
    if (!WizAskUserCipherToOperateEncryptedNote(arrayDocument, db))
        return;

    MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());

    CWizDocumentOperator* documentOperator = new CWizDocumentOperator(m_dbMgr);   
    CWizProgressDialog* progress = mainWindow->progressDialog();
    progress->setWindowTitle(QObject::tr("Copy notes to %1").arg(targetFolder));
    documentOperator->bindSignalsToProgressDialog(progress);
    documentOperator->copyDocumentsToPersonalFolder(arrayDocument, targetFolder, keepDocTime,
                                                    keepTag, mainWindow->downloaderHost());
    progress->exec();

    WizClearUserCipher(db, m_app.userSettings());
}

void CWizDocumentListView::copyDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument,
                                                      const WIZTAGDATA& targetTag, bool keepDocTime)
{
    CWizDatabase& db = m_dbMgr.db();
    if (!WizAskUserCipherToOperateEncryptedNote(arrayDocument, db))
        return;

    MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());    

    CWizDocumentOperator* documentOperator = new CWizDocumentOperator(m_dbMgr);
    CWizProgressDialog* progress = mainWindow->progressDialog();
    progress->setWindowTitle(QObject::tr("Copy notes to %1").arg(targetTag.strName));
    documentOperator->bindSignalsToProgressDialog(progress);
    documentOperator->copyDocumentsToGroupFolder(arrayDocument, targetTag, keepDocTime, mainWindow->downloaderHost());
    progress->exec();

    WizClearUserCipher(db, m_app.userSettings());
}

void CWizDocumentListView::duplicateDocuments(const CWizDocumentDataArray& arrayDocument)
{
    for (WIZDOCUMENTDATA doc : arrayDocument)
    {
        CWizDatabase& db = m_dbMgr.db(doc.strKbGUID);
        if (db.IsGroup())
        {
            CWizStdStringArray arrayTagGUID;
            db.GetDocumentTags(doc.strGUID, arrayTagGUID);
            WIZTAGDATA targetTag;
            if (arrayTagGUID.size() > 1)
            {
                qDebug() << "Too may tags found by document : " << doc.strTitle;
                continue;
            }
            else if (arrayTagGUID.size() == 1)
            {
                db.TagFromGUID(*arrayTagGUID.begin(), targetTag);
            }
            targetTag.strKbGUID = doc.strKbGUID;
            //
            CWizDocumentDataArray arrayCopyDoc;
            arrayCopyDoc.push_back(doc);
            copyDocumentsToGroupFolder(arrayCopyDoc, targetTag, true);
        }
        else
        {
            CWizDocumentDataArray arrayCopyDoc;
            arrayCopyDoc.push_back(doc);
            copyDocumentsToPersonalFolder(arrayCopyDoc, doc.strLocation, true, true);
        }
    }
}

bool CWizDocumentListView::acceptDocument(const WIZDOCUMENTDATA& document)
{
    /*
    bool categoryAccpet = m_app.category().acceptDocument(document);
    bool kbGUIDSame = (m_app.category().selectedItemKbGUID() == document.strKbGUID);

    return categoryAccpet; && kbGUIDSame;*/

    // there is no need to check if kbguid is same. especially when category item is  biz root.

    if (m_accpetAllSearchItems)
        return true;

    return m_app.category().acceptDocument(document);
}

void CWizDocumentListView::addAndSelectDocument(const WIZDOCUMENTDATA& document)
{
    if (!acceptDocument(document))
    {
        TOLOG1("[Locate] documentlist can not accpet document %1 ", document.strTitle);
        return;
    }

    int index = documentIndexFromGUID(document.strGUID);
    if (-1 == index) {
        index = addDocument(document, false);
    }

    if (-1 == index)
        return;

    setCurrentItem(item(index), QItemSelectionModel::ClearAndSelect);
    emit documentsSelectionChanged();
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

        arrayDocument.push_back(pDocumentItem->data().doc);
    }
}

/*
void CWizDocumentListView::contextMenuEvent(QContextMenuEvent * e)
{
    CWizDocumentListViewItem* pItem = dynamic_cast<CWizDocumentListViewItem*>(itemAt(e->pos()));

    if (!pItem)
        return;

    //if (pItem->itemType() == CWizDocumentListViewItem::TypeMessage) {
    //    m_menuMessage->popup(e->globalPos());
    //} else {
    m_menuDocument->popup(e->globalPos());
    //}
}
*/

void CWizDocumentListView::resetPermission()
{
    CWizDocumentDataArray arrayDocument;
    //QList<QListWidgetItem*> items = selectedItems();
    foreach (CWizDocumentListViewItem* item, m_rightButtonFocusedItems) {
        arrayDocument.push_back(item->document());
    }

    bool bGroup = isDocumentsWithGroupDocument(arrayDocument);
    bool bDeleted = isDocumentsWithDeleted(arrayDocument);
    bool bCanEdit = isDocumentsAllCanDelete(arrayDocument);
    bool bAlwaysOnTop = isDocumentsAlwaysOnTop(arrayDocument);

    bool bShareEnable = true;
    // if group documents or deleted documents selected
    if (bGroup || bDeleted) {
        findAction(WIZACTION_LIST_TAGS)->setVisible(false);
        findAction(WIZACTION_LIST_SHARE_DOCUMENT_BY_LINK)->setVisible(false);
        bShareEnable = false;
    } else {
        findAction(WIZACTION_LIST_TAGS)->setVisible(true);
        findAction(WIZACTION_LIST_SHARE_DOCUMENT_BY_LINK)->setVisible(bShareEnable);
    }

    // deleted user private documents
    findAction(WIZACTION_LIST_MOVE_DOCUMENT)->setEnabled(bCanEdit);

    // disable delete if permission is not enough
    findAction(WIZACTION_LIST_DELETE)->setEnabled(bCanEdit);

    findAction(WIZACTION_LIST_ALWAYS_ON_TOP)->setCheckable(true);
    findAction(WIZACTION_LIST_ALWAYS_ON_TOP)->setEnabled(bCanEdit);
    findAction(WIZACTION_LIST_ALWAYS_ON_TOP)->setChecked(bAlwaysOnTop);

    // disable note history if selection is not only one
    if (m_rightButtonFocusedItems.count() != 1)
    {
        findAction(WIZACTION_LIST_DOCUMENT_HISTORY)->setEnabled(false);
        //
        int num = numOfEncryptedDocuments(arrayDocument);
        if (num == arrayDocument.size())
        {
            setEncryptDocumentActionEnable(false);
        }
        else
        {
            setEncryptDocumentActionEnable(true);
        }
        bShareEnable = false;
    }
    else
    {
        findAction(WIZACTION_LIST_DOCUMENT_HISTORY)->setEnabled(true);
        WIZDOCUMENTDATA document = (*arrayDocument.begin());
        bool encryptEnable = !document.nProtected;
        setEncryptDocumentActionEnable(encryptEnable);
    }

    findAction(WIZACTION_LIST_SHARE_DOCUMENT_BY_LINK)->setEnabled(bShareEnable);

    if (bGroup)
    {
        findAction(WIZACTION_LIST_SHARE_DOCUMENT_BY_LINK)->setVisible(false);
        findAction(WIZACTION_LIST_ENCRYPT_DOCUMENT)->setVisible(false);
        findAction(WIZACTION_LIST_CANCEL_ENCRYPTION)->setVisible(false);
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

bool CWizDocumentListView::isDocumentsAlwaysOnTop(const CWizDocumentDataArray& arrayDocument)
{
    foreach (const WIZDOCUMENTDATAEX& doc, arrayDocument) {
        if (doc.nFlags & wizDocumentAlwaysOnTop) {
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
        if (!m_dbMgr.db(doc.strKbGUID).CanEditDocument(doc)) {
            return false;
        }
    }

    return true;
}

void CWizDocumentListView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragStartPosition.setX(event->pos().x());
        m_dragStartPosition.setY(event->pos().y());
        QListWidget::mousePressEvent(event);
    }
    else if (event->button() == Qt::RightButton)
    {
        m_rightButtonFocusedItems.clear();
        //
        CWizDocumentListViewItem* pItem = dynamic_cast<CWizDocumentListViewItem*>(itemAt(event->pos()));
        if (!pItem)
            return;

        // if selectdItems contains clicked item use all selectedItems as special focused item.
        if (selectedItems().contains(pItem))
        {
            foreach (QListWidgetItem* lsItem, selectedItems())
            {
                pItem = dynamic_cast<CWizDocumentListViewItem*>(lsItem);
                if (pItem)
                {
                    m_rightButtonFocusedItems.append(pItem);
                    pItem->setSpecialFocused(true);
                }

            }
        }
        else
        {
            m_rightButtonFocusedItems.append(pItem);
            pItem->setSpecialFocused(true);
        }
        //
        resetPermission();
        m_menuDocument->popup(event->globalPos());
    }
}

void CWizDocumentListView::mouseMoveEvent(QMouseEvent* event)
{
    if ((event->buttons() & Qt::LeftButton) && \
            (event->pos() - m_dragStartPosition).manhattanLength() > QApplication::startDragDistance()) {
        setState(QAbstractItemView::DraggingState);
    }

    QListWidget::mouseMoveEvent(event);
}

void CWizDocumentListView::mouseReleaseEvent(QMouseEvent* event)
{
    //
    if (m_itemSelectionChanged)
    {
        emit documentsSelectionChanged();
        m_itemSelectionChanged = false;
    }

    QListWidget::mouseReleaseEvent(event);
}


void CWizDocumentListView::keyReleaseEvent(QKeyEvent* event)
{
    //
    if (m_itemSelectionChanged)
    {
        emit documentsSelectionChanged();
        m_itemSelectionChanged = false;
    }

    QListWidget::keyReleaseEvent(event);
}

QPixmap WizGetDocumentDragBadget(int nCount)
{
    QString strFileName = Utils::PathResolve::resourcesPath() + "skins/document_drag.png";
    QPixmap pixmap(strFileName);

    if (pixmap.isNull()) {
        return QPixmap();
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

QString note2Mime(const CWizDocumentDataArray& arrayDocument)
{
    CWizStdStringArray arrayGUID;

    for (CWizDocumentDataArray::const_iterator it = arrayDocument.begin();
         it != arrayDocument.end();
         it++)
    {
        const WIZDOCUMENTDATA& data = *it;
        arrayGUID.push_back(data.strKbGUID + ":" + data.strGUID);
    }

    CString strMime;
    ::WizStringArrayToText(arrayGUID, strMime, ";");

    return strMime;
}

void CWizDocumentListView::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions);

    CWizDocumentDataArray arrayDocument;
    QList<QListWidgetItem*> items = selectedItems();
    foreach (QListWidgetItem* it, items) {
        if (CWizDocumentListViewItem* item = dynamic_cast<CWizDocumentListViewItem*>(it)) {
//            CWizDatabase& db = CWizDatabaseManager::instance()->db(item->document().strKbGUID);
            arrayDocument.push_back(item->document());
        }
    }

    if (!arrayDocument.size())
        return;

    QString strMime = note2Mime(arrayDocument);

    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData();
    mimeData->setData(WIZNOTE_MIMEFORMAT_DOCUMENTS, strMime.toUtf8());
    drag->setMimeData(mimeData);
    drag->setPixmap(WizGetDocumentDragBadget(items.size()));

//    Qt::KeyboardModifiers keyMod = QApplication::keyboardModifiers();
//    bool forceCopy = keyMod.testFlag(Qt::AltModifier);
//    Qt::DropAction deafult = forceCopy ? Qt::CopyAction : Qt::MoveAction;

    drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::MoveAction);

    //delete drag would cause crash
//    drag->deleteLater();
}

void CWizDocumentListView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_TAGS) ||
            event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS)) {
        event->acceptProposedAction();
        event->accept();
    }
    else
    {
        QListWidget::dragEnterEvent(event);
    }
}

void CWizDocumentListView::dragMoveEvent(QDragMoveEvent *event)
{   
    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_TAGS)||
            event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS)) {
        event->acceptProposedAction();
        event->accept();
    }
    else
    {
        QListWidget::dragMoveEvent(event);
    }
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
    else if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS))
    {
        ::WizGetAnalyzer().LogAction("documentListDuplicateDocument");
        CWizDocumentDataArray arrayDocument;
        ::WizMime2Note(event->mimeData()->data(WIZNOTE_MIMEFORMAT_DOCUMENTS), m_dbMgr, arrayDocument);

        if (!arrayDocument.size())
            return;

        Qt::KeyboardModifiers keyMod = QApplication::keyboardModifiers();
        bool forceCopy = keyMod.testFlag(Qt::AltModifier);

        if (!forceCopy)
            return;

        duplicateDocuments(arrayDocument);
    }    
}

void CWizDocumentListView::resetItemsViewType(int type)
{
    m_nViewType = (ViewType)type;

    for (int i = 0; i < count(); i++) {
        item(i)->setSizeHint(QSize(sizeHint().width(), Utils::StyleHelper::listViewItemHeight(m_nViewType)));
        //item(i)->setSizeHint(itemSizeFromViewType(m_nViewType));
    }
}

QSize CWizDocumentListView::itemSizeFromViewType(ViewType type)
{
    QSize sz = sizeHint();
    switch (type) {
    case CWizDocumentListView::TypeOneLine:
        sz.setHeight(fontMetrics().height() + 12);
        return sz;
    case CWizDocumentListView::TypeTwoLine:
        sz.setHeight(fontMetrics().height() * 2 + 15);
        return sz;
    case CWizDocumentListView::TypeThumbnail:
        //sz.setHeight(fontMetrics().height() * 4 + 30);
        sz.setHeight(Utils::StyleHelper::thumbnailHeight());
        return sz;
    default:
        Q_ASSERT(0);
    }

    return sz;
}

void CWizDocumentListView::resetItemsSortingType(int type)
{
    // FIXME!!!
    //QPixmapCache::clear();
    setItemsNeedUpdate();

    m_nSortingType = type;

    for (int i = 0; i < count(); i++) {
        CWizDocumentListViewItem* pItem = dynamic_cast<CWizDocumentListViewItem*>(item(i));
        pItem->setSortingType(type);
    }

    sortItems();
}

bool CWizDocumentListView::isSortedByAccessDate()
{
    return m_nSortingType == CWizSortingPopupButton::SortingAccessTime ||
            m_nSortingType == -CWizSortingPopupButton::SortingAccessTime;
}

void CWizDocumentListView::on_itemSelectionChanged()
{
    //resetPermission();
    m_itemSelectionChanged = true;

    m_rightButtonFocusedItems.clear();
    foreach (QListWidgetItem* lsItem, selectedItems())
    {
        CWizDocumentListViewItem* pItem = dynamic_cast<CWizDocumentListViewItem*>(lsItem);
        if (pItem)
        {
            m_rightButtonFocusedItems.append(pItem);
        }
    }
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
    if (!acceptDocumentChange(document))
        return;

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

    if (!acceptDocumentChange(documentNew))
        return;

    // FIXME: if user search on-going, acceptDocument will remove this document from the list.
    if (acceptDocument(documentNew))
    {
        int index = documentIndexFromGUID(documentNew.strGUID);
        if (-1 == index) {
            addDocument(documentNew, true);
        } else {
            if (CWizDocumentListViewItem* pItem = documentItemAt(index)) {
                pItem->reload(m_dbMgr.db(documentNew.strKbGUID));
                pItem->setSortingType(m_nSortingType);
                update(indexFromItem(pItem));
                sortItems();
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

void CWizDocumentListView::on_documentReadCount_changed(const WIZDOCUMENTDATA& document)
{
    if (acceptDocument(document))
    {
        int index = documentIndexFromGUID(document.strGUID);
        if (CWizDocumentListViewItem* pItem = documentItemAt(index))
        {
            pItem->reload(m_dbMgr.db(document.strKbGUID));
            update(indexFromItem(pItem));
        }
    }
}

void CWizDocumentListView::on_document_abstractLoaded(const WIZABSTRACT& abs)
{
    int index = documentIndexFromGUID(abs.guid);
    if (-1 == index)
        return;

    // kbGUID also should equal
    CWizDocumentListViewItem* pItem = documentItemAt(index);
    //if (!pItem->document().strKbGUID.isEmpty() &&
    //        pItem->document().strKbGUID != abs.strKbGUID) {
    //    return;
    //}

    //if (!pItem->message().kbGUID.isEmpty() &&
    //        pItem->message().kbGUID != abs.strKbGUID) {
    //    return;
    //}

    pItem->resetAbstract(abs);
    update(indexFromItem(pItem));
}

void CWizDocumentListView::on_userAvatar_loaded(const QString& strUserGUID)
{
    CWizDocumentListViewItem* pItem = NULL;
    for (int i = 0; i < count(); i++) {
        pItem = documentItemAt(i);
        if (pItem->data().strAuthorId == strUserGUID) {
            update(indexFromItem(pItem));
        }
    }
}

void CWizDocumentListView::onThumbCacheLoaded(const QString& strKbGUID, const QString& strGUID)
{
    setItemsNeedUpdate(strKbGUID, strGUID);

    CWizDocumentListViewItem* pItem = NULL;
    for (int i = 0; i < count(); i++) {
        pItem = documentItemAt(i);
        if (pItem->data().doc.strKbGUID == strKbGUID && pItem->data().doc.strGUID == strGUID) {
            update(indexFromItem(pItem));
        }
    }
}

void CWizDocumentListView::on_action_documentHistory()
{
    ::WizGetAnalyzer().LogAction("documentListMenuHistory");
    if (m_rightButtonFocusedItems.count() != 1)
        return;

   CWizDocumentListViewItem* item = m_rightButtonFocusedItems.first();
   if (!item)
       return;

   const WIZDOCUMENTDATA& doc = item->document();
   WizShowDocumentHistory(doc, window());
}

void CWizDocumentListView::on_action_shareDocumentByLink()
{
    ::WizGetAnalyzer().LogAction("documentListMenuShareByLink");
    if (m_rightButtonFocusedItems.count() != 1)
        return;

   CWizDocumentListViewItem* item = m_rightButtonFocusedItems.first();
   if (!item)
       return;

   const WIZDOCUMENTDATA& doc = item->document();

   emit shareDocumentByLinkRequest(doc.strKbGUID, doc.strGUID);
}

//void CWizDocumentListView::on_message_created(const WIZMESSAGEDATA& data)
//{
//
//}
//
//void CWizDocumentListView::on_message_modified(const WIZMESSAGEDATA& oldMsg,
//                                               const WIZMESSAGEDATA& newMsg)
//{
//    Q_UNUSED(oldMsg);
//
//    int index = documentIndexFromGUID(newMsg.documentGUID);
//    if (-1 != index) {
//        if (CWizDocumentListViewItem* pItem = documentItemAt(index)) {
//            pItem->reload(m_dbMgr.db());
//            update(indexFromItem(pItem));
//        }
//    }
//}
//
//void CWizDocumentListView::on_message_deleted(const WIZMESSAGEDATA& data)
//{
//    int index = documentIndexFromGUID(data.documentGUID);
//    if (-1 != index) {
//        takeItem(index);
//    }
//}

//void CWizDocumentListView::on_action_message_mark_read()
//{
//    QList<QListWidgetItem*> items = selectedItems();
//
//    CWizMessageDataArray arrayMessage;
//    foreach (QListWidgetItem* it, items) {
//        if (CWizDocumentListViewItem* pItem = dynamic_cast<CWizDocumentListViewItem*>(it)) {
//            if (pItem->itemType() == CWizDocumentListViewItem::TypeMessage) {
//                WIZMESSAGEDATA msg;
//                m_dbMgr.db().messageFromId(pItem->data().nMessageId, msg);
//                arrayMessage.push_back(msg);
//            }
//        }
//    }
//
//    // 1 means read
//    m_dbMgr.db().setMessageReadStatus(arrayMessage, 1);
//}
//
//void CWizDocumentListView::on_action_message_delete()
//{
//    QList<QListWidgetItem*> items = selectedItems();
//
//    foreach (QListWidgetItem* it, items) {
//        if (CWizDocumentListViewItem* pItem = dynamic_cast<CWizDocumentListViewItem*>(it)) {
//            if (pItem->type() == CWizDocumentListViewItem::TypeMessage) {
//                WIZMESSAGEDATA msg;
//                m_dbMgr.db().messageFromId(pItem->data().nMessageId, msg);
//                m_dbMgr.db().deleteMessageEx(msg);
//            }
//        }
//    }
//}

void CWizDocumentListView::on_action_selectTags()
{
    ::WizGetAnalyzer().LogAction("documentListMenuSelectTags");
    if (!m_tagList) {
        m_tagList = new CWizTagListWidget(this);
    }

    if (m_rightButtonFocusedItems.isEmpty())
        return;

    CWizDocumentDataArray arrayDocument;
    for (int i = 0; i < m_rightButtonFocusedItems.size(); i++) {
        CWizDocumentListViewItem* pItem = m_rightButtonFocusedItems.at(i);
        arrayDocument.push_back(pItem->document());
    }

    m_tagList->setDocuments(arrayDocument);
    m_tagList->showAtPoint(QCursor::pos());
}

void CWizDocumentListView::on_action_deleteDocument()
{
    ::WizGetAnalyzer().LogAction("documentListMenuDeleteDocument");
    if (m_rightButtonFocusedItems.isEmpty())
        return;
    //
    //
    blockSignals(true);
    int index = -1;
    QSet<QString> setKb;
    foreach (CWizDocumentListViewItem* item, m_rightButtonFocusedItems) {
        if (item->type() == CWizDocumentListViewItem::TypeMessage) {
            continue;
        }

        index = indexFromItem(item).row();
        CWizDocument doc(m_dbMgr.db(item->document().strKbGUID), item->document());
        setKb.insert(item->document().strKbGUID);
        doc.Delete();
    }
    blockSignals(false);

    for (QString strKb : setKb)
    {
        emit changeUploadRequest(strKb);
    }

    //change to next document
    int nItemCount = count();
    if(index >= nItemCount)
    {
        index = nItemCount - 1;
    }

    if (count() == 0)
    {
        emit lastDocumentDeleted();
        return;
    }
    else if (selectedItems().isEmpty())
    {
        setItemSelected(documentItemAt(index), true);
    }
    emit documentsSelectionChanged();
}

void CWizDocumentListView::on_action_moveDocument()
{
    ::WizGetAnalyzer().LogAction("documentListMenuMoveDocument");
    CWizFolderSelector* selector = new CWizFolderSelector(tr("Move notes"), m_app, WIZ_USERGROUP_AUTHOR, this);
    selector->setAcceptRoot(false);

    connect(selector, SIGNAL(finished(int)), SLOT(on_action_moveDocument_confirmed(int)));
    selector->exec();
}

void CWizDocumentListView::on_action_moveDocument_confirmed(int result)
{
    qDebug() << "move document confirmed : " << result;
    CWizFolderSelector* selector = qobject_cast<CWizFolderSelector*>(sender());
    WizScopeGuard guard([&]{
       selector->deleteLater();
    });

    if (result != QDialog::Accepted) {
        return;
    }

    QSet<QString> dbSet;
    // collect documents
    CWizDocumentDataArray arrayDocument;
    foreach (CWizDocumentListViewItem* item, m_rightButtonFocusedItems) {
        arrayDocument.push_back(item->document());
        dbSet.insert(item->document().strKbGUID);
    }

    if (selector->isSelectPersonalFolder())
    {
        QString strSelectedFolder = selector->selectedFolder();
        if (strSelectedFolder.isEmpty())
            return;
        qDebug() << "move docuemnt to private folder " << strSelectedFolder;
        moveDocumentsToPersonalFolder(arrayDocument, strSelectedFolder);
        dbSet.insert(m_dbMgr.db().kbGUID());
    }
    else if (selector->isSelectGroupFolder())
    {
        WIZTAGDATA tag = selector->selectedGroupFolder();
        if (tag.strKbGUID.isEmpty())
            return;
        qDebug() << "move docuemnt to group folder " << tag.strName;
        //
        moveDocumentsToGroupFolder(arrayDocument, tag);
        dbSet.insert(tag.strKbGUID);
    }

    MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());
    for (QString kbGuid : dbSet)
    {
        mainWindow->quickSyncKb(kbGuid);
    }
}

void CWizDocumentListView::on_action_copyDocument()
{
    ::WizGetAnalyzer().LogAction("documentListMenuCopyDocument");
    Q_ASSERT(!m_rightButtonFocusedItems.isEmpty());
    //
    CWizFolderSelector* selector = new CWizFolderSelector(tr("Copy documents"), m_app, WIZ_USERGROUP_AUTHOR, this);
    bool isGroup = m_dbMgr.db(m_rightButtonFocusedItems.first()->document().strKbGUID).IsGroup();
    selector->setCopyStyle(!isGroup);
    selector->setAcceptRoot(false);

    connect(selector, SIGNAL(finished(int)), SLOT(on_action_copyDocument_confirmed(int)));
    selector->exec();
}

void CWizDocumentListView::on_action_copyDocument_confirmed(int result)
{
    qDebug() << "copy document confirmed : " << result;
    CWizFolderSelector* selector = qobject_cast<CWizFolderSelector*>(sender());
    WizScopeGuard guard([&]{
       selector->deleteLater();
    });

    if (result != QDialog::Accepted) {
        return;
    }

    QSet<QString> dbSet;
    // collect documents
    CWizDocumentDataArray arrayDocument;
    foreach (CWizDocumentListViewItem* item, m_rightButtonFocusedItems) {
        arrayDocument.push_back(item->document());
        dbSet.insert(item->document().strKbGUID);
    }

    if (selector->isSelectPersonalFolder())
    {
        QString strSelectedFolder = selector->selectedFolder();
        if (strSelectedFolder.isEmpty()) {
            return;
        }
        qDebug() << "copy docuemnt to private folder " << strSelectedFolder;
        copyDocumentsToPersonalFolder(arrayDocument, strSelectedFolder, selector->isKeepTime(), selector->isKeepTag());
        dbSet.insert(m_dbMgr.db().kbGUID());
    }
    else if (selector->isSelectGroupFolder())
    {
        WIZTAGDATA tag = selector->selectedGroupFolder();
        if (tag.strKbGUID.isEmpty())
            return;
        qDebug() << "copy docuemnt to group folder " << tag.strName;
        //
        copyDocumentsToGroupFolder(arrayDocument, tag, selector->isKeepTime());
        dbSet.insert(tag.strKbGUID);
    }

    MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());
    for (QString kbGuid : dbSet)
    {
        mainWindow->quickSyncKb(kbGuid);
    }
}

void CWizDocumentListView::on_action_copyDocumentLink()
{
    ::WizGetAnalyzer().LogAction("documentListMenuCopyLink");
    if (m_rightButtonFocusedItems.isEmpty())
        return;
    //
    QList<WIZDOCUMENTDATA> documents;
    foreach(CWizDocumentListViewItem* item, m_rightButtonFocusedItems)
    {
        const WIZDOCUMENTDATA& document = item->document();
        documents.append(document);
    }
    m_dbMgr.db().CopyDocumentsLink(documents);
}

void CWizDocumentListView::on_action_showDocumentInFloatWindow()
{
    ::WizGetAnalyzer().LogAction("documentListMenuOpenInFloatWindow");
    MainWindow* mainWindow = qobject_cast<MainWindow*>(m_app.mainWindow());
    foreach(CWizDocumentListViewItem* item, m_rightButtonFocusedItems)
    {
        const WIZDOCUMENTDATA& document = item->document();
        mainWindow->viewNoteInSeparateWindow(document);
    }
}

void CWizDocumentListView::on_menu_aboutToHide()
{
    foreach(CWizDocumentListViewItem* item, m_rightButtonFocusedItems)
    {
        item->setSpecialFocused(false);
    }
    viewport()->update();
}

void CWizDocumentListView::on_action_encryptDocument()
{
    ::WizGetAnalyzer().LogAction("documentListMenuEncryptDocument");
    foreach(CWizDocumentListViewItem* item, m_rightButtonFocusedItems)
    {
        WIZDOCUMENTDATA doc = item->document();
        CWizDatabase& db = m_dbMgr.db(doc.strKbGUID);
        db.EncryptDocument(doc);
    }
}

void CWizDocumentListView::on_action_cancelEncryption()
{
    ::WizGetAnalyzer().LogAction("documentListMenuCancelEncryptionn");
    QString strUserCipher;
    CWizLineInputDialog dlg(tr("Password"), tr("Please input document password to cancel encrypt."),
                            "", 0, QLineEdit::Password);
    if (dlg.exec() == QDialog::Rejected)
        return;

    strUserCipher = dlg.input();
    if (strUserCipher.isEmpty())
        return;

    //
    foreach(CWizDocumentListViewItem* item, m_rightButtonFocusedItems)
    {
        WIZDOCUMENTDATA doc = item->document();
        if (doc.nProtected)
        {
            CWizDatabase& db = m_dbMgr.db(doc.strKbGUID);
            if (!db.CancelDocumentEncryption(doc, strUserCipher))
                return;
        }
    }
}

void CWizDocumentListView::on_action_alwaysOnTop()
{
    ::WizGetAnalyzer().LogAction("documentListMenuAlwaysOnTop");
    QAction *actionAlwaysOnTop = findAction(WIZACTION_LIST_ALWAYS_ON_TOP);
    actionAlwaysOnTop->setChecked(actionAlwaysOnTop->isChecked());
    bool bAlwaysOnTop = actionAlwaysOnTop->isChecked();

    foreach(CWizDocumentListViewItem* item, m_rightButtonFocusedItems)
    {
        CWizDatabase& db = m_dbMgr.db(item->document().strKbGUID);
        WIZDOCUMENTDATA doc;
        db.DocumentFromGUID(item->document().strGUID, doc);
        if (bAlwaysOnTop)
        {
            doc.nFlags |= wizDocumentAlwaysOnTop;
        }
        else
        {
            doc.nFlags &= ~wizDocumentAlwaysOnTop;
        }
        db.SetDocumentFlags(doc, QString::number(doc.nFlags), true);
        item->reload(db);
    }

    sortItems();
}

void CWizDocumentListView::on_action_addToShortcuts()
{
    ::WizGetAnalyzer().LogAction("documentListMenuAddToShortcuts");
    foreach(CWizDocumentListViewItem* item, m_rightButtonFocusedItems)
    {
        CWizDatabase& db = m_dbMgr.db(item->document().strKbGUID);
        WIZDOCUMENTDATA doc;
        if (db.DocumentFromGUID(item->document().strGUID, doc))
        {
            emit addDocumentToShortcutsRequest(doc);
        }
    }
}


int CWizDocumentListView::documentIndexFromGUID(const QString& strGUID)
{
    Q_ASSERT(!strGUID.isEmpty());

    for (int i = 0; i < count(); i++) {
        if (CWizDocumentListViewItem *pItem = documentItemAt(i)) {
            if (pItem->document().strGUID == strGUID) {
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

const WizDocumentListViewItemData& CWizDocumentListView::documentItemDataFromIndex(const QModelIndex& index) const
{
    return documentItemFromIndex(index)->data();
}

const WIZDOCUMENTDATA& CWizDocumentListView::documentFromIndex(const QModelIndex &index) const
{
    return documentItemFromIndex(index)->document();
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

    int delta = event->delta();
    switch (m_nViewType)
    {
    case TypeThumbnail:
        delta = delta / 3;
        break;
    case TypeTwoLine:
        delta = int(delta / 1.5);
        break;
    default:
        break;
    }
    QWheelEvent* newEvent = new QWheelEvent(event->pos(),
                                          event->globalPos(),
                                          delta,
                                          event->buttons(),
                                          event->modifiers(),
                                          event->orientation());
    QListWidget::wheelEvent(newEvent);
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

int CWizDocumentListView::numOfEncryptedDocuments(const CWizDocumentDataArray& docArray)
{
    int sum = 0;
    CWizDocumentDataArray::const_iterator it;
    for (it = docArray.begin(); it != docArray.end(); it++)
    {
        WIZDOCUMENTDATA document = *it;
        if (document.nProtected)
        {
            sum++;
        }
    }

    return sum;
}

void CWizDocumentListView::setEncryptDocumentActionEnable(bool enable)
{
    findAction(WIZACTION_LIST_ENCRYPT_DOCUMENT)->setVisible(enable);
    findAction(WIZACTION_LIST_CANCEL_ENCRYPTION)->setVisible(!enable);
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

void CWizDocumentListView::drawItem(QPainter* p, const QStyleOptionViewItemV4* vopt) const
{
    CWizDocumentListViewItem* pItem = documentItemFromIndex(vopt->index);
    if (pItem)
        pItem->draw(p, vopt, viewType());
}

void CWizDocumentListView::reloadItem(const QString& strKbGUID, const QString& strGUID)
{
    int index = documentIndexFromGUID(strGUID);
    if (-1 == index)
        return;

    CWizDocumentListViewItem* pItem = documentItemAt(index);
    if (pItem)
    {
        pItem->reload(m_dbMgr.db(strKbGUID));
        //m_dbMgr.db(strKbGUID).UpdateDocumentAbstract(strGUID);
        update(indexFromItem(pItem));
    }
}

void CWizDocumentListView::setAcceptAllSearchItems(bool bAccept)
{
    m_accpetAllSearchItems = bAccept;
}

void CWizDocumentListView::setItemsNeedUpdate(const QString& strKbGUID, const QString& strGUID)
{
    if (strKbGUID.isEmpty() || strGUID.isEmpty()) {
        for (int i = 0; i < count(); i++) {
            CWizDocumentListViewItem* pItem = dynamic_cast<CWizDocumentListViewItem*>(item(i));
            Q_ASSERT(pItem);

            pItem->setNeedUpdate();
        }

        return;
    }

    CWizDocumentListViewItem* pItem = NULL;
    for (int i = 0; i < count(); i++) {
        pItem = documentItemAt(i);
        if (pItem->data().doc.strKbGUID == strKbGUID && pItem->data().doc.strGUID == strGUID) {
            pItem->setNeedUpdate();
        }
    }
}
