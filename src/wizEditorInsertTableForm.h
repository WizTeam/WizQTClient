#ifndef WIZEDITORINSERTTABLEFORM_H
#define WIZEDITORINSERTTABLEFORM_H

#include <QDialog>

namespace Ui {
class CWizEditorInsertTableForm;
}

class CWizEditorInsertTableForm : public QDialog
{
    Q_OBJECT
    
public:
    explicit CWizEditorInsertTableForm(QWidget *parent = 0);
    ~CWizEditorInsertTableForm();

    int getRows();
    int getCols();
    void clear();
    
private slots:
    void on_pushButton_cancel_clicked();

    void on_pushButton_ok_clicked();

private:
    Ui::CWizEditorInsertTableForm *ui;
};

#endif // WIZEDITORINSERTTABLEFORM_H
