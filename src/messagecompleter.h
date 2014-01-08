#ifndef WIZSERVICE_MESSAGECOMPLETER_H
#define WIZSERVICE_MESSAGECOMPLETER_H

#include <QCompleter>

class QLineEdit;

namespace WizService {

class MessageCompleter : public QCompleter
{
    Q_OBJECT

public:
    explicit MessageCompleter(QWidget* parent);

private:
    QLineEdit* m_title;
};

} // namespace WizService

#endif // WIZSERVICE_MESSAGECOMPLETER_H
