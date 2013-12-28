#include "messagelistview.h"

#include <QListWidgetItem>
#include <QScrollBar>
#include <QResizeEvent>
#include <QPainter>
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
    //Q_OBJECT

public:
    explicit MessageListViewItem(const WIZMESSAGEDATA& data)
    {
        m_data = data;
    }

    const WIZMESSAGEDATA& data() const { return m_data; }

    virtual bool operator <(const QListWidgetItem &other) const
    {
        const MessageListViewItem* pOther = dynamic_cast<const MessageListViewItem*>(&other);
        return pOther->m_data.tCreated < m_data.tCreated;
    }


    //void setSortingType(int type);

    //const WizDocumentListViewItemData& data() { return m_data; }
    //const WIZDOCUMENTDATA& document() const { return m_data.doc; }
    //int itemType() const { return m_data.nType; }
    //void reload(CWizDatabase& db);

    //const WIZABSTRACT& abstract(CWizThumbIndexCache& thumbCache);

    //const QImage& avatar(const CWizDatabase& db,
    //                     CWizUserAvatarDownloaderHost& downloader);

    //// called by CWizDocumentListView when thumbCache pool is ready for reading
    //void resetAbstract(const WIZABSTRACT& abs);
    //void resetAvatar(const QString& strFileName);

private:
    WIZMESSAGEDATA m_data;
    //QString m_strLocation;
    //const QString& tags();
    //const QString& tagTree();

    //bool isAvatarNeedUpdate(const QString& strFileName);

//private Q_SLOTS:
//    void on_thumbnailReloaded();
//
//Q_SIGNALS:
//    void thumbnailReloaded();
};


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

    connect(AvatarHost::instance(), SIGNAL(loaded(const QString&)), SLOT(onAvatarLoaded(const QString&)));
}

void MessageListView::resizeEvent(QResizeEvent* event)
{
    // reset scrollbar position
    m_vScroll->resize(m_vScroll->sizeHint().width(), event->size().height());
    m_vScroll->move(event->size().width() - m_vScroll->sizeHint().width(), 0);

    QListWidget::resizeEvent(event);
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
    // FIXME: document will be deleted while message data still stand alone
    // we should delete these useless messages from database.
    //WIZDOCUMENTDATA data;
    //CWizDatabase& db = CWizDatabaseManager::instance()->db();
    //if (!db.DocumentFromGUID(msg.documentGUID, data)) {
    //    qDebug() << "[MessageListView]Failed to find note of message: " << msg.title;
    //    return;
    //}

    MessageListViewItem* pItem = new MessageListViewItem(msg);
    //pItem->setSizeHint(QSize(sizeHint().width(), fontMetrics().height() * 5));
    pItem->setSizeHint(QSize(sizeHint().width(), Utils::StyleHelper::thumbnailHeight()));
    addItem(pItem);

    if (sort) {
        sortItems();
    }

    Q_EMIT sizeChanged(count());
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
    int nMargin = Utils::StyleHelper::margin();
    Utils::StyleHelper::drawListViewSeperator(p, vopt->rect);
    Utils::StyleHelper::drawListViewBackground(p, vopt->rect, hasFocus(), vopt->state & QStyle::State_Selected);

    QRect rcd = vopt->rect.adjusted(5, 5, -5, -5);

    const WIZMESSAGEDATA& data = messageFromIndex(vopt->index);

    QPixmap pmAvatar;
    WizService::Internal::AvatarHost::avatar(data.senderId, &pmAvatar);
    QRect rectAvatar = Utils::StyleHelper::drawAvatar(p, rcd, pmAvatar);
    rcd.setLeft(rectAvatar.right() + nMargin);

    QFont f(Utils::StyleHelper::fontNormal());

    QString strSender = data.senderAlias.isEmpty() ? data.senderId : data.senderAlias;
    QRect rectSender = Utils::StyleHelper::drawText(p, rcd, strSender, 1, Qt::AlignVCenter, p->pen().color(), f);
    rcd.setTop(rectSender.bottom());

    QRect rcBottom(rcd);
    rcBottom.setTop(rcd.bottom() - QFontMetrics(f).height());
    QString strTime = Utils::Misc::time2humanReadable(data.tCreated);
    QRect rcTime = Utils::StyleHelper::drawText(p, rcBottom, strTime, 1, Qt::AlignRight | Qt::AlignVCenter, p->pen().color(), f);

    p->save();

    QSize sz(rcd.width() - nMargin * 2, rcd.height() - rcTime.height() - nMargin);
    QRect rcMsg(rcd.x() + nMargin, rcd.y() + nMargin + 4, sz.width() - nMargin * 2, sz.height() - nMargin * 2);
    QString strMsg = data.title.isEmpty() ? " " : data.title;
    QRect rcMsg2 = Utils::StyleHelper::drawText(p, rcMsg, strMsg, 2, Qt::AlignVCenter, p->pen().color(), f);

    QPolygon po = Utils::StyleHelper::bubbleFromSize(sz, 4);
    po.translate(rcd.topLeft());
    p->drawPolygon(po);
    p->restore();
}


} // namespace Internal
} // namespace WizService
