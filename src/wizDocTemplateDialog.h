#ifndef WIZDOCTEMPLATEDIALOG_H
#define WIZDOCTEMPLATEDIALOG_H

#include <QDialog>

namespace Ui {
class CWizDocTemplateDialog;
}

class CWizDocTemplateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CWizDocTemplateDialog(QWidget *parent = 0);
    ~CWizDocTemplateDialog();

private slots:
    void on_btn_downloadNew_clicked();

    void on_btn_useLocal_clicked();

private:
    enum StackIndex {
        StackIndex_downloadNew,
        StackIndex_useLocal
    };

    void shiftStackIndex(StackIndex index);
    //
    QStringList getLocalTemplates();

private:
    Ui::CWizDocTemplateDialog *ui;
};

#endif // WIZDOCTEMPLATEDIALOG_H
