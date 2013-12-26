#ifndef WIZSERVICE_INTERNAL_MESSAGELISTVIEW_H
#define WIZSERVICE_INTERNAL_MESSAGELISTVIEW_H

#include <QListWidget>
#include <deque>

class CWizScrollBar;

struct WIZMESSAGEDATA;
typedef std::deque<WIZMESSAGEDATA> CWizMessageDataArray;

namespace WizService {
namespace Internal {

class MessageListView : public QListWidget
{
    Q_OBJECT

public:
    explicit MessageListView(QWidget *parent = 0);

    void setMessages(const CWizMessageDataArray& arrayMsg);
    void addMessages(const CWizMessageDataArray& arrayMsg);
    void addMessage(const WIZMESSAGEDATA& msg, bool sort);

    const WIZMESSAGEDATA& MessageFromIndex(const QModelIndex &index) const;

protected:
    virtual void resizeEvent(QResizeEvent* event);

private:
    CWizScrollBar* m_vScroll;

Q_SIGNALS:
    void sizeChanged(int nCount);
};

} // namespace Internal
} // namespace WizService

#endif // WIZSERVICE_INTERNAL_MESSAGELISTVIEW_H
