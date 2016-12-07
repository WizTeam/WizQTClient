#ifndef WIZEDITORINSERTTABLEFORM_H
#define WIZEDITORINSERTTABLEFORM_H

#include <QDialog>

namespace Ui {
class WizEditorInsertTableForm;
}

class WizEditorInsertTableForm : public QDialog
{
    Q_OBJECT
    
public:
    explicit WizEditorInsertTableForm(QWidget *parent = 0);
    ~WizEditorInsertTableForm();

    int getRows();
    int getCols();
    void clear();
    
private slots:
    void on_pushButton_cancel_clicked();

    void on_pushButton_ok_clicked();

private:
    Ui::WizEditorInsertTableForm *ui;
};

#endif // WIZEDITORINSERTTABLEFORM_H
