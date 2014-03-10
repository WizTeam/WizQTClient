#include "messagecompleter.h"

#include <QLineEdit>
#include <QStringList>
#include <QStringListModel>
#include <QAbstractListModel>
#include <QAbstractItemView>
#include <QListView>
#include <QPainter>
#include <QItemDelegate>
#include <QDebug>

#include <coreplugin/icore.h>

#include "sync/avatar.h"
#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include "utils/pinyin.h"

#include "wizDocumentView.h"

using namespace Core;

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
            //
            QString part1 = user.alias;

            QString part2;
            WizToolsChinese2PinYin(name, WIZ_C2P_POLYPHONE, part2); // FIXME
            if (!part2.isEmpty())
            {
                part2 = part2.replace(",", "");
            }
            //
#if QT_VERSION >= 0x050200
            QString part3;
            WizToolsChinese2PinYin(name, WIZ_C2P_FIRST_LETTER_ONLY | WIZ_C2P_POLYPHONE, part3);
            if (!part3.isEmpty())
            {
                part3 = part3.replace(",", "");
            }
#endif
            //
#if QT_VERSION >= 0x050200
            QString matchText = part1 + "\n" + part2 + "\n" + part3;
#else
            QString matchText;
            if (part2.isEmpty())
            {
                matchText = part1;
            }
            else
            {
                matchText = part2;
            }
#endif

            m_users.insert(0, UserItem());
            m_users[0].strUserId = user.userId;
            m_users[0].strAlias = user.alias;
            m_users[0].strPinyin = matchText;
        }
    }

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const
    {
        Q_UNUSED(parent);
        return m_users.size();
    }

    virtual QVariant data(const QModelIndex& index, int role) const
    {
        if (index.row() < 0 || index.row() >= m_users.size()) {
            return QVariant();
        }

        if (role == Qt::EditRole) {
            return m_users[index.row()].strPinyin;
        } else if (role == Qt::DisplayRole) {
            return m_users[index.row()].strAlias;
        } else if (role == Qt::DecorationRole) {
            QPixmap pm;
            AvatarHost::avatar(m_users[index.row()].strUserId, &pm);
            return pm.scaled(28, 28, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        } else if (role == Qt::ToolTipRole) {
            return m_users[index.row()].strUserId;
        }

        return QVariant();
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
    }

    virtual int sizeHintForRow (int row) const
    {
        return QListView::sizeHintForRow(row) + spacing() * 2;
    }

    virtual int sizeHintForColumn(int column) const
    {
        return QListView::sizeHintForColumn(column) + spacing() * 2;
    }
};


MessageCompleter::MessageCompleter(QWidget *parent)
    : QCompleter(parent)
{
    m_title = qobject_cast<QLineEdit*>(parent);
    Q_ASSERT(m_title);

    setCaseSensitivity(Qt::CaseInsensitive);
    setWrapAround(false);

    setCompletionColumn(0);
    setCompletionRole(Qt::EditRole);
#if QT_VERSION >= 0x050200
    setFilterMode(Qt::MatchContains);
#endif

    connect(Core::ICore::instance(), SIGNAL(viewNoteLoaded(Core::INoteView*,const WIZDOCUMENTDATA&,bool)),
            SLOT(onNoteLoaded(Core::INoteView*,const WIZDOCUMENTDATA&,bool)));
}

CWizDocumentView* MessageCompleter::noteView()
{
    QWidget* pParent = m_title->parentWidget();
    while (pParent) {
        CWizDocumentView* view = dynamic_cast<CWizDocumentView*>(pParent);
        if (view) {
            return view;
        }

        pParent = pParent->parentWidget();
    }

    Q_ASSERT(0);
    return 0;
}

void MessageCompleter::onNoteLoaded(Core::INoteView* view, const WIZDOCUMENTDATA& doc, bool ok)
{
    if (view != noteView())
        return;

    if (!ok)
        return;

    update(doc.strKbGUID);
}

void MessageCompleter::update(const QString& strKbGUID)
{
    CWizBizUserDataArray arrayUser;
    if (CWizDatabaseManager::instance()->db().users(strKbGUID, arrayUser)) {
        MessageCompleterModel* model = new MessageCompleterModel(arrayUser, this);
        setModel(model);

        MessageCompleterPopup* popup = new MessageCompleterPopup();
        popup->setModel(model);
        popup->setSpacing(2);
        popup->setUniformItemSizes(true);
        popup->setResizeMode(QListView::Adjust);

        setPopup(popup);
    }
}

} // namespace WizService
