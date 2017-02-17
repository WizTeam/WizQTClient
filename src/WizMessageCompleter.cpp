#include "WizMessageCompleter.h"

#include <QLineEdit>
#include <QStringList>
#include <QStringListModel>
#include <QAbstractListModel>
#include <QAbstractItemView>
#include <QListView>
#include <QPainter>
#include <QItemDelegate>
#include <QScrollBar>
#include <QDebug>

#include "sync/WizAvatarHost.h"
#include "share/WizDatabaseManager.h"
#include "share/WizDatabase.h"
#include "share/WizGlobal.h"
#include "utils/WizPinyin.h"
#include "utils/WizStyleHelper.h"

#include "WizDocumentView.h"

class MessageCompleterModel : public QAbstractListModel
{
public:
    struct UserItem {
        QString strUserId;
        QString strAlias;
        QString strPinyin;
    };

    MessageCompleterModel(const CWizBizUserDataArray& arrayUser, QObject* parent = 0);

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

        switch (role) {
        case Qt::EditRole:
            return m_users[index.row()].strPinyin;
        case Qt::DisplayRole:
            return m_users[index.row()].strAlias;
        case Qt::DecorationRole:
        {
            QPixmap pm;
            WizAvatarHost::avatar(m_users[index.row()].strUserId, &pm);
            return pm.scaled(20, 20, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        case Qt::ToolTipRole:
            return m_users[index.row()].strUserId;
        case Qt::FontRole:
        {
            QFont font;
            font.setPixelSize(12);
            return font;
        }
        case Qt::BackgroundRole:
            return QColor(Qt::white);
        default:
            break;
        }

        return QVariant();
    }

private:
    QList<UserItem> m_users;
};

bool caseInsensitiveLessThan(const MessageCompleterModel::UserItem& item1,const MessageCompleterModel::UserItem& item2)
{
    return item1.strPinyin.toLower() < item2.strPinyin.toLower();
}

MessageCompleterModel::MessageCompleterModel(const CWizBizUserDataArray& arrayUser, QObject* parent)
    : QAbstractListModel(parent)
{
    CWizBizUserDataArray::const_iterator it = arrayUser.begin();
    for (; it != arrayUser.end(); it++) {
        const WIZBIZUSER& user = *it;

        QString part1 = user.alias;

        QString part2;
        WizToolsChinese2PinYin(user.alias, WIZ_C2P_POLYPHONE, part2); // FIXME
        if (!part2.isEmpty())
        {
            part2 = part2.replace(",", "");
        }
        //
#if QT_VERSION >= 0x050200
        QString part3;
        WizToolsChinese2PinYin(user.alias, WIZ_C2P_FIRST_LETTER_ONLY | WIZ_C2P_POLYPHONE, part3);
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
    if (!m_users.isEmpty())
    {
        qSort(m_users.begin(), m_users.end(), caseInsensitiveLessThan);
        m_users.insert(0, UserItem());
        m_users[0].strUserId = QString();
        m_users[0].strAlias = tr("all");
        m_users[0].strPinyin = "all";
    }
}

class MessageCompleterPopupDelegate: public QItemDelegate
{
public:
    explicit MessageCompleterPopupDelegate(QObject *parent = 0) : QItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index) const
    {    
        if (option.state & QStyle::State_Selected)
        {
            painter->fillRect(option.rect, QBrush("#5990EF"));
        }

        QColor color(rand() % 255, rand() % 255, rand() % 255, 200);
        painter->setPen(color);

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
        setStyleSheet("QListView::item:selected {background-color:#5990EF;}");
        verticalScrollBar()->setStyleSheet(Utils::WizStyleHelper::wizCommonScrollBarStyleSheet(2));
    }

    virtual int sizeHintForRow (int row) const
    {
        return QListView::sizeHintForRow(row) + spacing() * 2;
    }

    virtual int sizeHintForColumn(int column) const
    {
        return QListView::sizeHintForColumn(column) + spacing() * 2;
    }

protected:
    void showEvent(QShowEvent* ev)
    {
        QListView::showEvent(ev);

        setMask(Utils::WizStyleHelper::borderRadiusRegion(rect()));
    }
};


WizMessageCompleter::WizMessageCompleter(QWidget *parent)
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

    connect(WizGlobal::instance(), SIGNAL(viewNoteLoaded(WizDocumentView*,const WIZDOCUMENTDATAEX&,bool)),
            SLOT(onNoteLoaded(WizDocumentView*,const WIZDOCUMENTDATAEX&,bool)));
}

WizDocumentView* WizMessageCompleter::noteView()
{
    QWidget* pParent = m_title->parentWidget();
    while (pParent) {
        WizDocumentView* view = dynamic_cast<WizDocumentView*>(pParent);
        if (view) {
            return view;
        }

        pParent = pParent->parentWidget();
    }

    Q_ASSERT(0);
    return 0;
}

void WizMessageCompleter::onNoteLoaded(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool ok)
{
    if (view != noteView())
        return;

    if (!ok)
        return;

    update(doc.strKbGUID);
}

void WizMessageCompleter::update(const QString& strKbGUID)
{
    CWizBizUserDataArray arrayUser;
    if (WizDatabaseManager::instance()->db().users(strKbGUID, arrayUser)) {
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

