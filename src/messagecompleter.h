#ifndef WIZSERVICE_MESSAGECOMPLETER_H
#define WIZSERVICE_MESSAGECOMPLETER_H

#include <QCompleter>

class QLineEdit;

struct WIZDOCUMENTDATA;

namespace Core {
class CWizDocumentView;
class INoteView;
}

namespace WizService {

class MessageCompleter : public QCompleter
{
    Q_OBJECT

public:
    explicit MessageCompleter(QWidget* parent);

private:
    QLineEdit* m_title;
    Core::CWizDocumentView* noteView();
    void update(const QString& strKbGUID);

private Q_SLOTS:
    void onNoteLoaded(Core::INoteView* view, const WIZDOCUMENTDATA& doc, bool ok);
};

} // namespace WizService

#endif // WIZSERVICE_MESSAGECOMPLETER_H
