#ifndef WIZSERVICE_MESSAGECOMPLETER_H
#define WIZSERVICE_MESSAGECOMPLETER_H

#include <QCompleter>

class QLineEdit;
class QAbstractItemView;
class QStringListModel;
class MessageCompleterModel;

namespace WizService {

class MessageCompleter : public QCompleter
{
    Q_OBJECT

public:
    explicit MessageCompleter(QWidget* parent);

private:
    QLineEdit* m_title;
    QStringListModel* m_model;
    //MessageCompleterModel* m_model;
};

} // namespace WizService

#endif // WIZSERVICE_MESSAGECOMPLETER_H
