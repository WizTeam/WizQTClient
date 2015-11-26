#ifndef WIZSERVICE_INTERNAL_MESSAGELISTVIEW_H
#define WIZSERVICE_INTERNAL_MESSAGELISTVIEW_H

#include <QListWidget>
#include <QTimer>
#include <deque>
#include <QComboBox>
#include <QLabel>
#include <QList>
#include <QString>
#include <QToolButton>
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include "share/wizpopupwidget.h"
#include "wizdef.h"

class CWizScrollBar;
class CWizDatabaseManager;
class wizImageButton;
class CString;

struct WIZMESSAGEDATA;
struct WIZDOCUMENTDATA;
typedef std::deque<WIZMESSAGEDATA> CWizMessageDataArray;
typedef std::deque<CString> CWizStdStringArray;

#ifdef Q_OS_LINUX
#define WIZNOTE_CUSTOM_SCROLLBAR
#else
//#if QT_VERSION < 0x050000
#define WIZNOTE_CUSTOM_SCROLLBAR
//#endif
#endif

namespace WizService {
class AsyncApi;

namespace Internal {

class MessageListViewItem;

class WizSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    WizSortFilterProxyModel(QObject *parent = 0);
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

class WizSenderSelectorItem : public QListWidgetItem
{
public:
    explicit WizSenderSelectorItem(const QString& text, const QString& id, const QPixmap& avatar,
                                   QListWidget *view = 0, int type = Type);

    void draw(QPainter* p, const QStyleOptionViewItemV4* vopt) const;
    QString itemID() const;
    QString itemText() const;

    virtual bool operator<(const QListWidgetItem &other) const;

private:
    QPixmap m_avatar;
    QString m_text;
    QString m_id;
};

class WizMessageSenderSelector : public CWizPopupWidget
{
    Q_OBJECT
public:
    WizMessageSenderSelector(CWizDatabaseManager& dbMgr, QWidget* parent = 0);

    virtual QSize sizeHint() const;

    void setUsers(const CWizStdStringArray& arraySender);
    void appendUser(const QString& userGUID);
    void sort();

    QSet<QString>& userGUIDSet();

public slots:
    void on_selectorItem_clicked(QListWidgetItem* selectorItem);

signals:
    void senderSelected(const QString& userGUID, const QString& userAlias);


private:
    void addUser(const QString& userGUID);
    void setWidgetSize(const QSize& size);

    void adjustWidgetHeight();

private:
    CWizDatabaseManager& m_dbMgr;
    QListWidget* m_userList;
    QSet<QString> m_userGUIDSet;
};

class WizClickableLabel : public QLabel
{
    Q_OBJECT
public:
    WizClickableLabel(QWidget* parent = 0)
        : QLabel(parent)
    {}

signals:
    void labelClicked();

protected:
    void mouseReleaseEvent(QMouseEvent* ev);
};

class WizMessageSelectorItemDelegate : public QStyledItemDelegate
{
public:
    WizMessageSelectorItemDelegate(QObject *parent = 0);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option, const QModelIndex &index) const;

};

class WizMessageListTitleBar : public QWidget
{
    Q_OBJECT
public:
    WizMessageListTitleBar(CWizExplorerApp& app, QWidget* parent = 0);

    void setUnreadMode(bool unread, int unreadCount);
    bool isUnreadMode() const;

    QString currentSenderGUID() const;

signals:
    void messageSelector_senderSelected(QString userGUID);
    void markAllMessageRead_request(bool removeItems);

public slots:
    void on_message_created(const WIZMESSAGEDATA& msg);

    void on_sender_selected(const QString& userGUID, const QString& userAlias);

    void on_userSelectButton_clicked();
    void on_markAllReadbutton_clicked();

    void showUserSelector();

private:
    void addUserToSelector(const QString& userGUID);
    void initSenderSelector();
    void initUserList();
    void showTipsWidget();

private:
    CWizDatabaseManager& m_dbMgr;
    CWizExplorerApp& m_app;
    WizMessageSenderSelector* m_msgSenderSelector;
    WizClickableLabel* m_labelCurrentSender;
    QString m_currentSenderGUID;
    QToolButton* m_btnSelectSender;
    QLabel* m_msgListHintLabel;
    wizImageButton* m_msgListMarkAllBtn;
    bool m_bUnreadMode;
    int m_nUnreadCount;
};

class MessageListView : public QListWidget
{
    Q_OBJECT

public:
    explicit MessageListView(CWizDatabaseManager& dbMgr, QWidget *parent = 0);
    virtual QSize sizeHint() const { return QSize(200, 1); }

    void setMessages(const CWizMessageDataArray& arrayMsg);
    void addMessages(const CWizMessageDataArray& arrayMsg);
    void addMessage(const WIZMESSAGEDATA& msg, bool sort);
    void selectedMessages(QList<WIZMESSAGEDATA>& arrayMsg);
    void specialFocusedMessages(QList<WIZMESSAGEDATA>& arrayMsg);
    void selectMessage(qint64 nId);

    int rowFromId(qint64 nId) const;
    MessageListViewItem* messageItem(int row) const;
    MessageListViewItem* messageItem(const QModelIndex& index) const;
    const WIZMESSAGEDATA& messageFromIndex(const QModelIndex& index) const;

    void drawItem(QPainter* p, const QStyleOptionViewItemV4* vopt) const;

public slots:
    void markAllMessagesReaded(bool removeItems);
    void on_uploadReadStatus_finished(const QString& ids);
    void on_uploadDeleteStatus_finished(const QString& ids);

protected:
    virtual void resizeEvent(QResizeEvent* event);
    virtual void contextMenuEvent(QContextMenuEvent* event);
    virtual void wheelEvent(QWheelEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);

private:
    QMenu* m_menu;
#ifdef WIZNOTE_CUSTOM_SCROLLBAR
    CWizScrollBar* m_vScroll;
#endif

    MessageListViewItem* m_pCurrentItem;
    QList<MessageListViewItem*> m_rightButtonFocusedItems;
    QTimer m_timerRead;
    QList<qint64> m_readList;
    QList<qint64> m_deleteList;
    QTimer m_timerTriggerSync;
    WizService::AsyncApi* m_api;
    CWizDatabaseManager& m_dbMgr;

    void updateTreeItem();

Q_SIGNALS:
    void sizeChanged(int nCount);
    void loacteDocumetRequest(const QString strKbGuid, const QString strGuid);
    void viewNoteInSparateWindowRequest(const WIZDOCUMENTDATA& doc);
    void viewMessageRequest(const WIZMESSAGEDATA& msg);

private Q_SLOTS:
    void onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void onReadTimeout();
    void onSyncTimeout();

    void onAvatarLoaded(const QString& strUserId);

    void on_action_message_mark_read();
    void on_action_message_delete();
    void on_action_message_locate();
    void on_action_message_viewInSeparateWindow();

    void on_message_created(const WIZMESSAGEDATA& msg);
    void on_message_modified(const WIZMESSAGEDATA& oldMsg,
                             const WIZMESSAGEDATA& newMsg);
    void on_message_deleted(const WIZMESSAGEDATA& msg);

    void on_itemSelectionChanged();
    void on_itemDoubleClicked(QListWidgetItem * item);

    void clearRightMenuFocus();
};

} // namespace Internal
} // namespace WizService

#endif // WIZSERVICE_INTERNAL_MESSAGELISTVIEW_H
