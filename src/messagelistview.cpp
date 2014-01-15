#include "messagelistview.h"

#include <QListWidgetItem>
#include <QScrollBar>
#include <QResizeEvent>
#include <QPainter>
#include <QMenu>
#include <QDebug>

#include "utils/stylehelper.h"
#include "utils/misc.h"
#include "sync/avatar.h"

#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include "share/wizobject.h"
#include "wiznotestyle.h"
#include "widgets/wizScrollBar.h"

namespace WizService {
namespace Internal {

class MessageListViewItem : public QListWidgetItem
{
public:
    explicit MessageListViewItem(const WIZMESSAGEDATA& data)
    {
        m_data = data;
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
        WizService::Internal::AvatarHost::avatar(m_data.senderId, &pmAvatar);
        QRect rectAvatar = Utils::StyleHelper::drawAvatar(p, rcd, pmAvatar);
        rcd.setLeft(rectAvatar.right());

        QFont f;
        int nHeight = Utils::StyleHelper::fontNormal(f);

        QString strSender = m_data.senderAlias.isEmpty() ? m_data.senderId : m_data.senderAlias;
        QRect rectSender = Utils::StyleHelper::drawText(p, rcd, strSender, 1, Qt::AlignVCenter, p->pen().color(), f);
        rcd.setTop(rectSender.bottom());

        QRect rcBottom(rcd);
        rcBottom.setTop(rcd.bottom() - nHeight);
        QString strTime = Utils::Misc::time2humanReadable(m_data.tCreated);
        QRect rcTime = Utils::StyleHelper::drawText(p, rcBottom, strTime, 1, Qt::AlignRight | Qt::AlignVCenter, p->pen().color(), f);

        QSize sz(rcd.width() - nMargin * 2, rcd.height() - rcTime.height() - nMargin);
        if (!vopt->state.testFlag(QStyle::State_Selected)) {
            QPolygon po = Utils::StyleHelper::bubbleFromSize(sz, 4);
            po.translate(rcd.left() + nMargin, rcd.top());

            p->save();
            if (!m_data.nReadStatus) {
                p->setBrush(QBrush("#f5f5f5"));
            }
            p->setPen("#dcdcdc");
            p->drawPolygon(po);
            p->restore();
        }

        QRect rcMsg(rcd.x() + nMargin, rcd.y() + 4 + nMargin, sz.width(), sz.height());
        QString strMsg = m_data.title.isEmpty() ? " " : m_data.title;
        rcMsg = Utils::StyleHelper::drawText(p, rcMsg, strMsg, 2, Qt::AlignVCenter, p->pen().color(), f);

    }

private:
    WIZMESSAGEDATA m_data;
};

// Message actions
#define WIZACTION_LIST_MESSAGE_MARK_READ    QObject::tr("Mark as read")
#define WIZACTION_LIST_MESSAGE_DELETE       QObject::tr("Delete Message(s)")

MessageListView::MessageListView(QWidget *parent) : QListWidget(parent)
{
    setFrameStyle(QFrame::NoFrame);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QString strSkinName = "default"; // FIXME
    setStyle(::WizGetStyle(strSkinName));
    QPalette pal = palette();
    pal.setColor(QPalette::Base, WizGetDocumentsBackroundColor(strSkinName));
    setPalette(pal);

    // use custom scrollbar
#ifdef Q_OS_LINUX
    setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
#else
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
#endif
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_vScroll = new CWizScrollBar(this);
    m_vScroll->syncWith(verticalScrollBar());

    m_menu = new QMenu(this);
    m_menu->addAction(WIZACTION_LIST_MESSAGE_MARK_READ, this,
                      SLOT(on_action_message_mark_read()));
    m_menu->addAction(WIZACTION_LIST_MESSAGE_DELETE, this,
                      SLOT(on_action_message_delete()));

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
    // reset scrollbar position
    m_vScroll->resize(m_vScroll->sizeHint().width(), event->size().height());
    m_vScroll->move(event->size().width() - m_vScroll->sizeHint().width(), 0);

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



void MessageListView::onAvatarLoaded(const QString& strUserId)
{
    for (int i = 0; i < count(); i++) {
        MessageListViewItem* pItem = messageItem(i);
        if (pItem->data().senderId == strUserId) {
            update(indexFromItem(pItem));
        }
    }
}

void MessageListView::drawItem(QPainter* p, const QStyleOptionViewItemV4* vopt) const
{
    Utils::StyleHelper::drawListViewItemSeperator(p, vopt->rect);
    Utils::StyleHelper::drawListViewItemBackground(p, vopt->rect, hasFocus(), vopt->state & QStyle::State_Selected);

    messageItem(vopt->index)->paint(p, vopt);
}

void MessageListView::on_action_message_mark_read()
{
    QList<WIZMESSAGEDATA> arrayMsg;
    selectedMessages(arrayMsg);

    CWizMessageDataArray arrayMessage;
    for (int i = 0; i < arrayMsg.size(); i++) {
        arrayMessage.push_back(arrayMsg.at(i));
    }

    CWizDatabaseManager::instance()->db().setMessageReadStatus(arrayMessage, 1);
}

void MessageListView::on_action_message_delete()
{
    QList<WIZMESSAGEDATA> arrayMsg;
    selectedMessages(arrayMsg);

    for (int i = 0; i < arrayMsg.size(); i++) {
        CWizDatabaseManager::instance()->db().deleteMessageEx(arrayMsg.at(i));
    }
}

void MessageListView::on_message_created(const WIZMESSAGEDATA& msg)
{
    if (rowFromId(msg.nId) == -1) {
        addMessage(msg, true);
    }
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
}

void MessageListView::on_message_deleted(const WIZMESSAGEDATA& msg)
{
    int i = rowFromId(msg.nId);
    if (i != -1) {
        takeItem(i);
    }
}


} // namespace Internal
} // namespace WizService
