#include "messagecompleter.h"

#include <QLineEdit>
#include <QStringList>
#include <QStringListModel>
#include <QAbstractItemView>
#include <QDebug>

#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"

namespace WizService {

MessageCompleter::MessageCompleter(QWidget *parent)
    : QCompleter(parent)
{
    m_title = qobject_cast<QLineEdit*>(parent);
    Q_ASSERT(m_title);

    connect(m_title, SIGNAL(textChanged(const QString&)), SLOT(onTitleTextChanged(const QString&)));

    //m_view = popup();
    setCompletionMode(QCompleter::PopupCompletion);
    setCompletionPrefix("@");
    setCaseSensitivity(Qt::CaseInsensitive);
    setFilterMode(Qt::MatchStartsWith);

    init();
}

void MessageCompleter::init()
{
    CWizBizUserDataArray arrayUser;
    if (!CWizDatabaseManager::instance()->db().GetAllUsers(arrayUser)) {
        return;
    }

    QStringList lsUsers;
    CWizBizUserDataArray::const_iterator it = arrayUser.begin();
    for (; it != arrayUser.end(); it++) {
        const WIZBIZUSER& user = *it;
        lsUsers += user.alias;
    }

    qDebug() << lsUsers;

    QStringListModel* model = new QStringListModel(this);
    model->setStringList(lsUsers);
    setModel(model);

    //m_view->setModel(model);
    //setPopup(m_view);
}

void MessageCompleter::onTitleTextChanged(const QString& str)
{
    qDebug() << str;
    if (str.endsWith("@"))
        complete();
}



} // namespace WizService
