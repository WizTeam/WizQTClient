#ifndef WIZSERVICE_MESSAGECOMPLETER_H
#define WIZSERVICE_MESSAGECOMPLETER_H

#include <QCompleter>

class QLineEdit;

struct WIZDOCUMENTDATAEX;

class WizDocumentView;
class INoteView;


class WizMessageCompleter : public QCompleter
{
    Q_OBJECT

public:
    explicit WizMessageCompleter(QWidget* parent);

private:
    QLineEdit* m_title;
    WizDocumentView* noteView();
    void update(const QString& strKbGUID);

private Q_SLOTS:
    void onNoteLoaded(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool ok);
};


#endif // WIZSERVICE_MESSAGECOMPLETER_H
