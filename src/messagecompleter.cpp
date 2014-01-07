#include "messagecompleter.h"

#include <QLineEdit>
#include <QStringList>
#include <QStringListModel>
#include <QAbstractItemView>
#include <QListView>
#include <QDebug>

#include "sync/avatar.h"
#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include "utils/pinyin.h"

namespace WizService {

class MessageCompleterModel : public QStringListModel
{
public:
    MessageCompleterModel(QObject* parent = 0)
        : QStringListModel(parent)
    {

    }
};

class MessageCompleterPopup : public QListView
{
public:
    MessageCompleterPopup(QWidget* parent = 0)
        : QListView(parent)
    {
        //setViewMode(QListView::IconMode);
        //setFlow(QListView::TopToBottom);
        //setMovement(QListView::Static);
    }
};

MessageCompleter::MessageCompleter(QWidget *parent)
    : QCompleter(parent)
{
    m_title = qobject_cast<QLineEdit*>(parent);
    Q_ASSERT(m_title);

    //connect(this, SIGNAL(activated(const QString&)), SLOT(onInsertCompletion(const QString&)));

    //connect(m_title, SIGNAL(textChanged(const QString&)), SLOT(onTitleTextChanged(const QString&)));

    setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    setCaseSensitivity(Qt::CaseInsensitive);

    init();

    MessageCompleterPopup* popup = new MessageCompleterPopup();
    popup->setModel(model());
    setPopup(popup);
}

void MessageCompleter::init()
{
    CWizBizUserDataArray arrayUser;
    if (!CWizDatabaseManager::instance()->db().GetAllUsers(arrayUser)) {
        return;
    }

    MessageCompleterModel* model = new MessageCompleterModel(this);

    CWizBizUserDataArray::const_iterator it = arrayUser.begin();
    for (; it != arrayUser.end(); it++) {
        const WIZBIZUSER& user = *it;

        wchar_t name[user.alias.size()];
        user.alias.toWCharArray(name);
        QString py;
        WizToolsChinese2PinYin(name, WIZ_C2P_POLYPHONE, py);

        QPixmap pm;
        Internal::AvatarHost::avatar(user.userId, &pm);

        model->insertRows(0, 1);
        QModelIndex i = model->index(0);
        model->setData(i, py, Qt::EditRole);
        model->setData(i, user.alias, Qt::DisplayRole);
        model->setData(i, pm, Qt::DecorationRole);
    }

    setModel(model);
}


} // namespace WizService
