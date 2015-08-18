#include "messagelistview.h"

#include <QListWidgetItem>
#include <QScrollBar>
#include <QResizeEvent>
#include <QHBoxLayout>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QPainter>
#include <QMenu>
#include <QList>
#include <QSortFilterProxyModel>
#include <QDebug>
#include <QApplication>
#include <QTextCodec>

#include <extensionsystem/pluginmanager.h>

#include "utils/stylehelper.h"
#include "utils/misc.h"
#include "sync/avatar.h"
#include "sync/asyncapi.h"
#include "widgets/wizImageButton.h"

#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include "share/wizobject.h"
#include "wiznotestyle.h"
#include "widgets/wizScrollBar.h"

#include "wizCategoryView.h"
#include "wizCategoryViewItem.h"
#include "coreplugin/itreeview.h"
#include "utils/misc.h"

namespace WizService {
namespace Internal {

class MessageListViewItem : public QListWidgetItem
{
public:
    explicit MessageListViewItem(const WIZMESSAGEDATA& data):m_data(data), m_specialFocusd(false)
    {

    }

    void setData(const WIZMESSAGEDATA& data) { m_data = data; }
    const WIZMESSAGEDATA& data() const { return m_data; }

    virtual bool operator <(const QListWidgetItem &other) const
    {
        const MessageListViewItem* pOther = dynamic_cast<const MessageListViewItem*>(&other);
        return pOther->m_data.tCreated < m_data.tCreated;
    }

    void drawColorMessageBody(QPainter* p, const QRect& rcMsg, const QFont& f) const
    {
        QString strMsg;
        if (m_data.messageBody.isEmpty())
        {
            strMsg = m_data.title;
            Utils::StyleHelper::drawText(p, rcMsg, strMsg, 2, Qt::AlignVCenter, p->pen().color(), f);
        }
        else
        {
            strMsg = m_data.messageBody;
            strMsg.remove(0, m_data.senderAlias.length() + 1);
            QString strBeforeTitle = strMsg.left(strMsg.indexOf(m_data.title));
            QString strAfterTitle = strMsg.right(strMsg.length() - strMsg.lastIndexOf(m_data.title) - m_data.title.length());
            //
            QColor colorSummary = Qt::black;
            QColor colorTitle("#3998d6");

            QRect rcBeforeTitle = Utils::StyleHelper::drawText(p, rcMsg, strBeforeTitle, 1, Qt::AlignVCenter, p->pen().color(), f, false);
            if (strBeforeTitle.isEmpty() && rcBeforeTitle.height() < rcMsg.height())   //  第一行有剩余空间
            {
                QString strTitle(m_data.title);
                QRect rcTitle1(rcMsg.adjusted(rcBeforeTitle.width() - Utils::StyleHelper::margin(), 0, 0, 0));
                rcTitle1 = Utils::StyleHelper::drawText(p, rcTitle1, strTitle, 1, Qt::AlignVCenter, colorTitle, f, false);

                if (!strTitle.isEmpty())        //继续绘制标题
                {
                    QRect rcTitle2(rcMsg.adjusted(0, rcBeforeTitle.height(), 0, 0));
                    rcTitle2 = Utils::StyleHelper::drawText(p, rcTitle2, strTitle, 1, Qt::AlignVCenter, colorTitle, f);
                    if (strTitle.isEmpty() && rcTitle2.width() < rcMsg.width())
                    {
                        QRect rcAfterTitle(rcMsg.adjusted(rcTitle2.width() - Utils::StyleHelper::margin(), rcBeforeTitle.height(), 0, 0));
                        Utils::StyleHelper::drawText(p, rcAfterTitle, strAfterTitle, 1, Qt::AlignVCenter, colorSummary, f);
                    }
                }
                else if (rcTitle1.right() < rcMsg.right())   //在第一行绘制标题后内容
                {
                    QRect rcAfterTitle1(rcMsg.adjusted(rcBeforeTitle.width() + rcTitle1.width()  - Utils::StyleHelper::margin() * 2, 0, 0, 0));
                    rcAfterTitle1 = Utils::StyleHelper::drawText(p, rcAfterTitle1, strAfterTitle, 1, Qt::AlignVCenter, colorSummary, f, false);

                    if (!strAfterTitle.isEmpty()) {
                        QRect rcLine2(rcMsg.adjusted(0, rcAfterTitle1.height(), 0, 0));
                        rcLine2 = Utils::StyleHelper::drawText(p, rcLine2, strAfterTitle, 1, Qt::AlignVCenter, colorSummary, f);
                    }
                }
                else        //第一行没剩余空间，在第二行绘制标题后内容
                {
                    QRect rcLine2(rcMsg.adjusted(0, rcTitle1.height(), 0, 0));
                    Utils::StyleHelper::drawText(p, rcLine2, strAfterTitle, 1, Qt::AlignVCenter, colorSummary, f);
                }
            }
            else                // 第一行没有剩余空间
            {
                QRect rcBeforeTitle2(rcMsg.adjusted(0, rcBeforeTitle.height(), 0, 0));
                rcBeforeTitle2 = Utils::StyleHelper::drawText(p, rcBeforeTitle2, strBeforeTitle, 1, Qt::AlignVCenter, colorSummary, f);

                if (strBeforeTitle.isEmpty() && rcBeforeTitle2.width() < rcMsg.width())
                {
                    QString strTitle(m_data.title);
                    QRect rcTitle(rcMsg.adjusted(rcBeforeTitle2.width() - Utils::StyleHelper::margin(), rcBeforeTitle.height(), 0, 0));
                    rcTitle = Utils::StyleHelper::drawText(p, rcTitle, strTitle, 1, Qt::AlignVCenter, colorTitle, f);

                    //
                    if (strTitle.isEmpty())
                    {
                        QRect rcAfterTitle(rcMsg.adjusted(rcTitle.width() - Utils::StyleHelper::margin(), rcBeforeTitle.height(), 0, 0));
                        rcAfterTitle = Utils::StyleHelper::drawText(p, rcAfterTitle, strAfterTitle, 1, Qt::AlignVCenter, colorTitle, f);
                    }
                }

            }
        }
    }

    QString descriptionOfMessageType(int type) const
    {
        switch (type) {
        case WIZ_USER_MSG_TYPE_CALLED_IN_TITLE:
            return QObject::tr("@ you in note title");
            break;
        case WIZ_USER_MSG_TYPE_MODIFIED:
            return QObject::tr("Modified your note");
            break;
        case WIZ_USER_MSG_TYPE_COMMENT:
            return QObject::tr("Comment your note");
            break;
        case WIZ_USER_MSG_TYPE_CALLED_IN_COMMENT:
            return QObject::tr("@ you in note comment");
            break;
        case WIZ_USER_MSG_TYPE_COMMENT_REPLY:
            return QObject::tr("Reply your comment");
            break;
        }
        return QObject::tr("Unknown meesage type");
    }

    void paint(QPainter* p, const QStyleOptionViewItemV4* vopt) const
    {
        int nMargin = Utils::StyleHelper::margin();
        QRect rcd = vopt->rect.adjusted(nMargin, nMargin, -nMargin, -nMargin);

        QPixmap pmAvatar;
        WizService::AvatarHost::avatar(m_data.senderId, &pmAvatar);
        QRect rectAvatar = Utils::StyleHelper::drawAvatar(p, rcd, pmAvatar);
        int nAvatarRightMargin = 4;
        rcd.setLeft(rectAvatar.right() + nAvatarRightMargin);

        QFont f;
        int nHeight = Utils::StyleHelper::fontNormal(f);

        p->save();
        bool bItemActive = vopt->state.testFlag(QStyle::State_Selected) && vopt->state.testFlag(QStyle::State_HasFocus);
        if (bItemActive)
        {
            p->setPen("#FFFFFF");
        }
        QString strSender = m_data.senderAlias.isEmpty() ? m_data.senderId : m_data.senderAlias;
        QRect rectSender = Utils::StyleHelper::drawText(p, rcd, strSender, 1, Qt::AlignVCenter, p->pen().color(), f);

        QString strType = descriptionOfMessageType(m_data.nMessageType);
        QRect rectType = rcd;
        rectType.setLeft(rectSender.right() + 2);
        Utils::StyleHelper::drawText(p, rectType, strType, 1, Qt::AlignVCenter, QColor(bItemActive? "#ffffff" : "#999999"), f);
        rcd.setTop(rectSender.bottom());

        QRect rcBottom(rcd);
        rcBottom.setTop(rcd.bottom() - nHeight);
        QString strTime = Utils::Misc::time2humanReadable(m_data.tCreated);
        QRect rcTime = Utils::StyleHelper::drawText(p, rcBottom, strTime, 1, Qt::AlignRight | Qt::AlignVCenter, QColor(bItemActive? "#ffffff" : "#999999"), f);

        QSize sz(rcd.width() - nMargin * 2, rcd.height() - rcTime.height() - nMargin);
        QPolygonF po = Utils::StyleHelper::bubbleFromSize(sz, 4);
        po.translate(rcd.left() + nMargin, rcd.top());

        if (vopt->state.testFlag(QStyle::State_Selected)) {
            p->save();
            p->setBrush(Qt::NoBrush);
            if (vopt->state.testFlag(QStyle::State_HasFocus)) {
                p->setPen("#2a7aaf");
            } else {
                p->setPen("#a9b6bd");
            }
            p->drawPolygon(po);
            p->restore();
        } else {
            p->save();
            if (!m_data.nReadStatus) {
#ifdef Q_OS_MAC
                p->setBrush(QBrush("#CBEBFA"));
#else
                p->setBrush(QBrush("#dcdcdc"));
#endif
            }
            p->setPen("#dcdcdc");
            p->drawPolygon(po);
            p->restore();
        }

        QRect rcTitle(rcd.x() + 10, rcd.y() + 15, sz.width() - 6, sz.height() - 15);
//        drawColorMessageBody(p, rcMsg, f);
        QString strTitle(m_data.title);
        QRect rcFirstLine = Utils::StyleHelper::drawText(p, rcTitle, strTitle, 1, Qt::AlignLeft| Qt::AlignVCenter, p->pen().color(), f, false);
        if (!strTitle.isEmpty())
        {
            rcTitle.setTop(rcFirstLine.bottom());
            Utils::StyleHelper::drawText(p, rcTitle, strTitle, 1, Qt::AlignLeft| Qt::AlignVCenter, p->pen().color(), f);
        }

        p->restore();
    }

    bool specialFocusd() const { return m_specialFocusd; }
    void setSpecialFocused(bool specialFocusd) { m_specialFocusd = specialFocusd; }

private:
    WIZMESSAGEDATA m_data;
    bool m_specialFocusd;
};

// Message actions
#define WIZACTION_LIST_MESSAGE_MARK_READ    QObject::tr("Mark as read")
#define WIZACTION_LIST_MESSAGE_DELETE       QObject::tr("Delete Message(s)")
#define WIZACTION_LIST_MESSAGE_LOCATE       QObject::tr("Locate Message")

MessageListView::MessageListView(CWizDatabaseManager& dbMgr, QWidget *parent)
    : QListWidget(parent)
    , m_dbMgr(dbMgr)
    , m_pCurrentItem(NULL)
    , m_api(NULL)
{
    setMinimumWidth(150);
    setFrameStyle(QFrame::NoFrame);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QString strSkinName = "default"; // FIXME
    setStyle(::WizGetStyle(strSkinName));
    QPalette pal = palette();
    pal.setColor(QPalette::Base, Utils::StyleHelper::listViewBackground());
    setPalette(pal);

    setCursor(QCursor(Qt::ArrowCursor));

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

    // init
    m_timerRead.setInterval(100);
    m_timerRead.setSingleShot(true);
    connect(&m_timerRead, SIGNAL(timeout()), SLOT(onReadTimeout()));

    m_timerTriggerSync.setInterval(5000);
    m_timerTriggerSync.setSingleShot(true);
    connect(&m_timerTriggerSync, SIGNAL(timeout()), SLOT(onSyncTimeout()));

    m_menu = new QMenu(this);
    m_menu->addAction(WIZACTION_LIST_MESSAGE_MARK_READ, this,
                      SLOT(on_action_message_mark_read()));
    m_menu->addAction(WIZACTION_LIST_MESSAGE_LOCATE, this,
                      SLOT(on_action_message_locate()));
    m_menu->addAction(tr("View in Separate Window"), this,
                      SLOT(on_action_message_viewInSeparateWindow()));
    m_menu->addSeparator();
    m_menu->addAction(WIZACTION_LIST_MESSAGE_DELETE, this,
                      SLOT(on_action_message_delete()));

    connect(m_menu, SIGNAL(aboutToHide()), SLOT(clearRightMenuFocus()));

    connect(this, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            SLOT(onCurrentItemChanged(QListWidgetItem*,QListWidgetItem*)));

    connect(&m_dbMgr.db(),
            SIGNAL(messageCreated(const WIZMESSAGEDATA&)),
            SLOT(on_message_created(const WIZMESSAGEDATA&)));

    connect(&m_dbMgr.db(),
            SIGNAL(messageModified(const WIZMESSAGEDATA&, const WIZMESSAGEDATA&)),
            SLOT(on_message_modified(const WIZMESSAGEDATA&, const WIZMESSAGEDATA&)));

    connect(&m_dbMgr.db(),
            SIGNAL(messageDeleted(const WIZMESSAGEDATA&)),
            SLOT(on_message_deleted(const WIZMESSAGEDATA&)));

    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            SLOT(on_itemDoubleClicked(QListWidgetItem*)));

    connect(AvatarHost::instance(), SIGNAL(loaded(const QString&)), SLOT(onAvatarLoaded(const QString&)));    
}

void MessageListView::resizeEvent(QResizeEvent* event)
{
#ifdef WIZNOTE_CUSTOM_SCROLLBAR
    // reset scrollbar position
    m_vScroll->resize(m_vScroll->sizeHint().width(), event->size().height());
    m_vScroll->move(event->size().width() - m_vScroll->sizeHint().width(), 0);
#endif

    QListWidget::resizeEvent(event);
}

void MessageListView::contextMenuEvent(QContextMenuEvent* event)
{
    MessageListViewItem* pItem = dynamic_cast<MessageListViewItem*>(itemAt(event->pos()));

    if (!pItem)
        return;

    m_menu->popup(event->globalPos());
}

void MessageListView::setMessages(const CWizMessageDataArray& arrayMsg)
{
    clear();
    verticalScrollBar()->setValue(0);

    addMessages(arrayMsg);
}

void MessageListView::addMessages(const CWizMessageDataArray& arrayMessage)
{
    CWizMessageDataArray::const_iterator it;
    for (it = arrayMessage.begin(); it != arrayMessage.end(); it++) {
        addMessage(*it, false);
    }

    sortItems();
}

void MessageListView::addMessage(const WIZMESSAGEDATA& msg, bool sort)
{
    if (msg.nDeleteStatus == 1) {
//        qDebug() << "[Message]Deleted message would not be displayed : " << msg.title;
        return;
    }

    MessageListViewItem* pItem = new MessageListViewItem(msg);

    int nHeight = Utils::StyleHelper::thumbnailHeight() + Utils::StyleHelper::margin() * 5;
    pItem->setSizeHint(QSize(sizeHint().width(), nHeight));

    addItem(pItem);

    if (sort) {
        sortItems();
    }

    Q_EMIT sizeChanged(count());
}

int MessageListView::rowFromId(qint64 nId) const
{
    for (int i = 0; i < count(); i++) {
        if (MessageListViewItem* pItem = messageItem(i)) {
            if (pItem->data().nId == nId) {
                return i;
            }
        }
    }

    return -1;
}

void MessageListView::specialFocusedMessages(QList<WIZMESSAGEDATA>& arrayMsg)
{
    foreach(MessageListViewItem* item, m_rightButtonFocusedItems) {
        arrayMsg.push_back(item->data());
    }
}

void MessageListView::selectMessage(qint64 nId)
{
    for (int i = 0; i < count(); i++) {
        if (MessageListViewItem* pItem = messageItem(i)) {
            if (pItem->data().nId == nId) {
                setCurrentItem(pItem, QItemSelectionModel::ClearAndSelect);
            }
        }
    }
}

void MessageListView::selectedMessages(QList<WIZMESSAGEDATA>& arrayMsg)
{
    QList<QListWidgetItem*> items = selectedItems();

    foreach(QListWidgetItem* item, items) {
        MessageListViewItem* pItem = dynamic_cast<MessageListViewItem*>(item);
        arrayMsg.push_back(pItem->data());
    }
}

MessageListViewItem* MessageListView::messageItem(int row) const
{
    MessageListViewItem* pItem = dynamic_cast<MessageListViewItem*>(item(row));
    Q_ASSERT(pItem);
    return pItem;
}

MessageListViewItem* MessageListView::messageItem(const QModelIndex& index) const
{
    MessageListViewItem* pItem = dynamic_cast<MessageListViewItem*>(itemFromIndex(index));
    Q_ASSERT(pItem);
    return pItem;
}

const WIZMESSAGEDATA& MessageListView::messageFromIndex(const QModelIndex& index) const
{
    MessageListViewItem* pItem = dynamic_cast<MessageListViewItem*>(itemFromIndex(index));
    Q_ASSERT(pItem);
    return pItem->data();
}

void MessageListView::drawItem(QPainter* p, const QStyleOptionViewItemV4* vopt) const
{
    Utils::StyleHelper::drawListViewItemSeperator(p, vopt->rect);
    MessageListViewItem* pItem = messageItem(vopt->index);
    if (!(vopt->state & QStyle::State_Selected) && pItem->specialFocusd())
    {
        Utils::StyleHelper::drawListViewItemBackground(p, vopt->rect, false, true);
    }
    else
    {
        Utils::StyleHelper::drawListViewItemBackground(p, vopt->rect, hasFocus(), vopt->state & QStyle::State_Selected);
    }
    pItem->paint(p, vopt);
}

void MessageListView::markAllMessagesReaded()
{
    for (int i = 0; i < count(); i++) {
        MessageListViewItem* pItem = messageItem(i);
        if (!pItem->data().nReadStatus) {
            m_dbMgr.db().setMessageReadStatus(pItem->data());
            m_readList.push_back(pItem->data().nId);
        }
    }
    m_timerTriggerSync.start();
}

void MessageListView::on_uploadReadStatus_finished(const QString& ids)
{
    QStringList idList = ids.split(',');
    CWizDatabase& db = m_dbMgr.db();
    for (QString id : idList)
    {
#ifdef QT_DEBUG
        Q_ASSERT(!id.isEmpty() && id != "0");
#endif
        if (id.isEmpty() || id == "0")
            continue;

        WIZMESSAGEDATA msg;
        db.messageFromId(id.toLong(), msg);
        msg.nLocalChanged = msg.nLocalChanged & ~WIZMESSAGEDATA::localChanged_Read;
        db.updateMessage(msg);
    }
}

void MessageListView::on_uploadDeleteStatus_finished(const QString& ids)
{
    QStringList idList = ids.split(',');
    CWizDatabase& db = m_dbMgr.db();
    for (QString id : idList)
    {
#ifdef QT_DEBUG
        Q_ASSERT(!id.isEmpty() && id != "0");
#endif
        if (id.isEmpty() || id == "0")
            continue;

        WIZMESSAGEDATA msg;
        db.messageFromId(id.toLong(), msg);
        msg.nLocalChanged = msg.nLocalChanged & ~WIZMESSAGEDATA::localChanged_Delete;
        db.updateMessage(msg);
    }
}

void MessageListView::onAvatarLoaded(const QString& strUserId)
{
    for (int i = 0; i < count(); i++) {
        MessageListViewItem* pItem = messageItem(i);
        if (pItem->data().senderId == strUserId) {
            update(indexFromItem(pItem));
        }
    }
}

void MessageListView::onCurrentItemChanged(QListWidgetItem* current,QListWidgetItem* previous)
{
    Q_UNUSED(previous);

    if (current) {
        MessageListViewItem* pItem = dynamic_cast<MessageListViewItem*>(current);
        if (pItem && !pItem->data().nReadStatus) {
            m_pCurrentItem = pItem;
            m_timerRead.start();
        }
    }
}

void MessageListView::onReadTimeout()
{
    if (m_pCurrentItem && !m_pCurrentItem->data().nReadStatus) {
        m_dbMgr.db().setMessageReadStatus(m_pCurrentItem->data());
        m_readList.push_back(m_pCurrentItem->data().nId);
        m_timerTriggerSync.start();
    }
}

void MessageListView::onSyncTimeout()
{
    if (!m_api) {
        m_api = new WizService::AsyncApi(this);

        connect(m_api, SIGNAL(uploadMessageReadStatusFinished(QString)),
                SLOT(on_uploadReadStatus_finished(QString)));
        connect(m_api, SIGNAL(uploadMessageDeleteStatusFinished(QString)),
                SLOT(on_uploadDeleteStatus_finished(QString)));
    }

    if (m_readList.count() > 0)
    {
        QString ids;
        for (int i = 0; i < m_readList.size(); i++) {
            ids += QString::number(m_readList.at(i));

            if (i != m_readList.size() - 1) {
                ids += ",";
            }
        }

        if (ids.isEmpty())
            return;

        qDebug() << "upload messages read status:" << ids;
        m_api->setMessageReadStatus(ids, 1);

        m_readList.clear();
    }

    if (m_deleteList.count() > 0)
    {
        QString ids;
        for (int i = 0; i < m_deleteList.size(); i++) {
            ids += QString::number(m_deleteList.at(i));

            if (i != m_deleteList.size() - 1) {
                ids += ",";
            }
        }

        if (ids.isEmpty())
            return;

        qDebug() << "upload messages delete status:" << ids;
        m_api->setMessageDeleteStatus(ids, 1);

        m_deleteList.clear();
    }
}

void MessageListView::updateTreeItem()
{
    CWizCategoryView* tree = ExtensionSystem::PluginManager::getObject<CWizCategoryView>();
    if (tree) {
        CWizCategoryViewItemBase* pBase = tree->findAllMessagesItem();
        if (!pBase)
            return;

        CWizCategoryViewMessageItem* pItem = dynamic_cast<CWizCategoryViewMessageItem*>(pBase);
        Q_ASSERT(pItem);

        int nUnread = m_dbMgr.db().getUnreadMessageCount();
        pItem->setUnreadCount(nUnread);
    }
}



void MessageListView::on_action_message_mark_read()
{
    QList<WIZMESSAGEDATA> arrayMsg;
    specialFocusedMessages(arrayMsg);

    for (int i = 0; i < arrayMsg.size(); i++) {
        m_readList.append(arrayMsg.at(i).nId);
        m_dbMgr.db().setMessageReadStatus(arrayMsg.at(i));
    }


    updateTreeItem();
    m_timerTriggerSync.start();
}

void MessageListView::on_action_message_delete()
{
    QList<WIZMESSAGEDATA> arrayMsg;
    specialFocusedMessages(arrayMsg);

    for (int i = 0; i < arrayMsg.size(); i++) {
//        arrayMsg[i].nDeleteStatus = 1;
//        arrayMsg[i].nLocalChanged = arrayMsg.at(i).nLocalChanged | WIZMESSAGEDATA::localChanged_Delete;
//        m_dbMgr.db().modifyMessageEx(arrayMsg.at(i));
        m_dbMgr.db().setMessageDeleteStatus(arrayMsg.at(i));
        m_deleteList.append(arrayMsg.at(i).nId);
    }

    updateTreeItem();
    m_timerTriggerSync.start();
}

void MessageListView::on_action_message_locate()
{
    if (m_rightButtonFocusedItems.isEmpty())
        return;

    MessageListViewItem* pItem = m_rightButtonFocusedItems.first();
    if (pItem)
    {
        emit loacteDocumetRequest(pItem->data().kbGUID, pItem->data().documentGUID);
    }
}

void MessageListView::on_action_message_viewInSeparateWindow()
{
    if (m_rightButtonFocusedItems.isEmpty())
        return;

    MessageListViewItem* pItem = m_rightButtonFocusedItems.first();
    if (pItem)
    {
        WIZMESSAGEDATA message = pItem->data();
        WIZDOCUMENTDATA doc;
        CWizDatabase& db = m_dbMgr.db(message.kbGUID);
        if (!db.DocumentFromGUID(message.documentGUID, doc))
            return;

        if (db.IsDocumentDownloaded(doc.strGUID))
        {
            emit viewNoteInSparateWindowRequest(doc);
        }
    }
}

void MessageListView::on_message_created(const WIZMESSAGEDATA& msg)
{
    if (rowFromId(msg.nId) == -1) {
        addMessage(msg, true);
    }

    updateTreeItem();
}

void MessageListView::on_message_modified(const WIZMESSAGEDATA& oldMsg,
                                          const WIZMESSAGEDATA& newMsg)
{
    Q_UNUSED(oldMsg);
    int i = rowFromId(newMsg.nId);
    if (i != -1) {
        if (MessageListViewItem* pItem = messageItem(i)) {
            pItem->setData(newMsg);
            update(indexFromItem(pItem));
            //
            if (newMsg.nDeleteStatus == 1) {
                takeItem(i);
                delete pItem;
            }
        }
    }

    updateTreeItem();
}

void MessageListView::on_message_deleted(const WIZMESSAGEDATA& msg)
{
    int i = rowFromId(msg.nId);
    if (i != -1) {
        QListWidgetItem* item = takeItem(i);
        if (item)
            delete item;
    }

    updateTreeItem();
}

void MessageListView::on_itemDoubleClicked(QListWidgetItem* item)
{
    MessageListViewItem* pItem = dynamic_cast<MessageListViewItem*>(item);
    if (pItem)
    {
        WIZMESSAGEDATA message = pItem->data();
        WIZDOCUMENTDATA doc;
        CWizDatabase& db = m_dbMgr.db(message.kbGUID);
        if (!db.DocumentFromGUID(message.documentGUID, doc))
            return;

        if (db.IsDocumentDownloaded(doc.strGUID))
        {
            emit viewNoteInSparateWindowRequest(doc);
        }
    }
}

void MessageListView::clearRightMenuFocus()
{
    foreach (MessageListViewItem *item, m_rightButtonFocusedItems)
    {
        item->setSpecialFocused(false);
    }
}

void MessageListView::wheelEvent(QWheelEvent* event)
{
    int delta = event->delta();
    delta = delta / 3;
    QWheelEvent* newEvent = new QWheelEvent(event->pos(),
                                          event->globalPos(),
                                          delta,
                                          event->buttons(),
                                          event->modifiers(),
                                          event->orientation());
    QListWidget::wheelEvent(newEvent);
}

void MessageListView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        QListWidget::mousePressEvent(event);
    }
    else
    {
        MessageListViewItem* pItem = dynamic_cast<MessageListViewItem*>(itemAt(event->pos()));
        if (!pItem)
            return;

        m_rightButtonFocusedItems.clear();
        // if selectdItems contains clicked item use all selectedItems as special focused item.
        if (selectedItems().contains(pItem))
        {
            foreach (QListWidgetItem* lsItem, selectedItems())
            {
                pItem = dynamic_cast<MessageListViewItem*>(lsItem);
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

        m_menu->popup(event->globalPos());
    }
}

WizMessageSelector::WizMessageSelector(QWidget* parent)
    : QComboBox(parent)
{   
}

void WizMessageSelector::showPopup()
{
    setEditable(true);
    QComboBox::showPopup();

    QWidget* popup = findChild<QFrame*>();
    int topMargin = 4;
    QPoint pos(0, height() + topMargin * 2);
    pos = parentWidget()->mapToGlobal(pos);
    popup->move(pos);
}

//bool WizMessageSelector::event(QEvent* event)
//{
//    qDebug() << "event type  : " << event << "  cursor shape ; " << cursor().shape();
//    return QComboBox::event(event);
//}


void WizMessageSelector::focusOutEvent(QFocusEvent* event)
{
    QComboBox::focusOutEvent(event);
}

void WizMessageSelector::focusInEvent(QFocusEvent* event)
{
    //NOTE:为了使用弹出列表的滚动条，将combobox设置为可编辑。此处移除focusin来取消可编辑
//    QComboBox::focusInEvent(event);
    event->accept();
    setCursor(QCursor(Qt::ArrowCursor));
    clearFocus();
}

//void WizMessageSelector::mouseMoveEvent(QMouseEvent* event)
//{
//    QComboBox::mouseMoveEvent(event);
//    qDebug() << "mousr move event ; " << cursor().shape();
//    setCursor(QCursor(Qt::ArrowCursor));
//}

//void WizMessageSelector::enterEvent(QEvent* event)
//{
//    QComboBox::enterEvent(event);
//    setCursor(QCursor(Qt::ArrowCursor));
//}

WizMessageListTitleBar::WizMessageListTitleBar(CWizDatabaseManager& dbMgr, QWidget* parent)
    : QWidget(parent)
    , m_dbMgr(dbMgr)
{
    setFixedHeight(Utils::StyleHelper::titleEditorHeight());
    QHBoxLayout* layoutActions = new QHBoxLayout();
    layoutActions->setContentsMargins(2, 0, 16, 0);
    layoutActions->setSpacing(0);
    setLayout(layoutActions);

    m_msgSelector = new WizMessageSelector(this);
//    m_msgSelector->setMinimumHeight(22);
    m_msgSelector->setFixedHeight(22);
    m_msgSelector->setFixedWidth(122);
    m_msgSelector->setIconSize(QSize(20, 20));
    m_msgSelector->setEditable(true);

    QString strDropArrow = Utils::StyleHelper::skinResourceFileName("arrow");    
    m_msgSelector->setStyleSheet(QString("QComboBox{background-color: white;selection-color: #000000; selection-background-color: transparent;}"
                                             "QComboBox{border: 0px;padding: 1px 1px 1px 3px;}"                                         
                                             "QComboBox::drop-down {width: 15px;border:0px;subcontrol-origin: padding;subcontrol-position: top right;width: 15px;}"
                                             "QComboBox::down-arrow {image:url(%1);}"
                                         "QComboBox QListView QScrollBar {\
                                             background: transparent;\
                                             width: 10px;\
                                         }\
                                         QComboBox QListView QScrollBar::handle {\
                                             background: rgba(85, 85, 85, 200);\
                                             border-radius: 4px;\
                                             min-height: 30px;\
                                         }\
                                         QComboBox QListView QScrollBar::handle:vertical {\
                                             margin: 0px 2px 0px 0px;\
                                         }\
                                         QComboBox QListView QScrollBar::handle:horizontal {\
                                             margin: 0px 0px 2px 0px;\
                                         }\
                                         QComboBox QListView QScrollBar::add-page, QScrollBar::sub-page {\
                                             background: transparent;\
                                         }\
                                         QComboBox QListView QScrollBar::up-arrow, QScrollBar::down-arrow, QScrollBar::left-arrow, QScrollBar::right-arrow {\
                                             background: transparent;\
                                         }\
                                         QComboBox QListView QScrollBar::add-line, QScrollBar::sub-line {\
                                             height: 0px;\
                                             width: 0px;\
                                         }"\
                                             "QComboBox QListView{background-color:white;border:0px; min-width:180px;}"
                                             "QComboBox QAbstractItemView::item {min-height:24px; min-width:180px; max-width:180px; margin:2px 0px 2px 0px;background:transparent;}"
                                             /*"QComboBox::item:selected {background:transparent;color:#ffffff;}"*/).arg(strDropArrow));

    m_msgSelector->setMaxVisibleItems(15);
    WizMessageSelectorItemDelegate* itemDelegate = new WizMessageSelectorItemDelegate();
    m_msgSelector->setItemDelegate(itemDelegate);    

    initUserList();

    layoutActions->addWidget(m_msgSelector);
    connect(m_msgSelector, SIGNAL(currentIndexChanged(int)),
            SLOT(on_selector_indexChanged(int)));

    layoutActions->addStretch();
    m_msgListHintLabel = new QLabel(this);
    m_msgListHintLabel->setText(tr("Unread messages"));
    m_msgListHintLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    m_msgListHintLabel->setStyleSheet("color: #787878;padding-top:4px;margin-right:10px;");
    layoutActions->addWidget(m_msgListHintLabel);

    m_msgListMarkAllBtn = new wizImageButton(this);
    QIcon btnIcon = ::WizLoadSkinIcon(Utils::StyleHelper::themeName(), "actionMarkMessagesRead");
    m_msgListMarkAllBtn->setIcon(btnIcon);
    m_msgListMarkAllBtn->setFixedSize(QSize(16, 16));
    m_msgListMarkAllBtn->setToolTip(tr("Mark all messages read"));
    connect(m_msgListMarkAllBtn, SIGNAL(clicked()), SIGNAL(markAllMessageRead_request()));
    layoutActions->addWidget(m_msgListMarkAllBtn);

    connect(&m_dbMgr, SIGNAL(messageCreated(WIZMESSAGEDATA)),
            SLOT(on_message_created(WIZMESSAGEDATA)));
}

void WizMessageListTitleBar::setUnreadMode(bool unread)
{
    m_msgListHintLabel->setText(unread ? tr("Unread messages") : tr("All messages"));
    m_msgListMarkAllBtn->setVisible(unread);
    if (unread)
    {
        layout()->setContentsMargins(2, 0, 16, 0);
        m_msgListHintLabel->setStyleSheet("color: #787878;padding-top:4px;margin-right:10px;");
    }
    else
    {
        layout()->setContentsMargins(2, 0, 0, 0);
        m_msgListHintLabel->setStyleSheet("color: #787878;padding-top:4px;margin-right:0px;");
    }
}

bool WizMessageListTitleBar::isUnreadMode() const
{
    return m_msgListMarkAllBtn->isVisible();
}

void WizMessageListTitleBar::setSelectorIndex(int index)
{
    m_msgSelector->blockSignals(true);
    m_msgSelector->setCurrentIndex(index);
    m_msgSelector->blockSignals(false);
}

QString WizMessageListTitleBar::selectorItemData(int index) const
{
    return m_msgSelector->itemData(index).toString();
}

void WizMessageListTitleBar::on_message_created(const WIZMESSAGEDATA& msg)
{

    //FIXME:在添加新的项目到列表中时存在icon丢失的问题，暂不提供根据新消息添加用户的功能
//    if (msg.senderGUID.isEmpty())
//        return;

//    if (m_msgSelector->findData(msg.senderGUID, Qt::UserRole) == -1)
//    {
//        addUserToSelector(msg.senderGUID);
//        m_msgSelector->model()->sort(0);
//    }


    // update message list
    messageSelector_indexChanged(m_msgSelector->currentIndex());
}

void WizMessageListTitleBar::on_selector_indexChanged(int index)
{
    QString text = m_msgSelector->currentText();
    QFont f;
    QFontMetrics fm(f);
    const int elidedTextWidth = 78;
    QString elidedText = fm.elidedText(text, Qt::ElideRight, elidedTextWidth);
    m_msgSelector->setEditText(elidedText);
    emit messageSelector_indexChanged(index);
}

void WizMessageListTitleBar::addUserToSelector(const QString& userGUID)
{
    CWizBizUserDataArray arrayUser;
    if (!m_dbMgr.db().userFromGUID(userGUID, arrayUser))
        return;

    QSet<QString> userSet;
    QString strUserId;
    for (WIZBIZUSER user : arrayUser)
    {
        userSet.insert(user.alias);
        strUserId = user.userId;
    }
    QStringList userList(userSet.toList());
    QString strText = userList.join(";");
//    qDebug() << "add user to selector , guid : " << userGUID << "  alias : " << strText << "  user id ; " << strUserId;
    QPixmap pix;
    WizService::AvatarHost::load(strUserId);
    WizService::AvatarHost::avatar(strUserId, &pix);
    QIcon icon(pix);
    m_msgSelector->addItem(icon, strText, userGUID);
}

void WizMessageListTitleBar::initUserList()
{
    CWizStdStringArray arraySender;
    CWizDatabase& db = m_dbMgr.db();
    db.getAllMessageSenders(arraySender);
    for (auto sender : arraySender)
    {
        addUserToSelector(sender);
    }

    static bool once = true;
    if (once)
    {
        WizSortFilterProxyModel* proxy = new WizSortFilterProxyModel(m_msgSelector);
        proxy->setSourceModel(m_msgSelector->model());                            // <--
        m_msgSelector->model()->setParent(proxy);                                 // <--
        m_msgSelector->setModel(proxy);
        once = false;
    }
    //
    QPixmap pix(Utils::StyleHelper::skinResourceFileName("avatar_all"));
    QIcon icon(pix);
//    m_msgSelector->insertItem(0, icon, tr("All members"));
    m_msgSelector->addItem(icon, tr("All members"), "");

    m_msgSelector->model()->sort(0);
    m_msgSelector->setCurrentIndex(0);
}

WizMessageSelectorItemDelegate::WizMessageSelectorItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void WizMessageSelectorItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
//    QStyleOptionViewItem opt(option);
    QStyleOptionViewItemV4 opt = option;
    initStyleOption(&opt, index);
    //NOTE:QCombobox 的项目的背景css颜色样式无效，需要通过绘制实现。
    if (option.state & QStyle::State_Selected)
    {
        opt.palette.setColor(QPalette::Text, QColor(Qt::black));
        opt.palette.setColor(QPalette::WindowText, QColor(Qt::black));
        opt.palette.setColor(QPalette::HighlightedText, QColor(Qt::black));
    }
    if (option.state & QStyle::State_MouseOver)
    {
        painter->fillRect(option.rect, QBrush(QColor("#3397db")));
        opt.palette.setColor(QPalette::Text, QColor(Qt::white));
        opt.palette.setColor(QPalette::WindowText, QColor(Qt::white));
        opt.palette.setColor(QPalette::HighlightedText, QColor(Qt::white));
    }    

    painter->save();

    painter->setPen(QPen(opt.palette.color(QPalette::Text)));
    painter->setFont(opt.font);

    const int nMargin = 5;
    const int nIconSize = 20;
    QRect rcItem = opt.rect;
    QRect rcIcon(rcItem.left() + nMargin, rcItem.top() + (rcItem.height() - nIconSize) / 2, nIconSize , nIconSize);
    painter->drawPixmap(rcIcon, opt.icon.pixmap(nIconSize, nIconSize));
    QRect rcText(rcIcon.right() + nMargin + 1, rcItem.top(), rcItem.width() - rcIcon.width() - nMargin * 2, rcItem.height());
    painter->drawText(rcText, Qt::AlignLeft |Qt::AlignVCenter, opt.text);

    painter->restore();
}

WizSortFilterProxyModel::WizSortFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{

}

bool WizSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    QString leftGuid = sourceModel()->data(left, Qt::UserRole).toString();
    QString rightGuid = sourceModel()->data(right, Qt::UserRole).toString();
    if (leftGuid.isEmpty())
        return true;
    if (rightGuid.isEmpty())
        return false;

    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);

    QString leftString  = leftData.toString();
    QString rightString = rightData.toString();

//    return QString::localeAwareCompare(leftString, rightString) < 0;


    static bool isSimpChinese = Utils::Misc::isSimpChinese();
    if (isSimpChinese)
    {
        if (QTextCodec* pCodec = QTextCodec::codecForName("GBK"))
        {
            QByteArray arrThis = pCodec->fromUnicode(leftString);
            QByteArray arrOther = pCodec->fromUnicode(rightString);
            //
            std::string strThisA(arrThis.data(), arrThis.size());
            std::string strOtherA(arrOther.data(), arrOther.size());
            //
            return strThisA.compare(strOtherA.c_str()) < 0;
        }
    }
    //
    return leftString.compare(rightString) < 0;

}


} // namespace Internal
} // namespace WizService
