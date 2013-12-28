#ifndef WIZSERVICE_INTERNAL_MESSAGELISTVIEW_H
#define WIZSERVICE_INTERNAL_MESSAGELISTVIEW_H

#include <QListWidget>
#include <deque>

class CWizScrollBar;

struct WIZMESSAGEDATA;
typedef std::deque<WIZMESSAGEDATA> CWizMessageDataArray;

namespace WizService {
namespace Internal {

class MessageListViewItem;

class MessageListView : public QListWidget
{
    Q_OBJECT

public:
    explicit MessageListView(QWidget *parent = 0);

    void setMessages(const CWizMessageDataArray& arrayMsg);
    void addMessages(const CWizMessageDataArray& arrayMsg);
    void addMessage(const WIZMESSAGEDATA& msg, bool sort);

    MessageListViewItem* messageItem(int row) const;
    MessageListViewItem* messageItem(const QModelIndex& index) const;
    const WIZMESSAGEDATA& messageFromIndex(const QModelIndex& index) const;

    void drawItem(QPainter* p, const QStyleOptionViewItemV4* vopt) const;

protected:
    virtual void resizeEvent(QResizeEvent* event);

private:
    CWizScrollBar* m_vScroll;

Q_SIGNALS:
    void sizeChanged(int nCount);

private Q_SLOTS:
    void onAvatarLoaded(const QString& strUserId);
};

} // namespace Internal
} // namespace WizService

#endif // WIZSERVICE_INTERNAL_MESSAGELISTVIEW_H
