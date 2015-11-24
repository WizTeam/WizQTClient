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
#if QT_VERSION > 0x050000
#include <QtConcurrent>
#else
#include <QtConcurrentRun>
#endif

#include <extensionsystem/pluginmanager.h>

#include "rapidjson/document.h"

#include "utils/stylehelper.h"
#include "utils/misc.h"
#include "sync/avatar.h"
#include "sync/asyncapi.h"
#include "sync/apientry.h"
#include "sync/token.h"

#include "coreplugin/itreeview.h"

#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include "share/wizobject.h"
#include "share/wizsettings.h"
#include "share/wizsettings.h"
#include "widgets/wizScrollBar.h"
#include "widgets/wizImageButton.h"
#include "widgets/wizTipsWidget.h"

#include "core/wizAccountManager.h"

#include "wiznotestyle.h"
#include "wizCategoryView.h"
#include "wizCategoryViewItem.h"
#include "wizmainwindow.h"

#define ALLMENBERS QObject::tr("All Members")

QString processMssageTitle(CWizDatabaseManager& dbMgr, const WIZMESSAGEDATA& data)
{
    if (data.nMessageType == WIZ_USER_MSG_TYPE_REQUEST_JOIN_GROUP && !data.kbGUID.isEmpty())
    {
        WIZGROUPDATA group;
        if (!dbMgr.db().GetGroupData(data.kbGUID, group))
            return data.title;
        return QObject::tr("%1 applied to jion the group \" %2 \"").
                arg(data.senderAlias.isEmpty() ? data.senderId : data.senderAlias).
                arg(group.strGroupName);
    }
    else if (data.nMessageType == WIZ_USER_MSG_TYPE_ADDED_TO_GROUP && !data.kbGUID.isEmpty())
    {
        WIZGROUPDATA group;
        if (!dbMgr.db().GetGroupData(data.kbGUID, group))
            return data.title;

        return QObject::tr("%1 has agreed you to join the group \" %2 \"").
                arg(data.senderAlias.isEmpty() ? data.senderId : data.senderAlias).
                arg(group.strGroupName);
    }

    return data.title;
}

QString senderText(const WIZMESSAGEDATA& msg)
{
    if (msg.nMessageType == WIZ_USER_MSG_TYPE_SYSTEM)
    {
        return QObject::tr("System message");
    }
    else if (msg.nMessageType == WIZ_USER_MSG_TYPE_REQUEST_JOIN_GROUP)
    {
        return QObject::tr("Verification message");
    }
    else if (msg.nMessageType == WIZ_USER_MSG_TYPE_ADDED_TO_GROUP)
    {
        return QObject::tr("Verified message");
    }

    return msg.senderAlias.isEmpty() ? msg.senderId : msg.senderAlias;
}

QString bodyText(const WIZMESSAGEDATA& msg)
{
    return msg.title.isEmpty() ? msg.messageBody : msg.title;
}

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
            return QObject::tr("modified your note");
            break;
        case WIZ_USER_MSG_TYPE_COMMENT:
            return QObject::tr("comment your note");
            break;
        case WIZ_USER_MSG_TYPE_CALLED_IN_COMMENT:
            return QObject::tr("@ you in note comment");
            break;
        case WIZ_USER_MSG_TYPE_COMMENT_REPLY:
            return QObject::tr("reply your comment");
            break;
        case WIZ_USER_MSG_TYPE_REQUEST_JOIN_GROUP:
            return QString();
            break;
        case WIZ_USER_MSG_TYPE_ADDED_TO_GROUP:
            return QString();
            break;
        case WIZ_USER_MSG_TYPE_LIKE:
            return QObject::tr("like you note");
            break;
        case WIZ_USER_MSG_TYPE_SYSTEM:
            return QString();
            break;
        }
        return QObject::tr("unknown meesage type");
    }



    void draw(QPainter* p, const QStyleOptionViewItemV4* vopt) const
    {     
        //unread
        int nUnreadSymSize = 8;
        if (m_data.nReadStatus == 0)
        {
            QRect rcUnread = QRect(6, vopt->rect.top() + (vopt->rect.height() - nUnreadSymSize) / 2 + 3,
                                   nUnreadSymSize, nUnreadSymSize);
            p->setPen(Qt::NoPen);
            p->setBrush(QBrush(QColor("#5990EF")));
            p->setRenderHint(QPainter::Antialiasing, true);
            p->drawEllipse(rcUnread);
        }

        //avatar
        int nMargin = 12;
        QRect rcd = vopt->rect.adjusted(nMargin + nUnreadSymSize, 26, -nMargin, 0);
        QPixmap pmAvatar;
        WizService::AvatarHost::avatar(m_data.senderId, &pmAvatar);
        QRect rectAvatar = Utils::StyleHelper::drawAvatar(p, rcd, pmAvatar);
        int nAvatarRightMargin = 12;

        QFont f;
        f.setPixelSize(12);
        p->save();        
        p->setPen(QColor("#535353"));
        QString strSender = senderText(m_data);
        QRect  rectSender = rcd;
        rectSender.setLeft(rectAvatar.right() + nAvatarRightMargin);
        rectSender.setTop(vopt->rect.top() + 24);
        rectSender = Utils::StyleHelper::drawText(p, rectSender, strSender, 1,
                                                  Qt::AlignVCenter | Qt::AlignLeft, p->pen().color(), f);

        QString strType = descriptionOfMessageType(m_data.nMessageType);
        if (!strType.isEmpty())
        {
            QColor messageTextColor("#A7A7A7");
            QRect rectType = rcd;
            rectType.setLeft(rectSender.right());
            rectType.setTop(vopt->rect.top() + 25);
            f.setPixelSize(11);
            Utils::StyleHelper::drawText(p, rectType, strType, 1, Qt::AlignVCenter | Qt::AlignLeft,
                                         messageTextColor, f);
        }

        QRect rcTime = vopt->rect.adjusted(0, 8, -14, 0);
        QColor dateColor("#C1C1C1");
        f.setPixelSize(10);
        QString strTime = Utils::Misc::time2humanReadable(m_data.tCreated, "yyyy.MM.dd");
        rcTime = Utils::StyleHelper::drawText(p, rcTime, strTime, 1, Qt::AlignRight | Qt::AlignTop,
                                              dateColor, f);

        QRect rcTitle(rectAvatar.right() + nAvatarRightMargin, rectSender.bottom(), vopt->rect.right() - 12 - rectAvatar.right() - 16,
                      vopt->rect.bottom() - rectSender.bottom());
        f.setPixelSize(14);
        QString strTitle = bodyText(m_data);
        Utils::StyleHelper::drawText(p, rcTitle, strTitle, 1, Qt::AlignLeft| Qt::AlignTop, p->pen().color(), f);

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
    m_vScroll->applyStyle("#F5F5F5", "#C1C1C1", true);
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

    connect(this, SIGNAL(itemSelectionChanged()),
            SLOT(on_itemSelectionChanged()));

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
    if (msg.nDeleteStatus == 1 || msg.nMessageType > WIZ_USERGROUP_MAX)
    {
        return;
    }

    WIZMESSAGEDATA msgData = msg;
    msgData.title = processMssageTitle(m_dbMgr, msgData);

    MessageListViewItem* pItem = new MessageListViewItem(msgData);

    int nHeight = Utils::StyleHelper::messageViewItemHeight();
    pItem->setSizeHint(QSize(sizeHint().width(), nHeight));

    addItem(pItem);

    if (sort)
    {
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
    p->save();    
    MessageListViewItem* pItem = messageItem(vopt->index);

    if (!(vopt->state & QStyle::State_Selected) && pItem->specialFocusd())
    {
        Utils::StyleHelper::drawListViewItemBackground(p, vopt->rect, false, true);
    }
    else
    {
        Utils::StyleHelper::drawListViewItemBackground(p, vopt->rect, hasFocus(), vopt->state & QStyle::State_Selected);
    }
    pItem->draw(p, vopt);

    // draw seperator at last
    Utils::StyleHelper::drawListViewItemSeperator(p, vopt->rect);

    p->restore();
}

void MessageListView::markAllMessagesReaded(bool removeItems)
{
    for (int i = 0; i < count(); i++) {
        MessageListViewItem* pItem = messageItem(i);
        if (!pItem->data().nReadStatus) {
            m_dbMgr.db().setMessageReadStatus(pItem->data());
            m_readList.push_back(pItem->data().nId);
        }
    }
    m_timerTriggerSync.start();

    if (removeItems)
    {
        while (count() > 0)
        {
            if (QListWidgetItem* item = takeItem(0))
            {
                delete item;
            }
        }
    }
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

void MessageListView::on_itemSelectionChanged()
{
    QList<WIZMESSAGEDATA> listMsg;
    selectedMessages(listMsg);

    if (listMsg.size() == 1)
    {
        WIZMESSAGEDATA msgData;
        m_dbMgr.db().messageFromId(listMsg[0].nId, msgData);

        if (msgData.nMessageType < WIZ_USER_MSG_TYPE_REQUEST_JOIN_GROUP ||
                msgData.nMessageType == WIZ_USER_MSG_TYPE_LIKE)
        {
            emit viewMessageRequest(msgData);
        }
        else if (msgData.nMessageType == WIZ_USER_MSG_TYPE_REQUEST_JOIN_GROUP
                 || msgData.nMessageType == WIZ_USER_MSG_TYPE_ADDED_TO_GROUP)
        {
            QtConcurrent::run([msgData]() {
                QString command = QString("message_%1").arg(msgData.nMessageType);
                QString strUrl = WizService::CommonApiEntry::getUrlByCommand(command);
                qDebug() << "message url : " << strUrl;
                strUrl.replace("{token}", WizService::Token::token());
                strUrl.replace("{kb_guid}", msgData.kbGUID.isEmpty() ? "{kb_guid}" : msgData.kbGUID);
                strUrl.replace("{biz_guid}", msgData.bizGUID.isEmpty() ? "{biz_guid}" : msgData.bizGUID);
                strUrl.replace("{document_guid}", msgData.documentGUID.isEmpty() ? "{document_guid}" : msgData.documentGUID);
                strUrl.replace("{sender_guid}", msgData.senderGUID.isEmpty() ? "{sender_guid}" : msgData.senderGUID);
                strUrl.replace("{receiver_guid}", msgData.receiverGUID.isEmpty() ? "{receiver_guid}" : msgData.receiverGUID);
                strUrl.replace("{sender_id}", msgData.senderGUID.isEmpty() ? "{sender_id}" : msgData.senderGUID);
                strUrl.replace("{receiver_id}", msgData.receiverGUID.isEmpty() ? "{receiver_id}" : msgData.receiverGUID);

                QDesktopServices::openUrl(strUrl);
            });
        }
        else if (msgData.nMessageType == WIZ_USER_MSG_TYPE_SYSTEM && !msgData.note.isEmpty())
        {
            if (msgData.isAd())
            {
                CWizAccountManager account(m_dbMgr);
                if (account.isPaidUser())
                    return;
            }
            rapidjson::Document d;
            d.Parse<0>(msgData.note.toUtf8().constData());

            if (d.HasParseError() || !d.FindMember("link")) {
                qDebug() << "Error occured when try to parse json of messages : " << msgData.note;
                return;
            }

            QTextCodec* codec = QTextCodec::codecForName("UTF-8");
            QTextDecoder* encoder = codec->makeDecoder();
            const rapidjson::Value& u = d["link"];
            QString str = encoder->toUnicode(u.GetString(), u.GetStringLength());
            QtConcurrent::run([str]() {
                QString link = str;
                if (link.contains("{token}"))
                {
                    link.replace("{token}", WizService::Token::token());
                }
                QDesktopServices::openUrl(link);
            });
        }
    }
    viewport()->update();
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


const int SenderNameFontSize = 14;

WizMessageListTitleBar::WizMessageListTitleBar(CWizExplorerApp& app, QWidget* parent)
    : QWidget(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_msgSenderSelector(nullptr)
    , m_bUnreadMode(false)
    , m_nUnreadCount(0)
    , m_currentSenderGUID(QString())
{
    setFixedHeight(Utils::StyleHelper::titleEditorHeight());
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor("#F7F7F7"));
    setPalette(pal);
    setAutoFillBackground(true);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    setLayout(hLayout);

    QHBoxLayout* layoutActions = new QHBoxLayout;
    layoutActions->setContentsMargins(2, 0, 0, 0);
    layoutActions->setSpacing(0);

    m_labelCurrentSender = new WizClickableLabel(this);
    m_labelCurrentSender->setText(ALLMENBERS);
    m_labelCurrentSender->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
//    m_labelCurrentSender->setFixedWidth(102);
    m_labelCurrentSender->setMaximumWidth(102);
    m_labelCurrentSender->setStyleSheet(QString("padding-left:7px; padding-top:2px; "
                                                "padding-right:4px; font: %1px;color:#888888;")
                                        .arg(SenderNameFontSize));
    connect(m_labelCurrentSender, SIGNAL(labelClicked()), SLOT(on_userSelectButton_clicked()));
    layoutActions->addWidget(m_labelCurrentSender);

    m_btnSelectSender = new QToolButton(this);
    m_btnSelectSender->setStyleSheet(QString("border:0px;background-image:url(%1); "
                                             "background-repeat: no-repeat;"
                                             "background-position: center;").arg(
                                         Utils::StyleHelper::skinResourceFileName("messageSelectorDownArrow", false)));
    m_btnSelectSender->setFixedWidth(7);
    connect(m_btnSelectSender, SIGNAL(clicked(bool)), SLOT(on_userSelectButton_clicked()));
    layoutActions->addWidget(m_btnSelectSender);

    layoutActions->addStretch();
    m_msgListHintLabel = new QLabel(this);
    m_msgListHintLabel->setText(tr("Unread messages"));
    m_msgListHintLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    m_msgListHintLabel->setStyleSheet("color: #A7A7A7; font-size:14px; padding-top:2px; margin-right:6px;");
    layoutActions->addWidget(m_msgListHintLabel);

    m_msgListMarkAllBtn = new wizImageButton(this);
    QIcon btnIcon = ::WizLoadSkinIcon(Utils::StyleHelper::themeName(), "actionMarkMessagesRead");
    m_msgListMarkAllBtn->setIcon(btnIcon);
    m_msgListMarkAllBtn->setFixedSize(QSize(18, 18));
    m_msgListMarkAllBtn->setToolTip(tr("Mark all messages read"));
    connect(m_msgListMarkAllBtn, SIGNAL(clicked()), SLOT(on_markAllReadbutton_clicked()));
    layoutActions->addWidget(m_msgListMarkAllBtn);

    hLayout->addLayout(layoutActions);

    connect(&m_dbMgr, SIGNAL(messageCreated(WIZMESSAGEDATA)),
            SLOT(on_message_created(WIZMESSAGEDATA)));
}

#define MESSAGELISTTITLEBARTIPSCHECKED      "MessageListTitleBarTipsChecked"

void WizMessageListTitleBar::setUnreadMode(bool unread, int unreadCount)
{
    m_bUnreadMode = unread;
    m_nUnreadCount = unreadCount;
    m_msgListHintLabel->setText(unread ? tr("Unread messages") : tr("All messages"));
    m_msgListMarkAllBtn->setVisible(unread);
    if (m_bUnreadMode)
    {
        layout()->setContentsMargins(2, 0, 6, 0);

        //show tips
        bool showTips = m_app.userSettings().get(MESSAGELISTTITLEBARTIPSCHECKED).toInt() == 0;
        if (showTips)
        {
            showTipsWidget();
        }
    }
    else
    {
        layout()->setContentsMargins(2, 0, 0, 0);
    }
}

bool WizMessageListTitleBar::isUnreadMode() const
{
    return m_bUnreadMode;
}

QString WizMessageListTitleBar::currentSenderGUID() const
{
    return m_currentSenderGUID;
}

void WizMessageListTitleBar::on_message_created(const WIZMESSAGEDATA& msg)
{
    if (msg.senderGUID.isEmpty())
        return;

    if (nullptr == m_msgSenderSelector)
    {
        initSenderSelector();
    }

    if (m_msgSenderSelector->userGUIDSet().contains(msg.senderGUID))
        return;

    addUserToSelector(msg.senderGUID);
}

void WizMessageListTitleBar::on_sender_selected(const QString& userGUID, const QString& userAlias)
{
    QFont font;
    font.setPixelSize(SenderNameFontSize);
    QFontMetrics fm(font);
    QString text = fm.elidedText(userAlias, Qt::ElideRight, 90);
    m_labelCurrentSender->setText(text);
    m_currentSenderGUID = userGUID;
    emit messageSelector_senderSelected(userGUID);
}

void WizMessageListTitleBar::on_userSelectButton_clicked()
{
    showUserSelector();
}

void WizMessageListTitleBar::on_markAllReadbutton_clicked()
{
    emit markAllMessageRead_request(m_bUnreadMode);
}

void WizMessageListTitleBar::showUserSelector()
{
    if (nullptr == m_msgSenderSelector)
    {
        initSenderSelector();
    }

    QPoint leftTop = m_labelCurrentSender->mapToGlobal(m_labelCurrentSender->rect().bottomLeft());
    m_msgSenderSelector->move(leftTop);
    m_msgSenderSelector->show();
}

void WizMessageListTitleBar::initSenderSelector()
{
    m_msgSenderSelector = new WizMessageSenderSelector(m_dbMgr, this);
    initUserList();

    connect(m_msgSenderSelector, SIGNAL(senderSelected(QString,QString)), SLOT(on_sender_selected(QString,QString)));
}

void WizMessageListTitleBar::addUserToSelector(const QString& userGUID)
{
    m_msgSenderSelector->appendUser(userGUID);
}

void WizMessageListTitleBar::initUserList()
{
    CWizStdStringArray arraySender;
    CWizDatabase& db = m_dbMgr.db();
    db.getAllMessageSenders(arraySender);

    m_msgSenderSelector->setUsers(arraySender);
}

void WizMessageListTitleBar::showTipsWidget()
{
    CWizTipListManager* manager = CWizTipListManager::instance();
    if (manager->tipsWidgetExists(MESSAGELISTTITLEBARTIPSCHECKED))
        return;

    CWizTipsWidget* tipWidget = new CWizTipsWidget(MESSAGELISTTITLEBARTIPSCHECKED, this);
    connect(m_msgListMarkAllBtn, SIGNAL(clicked(bool)), tipWidget, SLOT(onTargetWidgetClicked()));
    tipWidget->setAttribute(Qt::WA_DeleteOnClose, true);
    tipWidget->setText(tr("Mark all as readed"), tr("Mark all messages as readed."));
    tipWidget->setSizeHint(QSize(280, 60));
    tipWidget->setButtonVisible(false);
    tipWidget->bindCloseFunction([](){
        if (Core::Internal::MainWindow* mainWindow = Core::Internal::MainWindow::instance())
        {
            mainWindow->userSettings().set(MESSAGELISTTITLEBARTIPSCHECKED, "1");
        }
    });
    tipWidget->addToTipListManager(m_msgListMarkAllBtn, -6, 2);
    //
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


    static bool isChinese = Utils::Misc::isChinese();
    if (isChinese)
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

class WizUserSelectorList : public QListWidget
{
public:
    explicit WizUserSelectorList(QWidget *parent = 0)
        : QListWidget(parent)
    {
        setMouseTracking(true);
        setAttribute(Qt::WA_MacShowFocusRect, false);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }

protected:
    void mouseMoveEvent(QMouseEvent* event)
    {
        QListWidget::mouseMoveEvent(event);

        QListWidgetItem* hoveredItem = itemAt(event->pos());
        if (hoveredItem != nullptr && hoveredItem != currentItem())
        {
            setCurrentItem(hoveredItem);
            update();
        }
    }
};

const int maxSenderSelectorHeight = 378;
const int maxSenderSelectorWidth = 152;
WizMessageSenderSelector::WizMessageSenderSelector(CWizDatabaseManager& dbMgr, QWidget* parent)
    : CWizPopupWidget(parent)
    , m_dbMgr(dbMgr)
    , m_userList(new WizUserSelectorList(this))
{
    setLeftAlign(true);
    setContentsMargins(0, 0, 0, 0);
    QSize wgtSize(maxSenderSelectorWidth, maxSenderSelectorHeight);
    setWidgetSize(wgtSize);
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 14, 0, 8);
    layout->setSpacing(0);
    setLayout(layout);

    layout->addWidget(m_userList);
    //
    setStyleSheet(QString(".QWidget{padding:0px; background-color:#FFFFFF;} \
                  QListWidget{border:0px;padding:0xp;background-color:#FFFFFF;}"));
    m_userList->verticalScrollBar()->setStyleSheet(Utils::StyleHelper::wizCommonScrollBarStyleSheet());
    QPalette pl = palette();
    pl.setColor(QPalette::Window, QColor(Qt::white));
    setPalette(pl);

    connect(m_userList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(on_selectorItem_clicked(QListWidgetItem*)));

    CWizListItemStyle<WizSenderSelectorItem>* listStyle = new CWizListItemStyle<WizSenderSelectorItem>();
    m_userList->setStyle(listStyle);

    connect(m_userList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(on_selectorItem_clicked(QListWidgetItem*)));
}

QSize WizMessageSenderSelector::sizeHint() const
{
    return QSize(maxSenderSelectorWidth, maxSenderSelectorHeight);
}

void WizMessageSenderSelector::setUsers(const CWizStdStringArray& arraySender)
{
    for (auto sender : arraySender)
    {
        addUser(sender);
    }

    QPixmap pix(Utils::StyleHelper::skinResourceFileName("avatar_all"));
    WizSenderSelectorItem* selectorItem = new WizSenderSelectorItem(ALLMENBERS, "", pix, m_userList);
    selectorItem->setSizeHint(QSize(width(), 22));
    m_userList->addItem(selectorItem);

    m_userList->sortItems();

    adjustWidgetHeight();
}

void WizMessageSenderSelector::appendUser(const QString& userGUID)
{
    addUser(userGUID);

    m_userList->sortItems();

    adjustWidgetHeight();
}

QSet<QString>&WizMessageSenderSelector::userGUIDSet()
{
    return m_userGUIDSet;
}

void WizMessageSenderSelector::on_selectorItem_clicked(QListWidgetItem* selectorItem)
{
    WizSenderSelectorItem* senderItem = dynamic_cast<WizSenderSelectorItem*>(selectorItem);
    if (senderItem)
    {
        QString userGUID = senderItem->itemID();

        emit senderSelected(userGUID, senderItem->itemText());
        hide();
    }
}

void WizMessageSenderSelector::addUser(const QString& userGUID)
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

    QPixmap pix;
    WizService::AvatarHost::load(strUserId);
    WizService::AvatarHost::avatar(strUserId, &pix);

    WizSenderSelectorItem* selectorItem = new WizSenderSelectorItem(strText, userGUID, pix, m_userList);
    selectorItem->setSizeHint(QSize(width(), 24));
    m_userList->addItem(selectorItem);

    m_userGUIDSet.insert(userGUID);
}

void WizMessageSenderSelector::sort()
{
    m_userList->sortItems();
}

void WizMessageSenderSelector::setWidgetSize(const QSize& size)
{
    setFixedSize(size);
}

void WizMessageSenderSelector::adjustWidgetHeight()
{
    int nHeight = m_userList->count() > 15 ?  maxSenderSelectorHeight :  m_userList->count() * 22 + 30;
    setWidgetSize(QSize(width(), nHeight));
}

const QSize SelectorAvatarSize = QSize(16, 16);

WizSenderSelectorItem::WizSenderSelectorItem(const QString& text, const QString& id, const QPixmap& avatar,
                                             QListWidget* view, int type)
    : QListWidgetItem(view, type)
    , m_text(text)
    , m_id(id)
    , m_avatar(avatar.scaled(SelectorAvatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation))
{

}

void WizSenderSelectorItem::draw(QPainter* p, const QStyleOptionViewItemV4* vopt) const
{
    p->save();

    bool selected = vopt->state & QStyle::State_Selected;

    if (selected)
    {
        p->fillRect(vopt->rect, QBrush("#5990EF"));
    }

    QRect rcAvatar(vopt->rect.x() + 8, vopt->rect.y() + 4, SelectorAvatarSize.width(), SelectorAvatarSize.height());
    p->drawPixmap(rcAvatar, m_avatar);

    QRect rcText(QPoint(rcAvatar.right() + 8, vopt->rect.y() + 5), QPoint(vopt->rect.right(), vopt->rect.bottom() - 5));
    p->setPen(QColor(selected ? "#FFFFFF" : "#535353"));
    QFont font = p->font();
    font.setPixelSize(12);
    QFontMetrics fm(p->font());
    QString text = fm.elidedText(m_text, Qt::ElideMiddle, rcText.width());
    p->setFont(font);
    p->drawText(rcText, Qt::AlignVCenter | Qt::AlignLeft, text);

    p->restore();
}

QString WizSenderSelectorItem::itemID() const
{
    return m_id;
}

QString WizSenderSelectorItem::itemText() const
{
    return m_text;
}

bool WizSenderSelectorItem::operator<(const QListWidgetItem& other) const
{
    const WizSenderSelectorItem* pOther = dynamic_cast<const WizSenderSelectorItem*>(&other);

    if (m_id.isEmpty())
        return true;
    if (pOther->itemID().isEmpty())
        return false;

    static bool isChinese = Utils::Misc::isChinese();
    if (isChinese)
    {
        if (QTextCodec* pCodec = QTextCodec::codecForName("GBK"))
        {
            QByteArray arrThis = pCodec->fromUnicode(m_text);
            QByteArray arrOther = pCodec->fromUnicode(pOther->itemText());
            //
            std::string strThisA(arrThis.data(), arrThis.size());
            std::string strOtherA(arrOther.data(), arrOther.size());
            //
            return strThisA.compare(strOtherA.c_str()) < 0;
        }
    }

    return m_text.localeAwareCompare(pOther->itemText()) < 0;
}

void WizClickableLabel::mouseReleaseEvent(QMouseEvent* ev)
{
    QLabel::mouseReleaseEvent(ev);

    emit labelClicked();
}


} // namespace Internal
} // namespace WizService
