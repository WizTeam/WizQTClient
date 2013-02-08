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
    explicit CWizNoteInfoForm(CWizDatabaseManager& db, QWidget *parent = 0);
    ~CWizNoteInfoForm();

    virtual QSize sizeHint() const;

    void setDocument(const WIZDOCUMENTDATA& data);

protected:
    CWizDatabaseManager& m_dbMgr;

private:
    Ui::CWizNoteInfoForm *ui;

};


#endif // WIZNOTEINFOFORM_H
