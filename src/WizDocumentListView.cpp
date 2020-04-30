#include "WizDocumentListView.h"

#include <QPixmapCache>
#include <QApplication>
#include <QMenu>
#include <QSet>

#include "utils/WizStyleHelper.h"
#include "utils/WizLogger.h"
#include "utils/WizPathResolve.h"
#include "share/WizDatabaseManager.h"
#include "share/WizSettings.h"
#include "share/WizAnalyzer.h"
#include "share/WizObjectOperator.h"
#include "share/WizThreads.h"
#include "sync/WizApiEntry.h"
#include "sync/WizAvatarHost.h"
#include "widgets/WizScrollBar.h"
#include "WizLineInputDialog.h"
#include "WizWebSettingsDialog.h"
#include "WizPopupButton.h"
#include "WizFolderSelector.h"
#include "WizProgressDialog.h"
#include "WizMainWindow.h"
#include "WizNoteStyle.h"
#include "WizTagListWidget.h"
#include "WizCategoryView.h"
#include "WizCombineNotesDialog.h"

#include "sync/WizKMSync.h"

#include "WizThumbCache.h"


// Document actions
#define WIZACTION_LIST_LOCATE   QObject::tr("Locate")
#define WIZACTION_LIST_DELETE   QObject::tr("Delete")
#define WIZACTION_LIST_TAGS     QObject::tr("Tags...")
#define WIZACTION_LIST_COMBINE     QObject::tr("Combine notes...")
#define WIZACTION_LIST_MOVE_DOCUMENT QObject::tr("Move to...")
#define WIZACTION_LIST_COPY_DOCUMENT QObject::tr("Copy to...")
#define WIZACTION_LIST_DOCUMENT_HISTORY QObject::tr("Version History...")
#define WIZACTION_LIST_COPY_DOCUMENT_LINK QObject::tr("Copy Internal Note Link")
#define WIZACTION_LIST_COPY_WEB_GROUP_LINK  QObject::tr("Copy Web Client Link")
#define WIZACTION_LIST_SHARE_DOCUMENT_BY_LINK QObject::tr("Share Link...")
#define WIZACTION_LIST_ENCRYPT_DOCUMENT QObject::tr("Encrypt Note")
#define WIZACTION_LIST_CANCEL_ENCRYPTION  QObject::tr("Cancel Note Encryption")
#define WIZACTION_LIST_ALWAYS_ON_TOP  QObject::tr("Always On Top")
//#define WIZACTION_LIST_CANCEL_ON_TOP  QObject::tr("Cancel always on top")


enum DocSize {
    _0KB = 0,
    _5KB = 5 * 1024,
    _10KB = 10 * 1024,
    _30KB = 30 * 1024,
    _60KB = 60 * 1024,
    _100KB = 100 * 1024,
    _200KB = 200 * 1024,
    _300KB = 300 * 1024,
    _500KB = 500 * 1024,
    _1MB = 1 * 1024 * 1024,
    _10MB = 10 * 1024 * 1024,
    _30MB = 30 * 1024 * 1024,
    _100MB = 100 * 1024 * 1024,
    _1GB = 1 * 1024 * 1024 * 1024
};

WizDocumentListView::WizDocumentListView(WizExplorerApp& app, QWidget *parent /*= 0*/)
    : QListWidget(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_tagList(NULL)
    , m_itemSelectionChanged(false)
    , m_accpetAllSearchItems(false)
    , m_nLeadInfoState(DocumentLeadInfo_None)
    , m_nAddedDocumentCount(0)
    , m_bSortDocumentsAfterAdded(false)
    , m_searchResult(false)
{
    setFrameStyle(QFrame::NoFrame);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setCursor(Qt::ArrowCursor);

    m_nViewType = (ViewType)app.userSettings().get("VIEW_TYPE").toInt();
    m_nSortingType = app.userSettings().get("SORT_TYPE").toInt();
    if (qAbs(m_nSortingType) < SortingByCreatedTime ||
            qAbs(m_nSortingType) > SortingBySize)
    {
        m_nSortingType = SortingByCreatedTime;
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
    m_vScroll = new WizScrollBar(this);
    m_vScroll->syncWith(verticalScrollBar());
    if (isDarkMode()) {
        //m_vScroll->applyStyle("#272727", QColor(Qt::transparent).name(), false);
    } else {
        //m_vScroll->applyStyle("#F5F5F5", QColor(Qt::transparent).name(), false);
    }
#endif

    // setup style
    QString strSkinName = m_app.userSettings().skin();
    setStyle(::WizGetStyle(strSkinName));

    QPalette pal = palette();
    pal.setColor(QPalette::Base, Utils::WizStyleHelper::listViewBackground());
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

    connect(&m_dbMgr, SIGNAL(documentParamModified(const WIZDOCUMENTPARAMDATA&)),
            SLOT(on_document_param_modified(const WIZDOCUMENTPARAMDATA&)));

    connect(&m_dbMgr, SIGNAL(documentReadCountChanged(const WIZDOCUMENTDATA&)),
            SLOT(on_documentReadCount_changed(const WIZDOCUMENTDATA&)));
    connect(&m_dbMgr, SIGNAL(documentAccessDateModified(WIZDOCUMENTDATA)),
            SLOT(on_documentAccessDate_changed(WIZDOCUMENTDATA)));
    //
    connect(&m_dbMgr, SIGNAL(documentUploaded(const QString&, const QString&)),
            SLOT(on_documentUploaded(const QString&, const QString&)));


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

    connect(WizThumbCache::instance(), SIGNAL(loaded(const QString& ,const QString&)),
            SLOT(onThumbCacheLoaded(const QString&, const QString&)));

    connect(WizAvatarHost::instance(), SIGNAL(loaded(const QString&)),
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
    m_menuDocument->addAction(WIZACTION_LIST_LOCATE, this,
                              SLOT(on_action_locate()));

    m_menuDocument->addAction(WIZACTION_LIST_TAGS, this,
                              SLOT(on_action_selectTags()));
    m_menuDocument->addSeparator();
    //
    m_menuDocument->addAction(tr("Open in new Window"), this,
                              SLOT(on_action_showDocumentInFloatWindow()));
    m_menuDocument->addAction(WIZACTION_LIST_COPY_DOCUMENT_LINK, this,
                              SLOT(on_action_copyDocumentLink()));
    m_menuDocument->addAction(WIZACTION_LIST_DOCUMENT_HISTORY, this,
                              SLOT(on_action_documentHistory()));

    m_menuDocument->addAction(WIZACTION_LIST_SHARE_DOCUMENT_BY_LINK, this,
                              SLOT(on_action_shareDocumentByLink()));
    m_menuDocument->addAction(WIZACTION_LIST_COPY_WEB_GROUP_LINK, this,
                              SLOT(on_action_copyWebClientLink()));

    m_menuDocument->addSeparator();

    QAction* actionAlwaysOnTop = m_menuDocument->addAction(WIZACTION_LIST_ALWAYS_ON_TOP,
                                                         this, SLOT(on_action_alwaysOnTop()));
    actionAlwaysOnTop->setCheckable(true);
    //
    m_menuDocument->addAction(QObject::tr("Add to Shortcuts"),
                                                              this, SLOT(on_action_addToShortcuts()));

    m_menuDocument->addSeparator();

    QAction* actionCopyDoc = m_menuDocument->addAction(WIZACTION_LIST_COPY_DOCUMENT,
                                                       this, SLOT(on_action_copyDocument()), QKeySequence("Ctrl+Shift+C"));
    QAction* actionMoveDoc = m_menuDocument->addAction(WIZACTION_LIST_MOVE_DOCUMENT,
                                                       this, SLOT(on_action_moveDocument()), QKeySequence("Ctrl+Shift+M"));
    m_menuDocument->addAction(WIZACTION_LIST_ENCRYPT_DOCUMENT, this,
                              SLOT(on_action_encryptDocument()));
    m_menuDocument->addAction(WIZACTION_LIST_CANCEL_ENCRYPTION, this,
                              SLOT(on_action_cancelEncryption()));
    //
    m_menuDocument->addAction(WIZACTION_LIST_COMBINE, this,
                              SLOT(on_action_combineNote()));

    m_menuDocument->addSeparator();
    QAction* actionDeleteDoc = m_menuDocument->addAction(WIZACTION_LIST_DELETE,
                                                         this, SLOT(on_action_deleteDocument()),
                                                     #ifdef Q_OS_OSX
                                                         QKeySequence()
                                                     #else
                                                         QKeySequence::Delete
                                                     #endif
                                                         );
    // not implement, hide currently.
//    actionCopyDoc->setVisible(false);



    //m_actionEncryptDocument = new QAction(tr("Encrypt Document"), m_menu);
    //connect(m_actionEncryptDocument, SIGNAL(triggered()), SLOT(on_action_encryptDocument()));
    //m_menu->addAction(m_actionEncryptDocument);
    connect(m_menuDocument, SIGNAL(aboutToHide()), SLOT(on_menu_aboutToHide()));

#ifdef Q_OS_MAC
    // add action to widget to bind shortcut
    actionDeleteDoc = new QAction(this);
    actionDeleteDoc->setShortcut(QKeySequence::Delete);
    actionDeleteDoc->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(actionDeleteDoc, SIGNAL(triggered(bool)), SLOT(on_action_deleteDocument()));

    actionCopyDoc = new QAction(this);
    actionCopyDoc->setShortcut(QKeySequence("Ctrl+Shift+C"));
    actionCopyDoc->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(actionCopyDoc, SIGNAL(triggered(bool)), SLOT(on_action_copyDocument()));


    actionMoveDoc = new QAction(this);
    actionMoveDoc->setShortcut(QKeySequence("Ctrl+Shift+M"));
    actionMoveDoc->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(actionMoveDoc, SIGNAL(triggered(bool)), SLOT(on_action_moveDocument()));

    addAction(actionDeleteDoc);
    addAction(actionCopyDoc);
    addAction(actionMoveDoc);
#endif
}

WizDocumentListView::~WizDocumentListView()
{
    disconnect();
}

void WizDocumentListView::resizeEvent(QResizeEvent* event)
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

void WizDocumentListView::setDocuments(const CWizDocumentDataArray& arrayDocument, bool searchResult /*= false*/)
{
    //reset
    clearAllItems();
    //
    m_searchResult = searchResult;

    verticalScrollBar()->setValue(0);

    if (searchResult)
    {
        appendDocumentsNoSort(arrayDocument);
    }
    else
    {
        appendDocuments(arrayDocument);
    }
}

void WizDocumentListView::appendDocumentsNoSort(const CWizDocumentDataArray& arrayDocument)
{
    CWizDocumentDataArray::const_iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
        addDocument(*it);
    }

    Q_EMIT documentCountChanged();
}

void WizDocumentListView::appendDocuments(const CWizDocumentDataArray& arrayDocument)
{
    CWizDocumentDataArray::const_iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
        addDocument(*it);
    }

    updateSectionItems();

    sortItems();

    Q_EMIT documentCountChanged();
}

int WizDocumentListView::addDocument(const WIZDOCUMENTDATAEX& doc, bool sort)
{
    int oldPosition = 0;
    if (QScrollBar* scrollBar = verticalScrollBar())
    {
        oldPosition = scrollBar->value();
    }
    //
    addDocument(doc);
#ifdef QT_DEBUG
    qDebug() << "add document: " << doc.strTitle;
#endif
    //
    if (sort)
    {
        m_bSortDocumentsAfterAdded = true;
    }
    //
    if (0 == m_nAddedDocumentCount)
    {
        m_nAddedDocumentCount++;
        //
        ::WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{
            //
#ifdef QT_DEBUG
            qDebug() << "sort documents ";
#endif
            //
            m_nAddedDocumentCount = 0;

            updateSectionItems();

            if (m_bSortDocumentsAfterAdded) {
                sortItems();
                m_bSortDocumentsAfterAdded = false;
                //
                if (QScrollBar* scrollBar = verticalScrollBar())
                {
                    scrollBar->setValue(oldPosition);
                }
                //
                QList<QListWidgetItem*> ls = selectedItems();
                if (!ls.empty())
                {
                    QListWidgetItem* item = ls[0];
                    scrollToItem(item, EnsureVisible);
                }
            }

            Q_EMIT documentCountChanged();
            //
        });

    }
    //
    int nCount = count();
    return nCount;
}

void WizDocumentListView::addDocument(const WIZDOCUMENTDATAEX& doc)
{
    WizDocumentListViewItemData data;
    data.doc = doc;

    if (doc.strKbGUID.isEmpty() || m_dbMgr.db().kbGUID() == doc.strKbGUID) {
        data.nType = WizDocumentListViewDocumentItem::TypePrivateDocument;
    } else {
        data.nType = WizDocumentListViewDocumentItem::TypeGroupDocument;
        data.strAuthorId = doc.strOwner;
    }


    WizDocumentListViewDocumentItem* pItem = new WizDocumentListViewDocumentItem(m_app, data);
    pItem->setSizeHint(QSize(sizeHint().width(), Utils::WizStyleHelper::listViewItemHeight(viewType())));
    pItem->setLeadInfoState(m_nLeadInfoState);
    pItem->setSortingType(m_nSortingType);

    addItem(pItem);
}

bool WizDocumentListView::acceptDocumentChange(const WIZDOCUMENTDATA& document)
{
    //  搜索模式下屏蔽因同步带来的笔记新增和修改
    if (m_accpetAllSearchItems)
    {
        if (documentIndexFromGUID(document.strGUID) == -1)
            return false;
    }

    return true;
}
bool WizDocumentListView::acceptDocumentChange(const QString &documentGuid)
{
    //  搜索模式下屏蔽因同步带来的笔记新增和修改
    if (m_accpetAllSearchItems)
    {
        if (documentIndexFromGUID(documentGuid) == -1)
            return false;
    }

    return true;
}


void WizDocumentListView::moveDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument, const QString& targetFolder)
{    
    WizDocumentOperator documentOperator(m_dbMgr);
    documentOperator.moveDocumentsToPersonalFolder(arrayDocument, targetFolder, false);
}

void WizDocumentListView::moveDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument, const WIZTAGDATA& targetTag)
{
    if (arrayDocument.empty()) {
        return;
    }
    WizDatabase& db = m_dbMgr.db(arrayDocument[0].strKbGUID);
    //
    if (!WizAskUserCipherToOperateEncryptedNote(arrayDocument, db))
        return;

    WizDocumentOperator documentOperator(m_dbMgr);
    documentOperator.moveDocumentsToGroupFolder(arrayDocument, targetTag, true);

    WizClearUserCipher(db, m_app.userSettings());
}

void WizDocumentListView::copyDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument,
                                                        const QString& targetFolder, bool keepDocTime, bool keepTag)
{
    if (arrayDocument.empty()) {
        return;
    }
    WizDatabase& db = m_dbMgr.db(arrayDocument[0].strKbGUID);
    //
    if (!WizAskUserCipherToOperateEncryptedNote(arrayDocument, db))
        return;

    WizDocumentOperator documentOperator(m_dbMgr);
    documentOperator.copyDocumentsToPersonalFolder(arrayDocument, targetFolder, keepDocTime,
                                                    keepTag, true);

    WizClearUserCipher(db, m_app.userSettings());
}

void WizDocumentListView::copyDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument,
                                                      const WIZTAGDATA& targetTag, bool keepDocTime)
{
    if (arrayDocument.empty()) {
        return;
    }
    WizDatabase& db = m_dbMgr.db(arrayDocument[0].strKbGUID);
    //
    if (!WizAskUserCipherToOperateEncryptedNote(arrayDocument, db))
        return;

    WizDocumentOperator documentOperator(m_dbMgr);
    documentOperator.copyDocumentsToGroupFolder(arrayDocument, targetTag, keepDocTime, true);

    WizClearUserCipher(db, m_app.userSettings());
}

void WizDocumentListView::duplicateDocuments(const CWizDocumentDataArray& arrayDocument)
{
    for (WIZDOCUMENTDATA doc : arrayDocument)
    {
        WizDatabase& db = m_dbMgr.db(doc.strKbGUID);
        if (db.isGroup())
        {
            CWizStdStringArray arrayTagGUID;
            db.getDocumentTags(doc.strGUID, arrayTagGUID);
            WIZTAGDATA targetTag;
            if (arrayTagGUID.size() > 1)
            {
                qDebug() << "Too may tags found by document : " << doc.strTitle;
                continue;
            }
            else if (arrayTagGUID.size() == 1)
            {
                db.tagFromGuid(*arrayTagGUID.begin(), targetTag);
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

void WizDocumentListView::addSectionItem(const WizDocumentListViewSectionData& secData, const QString& text, int docCount)
{
    WizDocumentListViewSectionItem* sectionItem = new WizDocumentListViewSectionItem(secData, text, docCount);
    sectionItem->setSizeHint(QSize(sizeHint().width(), Utils::WizStyleHelper::listViewItemHeight(Utils::WizStyleHelper::ListTypeSection)));
    sectionItem->setSortingType(m_nSortingType);
    addItem(sectionItem);
    m_sectionItems.append(sectionItem);
}

QString textFromSize(DocSize size)
{
    switch (size) {
    case _0KB:
        return QString(QObject::tr("Unknown size"));
    case _5KB:
        return QString("0KB ~ 5KB");
    case _10KB:
        return QString("5KB ~ 10KB");
    case _30KB:
        return QString("10KB ~ 30KB");
    case _60KB:
        return QString("30KB ~ 60KB");
    case _100KB:
        return QString("60KB ~ 100KB");
    case _200KB:
        return QString("100KB ~ 200KB");
    case _300KB:
        return QString("200KB ~ 300KB");
    case _500KB:
        return QString("300KB ~ 500KB");
    case _1MB:
        return QString("500KB ~ 1MB");
    case _10MB:
        return QString("1MB ~ 10MB");
    case  _30MB:
        return QString("10MB ~ 30MB");
    case  _100MB:
        return QString("30MB ~ 100MB");
    case _1GB:
        return QString(QObject::tr("More than 100MB"));
    default:
        Q_ASSERT(0);
        break;
    }
    return QString();
}

void WizDocumentListView::updateSectionItems()
{
    for (WizDocumentListViewSectionItem* sectionItem : m_sectionItems)
    {
        int index = row(sectionItem);

        takeItem(index);
        sectionItem->deleteLater();
    }
    m_sectionItems.clear();

    //
    switch (m_nSortingType) {
    case SortingByCreatedTime:
    case -SortingByCreatedTime:
    case SortingByModifiedTime:
    case -SortingByModifiedTime:
    case SortingByAccessedTime:
    case -SortingByAccessedTime:
    {
        QMap<QDate, int> dateMap;
        getDocumentDateSections(dateMap);
        QMap<QDate, int>::iterator it;
        for (it = dateMap.begin(); it != dateMap.end(); it++)
        {
            WizDocumentListViewSectionData secData;
            secData.date = it.key();
            QString text = secData.date.toString("yyyy-MM");
            addSectionItem(secData, text, it.value());
        }
    }
        break;
    case SortingByTitle:
    case -SortingByTitle:
    {
        QMap<QString, int> dateMap;
        getDocumentTitleSections(dateMap);
        QMap<QString, int>::iterator it;
        for (it = dateMap.begin(); it != dateMap.end(); it++)
        {
            WizDocumentListViewSectionData secData;
            secData.strInfo = it.key();
            addSectionItem(secData, secData.strInfo, it.value());
        }
    }
        break;
    case SortingByLocation:
    case -SortingByLocation:
    {
        QMap<QString, int> dateMap;
        getDocumentLocationSections(dateMap);
        QMap<QString, int>::iterator it;
        for (it = dateMap.begin(); it != dateMap.end(); it++)
        {
            WizDocumentListViewSectionData secData;
            secData.strInfo = it.key();
            addSectionItem(secData, secData.strInfo, it.value());
        }
    }
        break;
    case SortingBySize:
    case -SortingBySize:
    {
        QMap<QPair<int, int>, int> dateMap;
        getDocumentSizeSections(dateMap);
        QMap<QPair<int, int>, int>::iterator it;
        for (it = dateMap.begin(); it != dateMap.end(); it++)
        {
            WizDocumentListViewSectionData secData;
            secData.sizePair = it.key();
            QString text = textFromSize((DocSize)secData.sizePair.second);
            addSectionItem(secData, text, it.value());
        }
    }
        break;
    default:
        Q_ASSERT(0);
    }

}

bool WizDocumentListView::getDocumentDateSections(QMap<QDate, int>& dateMap)
{
    for (int i = 0; i < count(); i++)
    {
        QListWidgetItem * child = item(i);
        if (child->type() != WizDocumentListType_Document)
            continue;

        WizDocumentListViewDocumentItem* docItem = dynamic_cast<WizDocumentListViewDocumentItem*>(child);
        if (!docItem || docItem->document().isAlwaysOnTop())
            continue;

        //
        QDate dateTime;
        switch (m_nSortingType) {
        case SortingByAccessedTime:
        case -SortingByAccessedTime:
        {
            dateTime = docItem->document().tAccessed.date();
        }
            break;
        case SortingByCreatedTime:
        case -SortingByCreatedTime:
        {
            dateTime = docItem->document().tCreated.date();
        }
            break;
        case SortingByModifiedTime:
        case -SortingByModifiedTime:
        {
            dateTime = docItem->document().tDataModified.date();
        }
            break;
        default:
            return false;
        }
        //
        dateTime.setDate(dateTime.year(), dateTime.month(), 1);

        if (dateMap.contains(dateTime))
        {
            dateMap[dateTime] ++;
        }
        else
        {
            dateMap.insert(dateTime, 1);
        }
    }

    return !dateMap.isEmpty();
}

bool WizDocumentListView::getDocumentSizeSections(QMap<QPair<int, int>, int>& sizeMap)
{
    const int SizesCount  = 13;
    int sizes[SizesCount + 1] = {_0KB, _5KB, _10KB, _30KB, _60KB, _100KB, _200KB, _300KB,
                         _500KB, _1MB, _10MB, _30MB, _100MB, _1GB};
    sizeMap.insert(QPair<int, int>(_0KB, _0KB), 0);
    //
    for (int i = 0; i < count(); i++)
    {
        QListWidgetItem * child = item(i);
        if (child->type() != WizDocumentListType_Document)
            continue;

        WizDocumentListViewDocumentItem* docItem = dynamic_cast<WizDocumentListViewDocumentItem*>(child);
        if (!docItem || docItem->document().isAlwaysOnTop())
            continue;

        WizDatabase& db = m_app.databaseManager().db(docItem->document().strKbGUID);
        QString strFileName = db.getDocumentFileName(docItem->document().strGUID);
        QFileInfo fi(strFileName);
        if (!fi.exists())
        {
            sizeMap[QPair<int, int>(_0KB, _0KB)] ++;
        }
        else
        {
            int m_nSize = fi.size();
            for (int k = 0; k < SizesCount - 1; k++)
            {
                if (m_nSize > sizes[k] && m_nSize <= sizes[k + 1])
                {
                    if (sizeMap.contains(QPair<int, int>(sizes[k], sizes[k + 1])))
                        sizeMap[QPair<int, int>(sizes[k], sizes[k + 1])] ++;
                    else
                        sizeMap.insert(QPair<int, int>(sizes[k], sizes[k + 1]), 1);
                }
            }
        }
    }

    if (sizeMap.value(QPair<int, int>(_0KB, _0KB)) == 0)
    {
        sizeMap.remove(QPair<int, int>(_0KB, _0KB));
    }

    return !sizeMap.isEmpty();
}

bool WizDocumentListView::getDocumentTitleSections(QMap<QString, int>& titleMap)
{
    for (int i = 0; i < count(); i++)
    {
        QListWidgetItem * child = item(i);
        if (child->type() != WizDocumentListType_Document)
            continue;

        WizDocumentListViewDocumentItem* docItem = dynamic_cast<WizDocumentListViewDocumentItem*>(child);
        if (!docItem || docItem->document().isAlwaysOnTop())
            continue;

        QString title = docItem->document().strTitle.toUpper().trimmed();
        QString firstChar = title.left(1);
        if (titleMap.contains(firstChar))
            titleMap[firstChar] ++;
        else
            titleMap.insert(firstChar, 1);
    }
    return !titleMap.isEmpty();
}

bool WizDocumentListView::getDocumentLocationSections(QMap<QString, int>& locationMap)
{
    for (int i = 0; i < count(); i++)
    {
        QListWidgetItem * child = item(i);
        if (child->type() != WizDocumentListType_Document)
            continue;

        WizDocumentListViewDocumentItem* docItem = dynamic_cast<WizDocumentListViewDocumentItem*>(child);
        if (!docItem || docItem->document().isAlwaysOnTop())
            continue;

        WizDatabase& db = m_dbMgr.db(docItem->document().strKbGUID);
        QString firstChar = db.getDocumentLocation(docItem->document());// docItem->document().strLocation;
        if (!db.isGroup())
        {
            firstChar = WizLocation2Display(firstChar);
        }
        //
        if (locationMap.contains(firstChar))
            locationMap[firstChar] ++;
        else
            locationMap.insert(firstChar, 1);
    }
    return !locationMap.isEmpty();
}

bool WizDocumentListView::acceptDocument(const WIZDOCUMENTDATA& document)
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

void WizDocumentListView::addAndSelectDocument(const WIZDOCUMENTDATA& document)
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

void WizDocumentListView::getSelectedDocuments(CWizDocumentDataArray& arrayDocument)
{
    QList<QListWidgetItem*> items = selectedItems();

    QList<QListWidgetItem*>::const_iterator it;
    for (it = items.begin(); it != items.end(); it++)
    {
        QListWidgetItem* pItem = *it;

        WizDocumentListViewDocumentItem* pDocumentItem = dynamic_cast<WizDocumentListViewDocumentItem*>(pItem);
        if (!pDocumentItem)
            continue;

        arrayDocument.push_back(pDocumentItem->itemData().doc);
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

void WizDocumentListView::resetPermission()
{
    CWizDocumentDataArray arrayDocument;
    //QList<QListWidgetItem*> items = selectedItems();
    foreach (WizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems) {
        arrayDocument.push_back(item->document());
    }        

    findAction(WIZACTION_LIST_LOCATE)->setVisible(m_accpetAllSearchItems);

    bool bGroup = isDocumentsWithGroupDocument(arrayDocument);
    bool bDeleted = isDocumentsWithDeleted(arrayDocument);
    bool bCanEdit = isDocumentsAllCanDelete(arrayDocument);
    bool bAlwaysOnTop = isDocumentsAlwaysOnTop(arrayDocument);
    bool bMulti = arrayDocument.size() > 1;

    // if group documents or deleted documents selected
    if (bGroup || bDeleted) {
        findAction(WIZACTION_LIST_TAGS)->setVisible(false);
    } else {
        findAction(WIZACTION_LIST_TAGS)->setVisible(true);       
    }

    findAction(WIZACTION_LIST_COPY_WEB_GROUP_LINK)->setVisible(bGroup && !bDeleted);

    // deleted user private documents
    findAction(WIZACTION_LIST_MOVE_DOCUMENT)->setEnabled(bCanEdit);

    // disable delete if permission is not enough
    findAction(WIZACTION_LIST_DELETE)->setEnabled(bCanEdit);

    findAction(WIZACTION_LIST_COMBINE)->setEnabled(bCanEdit && bMulti);

    findAction(WIZACTION_LIST_ALWAYS_ON_TOP)->setEnabled(bCanEdit);
    findAction(WIZACTION_LIST_ALWAYS_ON_TOP)->setChecked(bAlwaysOnTop);

    findAction(WIZACTION_LIST_SHARE_DOCUMENT_BY_LINK)->setVisible(true);
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
        findAction(WIZACTION_LIST_SHARE_DOCUMENT_BY_LINK)->setVisible(false);
    }
    else
    {
        findAction(WIZACTION_LIST_DOCUMENT_HISTORY)->setEnabled(true);
        WIZDOCUMENTDATA document = (*arrayDocument.begin());
        bool encryptEnable = !document.nProtected;
        setEncryptDocumentActionEnable(encryptEnable);

        // hide share link action for personal group
        if (m_dbMgr.db(document.strKbGUID).isPersonalGroup())
        {
            findAction(WIZACTION_LIST_SHARE_DOCUMENT_BY_LINK)->setVisible(false);
        }
    }


    if (bGroup)
    {
        findAction(WIZACTION_LIST_ENCRYPT_DOCUMENT)->setVisible(false);
        findAction(WIZACTION_LIST_CANCEL_ENCRYPTION)->setVisible(false);
    }
}

QAction* WizDocumentListView::findAction(const QString& strName)
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

bool WizDocumentListView::isDocumentsWithDeleted(const CWizDocumentDataArray& arrayDocument)
{
    foreach (const WIZDOCUMENTDATAEX& doc, arrayDocument) {
       if (doc.strLocation.startsWith(LOCATION_DELETED_ITEMS)) {
           return true;
       }
    }

    return false;
}


bool WizDocumentListView::isDocumentsAlwaysOnTop(const CWizDocumentDataArray& arrayDocument)
{
    foreach (const WIZDOCUMENTDATAEX& doc, arrayDocument) {
        if (doc.isAlwaysOnTop()) {
            return true;
        }
    }

    return false;
}
bool WizDocumentListView::isDocumentsWithGroupDocument(const CWizDocumentDataArray& arrayDocument)
{
    QString strUserGUID = m_dbMgr.db().kbGUID();
    foreach (const WIZDOCUMENTDATAEX& doc, arrayDocument) {
        if (doc.strKbGUID != strUserGUID) {
            return true;
        }
    }

    return false;
}

bool WizDocumentListView::isDocumentsAllCanDelete(const CWizDocumentDataArray& arrayDocument)
{
    foreach (const WIZDOCUMENTDATAEX& doc, arrayDocument) {
        if (!m_dbMgr.db(doc.strKbGUID).canEditDocument(doc)) {
            return false;
        }
    }

    return true;
}

void WizDocumentListView::mousePressEvent(QMouseEvent* event)
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
        WizDocumentListViewDocumentItem* pItem = dynamic_cast<WizDocumentListViewDocumentItem*>(itemAt(event->pos()));
        if (!pItem)
            return;

        // if selectdItems contains clicked item use all selectedItems as special focused item.
        if (selectedItems().contains(pItem))
        {
            foreach (QListWidgetItem* lsItem, selectedItems())
            {
                pItem = dynamic_cast<WizDocumentListViewDocumentItem*>(lsItem);
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

void WizDocumentListView::mouseMoveEvent(QMouseEvent* event)
{
    if ((event->buttons() & Qt::LeftButton) && \
            (event->pos() - m_dragStartPosition).manhattanLength() > QApplication::startDragDistance()) {
        setState(QAbstractItemView::DraggingState);
    }

    QListWidget::mouseMoveEvent(event);
}

void WizDocumentListView::mouseReleaseEvent(QMouseEvent* event)
{
    //
    if (m_itemSelectionChanged)
    {
        emit documentsSelectionChanged();
        m_itemSelectionChanged = false;
    }

    QListWidget::mouseReleaseEvent(event);
}


void WizDocumentListView::keyReleaseEvent(QKeyEvent* event)
{
    //
    if (m_itemSelectionChanged)
    {
        emit documentsSelectionChanged();
        m_itemSelectionChanged = false;
    }

    QListWidget::keyReleaseEvent(event);
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

bool mime2Notes(const QString& mime, WizDatabaseManager& dbMgr, CWizDocumentDataArray& arrayDocument)
{
    if (mime.isEmpty())
        return false;

    QStringList docIDList = mime.split(';', QString::SkipEmptyParts);
    for (QString docID : docIDList)
    {
        QStringList docIDS = docID.split(':');
        if (docIDS.count() != 2)
            continue;

        QString kbGUID = docIDS.first();
        QString guid = docIDS.last();
        WIZDOCUMENTDATA doc;
        if (dbMgr.db(kbGUID).documentFromGuid(guid, doc))
        {
            arrayDocument.push_back(doc);
        }
    }
    return !arrayDocument.empty();
}
QPixmap CreateDocumentDragBadget(const CWizDocumentDataArray& arrayDocument)
{
    const int nImageWidth = 150;
    const int nItemHeight = 22;


    int nItemCount = arrayDocument.size() > 10 ? 10 : arrayDocument.size();

    QPixmap pix(nImageWidth, nItemHeight* (nItemCount + 2) + 8);
    pix.fill(Qt::transparent);


    QPainter pt(&pix);
    QRect rc = pix.rect();
    //draw number
    QRect rcNumber(rc.x() + 12, rc.y(), 18, 18);
    QFont font = pt.font();
    font.setPixelSize(12);
    QFontMetrics fm(font);
    int textWidth = fm.width(QString::number(arrayDocument.size()));
    if (rcNumber.width() < (textWidth + 8))
    {
        rcNumber.setWidth(textWidth + 8);
    }
    pt.setPen(QColor("#FF6052"));
    pt.setBrush(QColor("#FF6052"));
    pt.setRenderHint(QPainter::Antialiasing);
    pt.drawEllipse(rcNumber);

    pt.setPen(QColor("#FFFFFF"));
    pt.drawText(rcNumber, Qt::AlignCenter, QString::number(arrayDocument.size()));

    QRect rcItem(rc.left(), rcNumber.bottom() + 4, rc.width(), nItemHeight - 4);
    //draw doc item
    const int nIconHeight = 14;
    for (int i = 0; i < nItemCount; i++)
    {
        const WIZDOCUMENTDATAEX& doc = arrayDocument.at(i);

        QRect rcIcon(rcItem.left(), rcItem.top() + (rcItem.height() - nIconHeight)/2,
                     nIconHeight, nIconHeight);
        QPixmap pixIcon(Utils::WizStyleHelper::loadPixmap(
                            doc.nProtected == 1 ? "document_badge_encrypted" : "document_badge"));
        pt.drawPixmap(rcIcon, pixIcon);

        //
        QRect rcTitle(rcIcon.right() + 4, rcItem.top(), rcItem.right() - rcIcon.right() - 4, rcItem.height());
        QString text = fm.elidedText(doc.strTitle, Qt::ElideMiddle, rcTitle.width() - 14);
        rcTitle.setWidth(fm.width(text) + 14);
        int leftSpace = nImageWidth - rcIcon.width() - 4;
        rcTitle.setWidth(rcTitle.width() > leftSpace ? leftSpace : rcTitle.width());
        pt.setPen(QColor("#3177EE"));
        pt.setBrush(QColor("#3177EE"));
        pt.drawRoundedRect(rcTitle, 8, 6);

        rcTitle.adjust(4, 0, -4, 0);
        pt.setPen(QColor("#FFFFFF"));
        pt.drawText(rcTitle, Qt::AlignVCenter | Qt::AlignLeft, text);
        rcItem = QRect(rc.left(), rcItem.bottom() + 4, rc.width(), nItemHeight - 4);
    }

    //draw more
    if (nItemCount < arrayDocument.size())
    {
        rcItem.adjust(0, -nItemHeight / 2, 0, -nItemHeight / 2);
        QPen pen(QColor("#3177EE"));
        pen.setWidth(2);
        pt.setPen(pen);
        font.setPixelSize(20);
        pt.setFont(font);
        pt.drawText(rcItem, Qt::AlignLeft | Qt::AlignTop, "......");
    }

    return pix;
}

QPixmap createDragImage(const QString& strMime, WizDatabaseManager& dbMgr, Qt::DropActions supportedActions)
{
    CWizDocumentDataArray arrayDocument;
    if (!mime2Notes(strMime, dbMgr, arrayDocument))
        return QPixmap();

    return CreateDocumentDragBadget(arrayDocument);
}

void WizDocumentListView::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions);

    CWizDocumentDataArray arrayDocument;
    QList<QListWidgetItem*> items = selectedItems();
    foreach (QListWidgetItem* it, items) {
        if (WizDocumentListViewDocumentItem* item = dynamic_cast<WizDocumentListViewDocumentItem*>(it)) {
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
    drag->setPixmap(createDragImage(strMime, m_dbMgr, Qt::MoveAction));

//    Qt::KeyboardModifiers keyMod = QApplication::keyboardModifiers();
//    bool forceCopy = keyMod.testFlag(Qt::AltModifier);
//    Qt::DropAction deafult = forceCopy ? Qt::CopyAction : Qt::MoveAction;

    drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::MoveAction);

    //delete drag would cause crash
//    drag->deleteLater();
}

void WizDocumentListView::dragEnterEvent(QDragEnterEvent *event)
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

void WizDocumentListView::dragMoveEvent(QDragMoveEvent *event)
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

void WizDocumentListView::dropEvent(QDropEvent * event)
{
    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_TAGS))
    {
        if (WizDocumentListViewDocumentItem* item = dynamic_cast<WizDocumentListViewDocumentItem*>(itemAt(event->pos())))
        {
            QByteArray data = event->mimeData()->data(WIZNOTE_MIMEFORMAT_TAGS);
            QString strTagGUIDs = QString::fromUtf8(data, data.length());
            CWizStdStringArray arrayTagGUID;
            ::WizSplitTextToArray(strTagGUIDs, ';', arrayTagGUID);
            foreach (const CString& strTagGUID, arrayTagGUID)
            {
                WIZTAGDATA dataTag;
                if (m_dbMgr.db().tagFromGuid(strTagGUID, dataTag))
                {
                    WizDocument doc(m_dbMgr.db(), item->document());
                    doc.addTag(dataTag);
                }
            }
        }
        event->acceptProposedAction();
    }
    else if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS))
    {
        ::WizGetAnalyzer().logAction("documentListDuplicateDocument");
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

void WizDocumentListView::resetItemsViewType(int type)
{
    m_nViewType = (ViewType)type;

    for (int i = 0; i < count(); i++)
    {
        if (item(i)->type() != WizDocumentListType_Document)
            continue;

        int nHeight = Utils::WizStyleHelper::listViewItemHeight((item(i)->type() == WizDocumentListType_Document) ? viewType() : (int)Utils::WizStyleHelper::ListTypeSection);
        item(i)->setSizeHint(QSize(sizeHint().width(), nHeight));
    }
    //
    m_app.userSettings().set("VIEW_TYPE", QString::number(type));
}


void WizDocumentListView::setLeadInfoState(int state)
{
    if (m_nLeadInfoState != state)
    {
        m_nLeadInfoState = state;

        for (int i = 0; i < count(); i++) {
            WizDocumentListViewBaseItem* pItem = dynamic_cast<WizDocumentListViewBaseItem*>(item(i));
            pItem->setLeadInfoState(state);
        }

        setItemsNeedUpdate();
    }
}

void WizDocumentListView::resetItemsSortingType(int type)
{    
    setItemsNeedUpdate();

    if (m_nSortingType != type)
    {
        m_nSortingType = type;

        updateSectionItems();

        for (int i = 0; i < count(); i++) {
            WizDocumentListViewBaseItem* pItem = dynamic_cast<WizDocumentListViewBaseItem*>(item(i));
            pItem->setSortingType(type);
        }
        //
        m_app.userSettings().set("SORT_TYPE", QString::number(type));
    }

    sortItems();
}

bool WizDocumentListView::isSortedByAccessDate()
{
    return m_nSortingType == SortingByAccessedTime ||
            m_nSortingType == -SortingByAccessedTime;
}

int WizDocumentListView::documentCount() const
{
    int result = 0;
    for (int i = 0; i < count(); i++)
    {
        if (item(i)->type() != WizDocumentListType_Document)
            continue;

        result++;
    }
    return result;
}

void WizDocumentListView::clearAllItems()
{
    clear();
    m_sectionItems.clear();
}


void WizDocumentListView::on_itemSelectionChanged()
{
    //resetPermission();
    m_itemSelectionChanged = true;

    m_rightButtonFocusedItems.clear();
    foreach (QListWidgetItem* lsItem, selectedItems())
    {
        WizDocumentListViewDocumentItem* pItem = dynamic_cast<WizDocumentListViewDocumentItem*>(lsItem);
        if (pItem)
        {
            m_rightButtonFocusedItems.append(pItem);
        }
    }
}

void WizDocumentListView::on_tag_created(const WIZTAGDATA& tag)
{
    Q_UNUSED(tag);
}

void WizDocumentListView::on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew)
{
    Q_UNUSED(tagOld);
    Q_UNUSED(tagNew);
}

void WizDocumentListView::on_document_created(const WIZDOCUMENTDATA& document)
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

void WizDocumentListView::on_document_modified(const WIZDOCUMENTDATA& documentOld,
                                                const WIZDOCUMENTDATA& documentNew)
{
    Q_UNUSED(documentOld);

    if (!acceptDocumentChange(documentNew))
        return;

    if (acceptDocument(documentNew))
    {        
        int index = documentIndexFromGUID(documentNew.strGUID);
        if (-1 == index) {
            addDocument(documentNew, true);
        } else {
            if (WizDocumentListViewDocumentItem* pItem = documentItemAt(index)) {
                pItem->reload(m_dbMgr.db(documentNew.strKbGUID));
                pItem->setSortingType(m_nSortingType);
                update(indexFromItem(pItem));
                if (qAbs(m_nSortingType) == SortingByModifiedTime)
                {
                    updateSectionItems();
                }
                sortItems();
            }
        }
    } else {
        int index = documentIndexFromGUID(documentNew.strGUID);
        if (-1 != index) {
            takeItem(index);
            //
            resetSectionData();
        }
    }
    //
    WizMainWindow::instance()->quickSyncKb(documentNew.strKbGUID);
}

void WizDocumentListView::on_document_param_modified(const WIZDOCUMENTPARAMDATA& param)
{
    if (param.strName != "DOCUMENT_FLAGS")
        return;
    //
    if (!acceptDocumentChange(param.strDocumentGuid))
        return;
    //
    WIZDOCUMENTDATA document;
    if (!m_dbMgr.db(param.strKbGUID).documentFromGuid(param.strDocumentGuid, document))
        return;
    //
    if (acceptDocument(document))
    {
        int index = documentIndexFromGUID(document.strGUID);
        if (-1 == index) {
            addDocument(document, true);
        } else {
            if (WizDocumentListViewDocumentItem* pItem = documentItemAt(index)) {
                pItem->reload(m_dbMgr.db(document.strKbGUID));
                pItem->setSortingType(m_nSortingType);
                update(indexFromItem(pItem));
                updateSectionItems();
                sortItems();
            }
        }
    } else {
        int index = documentIndexFromGUID(document.strGUID);
        if (-1 != index) {
            takeItem(index);
            //
            resetSectionData();
        }
    }
}


void WizDocumentListView::on_documentUploaded(const QString& kbGuid, const QString& docGuid)
{
    int index = documentIndexFromGUID(docGuid);
    if (-1 == index)
        return;
    //
    if (WizDocumentListViewDocumentItem* pItem = documentItemAt(index)) {
        pItem->reload(m_dbMgr.db(kbGuid));
        update(indexFromItem(pItem));
    }
}

void WizDocumentListView::on_document_deleted(const WIZDOCUMENTDATA& document)
{    
    int index = documentIndexFromGUID(document.strGUID);
    if (-1 != index) {
        takeItem(index);

        //
        resetSectionData();
    }
}

void WizDocumentListView::on_documentAccessDate_changed(const WIZDOCUMENTDATA &document)
{
    if (acceptDocument(document) && qAbs(m_nSortingType) == SortingByAccessedTime)
    {
        int index = documentIndexFromGUID(document.strGUID);
        if (WizDocumentListViewDocumentItem* pItem = documentItemAt(index))
        {
            pItem->reload(m_dbMgr.db(document.strKbGUID));
            update(indexFromItem(pItem));
            //
            updateSectionItems();
            sortItems();
        }
    }
}

void WizDocumentListView::on_documentReadCount_changed(const WIZDOCUMENTDATA& document)
{
    if (acceptDocument(document))
    {
        int index = documentIndexFromGUID(document.strGUID);
        if (WizDocumentListViewDocumentItem* pItem = documentItemAt(index))
        {
            WizDatabase& db = m_dbMgr.db(document.strKbGUID);
            if (pItem->document().nReadCount == 0 && db.isGroup())
            {
                emit groupDocumentReadCountChanged(document.strKbGUID);
            }

            pItem->reload(db);
            update(indexFromItem(pItem));
        }
    }
}

void WizDocumentListView::on_action_locate()
{
    ::WizGetAnalyzer().logAction("documentListMenuLocate");
    if (m_rightButtonFocusedItems.size() == 0)
        return;

    WIZDOCUMENTDATA doc = m_rightButtonFocusedItems.first()->document();
    emit loacteDocumetRequest(doc);
}

void WizDocumentListView::on_document_abstractLoaded(const WIZABSTRACT& abs)
{
    int index = documentIndexFromGUID(abs.guid);
    if (-1 == index)
        return;

    // kbGUID also should equal
    if (WizDocumentListViewDocumentItem* pItem = documentItemAt(index))
    {
        pItem->resetAbstract(abs);
        update(indexFromItem(pItem));
    }
}

void WizDocumentListView::on_userAvatar_loaded(const QString& strUserGUID)
{
    WizDocumentListViewDocumentItem* pItem = NULL;
    for (int i = 0; i < count(); i++) {
        pItem = documentItemAt(i);
        if (pItem && pItem->itemData().strAuthorId == strUserGUID) {
            update(indexFromItem(pItem));
        }
    }
}

void WizDocumentListView::onThumbCacheLoaded(const QString& strKbGUID, const QString& strGUID)
{
    setItemsNeedUpdate(strKbGUID, strGUID);

    WizDocumentListViewDocumentItem* pItem = NULL;
    for (int i = 0; i < count(); i++) {
        pItem = documentItemAt(i);
        if (pItem && pItem->itemData().doc.strKbGUID == strKbGUID && pItem->itemData().doc.strGUID == strGUID) {
            update(indexFromItem(pItem));
        }
    }
}

void WizDocumentListView::resetSectionData()
{
    updateSectionItems();
    sortItems();
}

void WizDocumentListView::on_action_documentHistory()
{
    ::WizGetAnalyzer().logAction("documentListMenuHistory");
    if (m_rightButtonFocusedItems.count() != 1)
        return;

   WizDocumentListViewDocumentItem* item = m_rightButtonFocusedItems.first();
   if (!item)
       return;

   const WIZDOCUMENTDATA& doc = item->document();
   WizShowDocumentHistory(doc, window());
}

void WizDocumentListView::on_action_shareDocumentByLink()
{
    ::WizGetAnalyzer().logAction("documentListMenuShareByLink");
    if (m_rightButtonFocusedItems.count() != 1)
        return;

   WizDocumentListViewDocumentItem* item = m_rightButtonFocusedItems.first();
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

void WizDocumentListView::on_action_selectTags()
{
    ::WizGetAnalyzer().logAction("documentListMenuSelectTags");
    if (!m_tagList) {
        m_tagList = new WizTagListWidget(this);
    }

    if (m_rightButtonFocusedItems.isEmpty())
        return;

    CWizDocumentDataArray arrayDocument;
    for (int i = 0; i < m_rightButtonFocusedItems.size(); i++) {
        WizDocumentListViewDocumentItem* pItem = m_rightButtonFocusedItems.at(i);
        arrayDocument.push_back(pItem->document());
    }

    m_tagList->setDocuments(arrayDocument);
    m_tagList->showAtPoint(QCursor::pos());
}

void WizDocumentListView::on_action_deleteDocument()
{
    ::WizGetAnalyzer().logAction("documentListMenuDeleteDocument");
    if (m_rightButtonFocusedItems.isEmpty())
        return;
    //
    m_menuDocument->hide();
    //
    blockSignals(true);
    int index = -1;
    QSet<QString> setKb;
    foreach (WizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems) {
        if (item->type() == WizDocumentListViewDocumentItem::TypeMessage) {
            continue;
        }

        index = indexFromItem(item).row();
        WizDocument doc(m_dbMgr.db(item->document().strKbGUID), item->document());
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

void WizDocumentListView::on_action_moveDocument()
{
    if (m_rightButtonFocusedItems.isEmpty())
        return;
    //
    m_menuDocument->hide();

    CWizDocumentDataArray arrayDocument;
    getSelectedDocuments(arrayDocument);
    if (arrayDocument.size() > 1) {
        WIZKM_CHECK_SYNCING(this);
    }
    //
    ::WizGetAnalyzer().logAction("documentListMenuMoveDocument");
    WizFolderSelector* selector = new WizFolderSelector(tr("Move notes"), m_app, WIZ_USERGROUP_AUTHOR, this);
    selector->setAcceptRoot(false);

    connect(selector, SIGNAL(finished(int)), SLOT(on_action_moveDocument_confirmed(int)));
    //
    QTimer::singleShot(0, [=]() {
        selector->exec();
    });
}

void WizDocumentListView::on_action_moveDocument_confirmed(int result)
{
    qDebug() << "move document confirmed : " << result;
    WizFolderSelector* selector = qobject_cast<WizFolderSelector*>(sender());
    WizScopeGuard guard([&]{
       selector->deleteLater();
    });

    if (result != QDialog::Accepted) {
        return;
    }

    QSet<QString> dbSet;
    // collect documents
    CWizDocumentDataArray arrayDocument;
    foreach (WizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems) {
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

    WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
    for (QString kbGuid : dbSet)
    {
        mainWindow->quickSyncKb(kbGuid);
    }
}

void WizDocumentListView::on_action_copyDocument()
{
    if (m_rightButtonFocusedItems.isEmpty())
        return;
    //
    m_menuDocument->hide();
    //
    ::WizGetAnalyzer().logAction("documentListMenuCopyDocument");
    WizFolderSelector* selector = new WizFolderSelector(tr("Copy documents"), m_app, WIZ_USERGROUP_AUTHOR, this);
    bool isGroup = m_dbMgr.db(m_rightButtonFocusedItems.first()->document().strKbGUID).isGroup();
    selector->setCopyStyle(!isGroup);
    selector->setAcceptRoot(false);

    connect(selector, SIGNAL(finished(int)), SLOT(on_action_copyDocument_confirmed(int)));
    //
    QTimer::singleShot(0, [=]() {
        selector->exec();
    });
}

void WizDocumentListView::on_action_copyDocument_confirmed(int result)
{
    qDebug() << "copy document confirmed : " << result;
    WizFolderSelector* selector = qobject_cast<WizFolderSelector*>(sender());
    WizScopeGuard guard([&]{
       selector->deleteLater();
    });

    if (result != QDialog::Accepted) {
        return;
    }

    QSet<QString> dbSet;
    // collect documents
    CWizDocumentDataArray arrayDocument;
    foreach (WizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems) {
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

    WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
    for (QString kbGuid : dbSet)
    {
        mainWindow->quickSyncKb(kbGuid);
    }
}

void WizDocumentListView::on_action_copyDocumentLink()
{
    ::WizGetAnalyzer().logAction("documentListMenuCopyLink");
    if (m_rightButtonFocusedItems.isEmpty())
        return;
    //
    QList<WIZDOCUMENTDATA> documents;
    foreach(WizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems)
    {
        const WIZDOCUMENTDATA& document = item->document();
        documents.append(document);
    }

    WizCopyNotesAsInternalLink(documents);
}

void WizDocumentListView::on_action_copyWebClientLink()
{
    ::WizGetAnalyzer().logAction("documentListMenuCopyWebClientLink");
    if (m_rightButtonFocusedItems.isEmpty())
        return;
    //
    QList<WIZDOCUMENTDATA> documents;
    foreach(WizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems)
    {
        const WIZDOCUMENTDATA& document = item->document();
        documents.append(document);
    }

    WizCopyNotesAsWebClientLink(documents);
}

void WizDocumentListView::on_action_showDocumentInFloatWindow()
{
    WizMainWindow::instance()->trySaveCurrentNote([=](const QVariant& vRet) {
        //
        ::WizGetAnalyzer().logAction("documentListMenuOpenInFloatWindow");
        WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
        foreach(WizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems)
        {
            const WIZDOCUMENTDATA& document = item->document();
            mainWindow->viewNoteInSeparateWindow(document);
        }
    });
}

void WizDocumentListView::on_menu_aboutToHide()
{
    foreach(WizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems)
    {
        item->setSpecialFocused(false);
    }
    viewport()->update();
}

void WizDocumentListView::on_action_encryptDocument()
{
    WizMainWindow::instance()->trySaveCurrentNote([=](const QVariant& vRet) {
        //
        ::WizGetAnalyzer().logAction("documentListMenuEncryptDocument");
        foreach (WizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems)
        {
            WIZDOCUMENTDATA doc = item->document();
            WizDatabase& db = m_dbMgr.db(doc.strKbGUID);
            db.encryptDocument(doc);
        }
    });
}

void WizDocumentListView::on_action_cancelEncryption()
{
    WizMainWindow::instance()->trySaveCurrentNote([=](const QVariant& vRet) {
        //
        ::WizGetAnalyzer().logAction("documentListMenuCancelEncryptionn");
        //
        foreach(WizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems)
        {
            WIZDOCUMENTDATA doc = item->document();
            if (doc.nProtected)
            {
                WizDatabase& db = m_dbMgr.db(doc.strKbGUID);
                if (!db.cancelDocumentEncryption(doc))
                    return;
            }
        }
    });
}

void WizDocumentListView::on_action_combineNote()
{
    CWizDocumentDataArray documents;
    getSelectedDocuments(documents);
    WizCombineNotesDialog dialog(m_dbMgr, documents, NULL);
    dialog.exec();
}

void WizDocumentListView::on_action_alwaysOnTop()
{
    ::WizGetAnalyzer().logAction("documentListMenuAlwaysOnTop");
    QAction* actionAlwaysOnTop = findAction(WIZACTION_LIST_ALWAYS_ON_TOP);
#ifdef Q_OS_LINUX
    qDebug() << "always on top called, action : " << actionAlwaysOnTop;
#endif


    bool bAlwaysOnTop = actionAlwaysOnTop->isChecked();

    foreach(WizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems)
    {
        WizDatabase& db = m_dbMgr.db(item->document().strKbGUID);
        WIZDOCUMENTDATA doc;
        db.documentFromGuid(item->document().strGUID, doc);
        if (bAlwaysOnTop)
        {
            doc.nFlags |= wizDocumentAlwaysOnTop;
        }
        else
        {
            doc.nFlags &= ~wizDocumentAlwaysOnTop;
        }
        db.setDocumentFlags(doc.strGUID, QString::number(doc.nFlags));
        item->reload(db);
        //
        WizMainWindow::instance()->quickSyncKb(doc.strKbGUID);
    }

    resetSectionData();
}

void WizDocumentListView::on_action_addToShortcuts()
{
    ::WizGetAnalyzer().logAction("documentListMenuAddToShortcuts");
    foreach(WizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems)
    {
        WizDatabase& db = m_dbMgr.db(item->document().strKbGUID);
        WIZDOCUMENTDATA doc;
        if (db.documentFromGuid(item->document().strGUID, doc))
        {
            emit addDocumentToShortcutsRequest(doc);
        }
    }
}


int WizDocumentListView::documentIndexFromGUID(const QString& strGUID)
{
    Q_ASSERT(!strGUID.isEmpty());


    for (int i = 0; i < count();  i++) {
        if (item(i)->type() != WizDocumentListType_Document)
            continue;

        if (WizDocumentListViewDocumentItem *pItem = documentItemAt(i)) {
            if (pItem->document().strGUID == strGUID) {
                return i;
            }
        }
    }

    return -1;
}

WizDocumentListViewBaseItem* WizDocumentListView::itemFromIndex(int index) const
{
    return dynamic_cast<WizDocumentListViewBaseItem*>(item(index));
}

WizDocumentListViewBaseItem* WizDocumentListView::itemFromIndex(const QModelIndex& index) const
{
    return dynamic_cast<WizDocumentListViewBaseItem*>(QListWidget::itemFromIndex(index));
}

WizDocumentListViewDocumentItem*WizDocumentListView::documentItemAt(int index) const
{
    return dynamic_cast<WizDocumentListViewDocumentItem*>(item(index));
}

WizDocumentListViewDocumentItem *WizDocumentListView::documentItemFromIndex(const QModelIndex &index) const
{
    return dynamic_cast<WizDocumentListViewDocumentItem*>(itemFromIndex(index));
}

const WizDocumentListViewItemData& WizDocumentListView::documentItemDataFromIndex(const QModelIndex& index) const
{
    return documentItemFromIndex(index)->itemData();
}

const WIZDOCUMENTDATA& WizDocumentListView::documentFromIndex(const QModelIndex &index) const
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

void WizDocumentListView::wheelEvent(QWheelEvent* event)
{
    //if (event->orientation() == Qt::Vertical) {
        //vscrollBeginUpdate(event->delta());
        //return;
    //}

    int delta = event->delta();
    switch (m_nViewType)
    {
    case TypeThumbnail:
        //delta = delta / 3;
        break;
    case TypeTwoLine:
        //delta = int(delta / 1.5);
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

void WizDocumentListView::vscrollBeginUpdate(int delta)
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

void WizDocumentListView::on_vscrollAnimation_valueChanged(const QVariant& value)
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

void WizDocumentListView::on_vscrollAnimation_finished()
{
    QPropertyAnimation* animation = qobject_cast<QPropertyAnimation *>(sender());
    qDebug() << "end:" << animation->startValue() << ":" << animation->endValue();

    //reset
    //m_vscrollDelta = 0;
    //m_vscrollCurrent = 0;
}


int WizDocumentListView::numOfEncryptedDocuments(const CWizDocumentDataArray& docArray)
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

void WizDocumentListView::setEncryptDocumentActionEnable(bool enable)
{
    findAction(WIZACTION_LIST_ENCRYPT_DOCUMENT)->setVisible(enable);
    findAction(WIZACTION_LIST_CANCEL_ENCRYPTION)->setVisible(!enable);
}

void WizDocumentListView::on_vscroll_update()
{
    // scroll animation stop condition
    //if (qAbs(m_vscrollDelta) > m_vscrollCurrent) {
    //    verticalScrollBar()->setValue(m_vscrollOldPos - m_vscrollDelta/2);
    //    m_vscrollCurrent += qAbs(m_vscrollDelta/2);
    //} else {
    //    m_vscrollTimer.stop();
    //}
}

void WizDocumentListView::on_vscroll_valueChanged(int value)
{
    m_vscrollOldPos = value;
}

void WizDocumentListView::on_vscroll_actionTriggered(int action)
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

void WizDocumentListView::drawItem(QPainter* p, const QStyleOptionViewItem* vopt) const
{
    if (WizDocumentListViewBaseItem* pItem = itemFromIndex(vopt->index))
    {
        p->save();
        int nRightMargin = WizSmartScaleUI(12);
        QStyleOptionViewItem newVopt(*vopt);
        //newVopt.rect.setRight(newVopt.rect.right() - nRightMargin);
        pItem->draw(p, &newVopt, viewType());

        p->restore();
    }
}


void WizDocumentListView::reloadItem(const QString& strKbGUID, const QString& strGUID)
{
    int index = documentIndexFromGUID(strGUID);
    if (-1 == index)
        return;

    if (WizDocumentListViewDocumentItem* pItem = documentItemAt(index))
    {
        pItem->reload(m_dbMgr.db(strKbGUID));
        //m_dbMgr.db(strKbGUID).UpdateDocumentAbstract(strGUID);
        update(indexFromItem(pItem));
    }
}

bool WizDocumentListView::acceptAllSearchItems() const
{
    return m_accpetAllSearchItems;
}

void WizDocumentListView::setAcceptAllSearchItems(bool bAccept)
{
    m_accpetAllSearchItems = bAccept;
}

void WizDocumentListView::setItemsNeedUpdate(const QString& strKbGUID, const QString& strGUID)
{
    if (strKbGUID.isEmpty() || strGUID.isEmpty()) {
        for (int i = 0; i < count() && item(i)->type() == WizDocumentListType_Document; i++) {
            WizDocumentListViewDocumentItem* pItem = dynamic_cast<WizDocumentListViewDocumentItem*>(item(i));
            Q_ASSERT(pItem);

            pItem->setNeedUpdate();
        }

        return;
    }

    WizDocumentListViewDocumentItem* pItem = NULL;
    for (int i = 0; i < count(); i++) {
        pItem = documentItemAt(i);
        if (pItem && pItem->itemData().doc.strKbGUID == strKbGUID && pItem->itemData().doc.strGUID == strGUID) {
            pItem->setNeedUpdate();
        }
    }
}


void WizDocumentListView::paintEvent(QPaintEvent *e)
{
    QListView::paintEvent(e);
    if (model() && model()->rowCount(rootIndex()) > 0)
       return;
    // The view is empty.

    QPainter p(viewport());
    //
    if (m_emptyFolder.isNull()) {
        m_emptyFolder = QPixmap(Utils::WizStyleHelper::loadPixmap("empty_folder"));
    }
    if (m_emptySearch.isNull()) {
        m_emptySearch = QPixmap(Utils::WizStyleHelper::loadPixmap("empty_search"));
    }
    //
    QPixmap pixmap = isSearchResult() ? m_emptySearch : m_emptyFolder;
    QSize imageSize = pixmap.size();
#ifdef Q_OS_MAC
    imageSize.setWidth(int(imageSize.width() / pixmap.devicePixelRatio()));
    imageSize.setHeight(int(imageSize.height() / pixmap.devicePixelRatio()));
#endif
    QRect rc = rect();
    //
    QString text = isSearchResult()
            ? tr("No search results...\nTry to change another keyword or advanced searching")
            : tr("Nothing in here\nGo to create a note");
    //
    int spacing = WizSmartScaleUI(10);
    //
    QRect textRect = rc;
    textRect.adjust(WizSmartScaleUI(16), 0, WizSmartScaleUI(-16), 0);
    QFontMetrics fm(p.font());
    int textHeight = fm.boundingRect(textRect, Qt::TextWordWrap, text).height();
    int totalHeight = imageSize.height() + textHeight + spacing;
    //
    int x = (rc.size().width() - imageSize.width()) / 2;
    int y = (rc.size().height() - totalHeight) / 2;
    //
    p.drawPixmap(x, y, pixmap);
    //
    int textY = y + imageSize.height() + spacing;
    textRect.setTop(textY);
    textRect.setHeight(textHeight);
    //
    p.setPen(QColor(0xcc, 0xcc, 0xcc));
    p.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, text);
}
