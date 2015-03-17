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

    void on_editTitle_editingFinished();

    void on_editURL_editingFinished();

    void on_editAuthor_editingFinished();

    void on_checkEncrypted_clicked(bool checked);

private:
    Ui::CWizNoteInfoForm *ui;
    QString m_docKbGuid;
    QString m_docGuid;
};


#endif // WIZNOTEINFOFORM_H
