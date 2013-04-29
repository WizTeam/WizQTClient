#include "wizdocumentlistview.h"

//#include <QStyledItemDelegate>
//#include <QApplication>
//#include <QMenu>
//#include <QContextMenuEvent>
//#include <QScrollBar>
//#include <QDrag>
//#include <QMimeData>

#include "wizcategoryview.h"
#include "wiznotestyle.h"
#include "wiztaglistwidget.h"
#include "share/wizsettings.h"


class CWizDocumentListViewItem : public QListWidgetItem
{
protected:
    WIZDOCUMENTDATA m_data;
    WIZABSTRACT m_abstract;
    CString m_tags;

public:
    CWizDocumentListViewItem(const WIZDOCUMENTDATA& data, QListWidget *view = 0, int type = Type)
        : QListWidgetItem(view, type)
        , m_data(data)
    {
        setText(m_data.strTitle);
    }

    const WIZDOCUMENTDATA& document() const
    {
        return m_data;
    }

    WIZABSTRACT& abstract(CWizThumbIndexCache& thumbCache)
    {
        if (m_abstract.strKbGUID.isEmpty()) {
            thumbCache.load(m_data.strKbGUID, m_data.strGUID);
        }

        return m_abstract;
    }

    CString tags(CWizDatabase& db)
    {
        if (m_tags.IsEmpty())
        {
            m_tags = db.GetDocumentTagDisplayNameText(m_data.strGUID);
            m_tags = " " + m_tags;
        }

        return m_tags;
    }

    void reload(CWizDatabase& db)
    {
        db.DocumentFromGUID(m_data.strGUID, m_data);
        m_abstract = WIZABSTRACT();
        m_tags.clear();

        setText("");    //force repaint
        setText(m_data.strTitle);
    }

    virtual bool operator<(const QListWidgetItem &other) const
    {
        const CWizDocumentListViewItem* pOther = dynamic_cast<const CWizDocumentListViewItem*>(&other);
        Q_ASSERT(pOther);

        if (pOther->m_data.tCreated == m_data.tCreated)
        {
            return text().compare(other.text(), Qt::CaseInsensitive) < 0;
        }

        return pOther->m_data.tCreated < m_data.tCreated;
    }

    void resetAbstract(const WIZABSTRACT& abs)
    {
        m_abstract.strKbGUID = abs.strKbGUID;
        m_abstract.guid= abs.guid;
        m_abstract.text = abs.text;
        m_abstract.image = abs.image;
    }
};

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

#define WIZACTION_LIST_DELETE   QObject::tr("Delete")
#define WIZACTION_LIST_TAGS     QObject::tr("Tags...")

CWizDocumentListView::CWizDocumentListView(CWizExplorerApp& app, QWidget *parent /*= 0*/)
    : QListWidget(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_tagList(NULL)
{
    setFrameStyle(QFrame::NoFrame);
    setAttribute(Qt::WA_MacShowFocusRect, false);

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

    // setup background
    QPalette pal = palette();
    pal.setColor(QPalette::Base, QColor(247,247,247));
    setPalette(pal);

    setStyle(::WizGetStyle(m_app.userSettings().skin()));

    qRegisterMetaType<WIZTAGDATA>("WIZTAGDATA");
    qRegisterMetaType<WIZDOCUMENTDATA>("WIZDOCUMENTDATA");

    connect(&m_dbMgr, SIGNAL(tagCreated(const WIZTAGDATA&)), \
            SLOT(on_tag_created(const WIZTAGDATA&)));

    connect(&m_dbMgr, SIGNAL(tagModified(const WIZTAGDATA&, const WIZTAGDATA&)), \
            SLOT(on_tag_modified(const WIZTAGDATA&, const WIZTAGDATA&)));

    connect(&m_dbMgr, SIGNAL(documentCreated(const WIZDOCUMENTDATA&)), \
            SLOT(on_document_created(const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(documentModified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)), \
            SLOT(on_document_modified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(documentDeleted(const WIZDOCUMENTDATA&)), \
            SLOT(on_document_deleted(const WIZDOCUMENTDATA&)));


    m_thumbCache = new CWizThumbIndexCache(app);
    qRegisterMetaType<WIZABSTRACT>("WIZABSTRACT");
    connect(m_thumbCache, SIGNAL(loaded(const WIZABSTRACT&)),
            SLOT(on_document_abstractLoaded(const WIZABSTRACT&)));

    QThread *thread = new QThread();
    m_thumbCache->moveToThread(thread);
    thread->start();

    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);
    viewport()->setAcceptDrops(true);

    setSelectionMode(QAbstractItemView::ExtendedSelection);

    // setup popup menu
    m_menu = new QMenu(this);
    m_menu->addAction(WIZACTION_LIST_TAGS, this, SLOT(on_action_selectTags()));
    m_menu->addSeparator();
    m_menu->addAction(WIZACTION_LIST_DELETE, this, SLOT(on_action_deleteDocument()));
    //m_actionEncryptDocument = new QAction(tr("Encrypt Document"), m_menu);
    //connect(m_actionEncryptDocument, SIGNAL(triggered()), SLOT(on_action_encryptDocument()));
    //m_menu->addAction(m_actionEncryptDocument);
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
    clear();
    addDocuments(arrayDocument);
}

void CWizDocumentListView::addDocuments(const CWizDocumentDataArray& arrayDocument)
{
    CWizDocumentDataArray::const_iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
        addDocument(*it, false);
    }

    sortItems();

    //if (selectedItems().empty()) {
    //    setCurrentRow(0);
    //}
}

int CWizDocumentListView::addDocument(const WIZDOCUMENTDATA& data, bool sort)
{
    CWizDocumentListViewItem* pItem = new CWizDocumentListViewItem(data, this);

    addItem(pItem);
    if (sort) {
        sortItems();
    }

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
        if (pDocumentItem)
        {
            arrayDocument.push_back(pDocumentItem->document());
        }
    }
}


void CWizDocumentListView::contextMenuEvent(QContextMenuEvent * e)
{
    // reset permission
    resetPermission();
    m_menu->popup(mapToGlobal(e->pos()));
}

void CWizDocumentListView::resetPermission()
{
    // disable tags if group documents inside
    if (isSelectedGroupDocument()) {
        findAction(WIZACTION_LIST_TAGS)->setEnabled(false);
    } else {
        findAction(WIZACTION_LIST_TAGS)->setEnabled(true);
    }

    // disable delete if permission is not enough
    if (!isSelectedAllCanDelete()) {
        findAction(WIZACTION_LIST_DELETE)->setEnabled(false);
    } else {
        findAction(WIZACTION_LIST_DELETE)->setEnabled(true);
    }
}

QAction* CWizDocumentListView::findAction(const QString& strName)
{
    QList<QAction *> actionList;
    actionList.append(m_menu->actions());

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

bool CWizDocumentListView::isSelectedGroupDocument()
{
    QList<QListWidgetItem*> items = selectedItems();
    foreach (QListWidgetItem* it, items) {
        if (CWizDocumentListViewItem* item = dynamic_cast<CWizDocumentListViewItem*>(it)) {
            if (item->document().strKbGUID != m_dbMgr.db().kbGUID()) {
                return true;
            }
        }
    }

    return false;
}

bool CWizDocumentListView::isSelectedAllCanDelete()
{
    QList<QListWidgetItem*> items = selectedItems();
    foreach (QListWidgetItem* it, items) {
        if (CWizDocumentListViewItem* item = dynamic_cast<CWizDocumentListViewItem*>(it)) {
            if (m_dbMgr.db(item->document().strKbGUID).permission()
                    > WIZ_USERGROUP_EDITOR) {
                return false;
            }
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

    // FIXME: need deal with more then 1 drag event!
    if (items.size() == 1) {
        QRect rect = visualItemRect(items[0]);
        drag->setPixmap(QPixmap::grabWindow(winId(), rect.x(), rect.y(), rect.width(), rect.height()));
    }

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

void CWizDocumentListView::on_document_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew)
{
    Q_UNUSED(documentOld);

    if (acceptDocument(documentNew))
    {
        int index = documentIndexFromGUID(documentNew.strGUID);
        if (-1 == index)
        {
            addDocument(documentNew, true);
        }
        else
        {
            if (CWizDocumentListViewItem* pItem = documentItemAt(index))
            {
                pItem->reload(m_dbMgr.db());
            }
        }

    }
    else
    {
        int index = documentIndexFromGUID(documentNew.strGUID);
        if (-1 != index)
        {
            takeItem(index);
        }
    }
}

void CWizDocumentListView::on_document_deleted(const WIZDOCUMENTDATA& document)
{
    int index = documentIndexFromGUID(document.strGUID);
    if (-1 != index)
    {
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
    if (pItem->document().strKbGUID != abs.strKbGUID)
        return;

    pItem->resetAbstract(abs);
    update(indexFromItem(pItem));
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
            CWizDocument doc(m_dbMgr.db(item->document().strKbGUID), item->document());
            doc.Delete();
        }
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


int CWizDocumentListView::documentIndexFromGUID(const CString& strGUID)
{
    for (int i = 0; i < count(); i++)
    {
        if (CWizDocumentListViewItem *pItem = documentItemAt(i))
        {
            if (pItem->document().strGUID == strGUID)
            {
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

WIZDOCUMENTDATA CWizDocumentListView::documentFromIndex(const QModelIndex &index) const
{
    return documentItemFromIndex(index)->document();
}

WIZABSTRACT CWizDocumentListView::documentAbstractFromIndex(const QModelIndex &index) const
{
    return documentItemFromIndex(index)->abstract(*m_thumbCache);
}

CString CWizDocumentListView::documentTagsFromIndex(const QModelIndex &index) const
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
