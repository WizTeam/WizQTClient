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
#define WIZACTION_LIST_LOCATE   QObject::tr("Locate")
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


CWizDocumentListView::CWizDocumentListView(CWizExplorerApp& app, QWidget *parent /*= 0*/)
    : QListWidget(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_tagList(NULL)
    , m_itemSelectionChanged(false)
    , m_accpetAllSearchItems(false)
    , m_nLeadInfoState(DocumentLeadInfo_None)
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
    m_vScroll = new CWizScrollBar(this);
    m_vScroll->syncWith(verticalScrollBar());
    m_vScroll->applyStyle("#F5F5F5", "#C1C1C1", true);
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
    connect(&m_dbMgr, SIGNAL(documentAccessDateModified(WIZDOCUMENTDATA)),
            SLOT(on_documentAccessDate_changed(WIZDOCUMENTDATA)));

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
    m_menuDocument = std::make_shared<QMenu>();
    m_menuDocument->addAction(WIZACTION_LIST_LOCATE, this,
                              SLOT(on_action_locate()));

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
    QAction* actionAlwaysOnTop = m_menuDocument->addAction(WIZACTION_LIST_ALWAYS_ON_TOP,
                                                         this, SLOT(on_action_alwaysOnTop()));
    actionAlwaysOnTop->setCheckable(true);
//    m_menuDocument->addAction(WIZACTION_LIST_CANCEL_ON_TOP,
//                                                     this, SLOT(on_action_cancelOnTop()));


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

    m_menuDocument->addSeparator();
    QAction* actionDeleteDoc = m_menuDocument->addAction(WIZACTION_LIST_DELETE,
                                                         this, SLOT(on_action_deleteDocument()), QKeySequence::Delete);
    // not implement, hide currently.
//    actionCopyDoc->setVisible(false);


    actionDeleteDoc->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionMoveDoc->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionCopyDoc->setShortcutContext(Qt::WidgetWithChildrenShortcut);



    //m_actionEncryptDocument = new QAction(tr("Encrypt Document"), m_menu);
    //connect(m_actionEncryptDocument, SIGNAL(triggered()), SLOT(on_action_encryptDocument()));
    //m_menu->addAction(m_actionEncryptDocument);
    connect(m_menuDocument.operator ->(), SIGNAL(aboutToHide()), SLOT(on_menu_aboutToHide()));
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
    m_sectionItems.clear();


    verticalScrollBar()->setValue(0);

    appendDocuments(arrayDocument);
}

void CWizDocumentListView::appendDocuments(const CWizDocumentDataArray& arrayDocument)
{
    CWizDocumentDataArray::const_iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
        addDocument(*it);
    }

    updateSectionItems();

    sortItems();

    Q_EMIT documentCountChanged();
}

int CWizDocumentListView::addDocument(const WIZDOCUMENTDATA& doc, bool sort)
{
    addDocument(doc);

    updateSectionItems();

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
        data.nType = CWizDocumentListViewDocumentItem::TypePrivateDocument;
    } else {
        data.nType = CWizDocumentListViewDocumentItem::TypeGroupDocument;
        data.strAuthorId = doc.strOwner;
    }


    CWizDocumentListViewDocumentItem* pItem = new CWizDocumentListViewDocumentItem(m_app, data);
    pItem->setSizeHint(QSize(sizeHint().width(), Utils::StyleHelper::listViewItemHeight(m_nViewType)));
    pItem->setLeadInfoState(m_nLeadInfoState);
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

void CWizDocumentListView::addSectionItem(const WizDocumentListViewSectionData& secData, const QString& text, int docCount)
{
    CWizDocumentListViewSectionItem* sectionItem = new CWizDocumentListViewSectionItem(secData, text, docCount);
    sectionItem->setSizeHint(QSize(sizeHint().width(), Utils::StyleHelper::listViewItemHeight(Utils::StyleHelper::ListTypeSection)));
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

void CWizDocumentListView::updateSectionItems()
{
    for (CWizDocumentListViewSectionItem* sectionItem : m_sectionItems)
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
//            if (WizIsChineseLanguage(m_app.userSettings().locale()))
//            {
//                text = secData.date.toString("yyyy") + tr("year") + secData.date.toString("MMMM");
//            }
//            else
//            {
//                text = secData.date.toString("MMMM yyyy");
//            }
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
    case SortingByTag:
    case -SortingByTag:
        //TODO:
        break;
    default:
        Q_ASSERT(0);
    }

}

bool CWizDocumentListView::getDocumentDateSections(QMap<QDate, int>& dateMap)
{
    for (int i = 0; i < count(); i++)
    {
        QListWidgetItem * child = item(i);
        if (child->type() != WizDocumentListType_Document)
            continue;

        CWizDocumentListViewDocumentItem* docItem = dynamic_cast<CWizDocumentListViewDocumentItem*>(child);
        if (!docItem || docItem->document().nFlags & wizDocumentAlwaysOnTop)
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

bool CWizDocumentListView::getDocumentSizeSections(QMap<QPair<int, int>, int>& sizeMap)
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

        CWizDocumentListViewDocumentItem* docItem = dynamic_cast<CWizDocumentListViewDocumentItem*>(child);
        if (!docItem || docItem->document().nFlags & wizDocumentAlwaysOnTop)
            continue;

        CWizDatabase& db = m_app.databaseManager().db(docItem->document().strKbGUID);
        QString strFileName = db.GetDocumentFileName(docItem->document().strGUID);
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

bool CWizDocumentListView::getDocumentTitleSections(QMap<QString, int>& titleMap)
{
    for (int i = 0; i < count(); i++)
    {
        QListWidgetItem * child = item(i);
        if (child->type() != WizDocumentListType_Document)
            continue;

        CWizDocumentListViewDocumentItem* docItem = dynamic_cast<CWizDocumentListViewDocumentItem*>(child);
        if (!docItem || docItem->document().nFlags & wizDocumentAlwaysOnTop)
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

bool CWizDocumentListView::getDocumentLocationSections(QMap<QString, int>& locationMap)
{
    for (int i = 0; i < count(); i++)
    {
        QListWidgetItem * child = item(i);
        if (child->type() != WizDocumentListType_Document)
            continue;

        CWizDocumentListViewDocumentItem* docItem = dynamic_cast<CWizDocumentListViewDocumentItem*>(child);
        if (!docItem || docItem->document().nFlags & wizDocumentAlwaysOnTop)
            continue;

        CWizDatabase& db = m_dbMgr.db(docItem->document().strKbGUID);
        QString firstChar = db.GetDocumentLocation(docItem->document());// docItem->document().strLocation;
        if (!db.IsGroup())
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

        CWizDocumentListViewDocumentItem* pDocumentItem = dynamic_cast<CWizDocumentListViewDocumentItem*>(pItem);
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

void CWizDocumentListView::resetPermission()
{
#ifdef Q_OS_LINUX
    qDebug() << "reset context menu permission, action always on top : " << findAction(WIZACTION_LIST_ALWAYS_ON_TOP);
    qDebug() << "right menu selected item count : " << m_rightButtonFocusedItems.size();
#endif

    CWizDocumentDataArray arrayDocument;
    //QList<QListWidgetItem*> items = selectedItems();
    foreach (CWizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems) {
        arrayDocument.push_back(item->document());
    }        

    findAction(WIZACTION_LIST_LOCATE)->setVisible(m_accpetAllSearchItems);

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
        CWizDocumentListViewDocumentItem* pItem = dynamic_cast<CWizDocumentListViewDocumentItem*>(itemAt(event->pos()));
        if (!pItem)
            return;

        // if selectdItems contains clicked item use all selectedItems as special focused item.
        if (selectedItems().contains(pItem))
        {
            foreach (QListWidgetItem* lsItem, selectedItems())
            {
                pItem = dynamic_cast<CWizDocumentListViewDocumentItem*>(lsItem);
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

bool mime2Notes(const QString& mime, CWizDatabaseManager& dbMgr, CWizDocumentDataArray& arrayDocument)
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
        if (dbMgr.db(kbGUID).DocumentFromGUID(guid, doc))
        {
            arrayDocument.push_back(doc);
        }
    }
    return !arrayDocument.empty();
}

QPixmap createDragImage(const QString& strMime, CWizDatabaseManager& dbMgr, Qt::DropActions supportedActions)
{
    CWizDocumentDataArray arrayDocument;
    if (!mime2Notes(strMime, dbMgr, arrayDocument))
        return QPixmap();

    return WizGetDocumentDragBadget(arrayDocument.size());
}

void CWizDocumentListView::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions);

    CWizDocumentDataArray arrayDocument;
    QList<QListWidgetItem*> items = selectedItems();
    foreach (QListWidgetItem* it, items) {
        if (CWizDocumentListViewDocumentItem* item = dynamic_cast<CWizDocumentListViewDocumentItem*>(it)) {
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

    connect(drag, SIGNAL(actionChanged(Qt::DropAction)), SLOT(updateDragOperationImage(Qt::DropAction)));

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
        if (CWizDocumentListViewDocumentItem* item = dynamic_cast<CWizDocumentListViewDocumentItem*>(itemAt(event->pos())))
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

    for (int i = 0; i < count(); i++)
    {
        if (item(i)->type() != WizDocumentListType_Document)
            continue;

        int nHeight = Utils::StyleHelper::listViewItemHeight((item(i)->type() == WizDocumentListType_Document) ? m_nViewType : Utils::StyleHelper::ListTypeSection);
        item(i)->setSizeHint(QSize(sizeHint().width(), nHeight));
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
        sz.setHeight(Utils::StyleHelper::thumbnailHeight());
        return sz;
    default:
        Q_ASSERT(0);
    }

    return sz;
}

void CWizDocumentListView::setLeadInfoState(int state)
{
    if (m_nLeadInfoState != state)
    {
        m_nLeadInfoState = state;

        for (int i = 0; i < count(); i++) {
            CWizDocumentListViewBaseItem* pItem = dynamic_cast<CWizDocumentListViewBaseItem*>(item(i));
            pItem->setLeadInfoState(state);
        }

        setItemsNeedUpdate();
    }
}

void CWizDocumentListView::resetItemsSortingType(int type)
{    
    setItemsNeedUpdate();

    if (m_nSortingType != type)
    {
        m_nSortingType = type;

        updateSectionItems();

        for (int i = 0; i < count(); i++) {
            CWizDocumentListViewBaseItem* pItem = dynamic_cast<CWizDocumentListViewBaseItem*>(item(i));
            pItem->setSortingType(type);
        }
    }

    sortItems();
}

bool CWizDocumentListView::isSortedByAccessDate()
{
    return m_nSortingType == SortingByAccessedTime ||
            m_nSortingType == -SortingByAccessedTime;
}

int CWizDocumentListView::documentCount() const
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

void CWizDocumentListView::on_itemSelectionChanged()
{
    //resetPermission();
    m_itemSelectionChanged = true;

    m_rightButtonFocusedItems.clear();
    foreach (QListWidgetItem* lsItem, selectedItems())
    {
        CWizDocumentListViewDocumentItem* pItem = dynamic_cast<CWizDocumentListViewDocumentItem*>(lsItem);
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
            if (CWizDocumentListViewDocumentItem* pItem = documentItemAt(index)) {
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
}

void CWizDocumentListView::on_document_deleted(const WIZDOCUMENTDATA& document)
{    
    int index = documentIndexFromGUID(document.strGUID);
    if (-1 != index) {
        takeItem(index);

        //
        resetSectionData();
    }
}

void CWizDocumentListView::on_documentAccessDate_changed(const WIZDOCUMENTDATA &document)
{
    if (acceptDocument(document) && qAbs(m_nSortingType) == SortingByAccessedTime)
    {
        int index = documentIndexFromGUID(document.strGUID);
        if (CWizDocumentListViewDocumentItem* pItem = documentItemAt(index))
        {
            pItem->reload(m_dbMgr.db(document.strKbGUID));
            update(indexFromItem(pItem));
            //
            updateSectionItems();
            sortItems();
        }
    }
}

void CWizDocumentListView::on_documentReadCount_changed(const WIZDOCUMENTDATA& document)
{
    if (acceptDocument(document))
    {
        int index = documentIndexFromGUID(document.strGUID);
        if (CWizDocumentListViewDocumentItem* pItem = documentItemAt(index))
        {
            CWizDatabase& db = m_dbMgr.db(document.strKbGUID);
            if (pItem->document().nReadCount == 0 && db.IsGroup())
            {
                emit groupDocumentReadCountChanged(document.strKbGUID);
            }

            pItem->reload(db);
            update(indexFromItem(pItem));
        }
    }
}

void CWizDocumentListView::on_action_locate()
{
    ::WizGetAnalyzer().LogAction("documentListMenuLocate");
    if (m_rightButtonFocusedItems.size() == 0)
        return;

    WIZDOCUMENTDATA doc = m_rightButtonFocusedItems.first()->document();
    emit loacteDocumetRequest(doc);
}

void CWizDocumentListView::on_document_abstractLoaded(const WIZABSTRACT& abs)
{
    int index = documentIndexFromGUID(abs.guid);
    if (-1 == index)
        return;

    // kbGUID also should equal
    if (CWizDocumentListViewDocumentItem* pItem = documentItemAt(index))
    {
        pItem->resetAbstract(abs);
        update(indexFromItem(pItem));
    }
}

void CWizDocumentListView::on_userAvatar_loaded(const QString& strUserGUID)
{
    CWizDocumentListViewDocumentItem* pItem = NULL;
    for (int i = 0; i < count(); i++) {
        pItem = documentItemAt(i);
        if (pItem && pItem->itemData().strAuthorId == strUserGUID) {
            update(indexFromItem(pItem));
        }
    }
}

void CWizDocumentListView::onThumbCacheLoaded(const QString& strKbGUID, const QString& strGUID)
{
    setItemsNeedUpdate(strKbGUID, strGUID);

    CWizDocumentListViewDocumentItem* pItem = NULL;
    for (int i = 0; i < count(); i++) {
        pItem = documentItemAt(i);
        if (pItem && pItem->itemData().doc.strKbGUID == strKbGUID && pItem->itemData().doc.strGUID == strGUID) {
            update(indexFromItem(pItem));
        }
    }
}

void CWizDocumentListView::resetSectionData()
{
    updateSectionItems();
    sortItems();
}

void CWizDocumentListView::on_action_documentHistory()
{
    ::WizGetAnalyzer().LogAction("documentListMenuHistory");
    if (m_rightButtonFocusedItems.count() != 1)
        return;

   CWizDocumentListViewDocumentItem* item = m_rightButtonFocusedItems.first();
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

   CWizDocumentListViewDocumentItem* item = m_rightButtonFocusedItems.first();
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
        CWizDocumentListViewDocumentItem* pItem = m_rightButtonFocusedItems.at(i);
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
    foreach (CWizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems) {
        if (item->type() == CWizDocumentListViewDocumentItem::TypeMessage) {
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
    foreach (CWizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems) {
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
    foreach (CWizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems) {
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
    foreach(CWizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems)
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
    foreach(CWizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems)
    {
        const WIZDOCUMENTDATA& document = item->document();
        mainWindow->viewNoteInSeparateWindow(document);
    }
}

void CWizDocumentListView::on_menu_aboutToHide()
{
    foreach(CWizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems)
    {
        item->setSpecialFocused(false);
    }
    viewport()->update();
}

void CWizDocumentListView::on_action_encryptDocument()
{
    ::WizGetAnalyzer().LogAction("documentListMenuEncryptDocument");
    foreach(CWizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems)
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
    foreach(CWizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems)
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
    QAction* actionAlwaysOnTop = findAction(WIZACTION_LIST_ALWAYS_ON_TOP);
#ifdef Q_OS_LINUX
    qDebug() << "always on top called, action : " << actionAlwaysOnTop;
#endif


    bool bAlwaysOnTop = actionAlwaysOnTop->isChecked();

    foreach(CWizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems)
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

    resetSectionData();
}

void CWizDocumentListView::on_action_addToShortcuts()
{
    ::WizGetAnalyzer().LogAction("documentListMenuAddToShortcuts");
    foreach(CWizDocumentListViewDocumentItem* item, m_rightButtonFocusedItems)
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


    for (int i = 0; i < count();  i++) {
        if (item(i)->type() != WizDocumentListType_Document)
            continue;

        if (CWizDocumentListViewDocumentItem *pItem = documentItemAt(i)) {
            if (pItem->document().strGUID == strGUID) {
                return i;
            }
        }
    }

    return -1;
}

CWizDocumentListViewBaseItem* CWizDocumentListView::itemFromIndex(int index) const
{
    return dynamic_cast<CWizDocumentListViewBaseItem*>(item(index));
}

CWizDocumentListViewBaseItem* CWizDocumentListView::itemFromIndex(const QModelIndex& index) const
{
    return dynamic_cast<CWizDocumentListViewBaseItem*>(QListWidget::itemFromIndex(index));
}

CWizDocumentListViewDocumentItem*CWizDocumentListView::documentItemAt(int index) const
{
    return dynamic_cast<CWizDocumentListViewDocumentItem*>(item(index));
}

CWizDocumentListViewDocumentItem *CWizDocumentListView::documentItemFromIndex(const QModelIndex &index) const
{
    return dynamic_cast<CWizDocumentListViewDocumentItem*>(itemFromIndex(index));
}

const WizDocumentListViewItemData& CWizDocumentListView::documentItemDataFromIndex(const QModelIndex& index) const
{
    return documentItemFromIndex(index)->itemData();
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



void CWizDocumentListView::updateDragOperationImage(Qt::DropAction action)
{
    qDebug() << "drag drop action changed ; " << action;
    if (QDrag* drag = dynamic_cast<QDrag*>(sender()))
    {
//        drag->mimeData();
    }
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
    if (CWizDocumentListViewBaseItem* pItem = itemFromIndex(vopt->index))
    {
        p->save();
        int nRightMargin = 12;
        QStyleOptionViewItemV4 newVopt(*vopt);
        newVopt.rect.setRight(newVopt.rect.right() - nRightMargin);
        pItem->draw(p, &newVopt, m_nViewType);

        p->restore();
    }
}


void CWizDocumentListView::reloadItem(const QString& strKbGUID, const QString& strGUID)
{
    int index = documentIndexFromGUID(strGUID);
    if (-1 == index)
        return;

    if (CWizDocumentListViewDocumentItem* pItem = documentItemAt(index))
    {
        pItem->reload(m_dbMgr.db(strKbGUID));
        //m_dbMgr.db(strKbGUID).UpdateDocumentAbstract(strGUID);
        update(indexFromItem(pItem));
    }
}

bool CWizDocumentListView::acceptAllSearchItems() const
{
    return m_accpetAllSearchItems;
}

void CWizDocumentListView::setAcceptAllSearchItems(bool bAccept)
{
    m_accpetAllSearchItems = bAccept;
}

void CWizDocumentListView::setItemsNeedUpdate(const QString& strKbGUID, const QString& strGUID)
{
    if (strKbGUID.isEmpty() || strGUID.isEmpty()) {
        for (int i = 0; i < count() && item(i)->type() == WizDocumentListType_Document; i++) {
            CWizDocumentListViewDocumentItem* pItem = dynamic_cast<CWizDocumentListViewDocumentItem*>(item(i));
            Q_ASSERT(pItem);

            pItem->setNeedUpdate();
        }

        return;
    }

    CWizDocumentListViewDocumentItem* pItem = NULL;
    for (int i = 0; i < count(); i++) {
        pItem = documentItemAt(i);
        if (pItem && pItem->itemData().doc.strKbGUID == strKbGUID && pItem->itemData().doc.strGUID == strGUID) {
            pItem->setNeedUpdate();
        }
    }
}
