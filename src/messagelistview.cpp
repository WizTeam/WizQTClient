#include "messagelistview.h"

#include <QListWidgetItem>
#include <QScrollBar>
#include <QResizeEvent>
#include <QPainter>
#include <QMenu>
#include <QList>
#include <QDebug>

#include <extensionsystem/pluginmanager.h>

#include "utils/stylehelper.h"
#include "utils/misc.h"
#include "sync/avatar.h"
#include "sync/asyncapi.h"

#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include "share/wizobject.h"
#include "wiznotestyle.h"
#include "widgets/wizScrollBar.h"

#include "wizCategoryView.h"
#include "wizCategoryViewItem.h"
#include "coreplugin/itreeview.h"

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
        if (vopt->state.testFlag(QStyle::State_Selected) && vopt->state.testFlag(QStyle::State_HasFocus))
        {
            p->setPen("#FFFFFF");
        }
        QString strSender = m_data.senderAlias.isEmpty() ? m_data.senderId : m_data.senderAlias;
        QRect rectSender = Utils::StyleHelper::drawText(p, rcd, strSender, 1, Qt::AlignVCenter, p->pen().color(), f);
        rcd.setTop(rectSender.bottom());

        QRect rcBottom(rcd);
        rcBottom.setTop(rcd.bottom() - nHeight);
        QString strTime = Utils::Misc::time2humanReadable(m_data.tCreated);
        QRect rcTime = Utils::StyleHelper::drawText(p, rcBottom, strTime, 1, Qt::AlignRight | Qt::AlignVCenter, p->pen().color(), f);

        QSize sz(rcd.width() - nMargin * 2, rcd.height() - rcTime.height() - nMargin);
        QPolygon po = Utils::StyleHelper::bubbleFromSize(sz, 4);
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

        QRect rcMsg(rcd.x() + nMargin, rcd.y() + 4 + nMargin, sz.width(), sz.height());
        QString strMsg = m_data.title.isEmpty() ? " " : m_data.title;
        rcMsg = Utils::StyleHelper::drawText(p, rcMsg, strMsg, 2, Qt::AlignVCenter, p->pen().color(), f);
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

MessageListView::MessageListView(QWidget *parent)
    : QListWidget(parent)
    , m_pCurrentItem(NULL)
    , m_api(NULL)
{
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
    m_menu->addSeparator();
    m_menu->addAction(WIZACTION_LIST_MESSAGE_DELETE, this,
                      SLOT(on_action_message_delete()));

    connect(m_menu, SIGNAL(aboutToHide()), SLOT(clearRightMenuFocus()));

    connect(this, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            SLOT(onCurrentItemChanged(QListWidgetItem*,QListWidgetItem*)));

    connect(&CWizDatabaseManager::instance()->db(),
            SIGNAL(messageCreated(const WIZMESSAGEDATA&)),
            SLOT(on_message_created(const WIZMESSAGEDATA&)));

    connect(&CWizDatabaseManager::instance()->db(),
            SIGNAL(messageModified(const WIZMESSAGEDATA&, const WIZMESSAGEDATA&)),
            SLOT(on_message_modified(const WIZMESSAGEDATA&, const WIZMESSAGEDATA&)));

    connect(&CWizDatabaseManager::instance()->db(),
            SIGNAL(messageDeleted(const WIZMESSAGEDATA&)),
            SLOT(on_message_deleted(const WIZMESSAGEDATA&)));

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
    MessageListViewItem* pItem = new MessageListViewItem(msg);

    int nHeight = Utils::StyleHelper::thumbnailHeight() + Utils::StyleHelper::margin() * 2;
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
            CWizDatabaseManager::instance()->db().setMessageReadStatus(pItem->data(), 1);
            m_lsIds.push_back(pItem->data().nId);
        }
    }
    m_timerTriggerSync.start();
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
        CWizDatabaseManager::instance()->db().setMessageReadStatus(m_pCurrentItem->data(), 1);
        m_lsIds.push_back(m_pCurrentItem->data().nId);
        m_timerTriggerSync.start();
    }
}

void MessageListView::onSyncTimeout()
{
    if (!m_api) {
        m_api = new WizService::AsyncApi(this);
    }

    QString ids;
    for (int i = 0; i < m_lsIds.size(); i++) {
        ids += QString::number(m_lsIds.at(i));

        if (i != m_lsIds.size() - 1) {
            ids += ",";
        }
    }

    if (ids.isEmpty())
        return;

    qDebug() << "upload messages read status:" << ids;

    m_api->setMessageStatus(ids, 1);

    m_lsIds.clear();
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

        int nUnread = CWizDatabaseManager::instance()->db().getUnreadMessageCount();
        pItem->setUnreadCount(nUnread);
    }
}

void MessageListView::on_action_message_mark_read()
{
    QList<WIZMESSAGEDATA> arrayMsg;
    specialFocusedMessages(arrayMsg);

    CWizMessageDataArray arrayMessage;
    for (int i = 0; i < arrayMsg.size(); i++) {
        arrayMessage.push_back(arrayMsg.at(i));
    }

    CWizDatabaseManager::instance()->db().setMessageReadStatus(arrayMessage, 1);

    updateTreeItem();
}

void MessageListView::on_action_message_delete()
{
    QList<WIZMESSAGEDATA> arrayMsg;
    specialFocusedMessages(arrayMsg);

    for (int i = 0; i < arrayMsg.size(); i++) {
        CWizDatabaseManager::instance()->db().deleteMessageEx(arrayMsg.at(i));
    }

    updateTreeItem();
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
        }
    }

    updateTreeItem();
}

void MessageListView::on_message_deleted(const WIZMESSAGEDATA& msg)
{
    int i = rowFromId(msg.nId);
    if (i != -1) {
        takeItem(i);
    }

    updateTreeItem();
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



} // namespace Internal
} // namespace WizService
