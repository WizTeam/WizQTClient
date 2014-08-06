#ifndef WIZNOTEINFOFORM_H
#define WIZNOTEINFOFORM_H

#include "share/wizpopupwidget.h"
#include "share/wizDatabaseManager.h"

namespace Ui {
class CWizNoteInfoForm;
}

class CWizNoteInfoForm : public CWizPopupWidget
{
    Q_OBJECT
    
public:
    explicit CWizNoteInfoForm(QWidget *parent = 0);
    ~CWizNoteInfoForm();

    void setDocument(const WIZDOCUMENTDATA& data);

protected:
    virtual QSize sizeHint() const;

private slots:
    void on_labelOpenDocument_linkActivated(const QString &link);

private:
    Ui::CWizNoteInfoForm *ui;
    QString m_docKbGuid;
    QString m_docGuid;
};


#endif // WIZNOTEINFOFORM_H
