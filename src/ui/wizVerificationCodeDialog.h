#ifndef WIZVERIFICATIONCODEDIALOG_H
#define WIZVERIFICATIONCODEDIALOG_H

#include <QDialog>

namespace Ui {
class CWizVerificationCodeDialog;
}

class CWizVerificationCodeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CWizVerificationCodeDialog(QWidget *parent = 0);
    ~CWizVerificationCodeDialog();

private:
    Ui::CWizVerificationCodeDialog *ui;
};

#endif // WIZVERIFICATIONCODEDIALOG_H
