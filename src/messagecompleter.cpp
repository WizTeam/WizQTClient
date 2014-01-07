#include "messagecompleter.h"

#include <QLineEdit>
#include <QStringList>
#include <QAbstractListModel>
#include <QAbstractItemView>
#include <QListView>
#include <QPainter>
#include <QItemDelegate>
#include <QDebug>

#include "sync/avatar.h"
#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include "utils/pinyin.h"

namespace WizService {

class MessageCompleterModel : public QAbstractListModel
{
    struct UserItem {
        QString strUserId;
        QString strAlias;
        QString strPinyin;
    };

public:
    MessageCompleterModel(const CWizBizUserDataArray& arrayUser, QObject* parent = 0)
        : QAbstractListModel(parent)
    {
        CWizBizUserDataArray::const_iterator it = arrayUser.begin();
        for (; it != arrayUser.end(); it++) {
            const WIZBIZUSER& user = *it;

            wchar_t name[user.alias.size()];
            user.alias.toWCharArray(name);
            QString py;
            WizToolsChinese2PinYin(name, WIZ_C2P_POLYPHONE, py); // FIXME

            if (py.isEmpty()) {
                py = user.alias; // not chinese
            } else {
                py = py.split("\n").at(0);
                py = py.replace(",", "");
            }

            insertRows(0, 1);
            m_users[0].strUserId = user.userId;
            m_users[0].strAlias = user.alias;
            m_users[0].strPinyin = py;
        }
    }

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const
    {
        if (parent.isValid())
            return 0;

        return m_users.size();
    }

    virtual QVariant data(const QModelIndex &index, int role) const
    {
        if (index.row() <= 0 || index.row() >= m_users.size()) {
            return QVariant();
        }

        if (role == Qt::EditRole) {
            return m_users[index.row()].strPinyin;
        } else if (role == Qt::DisplayRole) {
            return m_users[index.row()].strAlias;
        } else if (role == Qt::DecorationRole) {
            QPixmap pm;
            Internal::AvatarHost::avatar(m_users[index.row()].strUserId, &pm);
            return pm;
        }

        return QVariant();
    }

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if (index.row() >= 0 && index.row() < m_users.size()) {
            if (role == Qt::EditRole) {
                m_users[index.row()].strPinyin = value.toString();
                Q_EMIT dataChanged(index, index);
                return true;
            } else if (role == Qt::DisplayRole) {
                m_users[index.row()].strAlias = value.toString();
                Q_EMIT dataChanged(index, index);
                return true;
            }
        }

        return false;
    }

    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex())
    {
        if (count < 1 || row < 0 || row > rowCount(parent))
            return false;

        beginInsertRows(QModelIndex(), row, row + count - 1);

        for (int r = 0; r < count; r++) {
            m_users.insert(row, UserItem());
        }

        endInsertRows();

        return true;
    }

private:
    QList<UserItem> m_users;
};


class MessageCompleterPopupDelegate: public QItemDelegate
{
public:
    explicit MessageCompleterPopupDelegate(QObject *parent = 0) : QItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index) const
    {
        QColor color(rand() % 255, rand() % 255, rand() % 255, 200);
        QBrush brush(color);
        painter->setBackground(brush);
        painter->setPen(color);
        painter->drawRect(option.rect);

        Qt::Alignment alignment = Qt::AlignRight;
        QTextOption textOption(alignment);
        QRectF rect = option.rect;
        qDebug() << index.data(Qt::DisplayRole);
        painter->drawText(rect, index.data().toString(), textOption);
    }
};


class MessageCompleterPopup : public QListView
{
public:
    MessageCompleterPopup(QWidget* parent = 0)
        : QListView(parent)
    {
        setItemDelegate(new MessageCompleterPopupDelegate());
    }
};


MessageCompleter::MessageCompleter(QWidget *parent)
    : QCompleter(parent)
{
    m_title = qobject_cast<QLineEdit*>(parent);
    Q_ASSERT(m_title);

    setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    setCaseSensitivity(Qt::CaseInsensitive);
    setWrapAround(false);

    CWizBizUserDataArray arrayUser;
    if (CWizDatabaseManager::instance()->db().GetAllUsers(arrayUser)) {
        MessageCompleterModel* model = new MessageCompleterModel(arrayUser, this);
        setModel(model);

        MessageCompleterPopup* popup = new MessageCompleterPopup();
        popup->setModel(model);
        setPopup(popup);
    }
}


} // namespace WizService
