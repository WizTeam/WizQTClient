#ifndef WIZSERVICE_MESSAGECOMPLETER_H
#define WIZSERVICE_MESSAGECOMPLETER_H

#include <QCompleter>

class QLineEdit;

struct WIZDOCUMENTDATA;

class CWizDocumentView;
class INoteView;


class MessageCompleter : public QCompleter
{
    Q_OBJECT

public:
    explicit MessageCompleter(QWidget* parent);

private:
    QLineEdit* m_title;
    CWizDocumentView* noteView();
    void update(const QString& strKbGUID);

private Q_SLOTS:
    void onNoteLoaded(CWizDocumentView* view, const WIZDOCUMENTDATA& doc, bool ok);
};


#endif // WIZSERVICE_MESSAGECOMPLETER_H
